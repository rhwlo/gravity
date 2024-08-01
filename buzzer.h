#ifndef BUZZER_H
#define BUZZER_H

#define BUZZER_PIN              9
#define BUZZER_TONE             622  // Eb
#define BUZZER_TONE_FOURTH      831  // Ab
#define BUZZER_TONE_FIFTH       932  // Bb
void singleBeep(void);
void doubleBeep(void);
void chirpFifth(void);
void chirpSus4(void);
void tripleBeep(void);

#endif