#ifndef EGL_SHIM_H
#define EGL_SHIM_H

#include <SDL2/SDL.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

void egl_shim_set_window(SDL_Window *window);
SDL_Window *egl_shim_get_window(void);

EGLDisplay egl_shim_eglGetDisplay(EGLNativeDisplayType display_id);
EGLBoolean egl_shim_eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
EGLBoolean egl_shim_eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list,
                                   EGLConfig *configs, EGLint config_size,
                                   EGLint *num_config);
EGLBoolean egl_shim_eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config,
                                       EGLint attribute, EGLint *value);
EGLContext egl_shim_eglCreateContext(EGLDisplay dpy, EGLConfig config,
                                    EGLContext share_context,
                                    const EGLint *attrib_list);
EGLSurface egl_shim_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
                                          EGLNativeWindowType win,
                                          const EGLint *attrib_list);
EGLBoolean egl_shim_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
                                   EGLSurface read, EGLContext ctx);
EGLBoolean egl_shim_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
EGLBoolean egl_shim_eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
                                   EGLint attribute, EGLint *value);
EGLBoolean egl_shim_eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
EGLBoolean egl_shim_eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
EGLBoolean egl_shim_eglTerminate(EGLDisplay dpy);
EGLint egl_shim_eglGetError(void);
void *egl_shim_eglGetProcAddress(const char *procname);

#ifdef __cplusplus
}
#endif

#endif /* EGL_SHIM_H */
