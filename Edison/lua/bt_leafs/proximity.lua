--[[
	isFrontLeftProximity,
	isFrontProximity,
	isFrontRightProximity,
	isRearLeftProximity,
	isRearProximity,
	isRearRightProximity,
	isLeftProximity,
	isRightProximity,
	isFrontLeftFarProximity,
	isFrontFarProximity,
	isFrontRightFarProximity,
]]
	
BT.Task:new({
  name = 'isFrontLeftProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isFrontLeftProximity))
  end
})
	
BT.Task:new({
  name = 'isFrontProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isFrontProximity))
  end
})
	
BT.Task:new({
  name = 'isFrontRightProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isFrontRightProximity))
  end
})
	
BT.Task:new({
  name = 'isRearLeftProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isRearLeftProximity))
  end
})
	
BT.Task:new({
  name = 'isRearProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isRearProximity))
  end
})
	
BT.Task:new({
  name = 'isRearRightProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isRearRightProximity))
  end
})
	
BT.Task:new({
  name = 'isLeftProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isLeftProximity))
  end
})
	
BT.Task:new({
  name = 'isRightProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isRightProximity))
  end
})
	
BT.Task:new({
  name = 'isFrontLeftFarProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isFrontLeftFarProximity))
  end
})
	
BT.Task:new({
  name = 'isFrontFarProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isFrontFarProximity))
  end
})
	
BT.Task:new({
  name = 'isFrontRightFarProximity',
  run = function(self, object)
  	result(ProximityAction(proximity.isFrontRightFarProximity))
  end
})
	