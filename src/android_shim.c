#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "android_shim.h"
#include "asset_shim.h"
#include "egl_shim.h"
#include "jni_shim.h"
#include "opensles_shim.h"
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef struct FakeInputEvent {
  int type;
  int action;
  int keycode;
  int source;
  float x;
  float y;
  int pointer_count;
  int pointer_id;
  int64_t event_time;
} FakeInputEvent;

#define MAX_INPUT_EVENTS 128
static FakeInputEvent g_input_queue[MAX_INPUT_EVENTS];
static int g_input_head = 0;
static int g_input_tail = 0;
static FakeInputEvent *g_current_event = NULL;

static struct android_app g_app;
static ANativeActivity g_activity;
static ANativeActivityCallbacks g_callbacks;
static SDL_Window *g_sdl_window = NULL;
static SDL_GameController *g_controller = NULL;

static long long g_fake_native_window = 1;
static long long g_fake_input_queue = 1;

static int g_select_pressed = 0;
static int g_start_pressed = 0;

/* Virtual Aim coordinates calculated from right stick */
static float g_aim_x = 320.0f;
static float g_aim_y = 240.0f;
static int g_aim_active = 0;

static int input_queue_count(void) {
  return (g_input_head - g_input_tail + MAX_INPUT_EVENTS) % MAX_INPUT_EVENTS;
}

static int input_queue_push(const FakeInputEvent *ev) {
  int next = (g_input_head + 1) % MAX_INPUT_EVENTS;
  if (next == g_input_tail) return 0; // full
  g_input_queue[g_input_head] = *ev;
  g_input_head = next;
  return 1;
}

static FakeInputEvent *input_queue_pop(void) {
  if (g_input_tail == g_input_head) return NULL;
  FakeInputEvent *ev = &g_input_queue[g_input_tail];
  g_input_tail = (g_input_tail + 1) % MAX_INPUT_EVENTS;
  return ev;
}

static void push_key_event(int action, int keycode) {
  FakeInputEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = AINPUT_EVENT_TYPE_KEY;
  ev.action = action;
  ev.keycode = keycode;
  ev.source = AINPUT_SOURCE_GAMEPAD;
  input_queue_push(&ev);
}

static void push_motion_event(int action, float x, float y, int pointer_id) {
  FakeInputEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = AINPUT_EVENT_TYPE_MOTION;
  ev.action = action;
  ev.source = AINPUT_SOURCE_TOUCHSCREEN;
  ev.x = x;
  ev.y = y;
  ev.pointer_count = 1;
  ev.pointer_id = pointer_id;
  input_queue_push(&ev);
}

void android_shim_send_cmd(int8_t cmd) {
  if (g_app.msgwrite >= 0) {
    if (write(g_app.msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
      fprintf(stderr, "[android_shim] write cmd %d failed\n", cmd);
    }
  }
}

static void process_cmd(struct android_app *app, struct android_poll_source *source) {
  (void)source;
  int8_t cmd;
  if (read(app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd)) {
    switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      pthread_mutex_lock(&app->mutex);
      app->window = (void *)&g_fake_native_window;
      pthread_cond_broadcast(&app->cond);
      pthread_mutex_unlock(&app->mutex);
      break;
    case APP_CMD_TERM_WINDOW:
      pthread_mutex_lock(&app->mutex);
      app->window = NULL;
      pthread_cond_broadcast(&app->cond);
      pthread_mutex_unlock(&app->mutex);
      break;
    case APP_CMD_RESUME:
    case APP_CMD_START:
    case APP_CMD_GAINED_FOCUS:
      pthread_mutex_lock(&app->mutex);
      app->activityState = 1;
      pthread_cond_broadcast(&app->cond);
      pthread_mutex_unlock(&app->mutex);
      break;
    case APP_CMD_PAUSE:
    case APP_CMD_STOP:
    case APP_CMD_LOST_FOCUS:
      pthread_mutex_lock(&app->mutex);
      app->activityState = 0;
      pthread_cond_broadcast(&app->cond);
      pthread_mutex_unlock(&app->mutex);
      break;
    case APP_CMD_DESTROY:
      pthread_mutex_lock(&app->mutex);
      app->destroyRequested = 1;
      pthread_cond_broadcast(&app->cond);
      pthread_mutex_unlock(&app->mutex);
      break;
    }
    if (app->onAppCmd) {
      app->onAppCmd(app, cmd);
    }
  }
}

