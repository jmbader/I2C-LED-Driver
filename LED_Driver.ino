#include <Arduino.h>
#include "FastLED.h"
#include "Wire.h"

//function definitions
void clearLEDs();
void setAll(uint8_t, uint8_t, uint8_t);
void receiveEvent();

// How many leds in your strip?
#define NUM_LEDS 18

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 2

// Define the array of leds
CRGB leds[NUM_LEDS];

//stores data from i2c coms
uint8_t data[3] = { 0x00, 0x00, 0x00 };

/**
 * sets up arduino as an i2c slave
 */
void i2cSetup() {
	Wire.begin(8);                // join i2c bus with address #8
	Wire.onReceive(receiveEvent); // register event
	Serial.println("i2c setup");
}

/**
 * Function called when data is recieved over i2c
 * @param howMany bytes in i2c data
 */
void receiveEvent(int howMany) {
	while (0 < Wire.available()) {
		uint8_t curr_byte = Wire.read(); // receive byte
		data[Wire.available()] = curr_byte;//puts byte in array
	}
}

void setup() {
	Serial.begin(9600);

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

	i2cSetup();
	Serial.println("Setup done");
}

uint8_t red = 255;
uint8_t green = 0;
uint8_t blue = 0;

void loop() {
	setAll(data[0], data[1], data[2]);
	FastLED.show();
	delay(20);
}

/**
 * Sets all LEDs in the strip to the color given
 * @param t_red amount of red 0 - 255
 * @param t_green amount of green 0 - 255
 * @param t_blue amount of blue 0 - 255
 */
void setAll(uint8_t t_red, uint8_t t_green, uint8_t t_blue) {
	for (int curr_led = 0; curr_led < NUM_LEDS; curr_led++) {
		leds[curr_led] = CRGB(t_red, t_green, t_blue);
		leds[curr_led].nscale8_video(20);
	}
	FastLED.show();
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

void back_and_forth() {
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

