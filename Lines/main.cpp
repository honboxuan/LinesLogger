/*
 * main.cpp
 *
 * Created: 15/11/2014 01:48:29
 *  Author: Bo Xuan Hon
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ADC.h"
#include "I2C.h"
#include "USART.h"

extern "C" {
	#include "ff.h"
}

#define DB_INIT()	DDRC  |= (1 << PORTC0)
#define	DB_H()		PORTC |= (1 << PORTC0)
#define DB_L()		PORTC &= ~(1 << PORTC0)
/*
#define LED_INIT()	DDRC  |= (1 << PORTC0)
#define	LED_H()		PORTC |= (1 << PORTC0)
#define LED_L()		PORTC &= ~(1 << PORTC0)
*/
#define LED_INIT()
#define	LED_H()
#define LED_L()

#define BUTTON_PRESS()	!(PINC & (1 << PINC1))

volatile uint32_t tenth_ms = 0;

ISR(TIMER0_COMPA_vect) {
	tenth_ms++;
}

volatile bool button_pressed = false;
volatile uint32_t button_time = 0;

ISR(PCINT1_vect) {
	if (BUTTON_PRESS() && (tenth_ms - button_time) > 10000UL) {
		button_pressed = true;
		button_time = tenth_ms;
	}
}

bool button_event(void) {
	if (button_pressed) {
		button_pressed = false;
		return true;
	} else {
		return false;
	}
}