static void process_input(struct android_app *app, struct android_poll_source *source) {
  (void)source;
  AInputEvent *event = NULL;
  while (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
    if (AInputQueue_preDispatchEvent(app->inputQueue, (void *)event))
      continue;
    int32_t handled = 0;
    if (app->onInputEvent) {
      handled = app->onInputEvent(app, event);
    }
    AInputQueue_finishEvent(app->inputQueue, (void *)event, handled);
  }
}

void android_shim_poll_events(void) {
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    if (ev.type == SDL_QUIT) {
      android_shim_send_cmd(APP_CMD_DESTROY);
      return;
    }

    /* Controller Hotkey: SELECT + START = Exit */
    if (ev.type == SDL_CONTROLLERBUTTONDOWN) {
      if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) g_select_pressed = 1;
      if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_START) g_start_pressed = 1;
      if (g_select_pressed && g_start_pressed) {
        printf("[android_shim] SELECT+START hotkey pressed -> Exiting.\n");
        exit(0);
      }
    } else if (ev.type == SDL_CONTROLLERBUTTONUP) {
      if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) g_select_pressed = 0;
      if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_START) g_start_pressed = 0;
    }

    /* Gamepad Button Mapping */
    if (ev.type == SDL_CONTROLLERBUTTONDOWN || ev.type == SDL_CONTROLLERBUTTONUP) {
      int action = (ev.type == SDL_CONTROLLERBUTTONDOWN) ? AKEY_EVENT_ACTION_DOWN : AKEY_EVENT_ACTION_UP;
      switch (ev.cbutton.button) {
      case SDL_CONTROLLER_BUTTON_A:
        push_key_event(action, AKEYCODE_BUTTON_A);
        break;
      case SDL_CONTROLLER_BUTTON_B:
        push_key_event(action, AKEYCODE_BUTTON_B);
        break;
      case SDL_CONTROLLER_BUTTON_X:
        push_key_event(action, AKEYCODE_BUTTON_X);
        break;
      case SDL_CONTROLLER_BUTTON_Y:
        push_key_event(action, AKEYCODE_BUTTON_Y);
        break;
      case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        push_key_event(action, AKEYCODE_BUTTON_L1);
        break;
      case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        push_key_event(action, AKEYCODE_BUTTON_R1);
        break;
      case SDL_CONTROLLER_BUTTON_START:
        push_key_event(action, AKEYCODE_BUTTON_START);
        break;
      case SDL_CONTROLLER_BUTTON_BACK:
        push_key_event(action, AKEYCODE_BUTTON_SELECT);
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_UP:
        push_key_event(action, AKEYCODE_DPAD_UP);
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        push_key_event(action, AKEYCODE_DPAD_DOWN);
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        push_key_event(action, AKEYCODE_DPAD_LEFT);
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        push_key_event(action, AKEYCODE_DPAD_RIGHT);
        break;
      }
    }

    /* Triggers and Analog Stick Mapping */
    if (ev.type == SDL_CONTROLLERAXISMOTION) {
      float val = (float)ev.caxis.value / 32767.0f;

      // Left Stick (Movement) -> Maps to Virtual Touch Joystick on Left Screen (x: 120, y: 360)
      if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX || ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
        static float lx = 0, ly = 0;
        if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) lx = val;
        if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) ly = val;

        float len = sqrtf(lx * lx + ly * ly);
        if (len > 0.2f) {
          float touch_x = 120.0f + lx * 60.0f;
          float touch_y = 360.0f + ly * 60.0f;
          push_motion_event(AMOTION_EVENT_ACTION_MOVE, touch_x, touch_y, 0);
        } else {
          push_motion_event(AMOTION_EVENT_ACTION_UP, 120.0f, 360.0f, 0);
        }
      }

      // Right Stick (Aiming in 360°) -> Maps to Virtual Aim Pointer on Right Screen (x: 520, y: 360)
      if (ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX || ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
        static float rx = 0, ry = 0;
        if (ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) rx = val;
        if (ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) ry = val;

        float len = sqrtf(rx * rx + ry * ry);
        if (len > 0.25f) {
          g_aim_x = 520.0f + rx * 70.0f;
          g_aim_y = 360.0f + ry * 70.0f;
          g_aim_active = 1;
          push_motion_event(AMOTION_EVENT_ACTION_MOVE, g_aim_x, g_aim_y, 1);
        } else if (g_aim_active) {
          g_aim_active = 0;
          push_motion_event(AMOTION_EVENT_ACTION_UP, g_aim_x, g_aim_y, 1);
        }
      }

      // R2 Trigger (Fire weapon)
      if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
        static int r2_down = 0;
        if (val > 0.4f && !r2_down) {
          r2_down = 1;
          push_key_event(AKEY_EVENT_ACTION_DOWN, AKEYCODE_BUTTON_R2);
          // Also simulate touch fire on aim point
          push_motion_event(AMOTION_EVENT_ACTION_DOWN, g_aim_x, g_aim_y, 1);
        } else if (val <= 0.4f && r2_down) {
          r2_down = 0;
          push_key_event(AKEY_EVENT_ACTION_UP, AKEYCODE_BUTTON_R2);
          push_motion_event(AMOTION_EVENT_ACTION_UP, g_aim_x, g_aim_y, 1);
        }
      }

      // L2 Trigger (Reload)
      if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
        static int l2_down = 0;
        if (val > 0.4f && !l2_down) {
          l2_down = 1;
          push_key_event(AKEY_EVENT_ACTION_DOWN, AKEYCODE_BUTTON_L2);
        } else if (val <= 0.4f && l2_down) {
          l2_down = 0;
          push_key_event(AKEY_EVENT_ACTION_UP, AKEYCODE_BUTTON_L2);
        }
      }
    }
  }
}

