#include "opensles_shim.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PLAYERS 16
#define RING_BUFFER_SIZE (512 * 1024)
#define RING_BUFFER_MASK (RING_BUFFER_SIZE - 1)
#define SDL_AUDIO_CHUNK 2048

static const int id_engine_tag = 1;
static const int id_play_tag = 2;
static const int id_volume_tag = 3;
static const int id_bufferqueue_tag = 4;
static const int id_seek_tag = 5;

const SLInterfaceID sl_IID_ENGINE = &id_engine_tag;
const SLInterfaceID sl_IID_PLAY = &id_play_tag;
const SLInterfaceID sl_IID_VOLUME = &id_volume_tag;
const SLInterfaceID sl_IID_BUFFERQUEUE = &id_bufferqueue_tag;
const SLInterfaceID sl_IID_SEEK = &id_seek_tag;

typedef struct {
  SLuint32 locatorType;
  SLuint32 numBuffers;
} SLDataLocator_BufferQueue;

typedef struct {
  SLuint32 formatType;
  SLuint32 numChannels;
  SLuint32 samplesPerSec;
  SLuint32 bitsPerSample;
  SLuint32 containerSize;
  SLuint32 channelMask;
  SLuint32 endianness;
} SLDataFormat_PCM;

typedef struct {
  void *pLocator;
  void *pFormat;
} SLDataSource;

typedef struct {
  void *pLocator;
  void *pFormat;
} SLDataSink;

typedef void (*slBufferQueueCallback)(void *caller, void *pContext);

/* Forward declarations */
struct SLBufferQueueItf_;
struct SLPlayItf_;
struct SLVolumeItf_;
struct SLSeekItf_;
struct SLObjectItf_;

typedef struct PlayerState {
  uint8_t ring[RING_BUFFER_SIZE];
  volatile uint32_t ring_head;
  volatile uint32_t ring_tail;

  slBufferQueueCallback bq_callback;
  void *bq_context;

  SLuint32 play_state;
  SLmillibel volume_level;
  SLBoolean is_muted;

  int in_use;

  const struct SLObjectItf_ *object_itf;
  const struct SLPlayItf_ *play_itf;
  const struct SLBufferQueueItf_ *bq_itf;
  const struct SLVolumeItf_ *volume_itf;
  const struct SLSeekItf_ *seek_itf;
} PlayerState;

static PlayerState g_players[MAX_PLAYERS];
static SDL_AudioDeviceID g_audio_device = 0;
static pthread_mutex_t g_audio_lock = PTHREAD_MUTEX_INITIALIZER;

static void sdl_audio_callback(void *userdata, Uint8 *stream, int len) {
  (void)userdata;
  memset(stream, 0, len);

  int16_t *out_samples = (int16_t *)stream;
  int num_samples = len / sizeof(int16_t);

  int32_t mix_buf[SDL_AUDIO_CHUNK * 2];
  if (num_samples > (int)(sizeof(mix_buf) / sizeof(mix_buf[0]))) {
    num_samples = sizeof(mix_buf) / sizeof(mix_buf[0]);
  }
  memset(mix_buf, 0, num_samples * sizeof(int32_t));

  pthread_mutex_lock(&g_audio_lock);
  int active = 0;

  for (int p = 0; p < MAX_PLAYERS; p++) {
    PlayerState *ps = &g_players[p];
    if (!ps->in_use || ps->play_state != SL_PLAYSTATE_PLAYING || ps->is_muted)
      continue;

    uint32_t head = ps->ring_head;
    uint32_t tail = ps->ring_tail;
    uint32_t avail_bytes = (head - tail) & RING_BUFFER_MASK;
    int avail_samples = avail_bytes / sizeof(int16_t);

    int to_read = (num_samples < avail_samples) ? num_samples : avail_samples;
    if (to_read <= 0) continue;

    active++;
    float vol = powf(10.0f, (float)ps->volume_level / 2000.0f);
    if (vol > 1.0f) vol = 1.0f;
    if (vol < 0.0f) vol = 0.0f;

    for (int i = 0; i < to_read; i++) {
      uint32_t offset = (tail + i * sizeof(int16_t)) & RING_BUFFER_MASK;
      int16_t s = *(int16_t *)&ps->ring[offset];
      mix_buf[i] += (int32_t)(s * vol);
    }
    ps->ring_tail = (tail + to_read * sizeof(int16_t)) & RING_BUFFER_MASK;
  }
  pthread_mutex_unlock(&g_audio_lock);

  if (active > 0) {
    for (int i = 0; i < num_samples; i++) {
      int32_t val = mix_buf[i];
      if (val > 32767) val = 32767;
      if (val < -32768) val = -32768;
      out_samples[i] = (int16_t)val;
    }
  }
}

