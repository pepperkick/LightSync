class Event {
public:
    Event(int t, int v) { type = t; value = v; };
    Event() { type = -1; value = -1; };
    int type;
    int value;
    
    void operator=(const Event& rhs) {
        this->type = rhs.type;
        this->value = rhs.value;
    }

    bool isEqual(const Event& rhs) {
        if (this->type != rhs.type) return false;
        if (this->value != rhs.value) return false;

        return true;
    }
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

    void operator=(const GameState& rhs) {
        this->isPaused = rhs.isPaused;
        this->beatmapEvent = rhs.beatmapEvent;
    }

    bool isEqual(const GameState& rhs) {
        if (this->isPaused != rhs.isPaused) return false;
        if (!this->beatmapEvent.isEqual(rhs.beatmapEvent)) return false;

        return true;
    }
};