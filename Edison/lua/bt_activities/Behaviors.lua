
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

ActivityList[NextActivity] =  'ReloadAllScripts'
NextActivity = NextActivity + 1

PowerOff = BT:new({
  tree = 'SystemPoweroff'
})

ActivityList[NextActivity] =  'PowerOff'
NextActivity = NextActivity + 1

Sleep = BT:new({
  tree = 'SystemSetResting'
})

ActivityList[NextActivity] =  'Sleep'
NextActivity = NextActivity + 1

Sit = BT:new({
  tree = 'Sit'
})

ActivityList[NextActivity] =  'Sit'
NextActivity = NextActivity + 1

Stand = BT:new({
  tree = 'Stand'
})

ActivityList[NextActivity] =  'Stand'
NextActivity = NextActivity + 1