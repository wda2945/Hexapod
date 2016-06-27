/* 
 * File: MsgFormatList.h
 * Author: martin
 *
 * Created on March 23, 2014, 1:11 PM
 */


//formatmacro(enum, type, var, size)
//-ve length for variable length names

formatmacro(PS_UNKNOWN_PAYLOAD,uint8_t,unknownPayload,0)
formatmacro(PS_NO_PAYLOAD,uint8_t,noPayload,0)
        //Logging
//formatmacro(PS_SYSLOG_PAYLOAD,psLogPayload_t,logPayload, sizeof(psLogPayload_t))
        //online msg
formatmacro(PS_RESPONSE_PAYLOAD,psResponsePayload_t,responsePayload,sizeof(psResponsePayload_t))
formatmacro(PS_TICK_PAYLOAD, psTickPayload_t, tickPayload, sizeof(psTickPayload_t))
formatmacro(PS_MASK_PAYLOAD,psEventMaskPayload_t,eventMaskPayload,sizeof(psEventMaskPayload_t))
        //Generic Data
formatmacro(PS_BYTE_PAYLOAD,psBytePayload_t,bytePayload,sizeof(psBytePayload_t))
formatmacro(PS_INT_PAYLOAD,psIntPayload_t,intPayload,sizeof(psIntPayload_t))
formatmacro(PS_FLOAT_PAYLOAD,psFloatPayload_t,floatPayload,sizeof(psFloatPayload_t))
formatmacro(PS_2FLOAT_PAYLOAD,ps2FloatPayload_t,twoFloatPayload,sizeof(ps2FloatPayload_t))
formatmacro(PS_3FLOAT_PAYLOAD,ps3FloatPayload_t,threeFloatPayload,sizeof(ps3FloatPayload_t))
        //Generic named data
formatmacro(PS_NAME_PAYLOAD,psNamePayload_t,namePayload,sizeof(psNamePayload_t))
formatmacro(PS_NAME_BYTE_PAYLOAD,psNameBytePayload_t,nameBytePayload,sizeof(psNameBytePayload_t))
formatmacro(PS_NAME_FLOAT_PAYLOAD,psNameFloatPayload_t,nameFloatPayload,sizeof(psNameFloatPayload_t))
formatmacro(PS_NAME_INT_PAYLOAD,psNameIntPayload_t,nameIntPayload,sizeof(psNameIntPayload_t))
formatmacro(PS_NAME_3FLOAT_PAYLOAD,psName3FloatPayload_t,name3FloatPayload,sizeof(psName3FloatPayload_t))
formatmacro(PS_NAME_3INT_PAYLOAD,psName3IntPayload_t,name3IntPayload,sizeof(psName3IntPayload_t))
formatmacro(PS_NAME_4BYTE_PAYLOAD,psName4BytePayload_t,name4BytePayload,sizeof(psName4BytePayload_t))
		//Config
formatmacro(PS_CONFIG_PAYLOAD,psConfigPayload_t,configPayload,sizeof(psConfigPayload_t))
formatmacro(PS_SETTING_PAYLOAD,psSettingPayload_t,settingPayload,sizeof(psSettingPayload_t))
formatmacro(PS_OPTION_PAYLOAD,psOptionPayload_t,optionPayload,sizeof(psOptionPayload_t))

formatmacro(PS_COMMS_STATS_PAYLOAD,psCommsStatsPayload_t,commsStatsPayload,sizeof(psCommsStatsPayload_t))
    //sensors
formatmacro(PS_IMU_PAYLOAD,psImuPayload_t,imuPayload,sizeof(psImuPayload_t))
formatmacro(PS_PROX_REPORT_PAYLOAD,psProxReportPayload_t,proxReportPayload,sizeof(psProxReportPayload_t))
formatmacro(PS_PROX_SUMMARY_PAYLOAD,psProxSummaryPayload_t,proxSummaryPayload,sizeof(psProxSummaryPayload_t))
formatmacro(PS_ODOMETRY_PAYLOAD,psOdometryPayload_t,odometryPayload,sizeof(psOdometryPayload_t))
formatmacro(PS_POSE_PAYLOAD, psPosePayload_t, posePayload, sizeof(psPosePayload_t))
     //environment
formatmacro(PS_BATTERY_PAYLOAD, psBatteryPayload_t, batteryPayload, sizeof(psBatteryPayload_t))
//motor
formatmacro(PS_MOVE_PAYLOAD, psMovePayload_t, movePayload, sizeof(psMovePayload_t))
//behavior
formatmacro(PS_BEHAVIOR_STATUS, psBehaviorStatusPayload_t, behaviorStatusPayload, sizeof(psBehaviorStatusPayload_t))


