#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class ButtonState {
    public:
        ButtonState(void);
        unsigned long last_changed_at;
        int last_state;
        uint8_t unprocessed_presses;
        bool ignore_next_release;
};

#endif // BUTTON_H