int main(void) {
	MCUCR |= (1 << PUD); //Disable internal pull-ups
	
	DB_INIT(); //Debug pin
	LED_INIT(); //LED indicator

	//========== Peripherals Config ==========
	I2C_Init();
	USART_Init(8);
	
	TCCR0A = (1 << WGM01); //Mode 2, CTC, Top is OCR0A
	TIMSK0 = (1 << OCIE0A); //Enable Compare Match A Interrupt
	OCR0A = 200; //Gives 10kHz with prescaler
	TCCR0B = (1 << CS01); //Prescaler = 8
	
	PCICR = (1 << PCIE1);
	PCMSK1 = (1 << PCINT9); //Button
	
	PORTC &= ~((1 << PORTC0)|(1 << PORTC2)|(1 << PORTC3));
	DIDR0 |= (1 << ADC0D)|(1 << ADC2D)|(1 << ADC3D);
	
	sei(); //Enable global interrupts
	
	//========== MPU-9250 Config ==========
	uint8_t ra_setting[2];
	
	//Configure MPU-9250
	ra_setting[0] = 0x37; //INT_PIN_CFG
	ra_setting[1] = 0x02; //BYPASS_EN
	I2C_Write(0x68, ra_setting, 2);
	//Configure Magnetometer (AK8963)
	ra_setting[0] = 0x0A; //AK8963_CNTL
	ra_setting[1] = 0x16; //16-bit output, continuous 10Hz
	I2C_Write(0x0C, ra_setting, 2);

	USART_Transmit("Ready\n");

	while(1) {
		while (!button_event());
		/*
		while (!button_event()) {
			uint8_t adc[2];
			ADC_Read(0, adc + 1, adc);
			uint16_t val = (uint16_t(adc[1]) << 8) | adc[0];
			USART_Transmit("%u\n", val);	
		}
		*/
	
		//========== FAT FS Start ==========
		FATFS FatFs;
		FIL Fil;

		//Mount
		if (f_mount(&FatFs, "", 0) == FR_OK) {
			//Find next filename
			TCHAR path[5];
			sprintf(path, "00000");
			uint16_t file_number = 0;
			while (f_stat(path, NULL) == FR_OK && file_number < 65535) {
				sprintf(path, "%05u", ++file_number);
			}
			USART_Transmit("Using %s\n", path);
			/*
			for (uint16_t i = 0; i < file_number; i++) {
				LED_H();
				_delay_ms(100);
				LED_L();
				_delay_ms(100);
			}
			*/
			_delay_ms(1000);
			
			if (f_open(&Fil, path, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
				uint8_t ra;
				uint8_t mag_drdy = 0;
				uint8_t bytes_count = 0;
				
				uint32_t time = 0;
				uint8_t accel_temp_gyro_data[14];
				uint8_t mag_data[7]; //Need to read ST2
				//uint16_t adc_data[3];
				uint8_t adc_data[6];
				
				const uint8_t bytes_count_size = sizeof(bytes_count);
				const uint8_t time_size = sizeof(time);
				const uint8_t accel_temp_gyro_data_size = sizeof(accel_temp_gyro_data);
				const uint8_t mag_data_size = sizeof(mag_data);
				const uint8_t adc_data_size = sizeof(adc_data);
				
				UINT bw;
				
				//Get gyroscope offset
				uint8_t gyro_offset[6];
				ra = 0x13;
				I2C_Write(0x68, &ra, 1); //RA for XG_OFFSET_H
				I2C_Read(0x68, gyro_offset, 6);
				
				//Get accelerometer offset
				uint8_t accel_offset[6];
				ra = 0x77;
				I2C_Write(0x68, &ra, 1); //RA for XA_OFFSET_H
				I2C_Read(0x68, accel_offset, 6);
				
				//Write file header
				uint8_t header_length = 11 + sizeof(gyro_offset) + sizeof(accel_offset);
				f_write(&Fil, &header_length, sizeof(header_length), &bw);
				f_write(&Fil, "LinesLogger", 11, &bw);
				f_write(&Fil, &gyro_offset, sizeof(gyro_offset), &bw);
				f_write(&Fil, &accel_offset, sizeof(accel_offset), &bw);
				
				//Loop
				while (!button_event()) {
					bytes_count = 0;
					while (tenth_ms < time + 20);
					time = tenth_ms;
					bytes_count += time_size;
				
					LED_H(); //LED turns on indicating first read
				
					//Accelerometer, Temperature, Gyroscope
					ra = 0x3B;
					I2C_Write(0x68, &ra, 1); //RA for ACCEL_XOUT_H
					I2C_Read(0x68, accel_temp_gyro_data, 14);
					bytes_count += accel_temp_gyro_data_size;
				
					//Magnetometer
					ra = 0x02;
					I2C_Write(0x0C, &ra, 1); //RA for AK8963_ST1
					I2C_Read(0x0C, &mag_drdy, 1);
					if (mag_drdy) {
						ra = 0x03;
						I2C_Write(0x0C, &ra, 1); //RA for AK8963_XOUT_L
						I2C_Read(0x0C, mag_data, 7);
						bytes_count += mag_data_size - 1;
					}
					
					//ADC a.k.a. Force
					/*for (uint8_t i = 0; i < 3; i++) {
						//adc_data[i] = ADC_Read(i);
						ADC_Read(i, adc_data + 2*i + 1, adc_data+ 2*i);
					}
					*/
					ADC_Read(0, adc_data + 1, adc_data);
					ADC_Read(2, adc_data + 3, adc_data + 2);
					ADC_Read(3, adc_data + 5, adc_data + 4);
					bytes_count += adc_data_size;
					
					//Output
					f_write(&Fil, &bytes_count, bytes_count_size, &bw);
					f_write(&Fil, &time, time_size, &bw);
					f_write(&Fil, accel_temp_gyro_data, accel_temp_gyro_data_size, &bw);
					f_write(&Fil, adc_data, adc_data_size, &bw);
					if (mag_drdy) {
						f_write(&Fil, mag_data, mag_data_size - 1, &bw);
					}
				}
				f_close(&Fil);
				LED_L();
			} else {
				USART_Transmit("Open failed\n");
			}
			//Unmount
			f_mount(0, "", 0);
			USART_Transmit("Done\n");
		} else {
			USART_Transmit("Mount failed\n");	
		}
	}
}