// =======================================================================================
//                         BB-8 Head -- Lights and Sounds
// =======================================================================================
//                            Written By: PatchBOTS
// =======================================================================================
//       Credit and  Major thank you to:  David Scott,  afreeland, and ron_dl
// ---------------------------------------------------------------------------------------
//                     microcontroller  ---  Sparkfun Pro Micro (5v)
// --------------------------------------------------------------------------------------
//
//                                 proMicro
//                                ------------
//                                |           |
//                     +--------------------------------+
//                     |                                |
//                     | [ ]TX                   RAW[ ] |
//              HC-05  | [ ]RX                   GND[ ] |
//                     | [ ]GND                  RST[ ] |
//                     | [ ]GND                  VCC[ ] |
//                     | [ ]2                     A3[ ] |
//               Busy  | [ ]3                     A2[ ] |
//              NeoPix | [ ]4                     A1[ ] |
//               State | [ ]5                     A0[ ] | Audio in
//                     | [ ]6                     15[ ] | SoftSerialTX
//                     | [ ]7                     14[ ] |
//                     | [ ]8                     16[ ] |
//                     | [ ]9                     10[ ] |
//                     |                                |
//                     +--------------------------------+
//
//
// ---------------------------------------------------------------------------------------
//                                   NOTES
// ---------------------------------------------------------------------------------------
//
//SMD HC-05 Bluetooth Module Slave address:  98d3,32,31870f

// -----------------------------------------------------------------------------
//                          Libraries
// -----------------------------------------------------------------------------
#include "MP3FLASH16P.h"
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

// -----------------------------------------------------------------------------
//                      Definitions/Variables
// -----------------------------------------------------------------------------

#define mp3busypin 3  // connected to Busy pin on Mp3 Unit
#define AudioIn    A0 //connected to SPK1 on mP3 player
#define neopixel   4  // neopixels out

//**********NeoPixels**************
#define NUMPIXELS      18
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, neopixel, NEO_GRB + NEO_KHZ800);

//*****Arrays for specific NeoLights
int blueLogicArray [] = {0, 1, 2};
int topLogicArray [] = {3, 5, 7};
int bottomLogicArray [] = {4, 6, 8};
int dualLogicArray [] = {3, 4, 5, 6, 7, 8}; //for both top and bottom white logic displays
int dualLogicHoloArray [] = {3, 4, 5, 6, 7, 8, 10}; //for both top and bottom white logics and the hololens
int radarArray [] = {11, 12, 13, 14, 15, 16, 17};
//PSI light is 9
//HoloLens is 10

//**********Serial Input**************
boolean newData = false;
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];
int trig1 = 1;
int trig2 = 1;
int trig3 = 1;
int trig4 = 1;
int trig5 = 1;
int volumeKnob = 500;
int aniMode = 0;

//**********PSI Light**************
int AudioReading;
int mp3volume;
int voiceBrightness = 0;
int audioUpper;
int audioLower;
int absoluteUpper;
int pulse;

//**********mp3Player**************
MP3FLASH16P myPlayer; //calls the David Scott mp3 library

//**********Timing**************
long previousMillis = 0;        // will store last time sound played was updated
long sound_interval = 15000; //the autonomous sound interval.  15000 = 15 seconds between sounds
int currentMillis = 0;

//*********AnimationVaribales*************************
//GLOBAL
long startTime = 0; //used as a stopwatch for the animations

//Color Variables
uint32_t low = pixels.Color(0, 0, 0); //a simple way of writing "turn this pixel off"
uint32_t neoRed = pixels.Color(255, 0, 0);//a simple way of writing "turn this pixel red"
uint32_t neoBlue = pixels.Color(0, 0, 255);//a simple way of writing "turn this pixel blue"
uint32_t neoWhite = pixels.Color(255, 255, 255);//a simple way of writing "turn this pixel white"
uint32_t voiceBright = pixels.Color(voiceBrightness, voiceBrightness, voiceBrightness); //a simple way of writing "make this pixel match teh voice brightness"
uint32_t voiceBrightRed = pixels.Color(voiceBrightness,0,0); //a simple way of writing "make this pixel match teh voice brightness in a red color"
uint32_t voiceBrightBlue = pixels.Color(0,0,voiceBrightness); //a simple way of writing "make this pixel match teh voice brightness in a Blue color"

