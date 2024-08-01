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