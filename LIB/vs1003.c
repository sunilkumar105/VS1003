/*
 * vs1003.c
 *
 *  Created on: Jul 25, 2024
 *      Author: WB07_24
 */

#include "vs1003.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

//EXTERN VARIABVLES
extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart2;

const uint8_t vs1003_chunk_size = 32;

// VS1003 SCI Write Command byte is 0x02
uint8_t VS_WRITE_COMMAND = 0x02;

// VS1003 SCI Read COmmand byte is 0x03
uint8_t VS_READ_COMMAND = 0x03;

// SCI Registers

const uint8_t SCI_MODE = 0x0;
const uint8_t SCI_STATUS = 0x1;
const uint8_t SCI_BASS = 0x2;
const uint8_t SCI_CLOCKF = 0x3;
const uint8_t SCI_DECODE_TIME = 0x4;
const uint8_t SCI_AUDATA = 0x5;
const uint8_t SCI_WRAM = 0x6;
const uint8_t SCI_WRAMADDR = 0x7;
const uint8_t SCI_HDAT0 = 0x8;
const uint8_t SCI_HDAT1 = 0x9;
const uint8_t SCI_AIADDR = 0xa;
const uint8_t SCI_VOL = 0xb;
const uint8_t SCI_AICTRL0 = 0xc;
const uint8_t SCI_AICTRL1 = 0xd;
const uint8_t SCI_AICTRL2 = 0xe;
const uint8_t SCI_AICTRL3 = 0xf;
const uint8_t SCI_num_registers = 0xf;

// SCI_MODE bits

const uint8_t SM_DIFF = 0;
const uint8_t SM_LAYER12 = 1;
const uint8_t SM_RESET = 2;
const uint8_t SM_OUTOFWAV = 3;
const uint8_t SM_EARSPEAKER_LO = 4;
const uint8_t SM_TESTS = 5;
const uint8_t SM_STREAM = 6;
const uint8_t SM_EARSPEAKER_HI = 7;
const uint8_t SM_DACT = 8;
const uint8_t SM_SDIORD = 9;
const uint8_t SM_SDISHARE = 10;
const uint8_t SM_SDINEW = 11;
const uint8_t SM_ADPCM = 12;
const uint8_t SM_ADCPM_HP = 13;
const uint8_t SM_LINE_IN = 14;

/*
 * Currently delaying in milliseconds, Change it to hardware timer to microseconds later
 * @param time
 * @TODO
 * */
void delayMicroseconds(uint8_t time) {
	HAL_Delay(time);
}
/*
 * @brief Wait unti-ll DREQ pin is LOW
 * REFER SECTION 7.3 OF DATASHEET
 * if DREQ is high then we can send 32 byte of SDI and 1 SCI command, if it is LOW we can't send any data on either of the bus
 * */
void vs1003_await_data_request(void) {
	while (!(HAL_GPIO_ReadPin(VS1003_DREQ_PORT, VS1003_DREQ_PIN)))
		;
}
/*
 * @brief enabling chip select pin for writing SCI [SERIAL CONTROL INTERFACE] Command
 *Turn DCS for SDA [SERIAL DATA INTERFACE] HIGH ~ it is disabled
 *Turn CS for SCI LOW ~ it is Enabled
 * */
void vs1003_control_mode_on(void) {
	HAL_GPIO_WritePin(VS1003_DCS_PORT, VS1003_DCS_PIN, 1);
	HAL_GPIO_WritePin(VS1003_CS_PORT, VS1003_CS_PIN, 0);
}

/**
 * @brief Just put the Chip select pin HIGH
 *
 */
void vs1003_control_mode_off(void) {
	HAL_GPIO_WritePin(VS1003_CS_PORT, VS1003_CS_PIN, 1);
}
/**
 * @brief turn data mode on by putting CH pin for SCI high ~ disabled
 * and DCS pin for SDI low ~ enabled
 *
 */
void vs1003_data_mode_on(void) {
	HAL_GPIO_WritePin(VS1003_CS_PORT, VS1003_CS_PIN, 1);
	HAL_GPIO_WritePin(VS1003_DCS_PORT, VS1003_DCS_PIN, 0);
}
/**
 * @brief just put the DCS pin high ~ SDI disabled
 *
 */
void vs1003_data_mode_off(void) {
	HAL_GPIO_WritePin(VS1003_DCS_PORT, VS1003_DCS_PIN, 1);
}

/**
 * @brief find which one is smaller
 * @param size_t a , size_t b
 * @return smaller number
 */
size_t find_min(size_t a, size_t b) {
	return a > b ? b : a;
}

/**
 * @brief read register of VS1003
 * @param uint8_t register address
 * @return uint16_t dataRead at the supplied address
 * */
uint16_t vs1003_read_register(uint8_t _reg) {
	uint16_t result;
	uint8_t temp = 0;
	vs1003_control_mode_on();
	delayMicroseconds(1); // tXCSS

//SPI 1
	HAL_SPI_Transmit(&hspi2, &VS_READ_COMMAND, 1, HAL_MAX_DELAY); //command for read
	HAL_SPI_Transmit(&hspi2, &_reg, 1, HAL_MAX_DELAY); //register
	//READ DATA
	HAL_SPI_Receive(&hspi2, &temp, 1, HAL_MAX_DELAY);
	result = temp << 8;
	temp = 0;
	HAL_SPI_Receive(&hspi2, &temp, 1, HAL_MAX_DELAY);
	result |= temp;

//@TODO WHILE USING THS TRANSMITT RECEIVE function, unable to read data, must be the timing issue between transmission and reception, will check it on logic analyzer later and will figure it out
//	uint8_t tx_data[] = { VS_READ_COMMAND, _reg };
//	uint8_t rx_data[2] = { 0, 0 };
//	HAL_SPI_TransmitReceive(&hspi2, tx_data, rx_data, 2, HAL_MAX_DELAY);
//	result = (rx_data[0] << 8) | rx_data[1];

	delayMicroseconds(1); // tXCSH
	vs1003_await_data_request();
	vs1003_control_mode_off();
	return result;
}
/**
 * @brief write register
 * @param uint8_t register address , uint16_t data to be written
 *
 * */
