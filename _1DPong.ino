#include "FastLED.h"
#include "LEDMatrix.h"
#include "LEDText.h"
#include "FontMatrise.h"

#include <Bounce.h> //Used for button de-bouncing

// How many leds are in the strip?
#define NUM_LEDS 147

// Data pin that led data will be written out over
#define DATA_PIN 9
#define SCOREBOARD1_DATA_PIN 8
#define SCOREBOARD2_DATA_PIN 7

// Define button Pins
#define BUTTON1 6
#define BUTTON2 5
#define FAST_BUTTON 4

// Define scoreboard matrix parameters
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define MATRIX_TYPE VERTICAL_MATRIX

// How often (# times loop() runs) the LEDs and scoreboards are "re-drawn".
// Given the time required for those updates, this
// can affect the overall update rate of the game.
// Generally, a higher value means faster overall speed
// as the LED updates happen less often
#define UPDATE_RATE 15

// Starting delay between ticks (shorter=faster ball)
// Once the main loop loops this many times:
// 1) The ball position will be updated
// 2) The dirrection will change if hit
// 3) The speed will increase if hit
// 4) Scoring logic will start if not hit
#define START_DELAY 100

// Max Speed the ball will travel. Shorter = faster
#define MAX_SPEED 5

// Divide paddlepower at time of hit by this, subtract result from
// previous ballspeed to get new ballspeed
#define HIT_SPEED_FACTOR 10

// 255-0 Smaller for longer time paddle is "up" / quicker recharge
#define PADDLE_RECHARGE 150

// rate at which the paddle power decreases (higher = slower)
#define PADDLE_RATE 7

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

//Scoreboard LEDs
//CRGB SBleds[64];
cLEDMatrix<MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE> SB1Matrix;
cLEDMatrix<MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE> SB2Matrix;
cLEDText SB1Message;
cLEDText SB2Message;
unsigned char TxtDemo[] = { "0" };

// direction of ball travel
volatile boolean dirrection;

// did the player fail to hit the ball?
volatile boolean scored;

// Player scores 
int p1Score = 0;
int p2Score = 0;

// used for the button interupts. Set to TRUE on button press
volatile boolean paddle1;
volatile boolean paddle2;

// If true, ball speeds up ;)
volatile boolean fastMode;

// is the padle considered "up" and able to defelct the ball?
boolean paddle1Up;
boolean paddle2Up;

// used to determine when paddle is "up" and control visual feedback of that state
int paddle1Power = 255;
int paddle2Power = 255;

// the ball starts in the middle of the string...
int ballLED=NUM_LEDS/2;

// Variables to track number of times through loop()
int paddle1Count = 0;
int paddle2Count = 0;
int updateCount = 0;
int ballCount = 0;

// This variable gets decreased after each hit, resulting
// in a faster traveling ball. Start it at the defined number
// of loop()s
int ballSpeed = START_DELAY;

boolean newGame = true;

// Used to give a visual representation of the power of a hit
int ballPowerColor = 0;

void setup() {
    // sanity check delay - allows reprogramming if accidently blowing power w/leds
    delay(2000);
    
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.addLeds<WS2812, SCOREBOARD1_DATA_PIN, GRB>(SB1Matrix[0], 64);
    FastLED.addLeds<WS2812, SCOREBOARD2_DATA_PIN, GRB>(SB2Matrix[0], 64);
    FastLED.setBrightness(32);
    FastLED.clear(true);

    // set button pins as inputs
    pinMode(BUTTON1, INPUT);
    pinMode(BUTTON2, INPUT);

    // attach interrupts to buttons
    attachInterrupt(BUTTON1, isrB1, RISING);
    attachInterrupt(BUTTON2, isrB2, RISING);
    attachInterrupt(FAST_BUTTON, isrFAST, RISING);

    // both paddles are down and buttons are not pushed
    paddle1 = false;
    paddle1Up = false;
    paddle2 = false;
    paddle2Up = false;

    // both paddles are at maximum power and ready to hit
    paddle1Power = 255;
    paddle2Power = 255;

    // randoomly choose the starting ball direction
    randomSeed(analogRead(14));
    if(random(10) < 5){dirrection=true;}
    else {dirrection=false;}

    // light up Player paddles, update display
    for(int i=0;i<=2;i++){leds[i] = CRGB::Orange;}
    for(int i=1;i<=3;i++){leds[NUM_LEDS-i] = CRGB::Blue;}
    SB1Message.SetFont(MatriseFontData);
    SB1Message.Init(&SB1Matrix, SB1Matrix.Width(), SB1Message.FontHeight() + 1, 1, 1);
    SB1Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    SB1Message.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0xdf, 0x00);
    SB2Message.SetFont(MatriseFontData);
    SB2Message.Init(&SB2Matrix, SB2Matrix.Width(), SB2Message.FontHeight() + 1, 1, 1);
    SB2Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    SB2Message.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0x00, 0x00, 0xff);
    TxtDemo[0]=p1Score+'0';
    SB1Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    SB1Message.UpdateText();
    TxtDemo[0]=p2Score+'0';
    SB2Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    SB2Message.UpdateText();
    FastLED.show();
}

