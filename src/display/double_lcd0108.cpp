#include <LCD_I2C.h>
#include "../game_state.h"
#include "double_lcd0108.h"


void blankBuffer(char buffer[COLS], char blank) {
    for (uint8_t i = 0; i < COLS; i++) {
        buffer[i] = blank;
    }
}

void blankBuffers(char buffers[2][COLS], char blank) {
    blankBuffer(buffers[0], blank);
    blankBuffer(buffers[1], blank);
}

void blankBuffers(char buffers[2][COLS]) {
    blankBuffers(buffers, CHAR_BLANK);
}

void str2Buffer(char buffer[COLS], const char *str) {
    for (uint8_t i = 0; i < COLS; i++) {
        if (str[i] == 0x00) {
            break;
        }
        buffer[i] = str[i];
    }
}

LCDDisplay8::LCDDisplay8(uint8_t addr_1, uint8_t addr_2) :
    player_1(addr_1, COLS, ROWS),
    player_2(addr_2, COLS, ROWS) {
    blankBuffers(last_displayed);
    last_cursor_indices[0] = -1;
    last_cursor_indices[1] = -1;
    backlight = false;
}

void LCDDisplay8::begin(void) {
    player_1.begin();
    player_1.clear();
    player_1.setCursor(0, 0);
    player_2.begin();
    player_2.clear();
    player_2.setCursor(0, 0);
    if (backlight) {
        player_1.backlight();
        player_2.backlight();
    } else {
        player_1.noBacklight();
        player_2.noBacklight();
    }
}

void makeOutOfTimeBuffer(char buffer[COLS]) {
    memcpy(buffer, " !TIME! ", COLS * sizeof(char));
}

void makeTimeDisplayBuffer(char buffer[COLS], unsigned long time, char sep, uint8_t mode) {
    char output[9] = "";
    if (mode == D_DL18_MODE_HH_MM_SS) {
        uint8_t hours = (time / 60  / 60 / 1000) % 100;
        uint8_t minutes = (time / 60 / 1000) % 60;
        uint8_t seconds = (time / 1000) % 60;
        sprintf(output, "%2d:%02d:%02d", hours, minutes, seconds);
    } else if (mode == D_DL18_MODE_MM_SS) {
        uint8_t minutes = (time / 60 / 1000) % 60;
        uint8_t seconds = (time / 1000) % 60;
        sprintf(output, " %2d:%02d  ", minutes, seconds);
    } else if (mode == D_DL18_MODE_MM_SSdSS) {
        uint8_t minutes = (time / 60 / 1000) % 60;
        uint8_t seconds = (time / 1000) % 60;
        uint8_t subseconds = (time / 100) % 10;
        sprintf(output, "%2d:%02d.%d ", minutes, seconds, subseconds);
    }
    memcpy(buffer, output, COLS * sizeof(char));
}

bool displayBuffersDiffer(char buf1[COLS], char buf2[COLS]) {
    uint8_t i;
    for (i = 0; i < COLS; i++) {
        if (buf1[i] != buf2[i]) {
            return true;
        }
    }
    return false;
}

void applyDisplayBuffer(LCD_I2C *lcd, char old_buffer[COLS], char buffer[COLS]) {
    uint8_t i;
    for (i = 0; i < COLS; i++) {
        if (buffer[i] != old_buffer[i]) {
            lcd->setCursor(i, 0);
            lcd->write(buffer[i]);
        }
    }
}

