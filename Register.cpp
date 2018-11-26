/*
 * Register.cpp
 *
 *  Created on: May 28, 2018
 *      Author: Bader
 */
#include "Arduino.h"
#include "Register.h"

#define DEBUG true

uint8_t data_register[register_size];

/**
 * writes data to the address
 * @param t_address the address to change data at
 * @param t_value the value to write to the address
 * @return the data written, 0 if the address is not available
 */
uint8_t write_register(uint8_t t_address, uint8_t t_value) {
	//check if address was in range
	if (t_address >= register_size) {
		if (DEBUG)
			Serial.println(F("Given register out of bound"));
		return 0;
	} else {
		//register address valid
		data_register[t_address] = t_value;
		if (DEBUG) {
			char buff[40];
			sprintf(buff, "DATA: %X, written to REG: %X", t_value, t_address);
			Serial.println(buff);
		}
		return t_value;
	}
}

/**
 * Reads the data at the address given
 * @param t_address the address of data to access
 * @return the data at the given address, 0 if address not available
 */
uint8_t read_register(uint8_t t_address) {
	uint8_t value = 0;
	//check if address was in range
	if (t_address >= register_size) {
		if (DEBUG)
			Serial.println(F("Given register out of bound"));
	} else {
		//register address valid
		value = data_register[t_address];

		if (DEBUG) {
			char buff[40];
			sprintf(buff, "REG: %X\t has DATA: %X", value, t_address);
			//Serial.println(buff);
		}

	}
	return value;
}

/**
 * Prints all of the values in the data_register
 */
void print_data_register() {
	Serial.println(F("\ndata_register print:"));
	for (uint8_t curr_addr = 0; curr_addr < register_size; curr_addr++) {
		char buff[24];
		uint8_t value = data_register[curr_addr];
		sprintf(buff, "REG: %X\t DATA: %X", curr_addr, value);
		Serial.println(buff);
	}
	Serial.print(F("\n"));//add a new line at the end
}

