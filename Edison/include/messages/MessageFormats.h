/* 
 * File:   MessageFormats.h
 * Author: martin
 *
 * Created on December 8, 2013, 10:53 AM
 */

#ifndef MESSAGEFORMATS_H
#define	MESSAGEFORMATS_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#ifdef __XC32
#include "FreeRTOS.h"
#include "queue.h"
#else
typedef uint32_t TickType_t;
#endif

#define PS_NAME_LENGTH 30
#define PS_SHORT_NAME_LENGTH 16

typedef uint8_t ps_message_id_t;
typedef uint8_t ps_topic_id_t;
typedef uint8_t ps_source_id_t;
typedef uint8_t ps_message_qos_t;

//Common header for all messages
typedef struct {
    ps_message_id_t messageType;    	///< ID of message in payload (psMessageType_enum)
    ps_topic_id_t 	messageTopic;     	///< destination topic
    ps_source_id_t 	messageSource;
    uint8_t			spare;
} psMessageHeader_t;

//----------------------PS_SYSLOG
//#define FILE_NAME_LENGTH 4
//typedef struct {
//    uint8_t severity;
//    char file[FILE_NAME_LENGTH + 1];
//    char text[PS_MAX_LOG_TEXT+1];
//} psLogPayload_t;

//----------------------SYSTEM MANAGEMENT

//PS_RESPONSE

typedef struct {
    uint8_t flags;					//pingResponseStatus_enum
    char subsystem[3];				//eg "MCP"
} psResponsePayload_t;

//PS_TICK_PAYLOAD

//Data broadcast by 1S tick
typedef struct {
    uint8_t systemPowerState;		//PowerState_enum
} psTickPayload_t;

//PS_MASK

typedef struct {
    NotificationMask_t value;
    NotificationMask_t valid;
}psEventMaskPayload_t;

//----------------------GENERAL PURPOSE FORMATS
//PS_BYTE

typedef struct {
    uint8_t value;
} psBytePayload_t;

//PS_INT

typedef struct {
    int32_t value;
} psIntPayload_t;

//PS_FLOAT

typedef struct {
    float value;
} psFloatPayload_t;

//PS_2FLOAT

typedef struct {
    union {
        struct {
            float x, y;
        };
        struct {
            float value1, value2;
        };
        struct {
        	float opening, speed;
        };
    };
} ps2FloatPayload_t;

//PS_3FLOAT

typedef struct {
    union {
        struct {
            float x, y, z;
        };
        struct {
            float value1, value2, value3;
        };
        struct {
            float xSpeed;
            float ySpeed;
            float zRotateSpeed;
         };
        struct {
        	float heading;
        	float pitch;
        	float roll;
        };
    };
} ps3FloatPayload_t;

//PS_NAME

typedef struct {
    char name[PS_NAME_LENGTH+1];
} psNamePayload_t;

//PS_NAMED_BYTE

typedef struct {
    uint8_t value;
    char name[PS_NAME_LENGTH+1];
} psNameBytePayload_t;

//PS_NAMED_FLOAT

typedef struct {
    float value;
    char name[PS_NAME_LENGTH+1];
} psNameFloatPayload_t;

//PS_NAMED_INT

typedef struct {
    int value;
    char name[PS_NAME_LENGTH+1];
} psNameIntPayload_t;

//PS_NAMED_3FLOAT

typedef struct {

    union {

        struct {
            float float1;
            float float2;
            float float3;
        };

        struct {
            float min;
            float max;
            float value;
        };
    };
    char name[PS_NAME_LENGTH+1];
} psName3FloatPayload_t;

//PS_NAMED_3INT

typedef struct {
    union {
        struct {
            int int1;
            int int2;
            int int3;
        };
        struct {
            int min;
            int max;
            int value;
        };
    };
    char name[PS_NAME_LENGTH+1];
} psName3IntPayload_t;

//PS_NAMED_4BYTE

typedef struct {
    union {
    uint8_t uint8[4];
    uint16_t uint16[2];
    uint32_t uint32;
    };
    char name[PS_NAME_LENGTH+1];
} psName4BytePayload_t;

//----------------------- CONFIG ---------------------------------------

//PS_CONFIG					//CONFIG & CONFIG_DONE

typedef struct {
	uint8_t	count;			//CONFIG_DONE only
}psConfigPayload_t;

//PS_SETTING				//SETTING and NEW_SETTING

typedef struct {
	float min;
 	float max;
  	float value;
  	uint8_t subsystem;      //requestor for SETTING, responder for NEW_SETTING
  	char name[PS_NAME_LENGTH+1];
} psSettingPayload_t;

//PS_OPTION					//OPTION & CHANGE_OPTION

typedef struct {
	int min;
 	int max;
  	int value;
  	uint8_t subsystem;      //requestor for OPTION, responder for CHANGE_OPTION
  	char name[PS_NAME_LENGTH+1];
} psOptionPayload_t;

//---------------------- STATS -----------------------------------------------

//PS_COMMS_STATS_PAYLOAD

typedef struct {
    char destination[4];
    int messagesSent;
    int addressDiscarded;
    int congestionDiscarded;      //congestion
    int logMessagesDiscarded;
    int messagesReceived;
    int addressIgnored;        //wrong address
    int parseErrors;
} psCommsStatsPayload_t;

//----------------------NAVIGATION & SENSORS------------------------------------------
//----------------------RAW

//PS_IMU

typedef struct {
    int16_t magX, 	magY, 	magZ;
    int16_t accelX,	accelY,	accelZ;
    int16_t gyroX, 	gyroY, 	gyroZ;
    struct timeval timestamp;
} psImuPayload_t;

//PS_ODOMETRY

typedef struct {
    //movement since last message
    float xMovement;         	// cm
    float yMovement;			// cm
    float zRotation;
    //status
    uint8_t motorsRunning;
} psOdometryPayload_t;

//------------------------COOKED

typedef struct {
	uint16_t heading;	//degrees
	int16_t pitch;		//degrees
	int16_t roll;		//degrees
} Orientation_struct;

typedef struct {
	float latitude;
	float longitude;
} Position_struct;

//PS_POSE_PAYLOAD

typedef struct {
	Position_struct position;
	Orientation_struct orientation;
	uint8_t orientationValid;
	uint8_t positionValid;
} psPosePayload_t;

//---------------------------------------------PROXIMITY----------------------------

//Detailed report
typedef struct {
    uint8_t direction;      //psProxDirection_enum (45 degree sectors clockwise from heading)
    uint8_t rangeReported;  //0 - 200 cm
    uint8_t confidence;     //0 - 100%
    uint8_t status;	
} psProxReportPayload_t;

//summary
typedef struct {       //bitmaps for 8 directions
	uint8_t proxStatus[PROX_SECTORS];		//ProxStatus_enum
} psProxSummaryPayload_t;

//----------------------MOVEMENT

//PS_MOVE_PAYLOAT

typedef struct {
    float xSpeed;
    float ySpeed;
    float zRotateSpeed;
    int	steps;
} psMovePayload_t;

//----------------------ENVIRONMENT

//PS_BATTERY
typedef struct {
    uint16_t volts;			//times 10
    uint8_t status;			//BatteryStatus_enum
} psBatteryPayload_t;

//PS_BEHAVIOR_STATUS

typedef struct {
    int status;
    char behavior[PS_SHORT_NAME_LENGTH];
    char lastLuaCallFail[PS_SHORT_NAME_LENGTH];
    char lastFailReason[PS_SHORT_NAME_LENGTH];
} psBehaviorStatusPayload_t;

#endif	/* MESSAGEFORMATS_H */

