#include <Arduino.h>
#include <FastLED.h>
#include <Wire.h>
#include "Register.h"

#define DEBUG 0

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE     Perform a factory reset when running this sketch
   
                            Enabling this will put your Bluefruit LE module
                            in a 'known good' state and clear any config
                            data set in previous sketches or projects, so
                            running this at least once is a good idea.
   
                            When deploying your project, however, you will
                            want to disable factory reset by setting this
                            value to 0.  If you are making changes to your
                            Bluefruit LE device via AT commands, and those
                            changes aren't persisting across resets, this
                            is the reason why.  Factory reset will erase
                            the non-volatile memory where config data is
                            stored, setting it back to factory default
                            values.
       
                            Some sketches that require you to bond to a
                            central device (HID mouse, keyboard, etc.)
                            won't work at all with this feature enabled
                            since the factory reset will clear all of the
                            bonding data stored on the chip, meaning the
                            central device won't be able to reconnect.
                            
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features    
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE        0
    #define MINIMUM_FIRMWARE_VERSION   "0.7.0"
/*=========================================================================*/

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

//function definitions
void clearLEDs();
void setAll(uint8_t, uint8_t, uint8_t);
void receiveEvent(int);
void back_and_forth(uint8_t);
void moving_rainbow();

// the max number of leds we support
//128 LEDs for pro mini ATmega168 5V 16MHz
#define MAX_LEDS 300 

// How many leds in your strip?
uint16_t NUM_LEDS = 30;

// this is being tested on a two-strip setup
#define NUM_LEDS_STRIP_ONE 11
#define NUM_LEDS_STRIP_TWO 18

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 2

// Define the array of leds
CRGB leds[MAX_LEDS];

byte led_pin_board = 11;//onboard led so you can see I2C comms

byte i2c_disable_pin = 9;

/*
 * Helper functions for leds
 */
 
void reverse() {
  uint8_t left = 0;
  uint8_t right = NUM_LEDS_STRIP_ONE-1;
  while (left < right) {
    CRGB temp = leds[left];
    leds[left++] = leds[right];
    leds[right--] = temp;
  }
}

void show() {
  reverse();
  FastLED.show();
  reverse();
}

void clear() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

/*
 * Bluetooth callbacks and whatnot
 */

int32_t charid_string;
int32_t charid_number;

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

boolean bleConnected = false;

void connected(void)
{
  Serial.println( F("Connected") );
  bleConnected = true;
}

void disconnected(void)
{
  Serial.println( F("Disconnected") );
  bleConnected = false;
}

void BleUartRX(char data[], uint16_t len)
{
  Serial.print( F("[BLE UART RX] " ) );
  Serial.println(len);
  digitalWrite(led_pin_board, HIGH);

  for(int i = 0; i+3 < len; i+=4) {
    int8_t curr_reg = data[i]+(255-data[i+1]); // receive byte
    uint8_t curr_data = data[i+2]+(255-data[i+3]); // receive byte
    Serial.print(curr_reg);
    Serial.print(", ");
    Serial.println(curr_data);
    write_register(curr_reg, curr_data);
  }
    
  digitalWrite(led_pin_board, LOW);
  
  //Serial.print(len);
  //Serial.write(data, len);
//  if(data[0] <= 1) {
//    Serial.write(data[0]);
//    Serial.write(data[1]);
//    if(data[0] == 0) {
//      clear();
//      mode = data[1];
//    }
//    if(data[0] == 1 && data[1] >= 0 && data[1] <= 255) {
//      brightness = data[1];
//    }
//  } else {
//    Serial.write(data, len);
//  }
  Serial.println();
}

void BleGattRX(int32_t chars_id, uint8_t data[], uint16_t len)
{
  Serial.print( F("[BLE GATT RX] (" ) );
  Serial.print(chars_id);
  Serial.print(") ");
  
  if (chars_id == charid_string)
  {  
    Serial.write(data, len);
    Serial.println();
  }else if (chars_id == charid_number)
  {
    int32_t val;
    memcpy(&val, data, len);
    Serial.println(val);
  }
}


