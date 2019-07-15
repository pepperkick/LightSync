#include <android/log.h>
#include <stdlib.h>
#include <time.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "include/inline-hook/inlineHook.h"
#include "include/utils/utils.h"

#define VERSION "1.0.0"

typedef struct __attribute__((__packed__)) {
    int m_Handle;
} Scene;

MAKE_HOOK(GetNameInternal, 0x98CC0C, cs_string*, int handle) {
	return GetNameInternal(handle);
}

MAKE_HOOK(SetActiveScene, 0x98D314, int, int handle) {	
    log("[GSI] Set Active Called");

    int r = SetActiveScene(handle);
	char* str[128];
	
	csstrtostr(GetNameInternal(handle), str);
	
    log("[GSI] Scene Loaded: %s!", str);
	
	return r;
}

MAKE_HOOK(PauseGame, 0x1015614, void, void* self) {
	PauseGame(self);
	
    log("[GSI] Paused Game!");
}

MAKE_HOOK(ResumeGame, 0x1015710, void, void* self) {
	ResumeGame(self);
	
    log("[GSI] Resumed Game!");
}

__attribute__((constructor)) void lib_main() {
    INSTALL_HOOK(GetNameInternal);
    INSTALL_HOOK(SetActiveScene);
    INSTALL_HOOK(PauseGame);
    INSTALL_HOOK(ResumeGame);
	
    log("[GSI] Loaded, Version: %s", VERSION);
}