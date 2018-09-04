#include <Arduino.h>
#include "FastLED.h"
#include "Wire.h"
#include "Register.h"

//function definitions
void clearLEDs();
void setAll(uint8_t, uint8_t, uint8_t);
void receiveEvent(int);
void back_and_forth(uint8_t);
void moving_rainbow();

// the max number of leds we support
#define MAX_LEDS 500

// How many leds in your strip?
// #define NUM_LEDS 60
uint16_t NUM_LEDS = MAX_LEDS;

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN PIN_D2

// Define the array of leds
CRGB leds[MAX_LEDS];

//stores data from i2c coms
uint8_t data[3] = { 0x00, 0x00, 0x00 };

uint8_t led_pin_board = 13;

/**
   sets up arduino as an i2c slave
*/
void i2cSetup() {
  Wire.setClock(400000);
  Wire.begin(8); // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.println("i2c has been setup");
}

/**
   Function called when data is received over i2c
   @param howMany bytes in i2c data
*/
void receiveEvent(int howMany) {
  while (2 < Wire.available()) {
    Wire.read();
  }
  while (0 < Wire.available()) {
    digitalWrite(led_pin_board, HIGH);
    uint8_t curr_reg = Wire.read(); // receive byte
    uint8_t curr_data = Wire.read(); // receive byte
    Serial.print(curr_reg);
    Serial.print(", ");
    Serial.println(curr_data);
    write_register(curr_reg, curr_data);
  }
  digitalWrite(led_pin_board, LOW);
}

void setup() {
  Serial.begin(9600);

  // Uncomment/edit one of the following lines for your leds arrangement.
  // FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  // FastLED.addLeds<APA104, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUM_LEDS);
  pinMode(led_pin_board, OUTPUT);
  digitalWrite(led_pin_board, LOW);

  i2cSetup();
  Serial.println("setup() done");
}

boolean interrupt = false;

void loop() {
  interrupt = false;
  //Serial.println("alive");
  uint8_t mode = read_register(REGISTER_MODE);
  uint8_t colorMode = read_register(REGISTER_COLOR_MODE);

  switch (mode) {
    case 0x00: {
        //all off
        clearLEDs();
        break;
      }
    case 0x01: {
        if (colorMode == 0x01) {
          uint8_t hue = read_register(0x04);
          uint8_t sat = read_register(0x05);
          uint8_t val = read_register(0x06);
          setAllHSV(hue, sat, val);
        } else {
          //solid color rgb
          uint8_t red = read_register(0x04);
          uint8_t green = read_register(0x05);
          uint8_t blue = read_register(0x06);
          setAll(red, green, blue);
        }
        break;
      }
    case 0x02: {
        uint8_t wait = read_register(REGISTER_DATA_3);
        CRGB color;
        if (colorMode == 0x01) {
          uint8_t hue = read_register(0x04);
          uint8_t sat = read_register(0x05);
          uint8_t val = read_register(0x06);
          color = CHSV(hue, sat, val);
        } else {
          //solid color rgb
          uint8_t red = read_register(0x04);
          uint8_t green = read_register(0x05);
          uint8_t blue = read_register(0x06);
          color = CRGB(red, green, blue);
        }
        knightRider(color, wait, 4);
          break;
      }
    case 0x03: {
        uint8_t wait = read_register(REGISTER_DATA_3);
        knightRiderRainbow(wait, 4);
        break;
      }
    case 0x04: {
        uint8_t wait = read_register(REGISTER_DATA_3);
        fullRainbow(wait);
        break;
      }
  }

  uint8_t brightness = read_register(REGISTER_BRIGHTNESS);
  FastLED.setBrightness(brightness);

  uint8_t ledCount0 = read_register(REGISTER_LED_COUNT_0);
  uint8_t ledCount1 = read_register(REGISTER_LED_COUNT_1);
  NUM_LEDS = ledCount0 + ledCount1 * 256;

  FastLED.show();
}

uint8_t previousMode = 0x00;
uint8_t previousBrighness = 0;

/**
   Checks for a change in mode to interrupt the current animation. Also polls brightness
   This permits usage of loops in animation methods, provided it calls this method
   and makes appropriate checks on the interrupt flag
*/
void checkChange() {
  uint8_t mode = read_register(REGISTER_MODE);
  if (mode != previousMode) {
    interrupt = true;
    previousMode = mode;
  }

  uint8_t brightness = read_register(REGISTER_BRIGHTNESS);
  // only update if it actually changed
  if (brightness != previousBrighness) {
    FastLED.setBrightness(brightness);
    previousBrighness = brightness;
  }
}

