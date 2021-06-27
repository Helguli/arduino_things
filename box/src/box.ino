#include "HardwareSerial.h"
#include <Arduino.h>
#include <Servo.h>

#define SOUND_SENSOR_PIN 12
#define SERVO_PIN 9
#define RED_LED_PIN 5
#define GREEN_LED_PIN 6
#define OPEN 120
#define CLOSE 0

Servo servo;
int previous_value;
bool is_open = false;

void setup()
{
    servo.attach(SERVO_PIN);
    servo.write(CLOSE);
    pinMode(SOUND_SENSOR_PIN, INPUT);
    previous_value = digitalRead(SOUND_SENSOR_PIN);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
}

void loop()
{
    int value = digitalRead(SOUND_SENSOR_PIN);
    if (value == HIGH && previous_value == LOW)
    {
        is_open=!is_open;
        digitalWrite(GREEN_LED_PIN, is_open?HIGH:LOW);
        digitalWrite(RED_LED_PIN, is_open?LOW:HIGH);
        servo.write(is_open?OPEN:CLOSE);
        delay(500);
    }
    previous_value = value;
    delay(1);
}
