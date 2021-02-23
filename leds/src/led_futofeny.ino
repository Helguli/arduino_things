#define num_of_leds 9
#define button_pin_num 12 // controls number of active LEDs
#define button_pin_direction 11 // controls the direction
#define analog_pin_duration A0 // controls the duration
const short led_pins[] = {2,3,4,5,6,7,8,9,10};

int led_states[] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};

unsigned long previous_millis = 0;
unsigned long button_press_start = 0;
unsigned long button_press_start2 = 0;
unsigned short input_value = 100;

unsigned short interval = 100; // duration of one state in ms

byte state = 0;
byte num_of_active_leds = 1;
char order = 1;

void setup() {
  for (int i = 0; i < num_of_leds; i++) {
    pinMode(led_pins[i], OUTPUT);
  }
  pinMode(button_pin_num, INPUT_PULLUP);
  pinMode(button_pin_direction, INPUT_PULLUP);
}

void loop() {
  unsigned long current_millis = millis();
  input_value = analogRead(analog_pin_duration);
  interval = input_value+30;
  if (digitalRead(button_pin_num) == HIGH) {
    button_press_start = current_millis;
  } else {
    if (current_millis - button_press_start >= 60 && current_millis - button_press_start <= 70) {
      button_press_start = 0;
      num_of_active_leds = (num_of_active_leds+1)%num_of_leds;
    }
  }
  if (digitalRead(button_pin_direction) == HIGH) {
    button_press_start2 = current_millis;
  } else {
    if (current_millis - button_press_start2 >= 60 && current_millis - button_press_start2 <= 70) {
      button_press_start2 = 0;
      order = -1*order;
    }
  }

  if (current_millis - previous_millis >= interval) {
    previous_millis = current_millis;
    
    state=(state+order+num_of_leds)%num_of_leds;

    led_states[state] = HIGH;
    led_states[(state+num_of_leds-num_of_active_leds*order)%num_of_leds] = LOW;
    
    for (int i = 0; i < num_of_leds; i++) {
      digitalWrite(led_pins[i], led_states[i]);
    }
  }
}