//Animation 1
int aniOneInterval = 60; //for single knightriders, time between each lightup
int aniOne_i = 0; //generic holder
int aniOne_iLast = 0;
int aniOneRiderFlag = 3; //this will allow the loops of knightrider scrolls to run sequentially
unsigned long aniOneMillis = 0;  //local stopwatch for Animation 1
long aniOnePreviousMillis = 0; //a ref point for timing
bool aniOneP2 = false; 

//Animation 2
unsigned long aniTwoMillis = 0;  //local stopwatch for Animation 1
bool aniTwoFlag1= false; 
bool aniTwoFlag2= false; 
bool aniTwoFlag3= false;
bool aniTwoFlag4= false;
bool aniTwoFlag5= false; 
int aniTwo_i = 0; //generic holder
int aniTwo_iLast = 0;
int aniTwoRiderFlag = 0; //this will allow the loops of knightrider scrolls to run sequentially
int aniTwoInterval = 60; //for single knightriders, time between each lightup
long aniTwoPreviousMillis = 0; //a ref point for timing

//Animation 3
unsigned long aniThreeMillis = 0;  //local stopwatch for Animation 1
long aniThreePreviousMillis = 0; //a ref point for timing
bool aniThreeFlag = false; 

//Animation 4
unsigned long aniFourMillis = 0;  //local stopwatch for Animation 1
long aniFourPreviousMillis = 0; //a ref point for timing
bool aniFourFlag = false; 

//Animation 5
unsigned long aniFiveMillis = 0;  //local stopwatch for Animation 1
long aniFivePreviousMillis = 0; //a ref point for timing
bool aniFiveFlag = false; 



// -----------------------------------------------------------------------------
//                              SETUP
// -----------------------------------------------------------------------------
void setup() {
  myPlayer.init(mp3busypin);   // Init the player with the busy pin connected to Arduino pin 3
  delay(1000);  // Startup delay
  Serial.begin(9600);  //Start Serial, eventually connecting to HC-05
  Serial1.begin(9600);
  pixels.begin(); // This initializes the NeoPixel library.
  pinMode(AudioIn, INPUT); //set pin to read voltage output
 knightRider();

}
// -----------------------------------------------------------------------------
//                             MAIN LOOP
// -----------------------------------------------------------------------------
void loop() {
  currentMillis = millis();

  serialStuff(); //reads and parses all incoming serial data into integers.  there is an function "showParsedData" which will print to Serial Monitor for debugging

  //**********Volume Knob************************
  mp3volume = map(volumeKnob, 0, 1023, 0, 30); //reads and scales the poteniometer value (0-1023) to a usable mp3 volume (0-30)
  psiLightData (); //this function calculates the light value for the PSI based on the volumeKnob value.

  neoPixels(); //default NeoPixels values for the droid.  Animations will potential update these values before they are shown in the trigModeAni function below
  autoMode(); // This will play a random sound at the desired interval. To change look for "long sound_interval" under "Timing"
  trigMode(); //This will play a sound triggered by the buttons when Animation Mode is not engaged.
  trigModeAni(); //This will play a sound triggered by the buttons with Animations when Animation Mode is engaged.

  aniOne();
  aniTwo();
  aniThree();
  aniFour(); 
  aniFive();

  pixels.show(); // This sends the updated pixel color to the hardware.

}



// -----------------------------------------------------------------------------
//                ADDITIONAL FUNCTIONS (NON ANIMATION)
// -----------------------------------------------------------------------------
//*******Serial***********
void serialStuff () {
  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    parseData();
    showParsedData(); //prints incoming data to Serial port. Mainly for debugging
    newData = false;
  }
}

//*******Autonomous Mode***********
void autoMode () {
  if (currentMillis - previousMillis > sound_interval) {
    previousMillis = currentMillis;
    myPlayer.playFile(random(1, 10), mp3volume);
  }
}

