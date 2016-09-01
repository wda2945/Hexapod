--[[
		"SaveSettings",
		"LoadSettings",
		"ResetSavedSettings",
	    "SystemPoweroff",
	    "SystemSetResting",
	    "SystemSetActive",
	    "ReloadScripts",
	    "isAppOnline",
]]

BT.Task:new({
  name = 'SaveSettings',
  run = function(self, object)
  	result(SystemAction(system.SaveSettings))
  end
})

BT.Task:new({
  name = 'LoadSettings',
  run = function(self, object)
  	result(SystemAction(system.LoadSettings))
  end
})

BT.Task:new({
  name = 'ResetSavedSettings',
  run = function(self, object)
  	result(SystemAction(system.ResetSavedSettings))
  end
})

BT.Task:new({
  name = 'SystemPoweroff',
  run = function(self, object)
  	result(SystemAction(system.SystemPoweroff))
  end
})

BT.Task:new({
  name = 'SystemReboot',
  run = function(self, object)
  	result(SystemAction(system.SystemReboot))
  end
})


BT.Task:new({
  name = 'SystemSetResting',
  run = function(self, object)
  	result(SystemAction(system.SystemSetResting))
  end
})

BT.Task:new({
  name = 'SystemSetActive',
  run = function(self, object)
  	result(SystemAction(system.SystemSetActive))
  end
})

BT.Task:new({
  name = 'ReloadScripts',
  run = function(self, object)
  	result(SystemAction(system.ReloadScripts))
  end
})

BT.Task:new({
  name = 'isAppOnline',
  run = function(self, object)
  	result(SystemAction(system.isAppOnline))
  end
})