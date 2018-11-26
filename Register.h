#ifndef REGISTER_H
#define REGISTER_H

#define register_size 10
extern uint8_t data_register[register_size];

#define REGISTER_STATUS       0x00
#define REGISTER_LED_COUNT_0  0x01
#define REGISTER_LED_COUNT_1  0x08
#define REGISTER_MODE         0x02
#define REGISTER_BRIGHTNESS   0x03
#define REGISTER_DATA_0       0x04
#define REGISTER_DATA_1       0x05
#define REGISTER_DATA_2       0x06
#define REGISTER_DATA_3       0x07
#define REGISTER_COLOR_MODE   0x09

uint8_t write_register(uint8_t, uint8_t);
uint8_t read_register(uint8_t);
void print_data_register();

#endif
