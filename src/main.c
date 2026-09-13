/*
 * main.c -- Ponto de entrada nativo do loader AArch64 para Alien Shooter v1.2.9
 *
 * Carrega libalien_shooter.so, resolve importações, configura ambiente
 * falso Android + JNI, e gerencia o ciclo de vida do jogo no R36S.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ucontext.h>
#include <unistd.h>

#include "android_shim.h"
#include "asset_shim.h"
#include "bionic_shims.h"
#include "egl_shim.h"
#include "imports.h"
#include "jni_shim.h"
#include "opensles_shim.h"
#include "so_util.h"

/* DRM / License / Java method bypass hooks */
static int hook_ret1(void) {
  return 1;
}

static int hook_ret0(void) {
  return 0;
}

static void hook_void(void) {
  return;
}

/* ===== Google Play Core / BundleManager Bypass Hooks ===== */

/*
 * O jogo usa a Google Play Core Library (playcore) para gerenciar
 * asset packs via BundleManager. No Android, ele chama
 * AssetPackManager_init() que requer a Play Store.
 * Nós interceptamos todo esse fluxo para usar assets locais.
 */

static char g_assets_path[PATH_MAX];

/* BundleManagerPrivate::initialize() -> retorna true (sucesso)
 * Isso impede que AssetPackManager_init seja chamada. */
static int hook_bundle_init(void *this_ptr) {
  (void)this_ptr;
  printf("[hook] BundleManagerPrivate::initialize() -> true (bypass playcore)\n");
  return 1;
}

/* BundleManagerPrivate::update(BundleInfo&) -> retorna true */
static int hook_bundle_update(void *this_ptr, void *bundle_info) {
  (void)this_ptr; (void)bundle_info;
  return 1;
}

/* AssetPackManager_init -> retorna 0 (ASSET_PACK_NO_ERROR) */
static int hook_assetpack_init(void *jvm, void *android_context) {
  (void)jvm; (void)android_context;
  printf("[hook] AssetPackManager_init -> 0 (bypassed)\n");
  return 0;
}

/* Fake AssetPackLocation object */
static struct {
  int storage_method;  /* 0 = STORAGE_FILES_APK, 1 = STORAGE_FILES_EXTERNAL */
  char assets_path[PATH_MAX];
  char pack_path[PATH_MAX];
} g_fake_pack_location;

/* AssetPackManager_getAssetPackLocation -> retorna location fake com path local */
static void *hook_assetpack_get_location(const char *name) {
  printf("[hook] AssetPackManager_getAssetPackLocation('%s') -> local path\n",
         name ? name : "null");
  return &g_fake_pack_location;
}

/* AssetPackLocation_getAssetsPath -> retorna caminho dos assets locais */
static const char *hook_assetpack_assets_path(void *location) {
  (void)location;
  printf("[hook] AssetPackLocation_getAssetsPath -> %s\n", g_assets_path);
  return g_assets_path;
}

/* AssetPackLocation_getStorageMethod -> 0 (STORAGE_FILES_ON_DEVICE) */
static int hook_assetpack_storage_method(void *location) {
  (void)location;
  return 0;
}

/* AssetPackManager_requestInfo -> retorna 0 (sucesso) */
static int hook_assetpack_request_info(const char **pack_names, int num_packs) {
  (void)pack_names; (void)num_packs;
  printf("[hook] AssetPackManager_requestInfo (bypassed)\n");
  return 0;
}

/* AssetPackManager_getDownloadState -> retorna NULL (sem download pendente) */
static void *hook_assetpack_download_state(const char *name) {
  (void)name;
  return NULL;
}

/* AssetPackManager_destroy -> noop */
static void hook_assetpack_destroy(void) {
  printf("[hook] AssetPackManager_destroy (noop)\n");
}

/* BundleManager::resolvePath -> retorna o path local dos assets */
static void hook_bundle_resolve_path(void *this_ptr, void *result_string, const void *input_string) {
  (void)this_ptr; (void)input_string;
  /* A STRING do engine é um tipo complexo. Ao invés de tentar manipulá-la,
   * confiamos que o AssetManager shim resolva os caminhos. */
  printf("[hook] BundleManagerPrivate::resolvePath called (passthrough)\n");
}

/* isBundleAccessable (namespace anônimo) -> sempre true */
static int hook_is_bundle_accessable(const char *path) {
  (void)path;
  printf("[hook] isBundleAccessable('%s') -> true\n", path ? path : "null");
  return 1;
}