//*******Trigger Mode (No Animations)***********
void trigMode () {
  if (aniMode == 0) {
    if ((trig1 == 0) && (!myPlayer.isBusy())) {
      previousMillis = currentMillis;
      myPlayer.playFile(1, mp3volume);
    }
    if ((trig2 == 0) && (!myPlayer.isBusy())) {
      previousMillis = currentMillis;
      myPlayer.playFile(2, mp3volume);
    }

    if ((trig3 == 0) && (!myPlayer.isBusy())) {
      previousMillis = currentMillis;
      myPlayer.playFile(3, mp3volume);
    }
    if ((trig4 == 0) && (!myPlayer.isBusy())) {
      previousMillis = currentMillis;
      myPlayer.playFile(4, mp3volume);
    }
    if ((trig5 == 0) && (!myPlayer.isBusy())) {
      previousMillis = currentMillis;
      myPlayer.playFile(5, mp3volume);
    }
  }
}

//*******Trigger Mode (With Animations)***********
void trigModeAni () {
  if (aniMode == 1) {
    if ((trig1 == 0) && (!myPlayer.isBusy())) {
      myPlayer.playFile(1, mp3volume);
      previousMillis = currentMillis;
      startTimer ();
      aniOneRiderFlag = 0;
      aniOneP2 = true; 
    }
    if ((trig2 == 0) && (!myPlayer.isBusy())) {
      myPlayer.playFile(2, mp3volume);
      previousMillis = currentMillis;
      startTimer ();
      aniTwoFlag1 = true; 
    }
    if ((trig3 == 0) && (!myPlayer.isBusy())) {
      myPlayer.playFile(3, mp3volume);
      previousMillis = currentMillis;
      startTimer ();
      aniThreeFlag = true; 
    }
    if ((trig4 == 0) && (!myPlayer.isBusy())) {
      myPlayer.playFile(4, mp3volume);
      previousMillis = currentMillis;
      startTimer ();
      aniFourFlag = true; 
    }
    if ((trig5 == 0) && (!myPlayer.isBusy())) {
      myPlayer.playFile(5, mp3volume);
      previousMillis = currentMillis;
      startTimer ();
      aniFiveFlag = true; 
    }
  }
}

//*******PSI Data Calculation****************
void psiLightData () {
  if ((volumeKnob >=   0) && (volumeKnob <=  33)) {
    audioUpper = 512; //0
    audioLower = 512;//0
    absoluteUpper = 0;
  }
  if ((volumeKnob >=  34) && (volumeKnob <= 132)) {
    audioUpper = 542; //30
    audioLower = 475; //37
    absoluteUpper = 37;
  }
  if ((volumeKnob >= 133) && (volumeKnob <= 231)) {
    audioUpper = 593; //81
    audioLower = 418; //94
    absoluteUpper = 94;
  }
  if ((volumeKnob >= 232) && (volumeKnob <= 330)) {
    audioUpper = 644; //132
    audioLower = 367; //145
    absoluteUpper = 145;
  }
  if ((volumeKnob >= 331) && (volumeKnob <= 429)) {
    audioUpper = 695; //183
    audioLower = 316; //196
    absoluteUpper = 196;
  }
  if ((volumeKnob >= 430) && (volumeKnob <= 528)) {
    audioUpper = 746; //234
    audioLower = 264; //248
    absoluteUpper = 248;
  }
  if ((volumeKnob >= 529) && (volumeKnob <= 627)) {
    audioUpper = 797; //285
    audioLower = 212; //300
    absoluteUpper = 300;
  }
  if ((volumeKnob >= 628) && (volumeKnob <= 726)) {
    audioUpper = 848; //336
    audioLower = 160; //352
    absoluteUpper = 352;
  }
  if ((volumeKnob >= 727) && (volumeKnob <= 825)) {
    audioUpper = 899; //387
    audioLower = 108; //404
    absoluteUpper = 404;
  }
  if ((volumeKnob >= 826) && (volumeKnob <= 924)) {
    audioUpper = 953; //441
    audioLower = 56; //456
    absoluteUpper = 456;
  }
  if ((volumeKnob >= 925) && (volumeKnob <= 1023)) {
    audioUpper = 1008; //496
    audioLower = 4; //508
    absoluteUpper = 508;
  }
}


