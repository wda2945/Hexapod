/*
 * i2c_common.h
 *
 *  Created on: Aug 16, 2015
 *      Author: martin
 */

#ifndef I2C_COMMON_H_
#define I2C_COMMON_H_

//file context for I2C bus
extern mraa_i2c_context i2c_context;

extern FILE *i2cDebugFile;

//#define LOGFILEPRINT(...)  tfprintf(i2cDebugFile, __VA_ARGS__);

#define ERRORPRINT(...) tprintf( __VA_ARGS__);tfprintf(i2cDebugFile, __VA_ARGS__);

#ifdef I2C_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(i2cDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) tfprintf(i2cDebugFile, __VA_ARGS__);
#endif


#endif /* I2C_COMMON_H_ */
