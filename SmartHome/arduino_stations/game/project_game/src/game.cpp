//***************** 8x8 -> pins 567 **********************//
//***************** Color strip -> pin 8 **********************//
//***************** speaker pin 9 **********************//
//***************** S1 pin 10 **********************//
//***************** S2 pin A3 **********************//
//***************** S3 pin 2 **********************//
//***************** S4 pin 3 **********************//

#include <Arduino.h>
#include <FastLED.h>
#include <MaxMatrix.h>
#include <PinChangeInt.h>

// pin change interrupts groups are:
// A0 to A5
// D0 to D7
// D8 to D13
// So using 2 groups pin 7 on one group and pin A3 on the other group
// This way I do not have to ready pin state to make sure which one was changed.
#define SWITCH3_PIN_CHANGE_INTERRUPT 10
#define SWITCH4_PIN_CHANGE_INTERRUPT A3
// interrupts from touch button
#define interruptTouch1Pin 2
#define interruptTouch2Pin 3

int DIN = 7; // DIN pin of MAX7219 module
int CLK = 6; // CLK pin of MAX7219 module
int CS = 5;  // CS pin of MAX7219 module
int maxInUse = 1;
MaxMatrix m(DIN, CS, CLK, maxInUse);

char ONE[] = {
    3, 8, B01000010, B01111111, B01000000, B00000000, B00000000, // 1
};

char TWO[] = {
    4, 8, B01100010, B01010001, B01001001, B01000110, B00000000, // 2
};

char THREE[] = {
    4, 8, B00100010, B01000001, B01001001, B00110110, B00000000, // 3
};

#define LED_PIN 8
#define NUM_LEDS 12
#define BRIGHTNESS 64
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100
CRGBPalette16 currentPalette;
TBlendType currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

int speakerOut = 9;
int melody[] = {C, b, g, C, b, e, R, C, c, g, a, C};
int beats[] = {16, 16, 16, 8, 8, 16, 32, 16, 16, 16, 8, 8};
int MAX_COUNT = sizeof(melody) / 2; // Melody length, for looping.

// Set overall tempo
long tempo = 10000;
// Set length of pause between notes
int pause = 1000;
// Loop variable to increase Rest length
int rest_count = 100; //<-BLETCHEROUS HACK; See NOTES

// Initialize core variables
int tone_ = 0;
int beat = 0;
long duration = 0;

// PLAY TONE  ==============================================
// Pulse the speaker to play a tone for a particular duration
void playTone() {
    long elapsed_time = 0;
    if (tone_ > 0) { // if this isn't a Rest beat, while the tone has
        //  played less long than 'duration', pulse speaker HIGH and LOW
        while (elapsed_time < duration) {

            digitalWrite(speakerOut, HIGH);
            delayMicroseconds(tone_ / 2);

            // DOWN
            digitalWrite(speakerOut, LOW);
            delayMicroseconds(tone_ / 2);

            // Keep track of how long we pulsed
            elapsed_time += (tone_);
        }
    } else {                                   // Rest beat; loop times delay
        for (int j = 0; j < rest_count; j++) { // See NOTE on rest_count
            delayMicroseconds(duration);
        }
    }
}

void playMusic() {
    for (int i = 0; i < MAX_COUNT; i++) {
        tone_ = melody[i];
        beat = beats[i];

        duration = beat * tempo; // Set up timing

        playTone();
        // A pause between notes...
        delayMicroseconds(pause);
    }
}
// #define rfCE 9
// #define rfCS 10
// #define rfMISO 12
// #define rfMOSI 11
// #define rfCLK 13

// pin change interrupts groups are:
// A0 to A5
// D0 to D7
// D8 to D13
// So using 2 groups pin 7 on one group and pin A3 on the other group
// This way I do not have to ready pin state to make sure which one was changed.

