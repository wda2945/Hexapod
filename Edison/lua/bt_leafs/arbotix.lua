--local BT = require('/root/lua/behaviortree/behaviour_tree')

--[[
	Stand,
	Sit,
	Turn,
	TurnLeft,
	TurnRight,
	TurnN,
	TurnS,
	TurnE,
	TurnW,
	TurnLeft90,
	TurnRight90,
	MoveForward,
	MoveBackward,
	MoveForward10,
	MoveBackward10,
	SetFastSpeed,
	SetMediumSpeed,
	SetLowSpeed,
	EnableFrontCloseStop,
	DisableFrontCloseStop,
	EnableRearCloseStop,
	DisableRearCloseStop,
	EnableFrontFarStop,
	DisableFrontFarStop,
]]

BT.Task:new({
  name = 'Stand',
  run = function(self, object)
  	result(HexapodAction(hexapod.Stand))
  end
})

BT.Task:new({
  name = 'Sit',
  run = function(self, object)
  	result(HexapodAction(hexapod.Sit))
  end
})

BT.Task:new({
  name = 'Turn',
  run = function(self, object)
  	result(HexapodAction(hexapod.Turn))
  end
})

BT.Task:new({
  name = 'TurnLeft',
  run = function(self, object)
  	result(HexapodAction(hexapod.TurnLeft))
  end
})

BT.Task:new({
  name = 'TurnRight',
  run = function(self, object)
  	result(HexapodAction(hexapod.TurnRight))
  end
})

BT.Task:new({
  name = 'TurnNorth',
  run = function(self, object)
  	result(HexapodAction(hexapod.TurnN))
  end
})

BT.Task:new({
  name = 'TurnSouth',
  run = function(self, object)
  	result(HexapodAction(hexapod.TurnS))
  end
})

BT.Task:new({
  name = 'TurnEast',
  run = function(self, object)
  	result(HexapodAction(hexapod.TurnE))
  end
})

BT.Task:new({
  name = 'TurnWest',
  run = function(self, object)
  	result(HexapodAction(hexapod.TurnW))
  end
})

BT.Task:new({
  name = 'TurnLeft90',
  run = function(self, object)
  	result(HexapodAction(hexapod.TurnLeft90))
  end
})

BT.Task:new({
  name = 'TurnRight90',
  run = function(self, object)
  	result(HexapodAction(hexapod.TurnRight90))
  end
})

BT.Task:new({
  name = 'MoveForward',
  run = function(self, object)
  	result(HexapodAction(hexapod.MoveForward))
  end
})

BT.Task:new({
  name = 'MoveBackward',
  run = function(self, object)
  	result(HexapodAction(hexapod.MoveBackward))
  end
})

BT.Task:new({
  name = 'MoveForward10',
  run = function(self, object)
  	result(HexapodAction(hexapod.MoveForward10))
  end
})

BT.Task:new({
  name = 'MoveBackward10',
  run = function(self, object)
  	result(HexapodAction(hexapod.MoveBackward10))
  end
})

BT.Task:new({
  name = 'SetFastSpeed',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetFastSpeed))
  end
})

BT.Task:new({
  name = 'SetMediumSpeed',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetMediumSpeed))
  end
})

BT.Task:new({
  name = 'SetLowSpeed',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetLowSpeed))
  end
})

BT.Task:new({
  name = 'EnableFrontCloseStop',
  run = function(self, object)
  	result(HexapodAction(hexapod.EnableFrontCloseStop))
  end
})

BT.Task:new({
  name = 'DisableFrontCloseStop',
  run = function(self, object)
  	result(HexapodAction(hexapod.DisableFrontCloseStop))
  end
})

BT.Task:new({
  name = 'EnableRearCloseStop',
  run = function(self, object)
  	result(HexapodAction(hexapod.EnableRearCloseStop))
  end
})

BT.Task:new({
  name = 'DisableRearCloseStop',
  run = function(self, object)
  	result(HexapodAction(hexapod.DisableRearCloseStop))
  end
})

BT.Task:new({
  name = 'EnableFrontFarStop',
  run = function(self, object)
  	result(HexapodAction(hexapod.EnableFrontFarStop))
  end
})

BT.Task:new({
  name = 'DisableFrontFarStop',
  run = function(self, object)
  	result(HexapodAction(hexapod.DisableFrontFarStop))
  end
})
