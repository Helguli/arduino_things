#include <Arduino.h>
#include "pitches.h"

#define NUM_OF_STRINGS 4
#define PLUCK 0
#define PIEZO_PIN 8

int string_pins[NUM_OF_STRINGS] = {2, 3, 4, 5};
int prev_values[NUM_OF_STRINGS];
int string_notes[NUM_OF_STRINGS] = {NOTE_C3, NOTE_D3, NOTE_E3, NOTE_G3};

void setup()
{
    pinMode(4, INPUT);
    for (int i = 0; i < NUM_OF_STRINGS; i++)
    {
        pinMode(string_pins[i], INPUT);
        prev_values[i] = digitalRead(string_pins[i]);
    }
}

void loop()
{
    for (int i = 0; i < NUM_OF_STRINGS; i++)
    {
        int value = digitalRead(string_pins[i]);
        if (value != prev_values[i])
        {
            prev_values[i] = value;
            if (value == PLUCK)
            {
                tone(PIEZO_PIN, string_notes[i], 125);
            }
        }
    }
}
