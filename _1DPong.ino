#include "FastLED.h"

#include <Bounce.h> //Used for button de-bouncing

// How many leds are in the strip?
#define NUM_LEDS 20

// Data pin that led data will be written out over
#define DATA_PIN 6

// Define button Pins
#define BUTTON1 7
#define BUTTON2 8

// Starting delay between ticks (shorter=faster ball)
#define START_DELAY 10000

//how much the delay decreases each successful hit
#define SPEED_STEP 500

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

//direction of ball travel
volatile boolean dirrection;

volatile boolean scored;

volatile boolean paddle1;
volatile boolean paddle2;

boolean paddle1Up;
boolean paddle2Up;

int paddle1Power = 255;
int paddle2Power = 255;

int whiteLed=NUM_LEDS/2;
int ballDelay=START_DELAY;

int stepCount=0;

// This function sets up the ledsand tells the controller about them
void setup() {
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
   	delay(2000);
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(64);

    // set button pins as inputs
    pinMode(BUTTON1, INPUT);
    pinMode(BUTTON2, INPUT);

    // attach interrupts to buttons
    attachInterrupt(BUTTON1, isrB1, RISING);
    attachInterrupt(BUTTON2, isrB2, RISING);

    dirrection = true;
    paddle1 = false;
    paddle1Up = false;
    paddle2 = false;
    paddle2Up = false;

    paddle1Power = 255;
    paddle2Power = 255;

    //start with specified maximum count (slowest speed)
    stepCount = ballDelay;

    randomSeed(analogRead(14));

    if(random(10) < 5){dirrection=true;}
    else {dirrection=false;}

    leds[0] = CRGB::Orange;
    leds[NUM_LEDS-1] = CRGB::Blue;
    FastLED.show();
}

void loop() {
   
  // for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1)
  //delay(ballDelay);
  
  stepCount--;

  if(paddle1==true){
    leds[0] = CRGB::White;
    if(stepCount%150 == 0){
      paddle1Power--;
      paddle1Up=true;
    }
    if(paddle1Power < 150){
      leds[0].setRGB(255-(-1*paddle1Power), 0, 0);
      paddle1Up=false;
    }
    if(paddle1Power == 0){
      paddle1Power=255;
      paddle1=false;
      leds[0] = CRGB::Orange;
    }
  }

  if(paddle2==true){
    leds[NUM_LEDS-1] = CRGB::White;
    if(stepCount%150 == 0){
      paddle2Power--;
      paddle2Up=true;
    }
    if(paddle2Power < 150){
      leds[NUM_LEDS-1].setRGB(255-(-1*paddle2Power), 0, 0);
      paddle2Up=false;
    }
    if(paddle2Power == 0){
      paddle2Power=255;
      paddle2=false;
      leds[NUM_LEDS-1] = CRGB::Blue;
    }
  }
  
  if(stepCount==0){
    //fill_solid(leds,NUM_LEDS,CRGB::Black);
    leds[whiteLed] = CRGB::Black;
    //FastLED.show();
    //leds[whiteLed] = CRGB::Black;
    if(dirrection){
      whiteLed++;
      if(whiteLed == NUM_LEDS-1 && paddle2Up){
        dirrection=false;
        ballDelay=ballDelay-SPEED_STEP;}
      if(whiteLed >= NUM_LEDS){scored=true;}
    }
    if(!dirrection){
      if(whiteLed == 1 && paddle1Up){
        dirrection = true;
        ballDelay=ballDelay-SPEED_STEP;
        whiteLed++;}
      if(whiteLed == 0){scored=true;}
      whiteLed--;
    }
    if(scored){
      handleScore();
    }
    stepCount=ballDelay;
  }
  if(stepCount%70 == 0){
    //paddle2Power--;
    leds[whiteLed] = CRGB::Cyan;
    FastLED.show();
  }
}

void handleScore()
{
  if(random(10) < 5){dirrection=true;}
  else {dirrection=false;}    
  whiteLed=NUM_LEDS/2;
  scored=false;
  fill_solid(leds,NUM_LEDS,CRGB::Red);
  FastLED.show();
  delay(1000);
  fill_solid(leds,NUM_LEDS,CRGB::Black);
  leds[0] = CRGB::Orange;
  leds[NUM_LEDS-1] = CRGB::Blue;
  FastLED.show();
  delay(100);
  ballDelay=START_DELAY;
  scored=false;
}

void isrB1()
{
  paddle1 = true;
}
void isrB2()
{
  paddle2 = true;
}

