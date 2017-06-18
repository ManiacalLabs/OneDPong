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
#define START_DELAY 500

//how much the delay decreases each successful hit
#define SPEED_STEP 100

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

//direction of ball travel
volatile boolean dirrection;

volatile boolean scored;

volatile boolean paddle1;
volatile boolean paddle2;

int whiteLed=NUM_LEDS/2;
int ballDelay=START_DELAY;

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
    paddle2 = false;

    randomSeed(analogRead(14));

    if(random(10) < 5){dirrection=true;}
    else {dirrection=false;}
}

void loop() {
   
  // for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1)

  leds[whiteLed] = CRGB::Cyan;
  FastLED.show();
  delay(ballDelay);
  leds[whiteLed] = CRGB::Black;
  if(dirrection){
    whiteLed++;
    if(whiteLed == NUM_LEDS-1 && paddle2){
      dirrection=false;
      ballDelay=ballDelay-SPEED_STEP;}
    if(whiteLed >= NUM_LEDS){scored=true;}
  }
  if(!dirrection){
    if(whiteLed == 1 && paddle1){
      dirrection = true;
      ballDelay=ballDelay-SPEED_STEP;
      whiteLed++;}
    if(whiteLed == 0){scored=true;}
    whiteLed--;
  }
  if(scored){
    handleScore();
  }
  paddle1=false;
  paddle2=false;
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