void opensles_shim_init(void) {
  if (g_audio_device != 0) return;

  SDL_AudioSpec wanted, obtained;
  memset(&wanted, 0, sizeof(wanted));
  wanted.freq = 44100;
  wanted.format = AUDIO_S16SYS;
  wanted.channels = 2;
  wanted.samples = 1024;
  wanted.callback = sdl_audio_callback;

  g_audio_device = SDL_OpenAudioDevice(NULL, 0, &wanted, &obtained, 0);
  if (g_audio_device > 0) {
    SDL_PauseAudioDevice(g_audio_device, 0);
    printf("[opensles] SDL Audio device opened: %d Hz, %d channels\n", obtained.freq, obtained.channels);
  } else {
    printf("[opensles] Warning: SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
  }
}

void opensles_shim_pump_callbacks(void) {
  for (int p = 0; p < MAX_PLAYERS; p++) {
    PlayerState *ps = &g_players[p];
    if (ps->in_use && ps->bq_callback) {
      uint32_t head = ps->ring_head;
      uint32_t tail = ps->ring_tail;
      uint32_t used = (head - tail) & RING_BUFFER_MASK;
      // Trigger callback if buffer is less than half full
      if (used < (RING_BUFFER_SIZE / 2)) {
        ps->bq_callback((void *)&ps->bq_itf, ps->bq_context);
      }
    }
  }
}

/* ---------------- BufferQueue Interface ---------------- */
typedef struct SLBufferQueueItf_ {
  SLresult (*RegisterCallback)(void *self, slBufferQueueCallback callback, void *pContext);
  SLresult (*Clear)(void *self);
  SLresult (*Enqueue)(void *self, const void *pBuffer, SLuint32 size);
  SLresult (*GetState)(void *self, void *pState);
} SLBufferQueueItf_;

static PlayerState *get_player_from_itf(void *itf) {
  return (PlayerState *)((uintptr_t)itf - offsetof(PlayerState, bq_itf));
}

static SLresult bq_RegisterCallback(void *self, slBufferQueueCallback callback, void *pContext) {
  PlayerState *ps = get_player_from_itf(self);
  ps->bq_callback = callback;
  ps->bq_context = pContext;
  return SL_RESULT_SUCCESS;
}

static SLresult bq_Clear(void *self) {
  PlayerState *ps = get_player_from_itf(self);
  pthread_mutex_lock(&g_audio_lock);
  ps->ring_head = ps->ring_tail = 0;
  pthread_mutex_unlock(&g_audio_lock);
  return SL_RESULT_SUCCESS;
}

static SLresult bq_Enqueue(void *self, const void *pBuffer, SLuint32 size) {
  if (!pBuffer || size == 0) return SL_RESULT_SUCCESS;
  PlayerState *ps = get_player_from_itf(self);

  pthread_mutex_lock(&g_audio_lock);
  uint32_t head = ps->ring_head;
  uint32_t tail = ps->ring_tail;
  uint32_t free_bytes = (tail - head - 1) & RING_BUFFER_MASK;

  uint32_t to_write = (size < free_bytes) ? size : free_bytes;
  const uint8_t *src = (const uint8_t *)pBuffer;

  for (uint32_t i = 0; i < to_write; i++) {
    ps->ring[(head + i) & RING_BUFFER_MASK] = src[i];
  }
  ps->ring_head = (head + to_write) & RING_BUFFER_MASK;
  pthread_mutex_unlock(&g_audio_lock);

  return SL_RESULT_SUCCESS;
}

typedef struct {
  SLuint32 count;
  SLuint32 playIndex;
} SLBufferQueueState;

static SLresult bq_GetState(void *self, void *pState) {
  if (!pState) return SL_RESULT_PARAMETER_INVALID;
  PlayerState *ps = get_player_from_itf(self);
  SLBufferQueueState *state = (SLBufferQueueState *)pState;
  uint32_t used = (ps->ring_head - ps->ring_tail) & RING_BUFFER_MASK;
  state->count = (used > 0) ? 1 : 0;
  state->playIndex = 0;
  return SL_RESULT_SUCCESS;
}

static const SLBufferQueueItf_ g_bq_itf_impl = {
  bq_RegisterCallback,
  bq_Clear,
  bq_Enqueue,
  bq_GetState
};

/* ---------------- Play Interface ---------------- */
typedef struct SLPlayItf_ {
  SLresult (*SetPlayState)(void *self, SLuint32 state);
  SLresult (*GetPlayState)(void *self, SLuint32 *pState);
  SLresult (*GetDuration)(void *self, SLmillisecond *pMsec);
  SLresult (*GetPosition)(void *self, SLmillisecond *pMsec);
  SLresult (*RegisterCallback)(void *self, void *callback, void *pContext);
  SLresult (*SetCallbackEventsMask)(void *self, SLuint32 eventFlags);
  SLresult (*GetCallbackEventsMask)(void *self, SLuint32 *pEventFlags);
  SLresult (*SetMarkerPosition)(void *self, SLmillisecond mSec);
  SLresult (*ClearMarkerPosition)(void *self);
  SLresult (*GetMarkerPosition)(void *self, SLmillisecond *pMsec);
  SLresult (*SetPositionUpdatePeriod)(void *self, SLmillisecond mSec);
  SLresult (*GetPositionUpdatePeriod)(void *self, SLmillisecond *pMsec);
} SLPlayItf_;

static PlayerState *get_player_from_play_itf(void *itf) {
  return (PlayerState *)((uintptr_t)itf - offsetof(PlayerState, play_itf));
}

static SLresult play_SetPlayState(void *self, SLuint32 state) {
  PlayerState *ps = get_player_from_play_itf(self);
  ps->play_state = state;
  if (state == SL_PLAYSTATE_PLAYING && ps->bq_callback) {
    ps->bq_callback((void *)&ps->bq_itf, ps->bq_context);
  }
  return SL_RESULT_SUCCESS;
}

static SLresult play_GetPlayState(void *self, SLuint32 *pState) {
  if (pState) {
    PlayerState *ps = get_player_from_play_itf(self);
    *pState = ps->play_state;
  }
  return SL_RESULT_SUCCESS;
}

static SLresult play_GetDuration(void *self, SLmillisecond *pMsec) {
  (void)self;
  if (pMsec) *pMsec = 0xFFFFFFFF;
  return SL_RESULT_SUCCESS;
}

static SLresult play_GetPosition(void *self, SLmillisecond *pMsec) {
  (void)self;
  if (pMsec) *pMsec = 0;
  return SL_RESULT_SUCCESS;
}

static SLresult play_RegisterCallback(void *self, void *callback, void *pContext) {
  (void)self; (void)callback; (void)pContext;
  return SL_RESULT_SUCCESS;
}

static SLresult play_SetCallbackEventsMask(void *self, SLuint32 eventFlags) {
  (void)self; (void)eventFlags;
  return SL_RESULT_SUCCESS;
}

static SLresult play_GetCallbackEventsMask(void *self, SLuint32 *pEventFlags) {
  (void)self;
  if (pEventFlags) *pEventFlags = 0;
  return SL_RESULT_SUCCESS;
}

static SLresult play_SetMarkerPosition(void *self, SLmillisecond mSec) { (void)self; (void)mSec; return SL_RESULT_SUCCESS; }
static SLresult play_ClearMarkerPosition(void *self) { (void)self; return SL_RESULT_SUCCESS; }
static SLresult play_GetMarkerPosition(void *self, SLmillisecond *pMsec) { (void)self; if (pMsec) *pMsec = 0; return SL_RESULT_SUCCESS; }
static SLresult play_SetPositionUpdatePeriod(void *self, SLmillisecond mSec) { (void)self; (void)mSec; return SL_RESULT_SUCCESS; }
static SLresult play_GetPositionUpdatePeriod(void *self, SLmillisecond *pMsec) { (void)self; if (pMsec) *pMsec = 0; return SL_RESULT_SUCCESS; }

static const SLPlayItf_ g_play_itf_impl = {
  play_SetPlayState, play_GetPlayState, play_GetDuration, play_GetPosition,
  play_RegisterCallback, play_SetCallbackEventsMask, play_GetCallbackEventsMask,
  play_SetMarkerPosition, play_ClearMarkerPosition, play_GetMarkerPosition,
  play_SetPositionUpdatePeriod, play_GetPositionUpdatePeriod
};

/* ---------------- Volume Interface ---------------- */
typedef struct SLVolumeItf_ {
  SLresult (*SetVolumeLevel)(void *self, SLmillibel level);
  SLresult (*GetVolumeLevel)(void *self, SLmillibel *pLevel);
  SLresult (*GetMaxVolumeLevel)(void *self, SLmillibel *pMaxLevel);
  SLresult (*SetMute)(void *self, SLBoolean mute);
  SLresult (*GetMute)(void *self, SLBoolean *pMute);
  SLresult (*EnableStereoPosition)(void *self, SLBoolean enable);
  SLresult (*IsEnabledStereoPosition)(void *self, SLBoolean *pEnable);
  SLresult (*SetStereoPosition)(void *self, SLmillibel position);
  SLresult (*GetStereoPosition)(void *self, SLmillibel *pPosition);
} SLVolumeItf_;

static PlayerState *get_player_from_vol_itf(void *itf) {
  return (PlayerState *)((uintptr_t)itf - offsetof(PlayerState, volume_itf));
}

static SLresult vol_SetVolumeLevel(void *self, SLmillibel level) {
  PlayerState *ps = get_player_from_vol_itf(self);
  ps->volume_level = level;
  return SL_RESULT_SUCCESS;
}

static SLresult vol_GetVolumeLevel(void *self, SLmillibel *pLevel) {
  PlayerState *ps = get_player_from_vol_itf(self);
  if (pLevel) *pLevel = ps->volume_level;
  return SL_RESULT_SUCCESS;
}

static SLresult vol_GetMaxVolumeLevel(void *self, SLmillibel *pMaxLevel) {
  (void)self;
  if (pMaxLevel) *pMaxLevel = 0;
  return SL_RESULT_SUCCESS;
}

static SLresult vol_SetMute(void *self, SLBoolean mute) {
  PlayerState *ps = get_player_from_vol_itf(self);
  ps->is_muted = mute;
  return SL_RESULT_SUCCESS;
}

static SLresult vol_GetMute(void *self, SLBoolean *pMute) {
  PlayerState *ps = get_player_from_vol_itf(self);
  if (pMute) *pMute = ps->is_muted;
  return SL_RESULT_SUCCESS;
}

static SLresult vol_EnableStereoPosition(void *self, SLBoolean enable) { (void)self; (void)enable; return SL_RESULT_SUCCESS; }
static SLresult vol_IsEnabledStereoPosition(void *self, SLBoolean *pEnable) { (void)self; if (pEnable) *pEnable = SL_BOOLEAN_FALSE; return SL_RESULT_SUCCESS; }
static SLresult vol_SetStereoPosition(void *self, SLmillibel position) { (void)self; (void)position; return SL_RESULT_SUCCESS; }
static SLresult vol_GetStereoPosition(void *self, SLmillibel *pPosition) { (void)self; if (pPosition) *pPosition = 0; return SL_RESULT_SUCCESS; }

static const SLVolumeItf_ g_vol_itf_impl = {
  vol_SetVolumeLevel, vol_GetVolumeLevel, vol_GetMaxVolumeLevel,
  vol_SetMute, vol_GetMute, vol_EnableStereoPosition, vol_IsEnabledStereoPosition,
  vol_SetStereoPosition, vol_GetStereoPosition
};

/* ---------------- Seek Interface ---------------- */
typedef struct SLSeekItf_ {
  SLresult (*SetPosition)(void *self, SLmillisecond msec, SLuint32 seekMode);
  SLresult (*SetLoop)(void *self, SLBoolean enable, SLmillisecond startPos, SLmillisecond endPos);
  SLresult (*GetLoop)(void *self, SLBoolean *pEnable, SLmillisecond *pStartPos, SLmillisecond *pEndPos);
} SLSeekItf_;

static SLresult seek_SetPosition(void *self, SLmillisecond msec, SLuint32 seekMode) {
  (void)self; (void)msec; (void)seekMode;
  return SL_RESULT_SUCCESS;
}
static SLresult seek_SetLoop(void *self, SLBoolean enable, SLmillisecond startPos, SLmillisecond endPos) {
  (void)self; (void)enable; (void)startPos; (void)endPos;
  return SL_RESULT_SUCCESS;
}
static SLresult seek_GetLoop(void *self, SLBoolean *pEnable, SLmillisecond *pStartPos, SLmillisecond *pEndPos) {
  (void)self;
  if (pEnable) *pEnable = SL_BOOLEAN_FALSE;
  if (pStartPos) *pStartPos = 0;
  if (pEndPos) *pEndPos = 0;
  return SL_RESULT_SUCCESS;
}

static const SLSeekItf_ g_seek_itf_impl = {
  seek_SetPosition, seek_SetLoop, seek_GetLoop
};

/* ---------------- Object Interface ---------------- */
typedef struct SLObjectItf_ {
  SLresult (*Realize)(void *self, SLBoolean async);
  SLresult (*Resume)(void *self, SLBoolean async);
  SLresult (*GetState)(void *self, SLuint32 *pState);
  SLresult (*GetInterface)(void *self, const SLInterfaceID iid, void *pInterface);
  SLresult (*RegisterCallback)(void *self, void *callback, void *pContext);
  void (*AbortAsyncOperation)(void *self);
  void (*Destroy)(void *self);
  SLresult (*SetPriority)(void *self, SLint32 priority, SLBoolean preemptable);
  SLresult (*GetPriority)(void *self, SLint32 *pPriority, SLBoolean *pPreemptable);
  SLresult (*SetLossOfControlNotification)(void *self, SLBoolean enable);
} SLObjectItf_;

static PlayerState *get_player_from_obj_itf(void *itf) {
  return (PlayerState *)((uintptr_t)itf - offsetof(PlayerState, object_itf));
}

static SLresult obj_Realize(void *self, SLBoolean async) { (void)self; (void)async; return SL_RESULT_SUCCESS; }
static SLresult obj_Resume(void *self, SLBoolean async) { (void)self; (void)async; return SL_RESULT_SUCCESS; }
static SLresult obj_GetState(void *self, SLuint32 *pState) { (void)self; if (pState) *pState = 2; return SL_RESULT_SUCCESS; }

static SLresult obj_GetInterface(void *self, const SLInterfaceID iid, void *pInterface) {
  if (!pInterface) return SL_RESULT_PARAMETER_INVALID;
  PlayerState *ps = get_player_from_obj_itf(self);

  if (iid == sl_IID_PLAY || *(const int *)iid == id_play_tag) {
    *(const SLPlayItf_ **)pInterface = &g_play_itf_impl;
    return SL_RESULT_SUCCESS;
  }
  if (iid == sl_IID_BUFFERQUEUE || *(const int *)iid == id_bufferqueue_tag) {
    *(const SLBufferQueueItf_ **)pInterface = &g_bq_itf_impl;
    return SL_RESULT_SUCCESS;
  }
  if (iid == sl_IID_VOLUME || *(const int *)iid == id_volume_tag) {
    *(const SLVolumeItf_ **)pInterface = &g_vol_itf_impl;
    return SL_RESULT_SUCCESS;
  }
  if (iid == sl_IID_SEEK || *(const int *)iid == id_seek_tag) {
    *(const SLSeekItf_ **)pInterface = &g_seek_itf_impl;
    return SL_RESULT_SUCCESS;
  }

  *(void **)pInterface = NULL;
  return SL_RESULT_RESOURCE_ERROR;
}

static SLresult obj_RegisterCallback(void *self, void *callback, void *pContext) { (void)self; (void)callback; (void)pContext; return SL_RESULT_SUCCESS; }
static void obj_AbortAsyncOperation(void *self) { (void)self; }

static void obj_Destroy(void *self) {
  PlayerState *ps = get_player_from_obj_itf(self);
  pthread_mutex_lock(&g_audio_lock);
  ps->in_use = 0;
  ps->play_state = SL_PLAYSTATE_STOPPED;
  ps->ring_head = ps->ring_tail = 0;
  ps->bq_callback = NULL;
  pthread_mutex_unlock(&g_audio_lock);
}

static SLresult obj_SetPriority(void *self, SLint32 priority, SLBoolean preemptable) { (void)self; (void)priority; (void)preemptable; return SL_RESULT_SUCCESS; }
static SLresult obj_GetPriority(void *self, SLint32 *pPriority, SLBoolean *pPreemptable) { (void)self; if (pPriority) *pPriority = 0; if (pPreemptable) *pPreemptable = SL_BOOLEAN_FALSE; return SL_RESULT_SUCCESS; }
static SLresult obj_SetLossOfControlNotification(void *self, SLBoolean enable) { (void)self; (void)enable; return SL_RESULT_SUCCESS; }

static const SLObjectItf_ g_obj_itf_impl = {
  obj_Realize, obj_Resume, obj_GetState, obj_GetInterface,
  obj_RegisterCallback, obj_AbortAsyncOperation, obj_Destroy,
  obj_SetPriority, obj_GetPriority, obj_SetLossOfControlNotification
};

/* ---------------- OutputMix Object ---------------- */
static void outputmix_Destroy(void *self) { (void)self; }
static const SLObjectItf_ g_outputmix_itf = {
  obj_Realize, obj_Resume, obj_GetState, obj_GetInterface,
  obj_RegisterCallback, obj_AbortAsyncOperation, outputmix_Destroy,
  obj_SetPriority, obj_GetPriority, obj_SetLossOfControlNotification
};
static const SLObjectItf_ *g_outputmix_ptr = &g_outputmix_itf;

/* ---------------- Engine Interface ---------------- */
typedef struct SLEngineItf_ {
  SLresult (*CreateLEDDevice)(void *self, void **pDevice, SLuint32 deviceID, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*CreateVibraDevice)(void *self, void **pDevice, SLuint32 deviceID, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*CreateAudioPlayer)(void *self, void **pPlayer, SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*CreateAudioRecorder)(void *self, void **pRecorder, SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*CreateMidiPlayer)(void *self, void **pPlayer, SLDataSource *pMidiSrc, SLDataSource *pBankSrc, SLDataSink *pAudioOutput, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*CreateListener)(void *self, void **pListener, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*Create3DGroup)(void *self, void **pGroup, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*CreateOutputMix)(void *self, void **pMix, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*CreateMetadataExtractor)(void *self, void **pMetadataExtractor, SLDataSource *pDataSource, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*CreateExtensionObject)(void *self, void **pObject, void *pParameters, SLuint32 objectID, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired);
  SLresult (*QueryNumSupportedInterfaces)(void *self, SLuint32 objectID, SLuint32 *pNumSupportedInterfaces);
  SLresult (*QuerySupportedInterfaces)(void *self, SLuint32 objectID, SLuint32 index, SLInterfaceID *pInterfaceId);
  SLresult (*QueryNumSupportedExtensions)(void *self, SLuint32 *pNumExtensions);
  SLresult (*QuerySupportedExtension)(void *self, SLuint32 index, SLchar *pExtensionName, SLint16 *pNameLength);
  SLresult (*IsExtensionSupported)(void *self, const SLchar *pExtensionName, SLBoolean *pSupported);
} SLEngineItf_;

static SLresult engine_CreateAudioPlayer(void *self, void **pPlayer,
                                        SLDataSource *pAudioSrc, SLDataSink *pAudioSnk,
                                        SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
                                        const SLBoolean *pInterfaceRequired) {
  (void)self; (void)pAudioSrc; (void)pAudioSnk; (void)numInterfaces; (void)pInterfaceIds; (void)pInterfaceRequired;
  opensles_shim_init();

  pthread_mutex_lock(&g_audio_lock);
  PlayerState *ps = NULL;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (!g_players[i].in_use) {
      ps = &g_players[i];
      memset(ps, 0, sizeof(PlayerState));
      ps->in_use = 1;
      ps->play_state = SL_PLAYSTATE_STOPPED;
      ps->object_itf = &g_obj_itf_impl;
      ps->play_itf = &g_play_itf_impl;
      ps->bq_itf = &g_bq_itf_impl;
      ps->volume_itf = &g_vol_itf_impl;
      ps->seek_itf = &g_seek_itf_impl;
      break;
    }
  }
  pthread_mutex_unlock(&g_audio_lock);

  if (!ps) {
    printf("[opensles] Out of player slots!\n");
    return SL_RESULT_MEMORY_FAILURE;
  }

  *pPlayer = (void *)&ps->object_itf;
  return SL_RESULT_SUCCESS;
}

