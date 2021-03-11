#include "MIDIUSB.h"
#include "DueTimer.h"
#include "U8g2lib.h"

#define NUM_OF_ALL_DRUMS 19
#define NUM_OF_ANALOG_DRUMS 4
#define AD_THRESHOLD 300
#define AD_THRESHOLD_LOW 30
#define DELAY 1000
#define BUTTON_PIN_0 A10
#define BUTTON_PIN_1 A11
#define LED_PIN 46
#define EVENT_DELAY 50
#define SWITCH_SAMPLING_TIME 10000
#define PIEZO_SAMPLING_TIME 1000
#define BUFFER_MAX_SIZE 8
#define TEMPO_SCALE 15000000
#define FIRST_LINE 7
#define SECOND_LINE 16
#define THIRD_LINE 25
#define FOURTH_LINE 34
#define FIFTH_LINE 43
#define SIXTH_LINE 52
#define LAST_LINE 66
#define MAX_PAGE 7


//int16_t ad_value;
//int16_t prev_ad_value;
uint16_t signal_detected[NUM_OF_ANALOG_DRUMS] = {0, 0, 0, 0};
int16_t drum_pins[NUM_OF_ANALOG_DRUMS] = {A0, A1, A2, A3};
uint8_t drum_notes_all[NUM_OF_ALL_DRUMS] = {36, 37, 38, 39, 42, 46, 41, 43, 45, 49, 51, 54, 55, 56, 60, 61, 63, 75, 81};
uint8_t drums_analog[NUM_OF_ANALOG_DRUMS] = {11, 12, 13, 18};
const char *drum_names_all[NUM_OF_ALL_DRUMS] = {"Kick", "Side Stick", "Snare", "Clap", "Closed Hi-Hat", "Open Hi-Hat", "Low Tom", "Mid Tom", "High Tom", "Crash", "Ride", "Tambourine", "Splash", "Cowbell", "High Bongo", "Low Bongo", "Conga", "Claves", "Triangle"};
int16_t ad_values[NUM_OF_ANALOG_DRUMS];
int16_t prev_ad_values[NUM_OF_ANALOG_DRUMS];
uint16_t sw_tresholds[] = {172, 430, 567, 652, 710, 753, 785, 850, 172, 430, 567, 652};
uint8_t button_no = 0;
uint16_t button_pressed = 0;
uint8_t led_pins[8] = {22, 23, 24, 25, 26, 27, 28, 29};
uint32_t led_values[NUM_OF_ALL_DRUMS] = {0, 0, 0, 0, 0, 0, 0, 0};
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
uint8_t fragment_update = 0;

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

void switchSampling()
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
}

void piezoSampling()
{
    //TODO
    for (int i = 0; i < 1/*NUM_OF_ANALOG_DRUMS*/; i++) 
    {
        int16_t prev_ad_value = ad_values[i];
        int16_t ad_value = analogRead(drum_pins[i]);
        prev_ad_values[i] = prev_ad_value;
        ad_values[i] = ad_value;
        if (ad_value > AD_THRESHOLD && prev_ad_value <= ad_value && !signal_detected[i])
        {
            signal_detected[i]++;
        }
        if (signal_detected[i] == 1 && prev_ad_value > ad_value)
        {
            uint8_t velocity = prev_ad_value >> 3;
            noteOn(drum_notes_all[drums_analog[i]], velocity);
            signal_detected[i]++;
        }
        if (signal_detected[i] && ad_value < AD_THRESHOLD_LOW)
        {
            signal_detected[i] = 0;
        }
    }
}

void playNote()
{
    for (int i = 0; i < NUM_OF_ALL_DRUMS; i++)
    {
        if (led_values[i] & (1 << fragment_no))
        {
            noteOn(drum_notes_all[i], 64);
        }
    }
    fragment_no++;
    fragment_no %= 32;
    fragment_update = 1;
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
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(led_pins[i], (led_values[selected_drum] & (1 << (i + 8 * measure_no))) >> (i + 8 * measure_no));
    }
}

void changeMeasure(uint8_t up)
{
    if (up)
    {
        measure_no++;
    }
    else
    {
        measure_no += 3;
    }
    measure_no %= 4;
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(led_pins[i], (led_values[selected_drum] & (1 << (i + 8 * measure_no))) >> (i + 8 * measure_no));
    }
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
            u8g2.print(measure_no + 1);
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
            u8g2.print(measure_no + 1);
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
            u8g2.print(measure_no + 1);
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
            u8g2.print(measure_no + 1);
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
            u8g2.print("Tempo: ");
            u8g2.print(tempo);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 1: ");
            u8g2.print(drum_names_all[drums_analog[0]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("Drum 2: ");
            u8g2.print(drum_names_all[drums_analog[1]]);
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("# Drum 3: <");
            u8g2.print(drum_names_all[drums_analog[2]]);
            u8g2.print(">");
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("Drum 4: ");
            u8g2.print(drum_names_all[drums_analog[3]]);
            break;
        case 6:
            u8g2.print("Tempo: ");
            u8g2.print(tempo);
            u8g2.setCursor(0, THIRD_LINE);
            u8g2.print("Drum 1: ");
            u8g2.print(drum_names_all[drums_analog[0]]);
            u8g2.setCursor(0, FOURTH_LINE);
            u8g2.print("Drum 2: ");
            u8g2.print(drum_names_all[drums_analog[1]]);
            u8g2.setCursor(0, FIFTH_LINE);
            u8g2.print("Drum 3: ");
            u8g2.print(drum_names_all[drums_analog[2]]);
            u8g2.setCursor(0, SIXTH_LINE);
            u8g2.print("# Drum 4: <");
            u8g2.print(drum_names_all[drums_analog[3]]);
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
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    Serial.begin(115200);
    MidiUSB.flush();
    drum_timer.attachInterrupt(playNote).start(fragment_delay);
    DueTimer switch_timer = Timer.getAvailable();
    switch_timer.attachInterrupt(switchSampling).start(SWITCH_SAMPLING_TIME);
    DueTimer piezo_timer = Timer.getAvailable();
    piezo_timer.attachInterrupt(piezoSampling).start(PIEZO_SAMPLING_TIME);
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
            uint8_t place = button_no - 1 + 8 * measure_no;
            led_values[selected_drum] ^= (1 << place);
            digitalWrite(led_pins[button_no - 1], (led_values[selected_drum] & (1 << place)) >> place);
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