void loop() {

  while(newGame){
    leds[NUM_LEDS/2] = CRGB::Green;
    FastLED.show();
    delay(500);
    leds[NUM_LEDS/2] = CRGB::White;
    FastLED.show();
    delay(500);
    
  }

  if(paddle1==true){
    for(int i=0;i<=2;i++){leds[i] = CRGB::White;}
    paddle1Up=true;
    if(paddle1Count >= PADDLE_RATE){
      paddle1Power--;
      paddle1Count=0;
    }
    if(paddle1Power < PADDLE_RECHARGE){
      for(int i=0;i<=2;i++){leds[i].setRGB(255-(-1*paddle1Power), 0, 0);}
      paddle1Up=false;
    }
    if(paddle1Power == 0){
      paddle1Power=255;
      paddle1=false;
      for(int i=0;i<=2;i++){leds[i] = CRGB::Orange;}
    }
  }

  if(paddle2==true){
    for(int i=1;i<=3;i++){leds[NUM_LEDS-i] = CRGB::White;}
    paddle2Up=true;
    if(paddle2Count >= PADDLE_RATE){
      paddle2Power--;
      paddle2Count=0;
    }
    if(paddle2Power < PADDLE_RECHARGE){
      for(int i=1;i<=3;i++){leds[NUM_LEDS-i].setRGB(255-(-1*paddle2Power), 0, 0);}
      paddle2Up=false;
    }
    if(paddle2Power == 0){
      paddle2Power=255;
      paddle2=false;
      for(int i=1;i<=3;i++){leds[NUM_LEDS-i] = CRGB::Blue;}
    }
  }
  
  if(ballCount == ballSpeed){
    leds[ballLED] = CRGB::Black;
    if(dirrection){
      ballLED++;
      if(ballLED == NUM_LEDS-3 && paddle2Up){ //if ball hit...
        dirrection=false;
        ballPowerColor=map(paddle2Power, PADDLE_RECHARGE, 255, 0, 255);
        ballSpeed=ballSpeed-map(paddle2Power, PADDLE_RECHARGE, 255, 0, 20);
        if(ballSpeed<=MAX_SPEED){ballSpeed=MAX_SPEED;}
        }
      if(ballLED >= NUM_LEDS-2){ //if ball not hit...
          scored=true;
          p1Score++;
          SB1Matrix.DrawLine(0,0,0,7,CRGB::Green);
          SB1Matrix.DrawLine(7,0,7,7,CRGB::Green);
        } 
    }
    if(!dirrection){
      if(ballLED == 3 && paddle1Up){ //if ball hit...
        dirrection = true;
        ballPowerColor=map(paddle1Power, PADDLE_RECHARGE, 255, 0, 255);
        //ballSpeed=ballSpeed-(paddle1Power/HIT_SPEED_FACTOR);
        ballSpeed=ballSpeed-map(paddle1Power, PADDLE_RECHARGE, 255, 0, 20);
        if(ballSpeed<=MAX_SPEED){ballSpeed=MAX_SPEED;}
        ballLED++;}
      if(ballLED <= 2){ //if ball not hit...
          scored=true;
          p2Score++;
          SB2Matrix.DrawLine(0,0,0,7,CRGB::Green);
          SB2Matrix.DrawLine(7,0,7,7,CRGB::Green);
        } 
      ballLED--;}
    if(scored){handleScore();}
    ballCount=0;
  }
  if(updateCount == UPDATE_RATE){ //update ball position
    //leds[ballLED] = CRGB::Cyan;
    leds[ballLED].setRGB(ballPowerColor, 0, (255-(ballPowerColor)));
    FastLED.show();
    updateCount=0;
  }
  paddle1Count++;
  paddle2Count++;
  updateCount++;
  ballCount++;
}

