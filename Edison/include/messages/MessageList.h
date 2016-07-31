//
//  MsgMacros.h
//  Common
//
//  Created by Martin Lane-Smith on 1/25/14.
//  Copyright (c) 2013 Martin Lane-Smith. All rights reserved.
//

//#define messagemacro(enum, qos, topic, payload, long name)

messagemacro(TICK_1S,PS_QOS2,TICK_1S_TOPIC,PS_TICK_PAYLOAD,"1S Tick")

//state management
messagemacro(SS_ONLINE,PS_QOS1,RESPONSE_TOPIC,PS_RESPONSE_PAYLOAD,"SS Online")        		//I am alive

messagemacro(COMMS_STATS,PS_QOS3,SYS_REPORT_TOPIC,PS_COMMS_STATS_PAYLOAD,"Comms Stats")

//raw sensors (sensor -> navigator)
messagemacro(IMU_REPORT,PS_QOS3,IMU_TOPIC,PS_3FLOAT_PAYLOAD,"IMU")
messagemacro(ODOMETRY,PS_QOS2,ODO_TOPIC,PS_ODOMETRY_PAYLOAD,"Odometry")

//Proximity
//messagemacro(PROXREP,PS_QOS2,PROX_TOPIC,PS_PROX_SUMMARY_PAYLOAD,"Prox Report")

//cooked sensors
messagemacro(POSE,PS_QOS2,NAV_TOPIC,PS_POSE_PAYLOAD,"Pose")

//raw movement -> Arbotix
messagemacro(ARB_STATE,PS_QOS2,REPORT_TOPIC,PS_BYTE_PAYLOAD,"Arbotix State")				//HEXBUG only

//#undef messagemacro
