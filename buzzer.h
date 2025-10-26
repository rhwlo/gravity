#ifndef BUZZER_H
#define BUZZER_H

#define BUZZER_PIN              9
#define BUZZER_TONE             622  // Eb
#define BUZZER_TONE_FOURTH      831  // Ab
#define BUZZER_TONE_FIFTH       932  // Bb
#define BUZZER_TONE_SEVENTH     1108 // Db

enum BeepEvent {
    BE_FLAG,
    BE_PAUSE,
    BE_RESET,
    BE_SELECT_SETTINGS,
    BE_TURN_CHANGE,
    BE_WARNING,
    BE_EDIT_SETTINGS,
    BE_SAVE_SETTINGS,
    BE_SPECIAL_TOGGLE
};

void beep(BeepEvent event);

#endif
