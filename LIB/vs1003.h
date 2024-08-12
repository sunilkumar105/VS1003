/*
 * vs1003.h
 *
 *  Created on: Jul 25, 2024
 *      Author: WB07_24
 */

#ifndef INC_VS1003_H_
#define INC_VS1003_H_

#include "main.h"

// GPIO PINS
#define VS1003_CS_PORT CS_GPIO_Port
#define VS1003_CS_PIN CS_Pin
#define VS1003_DCS_PORT DCS_GPIO_Port
#define VS1003_DCS_PIN DCS_Pin
#define VS1003_DREQ_PORT DREQ_GPIO_Port
#define VS1003_DREQ_PIN DREQ_Pin
#define VS1003_RESET_PORT RST_GPIO_Port
#define VS1003_RESET_PIN  RST_Pin

//FUNCTION DECLARAION
void delayMicroseconds(uint8_t time);
void vs1003_await_data_request(void);
void vs1003_control_mode_on(void);
void vs1003_control_mode_off(void);
void vs1003_data_mode_on(void);
void vs1003_data_mode_off(void);

uint16_t vs1003_read_register(uint8_t _reg);
void vs1003_write_register(uint8_t _reg, uint16_t _value);
void vs1003_sdi_send_buffer(uint8_t *data, size_t len);
void vs1003_sdi_send_zeroes(size_t length);
void vs1003_print_register_data(uint8_t reg);
size_t find_min(size_t a, size_t b);

void vs1003_begin(void);
void vs1003_startSong(void);
void vs1003_playChunk(uint8_t *data, size_t len);
void vs1003_stopSong(void);
void vs1003_print_all_register_details(void);
void vs1003_setVolume(uint8_t vol);

#endif /* INC_VS1003_H_ */
