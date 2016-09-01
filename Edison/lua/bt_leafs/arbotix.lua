--local BT = require('/root/lua/behaviortree/behaviour_tree')

--[[
	Stop,
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
	SetPoseMode,
	SetPoseSlow,
	SetPoseMedium,
	SetPoseFast,
	SetPoseBeat,
	SetPoseDownbeat,
	SetPoseUpbeat,
	EnableFrontCloseStop,
	DisableFrontCloseStop,
	EnableRearCloseStop,
	DisableRearCloseStop,
	EnableFrontFarStop,
	DisableFrontFarStop,
	
	ClapOutPose,
	ClapInPose,
	BodyLeftPose,
	BodyRightPose,
	BodyUpPose,
	BodyDownPose,
	DefaultPose,
	
--]]

BT.Task:new({
  name = 'Stop',
  run = function(self, object)
  	result(HexapodAction(hexapod.Stop))
  end
})

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
  name = 'MoveForward30',
  run = function(self, object)
  	result(HexapodAction(hexapod.MoveForward30))
  end
})

BT.Task:new({
  name = 'MoveBackward30',
  run = function(self, object)
  	result(HexapodAction(hexapod.MoveBackward30))
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
  name = 'SetPoseMode',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetPoseMode))
  end
})

BT.Task:new({
  name = 'SetPoseSlow',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetPoseSlow))
  end
})
BT.Task:new({
  name = 'SetPoseMedium',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetPoseMedium))
  end
})
BT.Task:new({
  name = 'SetPoseFast',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetPoseFast))
  end
})
BT.Task:new({
  name = 'SetPoseBeat',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetPoseBeat))
  end
})
BT.Task:new({
  name = 'SetPoseDownbeat',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetPoseDownbeat))
  end
})
BT.Task:new({
  name = 'SetPoseUpbeat',
  run = function(self, object)
  	result(HexapodAction(hexapod.SetPoseUpbeat))
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

BT.Task:new({
  name = 'ClapOutPose',
  run = function(self, object)
  	result(HexapodPose('clap_out'))
  end
})

BT.Task:new({
  name = 'ClapInPose',
  run = function(self, object)
  	result(HexapodPose('clap_in'))
  end
})

BT.Task:new({
  name = 'BodyLeftPose',
  run = function(self, object)
  	result(HexapodPose('body_left'))
  end
})

BT.Task:new({
  name = 'BodyRightPose',
  run = function(self, object)
  	result(HexapodPose('body_right'))
  end
})

BT.Task:new({
  name = 'BodyUpPose',
  run = function(self, object)
  	result(HexapodPose('body_up'))
  end
})

BT.Task:new({
  name = 'BodyDownPose',
  run = function(self, object)
  	result(HexapodPose('body_down'))
  end
})

BT.Task:new({
  name = 'TravoltaRightHighPose',
  run = function(self, object)
  	result(HexapodPose('travolta_right_high'))
  end
})


BT.Task:new({
  name = 'TravoltaRightLowPose',
  run = function(self, object)
  	result(HexapodPose('travolta_right_low'))
  end
})

BT.Task:new({
  name = 'DefaultPose',
  run = function(self, object)
  	result(HexapodPose('default'))
  end
})

