#include "serial.h"

SerialDisplay::SerialDisplay(Stream *s) {
    serial = s;
}

void SerialDisplay::begin(void) {
}

void SerialDisplay::prettyPrintTime(unsigned long time) {
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

void SerialDisplay::renderGameState(GameState *game_state) {
    if (game_state->clock_mode == CM_PAUSED) {
        serial->println("PAUSED");
    }
    if (game_state->player_states[0].outOfTime || game_state->player_states[1].outOfTime) {
        serial->print("Out of time: ");
        if (game_state->player_states[PLAYER_1].outOfTime) {
            serial->print("W ");
        }
        if (game_state->player_states[PLAYER_1].outOfTime) {
            serial->print("B ");
        }
        serial->println();
    }
    if (game_state->whoseTurn == PLAYER_1) {
        serial->println("White to move");
        prettyPrintTime(game_state->player_states[PLAYER_1].remainingMillis);
        serial->println();
    } else {
        serial->println("Black to move");
        prettyPrintTime(game_state->player_states[PLAYER_2].remainingMillis);
        serial->println();
    }
}
