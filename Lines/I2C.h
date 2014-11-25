/*
 * I2C.h
 *
 * Created: 15/11/2014 01:48:11
 *  Author: Bo Xuan Hon
 */ 


#ifndef I2C_H_
#define I2C_H_

void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
uint8_t I2C_Write(const uint8_t sla, uint8_t* const data, const uint8_t count);
uint8_t I2C_Read(const uint8_t sla, uint8_t* const data, const uint8_t count);
void I2C_Error(uint8_t status);

#endif /* I2C_H_ */