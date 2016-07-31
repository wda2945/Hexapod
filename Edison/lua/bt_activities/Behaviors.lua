
BT.Priority:new({
	name = 'isBatteryOKish',		-- battery not critical; OK or low
	nodes = {
		'isBatteryOK',
		'isBatteryLow',
		'BatteryFailAlert'
		}
});

ReloadAllScripts = BT:new({
  tree = 'ReloadScripts'
})

ActivityList[NextActivity] =  'ReloadScripts'
NextActivity = NextActivity + 1

PowerOff = BT:new({
  tree = 'SystemPoweroff'
})

ActivityList[NextActivity] =  'PowerOff'
NextActivity = NextActivity + 1

Sleep = BT:new({
  tree = 'SystemSetResting'
})

ActivityList[NextActivity] =  'Rest'
NextActivity = NextActivity + 1