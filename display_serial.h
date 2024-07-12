#ifndef DISPLAY_SERIAL_H
#define DISPLAY_SERIAL_H

#include <Arduino.h>
#include "game_state.h"
#include "display_common.h"


class SerialDisplay : public ChessDisplay {
    public:
        SerialDisplay(Stream *s);
        void renderGameState(GameState *game_state);
        void begin(void);
    private:
        void prettyPrintTime(unsigned long time);
        Stream *serial;
};

#endif // DISPLAY_SERIAL_H