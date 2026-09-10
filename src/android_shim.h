#ifndef ANDROID_SHIM_H
#define ANDROID_SHIM_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <poll.h>
#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct android_app;
typedef struct ALooper ALooper;
typedef struct AInputQueue AInputQueue;
typedef struct AInputEvent AInputEvent;
typedef struct ANativeActivity ANativeActivity;

/* Input event types */
#define AINPUT_EVENT_TYPE_KEY 1
#define AINPUT_EVENT_TYPE_MOTION 2

/* Key actions */
#define AKEY_EVENT_ACTION_DOWN 0
#define AKEY_EVENT_ACTION_UP 1

/* Motion actions */
#define AMOTION_EVENT_ACTION_DOWN 0
#define AMOTION_EVENT_ACTION_UP 1
#define AMOTION_EVENT_ACTION_MOVE 2
#define AMOTION_EVENT_ACTION_CANCEL 3
#define AMOTION_EVENT_ACTION_POINTER_DOWN 5
#define AMOTION_EVENT_ACTION_POINTER_UP 6

/* Keycodes */
#define AKEYCODE_BUTTON_A 96
#define AKEYCODE_BUTTON_B 97
#define AKEYCODE_BUTTON_C 98
#define AKEYCODE_BUTTON_X 99
#define AKEYCODE_BUTTON_Y 100
#define AKEYCODE_BUTTON_Z 101
#define AKEYCODE_BUTTON_L1 102
#define AKEYCODE_BUTTON_R1 103
#define AKEYCODE_BUTTON_L2 104
#define AKEYCODE_BUTTON_R2 105
#define AKEYCODE_BUTTON_THUMBL 106
#define AKEYCODE_BUTTON_THUMBR 107
#define AKEYCODE_BUTTON_START 108
#define AKEYCODE_BUTTON_SELECT 109
#define AKEYCODE_DPAD_UP 19
#define AKEYCODE_DPAD_DOWN 20
#define AKEYCODE_DPAD_LEFT 21
#define AKEYCODE_DPAD_RIGHT 22
#define AKEYCODE_BACK 4

/* Sources */
#define AINPUT_SOURCE_KEYBOARD 0x00000101
#define AINPUT_SOURCE_TOUCHSCREEN 0x00001002
#define AINPUT_SOURCE_JOYSTICK 0x01000010
#define AINPUT_SOURCE_GAMEPAD 0x00000401

/* Lifecycle Commands */
enum {
  APP_CMD_INPUT_CHANGED = 0,
  APP_CMD_INIT_WINDOW = 1,
  APP_CMD_TERM_WINDOW = 2,
  APP_CMD_WINDOW_RESIZED = 3,
  APP_CMD_WINDOW_REDRAW_NEEDED = 4,
  APP_CMD_CONTENT_RECT_CHANGED = 5,
  APP_CMD_GAINED_FOCUS = 6,
  APP_CMD_LOST_FOCUS = 7,
  APP_CMD_CONFIG_CHANGED = 8,
  APP_CMD_LOW_MEMORY = 9,
  APP_CMD_START = 10,
  APP_CMD_RESUME = 11,
  APP_CMD_SAVE_STATE = 12,
  APP_CMD_PAUSE = 13,
  APP_CMD_STOP = 14,
  APP_CMD_DESTROY = 15,
};

#define LOOPER_ID_MAIN 1
#define LOOPER_ID_INPUT 2

typedef struct ANativeActivityCallbacks {
  void (*onStart)(ANativeActivity *activity);
  void (*onResume)(ANativeActivity *activity);
  void *(*onSaveInstanceState)(ANativeActivity *activity, size_t *outSize);
  void (*onPause)(ANativeActivity *activity);
  void (*onStop)(ANativeActivity *activity);
  void (*onDestroy)(ANativeActivity *activity);
  void (*onWindowFocusChanged)(ANativeActivity *activity, int hasFocus);
  void (*onNativeWindowCreated)(ANativeActivity *activity, void *window);
  void (*onNativeWindowResized)(ANativeActivity *activity, void *window);
  void (*onNativeWindowRedrawNeeded)(ANativeActivity *activity, void *window);
  void (*onNativeWindowDestroyed)(ANativeActivity *activity, void *window);
  void (*onInputQueueCreated)(ANativeActivity *activity, AInputQueue *queue);
  void (*onInputQueueDestroyed)(ANativeActivity *activity, AInputQueue *queue);
  void (*onContentRectChanged)(ANativeActivity *activity, const void *rect);
  void (*onConfigurationChanged)(ANativeActivity *activity);
  void (*onLowMemory)(ANativeActivity *activity);
} ANativeActivityCallbacks;

typedef struct ANativeActivity {
  ANativeActivityCallbacks *callbacks;
  void *vm;
  void *env;
  void *clazz;
  const char *internalDataPath;
  const char *externalDataPath;
  int32_t sdkVersion;
  void *instance;
  void *assetManager;
  const char *obbPath;
} ANativeActivity;

struct android_poll_source {
  int32_t id;
  struct android_app *app;
  void (*process)(struct android_app *app, struct android_poll_source *source);
};

typedef struct android_app {
  void *userData;
  void (*onAppCmd)(struct android_app *app, int32_t cmd);
  int32_t (*onInputEvent)(struct android_app *app, AInputEvent *event);
  ANativeActivity *activity;
  void *config;
  void *savedState;
  size_t savedStateSize;
  ALooper *looper;
  AInputQueue *inputQueue;
  void *window;

  int activityState;
  int destroyRequested;

  pthread_mutex_t mutex;
  pthread_cond_t cond;

  int msgread;
  int msgwrite;

  pthread_t thread;

  struct android_poll_source cmdPollSource;
  struct android_poll_source inputPollSource;

  int running;
  int stateSaved;
  int destroyed;
  int redrawNeeded;
  AInputQueue *pendingInputQueue;
  void *pendingWindow;
} android_app;

/* Functions */
struct android_app *android_shim_init(void);
void android_shim_poll_events(void);
void android_shim_send_cmd(int8_t cmd);

ALooper *ALooper_prepare(int opts);
void ALooper_addFd(void *looper, int fd, int ident, int events, void *callback, void *data);
int ALooper_pollOnce(int timeoutMillis, int *outFd, int *outEvents, void **outData);
int ALooper_pollAll(int timeoutMillis, int *outFd, int *outEvents, void **outData);

void AInputQueue_attachLooper(void *queue, void *looper, int ident, void *callback, void *data);
void AInputQueue_detachLooper(void *queue);
int AInputQueue_getEvent(void *queue, AInputEvent **outEvent);
int AInputQueue_preDispatchEvent(void *queue, void *event);
void AInputQueue_finishEvent(void *queue, void *event, int handled);

int AInputEvent_getType(void *event);
int AInputEvent_getSource(void *event);
int AInputEvent_getDeviceId(void *event);

int AKeyEvent_getAction(void *event);
int AKeyEvent_getKeyCode(void *event);

int AMotionEvent_getAction(void *event);
int64_t AMotionEvent_getEventTime(void *event);
size_t AMotionEvent_getPointerCount(void *event);
int32_t AMotionEvent_getPointerId(void *event, size_t pointer_index);
float AMotionEvent_getX(void *event, size_t pointer_index);
float AMotionEvent_getY(void *event, size_t pointer_index);

void ANativeActivity_finish(ANativeActivity *activity);

#ifdef __cplusplus
}
#endif

#endif /* ANDROID_SHIM_H */
