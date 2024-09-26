#include <Arduino.h>
#include <../include/pitches.h>

#define SIZE 8
#define DIV 2
#define LEFT 0x01
#define UP 0x02
#define RIGHT 0x04
#define DOWN 0x08
#define CLEAR 0xff
#define START_SCREEN {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
#define START_PLAYER 0x0000
#define LOW_THRESHOLD 20
#define HIGH_THRESHOLD 1002
#define VRX A5
#define VRY A4
#define SW 12
#define SND 13
#define TONE_LENGTH 1000
#define TUNE_COUNT 8
#define COOLDOWN_LENGTH 200
#define DEMO_VERSION 0

/*
Memory game for Arduino with an 8x8 LED matrix (1588AS) and a joystick.

Connecting
Arduino pin     Martix pin     Joystick pin
0               R8 (5)
1               R7 (2)
2               R6 (7)
3               R5 (1)
4               R4 (12)
5               R3 (8)
6               R2 (14)
7               R1 (9)
8               C1 (13)
9               C2 (3)
10              C3 (4)
11              C4 (10)
12              -               SW
13              -               -
A0              C5 (6)
A1              C6 (11)
A2              C7 (15)
A3              C8 (16)
A4              -               VRY
A5              -               VRX

*/


uint8_t rows[] = {8, 9, 10, 11, A0, A1, A2, A3};

uint8_t screen[SIZE] = START_SCREEN;
uint16_t player = START_PLAYER;
bool selected = false;
uint16_t selected_tile = 0xFFFF;

uint8_t table[SIZE / DIV][SIZE / DIV];

bool playing_tune = false;

struct note {
    uint32_t freq;
    uint32_t length;
};

#if DEMO_VERSION == 1

uint16_t bpms[9] = {134, 65, 180, 260, 65, 240, 280 ,500, 240};