/**
   sets up arduino as an i2c slave
*/
void i2cSetup() {
  Wire.setClock(100000);
  Wire.begin(8); // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.println(F("i2c has been setup"));
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
  //FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
//  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, PIN_D6>(leds, 0, NUM_LEDS_STRIP_ONE);
  FastLED.addLeds<NEOPIXEL, PIN_D2>(leds, NUM_LEDS_STRIP_ONE, NUM_LEDS_STRIP_TWO);
  // FastLED.addLeds<APA104, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUM_LEDS);
  pinMode(led_pin_board, OUTPUT);
  digitalWrite(led_pin_board, LOW);

//  pinMode(i2c_disable_pin, INPUT_PULLUP); // default high
  //if pin is high i2c is enable
  //if pin is low no i2c, go to predefined mode
//  boolean i2c_enabled = digitalRead(i2c_disable_pin);
//
//  if(i2c_enabled){
//    Serial.println("I2C Enabled");
//    i2cSetup();
//  }else{
//    Serial.println("I2C Disabled");
//    write_register(REGISTER_MODE, 0x01);
//    write_register(REGISTER_COLOR_MODE, 0x00);//RGB mode
//    write_register(REGISTER_DATA_0, 0xFF);//red at 255
//    write_register(REGISTER_BRIGHTNESS, 0x0F);//set brgihtness
//  }
  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  
  if ( !ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    error( F("Callback requires at least 0.7.0") );
  }
  
  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=Ski One" )) ) {
    error(F("Could not set device name?"));
  }

  Serial.println( F("Adding Service 0x1234 with 2 chars 0x2345 & 0x6789") );
  ble.sendCommandCheckOK( F("AT+GATTADDSERVICE=uuid=0x1234") );
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2345,PROPERTIES=0x08,MIN_LEN=1,MAX_LEN=6,DATATYPE=string,DESCRIPTION=string,VALUE=abc"), &charid_string);
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x6789,PROPERTIES=0x08,MIN_LEN=4,MAX_LEN=4,DATATYPE=INTEGER,DESCRIPTION=number,VALUE=0"), &charid_number);

  ble.reset();

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
  
  /* Set callbacks */
  ble.setConnectCallback(connected);
  ble.setDisconnectCallback(disconnected);
  ble.setBleUartRxCallback(BleUartRX);
  
  /* Only one BLE GATT function should be set, it is possible to set it 
  multiple times for multiple Chars ID  */
  ble.setBleGattRxCallback(charid_string, BleGattRX);
  ble.setBleGattRxCallback(charid_number, BleGattRX);
  Serial.println("setup() done");
}

boolean interrupt = false;
enum Modes{all_off, all_one_color, knight_rider, knight_rider_rainbow, rainbow, candycane};

