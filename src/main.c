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
#include <unistd.h>

#include "android_shim.h"
#include "asset_shim.h"
#include "bionic_shims.h"
#include "egl_shim.h"
#include "imports.h"
#include "jni_shim.h"
#include "opensles_shim.h"
#include "so_util.h"

#define SO_NAME "lib/arm64-v8a/libalien_shooter.so"

/* DRM / License bypass hooks */
static int hook_is_legal(void) {
  return 1;
}

static int hook_check_certificate(void) {
  return 1;
}

static int hook_ad_init(void) {
  return 0;
}

int main(int argc, char *argv[]) {
  char gamedir[PATH_MAX];
  if (argc > 1 && argv[1]) {
    strncpy(gamedir, argv[1], sizeof(gamedir) - 1);
  } else {
    // Current working directory
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
  asset_shim_init(gamedir);
  jni_shim_init(gamedir);

  /* Verifica se a biblioteca existe */
  char so_path[PATH_MAX];
  snprintf(so_path, sizeof(so_path), "%s/%s", gamedir, SO_NAME);
  if (access(so_path, F_OK) != 0) {
    // Tenta relativo direto
    snprintf(so_path, sizeof(so_path), "%s", SO_NAME);
    if (access(so_path, F_OK) != 0) {
      fprintf(stderr, "ERRO: %s não encontrado em %s!\n", SO_NAME, gamedir);
      return 1;
    }
  }

  /* 1. Carrega o ELF da engine */
  if (so_load(so_path, NULL, 0) != 0) {
    fprintf(stderr, "ERRO: Falha ao carregar %s!\n", so_path);
    return 1;
  }

  /* 2. Aplica relocações internas (R_AARCH64_RELATIVE) */
  so_relocate();

  /* 3. Resolve importações com as funções da tabela */
  so_resolve(dynlib_functions, dynlib_num_functions, 1);

  /* 4. Trava de Licença e Certificado: Força retorno 1 (Legal / Full Game) */
  so_make_text_writable();

  uintptr_t is_legal_app = so_find_addr("_ZNK4core11Application7isLegalEv");
  if (is_legal_app) {
    hook_arm64(is_legal_app, (uintptr_t)hook_is_legal);
    printf("[main] Hooked Application::isLegal -> 1\n");
  }

  uintptr_t is_legal_native = so_find_addr("_ZNK7android17ApplicationNative7isLegalEv");
  if (is_legal_native) {
    hook_arm64(is_legal_native, (uintptr_t)hook_is_legal);
    printf("[main] Hooked ApplicationNative::isLegal -> 1\n");
  }

  uintptr_t cert_app = so_find_addr("_ZN4core11Application23checkPackageCertificateEv");
  if (cert_app) {
    hook_arm64(cert_app, (uintptr_t)hook_check_certificate);
    printf("[main] Hooked Application::checkPackageCertificate -> 1\n");
  }

  uintptr_t cert_native = so_find_addr("_ZN7android17ApplicationNative23checkPackageCertificateEv");
  if (cert_native) {
    hook_arm64(cert_native, (uintptr_t)hook_check_certificate);
    printf("[main] Hooked ApplicationNative::checkPackageCertificate -> 1\n");
  }

  uintptr_t ad_init = so_find_addr("_ZN2ad21AdvertisementProvider10initializeEv");
  if (ad_init) {
    hook_arm64(ad_init, (uintptr_t)hook_ad_init);
    printf("[main] Hooked AdvertisementProvider::initialize -> 0\n");
  }

  /* 5. Executa construtores (.init_array) */
  printf("[main] Executando construtores (.init_array)...\n");
  so_execute_init_array();

  /* 6. Restaura permissões do segmento executável */
  so_finalize();

  /* 7. Localiza o ponto de entrada da ANativeActivity */
  uintptr_t on_create_addr = so_find_addr("ANativeActivity_onCreate");
  if (!on_create_addr) {
    fprintf(stderr, "ERRO CRÍTICO: ANativeActivity_onCreate não encontrado!\n");
    return 1;
  }
  printf("[main] ANativeActivity_onCreate encontrado em %p\n", (void *)on_create_addr);

  /* 8. Inicializa ambiente fake Android e janela SDL2 */
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

  /* 10. Dispara os eventos de ciclo de vida */
  printf("[main] Notificando inicialização da janela e ciclo de vida...\n");
  android_shim_send_cmd(APP_CMD_INIT_WINDOW);
  android_shim_send_cmd(APP_CMD_START);
  android_shim_send_cmd(APP_CMD_RESUME);
  android_shim_send_cmd(APP_CMD_GAINED_FOCUS);

  if (app->activity->callbacks) {
    if (app->activity->callbacks->onStart)
      app->activity->callbacks->onStart(app->activity);
    if (app->activity->callbacks->onResume)
      app->activity->callbacks->onResume(app->activity);
    if (app->activity->callbacks->onNativeWindowCreated)
      app->activity->callbacks->onNativeWindowCreated(app->activity, app->window);
    if (app->activity->callbacks->onWindowFocusChanged)
      app->activity->callbacks->onWindowFocusChanged(app->activity, 1);
  }

  printf("[main] Jogo iniciado com sucesso! Entrando no loop principal...\n");

  /* Loop principal */
  while (!app->destroyRequested) {
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
