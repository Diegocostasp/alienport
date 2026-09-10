#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "egl_shim.h"
#include <GLES2/gl2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef struct {
  SDL_GLContext sdl_context;
  EGLBoolean is_pbuffer;
  int id;
} EglContextWrapper;

static SDL_Window *g_window = NULL;
static SDL_GLContext g_share_root = NULL;
static pthread_mutex_t g_egl_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_next_context_id = 1;
static _Thread_local EglContextWrapper *g_current_context = NULL;

void egl_shim_set_window(SDL_Window *window) {
  g_window = window;
}

SDL_Window *egl_shim_get_window(void) {
  return g_window;
}

EGLDisplay egl_shim_eglGetDisplay(EGLNativeDisplayType display_id) {
  (void)display_id;
  return (EGLDisplay)(uintptr_t)0x1;
}

EGLBoolean egl_shim_eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) {
  (void)dpy;
  if (major) *major = 1;
  if (minor) *minor = 4;
  return EGL_TRUE;
}

EGLBoolean egl_shim_eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list,
                                   EGLConfig *configs, EGLint config_size,
                                   EGLint *num_config) {
  (void)dpy; (void)attrib_list;
  if (num_config) *num_config = 1;
  if (configs && config_size > 0) {
    configs[0] = (EGLConfig)(uintptr_t)0x1;
  }
  return EGL_TRUE;
}

EGLContext egl_shim_eglCreateContext(EGLDisplay dpy, EGLConfig config,
                                    EGLContext share_context,
                                    const EGLint *attrib_list) {
  (void)dpy; (void)config; (void)share_context; (void)attrib_list;
  if (!g_window) return EGL_NO_CONTEXT;

  pthread_mutex_lock(&g_egl_lock);
  if (g_share_root) {
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
  }

  SDL_GLContext ctx = SDL_GL_CreateContext(g_window);
  if (!ctx) {
    fprintf(stderr, "[egl_shim] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
    pthread_mutex_unlock(&g_egl_lock);
    return EGL_NO_CONTEXT;
  }

  if (!g_share_root) {
    g_share_root = ctx;
  }

  EglContextWrapper *wrap = (EglContextWrapper *)calloc(1, sizeof(EglContextWrapper));
  wrap->sdl_context = ctx;
  wrap->id = g_next_context_id++;
  pthread_mutex_unlock(&g_egl_lock);

  printf("[egl_shim] Context #%d created\n", wrap->id);
  return (EGLContext)wrap;
}

EGLSurface egl_shim_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
                                          EGLNativeWindowType win,
                                          const EGLint *attrib_list) {
  (void)dpy; (void)config; (void)win; (void)attrib_list;
  return (EGLSurface)(uintptr_t)0x2;
}

EGLBoolean egl_shim_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
                                   EGLSurface read, EGLContext ctx) {
  (void)dpy; (void)draw; (void)read;
  if (!g_window) return EGL_FALSE;

  if (!ctx || ctx == EGL_NO_CONTEXT) {
    SDL_GL_MakeCurrent(g_window, NULL);
    g_current_context = NULL;
    return EGL_TRUE;
  }

  EglContextWrapper *wrap = (EglContextWrapper *)ctx;
  if (SDL_GL_MakeCurrent(g_window, wrap->sdl_context) == 0) {
    g_current_context = wrap;
    return EGL_TRUE;
  }

  fprintf(stderr, "[egl_shim] SDL_GL_MakeCurrent failed: %s\n", SDL_GetError());
  return EGL_FALSE;
}

EGLBoolean egl_shim_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
  (void)dpy; (void)surface;
  if (g_window) {
    SDL_GL_SwapWindow(g_window);
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

EGLBoolean egl_shim_eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
                                   EGLint attribute, EGLint *value) {
  (void)dpy; (void)surface;
  if (!value) return EGL_FALSE;

  if (attribute == EGL_WIDTH) {
    *value = SCREEN_WIDTH;
    return EGL_TRUE;
  }
  if (attribute == EGL_HEIGHT) {
    *value = SCREEN_HEIGHT;
    return EGL_TRUE;
  }
  return EGL_TRUE;
}

EGLBoolean egl_shim_eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
  (void)dpy; (void)surface;
  return EGL_TRUE;
}

EGLBoolean egl_shim_eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
  (void)dpy;
  if (!ctx || ctx == EGL_NO_CONTEXT) return EGL_TRUE;
  EglContextWrapper *wrap = (EglContextWrapper *)ctx;
  if (wrap->sdl_context) {
    SDL_GL_DeleteContext(wrap->sdl_context);
  }
  free(wrap);
  return EGL_TRUE;
}

EGLBoolean egl_shim_eglTerminate(EGLDisplay dpy) {
  (void)dpy;
  return EGL_TRUE;
}

EGLint egl_shim_eglGetError(void) {
  return EGL_SUCCESS;
}

void *egl_shim_eglGetProcAddress(const char *procname) {
  if (!procname) return NULL;
  return SDL_GL_GetProcAddress(procname);
}
