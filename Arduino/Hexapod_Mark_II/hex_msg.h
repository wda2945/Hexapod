//
// Definition of messages between controller & ArbotixM
// hex_msg.h
//
#ifndef HEX_MSG_H
#define HEX_MSG_H

//fixed-length message
typedef struct {
	union {
		//used from controller to Arbotix
		struct {                			//walk command
			short int xSpeed;          		//forward/back mm/sec
			short int ySpeed;          		//side-to-side mm/sec
			short int zRotateSpeed;    		//turn degrees/sec
			short int steps;				//0 for continuous
		};
		struct {                    		//Body move command
			short int xBody, yBody, zBody;
		};
		struct {                    		//set gait command
			short int gait;
		};
		struct {
			short int legNumber;
			union {
				struct {
					short int x, y, z; 			//get/set pose command
				};
				struct {
					short int coxa, femur, tibia;            //get/set servos
				};
			};
		};
		struct {
			short int time;
		};
		struct {               			//MSG_TYPE_READ_REG
			unsigned short int regReadId;
			unsigned short int regReadStart;
			unsigned short int regReadLength;
		};
		struct {               			//MSG_TYPE_WRITE_REG(2)
			unsigned short int regWriteId;
			unsigned short int regWriteStart;
			unsigned short int regWriteL;
			unsigned short int regWriteH;
		};

		//used from Arbotix to Controller
		struct {                			//Arb status report
			unsigned short int state;           	        //axState_enum below
			unsigned short int lastCommand;     	        //command (in progress)
			unsigned short int busy;	      		//0 if complete
			unsigned short int stepsToGo;
		};
		struct {		      					//battery voltage (x 10)
			unsigned short int volts;
		};
		struct {		     					 //MSG_TYPE_ODOMETRY
			short int xMovement;
			short int yMovement;
			short int zRotation;
		};
		struct {                				//MSG_TYPE_FAIL
			unsigned short int servo;         		//servo_enum
			short int angle;           		        //bad value
		};
		struct {                				//MSG_TYPE_REG
			unsigned short int ax12Reg;
		};
		struct {
			unsigned short int errorCode;        	 //msgType_enum
			unsigned short int errorState;           //axState_enum
			unsigned short int error1;        	 
			unsigned short int error2;                 
		};
#define MSG_INTS 4
		short int intValues[MSG_INTS];
#define TEXT_LEN 8
		unsigned char text[TEXT_LEN];  	//text message
	};
	unsigned char msgType;             //see msgType_enum below
} message_t;

#define MSG_LEN       	(9)      				//excludes MSG_START & checksum

#define MSG_START '@'

typedef enum {
	//commands.
	MSG_TYPE_NONE,
	MSG_TYPE_GETSTATUS, //no payload / MSG_TYPE_STATUS w/status payload
	//Controller->Arb (no reply)
	MSG_TYPE_HALT,      //no payload
	MSG_TYPE_WALK,      //walk payload
	MSG_TYPE_BODY,      //x,y,z payload
	MSG_TYPE_ROTATE,    //x,y,z payload (z not used)
	MSG_TYPE_GAIT,      //set gait payload
	MSG_TYPE_WRITE_REG,    //write payload
	MSG_TYPE_WRITE_REG2,   //write payload (2 byte)
	//Controller->Arb (with status reply when done)
	MSG_TYPE_RELAX,     //no payload
	MSG_TYPE_TORQUE,    //no payload
	MSG_TYPE_STAND,     //no payload
	MSG_TYPE_SIT,       //no payload
	MSG_TYPE_POSEMODE,  //no payload - start pose mode
	MSG_TYPE_MOVE,      //short int payload (action pose)
	//Controller->Arb (with reply)
	MSG_TYPE_READ_REG,  //read reg payload / MSG_TYPE_REG w/reg payload
	MSG_TYPE_SET_POSE,  //leg position payload / MSG_TYPE_SERVOS w/servo payload
	MSG_TYPE_SET_SERVOS,  //servo position payload / MSG_TYPE_SERVOS w/servo payload
	MSG_TYPE_GET_POSE,  //no payload / MSG_TYPE_POSE & MSG_TYPE_SERVOS
	//Arb->controller
	MSG_TYPE_STATUS,    //Status report payload
	MSG_TYPE_ODOMETRY,  //Odometry payload
	MSG_TYPE_POSE,      //leg position payload
	MSG_TYPE_SERVOS,    //Servo position payload
	MSG_TYPE_MSG,       //Text message
	MSG_TYPE_VOLTS,     //Battery message
	MSG_TYPE_FAIL,      //servo fail payload
	MSG_TYPE_REG,       //AX12 reg payload
	MSG_TYPE_ERROR,
	//errors Arb->controller
	MSG_TYPE_UNKNOWN,
	BAD_CHECKSUM,
	BAD_CONTEXT,
	LOW_VOLTAGE,
    SERVO_ERROR,
	NOMSG_TIMEOUT,
	MSG_TYPE_COUNT
}
msgType_enum;

