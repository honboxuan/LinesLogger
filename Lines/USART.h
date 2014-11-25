/*
 * USART.h
 *
 * Created: 07/12/2012 16:50:13
 *  Author: Bo Xuan Hon
 */ 


#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

void USART_Init (uint32_t ubrr = 10);
void USART_TransmitChar(uint8_t data);
void USART_Transmit(const char* format, ...);
uint8_t USART_Receive(void);

#endif /* USART_H_ */