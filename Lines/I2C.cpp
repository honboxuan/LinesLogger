/*
 * I2C.cpp
 *
 * Created: 15/11/2014 01:48:29
 *  Author: Bo Xuan Hon
 */ 

#include <avr/io.h>
#include "I2C.h"
#include "USART.h"

void I2C_Init(void) {
	DDRC |= (1 << PORTC4)|(1 << PORTC5); //SDA: PC4, SCL: PC5
	PORTC |= (1 << PORTC4)|(1 << PORTC5);
	
	//400kHz for MPU9250 Fast-mode
	//SCL Freq = F_CPU/(16 + 2*TWBR*Prescaler)
	//TWBR*Prescaler = 12
	
	TWBR = 12;
	//TWCR = (1 << TWEN); //TWEN enforced during read/write operations
	//TWSR as default Prescaler = 1
	
}

void I2C_Start(void) {
	TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN); //START via TWCR
	while (!(TWCR & (1 << TWINT))); //Wait for TWINT
	if ((TWSR & 0xF8) != 0x08) { //Check TWSR (prescaler bits masked) for START
		//Error
		I2C_Error(TWSR & 0xF8);
	}
}

void I2C_Stop(void) {
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTO); //STOP via TWCR
}

uint8_t I2C_Write(const uint8_t sla, uint8_t* const data, const uint8_t count) {
	I2C_Start();
	
	TWDR = (sla << 1); //SLA + W via TWDR
	TWCR = (1 << TWINT)|(1 << TWEN);
	while (!(TWCR & (1 << TWINT))); //Wait for TWINT
	if ((TWSR & 0xF8) != 0x18) { //Check TWSR (prescaler bits masked) for SLA + W sent and ACK received
		//Error
		I2C_Error(TWSR & 0xF8);
	}
	
	uint8_t sent_count = 0;
	while (sent_count < count) {
		TWDR = data[sent_count]; //Data via TWDR (TWINT must be set first)
		TWCR = (1 << TWINT)|(1 << TWEN);
		while (!(TWCR & (1 << TWINT))); //Wait for TWINT
		if ((TWSR & 0xF8) != 0x28) { //Check TWSR (prescaler bits masked) for data sent and ACK received
			//Error
			I2C_Error(TWSR & 0xF8);
		}
		sent_count++;
	}
	
	I2C_Stop();
	
	return sent_count;
}

uint8_t I2C_Read(const uint8_t sla, uint8_t* const data, const uint8_t count) {
	I2C_Start();
	
	TWDR = (sla << 1)|(1 << 0); //SLA + R via TWDR
	TWCR = (1 << TWINT)|(1 << TWEN);
	while (!(TWCR & (1 << TWINT))); //Wait for TWINT
	if ((TWSR & 0xF8) != 0x40) { //Check TWSR (prescaler bits masked) for SLA + R sent and ACK received
		//Error
		I2C_Error(TWSR & 0xF8);
	}
	
	uint8_t received_count = 0;
	while (received_count < count - 1) {
		TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWEA); //ACK
		while (!(TWCR & (1 << TWINT))); //Wait for TWINT
		if ((TWSR & 0xF8) != 0x50) { //Check TWSR (prescaler bits masked) for data received and ACK sent
			//Error
			I2C_Error(TWSR & 0xF8);
		}
		data[received_count] = TWDR;
		received_count++;
	}
	//Last byte
	TWCR = (1 << TWINT)|(1 << TWEN); //NACK
	while (!(TWCR & (1 << TWINT))); //Wait for TWINT
	if ((TWSR & 0xF8) != 0x58) { //Check TWSR (prescaler bits masked) for data received and NACK sent
		//Error
		I2C_Error(TWSR & 0xF8);
	}
	data[received_count] = TWDR;
	
	I2C_Stop();
	
	return received_count;
}

void I2C_Error(uint8_t status) {
	USART_Transmit("I2C Error: %x\n", status);
}