struct android_app *android_shim_init(void) {
  memset(&g_app, 0, sizeof(g_app));
  memset(&g_activity, 0, sizeof(g_activity));
  memset(&g_callbacks, 0, sizeof(g_callbacks));

  pthread_mutex_init(&g_app.mutex, NULL);
  pthread_cond_init(&g_app.cond, NULL);

  int msgpipe[2];
  if (pipe(msgpipe) < 0) {
    perror("[android_shim] pipe error");
    return NULL;
  }
  g_app.msgread = msgpipe[0];
  g_app.msgwrite = msgpipe[1];

  g_app.cmdPollSource.id = LOOPER_ID_MAIN;
  g_app.cmdPollSource.app = &g_app;
  g_app.cmdPollSource.process = process_cmd;

  g_app.inputPollSource.id = LOOPER_ID_INPUT;
  g_app.inputPollSource.app = &g_app;
  g_app.inputPollSource.process = process_input;

  g_activity.callbacks = &g_callbacks;
  g_activity.vm = jni_get_vm();
  g_activity.env = jni_get_env();
  g_activity.internalDataPath = asset_shim_get_savedir();
  g_activity.externalDataPath = asset_shim_get_savedir();
  g_activity.sdkVersion = 28;

  g_app.activity = &g_activity;
  g_app.inputQueue = (AInputQueue *)&g_fake_input_queue;

  /* Initialize SDL2 Video and GameController */
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "[android_shim] SDL_Init failed: %s\n", SDL_GetError());
    return NULL;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  g_sdl_window = SDL_CreateWindow(
      "Alien Shooter",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      SCREEN_WIDTH, SCREEN_HEIGHT,
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  if (!g_sdl_window) {
    fprintf(stderr, "[android_shim] SDL_CreateWindow failed: %s\n", SDL_GetError());
    return NULL;
  }

  egl_shim_set_window(g_sdl_window);

  /* Open GameController */
  if (SDL_NumJoysticks() > 0) {
    g_controller = SDL_GameControllerOpen(0);
    if (g_controller) {
      printf("[android_shim] Controller opened: %s\n", SDL_GameControllerName(g_controller));
    }
  }

  /* Open audio subsystem */
  opensles_shim_init();

  return &g_app;
}

/* ---------------- ALooper ---------------- */

ALooper *ALooper_prepare(int opts) {
  (void)opts;
  static long long fake_looper = 1;
  return (ALooper *)&fake_looper;
}

void ALooper_addFd(void *looper, int fd, int ident, int events, void *callback, void *data) {
  (void)looper; (void)fd; (void)ident; (void)events; (void)callback; (void)data;
}

