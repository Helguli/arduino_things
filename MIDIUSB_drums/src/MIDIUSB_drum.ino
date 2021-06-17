#include "MIDIUSB.h"
#include "DueTimer.h"
#include "U8g2lib.h"
#include "variant.h"
#include "wiring_analog.h"
#include "wiring_digital.h"
#include <stdint.h>

#define NUM_OF_ALL_DRUMS 33
#define NUM_OF_ANALOG_DRUMS 10
#define NUM_OF_DIGITAL_DRUMS 5
#define PIEZO_AD_THRESHOLD 300
#define PIEZO_AD_THRESHOLD_LOW 30
#define PHOTORESISTOR_DIFFERENCE 70
#define PHOTORESISTOR_DIFFERENCE_LOW 40
#define PRESSURE_DIFFERENCE 100
#define PRESSURE_DIFFERENCE_LOW 50
#define DELAY 1000
#define BUTTON_PIN_0 A10
#define BUTTON_PIN_1 A11
#define EVENT_DELAY 50
#define SWITCH_SAMPLING_TIME 10000
#define PIEZO_SAMPLING_TIME 1000
#define BUFFER_MAX_SIZE 12
#define TEMPO_SCALE 15000000
#define NORMAL_VELOCITY 64
#define FIRST_LINE 7
#define SECOND_LINE 16
#define THIRD_LINE 25
#define FOURTH_LINE 34
#define FIFTH_LINE 43
#define SIXTH_LINE 52
#define LAST_LINE 66
#define MAX_PAGE 18

//int16_t ad_value;
//int16_t prev_ad_value;
uint16_t signal_detected[NUM_OF_ANALOG_DRUMS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const int16_t drum_pins[NUM_OF_ANALOG_DRUMS] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9};
const int16_t digital_drum_pins[NUM_OF_DIGITAL_DRUMS] = {44, 45, 46, 52, 53};
const uint8_t drum_notes_all[NUM_OF_ALL_DRUMS] = {36, 37, 38, 39, 42, 46, 41, 43, 45, 49, 51, 52, 54, 55, 56, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 73, 74, 75, 76, 77, 80, 81};
uint8_t drums_analog[NUM_OF_ANALOG_DRUMS] = {11, 12, 13, 18, 1, 2, 3, 4, 5, 6};
uint8_t drums_digital[NUM_OF_DIGITAL_DRUMS] = {1, 2, 3, 24, 25};
const char *drum_names_all[NUM_OF_ALL_DRUMS] = {"Kick", "Side Stick", "Snare", "Clap", "Closed Hi-Hat", "Open Hi-Hat", "Low Tom", "Mid Tom", "High Tom", "Crash", "Ride", "Chinese Cymbal", "Tambourine", "Splash", "Cowbell", "High Bongo", "Low Bongo", "Mute High Conga", "Open High Conga", "Low Conga", "High Timbale", "Low Timbale", "High Agogo", "Low Agogo", "Cabasa", "Maracas", "Short Guiro", "Long Guiro", "Claves", "High Wood Block", "Low Wood Block", "Mute Triangle", "Triangle"};
volatile int16_t ad_values[NUM_OF_ANALOG_DRUMS];
volatile int16_t prev_ad_values[NUM_OF_ANALOG_DRUMS];
volatile int8_t digital_values[NUM_OF_DIGITAL_DRUMS];
volatile int8_t prev_digital_values[NUM_OF_DIGITAL_DRUMS];
const uint16_t sw_tresholds[] = {172, 430, 567, 652, 710, 753, 785, 850, 172, 430, 567, 652};
volatile uint8_t button_no = 0;
volatile uint16_t button_pressed = 0;
const uint8_t led_pins[8] = {22, 23, 26, 27, 28, 29, 30, 31};
uint32_t led_values[NUM_OF_ALL_DRUMS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t tempo = 125;
volatile uint8_t fragment_no = 0;
uint32_t fragment_delay = 120000;
uint8_t measure_no = 0;
uint8_t selected_drum = 0;
DueTimer drum_timer = Timer1;
midiEventPacket_t event_buffer[BUFFER_MAX_SIZE];
volatile size_t buffer_size = 0;
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
uint8_t page = 0;
volatile uint8_t fragment_update = 0;
uint8_t leds_merged = 0;


void write2Buffer(uint8_t header, uint8_t byte1, uint8_t byte2, uint8_t byte3)
{
    if (buffer_size == BUFFER_MAX_SIZE)
    {
        return;
    }
    event_buffer[buffer_size].header = header;
    event_buffer[buffer_size].byte1 = byte1;
    event_buffer[buffer_size].byte2 = byte2;
    event_buffer[buffer_size].byte3 = byte3;
    buffer_size++;
}

void noteOn(byte pitch, byte velocity)
{
    write2Buffer(0x09, 0x99, pitch, velocity);
}

void noteOff(byte pitch, byte velocity)
{
    write2Buffer(0x08, 0x89, pitch, velocity);
}

size_t sendNotes(midiEventPacket_t events[], size_t size)
{
	uint8_t data[4 * size];
    for (unsigned int i = 0; i < size; i++)
    {
	    data[0 + i * 4] = events[i].header;
	    data[1 + i * 4] = events[i].byte1;
	    data[2 + i * 4] = events[i].byte2;
	    data[3 + i * 4] = events[i].byte3;
    }
	return MidiUSB.write(data, 4 * size);
}

// Interrupt for sampling
void digitalSampling()
{
    int sensor_value = analogRead(BUTTON_PIN_0);
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        if (sensor_value < sw_tresholds[i])
        {
            if (button_pressed < 3)
            {
                button_no = i + 1;
            }
            button_pressed++;
            break;
        }
    }
    if (i == 8)
    {
        sensor_value = analogRead(BUTTON_PIN_1);
        for (; i < 12; i++)
        {
            if (sensor_value < sw_tresholds[i])
            {
                if (button_pressed < 3)
                {
                    button_no = i + 1;
                }
                button_pressed++;
                break;
            }
        }
    }
    if (i == 12)
    {
        button_no = 0;
        button_pressed = 0;
    }
    for (i = 0; i < NUM_OF_DIGITAL_DRUMS; i++)
    {
        int8_t digital_value = digitalRead(digital_drum_pins[i]);
        prev_digital_values[i] = digital_values[i];
        if (digital_value != digital_values[i])
        {
            digital_values[i] = digital_value;
            if (digital_value == HIGH)
            {
                noteOn(drum_notes_all[drums_digital[i]], NORMAL_VELOCITY);
            }
        }
    }
}