static void crash_handler(int sig, siginfo_t *info, void *uctx) {
  ucontext_t *uc = (ucontext_t *)uctx;
  uintptr_t pc = uc ? uc->uc_mcontext.pc : 0;
  uintptr_t fault_addr = info ? (uintptr_t)info->si_addr : 0;
  fprintf(stderr, "\n==================== CRASH INTERCEPTADO ====================\n");
  fprintf(stderr, "Sinal: %d (%s)\n", sig,
          sig == SIGSEGV ? "SIGSEGV" : sig == SIGBUS ? "SIGBUS" : sig == SIGABRT ? "SIGABRT" : "OUTRO");
  fprintf(stderr, "Endereço da Falha: %p\n", (void *)fault_addr);
  fprintf(stderr, "Contador de Programa (PC): %p\n", (void *)pc);
  if (text_base && pc >= (uintptr_t)text_base && pc < (uintptr_t)text_base + text_size) {
    fprintf(stderr, "PC dentro de libalien_shooter.so (offset: +0x%lx)\n", (unsigned long)(pc - (uintptr_t)text_base));
  }
  if (uc) {
    fprintf(stderr, "Registradores:\n");
    for (int i = 0; i < 31; i++) {
      fprintf(stderr, "  x%-2d = 0x%016lx%s", i, (unsigned long)uc->uc_mcontext.regs[i],
              (i % 3 == 2 || i == 30) ? "\n" : "");
    }
    fprintf(stderr, "  sp  = 0x%016lx\n", (unsigned long)uc->uc_mcontext.sp);
  }
  fprintf(stderr, "============================================================\n");
  fflush(stderr);
  fflush(stdout);
  sync();
  _exit(128 + sig);
}

static void install_crash_handler(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = crash_handler;
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
  sigaction(SIGABRT, &sa, NULL);
  sigaction(SIGFPE, &sa, NULL);
  sigaction(SIGILL, &sa, NULL);
}

