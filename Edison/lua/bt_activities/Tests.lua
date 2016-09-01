

TurnLeft90 = BT:new({
  name = 'Test.Turn.Left.90',
  tree = BT.Sequence:new({
	name = 'Test.Seq',
		nodes = {
			'DisableFrontCloseStop',
			'DisableRearCloseStop',
			'TurnLeft90',
			}
		})
});

ActivityList[NextActivity] =  'TurnLeft90'
NextActivity = NextActivity + 1

TurnRight90 = BT:new({
  name = 'Test.Turn.Right.90',
  tree = BT.Sequence:new({
	name = 'Test.Seq',
		nodes = {
			'DisableFrontCloseStop',
			'DisableRearCloseStop',
			'TurnRight90',
			}
		})
});

ActivityList[NextActivity] =  'TurnRight90'
NextActivity = NextActivity + 1

TurnNorth = BT:new({
  name = 'Test.Turn.North',
  tree = BT.Sequence:new({
	name = 'Test.Seq',
		nodes = {
			'DisableFrontCloseStop',
			'DisableRearCloseStop',
			'TurnNorth',
			}
		})
});

ActivityList[NextActivity] =  'TurnNorth'
NextActivity = NextActivity + 1

Forward30 = BT:new({
  name = 'Test.Forward.30',
  tree = BT.Sequence:new({
	name = 'Test.Seq',
		nodes = {
			'DisableFrontCloseStop',
			'DisableRearCloseStop',
			'MoveForward30',
			}
		})
});

ActivityList[NextActivity] =  'Forward30'
NextActivity = NextActivity + 1

Backward30 = BT:new({
  name = 'Test.Backward.30',
  tree = BT.Sequence:new({
	name = 'Test.Seq',
		nodes = {
			'DisableFrontCloseStop',
			'DisableRearCloseStop',
			'MoveBackward30',
			}
		})
});

ActivityList[NextActivity] =  'Backward30'
NextActivity = NextActivity + 1

SetPoseMode = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'SetPoseMode',
			}
		})
});

ClapOutPose = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'ClapOutPose',
			}
		})
});

ClapInPose = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'ClapInPose',
			}
		})
});

BodyLeftPose = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'BodyLeftPose',
			}
		})
});

BodyRightPose = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'BodyRightPose',
			}
		})
});

BodyUpPose = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'BodyUpPose',
			}
		})
});

BodyDownPose = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'BodyDownPose',
			}
		})
});


TravoltaRightHighPose = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'TravoltaRightHighPose',
			}
		})
});


TravoltaRightLowPose = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'TravoltaRightLowPose',
			}
		})
});

DefaultPose = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'DefaultPose',
			}
		})
});

Dance = BT:new({
  tree = BT.Sequence:new({
		nodes = {
			'SetPoseMode',
			'DefaultPose',
			'BodyLeftPose',
			'BodyRightPose',
			'BodyLeftPose',
			'BodyRightPose',
			'BodyUpPose',
			'BodyDownPose',
			'BodyUpPose',
			'BodyDownPose',
			'ClapOutPose',
			'ClapInPose',
			'ClapOutPose',
			'ClapInPose',
			'TravoltaRightHighPose',
			'TravoltaRightLowPose',
			'TravoltaRightHighPose',
			'TravoltaRightLowPose',
			'DefaultPose'
			}
		})
});

ActivityList[NextActivity] =  'SetPoseMode'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'ClapOutPose'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'ClapInPose'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'BodyLeftPose'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'BodyRightPose'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'BodyUpPose'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'BodyDownPose'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'TravoltaRightHighPose'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'TravoltaRightLowPose'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'DefaultPose'
NextActivity = NextActivity + 1
ActivityList[NextActivity] =  'Dance'
NextActivity = NextActivity + 1