// Interrupt for sampling
void adSampling()
{
    for (int i = 0; i < NUM_OF_ANALOG_DRUMS; i++) 
    {
        int16_t prev_ad_value = ad_values[i];
        int16_t ad_value = analogRead(drum_pins[i]);
        ad_values[i] = ad_value;
        if (i < 4)
        {
            prev_ad_values[i] = prev_ad_value;
            if (ad_value > PIEZO_AD_THRESHOLD && prev_ad_value <= ad_value && !signal_detected[i])
            {
                signal_detected[i]++;
            }
            if (signal_detected[i] == 1 && prev_ad_value > ad_value)
            {
                uint8_t velocity = prev_ad_value >> 3;
                noteOn(drum_notes_all[drums_analog[i]], velocity);
                signal_detected[i]++;
            }
            if (signal_detected[i] && ad_value < PIEZO_AD_THRESHOLD_LOW)
            {
                signal_detected[i] = 0;
            }
        }
        if (i >= 4 && i < 7)
        {
            if (ad_value < (prev_ad_values[i] - PHOTORESISTOR_DIFFERENCE) && prev_ad_value >= ad_value && !signal_detected[i])
            {
                signal_detected[i]++;
            }
            if (signal_detected[i] == 1 && prev_ad_value < ad_value)
            {
                uint8_t velocity = 64 + (prev_ad_values[i] - PHOTORESISTOR_DIFFERENCE - prev_ad_value);
                noteOn(drum_notes_all[drums_analog[i]], velocity);
                signal_detected[i]++;
            }
            if (signal_detected[i] && ad_value > (prev_ad_values[i] - PHOTORESISTOR_DIFFERENCE_LOW))
            {
                signal_detected[i] = 0;
            }
        }
        if (i >= 7)
        {
            if (ad_value < (prev_ad_values[i] - PRESSURE_DIFFERENCE) && prev_ad_value >= ad_value && !signal_detected[i])
            {
                signal_detected[i]++;
            }
            if (signal_detected[i] == 1 && prev_ad_value < ad_value)
            {
                uint8_t velocity = 64 + (prev_ad_values[i] - PRESSURE_DIFFERENCE - prev_ad_value);
                noteOn(drum_notes_all[drums_analog[i]], velocity);
                signal_detected[i]++;
            }
            if (signal_detected[i] && ad_value > (prev_ad_values[i] - PRESSURE_DIFFERENCE_LOW))
            {
                signal_detected[i] = 0;
            }
        }
    }
}

