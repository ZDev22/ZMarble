/* licensed under GPL v3.0 see https://github.com/ZDev22/ZEngine/blob/main/LICENSE for current license

An example implementation on how to init and use zengine, as well as a few zdeps!

#define FPS_CAP 60.f // set the framerate, dont define for no FPS cap
*/

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

/* main.c */
#define FPS_CAP 120.f
#define TRACK_FPS

/* ZENGINE */
#define ZENGINE_IMPLEMENTATION
#define ZENGINE_DEPS_DEFINED
//#define ZENGINE_DISABLE_VSYNC
#include "zengine.h"
#include "ztext.h"

#define SPRITES_IMPLEMENTATION
#include "sprites.h"

/* ZFleet */
#include "zmarble/zmarble.h"

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

unsigned int fps = 0;
float appTimer = 0.f;
struct timespec fpsTime;
struct timespec fpsLastTime;

int main(void) {
    /* set gpu vars */
    setenv("__GL_YIELD", "USLEEP", 1);
    setenv("MESA_NO_ERROR", "1", 1);

    /* init window */
    zwindow = RGFW_createWindow("...", 0, 0, 720, 480, 0);

    /* init zengine */
    ZEngineInit();
    initBoard();
    RGFW_window_maximize(zwindow);

    /* set runtime vars  */
    clock_gettime(CLOCK_MONOTONIC, &fpsLastTime);
    srand(fpsLastTime.tv_nsec);

    while (!RGFW_window_shouldClose(zwindow)) {
        /* calculate fps */
        clock_gettime(CLOCK_MONOTONIC, &fpsTime);
        deltaTime = (double)(fpsTime.tv_sec - fpsLastTime.tv_sec) + (double)(fpsTime.tv_nsec - fpsLastTime.tv_nsec) / 1000000000.0;
        fpsLastTime.tv_nsec = fpsTime.tv_nsec;
        fpsLastTime.tv_sec = fpsTime.tv_sec;
        appTimer += deltaTime;

#ifdef FPS_CAP
        usleep((int)(((1.f / FPS_CAP) * 1000000.f) - (deltaTime * 100000.f)));
#endif

#ifdef TRACK_FPS
        if (appTimer > 1.f) {
            char name[6];
            sprintf(name, "%hu", fps);
            RGFW_window_setName(zwindow, name);
            appTimer = 0.f;
            fps = 0;
        }
        fps++;
#endif

        /* poll window events */
        RGFW_event event;
        while (RGFW_window_checkEvent(zwindow, &event)) {
            if (event.type == RGFW_windowResized) {
                framebufferResized = 1;
                break;
            }
        }

        updateBoard();
        ZEngineRender();
    }

    /* free window */
    RGFW_window_close(zwindow);
    zwindow = NULL;

    /* deinit zengine */
    ZEngineDeinit();
}
