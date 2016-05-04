//
//  SoftwareProfile.h
//  Hex
//
//  Created by Martin Lane-Smith on 6/17/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#ifndef SoftwareProfile_h
#define SoftwareProfile_h

#define HEXBUG
#define EDISON

#define 	Agent_INIT()		AgentInit()
#define		Arbotix_INIT()		0 //ArbotixInit()
#define		Autopilot_INIT()	0 //AutopilotInit()
#define		Behavior_INIT()		BehaviorInit()
#define		Dancer_INIT()		0 //DancerInit()
#define		Gripper_INIT()		0 //GripperInit()
#define		I2C_INIT()			0 //I2CInit()
#define		Lidar_INIT()		LidarInit()
#define		Navigator_INIT()	0 //NavigatorInit()
#define		Responder_INIT()	ResponderInit()
#define		SysLog_INIT()		SysLogInit()
#define 	Broker_INIT()		BrokerInit()

#define 	Agent_PROCESS_MESSAGE(msg)		AgentProcessMessage(msg)
#define		Arbotix_PROCESS_MESSAGE(msg)	//ArbotixProcessMessage(msg)
#define		Autopilot_PROCESS_MESSAGE(msg)	//AutopilotProcessMessage(msg)
#define		Behavior_PROCESS_MESSAGE(msg)	//BehaviorProcessMessage(msg)
#define		Dancer_PROCESS_MESSAGE(msg)		//DancerProcessMessage(msg)
#define		I2C_PROCESS_MESSAGE(msg)		//I2CProcessMessage(msg)
#define		Navigator_PROCESS_MESSAGE(msg)	//NavigatorProcessMessage(msg)
#define		Responder_PROCESS_MESSAGE(msg)	ResponderProcessMessage(msg)
#define		SysLog_PROCESS_MESSAGE(msg)		SysLogProcessMessage(msg)

#define MAIN_DEBUG
#define AGENT_DEBUG
#define ARBOTIX_DEBUG
#define AUTOPILOT_DEBUG
#define BEHAVIOR_DEBUG
#define BLACKBOARD_DEBUG
#define BROKER_DEBUG
#define GRIPPER_DEBUG
#define I2C_DEBUG
#define LIDAR_DEBUG
#define NAVIGATOR_DEBUG

#define BEHAVIOR_TREE_CLASS	"/home/root/lua/behaviortree/behavior_tree.lua"
#define INIT_SCRIPT_PATH 	"/home/root/lua/init"				//initialization
#define BT_LEAF_PATH		"/home/root/lua/bt_leafs"
#define BT_ACTIVITY_PATH	"/home/root/lua/bt_activities"
#define HOOK_SCRIPT_PATH 	"/home/root/lua/hooks"				//Message hooks
#define GENERAL_SCRIPT_PATH "/home/root/lua/scripts"				//General scripts

#define WAYPOINT_FILE_PATH	"/home/root/data/waypoints.xml"
#define SAVED_SETTINGS_FILE "/home/root/data/settings.txt"
#define SAVED_OPTIONS_FILE  "/home/root/data/options.txt"


#define UART1	0
#define UART2	1

//I2C Peripherals
#define I2C_BUS		1

//IMU 9DOF module
#define IMU_XM_I2C_ADDRESS   	0x1D
#define IMU_G_I2C_ADDRESS   	0x6B

//ADC module
#define ADC_ADDRESS 			0x48
#define ADC_AUDIO_CHAN			0		//audio envelope
#define ADC_BATTERY_CHAN 		2

#define BATTERY_VOLTS_FACTOR	((5.0 / 255) * (11.6 / 1.22) * (11.66 / 11.0) * (11.6 / 12.0))
#define BATTERY_CRITICAL_VOLTS	8.0f
#define BATTERY_LOW_VOLTS		10.0f

//ArbotixM
#define ARB_UART_DEVICE 		"/dev/ttyMFD2"
#define ARB_UART_BAUDRATE 		38400

//RPLIDAR
#define RPLIDAR_UART_DEVICE 	UART1
#define RPLIDAR_UART_BAUDRATE 	115200
#define RPLIDAR_PWM_PIN			MRAA_INTEL_EDISON_GP183
//adjacent pins
#define J19_9					MRAA_INTEL_EDISON_GP14
#define J20_9					MRAA_INTEL_EDISON_GP42

//Gripper
#define GRIPPER_SERVO_PIN		MRAA_INTEL_EDISON_GP182

//Logging levels
#define LOG_TO_SERIAL            SYSLOG_ROUTINE     //printed in real-time
#define SYSLOG_LEVEL             SYSLOG_ROUTINE  	//published log

#endif