//********PSI Light********************
void psiLight() {
  AudioReading = analogRead(AudioIn);
  //I'm using an arbitrary 34 on the volume knob as the lower end. Anything below that value is effectively "off"
  if ((myPlayer.isBusy()) && (volumeKnob >= 34)) { //if the mP3 player is and the volume is over the 34 lower limit
    if ((AudioReading >= 485) && (AudioReading <= 540)) { //there is a little dead zone that has no audible sound, but still makes the light come on, looks weird.
      pixels.setPixelColor(9, pixels.Color(0, 0, 0)); //this says turn the light off in that deadzone.
    }

    else { //otherwise, if not in the dead zone
      voiceBrightness = constrain(map(AudioReading, 512, audioUpper, -10, 300), 0, 255); //map the upper portion of the audio reading (512 through upper limit determined by volume knob) to -10 and 300, and then constrain that to 0-255.  This constraint hacks off some edges that don't help the effect.
      pixels.setPixelColor(9, pixels.Color(voiceBrightness, voiceBrightness, voiceBrightness)); //set PSI light to calculated value
    }
  }
  else { //otherwise if the mp3 player isn't busy or the volume knob isn't greater than 34....
    pixels.setPixelColor(9, pixels.Color(0, 0, 0));//....turn the light off
  }

}

//********HoloLens**************
void holoLens() {
  pulse = 100 + (sin(millis() / 1000.00) * 80); //generates an oscillating sine wave of values
  pixels.setPixelColor(10, pixels.Color(0, 0, pulse)); // writes the value to the Blue channel of the neopixel
}


//********Logics***************
void logics() {
  // Blue Logics
  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Set Side Logics to Blue
  pixels.setPixelColor(1, pixels.Color(0, 0, 255)); // Set Side Logics to Blue
  pixels.setPixelColor(2, pixels.Color(0, 0, 255)); // Set Side Logics to Blue

  //Top Logics
  pixels.setPixelColor(3, pixels.Color(200, 200, 200)); // Set Top Logics to White
  pixels.setPixelColor(5, pixels.Color(200, 200, 200)); // Set Top Logics to White
  pixels.setPixelColor(7, pixels.Color(200, 200, 200)); // Set Top Logics to White

  //Bottom Logics
  pixels.setPixelColor(4, pixels.Color(0, 0, 0)); // Set Bottom Logics off
  pixels.setPixelColor(6, pixels.Color(0, 0, 0)); // Set Bottom Logics off
  pixels.setPixelColor(8, pixels.Color(0, 0, 0)); // Set Bottom Logics off
}

//*********SerialStuff*******************
void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial1.available() > 0 && newData == false) {
    rc = Serial1.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }
    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

//*********SerialStuff*******************
void parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the first part - the string
  trig1 = atoi(strtokIndx);  // convert to an integer (trig1)

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  trig2 = atoi(strtokIndx);     // convert to an integer (trig2)

  strtokIndx = strtok(NULL, ",");
  trig3 = atoi(strtokIndx);

  strtokIndx = strtok(NULL, ",");
  trig4 = atoi(strtokIndx);

  strtokIndx = strtok(NULL, ",");
  trig5 = atoi(strtokIndx);

  strtokIndx = strtok(NULL, ",");
  volumeKnob = atoi(strtokIndx);

  strtokIndx = strtok(NULL, ",");
  aniMode = atoi(strtokIndx);

}

//*********SerialStuff*******************
void showParsedData() {

  Serial.print(trig1);
  Serial.print(",");
  Serial.print(trig2);
  Serial.print(",");
  Serial.print(trig3);
  Serial.print(",");
  Serial.print(trig4);
  Serial.print(",");
  Serial.print(trig5);
  Serial.print(",");
  Serial.print(volumeKnob);
  Serial.print(",");
  Serial.println(aniMode);
}

//*********NeoPixels*******************

void neoPixels () {
  //Radar Light
  pixels.setPixelColor(11, pixels.Color(200, 0, 0)); // Set First Radar light to red
  logics();
  psiLight();
  holoLens();
}



// -----------------------------------------------------------------------------
//               Animation Functinos
// -----------------------------------------------------------------------------

// A timer function that runs in a lot of the animations
void startTimer () {
  startTime = millis();
  return;
}