static SLresult engine_CreateOutputMix(void *self, void **pMix, SLuint32 numInterfaces,
                                       const SLInterfaceID *pInterfaceIds, const SLBoolean *pInterfaceRequired) {
  (void)self; (void)numInterfaces; (void)pInterfaceIds; (void)pInterfaceRequired;
  opensles_shim_init();
  *pMix = (void *)&g_outputmix_ptr;
  return SL_RESULT_SUCCESS;
}

static const SLEngineItf_ g_engine_itf = {
  NULL, NULL,
  engine_CreateAudioPlayer,
  NULL, NULL, NULL, NULL,
  engine_CreateOutputMix,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
static const SLEngineItf_ *g_engine_itf_ptr = &g_engine_itf;

static SLresult engine_obj_GetInterface(void *self, const SLInterfaceID iid, void *pInterface) {
  (void)self;
  if (!pInterface) return SL_RESULT_PARAMETER_INVALID;
  if (iid == sl_IID_ENGINE || *(const int *)iid == id_engine_tag) {
    *(const SLEngineItf_ ***)pInterface = (const SLEngineItf_ **)&g_engine_itf_ptr;
    return SL_RESULT_SUCCESS;
  }
  *(void **)pInterface = NULL;
  return SL_RESULT_RESOURCE_ERROR;
}

static const SLObjectItf_ g_engine_obj_itf = {
  obj_Realize, obj_Resume, obj_GetState, engine_obj_GetInterface,
  obj_RegisterCallback, obj_AbortAsyncOperation, obj_Destroy,
  obj_SetPriority, obj_GetPriority, obj_SetLossOfControlNotification
};
static const SLObjectItf_ *g_engine_obj_ptr = &g_engine_obj_itf;

SLresult slCreateEngine(void **pEngine, SLuint32 numOptions,
                        const void *pEngineOptions,
                        SLuint32 numInterfaces,
                        const SLInterfaceID *pInterfaceIds,
                        const SLBoolean *pInterfaceRequired) {
  (void)numOptions; (void)pEngineOptions; (void)numInterfaces; (void)pInterfaceIds; (void)pInterfaceRequired;
  printf("[opensles] slCreateEngine called\n");
  opensles_shim_init();
  *pEngine = (void *)&g_engine_obj_ptr;
  return SL_RESULT_SUCCESS;
}
