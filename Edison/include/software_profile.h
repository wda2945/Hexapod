//
//  software_profile.h
//  Hexapod
//
//  Created by Martin Lane-Smith on 6/17/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#ifndef software_profile_h
#define software_profile_h

#define		Arbotix_INIT()		ArbotixInit()
#define		Autopilot_INIT()	0 //AutopilotInit()
#define		Behavior_INIT()		BehaviorInit()
#define		Dancer_INIT()		0 //DancerInit()
#define		I2C_INIT()			I2CInit()
#define		Scanner_INIT()		ScannerInit()
#define		Navigator_INIT()	0 //NavigatorInit()
#define		Responder_INIT()	ResponderInit()

#define MAIN_DEBUG
#define ARBOTIX_DEBUG
#define AUTOPILOT_DEBUG
#define BEHAVIOR_DEBUG
#define I2C_DEBUG
#define LIDAR_DEBUG
#define NAVIGATOR_DEBUG
#define SCANNER_DEBUG

#define BEHAVIOR_TREE_CLASS	"/home/root/lua/behaviortree/behavior_tree.lua"
#define INIT_SCRIPT_PATH 	"/home/root/lua/init"				//initialization
#define BT_LEAF_PATH		"/home/root/lua/bt_leafs"
#define BT_ACTIVITY_PATH	"/home/root/lua/bt_activities"
#define HOOK_SCRIPT_PATH 	"/home/root/lua/hooks"				//Message hooks
#define GENERAL_SCRIPT_PATH "/home/root/lua/scripts"				//General scripts

#define WAYPOINT_FILE_PATH	"/home/root/data/waypoints.xml"
#define SAVED_SETTINGS_FILE "/home/root/data/settings.txt"
#define SAVED_OPTIONS_FILE  "/home/root/data/options.txt"

#define POSE_FOLDER 		"/home/root/poses"

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

//ArbotixM on UART 2
//#define ARB_UART_DEVICE 		"/dev/ttyMFD2"
//#define ARB_UART_BAUDRATE 		38400
//#define ARB_UART_RAW

//ArbotixM on UART 1
#define ARB_UART_DEVICE 		UART1
#define ARB_UART_BAUDRATE 		38400

//Unused PWM
//#define GRIPPER_SERVO_PIN		MRAA_INTEL_EDISON_GP182
//#define RPLIDAR_PWM_PIN		MRAA_INTEL_EDISON_GP183
//adjacent pins
//#define J19_9					MRAA_INTEL_EDISON_GP14
//#define J20_9					MRAA_INTEL_EDISON_GP42

#endif //software_profile_h
