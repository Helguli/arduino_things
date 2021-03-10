#include "MIDIUSB.h"
#include "DueTimer.h"

#define NUM_OF_DRUMS 4
#define AD_THRESHOLD 200
#define DELAY 1000
#define BUTTON_PIN_0 A10
#define BUTTON_PIN_1 A11
#define LED_PIN 46
#define EVENT_DELAY 50
#define SWITCH_SAMPLING_TIME 10000

//int16_t ad_value;
//int16_t prev_ad_value;
uint16_t signal_detected[] = {0, 0, 0, 0};
int16_t drum_pins[] = {A0, A1, A2, A3};
uint8_t drum_notes[] = {36, 39, 42, 49};
int16_t ad_values[4];
int16_t prev_ad_values[4];
uint16_t sw_tresholds[] = {172, 430, 567, 652, 710, 753, 785, 850, 172, 430, 567, 652};
uint8_t button_no = 0;
uint16_t button_pressed = 0;
uint8_t led_pins[] = {22, 23, 24, 25, 26, 27, 28, 29};
uint8_t led_values[] = {0, 0, 0, 0}; //uint32_t-re kell majd változtatni
uint8_t tempo = 125;
volatile uint8_t fragment_no = 0;
uint32_t fragment_delay = 120000;
uint8_t selected_drum = 0;
volatile uint8_t drums_to_play = 0;
//DueTimer drum_timer;

void noteOn(byte channel, byte pitch, byte velocity)
{
    midiEventPacket_t noteOn = {0x09, (uint8_t) (0x90 | channel), pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity)
{
    midiEventPacket_t noteOff = {0x08, (uint8_t) (0x80 | channel), pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value)
{
    midiEventPacket_t event = {0x0B, (uint8_t) (0xB0 | channel), control, value};
    MidiUSB.sendMIDI(event);
}

void switchSample()
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

void playNote()
{
    for (int i = 0; i < NUM_OF_DRUMS; i++)
    {
        if (led_values[i] & (1 << fragment_no))
        {
            //Serial.println("Drum detected!");
            drums_to_play |= (1 << i);

        }
    }
    //Serial.println(led_values[i] & (1 << fragment_no));
    //Serial.println(fragment_no);
    //Serial.println(drums_to_play);
    fragment_no++;
    fragment_no %= 8;
}

void changeInstrument(uint8_t up)
{
    if (up)
    {
        selected_drum++;
    }
    else
    {
        selected_drum--;
    }
    selected_drum %= NUM_OF_DRUMS;
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(led_pins[i], (led_values[selected_drum] & (1 << i)) >> i);
    }
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
    DueTimer drum_timer = Timer.getAvailable();
    drum_timer.attachInterrupt(playNote).start(fragment_delay);
    DueTimer switch_timer = Timer.getAvailable();
    switch_timer.attachInterrupt(switchSample).start(SWITCH_SAMPLING_TIME);
}

void loop() 
{
    if (drums_to_play)
    {
        for (int i = 0; i < NUM_OF_DRUMS; i++)
        {
            if (drums_to_play & (1 << i))
            {
                //Serial.println("Note sent.");
                drums_to_play &= ~(1 << i);
                noteOn(9, drum_notes[i], 64);
                MidiUSB.flush();
                delayMicroseconds(EVENT_DELAY);
                //break;
            }
        }
    }
    /*
    for (int i = 0; i < NUM_OF_DRUMS; i++) 
    {
        int16_t prev_ad_value = ad_values[i];
        int16_t ad_value = analogRead(drum_pins[i]);
        ad_value = abs(ad_value);
        prev_ad_values[i] = prev_ad_value;
        ad_values[i] = ad_value;
        if (ad_value > AD_THRESHOLD && prev_ad_value <= ad_value && signal_detected[i] == 0) 
        {
            signal_detected[i] = 2000;
        }
        if (signal_detected[i] > 1500 && prev_ad_value > ad_value) 
        {
            uint8_t velocity = prev_ad_value/8;
            noteOn(9, drum_notes[i], velocity);
//            Serial.print(prev_ad_value);
//            Serial.print(":");
//            Serial.println(signal_detected[i]);
            MidiUSB.flush();
            signal_detected[i] = 1500;
        }
        //Serial.println(ad_value);
        if (signal_detected[i]) 
        {
            signal_detected[i] = signal_detected[i] - 1;
        }
    }*/
    if (button_no && button_pressed >= 5)
    {
        // TODO handle button events
        if (button_no < 9)
        {
            led_values[selected_drum] ^= (1 << (button_no - 1));
            digitalWrite(led_pins[button_no - 1], (led_values[selected_drum] & (1 << (button_no - 1))) >> (button_no - 1));
        }
        if (button_no == 9)
        {
            changeInstrument(1);
        }
        button_no = 0;
    }
//    Serial.println(ad_value);
//   delay(500);
   // noteOff(9, 39, 64);
   // MidiUSB.flush();
//    delay(1500);

  // controlChange(0, 10, 65); // Set the value of controller 10 on channel 0 to 65
}
