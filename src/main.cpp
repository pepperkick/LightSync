#include <android/log.h>
#include <stdlib.h>
#include <time.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string>
#include <thread>

#include "include/utils/utils.h"
#include "include/inline-hook/inlineHook.h"
#include "include/hueplusplus/Hue.h"
#include "include/hueplusplus/LinHttpHandler.h"

#include "state.h"

#define VERSION "1.0.0"
#define LOG_TAG "LSYNC"

using namespace std;

void StartPhilipsHue();
void PlayWelcomeSequence();
void CheckData();

std::vector<std::reference_wrapper<HueLight>> lights;

GameState curState;
GameStateUpdate stateUpdate;

typedef struct __attribute__((__packed__)) {
    int m_Handle;
} Scene;

MAKE_HOOK(Hook_Scene_GetNameInternal, 0xBE31C4, cs_string*, int handle) {
	return Hook_Scene_GetNameInternal(handle);
}

MAKE_HOOK(Hook_SceneManager_SetActiveScene, 0xBE38CC, int, Scene scene) {
    int r = Hook_SceneManager_SetActiveScene(scene);

    cs_string* string = GetNameInternal(scene.m_Handle);
    char eventText[128];

    csstrtostr(string, &eventText[0]);
    
    log("[%s] Scene Loaded: %s!", LOG_TAG, eventText);

    return r;
}

MAKE_HOOK(Hook_GamePauseManager_PauseGame, 0x1327EAC, void, void* self) {
    Hook_GamePauseManager_PauseGame(self);
	
    log("[%s] Paused Game!", LOG_TAG);

    stateUpdate.type = UpdateType::GAME_PAUSE;
}

MAKE_HOOK(Hook_GamePauseManager_ResumeGame, 0x1327FA8, void, void* self) {
    Hook_GamePauseManager_ResumeGame(self);
	
    log("[%s] Resumed Game!", LOG_TAG);

    stateUpdate.type = UpdateType::GAME_UNPAUSE;
}

MAKE_HOOK(Hook_BeatmapEventData_GetType, 0x12A966C, int, void* self) {
    return Hook_BeatmapEventData_GetType(self);
}

MAKE_HOOK(Hook_BeatmapEventData_GetValue, 0x12AD20C, int, void* self) {
    return Hook_BeatmapEventData_GetValue(self);
}

MAKE_HOOK(Hook_BeatmapObjectCallback_TriggerEvent, 0x12B5130, void, void* self, void* event) {
    Hook_BeatmapObjectCallback_TriggerEvent(self, event);
    
    int type = (unsigned int) BeatmapEventDataGetType(event);
    int value = (unsigned int) BeatmapEventDataGetValue(event);

    Event* beatEvent = new Event(type, value);
    stateUpdate.beatmapEvent = *beatEvent;
    stateUpdate.type = UpdateType::BEATMAP_EVENT;
}

__attribute__((constructor)) void lib_main() {
    INSTALL_HOOK(Hook_Scene_GetNameInternal);
    INSTALL_HOOK(Hook_SceneManager_SetActiveScene);
    INSTALL_HOOK(Hook_GamePauseManager_PauseGame);
    INSTALL_HOOK(Hook_GamePauseManager_ResumeGame);
    INSTALL_HOOK(Hook_BeatmapEventData_GetValue);
    INSTALL_HOOK(Hook_BeatmapEventData_GetType);
    INSTALL_HOOK(Hook_BeatmapObjectCallback_TriggerEvent);

    thread phillipsHue_Thread(StartPhilipsHue);
    phillipsHue_Thread.detach();

    log("[%s] Loaded, Version: %s", LOG_TAG, VERSION);
}

void StartPhilipsHue() {
    log("[%s] Searching for HUE Bridges", LOG_TAG);

    auto handler = std::make_shared<LinHttpHandler>();
    HueFinder finder(handler);
    vector<HueFinder::HueIdentification> bridges = finder.FindBridges();

    if (bridges.empty()) {
        log("[%s] No HUE bridges found", LOG_TAG);

        return;
    }

    Hue bridge = finder.GetBridge(bridges[0]);
    if (bridge.getUsername().empty()) {
        log("[%s] Could not authenticate with HUE bridge", LOG_TAG);

        return;
    }

    log("[%s] Connected to HUE bridge %s", LOG_TAG, bridge.getBridgeIP().c_str());

    lights = bridge.getAllLights();
    log("[%s] Found %d HUE lights", LOG_TAG, lights.size());
    
    PlayWelcomeSequence();

    while (true) {
        CheckData();
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

pair<float, float> lastColor(0.0f, 0.0f);
void CheckData() {
    if (stateUpdate.type == UpdateType::UNKNOWN) return;

    for (int i = 0; i < lights.size(); i++) {      
        HueLight light = lights.at(i);  

        if (stateUpdate.type == UpdateType::GAME_PAUSE) {
            lastColor = light.getColorXY();  
            light.setBrightness(0, 10);

            curState.isPaused = true;
        } else if (stateUpdate.type == UpdateType::GAME_UNPAUSE) {
            light.setBrightness(255, 0);
            light.setColorXY(lastColor.first, lastColor.second, 2);            

            curState.isPaused = false;
        } else if (stateUpdate.type == UpdateType::BEATMAP_EVENT) {
            curState.isPaused = false;     

            int type = stateUpdate.beatmapEvent.type;
            int value = stateUpdate.beatmapEvent.value;     

            if (type > 4) continue;

            switch (value) {
                case 0:
                    light.setBrightness(0, 1);
                    break;
                case 1:
                case 2:
                    light.setColorRGB(48, 158, 255, 1);
                    light.setBrightness(255, 2);
                    break;
                case 3: 
                    light.setColorRGB(48, 158, 255, 1);
                    light.setBrightness(125, 1);
                    break;
                case 4:
                    break;
                case 5:
                case 6:
                    light.setColorRGB(240, 48, 48, 1);
                    light.setBrightness(255, 1);
                    break;
                case 7:
                    light.setColorRGB(240, 48, 48, 1);
                    light.setBrightness(125, 1);
                    break;
            }
        }
    }

    stateUpdate.type = UpdateType::UNKNOWN;
}

void PlayWelcomeSequence() {
    HueLight light = lights.at(0);
    pair<float, float> lastColor(0.0f, 0.0f);
    lastColor = light.getColorXY();  

    light.setBrightness(255, 0);
    light.setColorRGB(240, 48, 48, 5);
    this_thread::sleep_for(chrono::milliseconds(750));
    light.setColorRGB(48, 158, 255, 5);
    this_thread::sleep_for(chrono::milliseconds(750));
    light.setColorXY(lastColor.first, lastColor.second, 5);      
    this_thread::sleep_for(chrono::milliseconds(750));
}