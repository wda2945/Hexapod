/*
 * PCF8591.h
 *
 *  Created on: Aug 10, 2015
 *      Author: martin
 */

#ifndef I2C_TASK_PCF8591_H_
#define I2C_TASK_PCF8591_H_

#ifdef __cplusplus
extern "C" {
#endif

//initialize ADC
int PCF8591Init();

//read selected channel
int PCF8591Read(int channel);



#ifdef __cplusplus
}
#endif

#endif /* I2C_TASK_PCF8591_H_ */