//BootUp Animation
void knightRider() {
  int krcount = 0;
  int krtimer = 40;


  // Flash Lights
  for (int i = 0; i < 10; i++) { //set number of loops    i < [numberofloops]
    for (krcount = 0; krcount < 5; krcount++) {
      pixels.setPixelColor(radarArray[krcount], neoRed);
      pixels.show();
      delay(krtimer);
      pixels.setPixelColor(radarArray[krcount + 1], neoRed);
      pixels.show();
      delay(krtimer);
      pixels.setPixelColor(radarArray[krcount], low);
      pixels.show();
      delay(krtimer);
    }
    for (krcount = 5; krcount > 0; krcount--) {
      pixels.setPixelColor(radarArray[krcount], neoRed);
      pixels.show();
      delay(krtimer);
      pixels.setPixelColor(radarArray[krcount - 1], neoRed);
      pixels.show();
      delay(krtimer);
      pixels.setPixelColor(radarArray[krcount], low);
      pixels.show();
      delay(krtimer);
    }
  }

  // Turn them off
  for ( int i = 0; i < 6; i++) {
    pixels.setPixelColor(i, low);
    pixels.show();
  }
}
//Sound 1 - Animation

void aniOne () {
  aniOneMillis = millis();

  if (aniOneMillis - startTime > 900) {
    if (aniOneRiderFlag < 3) {
      pixels.setPixelColor(11, low); 
      pixels.show();
      pixels.setPixelColor(radarArray[aniOne_i], neoRed);
      pixels.show();
      if (aniOneMillis - aniOnePreviousMillis > aniOneInterval) {
        aniOnePreviousMillis = aniOneMillis;
        pixels.setPixelColor(11, low); // Set Side Logics to Blue
        pixels.show();
        pixels.setPixelColor(radarArray[aniOne_iLast], low);
        pixels.show();
        aniOne_iLast = aniOne_i;
        aniOne_i += 1;
        if (aniOne_i > 6) {
          aniOne_i = 0;
          aniOneRiderFlag += 1;
        }
      }
    }
  }

  if (aniOneMillis - startTime > 2300 && aniOneP2 == true) {
    pixels.setPixelColor(4, pixels.Color(voiceBrightness, voiceBrightness, voiceBrightness));
    pixels.setPixelColor(6, pixels.Color(voiceBrightness, voiceBrightness, voiceBrightness));
    pixels.setPixelColor(8, pixels.Color(voiceBrightness, voiceBrightness, voiceBrightness));
    pixels.show(); 
  }
  if (aniOneMillis - startTime > 3000 && aniOneP2 == true) {
    aniOneP2 = false;
    return;
  }
}
//Sound 2 - Animation

