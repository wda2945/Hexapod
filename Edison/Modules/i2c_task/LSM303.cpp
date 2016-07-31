/*
 ============================================================================
 Name        : LSM303.c
 Author      : Martin
 Version     :
 Copyright   : (c) 2015 Martin Lane-Smith
 Description : Reads IMU
 ============================================================================
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#ifndef __USE_MISC
#define __USE_MISC
#endif

#include <math.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "mraa.h"

#include "hexapod.h"

#include "i2c_task/i2c_task.hpp"
#include "i2c_task/i2c_common.h"
#include "i2c_task/LSM303.h"
#include "i2c_task/LSM303_Regs.h"

#define DECLINATION (M_PI * (9.5f / 180.0f))

int LSM303_readMag(void);
int LSM303_readAcc(void);

float LSM303_heading(void);
float LSM303_pitch(void);
float LSM303_roll(void);

typedef struct {
	int16_t x,y,z;
} vector_int16_t;

vector_int16_t LSM303_a; // accelerometer readings
vector_int16_t LSM303_m; // magnetometer readings

// vector functions
typedef struct {
	double x,y,z;
} vector_float;

static void vector_cross(const vector_float *a, const vector_float *b, vector_float *out);
static float vector_dot(const vector_float *a, const vector_float *b);
static void vector_normalize(vector_float *a);

///////////////////////////////////////////////

    vector_int16_t m_max // maximum magnetometer values, used for calibration
   			= {3764, 2802, 4934};
    vector_int16_t m_min // minimum magnetometer values, used for calibration
    		= {-3438, -3128, -8509};

    int LSM303_setAddress();
    int LSM303_writeReg(uint8_t reg, uint8_t value);
    int LSM303_readReg(uint8_t reg);

void LSM303Calibrate()
{
	vector_int16_t LSMmin = {+32767, +32767, +32767};
	vector_int16_t LSMmax = {-32767, -32767, -32767};

	while (LSM303Init() < 0)
	{
		sleep(1);
	}

	while(1)
	{

		if (LSM303_readMag() < 0)
		{
			printf("ReadMag fail");
		}

		if (LSM303_m.x > LSMmax.x) LSMmax.x = LSM303_m.x;
		if (LSM303_m.y > LSMmax.y) LSMmax.y = LSM303_m.y;
		if (LSM303_m.z > LSMmax.z) LSMmax.z = LSM303_m.z;

		if (LSM303_m.x < LSMmin.x) LSMmin.x = LSM303_m.x;
		if (LSM303_m.y < LSMmin.y) LSMmin.y = LSM303_m.y;
		if (LSM303_m.z < LSMmin.z) LSMmin.z = LSM303_m.z;

		printf("max = {%i,%i,%i} min  = {%i,%i,%i}", LSMmax.x,LSMmax.y,LSMmax.z,LSMmin.x,LSMmin.y,LSMmin.z);
	}
}

/*
Enables the LSM303's accelerometer and magnetometer. Also:
- Sets sensor full scales (gain) to default power-on values, which are
  +/- 2 g for accelerometer and +/- 1.3 gauss for magnetometer
  (+/- 4 gauss on LSM303D).
- Selects 50 Hz ODR (output data rate) for accelerometer and 7.5 Hz
  ODR for magnetometer (6.25 Hz on LSM303D). (These are the ODR
  settings for which the electrical characteristics are specified in
  the datasheets.)
- Enables high resolution modes (if available).
Note that this function will also reset other settings controlled by
the registers it writes to.
*/
int LSM303Init()
{
    // Accelerometer
	if (LSM303_setAddress() < 0) return -1;

    // 0x00 = 0b00000000
    // AFS = 0 (+/- 2 g full scale)
	if (LSM303_writeReg(CTRL2, 0x00) < 0) return -1;

    // 0x57 = 0b01010111
    // AODR = 0101 (50 Hz ODR); AZEN = AYEN = AXEN = 1 (all axes enabled)
	if (LSM303_writeReg(CTRL1, 0x57) < 0) return -1;

    // Magnetometer

    // 0x64 = 0b01100100
    // M_RES = 11 (high resolution mode); M_ODR = 001 (6.25 Hz ODR)
	if (LSM303_writeReg(CTRL5, 0x64) < 0) return -1;

    // 0x20 = 0b00100000
    // MFS = 01 (+/- 4 gauss full scale)
	if (LSM303_writeReg(CTRL6, 0x20) < 0) return -1;

    // 0x00 = 0b00000000
    // MLP = 0 (low power mode off); MD = 00 (continuous-conversion mode)
	if (LSM303_writeReg(CTRL7, 0x00) < 0) return -1;

	ps_registry_add_new("IMU", "heading", PS_REGISTRY_INT_TYPE, PS_REGISTRY_SRC_WRITE | PS_REGISTRY_LOCAL);
	ps_registry_add_new("IMU", "pitch", PS_REGISTRY_INT_TYPE, PS_REGISTRY_SRC_WRITE | PS_REGISTRY_LOCAL);
	ps_registry_add_new("IMU", "roll", PS_REGISTRY_INT_TYPE, PS_REGISTRY_SRC_WRITE | PS_REGISTRY_LOCAL);

	return 0;
}

int LSM303_setAddress()
{
	//set IMU I2C address
	if (mraa_i2c_address(i2c_context, IMU_XM_I2C_ADDRESS) != MRAA_SUCCESS)
	{
		ERRORPRINT("IMU_XM: mraa_i2c_address error");
		return -1;
	}
	return 0;
}

