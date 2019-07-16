#include <android/log.h>
#include <stdlib.h>
#include <time.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string>

#include "include/utils/utils.h"
#include "include/inline-hook/inlineHook.h"
#include "include/librws.h"
#include "include/config.h"

#define VERSION "1.0.0"

using namespace std;

rws_socket _socket = NULL;
rws_socket _serverSocket = NULL;

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
    
    if (_serverSocket)
        rws_socket_send_text(_serverSocket, "{\"event\":\"pause\"}");
}

MAKE_HOOK(ResumeGame, 0x1015710, void, void* self) {
	ResumeGame(self);
	
    log("[GSI] Resumed Game!");
    
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

    _socket = rws_socket_create();
    rws_socket_set_scheme(_socket, "ws");
    rws_socket_set_host(_socket, SOCKET_IP);
    rws_socket_set_port(_socket, SOCKET_PORT);
    rws_socket_set_path(_socket, SOCKET_PATH);    
    rws_socket_set_on_disconnected(_socket, &on_socket_disconnected);
    rws_socket_set_on_connected(_socket, &on_socket_connected);
    rws_socket_set_on_received_text(_socket, &on_socket_received_text);
    rws_socket_connect(_socket);
    
    log("[GSI] Loaded, Version: %s", VERSION);
}