/*
 * i2c_common.h
 *
 *  Created on: Aug 16, 2015
 *      Author: martin
 */

#ifndef I2C_TASK_I2C_COMMON_H_
#define I2C_TASK_I2C_COMMON_H_

#include "softwareProfile.h"

//file context for I2C bus
extern mraa_i2c_context i2c_context;

extern FILE *i2cDebugFile;

#define LOGFILEPRINT(...)  fprintf(i2cDebugFile, __VA_ARGS__);fflush(i2cDebugFile);
#define ERRORPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(i2cDebugFile, __VA_ARGS__);fflush(i2cDebugFile);

#ifdef I2C_DEBUG
#define DEBUGPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(i2cDebugFile, __VA_ARGS__);fflush(i2cDebugFile);
#else
#define DEBUGPRINT(...) LOGFILEPRINT(...)
#endif


#endif /* I2C_TASK_I2C_COMMON_H_ */
