/*
 * ADC.h
 *
 * Created: 01/12/2014 22:20:49
 *  Author: Bo Xuan Hon
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdint.h>

//uint16_t ADC_Read(const uint8_t ch);
void ADC_Read(const uint8_t ch);
void ADC_Read(const uint8_t ch, uint8_t* const h, uint8_t* const l);

#endif /* ADC_H_ */