#ifndef REGISTER_H
#define REGISTER_H

#define register_size 8
extern uint8_t data_register[8];

uint8_t write_register(uint8_t, uint8_t);
uint8_t read_register(uint8_t);
void print_data_register();

#endif REGISTER_H

