
BT.Priority:new({
	name = 'isBatteryOKish',		-- battery not critical; OK or low
	nodes = {
		'isBatteryOK',
		'isBatteryLow',
		'BatteryFailAlert'
		}
});

ReloadScripts = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'ReloadScripts',
			}
		})
});

SystemPoweroff = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'SystemPoweroff',
			}
		})
});

SystemReboot = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'SystemReboot',
			}
		})
});

SystemSetResting = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'SystemSetResting',
			}
		})
});

Stop = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'Stop',
			}
		})
});

Sit = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'Sit',
			}
		})
});

Stand = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'Stand',
			}
		})
});


ActivityList[NextActivity] =  'ReloadScripts'
NextActivity = NextActivity + 1

ActivityList[NextActivity] =  'SystemPoweroff'
NextActivity = NextActivity + 1

ActivityList[NextActivity] =  'SystemReboot'
NextActivity = NextActivity + 1

ActivityList[NextActivity] =  'SystemSetResting'
NextActivity = NextActivity + 1

ActivityList[NextActivity] =  'Stop'
NextActivity = NextActivity + 1

ActivityList[NextActivity] =  'Sit'
NextActivity = NextActivity + 1

ActivityList[NextActivity] =  'Stand'
NextActivity = NextActivity + 1