// Interrupt for playing notes
void playNote()
{
    for (int i = 0; i < NUM_OF_ALL_DRUMS; i++)
    {
        if (led_values[i] & (1 << fragment_no))
        {
            noteOn(drum_notes_all[i], NORMAL_VELOCITY);
        }
    }
    fragment_no++;
    fragment_no %= 32;
    fragment_update = 1;
}

void updateLeds()
{
    if (measure_no < 4)
    {
        for (int i = 0; i < 8; i++)
        {
            digitalWrite(led_pins[i], (led_values[selected_drum] & (1 << (i + 8 * measure_no))) >> (i + 8 * measure_no));
        }
    }
    else
    {
        leds_merged = led_values[selected_drum] & (led_values[selected_drum] >> 8) & (led_values[selected_drum] >> 16) & (led_values[selected_drum] >> 24);
        for (int i = 0; i < 8; i++)
        {
            digitalWrite(led_pins[i], leds_merged & (1 << i));
        }
    }
}

void changeInstrument(uint8_t up)
{
    if (up)
    {
        selected_drum++;
    }
    else
    {
        selected_drum += NUM_OF_ALL_DRUMS - 1;
    }
    selected_drum %= NUM_OF_ALL_DRUMS;
    updateLeds();
}

void changeAnalogInstrument(uint8_t up, uint8_t drum_number)
{
    if (up)
    {
        drums_analog[drum_number]++;
    }
    else
    {
        drums_analog[drum_number] += NUM_OF_ALL_DRUMS - 1;
    }
    drums_analog[drum_number] %= NUM_OF_ALL_DRUMS;
}

void changeDigitalInstrument(uint8_t up, uint8_t drum_number)
{
    if (up)
    {
        drums_digital[drum_number]++;
    }
    else
    {
        drums_digital[drum_number] += NUM_OF_ALL_DRUMS - 1;
    }
    drums_digital[drum_number] %= NUM_OF_ALL_DRUMS;
}

void changeMeasure(uint8_t up)
{
    if (up)
    {
        measure_no++;
    }
    else
    {
        measure_no += 4;
    }
    measure_no %= 5;
    updateLeds();
}

void changeTempo(uint8_t up)
{
    if (up)
    {
        tempo++;
    }
    else
    {
        tempo--;
    }
    fragment_delay = TEMPO_SCALE / tempo;
    drum_timer.setPeriod(fragment_delay).start();
}

