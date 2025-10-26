#include <Arduino.h>
#include "buzzer.h"

void single_beep(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(110);
    noTone(BUZZER_PIN);
}

void double_beep(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(110);
    noTone(BUZZER_PIN);
    delay(40);
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(110);
    noTone(BUZZER_PIN);
}

void chirp_fifth(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_FIFTH);
    delay(110);
    noTone(BUZZER_PIN);
}

void chirp_five_seven(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_FIFTH);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_SEVENTH);
    delay(110);
    noTone(BUZZER_PIN);
}

void chirp_sus4(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_FOURTH);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_FIFTH);
    delay(110);
    noTone(BUZZER_PIN);
}

void triple_beep(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(110);
    noTone(BUZZER_PIN);
    delay(40);
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(110);
    noTone(BUZZER_PIN);
    delay(40);
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(110);
    noTone(BUZZER_PIN);
}

void beep(BeepEvent event) {
    switch (event)
    {
    case BE_PAUSE:
    case BE_TURN_CHANGE:
        single_beep();
        return;
    case BE_SELECT_SETTINGS:
    case BE_RESET:
        chirp_fifth();
        return;
    case BE_EDIT_SETTINGS:
    case BE_SAVE_SETTINGS:
        chirp_five_seven();
        return;
    case BE_FLAG:
        triple_beep();
        return;
    case BE_WARNING:
        double_beep();
        return;
    case BE_SPECIAL_TOGGLE:
        chirp_five_seven();
        chirp_five_seven();
        return;
    default:
        break;
    }
}
