#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"
#include <EEPROM.h>


#define VALIDATION_BYTE_0 0xE4
#define VALIDATION_BYTE_1 0xC5
#define GAME_SETTINGS_LEN 4

enum clock_mode_t {
    PAUSED,
    ACTIVE,
    SELECT_SETTINGS
};

struct player_settings_t
{
    unsigned long  totalMillis;
    unsigned short perTurnIncrMillis;
    unsigned short gracePeriodMillis;
};

struct game_settings_t
{
    player_settings_t player_settings[NUM_PLAYERS];
    bool warningBeep;   // warn when we cross a threshold
    bool flagBeep;      // beep when a player runs out of time
    bool turnBeep;      // beep when we change whose turn it is
};

int write_setting_to_eeprom(game_settings_t *game_settings, EEPROMClass *eeprom, int eeprom_offset);
int write_validation_to_eeprom(EEPROMClass *eeprom, int eeprom_offset);
int read_setting_from_eeprom(game_settings_t *game_settings, EEPROMClass *eeprom, int eeprom_offset);
bool read_validation_from_eeprom(EEPROMClass *eeprom, int eeprom_offset);

struct player_state_t {
    bool outOfTime;
    unsigned long remainingMillis;
    unsigned short gracePeriodMillis;
};

class GameState {
    public:
        GameState(game_settings_t *game_settings);
        bool setTurn(unsigned short newTurn);
        clock_mode_t clock_mode;
        void pause(void);
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