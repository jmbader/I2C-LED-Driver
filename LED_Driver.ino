#include <Arduino.h>
#include "FastLED.h"

//function definitions
void clearLEDs();

// How many leds in your strip?
#define NUM_LEDS 18

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 2

// Define the array of leds
CRGB leds[NUM_LEDS];

//LED brightness
uint8_t brightness = 255/4;

void setup() {
	// Uncomment/edit one of the following lines for your leds arrangement.
	// FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
	// FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
	// FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
	// FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
	// FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
	// FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
	FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
	// FastLED.addLeds<APA104, DATA_PIN, RGB>(leds, NUM_LEDS);
	// FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
	// FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);
	// FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUM_LEDS);
	// FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUM_LEDS);

	clearLEDs();
}

uint8_t red = 255;
uint8_t green = 0;
uint8_t blue = 0;

void loop() {

	for (int curr_led = 0; curr_led < NUM_LEDS; curr_led++) {

		leds[curr_led] = CRGB(red, green, blue);
		leds[curr_led].nscale8_video(50);

		if (curr_led > 0) {
			leds[curr_led - 1] = CRGB::Black;
			delay(50);
		}

		FastLED.show();
		red -= 14;
		green += 14;
	}

	for (int curr_led = NUM_LEDS - 1; curr_led >= 0; curr_led--) {

		leds[curr_led] = CRGB(red, green, blue);
		leds[curr_led].nscale8_video(50);

		if (curr_led < NUM_LEDS - 1) {
			leds[curr_led + 1] = CRGB::Black;
			delay(50);
		}

		FastLED.show();
		red += 14;
		green -= 14;
	}
}

/**
 * Turns off all of the leds in the CRGB object called 'leds'
 */
void clearLEDs() {
	for (int curr_led = 0; curr_led < NUM_LEDS; curr_led++) {
		leds[curr_led] = CRGB::Black;
	}
	FastLED.show();
}