void draw()
{
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.setCursor(0, FIRST_LINE);
    u8g2.print(tempo);
    u8g2.setCursor(20, FIRST_LINE);
    u8g2.print(drum_names_all[selected_drum]);
    u8g2.setCursor(92, FIRST_LINE);
    u8g2.print("Beat: ");
    u8g2.print(((fragment_no >> 2) + 1));
    u8g2.setCursor(0, SECOND_LINE);
    switch (page)
    {
        case 0:
            u8g2.print("# Drum: <");
            u8g2.print(drum_names_all[selected_drum]);
            u8g2.print(">");
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Measure: ");
            if (measure_no < 4)
            {
                u8g2.print(measure_no + 1);
            }
            else
            {
                u8g2.print("all");
            }
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("Tempo: ");
            u8g2.print(tempo);
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 1: ");
            u8g2.print(drum_names_all[drums_analog[0]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 2: ");
            u8g2.print(drum_names_all[drums_analog[1]]);
            break;
        case 1:
            u8g2.print("Drum: ");
            u8g2.print(drum_names_all[selected_drum]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("# Measure: <");
            if (measure_no < 4)
            {
                u8g2.print(measure_no + 1);
            }
            else
            {
                u8g2.print("all");
            }
            u8g2.print(">");
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("Tempo: ");
            u8g2.print(tempo);
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 1: ");
            u8g2.print(drum_names_all[drums_analog[0]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 2: ");
            u8g2.print(drum_names_all[drums_analog[1]]);
            break;
        case 2:
            u8g2.print("Drum: ");
            u8g2.print(drum_names_all[selected_drum]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Measure: ");
            if (measure_no < 4)
            {
                u8g2.print(measure_no + 1);
            }
            else
            {
                u8g2.print("all");
            }
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Tempo: <");
            u8g2.print(tempo);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 1: ");
            u8g2.print(drum_names_all[drums_analog[0]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 2: ");
            u8g2.print(drum_names_all[drums_analog[1]]);
            break;
        case 3:
            u8g2.print("Measure: ");
            if (measure_no < 4)
            {
                u8g2.print(measure_no + 1);
            }
            else
            {
                u8g2.print("all");
            }
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Tempo: ");
            u8g2.print(tempo);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 1: <");
            u8g2.print(drum_names_all[drums_analog[0]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 2: ");
            u8g2.print(drum_names_all[drums_analog[1]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 3: ");
            u8g2.print(drum_names_all[drums_analog[2]]);
            break;
        case 4:
            u8g2.print("Tempo: ");
            u8g2.print(tempo);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 1: ");
            u8g2.print(drum_names_all[drums_analog[0]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 2: <");
            u8g2.print(drum_names_all[drums_analog[1]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 3: ");
            u8g2.print(drum_names_all[drums_analog[2]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 4: ");
            u8g2.print(drum_names_all[drums_analog[3]]);
            break;
        case 5:
            u8g2.print("Drum 1: ");
            u8g2.print(drum_names_all[drums_analog[0]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 2: ");
            u8g2.print(drum_names_all[drums_analog[1]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 3: < ");
            u8g2.print(drum_names_all[drums_analog[2]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 4: ");
            u8g2.print(drum_names_all[drums_analog[3]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 5: ");
            u8g2.print(drum_names_all[drums_analog[4]]);
            break;
        case 6:
            u8g2.print("Drum 2: ");
            u8g2.print(drum_names_all[drums_analog[1]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 3: ");
            u8g2.print(drum_names_all[drums_analog[2]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 4: < ");
            u8g2.print(drum_names_all[drums_analog[3]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 5: ");
            u8g2.print(drum_names_all[drums_analog[4]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 6: ");
            u8g2.print(drum_names_all[drums_analog[5]]);
            break;
        case 7:
            u8g2.print("Drum 3: ");
            u8g2.print(drum_names_all[drums_analog[2]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 4: ");
            u8g2.print(drum_names_all[drums_analog[3]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 5: < ");
            u8g2.print(drum_names_all[drums_analog[4]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 6: ");
            u8g2.print(drum_names_all[drums_analog[5]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 7: ");
            u8g2.print(drum_names_all[drums_analog[6]]);
           break;
        case 8:
            u8g2.print("Drum 4: ");
            u8g2.print(drum_names_all[drums_analog[3]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 5: ");
            u8g2.print(drum_names_all[drums_analog[4]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 6: < ");
            u8g2.print(drum_names_all[drums_analog[5]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 7: ");
            u8g2.print(drum_names_all[drums_analog[6]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 8: ");
            u8g2.print(drum_names_all[drums_analog[7]]);
           break;
        case 9:
            u8g2.print("Drum 5: ");
            u8g2.print(drum_names_all[drums_analog[4]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 6: ");
            u8g2.print(drum_names_all[drums_analog[5]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 7: < ");
            u8g2.print(drum_names_all[drums_analog[6]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 8: ");
            u8g2.print(drum_names_all[drums_analog[7]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 9: ");
            u8g2.print(drum_names_all[drums_analog[8]]);
           break;
        case 10:
            u8g2.print("Drum 6: ");
            u8g2.print(drum_names_all[drums_analog[5]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 7: ");
            u8g2.print(drum_names_all[drums_analog[6]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 8: < ");
            u8g2.print(drum_names_all[drums_analog[7]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 9: ");
            u8g2.print(drum_names_all[drums_analog[8]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 10: ");
            u8g2.print(drum_names_all[drums_analog[9]]);
           break;
        case 11:
            u8g2.print("Drum 7: ");
            u8g2.print(drum_names_all[drums_analog[6]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 8: ");
            u8g2.print(drum_names_all[drums_analog[7]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 9: < ");
            u8g2.print(drum_names_all[drums_analog[8]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 10: ");
            u8g2.print(drum_names_all[drums_analog[9]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 11: ");
            u8g2.print(drum_names_all[drums_digital[0]]);
           break;
        case 12:
            u8g2.print("Drum 8: ");
            u8g2.print(drum_names_all[drums_analog[7]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 9: ");
            u8g2.print(drum_names_all[drums_analog[8]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 10: < ");
            u8g2.print(drum_names_all[drums_analog[9]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 11: ");
            u8g2.print(drum_names_all[drums_digital[0]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 12: ");
            u8g2.print(drum_names_all[drums_digital[1]]);
            break;
        case 13:
            u8g2.print("Drum 9: ");
            u8g2.print(drum_names_all[drums_analog[8]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 10: ");
            u8g2.print(drum_names_all[drums_analog[9]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 11: < ");
            u8g2.print(drum_names_all[drums_digital[0]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 12: ");
            u8g2.print(drum_names_all[drums_digital[1]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 13: ");
            u8g2.print(drum_names_all[drums_digital[2]]);
           break;
        case 14:
            u8g2.print("Drum 10: ");
            u8g2.print(drum_names_all[drums_analog[9]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 11: ");
            u8g2.print(drum_names_all[drums_digital[0]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 12: < ");
            u8g2.print(drum_names_all[drums_digital[1]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 13: ");
            u8g2.print(drum_names_all[drums_digital[2]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 14: ");
            u8g2.print(drum_names_all[drums_digital[3]]);
           break;
        case 15:
            u8g2.print("Drum 11: ");
            u8g2.print(drum_names_all[drums_digital[0]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 12: ");
            u8g2.print(drum_names_all[drums_digital[1]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("# Drum 13: < ");
            u8g2.print(drum_names_all[drums_digital[2]]);
            u8g2.print(">");
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 14: ");
            u8g2.print(drum_names_all[drums_digital[3]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 15: ");
            u8g2.print(drum_names_all[drums_digital[4]]);
            break;
        case 16:
            u8g2.print("Drum 11: ");
            u8g2.print(drum_names_all[drums_digital[0]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 12: ");
            u8g2.print(drum_names_all[drums_digital[1]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("Drum 13: ");
            u8g2.print(drum_names_all[drums_digital[2]]);
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("# Drum 14: < ");
            u8g2.print(drum_names_all[drums_digital[3]]);
            u8g2.print(">");
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 15: ");
            u8g2.print(drum_names_all[drums_digital[4]]);
           break;
        case 17:
            u8g2.print("Drum 11: ");
            u8g2.print(drum_names_all[drums_digital[0]]);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 12: ");
            u8g2.print(drum_names_all[drums_digital[1]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("Drum 13: ");
            u8g2.print(drum_names_all[drums_digital[2]]);
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 14: ");
            u8g2.print(drum_names_all[drums_digital[3]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("# Drum 15: < ");
            u8g2.print(drum_names_all[drums_digital[4]]);
            u8g2.print(">");
           break;

    }
    u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
    u8g2.setCursor(5, LAST_LINE);
    u8g2.print("");
    u8g2.setCursor(44, LAST_LINE);
    u8g2.print("");
    u8g2.setCursor(82, LAST_LINE);
    u8g2.print("");
    u8g2.setCursor(120, LAST_LINE);
    u8g2.print("");
}

void setup()
{
    pinMode(BUTTON_PIN_0, INPUT);
    pinMode(BUTTON_PIN_1, INPUT);
    for (int i = 0; i < 8; i++)
    {
        pinMode(led_pins[i], OUTPUT);
    }
    for (int i = 0; i < NUM_OF_ANALOG_DRUMS; i++)
    {
        pinMode(drum_pins[i], INPUT);
    }
    for (int i = 0; i < NUM_OF_DIGITAL_DRUMS; i++)
    {
        pinMode(drums_digital[i], INPUT);
    }
    pinMode(52, INPUT_PULLUP);
    for (int i = 4; i < 10; i++)
    {
        int ad_value = 0;
        for (int j = 0; j < 8; j++)
        {
            ad_value += analogRead(drum_pins[i]);
        }
        prev_ad_values[i] = ad_value >> 3;
    }
    Serial.begin(115200);
    MidiUSB.flush();
    drum_timer.attachInterrupt(playNote).start(fragment_delay);
    DueTimer digital_timer = Timer.getAvailable();
    digital_timer.attachInterrupt(digitalSampling).start(SWITCH_SAMPLING_TIME);
    DueTimer ad_timer = Timer.getAvailable();
    ad_timer.attachInterrupt(adSampling).start(PIEZO_SAMPLING_TIME);
    u8g2.begin();
    u8g2.enableUTF8Print();
}

void loop()
{
    if (buffer_size)
    {
        if (sendNotes(event_buffer, buffer_size))
        {
            buffer_size = 0;
        }
    }
    if (fragment_update && !buffer_size)
    {
        fragment_update = 0;
            u8g2.firstPage();
            do
            {
                draw();
            } while (u8g2.nextPage());
    }
    if (button_no && button_pressed >= 5)
    {
        // TODO handle button events
        if (button_no < 9)
        {
            uint8_t new_value = 0;
            if (measure_no < 4)
            {
                uint8_t place = button_no - 1 + 8 * measure_no;
                led_values[selected_drum] ^= (1 << place);
                new_value = (led_values[selected_drum] & (1 << place)) >> place;
            }
            else
            {
                leds_merged ^= 1 << (button_no - 1);
                new_value = leds_merged & (1 << (button_no - 1));
                if (new_value)
                {
                    led_values[selected_drum] |= (1 << (button_no - 1)) | (1 << (button_no + 7)) | (1 << (button_no + 15)) | (1 << (button_no + 23));
                }
                else
                {
                    led_values[selected_drum] &= ~((1 << (button_no - 1)) | (1 << (button_no + 7 )) | (1 << (button_no + 15)) | (1 << (button_no + 23)));
                }
            }
            digitalWrite(led_pins[button_no - 1], new_value);
        }
        if (button_no == 9)
        {
            switch (page)
            {
                case 0:
                    changeInstrument(0);
                    break;
                case 1:
                    changeMeasure(0);
                    break;
                case 2:
                    changeTempo(0);
                    break;
                case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
                    changeAnalogInstrument(0, page - 3);
                    break;
                case 13: case 14: case 15: case 16: case 17:
                    changeDigitalInstrument(0, page - 13);
                    break;
            }
        }
        else if (button_no == 10)
        {
            switch (page)
            {
                case 0:
                    changeInstrument(1);
                    break;
                case 1:
                    changeMeasure(1);
                    break;
                case 2:
                    changeTempo(1);
                    break;
                case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
                    changeAnalogInstrument(1, page - 3);
                    break;
                case 13: case 14: case 15: case 16: case 17:
                    changeDigitalInstrument(1, page - 13);
                    break;
            }
        }
        else if (button_no == 11)
        {
            page = (page + MAX_PAGE - 1) % MAX_PAGE;
        }
        else if (button_no == 12)
        {
            page = (page + 1) % MAX_PAGE;
        }
        button_no = 0;
    }
}
