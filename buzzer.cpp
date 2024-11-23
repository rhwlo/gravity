#include <Arduino.h>
#include "buzzer.h"

void singleBeep(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(110);
    noTone(BUZZER_PIN);
}

void doubleBeep(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(110);
    noTone(BUZZER_PIN);
    delay(40);
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(110);
    noTone(BUZZER_PIN);
}

void chirpFifth(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_FIFTH);
    delay(110);
    noTone(BUZZER_PIN);
}

void chirpFiveSeven(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_FIFTH);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_SEVENTH);
    delay(110);
    noTone(BUZZER_PIN);
}

void chirpSus4(void) {
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_FOURTH);
    delay(60);
    tone(BUZZER_PIN, BUZZER_TONE_FIFTH);
    delay(110);
    noTone(BUZZER_PIN);
}

void tripleBeep(void) {
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

void beep(beep_event_t event) {
    switch (event)
    {
    case BE_PAUSE:
    case BE_TURN_CHANGE:
        singleBeep();
        return;
    case BE_SELECT_SETTINGS:
    case BE_RESET:
        chirpFifth();
        return;
    case BE_EDIT_SETTINGS:
    case BE_SAVE_SETTINGS:
        chirpFiveSeven();
        return;
    case BE_FLAG:
        tripleBeep();
        return;
    case BE_WARNING:
        doubleBeep();
        return;
    case BE_SPECIAL_TOGGLE:
        chirpFiveSeven();
        chirpFiveSeven();
        return;
    default:
        break;
    }
}