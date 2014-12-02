/*
 * ADC.cpp
 *
 * Created: 01/12/2014 22:20:49
 *  Author: Bo Xuan Hon
 */ 

#include "ADC.h"

//uint16_t ADC_Read(const uint8_t ch) {
void ADC_Read(const uint8_t ch) {
	switch (ch) {
		case 0:
			ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
			break;
		case 1:
			ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1));
			ADMUX |= (1 << MUX0);
			break;
		case 2:
			ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX0));
			ADMUX |= (1 << MUX1);
			break;
		case 3:
			ADMUX &= ~((1 << MUX3) | (1 << MUX2));
			ADMUX |= (1 << MUX1) | (1 << MUX0);
			break;
		case 4:
			ADMUX &= ~((1 << MUX3) | (1 << MUX1) | (1 << MUX0));
			ADMUX |= (1 << MUX2);
			break;
		case 5:
			ADMUX &= ~((1 << MUX3) | (1 << MUX1));
			ADMUX |= (1 << MUX2) | (1 << MUX0);
			break;
		case 6:
			ADMUX &= ~((1 << MUX3) | (1 << MUX0));
			ADMUX |= (1 << MUX2) | (1 << MUX1);
			break;
		case 7:
			ADMUX &= ~(1 << MUX3);
			ADMUX |= (1 << MUX2) | (1 << MUX1) | (1 << MUX0);
			break;
		case 8:
			//Temperature
			ADMUX &= ~((1 << MUX2) | (1 << MUX1) | (1 << MUX0));
			ADMUX |= (1 << MUX3);
			break;
	}
	
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //Clock division factor 128
	//ADCSRA |= (1 << ADEN); //Clock division factor 2
	ADCSRA |= (1 << ADSC);
	while (!(ADCSRA & (1 << ADIF)));
	
	//return ((uint16_t(ADCH) << 8) | ADCL);
}

void ADC_Read(const uint8_t ch, uint8_t* const h, uint8_t* const l) {
	ADC_Read(ch);
	*l = ADCL;
	*h = ADCH;
}