void handleScore()
{
  if(random(10) < 5){dirrection=true;}
  else {dirrection=false;}    
  ballLED=NUM_LEDS/2;
  scored=false;
  fill_solid(leds,NUM_LEDS,CRGB::Red);
  FastLED.show();
  delay(1000);
  fill_solid(leds,NUM_LEDS,CRGB::Black);
  SB1Matrix.DrawRectangle(0,0,7,7,CRGB::Black);
  SB2Matrix.DrawRectangle(0,0,7,7,CRGB::Black);
  FastLED.show();
  for(int i=0;i<=2;i++){leds[i] = CRGB::Orange;}
  for(int i=1;i<=3;i++){leds[NUM_LEDS-i] = CRGB::Blue;}
  TxtDemo[0]=p1Score+'0';
  SB1Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  SB1Message.UpdateText();
  TxtDemo[0]=p2Score+'0';
  SB2Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  SB2Message.UpdateText();
  FastLED.show();
  delay(100);
  if(p1Score == 3){
    handleP1Win();
    newGame=true;
  }
  if(p2Score == 3){
    handleP2Win();
    newGame=true;
  }
  ballSpeed=START_DELAY;
  scored=false;
}

void handleP1Win()
{
  for(int i=0;i<5;i++){
    SB1Matrix.DrawLine(0,0,0,7,CRGB::White);
    SB1Matrix.DrawLine(7,0,7,7,CRGB::White);
    FastLED.show();
    delay(500);
    SB1Matrix.DrawLine(0,0,0,7,CRGB::Green);
    SB1Matrix.DrawLine(7,0,7,7,CRGB::Green);
    FastLED.show();
    delay(500);
  }
  SB1Matrix.DrawRectangle(0,0,7,7,CRGB::Black);
  SB2Matrix.DrawRectangle(0,0,7,7,CRGB::Black);
  FastLED.show();
  p1Score=0;
  p2Score=0;
  TxtDemo[0]=p1Score+'0';
  SB1Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  SB1Message.UpdateText();
  TxtDemo[0]=p2Score+'0';
  SB2Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  SB2Message.UpdateText();
  FastLED.show();
}

void handleP2Win()
{
  for(int i=0;i<5;i++){
    SB2Matrix.DrawLine(0,0,0,7,CRGB::White);
    SB2Matrix.DrawLine(7,0,7,7,CRGB::White);
    FastLED.show();
    delay(500);
    SB2Matrix.DrawLine(0,0,0,7,CRGB::Green);
    SB2Matrix.DrawLine(7,0,7,7,CRGB::Green);
    FastLED.show();
    delay(500);
  }
  SB1Matrix.DrawRectangle(0,0,7,7,CRGB::Black);
  SB2Matrix.DrawRectangle(0,0,7,7,CRGB::Black);
  FastLED.show();
  p1Score=0;
  p2Score=0;
  TxtDemo[0]=p1Score+'0';
  SB1Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  SB1Message.UpdateText();
  TxtDemo[0]=p2Score+'0';
  SB2Message.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  SB2Message.UpdateText();
  FastLED.show();
}

void isrB1()
{
  paddle1 = true;
  newGame = false;
}
void isrB2()
{
  paddle2 = true;
  newGame = false;
}

void isrFAST()
{
  //fastMode = true;
  ballSpeed=MAX_SPEED;
  ballCount=0;
}
