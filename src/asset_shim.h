#ifndef ASSET_SHIM_H
#define ASSET_SHIM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void asset_shim_init(const char *gamedir);
const char *asset_shim_get_gamedir(void);
const char *asset_shim_get_savedir(void);

/* AAssetManager / AAsset NDK API */
void *AAssetManager_open(void *mgr, const char *filename, int mode);
int AAsset_read(void *asset, void *buf, size_t count);
long AAsset_seek(void *asset, long offset, int whence);
long AAsset_getLength(void *asset);
long AAsset_getRemainingLength(void *asset);
void AAsset_close(void *asset);
const void *AAsset_getBuffer(void *asset);
int AAsset_openFileDescriptor(void *asset, long *outStart, long *outLength);

/* AConfiguration NDK API */
void *AConfiguration_new(void);
void AConfiguration_delete(void *config);
void AConfiguration_fromAssetManager(void *out, void *am);
void AConfiguration_getCountry(void *config, char *outCountry);
void AConfiguration_getLanguage(void *config, char *outLanguage);
int32_t AConfiguration_getDensity(void *config);
int32_t AConfiguration_getScreenSize(void *config);
int32_t AConfiguration_getScreenLong(void *config);

/* ANativeWindow helpers */
int ANativeWindow_setBuffersGeometry(void *window, int width, int height, int format);

#ifdef __cplusplus
}
#endif

#endif /* ASSET_SHIM_H */