int ALooper_pollOnce(int timeoutMillis, int *outFd, int *outEvents, void **outData) {
  return ALooper_pollAll(timeoutMillis, outFd, outEvents, outData);
}

int ALooper_pollAll(int timeoutMillis, int *outFd, int *outEvents, void **outData) {
  (void)outFd; (void)outEvents;

  // Pump audio callbacks before any polling
  opensles_shim_pump_callbacks();

  // 1. Check command pipe
  struct pollfd pfd;
  pfd.fd = g_app.msgread;
  pfd.events = POLLIN;
  pfd.revents = 0;

  int timeout = timeoutMillis;
  if (timeout < 0 || timeout > 5) timeout = 5;

  int ret = poll(&pfd, 1, timeout);
  if (ret > 0 && (pfd.revents & POLLIN)) {
    if (outData) *outData = &g_app.cmdPollSource;
    return LOOPER_ID_MAIN;
  }

  // 2. Poll SDL events
  android_shim_poll_events();

  // 3. Check input queue
  if (input_queue_count() > 0) {
    if (outData) *outData = &g_app.inputPollSource;
    return LOOPER_ID_INPUT;
  }

  opensles_shim_pump_callbacks();
  return -1;
}

/* ---------------- AInputQueue ---------------- */

void AInputQueue_attachLooper(void *queue, void *looper, int ident, void *callback, void *data) {
  (void)queue; (void)looper; (void)ident; (void)callback; (void)data;
}

void AInputQueue_detachLooper(void *queue) { (void)queue; }

int AInputQueue_getEvent(void *queue, AInputEvent **outEvent) {
  (void)queue;
  FakeInputEvent *ev = input_queue_pop();
  if (!ev) {
    if (outEvent) *outEvent = NULL;
    return -1;
  }
  g_current_event = ev;
  if (outEvent) *outEvent = (AInputEvent *)ev;
  return 0;
}

int AInputQueue_preDispatchEvent(void *queue, void *event) {
  (void)queue; (void)event;
  return 0;
}

void AInputQueue_finishEvent(void *queue, void *event, int handled) {
  (void)queue; (void)event; (void)handled;
  g_current_event = NULL;
}

/* ---------------- AInputEvent getters ---------------- */

int AInputEvent_getType(void *event) {
  FakeInputEvent *ev = (FakeInputEvent *)event;
  return ev ? ev->type : 0;
}

int AInputEvent_getSource(void *event) {
  FakeInputEvent *ev = (FakeInputEvent *)event;
  return ev ? ev->source : AINPUT_SOURCE_GAMEPAD;
}

int AInputEvent_getDeviceId(void *event) {
  (void)event;
  return 1;
}

int AKeyEvent_getAction(void *event) {
  FakeInputEvent *ev = (FakeInputEvent *)event;
  return ev ? ev->action : 0;
}

int AKeyEvent_getKeyCode(void *event) {
  FakeInputEvent *ev = (FakeInputEvent *)event;
  return ev ? ev->keycode : 0;
}

int AMotionEvent_getAction(void *event) {
  FakeInputEvent *ev = (FakeInputEvent *)event;
  return ev ? ev->action : 0;
}

int64_t AMotionEvent_getEventTime(void *event) {
  (void)event;
  return (int64_t)SDL_GetTicks() * 1000000LL;
}

size_t AMotionEvent_getPointerCount(void *event) {
  FakeInputEvent *ev = (FakeInputEvent *)event;
  return ev ? ev->pointer_count : 1;
}

int32_t AMotionEvent_getPointerId(void *event, size_t pointer_index) {
  (void)pointer_index;
  FakeInputEvent *ev = (FakeInputEvent *)event;
  return ev ? ev->pointer_id : 0;
}

float AMotionEvent_getX(void *event, size_t pointer_index) {
  (void)pointer_index;
  FakeInputEvent *ev = (FakeInputEvent *)event;
  return ev ? ev->x : 0.0f;
}

float AMotionEvent_getY(void *event, size_t pointer_index) {
  (void)pointer_index;
  FakeInputEvent *ev = (FakeInputEvent *)event;
  return ev ? ev->y : 0.0f;
}

void ANativeActivity_finish(ANativeActivity *activity) {
  (void)activity;
  printf("[android_shim] ANativeActivity_finish called -> exiting\n");
  exit(0);
}