note* tunes[9] =
{
    // Bring back the only demoart
(note[]){{1, TONE_LENGTH / 2}, {NOTE_F4, TONE_LENGTH / 2}, {NOTE_C5, TONE_LENGTH / 2}, {NOTE_F4, TONE_LENGTH / 2}, {NOTE_C5, TONE_LENGTH / 2}, {NOTE_DS5, TONE_LENGTH / 2}, {1, TONE_LENGTH / 2}, {NOTE_DS4, TONE_LENGTH / 2}, {1, TONE_LENGTH / 2}, {NOTE_DS4, TONE_LENGTH / 2}, {NOTE_GS4, TONE_LENGTH / 2}, {NOTE_DS4, TONE_LENGTH / 2}, {NOTE_GS4, TONE_LENGTH / 2}, {1, TONE_LENGTH / 2}, {NOTE_GS4, TONE_LENGTH / 2}, {NOTE_G4, TONE_LENGTH / 2}, {0, TONE_LENGTH}},
    // 2ND_PM
 (note[]){{NOTE_AS4, TONE_LENGTH / 8}, {1, TONE_LENGTH / 8}, {NOTE_AS5, TONE_LENGTH / 8}, {NOTE_F5, TONE_LENGTH / 8}, {NOTE_AS5, TONE_LENGTH / 8}, {NOTE_FS5, TONE_LENGTH / 8}, {NOTE_AS4, TONE_LENGTH / 8}, {1, TONE_LENGTH / 8}, {NOTE_AS5, TONE_LENGTH / 8}, {NOTE_F5, TONE_LENGTH / 8}, {NOTE_AS5, TONE_LENGTH / 8}, {NOTE_FS5, TONE_LENGTH / 8}, {NOTE_CS5, TONE_LENGTH / 8}, {1, TONE_LENGTH / 8}, {NOTE_GS4, TONE_LENGTH / 4}, {0, TONE_LENGTH}},
    // Pile
    (note[]){{NOTE_B4, TONE_LENGTH * 3 / 4}, {NOTE_B4, TONE_LENGTH * 3 / 4}, {NOTE_G4, TONE_LENGTH * 3 / 4}, {NOTE_G4, TONE_LENGTH * 3 / 4}, {NOTE_G4, TONE_LENGTH}, {NOTE_B4, TONE_LENGTH * 3 / 4}, {NOTE_B4, TONE_LENGTH * 3 / 4}, {NOTE_G4, TONE_LENGTH * 3 / 4}, {NOTE_G4, TONE_LENGTH * 3 / 4}, {NOTE_G4, TONE_LENGTH}, {NOTE_D5, TONE_LENGTH * 3 / 4}, {NOTE_D5, TONE_LENGTH * 3 / 4}, {NOTE_C5, TONE_LENGTH * 3 / 4}, {NOTE_C5, TONE_LENGTH * 3 / 4}, {NOTE_C5, TONE_LENGTH}, {NOTE_D5, TONE_LENGTH * 3 / 4}, {NOTE_D5, TONE_LENGTH * 3 / 4}, {NOTE_C5, TONE_LENGTH * 3 / 4}, {NOTE_C5, TONE_LENGTH * 3 / 4}, {NOTE_C5, TONE_LENGTH}, {0, TONE_LENGTH}},
    // Monk
(note[]){{NOTE_AS4, TONE_LENGTH}, {NOTE_AS5, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH}, {1, TONE_LENGTH}, {NOTE_F5, TONE_LENGTH}, {1, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH}, {1, TONE_LENGTH}, {NOTE_CS5, TONE_LENGTH}, {1, TONE_LENGTH}, {NOTE_DS5, TONE_LENGTH}, {NOTE_F5, TONE_LENGTH}, {0, TONE_LENGTH}},
    //Median
(note[]){{NOTE_FS4, TONE_LENGTH}, {NOTE_CS4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH * 2}, {NOTE_D4, TONE_LENGTH}, {NOTE_B3, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
    //Bad Apple
(note[]){{NOTE_DS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_GS4, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH * 2}, {NOTE_DS5, TONE_LENGTH}, {NOTE_CS5, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH * 2}, {NOTE_DS4, TONE_LENGTH * 2}, {NOTE_AS4, TONE_LENGTH}, {NOTE_GS4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_DS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_GS4, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
    //Deadlock
(note[]){{NOTE_DS5, TONE_LENGTH * 5}, {NOTE_AS4, TONE_LENGTH}, {NOTE_F5, TONE_LENGTH}, {NOTE_DS5, TONE_LENGTH / 2}, {NOTE_AS5, TONE_LENGTH * 11 / 2}, {NOTE_G5, TONE_LENGTH}, {NOTE_F5, TONE_LENGTH}, {NOTE_DS5, TONE_LENGTH / 2}, {NOTE_F5, TONE_LENGTH * 11 / 2}, {0, TONE_LENGTH}},
    //Elysium
(note[]){{NOTE_C5, TONE_LENGTH}, {NOTE_C5, TONE_LENGTH}, {1, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_C5, TONE_LENGTH}, {1, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH}, {1, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_DS4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
    //Bad Apple 2
    (note[]){{NOTE_DS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_GS4, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH * 2}, {NOTE_DS5, TONE_LENGTH}, {NOTE_CS5, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH * 2}, {NOTE_DS4, TONE_LENGTH * 2}, {NOTE_AS4, TONE_LENGTH}, {NOTE_GS4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_DS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_GS4, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH * 2}, {NOTE_GS4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_DS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_DS4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_DS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_GS4, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH * 2}, {NOTE_DS5, TONE_LENGTH}, {NOTE_CS5, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH * 2}, {NOTE_DS4, TONE_LENGTH * 2}, {NOTE_AS4, TONE_LENGTH}, {NOTE_GS4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_DS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_GS4, TONE_LENGTH}, {NOTE_AS4, TONE_LENGTH * 2}, {NOTE_GS4, TONE_LENGTH}, {NOTE_FS4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH * 2}, {NOTE_FS4, TONE_LENGTH * 2}, {NOTE_GS4, TONE_LENGTH * 2}, {NOTE_AS4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
};


#else
uint16_t bpms[9] = {150, 150, 150, 150, 150, 150, 150 ,150, 150};

note* tunes[9] =
{
(note[]){{NOTE_C4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_A4, TONE_LENGTH}, {NOTE_A4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
(note[]){{NOTE_C4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH * 2}, {NOTE_G4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
(note[]){{NOTE_E4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH * 3 / 2}, {NOTE_D4, TONE_LENGTH / 2}, {NOTE_D4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
(note[]){{NOTE_G4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH / 2}, {NOTE_FS4, TONE_LENGTH / 2}, {NOTE_G4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH}, {0, TONE_LENGTH}},
(note[]){{NOTE_C4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH / 2}, {NOTE_E4, TONE_LENGTH / 2}, {NOTE_C4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH}, {0, TONE_LENGTH}},
(note[]){{NOTE_G4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_A4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
(note[]){{NOTE_C4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_A4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH * 2}, {NOTE_C4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
(note[]){{NOTE_A4, TONE_LENGTH}, {NOTE_A4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_A4, TONE_LENGTH}, {NOTE_A4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
(note[]){{NOTE_C4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH * 2}, {NOTE_G4, TONE_LENGTH * 2}, {NOTE_C4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH * 2}, {NOTE_G4, TONE_LENGTH * 2}, {NOTE_C5, TONE_LENGTH}, {NOTE_B4, TONE_LENGTH}, {NOTE_A4, TONE_LENGTH}, {NOTE_G4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH * 2}, {NOTE_A4, TONE_LENGTH * 2}, {NOTE_G4, TONE_LENGTH}, {NOTE_F4, TONE_LENGTH}, {NOTE_E4, TONE_LENGTH}, {NOTE_D4, TONE_LENGTH}, {NOTE_C4, TONE_LENGTH * 2}, {NOTE_C4, TONE_LENGTH * 2}, {0, TONE_LENGTH}},
};

#endif

uint8_t tune_nr = 1;
unsigned long last_tone_start = 0;
uint16_t tone_length = 400;
uint8_t tune_position = 0;

bool is_free_pixel(uint16_t pixel) {
    return pixel != selected_tile && table[highByte(pixel)][lowByte(pixel)] != CLEAR;
}

void refresh_screen() {
    for (int row = 0; row < SIZE; row++) {
        digitalWrite(rows[row], HIGH);
        PORTD = screen[row];
        delayMicroseconds(10);
        PORTD = CLEAR;
        digitalWrite(rows[row], LOW);
    }
}

struct note get_note_from_flash(const note* tune, int index) {
    note temp;
    memcpy_P(&temp, &tune[index], sizeof(note));
    return temp;
}

void play_tune() {
    if (playing_tune) {
        unsigned long time = millis();
        if (time - last_tone_start > tone_length) {
            note t = tunes[tune_nr][tune_position];
            tune_position++;
            if (t.freq == 0) {
                noTone(SND);
                playing_tune = false;
                tune_position = 0;
                return;
            } else {
                tone_length = 60 * t.length / bpms[tune_nr];
                tone(SND, t.freq, tone_length * 7 / 8);
                last_tone_start = time;
            }

        }
    }
}

uint8_t get_direction() {
    uint8_t direction = 0;
    int analog_value = analogRead(VRX);
    if (analog_value < LOW_THRESHOLD) {
        direction |= LEFT;
    }
    if (analog_value > HIGH_THRESHOLD) {
        direction |= RIGHT;
    }
    analog_value = analogRead(VRY);
    if (analog_value < LOW_THRESHOLD) {
        direction |= UP;
    }
    if (analog_value > HIGH_THRESHOLD) {
        direction |= DOWN;
    }
    return direction;
}

void randomize_table() {
    uint8_t t[TUNE_COUNT];
    for (int i = 0; i < TUNE_COUNT; i++) {
        t[i] = (SIZE / DIV) * (SIZE / DIV) / TUNE_COUNT;
    }
    for (int i = 0; i < SIZE / DIV; i++) {
        for (int j = 0; j < SIZE / DIV; j++) {
            int a;
            do {
                a = random(TUNE_COUNT);
            } while(t[a] == 0);
            t[a]--;
            table[i][j] = a;
        }
    }
}

void calculate_screen() {
    static int count = 0;
    count ++;
    if (count > 20) {
        count = 0;
    }
    screen[0] = CLEAR;
    screen[1] = CLEAR;
    screen[2] = CLEAR;
    screen[3] = CLEAR;
    screen[4] = CLEAR;
    screen[5] = CLEAR;
    screen[6] = CLEAR;
    screen[7] = CLEAR;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (table[i / DIV][j / DIV] == CLEAR) {
                screen[i] |= (1 << j);
            } else {
                screen[i] &= ~(1 << j);
            }
        }
    }
    if (count > 10) {
        for (int i = 0; i < DIV; i++) {
            for (int j = 0; j < DIV; j++) {
                screen[highByte(player) * DIV + j] |= (1 << (lowByte(player) * DIV + i));
                if (selected) {
                    screen[highByte(selected_tile) * DIV + j] |= (1 << (lowByte(selected_tile * DIV + i)));
                }
            }
        }
    } else {
        for (int i = 0; i < DIV; i++) {
            for (int j = 0; j < DIV; j++) {
                screen[highByte(player) * DIV + j] &= ~(1 << (lowByte(player) * DIV + i));
                if (selected) {
                    screen[highByte(selected_tile) * DIV + j] &= ~(1 << (lowByte(selected_tile) * DIV +i));
                }
            }
        }
    }
}

void setup() {
    for (int i = 0; i < SIZE; i++) {
        pinMode(rows[i], OUTPUT);
        digitalWrite(rows[i], LOW);
    }
    pinMode(VRX, INPUT);
    pinMode(VRY, INPUT);
    pinMode(SW, INPUT_PULLUP);
    DDRD = 0xFF;
    PORTD = CLEAR;
    unsigned long count = 0;
    while(digitalRead(SW)) {
        delay(10);
        count++;
    }
    while(!digitalRead(SW)) {
        delay(10);
        count++;
    }
    randomSeed(count);
    //randomSeed(count);
    //randomSeed(1234);
    randomize_table();
    
}

void loop() {
    refresh_screen();
    play_tune();
    static int count = 0;
    static int cooldown = COOLDOWN_LENGTH;
    static bool pressed = false;
    static bool moving = false;
    static uint8_t direction = 0;
    
    count++;
    if (cooldown > 0) {
        cooldown--;
    }
    if (count % 256 == 0) {
        calculate_screen();
    }
    direction = get_direction();
    if (direction == 0) {
        moving = false;
    }
    if (digitalRead(SW)) {
        pressed = false;
    }
    // get direction from input

    if (cooldown == 0 && !pressed && !digitalRead(SW)) {
        cooldown = COOLDOWN_LENGTH;
        pressed = true;
        if (is_free_pixel(player)) {
            int prev_t_nr = tune_nr;
            tune_nr = table [highByte(player)][lowByte(player)];
            if (prev_t_nr != tune_nr) {
                tune_position = 0;
            }
            playing_tune = true;
            if (selected) {
                if (table[highByte(player)][lowByte(player)] == table[highByte(selected_tile)][lowByte(selected_tile)]) {
                    table[highByte(player)][lowByte(player)] = CLEAR;
                    table[highByte(selected_tile)][lowByte(selected_tile)] = CLEAR;
                }
                selected_tile = 0xFFFF;
            } else {
                selected_tile = player;
            }
            selected = !selected;
        }
    }
    if (cooldown == 0 && moving == false) {
        cooldown = COOLDOWN_LENGTH;
        // move & update screeen
        if (direction != 0) {
            moving = true;
            if ((direction & UP) != 0 && highByte(player) != 0) {
                player = player - 0x0100;
            }
            if ((direction & DOWN) != 0 && highByte(player) != SIZE / DIV - 1) {
                player = player + 0x0100;
            }
            if ((direction & LEFT) != 0 && lowByte(player) != 0) {
                player = player - 1;
            }
            if ((direction & RIGHT) != 0 && lowByte(player) != SIZE / DIV - 1) {
                player = player + 1;
            }
        }
    }
    bool win = true;
    for (int i = 0; i < SIZE / DIV; i ++) {
        for (int j = 0; j < SIZE / DIV; j++) {
            if (table[i][j] != CLEAR) {
                win = false;
                goto outer;
            }
        }
    }
outer:
    if (win) {
        randomize_table();
        tune_nr = 8;
        tune_position = 0;
        playing_tune = true;
    }
}