void aniTwo () {
  aniTwoMillis = millis();

// SOUND STAGE 1  - THIS IS GOING TO REPLICATE THE VOICE BRIGHTNESS ON THE BOTTOM LOGICS
  if (aniTwoMillis - startTime > 100 && aniTwoMillis - startTime < 800 && aniTwoFlag1 ==true){ 
    pixels.setPixelColor(4, pixels.Color(voiceBrightness+50, voiceBrightness+50, voiceBrightness+50));
    pixels.setPixelColor(6, pixels.Color(voiceBrightness+50, voiceBrightness+50, voiceBrightness+50));
    pixels.setPixelColor(8, pixels.Color(voiceBrightness+50, voiceBrightness+50, voiceBrightness+50));
    pixels.show(); 
    aniTwoFlag2 = true; 
  }
 //SOUND GAP 1
  if (aniTwoMillis - startTime >= 800 && aniTwoMillis - startTime < 1250 && aniTwoFlag2 == true){
    pixels.setPixelColor(3, low);
    pixels.setPixelColor(4, low);
    pixels.setPixelColor(5, low);
    pixels.setPixelColor(6, low);
    pixels.setPixelColor(7, low);
    pixels.setPixelColor(8, low);
    pixels.show(); 
    aniTwoRiderFlag =1; 
    aniTwoFlag3 = true; 
  }
// SOUND STAGE 2  - This is the "knightrider" style lines
  if (aniTwoMillis - startTime >=1250 && aniTwoMillis - startTime < 2175 && aniTwoFlag3 == true){
      if (aniTwoRiderFlag == 1 || aniTwoRiderFlag == 3)  {
      pixels.setPixelColor(3, low);
      pixels.setPixelColor(5, low);
      pixels.setPixelColor(7, low); 
      pixels.show();
      pixels.setPixelColor(topLogicArray[aniTwo_i], neoWhite);
      pixels.show();
      if (aniTwoMillis - aniTwoPreviousMillis > aniTwoInterval) {
        aniTwoPreviousMillis = aniTwoMillis;
        pixels.setPixelColor(3, low); 
        pixels.setPixelColor(5, low);
        pixels.setPixelColor(7, low);
        pixels.show();
        pixels.setPixelColor(topLogicArray[aniTwo_iLast], low);
        pixels.show();
        aniTwo_iLast = aniTwo_i;
        aniTwo_i += 1;
        if (aniTwo_i > 3) {
          aniTwo_i = 0;
          aniTwoRiderFlag += 1;
        }
      }
    }
      if (aniTwoRiderFlag == 2 || aniTwoRiderFlag == 4){
      pixels.setPixelColor(4, low);
      pixels.setPixelColor(6, low);
      pixels.setPixelColor(8, low); 
      pixels.show();
      pixels.setPixelColor(bottomLogicArray[aniTwo_i], neoWhite);
      pixels.show();
      if (aniTwoMillis - aniTwoPreviousMillis > aniTwoInterval) {
        aniTwoPreviousMillis = aniTwoMillis;
        pixels.setPixelColor(4, low); 
        pixels.setPixelColor(6, low);
        pixels.setPixelColor(8, low);
        pixels.show();
        pixels.setPixelColor(bottomLogicArray[aniTwo_iLast], low);
        pixels.show();
        aniTwo_iLast = aniTwo_i;
        aniTwo_i += 1;
        if (aniTwo_i > 3) {
          aniTwo_i = 0;
          aniTwoRiderFlag += 1;
        }
      }
    }
      if (aniTwoRiderFlag == 5){
      pixels.setPixelColor(4, low);
      pixels.setPixelColor(6, low);
      pixels.setPixelColor(8, low); 
      pixels.setPixelColor(3, low);
      pixels.setPixelColor(5, low);
      pixels.setPixelColor(7, low);
      pixels.show();
      pixels.setPixelColor(bottomLogicArray[aniTwo_i], neoWhite);
      pixels.setPixelColor(topLogicArray[aniTwo_i], neoWhite);
      pixels.show();
      if (aniTwoMillis - aniTwoPreviousMillis > aniTwoInterval) {
        aniTwoPreviousMillis = aniTwoMillis;
      pixels.setPixelColor(4, low);
      pixels.setPixelColor(6, low);
      pixels.setPixelColor(8, low); 
      pixels.setPixelColor(3, low);
      pixels.setPixelColor(5, low);
      pixels.setPixelColor(7, low);
        pixels.show();
        pixels.setPixelColor(bottomLogicArray[aniTwo_iLast], low);
        pixels.setPixelColor(topLogicArray[aniTwo_iLast], low);
        pixels.show();
        aniTwo_iLast = aniTwo_i;
        aniTwo_i += 1;
        if (aniTwo_i > 3) {
          aniTwo_i = 0;
          aniTwoRiderFlag += 1;
        }
      }
    }
    pixels.show(); 
    aniTwoFlag4 = true; 
  }
//Sound Gap 2 
      if (aniTwoMillis - startTime >=2175 && aniTwoMillis - startTime < 2700 && aniTwoFlag4 == true){
    pixels.setPixelColor(3, low);
    pixels.setPixelColor(4, low);
    pixels.setPixelColor(5, low);
    pixels.setPixelColor(6, low);
    pixels.setPixelColor(7, low);
    pixels.setPixelColor(8, low);
    pixels.show();
    aniTwoFlag5 = true; 
  }
  //BEGIN SOUND STAGE 3 - 2800 - 3800
    if (aniTwoMillis - startTime >=2700 && aniTwoMillis - startTime < 2800 && aniTwoFlag5 == true){
    pixels.setPixelColor(3, low);
    pixels.setPixelColor(4, low);
    pixels.setPixelColor(5, neoWhite);
    pixels.setPixelColor(6, low);
    pixels.setPixelColor(7, low);
    pixels.setPixelColor(8, neoWhite);
    pixels.show(); 
  }
    if (aniTwoMillis - startTime >=2800 && aniTwoMillis - startTime < 2900 && aniTwoFlag5 == true){
    pixels.setPixelColor(3, low);
    pixels.setPixelColor(4, low);
    pixels.setPixelColor(5, low);
    pixels.setPixelColor(6, neoWhite);
    pixels.setPixelColor(7, neoWhite);
    pixels.setPixelColor(8, low);
    pixels.show(); 
  }
    if (aniTwoMillis - startTime >=2900 && aniTwoMillis - startTime < 3000 && aniTwoFlag5 == true){
    pixels.setPixelColor(3, neoWhite);
    pixels.setPixelColor(4, low);
    pixels.setPixelColor(5, low);
    pixels.setPixelColor(6, low);
    pixels.setPixelColor(7, low);
    pixels.setPixelColor(8, neoWhite);
    pixels.show(); 
  }
    if (aniTwoMillis - startTime >=3000&& aniTwoMillis - startTime < 3100 && aniTwoFlag5 == true){
    pixels.setPixelColor(3, low);
    pixels.setPixelColor(4, neoWhite);
    pixels.setPixelColor(5, neoWhite);
    pixels.setPixelColor(6, low);
    pixels.setPixelColor(7, low);
    pixels.setPixelColor(8, low);
    pixels.show(); 
  }
    if (aniTwoMillis - startTime >=3100 && aniTwoMillis - startTime < 3200 && aniTwoFlag5 == true){
     pixels.setPixelColor(3, low);
    pixels.setPixelColor(4, low);
    pixels.setPixelColor(5, neoWhite);
    pixels.setPixelColor(6, neoWhite);
    pixels.setPixelColor(7, low);
    pixels.setPixelColor(8, low);
    pixels.show(); 
  }
    if (aniTwoMillis - startTime >=3200 && aniTwoMillis - startTime < 3300 && aniTwoFlag5 == true){
    pixels.setPixelColor(3, low);
    pixels.setPixelColor(4, neoWhite);
    pixels.setPixelColor(5, low);
    pixels.setPixelColor(6, low);
    pixels.setPixelColor(7, low);
    pixels.setPixelColor(8, neoWhite);
    pixels.show(); 
  }
    if (aniTwoMillis - startTime >=3300 && aniTwoMillis - startTime < 3400 && aniTwoFlag5 == true){
    pixels.setPixelColor(3, neoWhite);
    pixels.setPixelColor(4, low);
    pixels.setPixelColor(5, low);
    pixels.setPixelColor(6, low);
    pixels.setPixelColor(7, neoWhite);
    pixels.setPixelColor(8, low);
    pixels.show(); 
  }    
   if (aniTwoMillis - startTime >=3300){
   aniTwoFlag1 = false; 
   aniTwoFlag2 = false;
   aniTwoFlag3 = false;
   aniTwoFlag4 = false;
   aniTwoFlag5 = false;
   return; 
   }
}


