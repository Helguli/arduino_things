#include "MIDIUSB.h"

#define BASS_DRUM A0
#define SNARE_DRUM A1
#define HIGH_HAT A2
#define CRASH A3
#define NUM_OF_DRUMS 4

//int16_t ad_value;
//int16_t prev_ad_value;
uint16_t signal_detected[] = {0, 0, 0, 0};
int16_t drum_pins[] = {A0, A1, A2, A3};
uint8_t drum_notes[] = {36, 38, 42, 49};
int16_t ad_values[4];
int16_t prev_ad_values[4];


void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
}

void setup() {
    Serial.begin(115200);
    MidiUSB.flush();
}

void controlChange(byte channel, byte control, byte value) {
    midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
    MidiUSB.sendMIDI(event);
}

void loop() {
    for (int i = 0; i < NUM_OF_DRUMS; i++) {
        int16_t prev_ad_value = ad_values[i];
        int16_t ad_value = analogRead(drum_pins[i]) - 512;
        ad_value = abs(ad_value);
        prev_ad_values[i] = prev_ad_value;
        ad_values[i] = ad_value;
        if (ad_value > 50 && prev_ad_value <= ad_value && signal_detected[i] == 0) {
            signal_detected[i] = 200;
        }
        if (signal_detected[i] == 199 && prev_ad_value > ad_value) {
            noteOn(9, drum_notes[i], 64);
            Serial.println(ad_value);
            MidiUSB.flush();
        }
        Serial.println(ad_value);
        if (signal_detected[i]) {
            signal_detected[i] = signal_detected[i] - 1;
        }
    }
//    Serial.println(ad_value);
//   delay(500);
   // noteOff(9, 39, 64);
   // MidiUSB.flush();
//    delay(1500);

  // controlChange(0, 10, 65); // Set the value of controller 10 on channel 0 to 65
}
