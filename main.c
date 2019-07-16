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

MAKE_HOOK(SetActiveScene, 0x98D314, int, Scene scene) {	
    int r = SetActiveScene(scene);
	char* str[128];
	
	csstrtostr(GetNameInternal(scene.m_Handle), str);
	
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

MAKE_HOOK(BeatmapEventDataGetType, 0xF9A1D0, int, void* self) {
    return BeatmapEventDataGetType(self);
}

MAKE_HOOK(BeatmapEventDataGetValue, 0xF9A1F0, int, void* self) {
    return BeatmapEventDataGetValue(self);
}

MAKE_HOOK(SendBeatmapEventDidTriggerEvent, 0xFA1F94, void, void* self, void* event) {
    SendBeatmapEventDidTriggerEvent(self, event);
    
    log("[GSI] BeatmapEvent Data: %d %d!", (unsigned int) BeatmapEventDataGetValue(event), (unsigned int) BeatmapEventDataGetType(event));
}

__attribute__((constructor)) void lib_main() {
    INSTALL_HOOK(GetNameInternal);
    INSTALL_HOOK(SetActiveScene);
    INSTALL_HOOK(PauseGame);
    INSTALL_HOOK(ResumeGame);
    INSTALL_HOOK(BeatmapEventDataGetValue);
    INSTALL_HOOK(BeatmapEventDataGetType);
    INSTALL_HOOK(SendBeatmapEventDidTriggerEvent);
	
    log("[GSI] Loaded, Version: %s", VERSION);
}