void renderSelectSettings(char buffers[2][COLS], game_settings_t *game_settings) {
    // TODO: smarter display for select settings to accommodate the character limits
    char output[9] = "";
    player_settings_t *ps = &(game_settings->player_settings[0]);

    uint8_t hours = (ps->totalMillis / 60  / 60 / 1000) % 100;
    uint8_t minutes = (ps->totalMillis / 60 / 1000) % 60;
    uint8_t seconds = (ps->totalMillis / 1000) % 60;

    uint8_t incr_seconds = (ps->perTurnIncrMillis / 1000);

    if (hours == 0 && incr_seconds < 100 && incr_seconds > 0) {
        if (seconds == 0) {
            sprintf(output, "%2dm+%d", minutes, incr_seconds);
        } else {
            sprintf(output, "%2dm%02d+%d", minutes, seconds, incr_seconds);
        }
    } else if (hours == 0) {
        if (minutes == 0 && seconds != 0) {
            sprintf(output, "%2ds", seconds);
        } else if (seconds == 0 && minutes != 0) {
            sprintf(output, "%2dm", minutes);
        } else {
            sprintf(output, "%2dm%02ds", minutes, seconds);
        }
    } else {
        sprintf(output, "%02dh%02dm%02d", hours, minutes, seconds);
    }

    for (int i = 0; i < COLS; i++) {
        if (output[i] == 0x00) break;
        buffers[0][i] = output[i];
    }

    ps = &(game_settings->player_settings[1]);

    hours = (ps->totalMillis / 60  / 60 / 1000) % 100;
    minutes = (ps->totalMillis / 60 / 1000) % 60;
    seconds = (ps->totalMillis / 1000) % 60;

    incr_seconds = (ps->perTurnIncrMillis / 1000);

    if (hours == 0 && incr_seconds < 100 && incr_seconds > 0) {
        if (seconds == 0) {
            sprintf(output, "%2dm+%d", minutes, incr_seconds);
        } else {
            sprintf(output, "%2dm%02d+%d", minutes, seconds, incr_seconds);
        }
    } else if (hours == 0) {
        if (minutes == 0 && seconds != 0) {
            sprintf(output, "%2ds", seconds);
        } else if (seconds == 0 && minutes != 0) {
            sprintf(output, "%2dm", minutes);
        } else {
            sprintf(output, "%2dm%02ds", minutes, seconds);
        }
    } else {
        sprintf(output, "%dh%02dm%02ds", hours, minutes, seconds);
    }

    for (int i = 0; i < COLS; i++) {
        if (output[i] == 0x00) break;
        buffers[1][i] = output[i];
    }
}

void renderEditSettings(char buffers[2][COLS], int cursor_positions[2], GameState *game_state, game_settings_t *game_settings) {
    cursor_positions[0] = cursor_positions[1] = -1;
    char output[9] = "";
    if (game_state->option_index == OI_TURN_TEN_SECONDS || game_state->option_index == OI_TURN_SECONDS) {
        memcpy(buffers[1], "per turn", COLS * sizeof(char));
        sprintf(output, "  +%2ds  ", game_settings->player_settings[0].perTurnIncrMillis / 1000);
        memcpy(buffers[0], output, COLS * sizeof(char));
        cursor_positions[0] = (game_state->option_index == OI_TURN_TEN_SECONDS) ? 3 : 4;
    } else if (game_state->option_index == OI_FLAG_BEEP || game_state->option_index == OI_TURN_BEEP) {
        memcpy(buffers[1], "beep for", COLS * sizeof(char));
        memcpy(buffers[0], "???? [ ]", COLS * sizeof(char));
        if (game_state->option_index == OI_FLAG_BEEP) {
            buffers[0][0] = 'f'; buffers[0][1] = 'l'; buffers[0][2] = 'a'; buffers[0][3] = 'g';
            if (game_settings->flagBeep) {
                buffers[0][6] = 'X';
            }
        } else {
            buffers[0][0] = 't'; buffers[0][1] = 'u'; buffers[0][2] = 'r'; buffers[0][3] = 'n';
            if (game_settings->turnBeep) {
                buffers[0][6] = 'X';
            }
        }
        cursor_positions[0] = 6;
    } else {
        makeTimeDisplayBuffer(buffers[0], game_settings->player_settings[0].totalMillis, ':', D_DL18_MODE_HH_MM_SS);
        makeTimeDisplayBuffer(buffers[1], game_settings->player_settings[0].totalMillis, ':', D_DL18_MODE_HH_MM_SS);
        switch (game_state->option_index) {
            case OI_P1_HOURS:
                cursor_positions[0] = 1;
                break;
            case OI_P1_TEN_MINUTES:
                cursor_positions[0] = 3;
                break;
            case OI_P1_MINUTES:
                cursor_positions[0] = 4;
                break;
            case OI_P1_TEN_SECONDS:
                cursor_positions[0] = 6;
                break;
            case OI_P1_SECONDS:
                cursor_positions[0] = 7;
                break;
            case OI_P2_HOURS:
                cursor_positions[1] = 1;
                break;
            case OI_P2_TEN_MINUTES:
                cursor_positions[1] = 3;
                break;
            case OI_P2_MINUTES:
                cursor_positions[1] = 4;
                break;
            case OI_P2_TEN_SECONDS:
                cursor_positions[1] = 6;
                break;
            case OI_P2_SECONDS:
                cursor_positions[1] = 7;
                break;
        }
    }
}

