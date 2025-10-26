#ifndef DISPLAY_SSD_H
#define DISPLAY_SSD_H

#include <Adafruit_SSD1306.h>
#include "../game_state.h"
#include "../display.h"

class SSD1306Display : public ChessDisplay {
    public:
        SSD1306Display(void);
        void begin(void);
        void render_game_state(GameState *game_state);
        void special_toggle(void);
        void print(const char *str);
    private:
        void pretty_print_time(unsigned long time);
        Adafruit_SSD1306 ssd;
};

#endif // DISPLAY_SSD_H
