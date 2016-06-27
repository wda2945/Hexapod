/*
 * i2c_task.h
 *
 *      Author: martin
 */

#ifndef I2C_TASK_H
#define I2C_TASK_H

//start I2C Thread
int I2CInit();

//process message
void I2CProcessMessage(psMessage_t *msg);

//set selected GPIO pin
void I2CSetGPIOPin(int pinNumber, bool state);

extern uint8_t GPIObits;

void LSM303StartCalibrate();

#endif
