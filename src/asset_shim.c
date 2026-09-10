#include "asset_shim.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char g_gamedir[PATH_MAX];
static char g_assets[PATH_MAX];
static char g_savedir[PATH_MAX];

typedef struct {
  FILE *f;
  long length;
} AsAsset;

static void mkpath(const char *p) {
  char tmp[PATH_MAX];
  snprintf(tmp, sizeof(tmp), "%s", p);
  for (char *s = tmp + 1; *s; s++) {
    if (*s != '/') continue;
    *s = '\0';
    mkdir(tmp, 0755);
    *s = '/';
  }
  mkdir(tmp, 0755);
}

void asset_shim_init(const char *gamedir) {
  if (gamedir && strlen(gamedir) > 0) {
    snprintf(g_gamedir, sizeof(g_gamedir), "%s", gamedir);
  } else {
    strcpy(g_gamedir, ".");
  }
  snprintf(g_assets, sizeof(g_assets), "%s/assets", g_gamedir);
  snprintf(g_savedir, sizeof(g_savedir), "%s/savedata", g_gamedir);
  mkpath(g_assets);
  mkpath(g_savedir);
  printf("[asset] asset_shim_init: gamedir=%s assets=%s savedir=%s\n", g_gamedir, g_assets, g_savedir);
}

const char *asset_shim_get_gamedir(void) { return g_gamedir; }
const char *asset_shim_get_savedir(void) { return g_savedir; }

void *AAssetManager_open(void *mgr, const char *filename, int mode) {
  (void)mgr; (void)mode;
  if (!filename) return NULL;
  char path[PATH_MAX];

  // 1. gamedir/assets/<filename>
  snprintf(path, sizeof(path), "%s/%s", g_assets, filename);
  FILE *f = fopen(path, "rb");

  // 2. gamedir/savedata/<filename>
  if (!f) {
    snprintf(path, sizeof(path), "%s/%s", g_savedir, filename);
    f = fopen(path, "rb");
  }

  // 3. gamedir/<filename>
  if (!f) {
    snprintf(path, sizeof(path), "%s/%s", g_gamedir, filename);
    f = fopen(path, "rb");
  }

  // 4. Strip path if needed (e.g. assets/foo -> foo)
  if (!f && strncmp(filename, "assets/", 7) == 0) {
    snprintf(path, sizeof(path), "%s/%s", g_assets, filename + 7);
    f = fopen(path, "rb");
  }

  if (!f) {
    // printf("[asset] AAssetManager_open NOT FOUND: %s\n", filename);
    return NULL;
  }

  AsAsset *a = (AsAsset *)calloc(1, sizeof(AsAsset));
  if (!a) {
    fclose(f);
    return NULL;
  }
  a->f = f;
  fseek(f, 0, SEEK_END);
  a->length = ftell(f);
  fseek(f, 0, SEEK_SET);
  return a;
}

int AAsset_read(void *asset, void *buf, size_t count) {
  AsAsset *a = (AsAsset *)asset;
  if (!a || !a->f) return -1;
  return (int)fread(buf, 1, count, a->f);
}

long AAsset_seek(void *asset, long offset, int whence) {
  AsAsset *a = (AsAsset *)asset;
  if (!a || !a->f) return -1;
  if (fseek(a->f, offset, whence) != 0) return -1;
  return ftell(a->f);
}

long AAsset_getLength(void *asset) {
  AsAsset *a = (AsAsset *)asset;
  return a ? a->length : 0;
}

long AAsset_getRemainingLength(void *asset) {
  AsAsset *a = (AsAsset *)asset;
  if (!a || !a->f) return 0;
  return a->length - ftell(a->f);
}

void AAsset_close(void *asset) {
  AsAsset *a = (AsAsset *)asset;
  if (!a) return;
  if (a->f) fclose(a->f);
  free(a);
}

const void *AAsset_getBuffer(void *asset) {
  AsAsset *a = (AsAsset *)asset;
  if (!a || !a->f) return NULL;
  long pos = ftell(a->f);
  void *mem = malloc((size_t)a->length + 1);
  if (!mem) return NULL;
  fseek(a->f, 0, SEEK_SET);
  size_t rd = fread(mem, 1, (size_t)a->length, a->f);
  ((char *)mem)[rd] = 0;
  fseek(a->f, pos, SEEK_SET);
  return mem;
}

int AAsset_openFileDescriptor(void *asset, long *outStart, long *outLength) {
  AsAsset *a = (AsAsset *)asset;
  if (!a || !a->f) return -1;
  if (outStart) *outStart = 0;
  if (outLength) *outLength = a->length;
  return dup(fileno(a->f));
}

/* ---------------- AConfiguration ---------------- */

typedef struct {
  char country[3];
  char language[3];
  int density;
  int screenSize;
  int screenLong;
} FakeConfig;

void *AConfiguration_new(void) {
  FakeConfig *c = (FakeConfig *)calloc(1, sizeof(FakeConfig));
  strcpy(c->country, "US");
  strcpy(c->language, "en");
  c->density = 160;     // ACONFIGURATION_DENSITY_MEDIUM
  c->screenSize = 2;    // ACONFIGURATION_SCREENSIZE_NORMAL
  c->screenLong = 1;    // ACONFIGURATION_SCREENLONG_NO
  return c;
}

void AConfiguration_delete(void *config) {
  free(config);
}

void AConfiguration_fromAssetManager(void *out, void *am) {
  (void)am;
  if (!out) return;
  FakeConfig *c = (FakeConfig *)out;
  strcpy(c->country, "US");
  strcpy(c->language, "en");
  c->density = 160;
  c->screenSize = 2;
  c->screenLong = 1;
}

void AConfiguration_getCountry(void *config, char *outCountry) {
  if (outCountry) {
    FakeConfig *c = (FakeConfig *)config;
    strcpy(outCountry, c ? c->country : "US");
  }
}

void AConfiguration_getLanguage(void *config, char *outLanguage) {
  if (outLanguage) {
    FakeConfig *c = (FakeConfig *)config;
    strcpy(outLanguage, c ? c->language : "en");
  }
}

int32_t AConfiguration_getDensity(void *config) {
  FakeConfig *c = (FakeConfig *)config;
  return c ? c->density : 160;
}

int32_t AConfiguration_getScreenSize(void *config) {
  FakeConfig *c = (FakeConfig *)config;
  return c ? c->screenSize : 2;
}

int32_t AConfiguration_getScreenLong(void *config) {
  FakeConfig *c = (FakeConfig *)config;
  return c ? c->screenLong : 1;
}

int ANativeWindow_setBuffersGeometry(void *window, int width, int height, int format) {
  (void)window; (void)width; (void)height; (void)format;
  return 0;
}
