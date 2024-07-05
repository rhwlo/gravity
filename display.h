#ifndef DISPLAY_H
#define DISPLAY_H

#include "game_state.h"
#ifndef ARDUINO
#define ARDUINO 100 
#endif  // ARDUINO
#include <Adafruit_SSD1306.h>

class ChessDisplay {
    public:
        virtual void renderGameState(GameState *game_state);
};

class SerialDisplay : public ChessDisplay {
    public:
        SerialDisplay(Stream *s);
        void renderGameState(GameState *game_state);
        void begin(void);
    private:
        void prettyPrintTime(unsigned long time);
        Stream *serial;
};

class SSD1306Display : public ChessDisplay {
    public:
        SSD1306Display(Adafruit_SSD1306 *d);
        void begin(void);
        void renderGameState(GameState *game_state);
    private:
        void prettyPrintTime(unsigned long time);
        Adafruit_SSD1306 *display;
};

#endif // DISPLAY_H