// Write a register
int LSM303_writeReg(uint8_t reg, uint8_t value)
{
	//write to port register
	if (mraa_i2c_write_byte_data(i2c_context, value, reg)  != MRAA_SUCCESS)
	{
		ERRORPRINT("IMU_XM: mraa_i2c_write_byte_data(0x%02x, 0x%02x) error", reg, value);
		return -1;
	}
	return 0;
}

// Read a register
int LSM303_readReg(uint8_t reg)
{
	uint8_t result;
	//read from port register
	if (mraa_i2c_read_bytes_data(i2c_context, reg, &result, 1) < 0)
	{
		ERRORPRINT("IMU_XM: i2c_read_byte_data(0x%02x) - %s", reg, strerror(errno));
		return -1;
	}
	return result;
}

// Reads the 3 accelerometer channels and stores them in vector a
int LSM303_readAcc(void)
{
	int i, result;
	bool readerror = false;
	uint8_t regData[6];

	if (LSM303_setAddress() < 0) return -1;

	for (i=0; i<6; i++)
	{
		result = LSM303_readReg(OUT_X_L_A + i);
		if (result >= 0)
		{
			regData[i] = result;
		}
		else
		{
			regData[i] = 0;
			readerror = true;
		}
	}

  // combine high and low bytes
  // This no longer drops the lowest 4 bits of the readings from the DLH/DLM/DLHC, which are always 0
  // (12-bit resolution, left-aligned). The D has 16-bit resolution
  LSM303_a.x = (int16_t)(regData[1] << 8 | regData[0]);
  LSM303_a.y = (int16_t)(regData[3] << 8 | regData[2]);
  LSM303_a.z = (int16_t)(regData[5] << 8 | regData[4]);

  DEBUGPRINT("Accel: %i, %i, %i", LSM303_a.x, LSM303_a.y, LSM303_a.z);

	return (readerror ? -1 : 0);
}

// Reads the 3 magnetometer channels and stores them in vector m
int LSM303_readMag(void)
{
	int i, result;
	bool readerror = false;
	uint8_t regData[6];

	if (LSM303_setAddress() < 0) return -1;

	for (i=0; i<6; i++)
	{
		result = LSM303_readReg(OUT_X_L_M + i);
		if (result >= 0)
		{
			regData[i] = result;
		}
		else
		{
			regData[i] = 0;
			readerror = true;
		}
	}
	// combine high and low bytes
	LSM303_m.x = (int16_t)(regData[1] << 8 | regData[0]);
	LSM303_m.y = (int16_t)(regData[3] << 8 | regData[2]);
	LSM303_m.z = (int16_t)(regData[5] << 8 | regData[4]);

	DEBUGPRINT("Mag: %i, %i, %i", LSM303_m.x, LSM303_m.y, LSM303_m.z);

	return (readerror ? -1 : 0);
}

// Reads all 6 channels of the LSM303
int LSM303Read(void)
{
	psMessage_t msg;

	if (LSM303_readAcc() < 0) return -1;
	if (LSM303_readMag() < 0) return -1;

	ps_registry_set_int("IMU", "heading", LSM303_heading());
	ps_registry_set_int("IMU", "pitch", LSM303_pitch());
	ps_registry_set_int("IMU", "roll", LSM303_roll());

	DEBUGPRINT("Heading: %0.0f", msg.threeFloatPayload.heading);

	return 0;
}

void vector_normalize(vector_float *a)
{
  float mag = sqrt(vector_dot(a, a));
  a->x /= mag;
  a->y /= mag;
  a->z /= mag;
}

//////////////////
/*
Returns the angular difference in the horizontal plane between the
"from" vector and north, in degrees.

Description of heading algorithm:
Shift and scale the magnetic reading based on calibration data to find
the North vector. Use the acceleration readings to determine the Up
vector (gravity is measured as an upward acceleration). The cross
product of North and Up vectors is East. The vectors East and North
form a basis for the horizontal plane. The From vector is projected
into the horizontal plane and the angle between the projected vector
and horizontal north is returned.
*/
float LSM303_heading()
{
	vector_float from = {1,0,0};			//+X axis forward
	vector_float temp_m = {(double)LSM303_m.x, (double)LSM303_m.y, (double)LSM303_m.z};
	vector_float temp_a = {(double)LSM303_a.x, (double)LSM303_a.y, (double)LSM303_a.z};

    // subtract offset (average of min and max) from magnetometer readings
    temp_m.x -= ((float)m_min.x + m_max.x) / 2;
    temp_m.y -= ((float)m_min.y + m_max.y) / 2;
    temp_m.z -= ((float)m_min.z + m_max.z) / 2;

//	printf("Mag = {%f, %f, %f}", temp_m.x, temp_m.y, temp_m.z);

    // compute E and N
    vector_float E;
    vector_float N;
    vector_cross(&temp_m, &temp_a, &E);
    vector_normalize(&E);
    vector_cross(&temp_a, &E, &N);
    vector_normalize(&N);

    // compute heading
    float heading = atan2(vector_dot(&E, &from), vector_dot(&N, &from)) * 180 / M_PI;

    if (heading < 0) heading += 360;
    if (heading >= 360) heading -= 360;

//    DEBUGPRINT("IMU Heading: %f", heading)

    return heading;
}

float LSM303_pitch(void)
{
	return atan2(LSM303_a.x, LSM303_a.z) * 180 / M_PI;
}
float LSM303_roll(void)
{
	return atan2(LSM303_a.y, LSM303_a.z) * 180 / M_PI;
}

void vector_cross(const vector_float *a, const vector_float *b, vector_float *out)
{
  out->x = (a->y * b->z) - (a->z * b->y);
  out->y = (a->z * b->x) - (a->x * b->z);
  out->z = (a->x * b->y) - (a->y * b->x);
}

float vector_dot(const vector_float *a, const vector_float *b)
{
  return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}
