#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"
#include <EEPROM.h>


#define VALIDATION_BYTE_0 0xE4
#define VALIDATION_BYTE_1 0xC5
#define GAME_SETTINGS_LEN 4

#define OI_P2_HOURS             0
#define OI_P2_TEN_MINUTES       1
#define OI_P2_MINUTES           2
#define OI_P2_TEN_SECONDS       3
#define OI_P2_SECONDS           4
#define OI_P1_HOURS             5
#define OI_P1_TEN_MINUTES       6
#define OI_P1_MINUTES           7
#define OI_P1_TEN_SECONDS       8
#define OI_P1_SECONDS           9
#define OI_TURN_TEN_SECONDS     10
#define OI_TURN_SECONDS         11
#define OI_FLAG_BEEP            12
#define OI_TURN_BEEP            13
#define OI_COUNT                14

enum clock_mode_t {
    CM_PAUSED,
    CM_ACTIVE,
    CM_SELECT_SETTINGS,
    CM_EDIT_SETTINGS
};

struct player_settings_t
{
    unsigned long  totalMillis;
    unsigned short perTurnIncrMillis;
};

struct game_settings_t
{
    player_settings_t player_settings[NUM_PLAYERS];
    bool flagBeep;      // beep when a player runs out of time
    bool turnBeep;      // beep when we change whose turn it is
};

void write_state_to_eeprom(EEPROMClass *eeprom);
bool read_state_from_eeprom(EEPROMClass *eeprom);

struct player_state_t {
    bool outOfTime;
    unsigned long remainingMillis;
};

class GameState {
    public:
        GameState(game_settings_t *game_settings);
        bool setTurn(unsigned short newTurn);
        clock_mode_t clock_mode;
        int option_index;
        void reset(void);
        unsigned short whoseTurn;
        player_state_t player_states[NUM_PLAYERS];
        player_state_t *curr_player_state;
        game_settings_t *settings;
};

extern game_settings_t setting_blitz_5m_0, standard_settings, setting_blitz_30s_0, setting_blitz_5m_3s;
extern game_settings_t all_game_settings[GAME_SETTINGS_LEN];
extern uint8_t selected_game_settings;


#endif  // GAME_STATE_H