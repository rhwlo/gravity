#include "button.h"

ButtonState::ButtonState(void) {
    last_changed_at = 0;
    last_state = HIGH;
    unprocessed_presses = 0;
    ignore_next_release = false;
}
