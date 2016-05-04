//
//  MsgMacros.h
//  Common
//
//  Created by Martin Lane-Smith on 1/25/14.
//  Copyright (c) 2013 Martin Lane-Smith. All rights reserved.
//

//#define messagemacro(enum, qos, topic, payload, long name)

messagemacro(PS_UNSPECIFIED,PS_QOS3,PS_UNDEFINED_TOPIC,PS_UNKNOWN_PAYLOAD,"Unknown")

//logging
messagemacro(SYSLOG_MSG,PS_QOS2,LOG_TOPIC,PS_SYSLOG_PAYLOAD,"Sys Log")

//system management (generally down the chain of command)
messagemacro(PING_MSG,PS_QOS2,ANNOUNCEMENTS_TOPIC,PS_NO_PAYLOAD,"Ping")
messagemacro(PING_RESPONSE,PS_QOS2,RESPONSE_TOPIC,PS_RESPONSE_PAYLOAD,"Ping response")      	//I am alive

messagemacro(TICK_1S,PS_QOS2,TICK_1S_TOPIC,PS_TICK_PAYLOAD,"1S Tick")

//state management
messagemacro(SS_ONLINE,PS_QOS1,RESPONSE_TOPIC,PS_RESPONSE_PAYLOAD,"SS Online")        		//I am alive

//Notifications
messagemacro(NOTIFY,PS_QOS1,EVENTS_TOPIC,PS_INT_PAYLOAD,"Notify")                               //Event
messagemacro(CONDITIONS,PS_QOS1,CONDITIONS_TOPIC,PS_MASK_PAYLOAD,"Conditions")                //Conditions

//config
messagemacro(CONFIG,PS_QOS2,ANNOUNCEMENTS_TOPIC,PS_CONFIG_PAYLOAD,"Send Config")            //send available settings and options
messagemacro(SETTING,PS_QOS1,CONFIG_TOPIC,PS_SETTING_PAYLOAD,"Setting")
messagemacro(OPTION,PS_QOS1,CONFIG_TOPIC,PS_OPTION_PAYLOAD,"Option")
messagemacro(CONFIG_DONE,PS_QOS1,CONFIG_TOPIC,PS_CONFIG_PAYLOAD,"Config Sent")       		//sent available settings and options
messagemacro(NEW_SETTING,PS_QOS1,ANNOUNCEMENTS_TOPIC,PS_SETTING_PAYLOAD,"New Setting")      //change setting (float)
messagemacro(SET_OPTION,PS_QOS1,ANNOUNCEMENTS_TOPIC,PS_OPTION_PAYLOAD,"Set Option")         //change option  (int/bool)

messagemacro(COMMS_STATS,PS_QOS3,SYS_REPORT_TOPIC,PS_COMMS_STATS_PAYLOAD,"Comms Stats")

//environment reports
messagemacro(BATTERY,PS_QOS1,SYS_REPORT_TOPIC,PS_BATTERY_PAYLOAD,"Battery")

//General data reports (-> App)
messagemacro(FLOAT_DATA,PS_QOS3,SYS_REPORT_TOPIC,PS_NAME_FLOAT_PAYLOAD,"fData")
messagemacro(INT_DATA,PS_QOS3,SYS_REPORT_TOPIC,PS_NAME_INT_PAYLOAD,"iData")

//raw sensors (sensor -> navigator)
messagemacro(IMU_REPORT,PS_QOS3,RAW_NAV_TOPIC,PS_3FLOAT_PAYLOAD,"IMU")
messagemacro(ODOMETRY,PS_QOS2,RAW_NAV_TOPIC,PS_ODOMETRY_PAYLOAD,"Odometry")

//Proximity
//messagemacro(PROXREP,PS_QOS2,PROX_TOPIC,PS_PROX_SUMMARY_PAYLOAD,"Prox Report")

//cooked sensors
messagemacro(POSE,PS_QOS2,NAV_TOPIC,PS_POSE_PAYLOAD,"Pose")
messagemacro(POSEREP,PS_QOS2,SYS_REPORT_TOPIC,PS_POSE_PAYLOAD,"Poserep")						//version for APP

//raw movement -> Arbotix
messagemacro(MOVE,PS_QOS2,ARBOTIX_TOPIC,PS_MOVE_PAYLOAD,"Move")
messagemacro(ARB_STATE,PS_QOS2,REPORT_TOPIC,PS_BYTE_PAYLOAD,"Arbotix State")				//HEXBUG only

//Gripper
messagemacro(GRIP,PS_QOS2,GRIPPER_TOPIC,PS_2FLOAT_PAYLOAD,"Grip")

//OVM reports (-> App)
messagemacro(ACTIVITY,PS_QOS1,SYS_REPORT_TOPIC,PS_BEHAVIOR_STATUS,"Active Behavior")
messagemacro(SCRIPT,PS_QOS1,CONFIG_TOPIC,PS_NAME_PAYLOAD,"Available Script")

//OVM script control
messagemacro(ACTIVATE,PS_QOS1,ACTION_TOPIC,PS_NAME_PAYLOAD,"Execute Script")
messagemacro(RELOAD,PS_QOS1,ACTION_TOPIC,PS_NO_PAYLOAD,"Reload Scripts")

//Alert
messagemacro(ALERT,PS_QOS1,SYS_REPORT_TOPIC,PS_NAME_PAYLOAD,"Alert")

messagemacro(KEEPALIVE,PS_QOS1,PS_UNDEFINED_TOPIC,PS_NO_PAYLOAD,"Keepalive")

//#undef messagemacro
