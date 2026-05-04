#include <SDL.h>
#include <SDL_opengles2.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "so_util.h"
#include "android_bridge.h"

// Define the ANativeActivity struct (simplified)

typedef struct ANativeActivityCallbacks {
    void (*onStart)(ANativeActivity* activity);
    void (*onResume)(ANativeActivity* activity);
    void* (*onSaveInstanceState)(ANativeActivity* activity, size_t* outSize);
    void (*onPause)(ANativeActivity* activity);
    void (*onStop)(ANativeActivity* activity);
    void (*onDestroy)(ANativeActivity* activity);
    void (*onWindowFocusChanged)(ANativeActivity* activity, int hasFocus);
    void (*onNativeWindowCreated)(ANativeActivity* activity, void* window);
    void (*onNativeWindowResized)(ANativeActivity* activity, void* window);
    void (*onNativeWindowRedrawNeeded)(ANativeActivity* activity, void* window);
    void (*onNativeWindowDestroyed)(ANativeActivity* activity, void* window);
    void (*onInputQueueCreated)(ANativeActivity* activity, void* queue);
    void (*onInputQueueDestroyed)(ANativeActivity* activity, void* queue);
    void (*onContentRectChanged)(ANativeActivity* activity, const void* rect);
    void (*onConfigurationChanged)(ANativeActivity* activity);
    void (*onLowMemory)(ANativeActivity* activity);
} ANativeActivityCallbacks;

typedef struct ANativeActivity {
    struct ANativeActivityCallbacks* callbacks;

    void* vm;
    void* env;
    void* clazz;
    const char* internalDataPath;
    const char* externalDataPath;
    int32_t sdkVersion;
    void* instance;
    void* assetManager;
    const char* obbPath;
} ANativeActivity;

typedef void (*ANativeActivity_createFunc)(ANativeActivity*, void*, size_t);

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Alien Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    // Allocate memory for the .so
    void* base = mmap(NULL, 0x10000000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (so_load("libalien_shooter.so", base, 0x10000000) < 0) {
        printf("Failed to load libalien_shooter.so\n");
        return 1;
    }

    so_relocate();
    so_resolve(android_imports, num_android_imports, 0);
    so_execute_init_array();
    so_finalize();

    ANativeActivity_createFunc onCreate = (ANativeActivity_createFunc)so_find_addr("ANativeActivity_onCreate");
    if (!onCreate) {
        printf("ANativeActivity_onCreate not found\n");
        return 1;
    }

    ANativeActivity activity;
    memset(&activity, 0, sizeof(activity));
    activity.internalDataPath = "./data";
    activity.externalDataPath = "./data";
    activity.sdkVersion = 30;

    onCreate(&activity, NULL, 0);

    // Start the game's loop (usually it starts a thread)
    if (activity.callbacks && activity.callbacks->onStart) {
        activity.callbacks->onStart(&activity);
    }

    // SDL Event Loop
    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            // Map SDL events to Android Input events and send to activity
        }
        // NativeActivity games usually render in their own thread
        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }

    SDL_Quit();
    return 0;
}
