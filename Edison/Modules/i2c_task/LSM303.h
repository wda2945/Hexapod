#ifndef LSM303_h
#define LSM303_h

#ifdef __cplusplus
extern "C" {
#endif

//initialize IMU
int LSM303Init();

//read and process data
int LSM303Read();

void LSM303Calibrate();



#ifdef __cplusplus
}
#endif

#endif



