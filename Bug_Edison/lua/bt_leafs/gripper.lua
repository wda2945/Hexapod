--[[
	setGripperSpeedSlow,
	setGripperSpeedFast,
	openGripper,
	closeGripper
]]
	
BT.Task:new({
  name = 'setGripperSpeedSlow',
  run = function(self, object)
  	result(GripperAction(gripper.setGripperSpeedSlow))
  end
})
	
BT.Task:new({
  name = 'setGripperSpeedFast',
  run = function(self, object)
  	result(GripperAction(gripper.setGripperSpeedFast))
  end
})
	
BT.Task:new({
  name = 'openGripper',
  run = function(self, object)
  	result(GripperAction(gripper.openGripper))
  end
})
	
BT.Task:new({
  name = 'closeGripper',
  run = function(self, object)
  	result(GripperAction(gripper.closeGripper))
  end
})