void renderGameClock(char buffers[2][COLS], GameState *game_state, bool paused) {
    uint8_t mode = D_DL18_MODE_HH_MM_SS;

    if (game_state->settings->player_settings[0].totalMillis < 3600000 && game_state->settings->player_settings[0].perTurnIncrMillis == 0) {
        mode = D_DL18_MODE_MM_SS;
    } else if (game_state->settings->player_settings[0].perTurnIncrMillis > 0) {
        mode = D_DL18_MODE_MM_SSdSS;
    }

    if (game_state->player_states[0].outOfTime) {
        makeOutOfTimeBuffer(buffers[0]);
    } else {
        makeTimeDisplayBuffer(buffers[0], game_state->player_states[0].remainingMillis, ':', mode);
    }

    if (game_state->player_states[1].outOfTime) {
        makeOutOfTimeBuffer(buffers[1]);
    } else {
        makeTimeDisplayBuffer(buffers[1], game_state->player_states[1].remainingMillis, ':', mode);
    }
}

void LCDDisplay8::applyBuffers(char buffers[2][COLS]) {
    applyDisplayBuffer(&player_1, last_displayed[0], buffers[0]);
    applyDisplayBuffer(&player_2, last_displayed[1], buffers[1]);
    memcpy(last_displayed[0], buffers[0], COLS * sizeof(buffers[0][0]));
    memcpy(last_displayed[1], buffers[1], COLS * sizeof(buffers[0][0]));
}

/* LCDDisplay8 interprets the "specialToggle" event to mean that it should toggle both backlights. */
void LCDDisplay8::specialToggle(void) {
    backlight = !backlight;
    if (backlight) {
        player_1.backlight();
        player_2.backlight();
    } else {
        player_1.noBacklight();
        player_2.noBacklight();
    }
}


void LCDDisplay8::print(const char *strA, const char *strB) {
    char new_buffers[2][COLS];
    blankBuffers(new_buffers);
    uint8_t i;
    str2Buffer(new_buffers[1], strA);
    str2Buffer(new_buffers[0], strB);
    applyBuffers(new_buffers);
}

/* Display text directly on the screens */
void LCDDisplay8::print(const char *str) {
    char new_buffers[2][COLS];
    blankBuffers(new_buffers);
    uint8_t i;
    for (i = 0; i < COLS * 2; i++) {
        if (str[i] == 0x00) {
            break;
        }
        new_buffers[1 - (i / COLS)][i % COLS] = str[i];
    }
    applyBuffers(new_buffers);
}

void LCDDisplay8::renderGameState(GameState *game_state) {
    // handle the following game states:
    char new_buffers[2][COLS];
    int cursor_positions[2] = {-1, -1};
    blankBuffers(new_buffers);

    switch (game_state->clock_mode) {
        case CM_SELECT_SETTINGS:
            renderSelectSettings(new_buffers, game_state->settings);
            break;
        case CM_ACTIVE:
        case CM_PAUSED:
            renderGameClock(new_buffers, game_state, game_state->clock_mode == CM_PAUSED);
            break;
        case CM_EDIT_SETTINGS:
            renderEditSettings(new_buffers, cursor_positions, game_state, game_state->settings);
            break;
        default:
            print("Invalid", "state");
            return;
    }
    applyBuffers(new_buffers);

    if (cursor_positions[0] == -1) {
        player_1.noCursor();
    } else if (cursor_positions[0] < COLS) {
        player_1.cursor();
        player_1.setCursor(cursor_positions[0], 0);
    }
    if (cursor_positions[1] == -1) {
        player_2.noCursor();
    } else if (cursor_positions[1] < COLS) {
        player_2.cursor();
        player_2.setCursor(cursor_positions[1], 0);
    }
}