/**
   Sets all LEDs in the strip to the color given
   @param t_red amount of red 0 - 255
   @param t_green amount of green 0 - 255
   @param t_blue amount of blue 0 - 255
*/
void setAll(uint8_t t_red, uint8_t t_green, uint8_t t_blue) {
  for (int curr_led = 0; curr_led < NUM_LEDS; curr_led++) {
    leds[curr_led] = CRGB(t_red, t_green, t_blue);
  }
  FastLED.show();
}

void setAllHSV(uint8_t t_hue, uint8_t t_sat, uint8_t t_val) {
  for (int curr_led = 0; curr_led < NUM_LEDS; curr_led++) {
    leds[curr_led] = CHSV(t_hue, t_sat, t_val);
  }
  FastLED.show();
}

/**
   Turns off all of the leds in the CRGB object called 'leds'
*/
void clearLEDs() {
  for (int curr_led = 0; curr_led < NUM_LEDS; curr_led++) {
    leds[curr_led] = CRGB::Black;
  }
  FastLED.show();
}

void back_and_forth(uint8_t t_speed) {
  uint8_t red = 0, green = 0, blue = 0;
  for (int curr_led = 0; curr_led < NUM_LEDS; curr_led++) {
    leds[curr_led] = CRGB(red, green, blue);

    if (curr_led > 0) {
      leds[curr_led - 1] = CRGB::Black;
      delay(t_speed);
    }

    FastLED.show();
    red -= 14;
    green += 14;
  }

  for (int curr_led = NUM_LEDS - 1; curr_led >= 0; curr_led--) {
    leds[curr_led] = CRGB(red, green, blue);

    if (curr_led < NUM_LEDS - 1) {
      leds[curr_led + 1] = CRGB::Black;
      delay(t_speed);
    }

    FastLED.show();
    red += 14;
    green -= 14;
  }
}

void moving_rainbow() {
  while (1) {
    uint8_t hue = 0;
    while (hue < 213) {
      fill_gradient(leds, 0, CHSV(hue, 255, 255), NUM_LEDS, CHSV(213 - hue, 255, 255), BACKWARD_HUES);
      FastLED.show();
      delay(20);
      hue++;
    }
  }
}

void fullRainbow(uint8_t wait) {
  for (int startHue = 255; startHue > 0; startHue--) {
    for (int i = 0; i < NUM_LEDS; i++) {
      int hue = startHue + float(i) / float(NUM_LEDS) * 255;
      while (hue > 255) {
        hue -= 256;
      }
      leds[i] = CHSV(hue, 255, 255);
    }
    FastLED.show();
    checkChange();
    if (interrupt) {
      return;
    }
    delay(wait);
  }
}

void knightRider(CRGB color, int wait, int num) {
  for (int i = 0; i <= NUM_LEDS - num; i++) {
    FastLED.clear();
    for (int j = 0; j < num; j++) {
      leds[i + j] = color;
    }
    FastLED.show();
    checkChange();
    if (interrupt) {
      return;
    }
    delay(wait);
  }
  for (int i = NUM_LEDS - 2; i > num; i--) {
    FastLED.clear();
    for (int j = 0; j < num; j++) {
      leds[i - j] = color;
    }
    FastLED.show();
    checkChange();
    if (interrupt) {
      return;
    }
    delay(wait);
  }
}

void knightRiderRainbow(int wait, int num) {
  for(int startHue = 255; startHue > 0; startHue--) {
    for (int i = 0; i <= NUM_LEDS - num; i++) {
      FastLED.clear();
      for (int j = 0; j < num; j++) {
        int hue = startHue + float(i+j) / float(NUM_LEDS) * 255;
        while(hue > 255) {
          hue-=256;
        }
        leds[i + j] = CHSV(hue, 255, 255);
      }
      FastLED.show();
      checkChange();
      if (interrupt) {
        return;
      }
      delay(wait);
    }
    for (int i = NUM_LEDS - 2; i > num; i--) {
      FastLED.clear();
      for (int j = 0; j < num; j++) {
        int hue = startHue + float(i - j) / float(NUM_LEDS) * 255;
        while(hue > 255) {
          hue-=256;
        }
        leds[i - j] = CHSV(hue, 255, 255);
      }
      FastLED.show();
      checkChange();
      if (interrupt) {
        return;
      }
      delay(wait);
    }
  }
}

