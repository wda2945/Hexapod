#ifndef LSM303_h
#define LSM303_h

//initialize IMU
int LSM303Init();

//read and process data
int LSM303Read();

void LSM303Calibrate();

#endif



