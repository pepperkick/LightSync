enum class UpdateType {
    UNKNOWN = -1,
    GAME_PAUSE = 0,
    GAME_UNPAUSE = 1,
    BEATMAP_EVENT = 2        
}; 

class Event {
public:
    Event(int t, int v) { type = t; value = v; };
    Event() { type = -1; value = -1; };
    int type;
    int value;
};

class GameState {
public:
    GameState() {
        isPaused = false;

        Event* beatmapEvent = new Event();
        this->beatmapEvent = *beatmapEvent;
    };
    Event beatmapEvent;
    bool isPaused;
};

class GameStateUpdate {
public:
    GameStateUpdate() {
        type = UpdateType::UNKNOWN;

        Event* beatmapEvent = new Event();
        this->beatmapEvent = *beatmapEvent;
    }

    UpdateType type;
    Event beatmapEvent;
};