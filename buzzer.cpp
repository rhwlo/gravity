#include <Arduino.h>
#include "buzzer.h"

void singleBeep(void) {
    tone(BUZZER_PIN, 662);
    delay(110);
    noTone(BUZZER_PIN);
}

void tripleBeep(void) {
    tone(BUZZER_PIN, 662);
    delay(110);
    noTone(BUZZER_PIN);
    delay(40);
    tone(BUZZER_PIN, 662);
    delay(110);
    noTone(BUZZER_PIN);
    delay(40);
    tone(BUZZER_PIN, 662);
    delay(110);
    noTone(BUZZER_PIN);
}