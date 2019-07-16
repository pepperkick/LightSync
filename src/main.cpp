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
#include "include/hueplusplus/IHttpHandler.h"
#include "include/hueplusplus/LinHttpHandler.h"
#include "include/librws.h"
#include "include/config.h"

#include "state.h"

#define VERSION "1.0.0"

using namespace std;

void StartWebsocket();
void StartPhilipsHue();
void CheckData();

rws_socket _socket = NULL;
rws_socket _serverSocket = NULL;
std::vector<std::reference_wrapper<HueLight>> lights;

GameState curState, oldState;

typedef struct __attribute__((__packed__)) {
    int m_Handle;
} Scene;

MAKE_HOOK(GetNameInternal, 0x98CC0C, cs_string*, int handle) {
	return GetNameInternal(handle);
}

MAKE_HOOK(SetActiveScene, 0x98D314, int, Scene scene) {	
    int r = SetActiveScene(scene);
    cs_string* string = GetNameInternal(scene.m_Handle);
    char eventText[128];

    csstrtostr(string, &eventText[0]);
	
    log("[GSI] Scene Loaded: %s!", eventText);

	return r;
}

MAKE_HOOK(PauseGame, 0x1015614, void, void* self) {
	PauseGame(self);
	
    log("[GSI] Paused Game!");

    curState.isPaused = true;
    
    if (_serverSocket)
        rws_socket_send_text(_serverSocket, "{\"event\":\"pause\"}");
}

MAKE_HOOK(ResumeGame, 0x1015710, void, void* self) {
	ResumeGame(self);
	
    log("[GSI] Resumed Game!");

    curState.isPaused = false;
    
    if (_serverSocket)
        rws_socket_send_text(_serverSocket, "{\"event\":\"resume\"}");
}

MAKE_HOOK(BeatmapEventDataGetType, 0xF9A1D0, int, void* self) {
    return BeatmapEventDataGetType(self);
}

MAKE_HOOK(BeatmapEventDataGetValue, 0xF9A1F0, int, void* self) {
    return BeatmapEventDataGetValue(self);
}

MAKE_HOOK(SendBeatmapEventDidTriggerEvent, 0xFA1F94, void, void* self, void* event) {
    SendBeatmapEventDidTriggerEvent(self, event);
    
    int type = (unsigned int) BeatmapEventDataGetType(event);
    int value = (unsigned int) BeatmapEventDataGetValue(event);

    Event* beatEvent = new Event(type, value);
    curState.beatmapEvent = *beatEvent;

    char eventText[128];
    sprintf(&eventText[0], "{ \"event\": \"beatmapEvent\", \"beatmapEvent\": { \"type\": %d, \"value\": %d } }", type, value);

    if (_serverSocket)
        rws_socket_send_text(_serverSocket, eventText);
}

static void on_socket_connected(rws_socket socket) {
    log("[GSI] Socket connected");
    rws_socket_send_text(socket, "{\"event\":\"hello\"}");
    _serverSocket = socket;
}

static void on_socket_disconnected(rws_socket socket) {
    rws_error error = rws_socket_get_error(socket);

    if (error) { 
        log("[GSI] Socket disconnected with code, error: %i, %s", rws_error_get_code(error), rws_error_get_description(error)); 
    } else {
        log("[GSI] Socket disconnected");
    }

    _socket = NULL;
    _serverSocket = NULL;
}

static void on_socket_received_text(rws_socket socket, const char * text, const unsigned int length) {
    log("[GSI] Socket Received Text: %s", text);
}

__attribute__((constructor)) void lib_main() {
    INSTALL_HOOK(GetNameInternal);
    INSTALL_HOOK(SetActiveScene);
    INSTALL_HOOK(PauseGame);
    INSTALL_HOOK(ResumeGame);
    INSTALL_HOOK(BeatmapEventDataGetValue);
    INSTALL_HOOK(BeatmapEventDataGetType);
    INSTALL_HOOK(SendBeatmapEventDidTriggerEvent);

    thread websocketThread(StartWebsocket);
    thread phillipshueThread(StartPhilipsHue);

    websocketThread.detach();
    phillipshueThread.detach();

    log("[GSI] Loaded, Version: %s", VERSION);
}

void StartWebsocket() {
    _socket = rws_socket_create();
    rws_socket_set_scheme(_socket, "ws");
    rws_socket_set_host(_socket, SOCKET_IP);
    rws_socket_set_port(_socket, SOCKET_PORT);
    rws_socket_set_path(_socket, SOCKET_PATH);    
    rws_socket_set_on_disconnected(_socket, &on_socket_disconnected);
    rws_socket_set_on_connected(_socket, &on_socket_connected);
    rws_socket_set_on_received_text(_socket, &on_socket_received_text);
    rws_socket_connect(_socket);

    log("[GSI] Websocket Started");
}

void StartPhilipsHue() {
    log("[GSI] HUE Starting...");

    auto handler = std::make_shared<LinHttpHandler>();
    HueFinder finder(handler);
    vector<HueFinder::HueIdentification> bridges = finder.FindBridges();
    log("[GSI] Searched Bridges...");

    if (bridges.empty()) {
        log("[GSI] No HUE bridges found");

        return;
    }

    Hue bridge = finder.GetBridge(bridges[0]);

    log("[GSI] Connected to HUE bridge %s", bridge.getBridgeIP().c_str());

    lights = bridge.getAllLights();
    log("[GSI] Found %d HUE lights", lights.size());
    
    HueLight light = lights.at(0);
    light.setBrightness(255, 0);
    light.setColorRGB(240, 48, 48, 2);
    this_thread::sleep_for(chrono::milliseconds(250));
    light.setColorRGB(48, 158, 255, 2);
    this_thread::sleep_for(chrono::milliseconds(250));
    light.setColorRGB(255, 255, 255, 2);        

    while (true) {
        CheckData();
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

pair<float, float> lastColor(0.0f, 0.0f);
void CheckData() {       
    for (int i = 0; i < lights.size(); i++) {      
        HueLight light = lights.at(i);  

        if (curState.isPaused && !oldState.isPaused) {
            lastColor = light.getColorXY();  
            light.setBrightness(0, 10);
            log("[GSI] Pause Color");
        } else if (!curState.isPaused && oldState.isPaused) {
            light.setBrightness(255, 0);
            light.setColorXY(lastColor.first, lastColor.second, 2);            
        } else if (curState.beatmapEvent.type < 5) {     
            int value = curState.beatmapEvent.value;     
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

    oldState = curState;
}