void aniThree () {
    aniThreeMillis = millis();
    if (aniThreeMillis - startTime > 1300 && aniOneMillis - startTime <1450 && aniThreeFlag ==true) {
      pixels.setPixelColor(11, neoRed);
      pixels.setPixelColor(12, neoRed);
      pixels.setPixelColor(13, neoRed);
      pixels.setPixelColor(14, neoRed);
      pixels.setPixelColor(15, neoRed);
      pixels.setPixelColor(16, neoRed);        
      }
    if (aniThreeMillis - startTime >= 1450 && aniOneMillis - startTime <1550 && aniThreeFlag ==true) {
      pixels.setPixelColor(11, low);
      pixels.setPixelColor(12, low);
      pixels.setPixelColor(13, low);
      pixels.setPixelColor(14, low);
      pixels.setPixelColor(15, low);
      pixels.setPixelColor(16, low);        
      }
        if (aniThreeMillis - startTime >= 1550 && aniOneMillis - startTime <1700 && aniThreeFlag ==true) {
      pixels.setPixelColor(11, neoRed);
      pixels.setPixelColor(12, neoRed);
      pixels.setPixelColor(13, neoRed);
      pixels.setPixelColor(14, neoRed);
      pixels.setPixelColor(15, neoRed);
      pixels.setPixelColor(16, neoRed);        
      }
    if (aniThreeMillis - startTime >= 1700 && aniThreeFlag ==true) {
      pixels.setPixelColor(11, neoRed);
      pixels.setPixelColor(12, low);
      pixels.setPixelColor(13, low);
      pixels.setPixelColor(14, low);
      pixels.setPixelColor(15, low);
      pixels.setPixelColor(16, low);
      aniThreeFlag = false; 
      return;         
      }
    

}

