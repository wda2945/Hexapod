//
//  Settings.h
//  Hex
//
//  Created by Martin Lane-Smith on 6/14/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

//settingmacro(name, var, min, max, def)
settingmacro("Arb Timeout", arbTimeout, 1, 20, 10)

settingmacro("Close Range", closeRange, 1, 100, 30)
settingmacro("Far Range", farRange, 1, 100, 60)
settingmacro("ProxThreshold", proxThreshold, 0.0, 1, 0.25)

//navigator
settingmacro("Nav Loop mS", navLoopDelay, 100, 1000, 500)
settingmacro("Nav IMU Loop mS", imuLoopDelay, 100, 5000, 500)
settingmacro("Nav App Report Interval S", appReportInterval, 1, 60, 10)
settingmacro("Nav Kalman Gain", KalmanGain, 0, 1, 0)

//pilot
settingmacro("Pilot Loop mS", pilotLoopDelay, 100, 1000, 500)
settingmacro("Pilot Arrival Heading", arrivalHeading, 1, 30, 10)
settingmacro("Pilot Arrival Range", arrivalRange, 1, 30, 10)
settingmacro("Pilot FastSpeed", FastSpeed, 10, 100, 50)
settingmacro("Pilot MediumSpeed", MediumSpeed, 10, 100, 25)
settingmacro("Pilot SlowSpeed", SlowSpeed, 10, 100, 10)
settingmacro("Pilot Default Move cm", defMove, 10, 100, 50)
settingmacro("Pilot Default Turn deg", defTurn, 10, 100, 50)
settingmacro("Pilot Default Turnrate", defTurnRate, 10, 100, 50)
settingmacro("Pilot Motor Start TO", motorsStartTimeout, 1, 100, 5)
settingmacro("Pilot Motor TO/cm", timeoutPerCM, 0.1, 1, 0.1)
settingmacro("Pilot Motor Max cm", motorMaxCM, 10, 1000, 100)

//
settingmacro("BT Loop mS", behLoopDelay, 100, 1000, 250)
settingmacro("BT Ping Interval S", pingSecs, 5, 100, 5)
settingmacro("BT Network TO S", networkTimeout, 5, 100, 30)

settingmacro("APP Max Routine", maxUARTRoutine, 0, 100, 0)
settingmacro("APP Max Info", maxUARTInfo, 0, 100, 10)
settingmacro("APP Max Warning", maxUARTWarning, 0, 100, 10)
settingmacro("APP Max Error", maxUARTError, 0, 100, 50)

settingmacro("Arb Fast", arbFastSpeed, 10, 100, 50)
settingmacro("Arb Medium", arbMediumSpeed, 10, 100, 25)
settingmacro("Arb Slow", arbSlowSpeed, 10, 100, 10)
settingmacro("Deg/step", degreesPerStep, 1, 30, 10)
settingmacro("Cm/step", cmPerStep, 1, 100, 20)