int main(int argc, char *argv[]) {
  // Desativa bufferização de stdout/stderr para garantir saída no log.txt em caso de crash
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  // Instala capturador de falhas de memória
  install_crash_handler();

  // Garante inicialização do stack guard pad de TLS
  g_bionic_guard_pad[0x28] = 0x42;

  char gamedir[PATH_MAX];
  if (argc > 1 && argv[1]) {
    strncpy(gamedir, argv[1], sizeof(gamedir) - 1);
  } else {
    // Diretório atual de trabalho
    if (!getcwd(gamedir, sizeof(gamedir))) {
      strcpy(gamedir, ".");
    }
  }

  printf("==================================================\n");
  printf("  Alien Shooter v1.2.9 PortMaster / NextOS Loader \n");
  printf("  Alvo: R36S (RK3326 ARM64) - 640x480 @ 60 FPS    \n");
  printf("  Diretório do jogo: %s\n", gamedir);
  printf("==================================================\n");

  /* Inicializa shims básicos */
  printf("[main] Inicializando shims de Asset e JNI...\n");
  asset_shim_init(gamedir);
  jni_shim_init(gamedir);

  /* Busca o binário da engine em caminhos possíveis */
  const char *candidate_paths[] = {
    "lib/arm64-v8a/libalien_shooter.so",
    "lib/libalien_shooter.so",
    "libalien_shooter.so"
  };
  char so_path[PATH_MAX] = {0};
  int found_so = 0;

  for (size_t i = 0; i < sizeof(candidate_paths) / sizeof(candidate_paths[0]); i++) {
    char test_path[PATH_MAX];
    snprintf(test_path, sizeof(test_path), "%s/%s", gamedir, candidate_paths[i]);
    if (access(test_path, F_OK) == 0) {
      snprintf(so_path, sizeof(so_path), "%s", test_path);
      found_so = 1;
      break;
    }
    if (access(candidate_paths[i], F_OK) == 0) {
      snprintf(so_path, sizeof(so_path), "%s", candidate_paths[i]);
      found_so = 1;
      break;
    }
  }

  if (!found_so) {
    fprintf(stderr, "ERRO CRÍTICO: libalien_shooter.so não encontrado em %s!\n", gamedir);
    return 1;
  }
  printf("[main] Biblioteca do jogo encontrada em: %s\n", so_path);

  /* 1. Carrega o ELF da engine */
  printf("[main] Carregando ELF na memória virtual...\n");
  if (so_load(so_path, NULL, 0) != 0) {
    fprintf(stderr, "ERRO: Falha ao carregar %s!\n", so_path);
    return 1;
  }

  /* 2. Aplica relocações internas e relativas */
  printf("[main] Aplicando relocações ELF...\n");
  so_relocate();

  /* 3. Resolve importações com as funções da tabela */
  printf("[main] Resolvendo símbolos de importação com a libc/shims...\n");
  so_resolve(dynlib_functions, dynlib_num_functions, 1);

  /* 4. Trava de Licença, Certificado e Registro JNI */
  so_make_text_writable();

  uintptr_t is_legal_app = so_find_addr("_ZNK4core11Application7isLegalEv");
  if (is_legal_app) {
    hook_arm64(is_legal_app, (uintptr_t)hook_ret1);
    printf("[main] Hooked Application::isLegal -> 1\n");
  }

  uintptr_t is_legal_native = so_find_addr("_ZNK7android17ApplicationNative7isLegalEv");
  if (is_legal_native) {
    hook_arm64(is_legal_native, (uintptr_t)hook_ret1);
    printf("[main] Hooked ApplicationNative::isLegal -> 1\n");
  }

  uintptr_t cert_app = so_find_addr("_ZN4core11Application23checkPackageCertificateEv");
  if (cert_app) {
    hook_arm64(cert_app, (uintptr_t)hook_ret1);
    printf("[main] Hooked Application::checkPackageCertificate -> 1\n");
  }

  uintptr_t cert_native = so_find_addr("_ZN7android17ApplicationNative23checkPackageCertificateEv");
  if (cert_native) {
    hook_arm64(cert_native, (uintptr_t)hook_ret1);
    printf("[main] Hooked ApplicationNative::checkPackageCertificate -> 1\n");
  }

  uintptr_t ad_init = so_find_addr("_ZN2ad21AdvertisementProvider10initializeEv");
  if (ad_init) {
    hook_arm64(ad_init, (uintptr_t)hook_ret0);
    printf("[main] Hooked AdvertisementProvider::initialize -> 0\n");
  }

  /* Força registro bem-sucedido de métodos JNIpp para evitar abort em ApplicationNative::run() */
  uintptr_t reg_methods = so_find_addr("_ZN5jnipp12NativeMethod18registerAllMethodsEv");
  if (reg_methods) {
    hook_arm64(reg_methods, (uintptr_t)hook_ret1);
    printf("[main] Hooked NativeMethod::registerAllMethods -> 1\n");
  }

  /* Desativa verificações de licença, paywalls e saídas forçadas */
  const char *license_hooks_ret0[] = {
    "_ZN7android21LicenseCheckerService10startCheckEv",
    "_ZN7android21LicenseCheckerService18showPaywallAndExitEv",
    "_ZN7android17ApplicationNative17licenseStartCheckEv",
    "_ZN7android17ApplicationNative25licenseShowPaywallAndExitEv",
    "_ZN4core11Application25licenseShowPaywallAndExitEv",
    "_ZN7android17ApplicationNative11forceFinishEv",
    "_ZN7android17ApplicationNative4exitEv",
    "_ZN4core11Application4exitEv",
  };
  for (size_t i = 0; i < sizeof(license_hooks_ret0) / sizeof(license_hooks_ret0[0]); i++) {
    uintptr_t h_addr = so_find_addr(license_hooks_ret0[i]);
    if (h_addr) {
      hook_arm64(h_addr, (uintptr_t)hook_ret0);
      printf("[main] Hooked %s -> 0\n", license_hooks_ret0[i]);
    }
  }

  /* ===== BundleManager / Google Play Core Bypass ===== */
  /* O jogo usa playcore para carregar asset packs. Sem Play Store,
   * AssetPackManager_init falha. Hookamos tudo para usar assets locais. */

  /* Configura caminho dos assets locais */
  snprintf(g_assets_path, sizeof(g_assets_path), "%s/assets", gamedir);
  snprintf(g_fake_pack_location.assets_path, sizeof(g_fake_pack_location.assets_path),
           "%s/assets", gamedir);
  snprintf(g_fake_pack_location.pack_path, sizeof(g_fake_pack_location.pack_path),
           "%s", gamedir);
  g_fake_pack_location.storage_method = 0; /* STORAGE_FILES_ON_DEVICE */
  printf("[main] Assets path configurado: %s\n", g_assets_path);

  /* Hook principal: BundleManagerPrivate::initialize() -> true */
  uintptr_t bm_init = so_find_addr("_ZN4core6detail20BundleManagerPrivate10initializeEv");
  if (bm_init) {
    hook_arm64(bm_init, (uintptr_t)hook_bundle_init);
    printf("[main] Hooked BundleManagerPrivate::initialize -> true\n");
  } else {
    printf("[main] AVISO: BundleManagerPrivate::initialize não encontrado!\n");
  }

  /* Hook BundleManagerPrivate::update -> true */
  uintptr_t bm_update = so_find_addr("_ZN4core6detail20BundleManagerPrivate6updateERNS_13BundleManager10BundleInfoE");
  if (bm_update) {
    hook_arm64(bm_update, (uintptr_t)hook_bundle_update);
    printf("[main] Hooked BundleManagerPrivate::update -> true\n");
  }

  /* Hook PlayCore C API: AssetPackManager_init -> 0 (success) */
  uintptr_t apm_init = so_find_addr("AssetPackManager_init");
  if (apm_init) {
    hook_arm64(apm_init, (uintptr_t)hook_assetpack_init);
    printf("[main] Hooked AssetPackManager_init -> 0\n");
  }

  /* Hook PlayCore: AssetPackManager_destroy -> noop */
  uintptr_t apm_destroy = so_find_addr("AssetPackManager_destroy");
  if (apm_destroy) {
    hook_arm64(apm_destroy, (uintptr_t)hook_assetpack_destroy);
    printf("[main] Hooked AssetPackManager_destroy -> noop\n");
  }

  /* Hook PlayCore: AssetPackManager_requestInfo -> 0 (success) */
  uintptr_t apm_req = so_find_addr("AssetPackManager_requestInfo");
  if (apm_req) {
    hook_arm64(apm_req, (uintptr_t)hook_assetpack_request_info);
    printf("[main] Hooked AssetPackManager_requestInfo -> 0\n");
  }

  /* Hook PlayCore: AssetPackManager_getAssetPackLocation -> fake local */
  uintptr_t apm_loc = so_find_addr("AssetPackManager_getAssetPackLocation");
  if (apm_loc) {
    hook_arm64(apm_loc, (uintptr_t)hook_assetpack_get_location);
    printf("[main] Hooked AssetPackManager_getAssetPackLocation -> local\n");
  }

  /* Hook PlayCore: AssetPackLocation_getAssetsPath -> assets path local */
  uintptr_t apl_path = so_find_addr("AssetPackLocation_getAssetsPath");
  if (apl_path) {
    hook_arm64(apl_path, (uintptr_t)hook_assetpack_assets_path);
    printf("[main] Hooked AssetPackLocation_getAssetsPath -> %s\n", g_assets_path);
  }

  /* Hook PlayCore: AssetPackLocation_getStorageMethod -> 0 */
  uintptr_t apl_storage = so_find_addr("AssetPackLocation_getStorageMethod");
  if (apl_storage) {
    hook_arm64(apl_storage, (uintptr_t)hook_assetpack_storage_method);
    printf("[main] Hooked AssetPackLocation_getStorageMethod -> 0\n");
  }

  /* Hook PlayCore: AssetPackLocation_destroy -> noop */
  uintptr_t apl_destroy = so_find_addr("AssetPackLocation_destroy");
  if (apl_destroy) {
    hook_arm64(apl_destroy, (uintptr_t)hook_void);
    printf("[main] Hooked AssetPackLocation_destroy -> noop\n");
  }

  /* Hook PlayCore: AssetPackManager_getDownloadState -> NULL */
  uintptr_t apm_ds = so_find_addr("AssetPackManager_getDownloadState");
  if (apm_ds) {
    hook_arm64(apm_ds, (uintptr_t)hook_assetpack_download_state);
    printf("[main] Hooked AssetPackManager_getDownloadState -> NULL\n");
  }

  /* Hooks adicionais de PlayCore para evitar chamadas à Play Store */
  const char *playcore_noop_hooks[] = {
    "AssetPackManager_onResume",
    "AssetPackManager_onPause",
    "AssetPackManager_requestRemoval",
    "AssetPackManager_cancelDownload",
    "AssetPackManager_requestDownload",
    "AssetPackManager_getShowCellularDataConfirmationStatus",
    "AssetPackManager_showCellularDataConfirmation",
  };
  for (size_t i = 0; i < sizeof(playcore_noop_hooks) / sizeof(playcore_noop_hooks[0]); i++) {
    uintptr_t pc_addr = so_find_addr(playcore_noop_hooks[i]);
    if (pc_addr) {
      hook_arm64(pc_addr, (uintptr_t)hook_ret0);
      printf("[main] Hooked %s -> 0\n", playcore_noop_hooks[i]);
    }
  }

  /* Hook BundleManagerPrivate::show_cellular_data_confirmation -> noop */
  uintptr_t bm_cellular = so_find_addr("_ZN4core6detail20BundleManagerPrivate31show_cellular_data_confirmationEv");
  if (bm_cellular) {
    hook_arm64(bm_cellular, (uintptr_t)hook_void);
    printf("[main] Hooked BundleManagerPrivate::show_cellular_data_confirmation -> noop\n");
  }

  /* Hook BundleManager::bundlesAvailable -> retorna true */
  uintptr_t bm_avail = so_find_addr("_ZNK4core13BundleManager16bundlesAvailableEv");
  if (bm_avail) {
    hook_arm64(bm_avail, (uintptr_t)hook_ret1);
    printf("[main] Hooked BundleManager::bundlesAvailable -> true\n");
  }

  printf("[main] === BundleManager/PlayCore bypass completo! ===\n");

  /* 5. Executa construtores (.init_array) */
  printf("[main] Executando construtores (.init_array)...\n");
  so_execute_init_array();
  printf("[main] Construtores (.init_array) finalizados com sucesso!\n");

  /* 6. Restaura permissões do segmento executável */
  so_finalize();
  so_flush_caches();

  /* 7. Localiza o ponto de entrada da ANativeActivity */
  uintptr_t on_create_addr = so_find_addr("ANativeActivity_onCreate");
  if (!on_create_addr) {
    fprintf(stderr, "ERRO CRÍTICO: ANativeActivity_onCreate não encontrado!\n");
    return 1;
  }
  printf("[main] ANativeActivity_onCreate encontrado em %p\n", (void *)on_create_addr);

  /* 8. Inicializa ambiente fake Android e janela SDL2 */
  printf("[main] Inicializando janela SDL2 e subsistema gráfico EGL/GLES2...\n");
  struct android_app *app = android_shim_init();
  if (!app) {
    fprintf(stderr, "ERRO: Falha ao inicializar o android_shim!\n");
    return 1;
  }

  /* 9. Chama ANativeActivity_onCreate */
  printf("[main] Chamando ANativeActivity_onCreate...\n");
  void (*create_func)(ANativeActivity *, void *, size_t) =
      (void (*)(ANativeActivity *, void *, size_t))on_create_addr;
  create_func(app->activity, NULL, 0);

  /* Pega a instância real de android_app criada pela engine */
  struct android_app *real_app = android_shim_get_app();
  void *window = android_shim_get_window();
  printf("[main] Real android_app: %p (msgread=%d, msgwrite=%d, window=%p)\n",
         real_app, real_app->msgread, real_app->msgwrite, window);

  /* 10. Dispara os eventos de ciclo de vida na ordem padrão do Android NativeActivity */
  printf("[main] Notificando inicialização da janela e ciclo de vida...\n");
  if (app->activity->callbacks) {
    if (app->activity->callbacks->onStart) {
      printf("[main] Calling callbacks->onStart...\n");
      app->activity->callbacks->onStart(app->activity);
    }
    if (app->activity->callbacks->onResume) {
      printf("[main] Calling callbacks->onResume...\n");
      app->activity->callbacks->onResume(app->activity);
    }
    if (app->activity->callbacks->onNativeWindowCreated) {
      printf("[main] Calling callbacks->onNativeWindowCreated (window=%p)...\n", window);
      app->activity->callbacks->onNativeWindowCreated(app->activity, window);
    }
    if (app->activity->callbacks->onWindowFocusChanged) {
      printf("[main] Calling callbacks->onWindowFocusChanged...\n");
      app->activity->callbacks->onWindowFocusChanged(app->activity, 1);
    }
  }

  printf("[main] Jogo iniciado com sucesso! Entrando no loop principal...\n");

  /* Loop principal */
  while (!real_app->destroyRequested) {
    android_shim_poll_events();
    usleep(16000); // ~60 FPS
  }

  printf("[main] Finalizando jogo...\n");
  if (app->activity->callbacks && app->activity->callbacks->onDestroy) {
    app->activity->callbacks->onDestroy(app->activity);
  }

  _exit(0);
  return 0;
}
