#include <LCD_I2C.h>
#include "../game_state.h"
#include "double_lcd0108.h"


void blank_buffer(char buffer[COLS], char blank) {
    for (uint8_t i = 0; i < COLS; i++) {
        buffer[i] = blank;
    }
}

void blank_buffers(char buffers[2][COLS], char blank) {
    blank_buffer(buffers[0], blank);
    blank_buffer(buffers[1], blank);
}

void blank_buffers(char buffers[2][COLS]) {
    blank_buffers(buffers, CHAR_BLANK);
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
    blank_buffers(last_displayed);
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

void make_out_of_time_buffer(char buffer[COLS]) {
    memcpy(buffer, " !TIME! ", COLS * sizeof(char));
}

void make_time_display_buffer(char buffer[COLS], unsigned long time, char sep, uint8_t mode) {
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

bool display_buffers_differ(char buf1[COLS], char buf2[COLS]) {
    uint8_t i;
    for (i = 0; i < COLS; i++) {
        if (buf1[i] != buf2[i]) {
            return true;
        }
    }
    return false;
}

void apply_display_buffer(LCD_I2C *lcd, char old_buffer[COLS], char buffer[COLS]) {
    uint8_t i;
    for (i = 0; i < COLS; i++) {
        if (buffer[i] != old_buffer[i]) {
            lcd->setCursor(i, 0);
            lcd->write(buffer[i]);
        }
    }
}

void render_select_settings(char buffers[2][COLS], GameSettings *game_settings) {
    // TODO: smarter display for select settings to accommodate the character limits
    char output[9] = "";
    PlayerSettings *ps = &(game_settings->player_settings[0]);

    uint8_t hours = (ps->total_millis / 60  / 60 / 1000) % 100;
    uint8_t minutes = (ps->total_millis / 60 / 1000) % 60;
    uint8_t seconds = (ps->total_millis / 1000) % 60;

    uint8_t incr_seconds = (ps->per_turn_incr_millis / 1000);

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

    hours = (ps->total_millis / 60  / 60 / 1000) % 100;
    minutes = (ps->total_millis / 60 / 1000) % 60;
    seconds = (ps->total_millis / 1000) % 60;

    incr_seconds = (ps->per_turn_incr_millis / 1000);

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

void render_edit_settings(char buffers[2][COLS], int cursor_positions[2], GameState *game_state, GameSettings *game_settings) {
    cursor_positions[0] = cursor_positions[1] = -1;
    char output[9] = "";
    if (game_state->option_index == OI_TURN_TEN_SECONDS || game_state->option_index == OI_TURN_SECONDS) {
        memcpy(buffers[1], "per turn", COLS * sizeof(char));
        sprintf(output, "  +%2ds  ", game_settings->player_settings[0].per_turn_incr_millis / 1000);
        memcpy(buffers[0], output, COLS * sizeof(char));
        cursor_positions[0] = (game_state->option_index == OI_TURN_TEN_SECONDS) ? 3 : 4;
    } else if (game_state->option_index == OI_FLAG_BEEP || game_state->option_index == OI_TURN_BEEP) {
        memcpy(buffers[1], "beep for", COLS * sizeof(char));
        memcpy(buffers[0], "???? [ ]", COLS * sizeof(char));
        if (game_state->option_index == OI_FLAG_BEEP) {
            buffers[0][0] = 'f'; buffers[0][1] = 'l'; buffers[0][2] = 'a'; buffers[0][3] = 'g';
            if (game_settings->flag_beep) {
                buffers[0][6] = 'X';
            }
        } else {
            buffers[0][0] = 't'; buffers[0][1] = 'u'; buffers[0][2] = 'r'; buffers[0][3] = 'n';
            if (game_settings->turn_beep) {
                buffers[0][6] = 'X';
            }
        }
        cursor_positions[0] = 6;
    } else {
        make_time_display_buffer(buffers[0], game_settings->player_settings[0].total_millis, ':', D_DL18_MODE_HH_MM_SS);
        make_time_display_buffer(buffers[1], game_settings->player_settings[1].total_millis, ':', D_DL18_MODE_HH_MM_SS);
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

void render_game_clock(char buffers[2][COLS], GameState *game_state, bool paused) {
    uint8_t mode = D_DL18_MODE_HH_MM_SS;

    if (game_state->settings->player_settings[0].total_millis < 3600000 && game_state->settings->player_settings[0].per_turn_incr_millis == 0) {
        mode = D_DL18_MODE_MM_SS;
    } else if (game_state->settings->player_settings[0].per_turn_incr_millis > 0) {
        mode = D_DL18_MODE_MM_SSdSS;
    }

    if (game_state->player_states[0].out_of_time) {
        make_out_of_time_buffer(buffers[0]);
    } else {
        make_time_display_buffer(buffers[0], game_state->player_states[0].remaining_millis, ':', mode);
    }

    if (game_state->player_states[1].out_of_time) {
        make_out_of_time_buffer(buffers[1]);
    } else {
        make_time_display_buffer(buffers[1], game_state->player_states[1].remaining_millis, ':', mode);
    }
}

void LCDDisplay8::apply_buffers(char buffers[2][COLS]) {
    apply_display_buffer(&player_1, last_displayed[0], buffers[0]);
    apply_display_buffer(&player_2, last_displayed[1], buffers[1]);
    memcpy(last_displayed[0], buffers[0], COLS * sizeof(buffers[0][0]));
    memcpy(last_displayed[1], buffers[1], COLS * sizeof(buffers[0][0]));
}

/* LCDDisplay8 interprets the "special_toggle" event to mean that it should toggle both backlights. */
void LCDDisplay8::special_toggle(void) {
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
    blank_buffers(new_buffers);
    uint8_t i;
    str2Buffer(new_buffers[1], strA);
    str2Buffer(new_buffers[0], strB);
    apply_buffers(new_buffers);
}

/* Display text directly on the screens */
void LCDDisplay8::print(const char *str) {
    char new_buffers[2][COLS];
    blank_buffers(new_buffers);
    uint8_t i;
    for (i = 0; i < COLS * 2; i++) {
        if (str[i] == 0x00) {
            break;
        }
        new_buffers[1 - (i / COLS)][i % COLS] = str[i];
    }
    apply_buffers(new_buffers);
}

void LCDDisplay8::render_game_state(GameState *game_state) {
    // handle the following game states:
    char new_buffers[2][COLS];
    int cursor_positions[2] = {-1, -1};
    blank_buffers(new_buffers);

    switch (game_state->clock_mode) {
        case CM_SELECT_SETTINGS:
            render_select_settings(new_buffers, game_state->settings);
            break;
        case CM_ACTIVE:
        case CM_PAUSED:
            render_game_clock(new_buffers, game_state, game_state->clock_mode == CM_PAUSED);
            break;
        case CM_EDIT_SETTINGS:
            render_edit_settings(new_buffers, cursor_positions, game_state, game_state->settings);
            break;
        default:
            print("Invalid", "state");
            return;
    }
    apply_buffers(new_buffers);

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