void vs1003_write_register(uint8_t _reg, uint16_t _value) {
	vs1003_control_mode_on();
	delayMicroseconds(1); // tXCSS

	uint8_t data[4] = { VS_WRITE_COMMAND, _reg, (_value >> 8), (_value & 0xFF) };
	HAL_SPI_Transmit(&hspi2, data, 4, HAL_MAX_DELAY);

	vs1003_await_data_request();
	vs1003_control_mode_off();
}

/**
 * @brief send SDI Buffer Data
 * @param uint8_t * data, size_t length of data
 * */
void vs1003_sdi_send_buffer(uint8_t *data, size_t len) {
	vs1003_data_mode_on();
	while (len) {
		vs1003_await_data_request();
		delayMicroseconds(1);

		size_t chunk_length = find_min(len, vs1003_chunk_size);
		len -= chunk_length;
		while (chunk_length--)
			HAL_SPI_Transmit(&hspi2, data++, 1, 100);
	}
	vs1003_data_mode_off();
}

/**
 * @brief initialize sensor
 * @param length of the zeroes to be sent
 * */
void vs1003_sdi_send_zeroes(size_t len) {
	vs1003_data_mode_on();
	while (len) {
		vs1003_await_data_request();

		size_t chunk_length = find_min(len, vs1003_chunk_size);
		len -= chunk_length;
		uint8_t temp = 0;
		while (chunk_length--)
			HAL_SPI_Transmit(&hspi2, &temp, 1, 100);
	}
	vs1003_data_mode_off();
}

/**
 *@brief initialize sensor
 *@TODO check this software reset why it is there, do we need it
 * */
void vs1003_begin(void) {

// Keep the chip in reset until we are ready
	HAL_GPIO_WritePin(VS1003_RESET_PORT, VS1003_RESET_PIN, 0);

// The SCI and SDI will start deselected
	HAL_GPIO_WritePin(VS1003_CS_PORT, VS1003_CS_PIN, 1);
	HAL_GPIO_WritePin(VS1003_DCS_PORT, VS1003_DCS_PIN, 1);

	HAL_Delay(1);

// release from reset
	HAL_GPIO_WritePin(VS1003_RESET_PORT, VS1003_RESET_PIN, 1);

// Declick: Immediately switch analog off //ANALOG POWER DOWN MODE, sec-8.6.11 pg-36 datasheet
	vs1003_write_register(SCI_VOL, 0xffff); // VOL

	/* Declick: Slow sample rate for slow analog part startup */
	vs1003_write_register(SCI_AUDATA, 10);

	HAL_Delay(100);

	/* Switch on the analog parts */
	vs1003_write_register(SCI_VOL, 0xfefe); // VOL

//	printf_P(PSTR("VS1003 still booting\r\n"));

	vs1003_write_register(SCI_AUDATA, 44101); // 44.1kHz stereo

	vs1003_write_register(SCI_VOL, 0x2020); // VOL

// soft reset
	vs1003_write_register(SCI_MODE, (1 << SM_SDINEW) | (1 << SM_RESET));
	HAL_Delay(1);
	vs1003_await_data_request();
	vs1003_write_register(SCI_CLOCKF, 0xB800); // Experimenting with higher clock settings
	HAL_Delay(1);
	vs1003_await_data_request();
}

/*
 * @brief set volume
 * @param uint8_t volume (0-255 LOWER IS LOUDER)
 * DUPLICATING IS TO PUTTING VOLUME FOR LEFT AND RIGHT CHANNEL
 * */

void vs1003_setVolume(uint8_t vol) {
	uint16_t value = vol;
	value <<= 8;
	value |= vol;

	vs1003_write_register(SCI_VOL, value); // VOL
}

/**
 * @brief start song
 * basically send zeroes to have nothing getting played
 * call this each time a new song start
 * */
void vs1003_startSong(void) {
	vs1003_sdi_send_zeroes(10);
}
/**
 * @brief play a chunk of data
 * @param uint8_t *data
 * @param size_t length of data
 * */
void vs1003_playChunk(uint8_t *data, size_t len) {
	vs1003_sdi_send_buffer(data, len);
}

/*
 * @brief send 2049 zeroes, so it it will play nothing
 * call this after you have played the complete chunk
 * **/

void vs1003_stopSong(void) {
	vs1003_sdi_send_zeroes(2048);
}
/**
 * @brief print a single register data
 * @param uint8_t registerAddress
 * */

void vs1003_print_register_data(uint8_t reg) {
	uint16_t readValue = vs1003_read_register(reg);
	char str[50];
	sprintf(str, "register address %#X Data is %#X \n", reg, readValue);
	HAL_UART_Transmit(&huart2, (const uint8_t*) str, strlen(str),
	HAL_MAX_DELAY);
}

/*
 * @brief print complete details of the chip
 * read all registers and print it
 * */

void vs1003_print_all_register_details(void) {
	for (uint8_t i = 0; i <= SCI_num_registers; i++) {
		vs1003_print_register_data(i);
	}
	HAL_UART_Transmit(&huart2, (uint8_t*) "**************\n", 15, 100);
}

/****************************************************************************/

