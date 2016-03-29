--local BT = require('/root/lua/behaviortree/behaviour_tree')

--[[
	isPilotReady,
	ComputeHomePosition,
	ComputeRandomExplorePosition,
	ComputeRandomClosePosition,
	Orient,
	Engage,
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
  name = 'isPilotReady',
  run = function(self, object)
  	result(PilotAction(pilot.isPilotReady))
  end
})

BT.Task:new({
  name = 'ComputeHomePosition',
  run = function(self, object)
  	result(PilotAction(pilot.ComputeHomePosition))
  end
})

BT.Task:new({
  name = 'ComputeRandomExplorePosition',
  run = function(self, object)
  	result(PilotAction(pilot.ComputeRandomExplorePosition))
  end
})

BT.Task:new({
  name = 'ComputeRandomClosePosition',
  run = function(self, object)
  	result(PilotAction(pilot.ComputeRandomClosePosition))
  end
})

BT.Task:new({
  name = 'Orient',
  run = function(self, object)
  	result(PilotAction(pilot.Orient))
  end
})

BT.Task:new({
  name = 'Engage',
  run = function(self, object)
  	result(PilotAction(pilot.Engage))
  end
})

BT.Task:new({
  name = 'TurnLeft',
  run = function(self, object)
  	result(PilotAction(pilot.TurnLeft))
  end
})

BT.Task:new({
  name = 'TurnRight',
  run = function(self, object)
  	result(PilotAction(pilot.TurnRight))
  end
})

BT.Task:new({
  name = 'TurnNorth',
  run = function(self, object)
  	result(PilotAction(pilot.TurnN))
  end
})

BT.Task:new({
  name = 'TurnSouth',
  run = function(self, object)
  	result(PilotAction(pilot.TurnS))
  end
})

BT.Task:new({
  name = 'TurnEast',
  run = function(self, object)
  	result(PilotAction(pilot.TurnE))
  end
})

BT.Task:new({
  name = 'TurnWest',
  run = function(self, object)
  	result(PilotAction(pilot.TurnW))
  end
})

BT.Task:new({
  name = 'TurnLeft90',
  run = function(self, object)
  	result(PilotAction(pilot.TurnLeft90))
  end
})

BT.Task:new({
  name = 'TurnRight90',
  run = function(self, object)
  	result(PilotAction(pilot.TurnRight90))
  end
})

BT.Task:new({
  name = 'MoveForward',
  run = function(self, object)
  	result(PilotAction(pilot.MoveForward))
  end
})

BT.Task:new({
  name = 'MoveBackward',
  run = function(self, object)
  	result(PilotAction(pilot.MoveBackward))
  end
})

BT.Task:new({
  name = 'MoveForward10',
  run = function(self, object)
  	result(PilotAction(pilot.MoveForward10))
  end
})

BT.Task:new({
  name = 'MoveBackward10',
  run = function(self, object)
  	result(PilotAction(pilot.MoveBackward10))
  end
})

BT.Task:new({
  name = 'SetFastSpeed',
  run = function(self, object)
  	result(PilotAction(pilot.SetFastSpeed))
  end
})

BT.Task:new({
  name = 'SetMediumSpeed',
  run = function(self, object)
  	result(PilotAction(pilot.SetMediumSpeed))
  end
})

BT.Task:new({
  name = 'SetLowSpeed',
  run = function(self, object)
  	result(PilotAction(pilot.SetLowSpeed))
  end
})

BT.Task:new({
  name = 'EnableFrontCloseStop',
  run = function(self, object)
  	result(PilotAction(pilot.EnableFrontCloseStop))
  end
})

BT.Task:new({
  name = 'DisableFrontCloseStop',
  run = function(self, object)
  	result(PilotAction(pilot.DisableFrontCloseStop))
  end
})

BT.Task:new({
  name = 'EnableRearCloseStop',
  run = function(self, object)
  	result(PilotAction(pilot.EnableRearCloseStop))
  end
})

BT.Task:new({
  name = 'DisableRearCloseStop',
  run = function(self, object)
  	result(PilotAction(pilot.DisableRearCloseStop))
  end
})

BT.Task:new({
  name = 'EnableFrontFarStop',
  run = function(self, object)
  	result(PilotAction(pilot.EnableFrontFarStop))
  end
})

BT.Task:new({
  name = 'DisableFrontFarStop',
  run = function(self, object)
  	result(PilotAction(pilot.DisableFrontFarStop))
  end
})
