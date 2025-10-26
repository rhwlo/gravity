#include "serial.h"

SerialDisplay::SerialDisplay(Stream *s) {
    serial = s;
}

void SerialDisplay::begin(void) {
}

void SerialDisplay::pretty_print_time(unsigned long time) {
    unsigned long remaining_time = time;
    if (remaining_time >= HOUR_MILLIS * 10) {
        serial->print((int) (remaining_time / HOUR_MILLIS), 10);
    } else if (remaining_time >= HOUR_MILLIS) {
        serial->print("0");
        serial->print((int) (remaining_time / HOUR_MILLIS), 10);
    } else {
        serial->print("00");
    }
    remaining_time %= HOUR_MILLIS;

    serial->print(':');

    if (remaining_time >= MINUTE_MILLIS * 10) {
        serial->print((int) (remaining_time / MINUTE_MILLIS), 10);
    } else if (remaining_time >= MINUTE_MILLIS) {
        serial->print("0");
        serial->print((int) (remaining_time / MINUTE_MILLIS), 10);
    } else {
        serial->print("00");
    }
    remaining_time %= MINUTE_MILLIS;

    serial->print(':');

    if (remaining_time >= SECOND_MILLIS * 10) {
        serial->print((int) (remaining_time / SECOND_MILLIS), 10);
    } else if (remaining_time >= SECOND_MILLIS) {
        serial->print("0");
        serial->print((int) (remaining_time / SECOND_MILLIS), 10);
    } else {
        serial->print("00");
    }
}

void SerialDisplay::special_toggle(void) {
    serial->println("Special toggle event");
}

void SerialDisplay::render_game_state(GameState *game_state) {
    if (game_state->clock_mode == CM_PAUSED) {
        serial->println("PAUSED");
    }
    if (game_state->player_states[0].out_of_time || game_state->player_states[1].out_of_time) {
        serial->print("Out of time: ");
        if (game_state->player_states[PLAYER_1].out_of_time) {
            serial->print("W ");
        }
        if (game_state->player_states[PLAYER_1].out_of_time) {
            serial->print("B ");
        }
        serial->println();
    }
    if (game_state->whose_turn == PLAYER_1) {
        serial->println("White to move");
        pretty_print_time(game_state->player_states[PLAYER_1].remaining_millis);
        serial->println();
    } else {
        serial->println("Black to move");
        pretty_print_time(game_state->player_states[PLAYER_2].remaining_millis);
        serial->println();
    }
}

void SerialDisplay::print(const char *str) {
    serial->println(str);
}