void loop() {
  if(DEBUG){
      Serial.println(F("alive"));
  }

  ble.update(200);
  
  interrupt = false;
  enum Modes mode = (Modes) read_register(REGISTER_MODE);
  uint8_t colorMode = read_register(REGISTER_COLOR_MODE);

  uint8_t brightness = read_register(REGISTER_BRIGHTNESS);
  FastLED.setBrightness(brightness);

  uint8_t ledCount0 = read_register(REGISTER_LED_COUNT_0);
  uint8_t ledCount1 = read_register(REGISTER_LED_COUNT_1);
  NUM_LEDS = ledCount0 + ledCount1 * 256;

  switch (mode) {
    case all_off: {
        //all off
        clearLEDs();
        break;
      }
    case all_one_color: {
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
    case knight_rider: {
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
    case knight_rider_rainbow: {
        uint8_t wait = read_register(REGISTER_DATA_3);
        knightRiderRainbow(wait, 4);
        break;
      }
    case rainbow: {
        uint8_t wait = read_register(REGISTER_DATA_3);
        fullRainbow(wait);
        break;
      }
    case candycane: {
      uint8_t width = read_register(REGISTER_DATA_0);
      uint8_t wait = read_register(REGISTER_DATA_3);
      candyCane(wait, width);
      break;
    }
  }

  show();
}

uint8_t previousMode = 0x00;
uint8_t previousBrighness = 0;

/**
   Checks for a change in mode to interrupt the current animation. Also polls brightness
   This permits usage of loops in animation methods, provided it calls this method
   and makes appropriate checks on the interrupt flag
*/
void checkChange() {
  ble.update(200);
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
      delay(read_register(REGISTER_DATA_3));
    }

    show();
    red -= 14;
    green += 14;
  }

  for (int curr_led = NUM_LEDS - 1; curr_led >= 0; curr_led--) {
    leds[curr_led] = CRGB(red, green, blue);

    if (curr_led < NUM_LEDS - 1) {
      leds[curr_led + 1] = CRGB::Black;
      delay(read_register(REGISTER_DATA_3));
    }

    show();
    red += 14;
    green -= 14;
  }
}

void moving_rainbow() {
  while (1) {
    uint8_t hue = 0;
    while (hue < 213) {
      fill_gradient(leds, 0, CHSV(hue, 255, 255), NUM_LEDS, CHSV(213 - hue, 255, 255), BACKWARD_HUES);
      show();
      delay(read_register(REGISTER_DATA_3));
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
    show();
    checkChange();
    if (interrupt) {
      return;
    }
    delay(read_register(REGISTER_DATA_3));
  }
}

void knightRider(CRGB color, int wait, int num) {
  for (int i = 0; i <= NUM_LEDS - num; i++) {
    FastLED.clear();
    for (int j = 0; j < num; j++) {
      leds[i + j] = color;
    }
    show();
    checkChange();
    if (interrupt) {
      return;
    }
    delay(read_register(REGISTER_DATA_3));
  }
  for (int i = NUM_LEDS - 2; i > num; i--) {
    FastLED.clear();
    for (int j = 0; j < num; j++) {
      leds[i - j] = color;
    }
    show();
    checkChange();
    if (interrupt) {
      return;
    }
    delay(read_register(REGISTER_DATA_3));
  }
}

void knightRiderRainbow(int wait, int num) {
  for (int startHue = 255; startHue > 0; startHue--) {
    for (int i = 0; i <= NUM_LEDS - num; i++) {
      FastLED.clear();
      for (int j = 0; j < num; j++) {
        int hue = startHue + float(i + j) / float(NUM_LEDS) * 255;
        while (hue > 255) {
          hue -= 256;
        }
        leds[i + j] = CHSV(hue, 255, 255);
      }
      show();
      checkChange();
      if (interrupt) {
        return;
      }
      delay(read_register(REGISTER_DATA_3));
    }
    for (int i = NUM_LEDS - 2; i > num; i--) {
      FastLED.clear();
      for (int j = 0; j < num; j++) {
        int hue = startHue + float(i - j) / float(NUM_LEDS) * 255;
        while (hue > 255) {
          hue -= 256;
        }
        leds[i - j] = CHSV(hue, 255, 255);
      }
      show();
      checkChange();
      if (interrupt) {
        return;
      }
      delay(read_register(REGISTER_DATA_3));
    }
  }
}

void candyCane(int wait, int segmentSize) {
  for (int i = -segmentSize*2 + 1; i < 0; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        //Serial.println(j);
        checkChange();
        if (interrupt) {
          return;
        }
        if((j+segmentSize*2+i) % (segmentSize*2) >= segmentSize) {
          //Serial.println("red");
          if(j < NUM_LEDS) leds[j] = CRGB::Red;
        } else {
          //Serial.println("white");
          if(j < NUM_LEDS) leds[j] = CRGB::White;
        }
      }
      show();
      delay(read_register(REGISTER_DATA_3));
  }
}

