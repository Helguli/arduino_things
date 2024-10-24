#include <UTFTGLUE.h>

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH  320
#define PAD_HEIGHT    8
#define PAD_WIDTH     40


UTFTGLUE myGLCD(0x9341,A2,A1,A3,A4,A0);


union point
{
    struct {
        int16_t x;
        int16_t y;
    };
    uint32_t p;
};

point ball = {{30, 30}};
point ball_prev = {{30, 30}};
point player = {{30, SCREEN_HEIGHT - PAD_HEIGHT - 1}};
point player_prev = {{0, 0}};
point computer = {{30, 0}};
point computer_prev = {{0, 0}};
int8_t ball_speed_x = 2;
int8_t ball_speed_y = 2;

void updateScreen()
{
    //myGLCD.clrScr();
    if (computer.p != computer_prev.p)
    {
        myGLCD.setColor(0, 0, 0);
        myGLCD.fillRect(computer_prev.x, computer_prev.y, computer_prev.x + PAD_WIDTH, computer_prev.y + PAD_HEIGHT);
        myGLCD.setColor(255, 0, 0);
        myGLCD.fillRoundRect(computer.x, computer.y, computer.x + PAD_WIDTH, computer.y + PAD_HEIGHT);
    }
    if (player.p != player_prev.p)
    {
        myGLCD.setColor(0, 0, 0);
        myGLCD.fillRoundRect(player_prev.x, player_prev.y, player_prev.x + PAD_WIDTH, player_prev.y + PAD_HEIGHT);
        myGLCD.setColor(255, 0, 0);
        myGLCD.fillRoundRect(player.x, player.y, player.x + PAD_WIDTH, player.y + PAD_HEIGHT);
    }
    if (ball.p != ball_prev.p)
    {
        myGLCD.setColor(0, 0, 0);
        myGLCD.fillRect(ball_prev.x - PAD_HEIGHT / 2, ball_prev.y - PAD_HEIGHT / 2, ball_prev.x + PAD_HEIGHT / 2, ball_prev.y + PAD_HEIGHT / 2);
        myGLCD.setColor(255, 0, 0);
        myGLCD.fillCircle(ball.x, ball.y, PAD_HEIGHT / 2);
    }
}


void setup()
{
    randomSeed(analogRead(0));
    myGLCD.InitLCD();
    myGLCD.setFont(SmallFont);
    myGLCD.invertDisplay(true);
    myGLCD.clrScr();
    updateScreen();
}

void loop()
{
    player_prev = player;
    computer_prev = computer;
    ball_prev = ball;
    ball.x += ball_speed_x;
    ball.y += ball_speed_y;
    if (ball.x < PAD_HEIGHT / 2)
    {
        ball.x = PAD_HEIGHT - ball.x;
        ball_speed_x *= -1;
    } else if (ball.x > SCREEN_WIDTH - PAD_HEIGHT / 2)
    {
        ball.x = SCREEN_WIDTH * 2 - ball.x - PAD_HEIGHT;
        ball_speed_x *= -1;
    }
    if (ball.y < PAD_HEIGHT)
    {
        ball_speed_y *= -1;
    } else if (ball.y > SCREEN_HEIGHT - PAD_HEIGHT)
    {
        ball_speed_y *= -1;
    }
    updateScreen();
    delay(100);
}