void setAllColor(CRGB::HTMLColorCode color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
    FastLED.show();
}
void FillLEDsFromPaletteColors(uint8_t colorIndex) {
    uint8_t brightness = 255;

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness,
                                   currentBlending);
        colorIndex += 3;
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette() {
    for (int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV(random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette() {
    // 'black out' all 16 palette entries...
    fill_solid(currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette() {
    CRGB purple = CHSV(HUE_PURPLE, 255, 255);
    CRGB green = CHSV(HUE_GREEN, 255, 255);
    CRGB black = CRGB::Black;

    currentPalette =
        CRGBPalette16(green, green, black, black, purple, purple, black, black,
                      green, green, black, black, purple, purple, black, black);
}
// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p,
// RainbowStripeColors_p, OceanColors_p, CloudColors_p, LavaColors_p,
// ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can
// write code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically() {
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if (lastSecond != secondHand) {
        lastSecond = secondHand;
        if (secondHand == 0) {
            currentPalette = RainbowColors_p;
            currentBlending = LINEARBLEND;
        }
        if (secondHand == 10) {
            currentPalette = RainbowStripeColors_p;
            currentBlending = NOBLEND;
        }
        if (secondHand == 15) {
            currentPalette = RainbowStripeColors_p;
            currentBlending = LINEARBLEND;
        }
        if (secondHand == 20) {
            SetupPurpleAndGreenPalette();
            currentBlending = LINEARBLEND;
        }
        if (secondHand == 25) {
            SetupTotallyRandomPalette();
            currentBlending = LINEARBLEND;
        }
        if (secondHand == 30) {
            SetupBlackAndWhiteStripedPalette();
            currentBlending = NOBLEND;
        }
        if (secondHand == 35) {
            SetupBlackAndWhiteStripedPalette();
            currentBlending = LINEARBLEND;
        }
        if (secondHand == 40) {
            currentPalette = CloudColors_p;
            currentBlending = LINEARBLEND;
        }
        if (secondHand == 45) {
            currentPalette = PartyColors_p;
            currentBlending = LINEARBLEND;
        }
        if (secondHand == 50) {
            currentPalette = myRedWhiteBluePalette_p;
            currentBlending = NOBLEND;
        }
        if (secondHand == 55) {
            currentPalette = myRedWhiteBluePalette_p;
            currentBlending = LINEARBLEND;
        }
    }
}

// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM = {
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue, CRGB::Black,

    CRGB::Red,  CRGB::Gray,  CRGB::Blue,  CRGB::Black,

    CRGB::Red,  CRGB::Red,   CRGB::Gray,  CRGB::Gray,
    CRGB::Blue, CRGB::Blue,  CRGB::Black, CRGB::Black};

// Additionl notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.

// char A[] = {
//     4, 8, B01111110, B00010001, B00010001, B01111110,
// };

void onS1() {}
void onS2() {}
void onS3() {}
void onS4() {}

void initInterrupts() {
    noInterrupts();
    pinMode(interruptTouch1Pin, INPUT_PULLUP);
    pinMode(interruptTouch2Pin, INPUT_PULLUP);
    pinMode(SWITCH3_PIN_CHANGE_INTERRUPT, INPUT);
    pinMode(SWITCH4_PIN_CHANGE_INTERRUPT, INPUT);
    attachInterrupt(digitalPinToInterrupt(interruptTouch1Pin), onS1, RISING);
    attachInterrupt(digitalPinToInterrupt(interruptTouch2Pin), onS2, RISING);
    // attachPinChangeInterrupt(SWITCH3_PIN_CHANGE_INTERRUPT, onS3, CHANGE);
    // attachPinChangeInterrupt(SWITCH4_PIN_CHANGE_INTERRUPT, onS4, CHANGE);
    attachPinChangeInterrupt(SWITCH3_PIN_CHANGE_INTERRUPT, onS3, RISING);
    attachPinChangeInterrupt(SWITCH4_PIN_CHANGE_INTERRUPT, onS4, RISING);
    interrupts();
}

void setup() {
    initInterrupts();
    m.init();          // MAX7219 initialization
    m.setIntensity(8); // initial led matrix intensity, 0-15

    delay(3000); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}

void loop() {

    m.clear();
    // Displaying the character at x,y (upper left corner of the character)
    m.writeSprite(2, 0, ONE);
    for (int i = 0; i < 8; i++) {
        m.shiftLeft(false, false);
        delay(100);
    }

    ChangePalettePeriodically();

    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */

    FillLEDsFromPaletteColors(startIndex);

    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
    //  delay(100);
    // return;
    // while (!Mirf.dataReady())
    // {
    //   //Serial.print(".");
    //   watchdogReset();
    //   blinkReady();
    // };

    // //Serial.print("TICK");
    // checkIfOtaRequestOrLoadCommand(cmd);
    // Serial.print(F("New command "));
    // Serial.println(cmd);
    // if (strcmp(cmd, "001") == 0)
    // {
    //   Serial.print("remtoe restart");
    //   soft_restart();
    //   return;
    // }
    // //s1
    // else if (strcmp(cmd, "002") == 0)
    // {
    //   return;
    // }
}