#define DEBUG_MSG_L {\
		"Msg_Type_None",\
		"Msg_Type_Getstatus",\
		"Msg_Type_Halt",\
		"Msg_Type_Walk",\
		"Msg_Type_Body",\
		"Msg_Type_Rotate",\
		"Msg_Type_Gait",\
		"Msg_Type_Write_Reg",\
		"Msg_Type_Write_Reg2",\
		"Msg_Type_Relax",\
		"Msg_Type_Torque",\
		"Msg_Type_Stand",\
		"Msg_Type_Sit",\
		"Msg_Type_Posemode",\
		"Msg_Type_Move",\
		"Msg_Type_Read_Reg",\
		"Msg_Type_Set_Pose",\
		"Msg_Type_Set_Servos",\
		"Msg_Type_Get_Pose",\
		"Msg_Type_Status",\
		"Msg_Type_Odometry",\
		"Msg_Type_Pose",\
		"Msg_Type_Servos",\
		"Msg_Type_Msg",\
		"Msg_Type_Volts",\
		"Msg_Type_Fail",\
		"Msg_Type_Reg",\
		"Msg_Type_Error",\
		"Msg_Type_Unknown",\
		"Bad_Checksum",\
		"Bad_Context",\
		"Low_Voltage",\
		"Servo_Error",\
		"Nomsg_Timeout"\
}

//ArbotixM State enum
typedef enum {
	AX_RELAXED,        //torque off
	AX_LOWVOLTAGE,
	AX_TORQUED,        //powered, torque on, sitting
	AX_READY,          //standing ready to walk
	AX_WALKING,
	AX_POSING,         //under pose control
	AX_POSE_READY,     //under pose control
	AX_STOPPING,       //in process
	AX_SITTING,        //in process
	AX_STANDING,       //in process
	AX_STATE_COUNT
}
axState_enum;

#define AX_STATE_NAMES {"relaxed","lowvolts","torqued","ready","walking","posing",\
		"stopping","sitting","standing"}
//servo enum
typedef enum {
LFC,
RFC,
LFF,
RFF,
LFT,
RFT,
LRC,
RRC,
LRF,
RRF,
LRT,
RRT,
LMC,
RMC,
LMF,
RMF,
LMT,
RMT
}
servo_enum;

#define SERVO_NAMES {\
"LFC",\
"RFC",\
"LFF",\
"RFF",\
"LFT",\
"RFT",\
"LRC",\
"RRC",\
"LRF",\
"RRF",\
"LRT",\
"RRT",\
"LMC",\
"RMC",\
"LMF",\
"RMF",\
"LMT",\
"RMT"\
}

//gaits
#define RIPPLE                  0
#define RIPPLE_SMOOTH           3
#define AMBLE                   4
#define AMBLE_SMOOTH            5
#define TRIPOD                  6

#define STANDUP              0x10
#define SITDOWN              0x11
#define STANCE               0x12

//legs
typedef enum {
	RF, RR, LF, LR, RM, LM
}
leg_enum;

#define LEG_NAMES {"RF", "RR", "LF", "LR", "RM", "LM"}

#endif


