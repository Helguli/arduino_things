#define SIZE 8
#define LEFT 0
#define UP 1
#define RIGHT 2
#define DOWN 3
#define CLEAR 0xff
#define START_SCREEN {0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xdf, 0xff};
#define START_SNAKE_HEAD 0x0505
#define START_SNAKE_TAIL 0x0605
#define SNAKE_MAX_LENGTH 32
#define START_LENGTH 1500
#define LOW_THRESHOLD 20
#define HIGH_THRESHOLD 1002
#define VRX A4
#define VRY A5
#define SW 12

/*
Snake game for Arduino with an 8x8 LED matrix and a joystick.

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
A4              -               VRX
A5              -               VRY

*/




// short led_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2, A3};
uint8_t rows[] = {8, 9, 10, 11, A0, A1, A2, A3}; //{10, 7, 6, A2, 4, A1, 12, 13};
uint8_t columns[] = {0, 1, 2, 3, 4, 5, 6, 7}; //{5, 8, 3, 9, A0, 2, 11, A3};

uint16_t count = 0;
uint16_t lenth = START_LENGTH;
uint8_t direction = UP; // default direction is up
uint8_t screen[SIZE] = START_SCREEN;;
uint16_t snake[SNAKE_MAX_LENGTH]; // stores the coordinates of the snake
uint16_t food = 0x0205; // stores the coordinates of the food
uint8_t snake_head = 1; // array index
uint8_t snake_tail = 0; // array index
uint8_t lost = 0;
uint8_t prev_direction = direction;

bool is_free_pixel(uint16_t pixel) {
    for (int i = 0; i < SNAKE_MAX_LENGTH; i++) {
        if ((snake_head > snake_tail && i >= snake_tail && i <= snake_head) || (snake_head < snake_tail && ( i >= snake_tail || i <= snake_head))) {
            if (snake[i] == pixel) {
                return false;
            }
        }
    }
    return true;
}

void refresh_screen() {
    for (int row = 0; row < SIZE; row++) {
        digitalWrite(rows[row], HIGH);
        PORTD = screen[row];
        PORTD = CLEAR;
        digitalWrite(rows[row], LOW);
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
    food = ((random(8)) << 8) + random(8);
    screen[highByte(food)] &= ~(1 << lowByte(food)); 
   // init();
    snake[snake_head] = START_SNAKE_HEAD;
    snake[snake_tail] = START_SNAKE_TAIL;
}

void loop() {
    refresh_screen();
    count++;
    // get direction from input
    int analog_value = analogRead(VRX);
    if (analog_value < LOW_THRESHOLD) {
        direction = LEFT;
    }
    if (analog_value > HIGH_THRESHOLD) {
        direction = RIGHT;
    }
    analog_value = analogRead(VRY);
    if (analog_value < LOW_THRESHOLD) {
        direction = UP;
    }
    if (analog_value > HIGH_THRESHOLD) {
        direction = DOWN;
    }
    if ((direction + prev_direction) % 2 == 0) {
        direction = prev_direction;
    }

    // oke, most mar minden esetben jo parameterekkel kezdi ujra a jatekot gombnyomaskor
    if (!digitalRead(SW)) {
        while(!digitalRead(SW)) {
            delay(10);
        }
        screen[0] = CLEAR;
        screen[1] = CLEAR;
        screen[2] = CLEAR;
        screen[3] = CLEAR;
        screen[4] = CLEAR;
        screen[5] = 0xdf;
        screen[6] = 0xdf;
        screen[7] = CLEAR;
        snake_head = 1;
        snake_tail = 0;
        snake[snake_head] = START_SNAKE_HEAD;
        snake[snake_tail] = START_SNAKE_TAIL;
        food = ((random(8)) << 8) + random(8);
        screen[highByte(food)] &= ~(1 << lowByte(food));
        lost = 0;
        lenth = START_LENGTH;
        direction = UP;
        prev_direction = UP;
    }
    if (count == lenth) {
        prev_direction = direction;
        count = 0;
        if (lost) {
            return;
        }
        // move & update screeen
        uint8_t next_head = (snake_head+1)%SNAKE_MAX_LENGTH;
        uint16_t previous_head_coordinates = snake[snake_head];
        uint16_t next_head_coordinates = previous_head_coordinates;
        if (direction == UP && highByte(snake[snake_head]) != 0) {
            next_head_coordinates = snake[snake_head]-0x0100;
        } else if (direction == DOWN && highByte(snake[snake_head]) != 7) {
            next_head_coordinates = snake[snake_head]+0x0100;
        } else if (direction == LEFT && lowByte(snake[snake_head]) != 0) {
            next_head_coordinates = snake[snake_head]-1;
        } else if (direction == RIGHT && lowByte(snake[snake_head]) != 7) {
            next_head_coordinates = snake[snake_head]+1;
        } else {
            lost = 1;
            return;
        }
        if (!is_free_pixel(next_head_coordinates)) {
            lost = 1;
            return;
        }
        screen[highByte(next_head_coordinates)] &= ~(1 << lowByte(next_head_coordinates));
        snake_head = next_head;
        snake[snake_head] = next_head_coordinates;
        if (next_head_coordinates == food) {
            while (!is_free_pixel(food)) {
                food = ((rand()%8) << 8) + rand()%8;
            }
            // kaja talalaskor gyorsul
            lenth -= 15;
            screen[highByte(food)] &= ~(1 << lowByte(food));
        } else {
            screen[highByte(snake[snake_tail])] |= (1 << lowByte(snake[snake_tail]));
            snake_tail = (snake_tail+1)%SNAKE_MAX_LENGTH;
        }
    }
}
