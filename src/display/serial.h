#ifndef DISPLAY_SERIAL_H
#define DISPLAY_SERIAL_H

#include <Arduino.h>
#include "../game_state.h"
#include "../display.h"


class SerialDisplay : public ChessDisplay {
    public:
        SerialDisplay(Stream *s);
        void render_game_state(GameState *game_state);
        void begin(void);
        void special_toggle(void);
        void print(const char *str);
    private:
        void pretty_print_time(unsigned long time);
        Stream *serial;
};

#endif // DISPLAY_SERIAL_H