void aniFour () {
    aniFourMillis = millis();
    if (aniFourMillis - startTime > 300 && aniOneMillis - startTime <1000 && aniFourFlag ==true) {
      pixels.setPixelColor(4, neoWhite);
      pixels.setPixelColor(6, neoWhite);
      pixels.setPixelColor(8, neoWhite);
      pixels.setPixelColor(10,neoBlue); 
      pixels.setPixelColor(11, neoRed);
      pixels.setPixelColor(12, neoRed);
      pixels.setPixelColor(13, neoRed);
      pixels.setPixelColor(14, neoRed);
      pixels.setPixelColor(15, neoRed);
      pixels.setPixelColor(16, neoRed);
                
      }
    if (aniFourMillis - startTime >= 1000 && aniFourFlag ==true) {
      pixels.setPixelColor(4, low);
      pixels.setPixelColor(6, low);
      pixels.setPixelColor(8, low);
      pixels.setPixelColor(10,voiceBright); 
      pixels.setPixelColor(11, neoRed);
      pixels.setPixelColor(12, low);
      pixels.setPixelColor(13, low);
      pixels.setPixelColor(14, low);
      pixels.setPixelColor(15, low);
      pixels.setPixelColor(16, low);
      aniFourFlag = false; 
      return;         
    }
}

void aniFive () {
    aniFiveMillis = millis();
    if (aniFiveMillis - startTime > 100 && aniFiveMillis - startTime <600 && aniFiveFlag ==true) {
      pixels.setPixelColor(4, neoWhite);
      pixels.setPixelColor(6, neoWhite);
      pixels.setPixelColor(8, neoWhite);
      pixels.setPixelColor(5, neoWhite);
      pixels.setPixelColor(7, neoWhite);
      pixels.setPixelColor(9, neoWhite);
      pixels.setPixelColor(10,neoBlue); 
      pixels.setPixelColor(11, neoRed);
      pixels.setPixelColor(12, neoRed);
      pixels.setPixelColor(13, neoRed);
      pixels.setPixelColor(14, neoRed);
      pixels.setPixelColor(15, neoRed);
      pixels.setPixelColor(16, neoRed);
        }
    if (aniFiveMillis - startTime >= 600 && aniFiveMillis - startTime <1650 && aniFiveFlag ==true) {
      pixels.setPixelColor(4, pixels.Color(voiceBrightness+80, voiceBrightness+80, voiceBrightness+80));
      pixels.setPixelColor(6, pixels.Color(voiceBrightness-60, voiceBrightness-60, voiceBrightness-60));
      pixels.setPixelColor(8, pixels.Color(voiceBrightness, voiceBrightness, voiceBrightness));
      pixels.setPixelColor(9, pixels.Color(voiceBrightness+80, voiceBrightness+80, voiceBrightness+80));
      pixels.setPixelColor(7, pixels.Color(voiceBrightness-60, voiceBrightness-60, voiceBrightness-60));
      pixels.setPixelColor(5, pixels.Color(voiceBrightness, voiceBrightness, voiceBrightness));
      pixels.setPixelColor(10,pixels.Color(0, 0, voiceBrightness+80)); 
      pixels.setPixelColor(11, pixels.Color(voiceBrightness+80, 0, 0));
      pixels.setPixelColor(12, pixels.Color(voiceBrightness-40, 0, 0));
      pixels.setPixelColor(13, pixels.Color(voiceBrightness+70, 0, 0));
      pixels.setPixelColor(14, pixels.Color(voiceBrightness-80, 0, 0));
      pixels.setPixelColor(15, pixels.Color(voiceBrightness, 0, 0));
      pixels.setPixelColor(16, pixels.Color(voiceBrightness-40, 0, 0));
                
      }
    if (aniFiveMillis - startTime >= 1650 && aniFiveFlag ==true) {
      pixels.setPixelColor(4, low);
      pixels.setPixelColor(6, low);
      pixels.setPixelColor(8, low);
      pixels.setPixelColor(10,voiceBright); 
      pixels.setPixelColor(11, neoRed);
      pixels.setPixelColor(12, low);
      pixels.setPixelColor(13, low);
      pixels.setPixelColor(14, low);
      pixels.setPixelColor(15, low);
      pixels.setPixelColor(16, low);
      aniFiveFlag = false; 
      return;         
    }
}

