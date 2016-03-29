

Test90 = BT:new({
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

ActivityList[NextActivity] =  'Test90'
NextActivity = NextActivity + 1

TestNorth = BT:new({
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

ActivityList[NextActivity] =  'TestNorth'
NextActivity = NextActivity + 1

TestMove10 = BT:new({
  name = 'Test.Move.10',
  tree = BT.Sequence:new({
	name = 'Test.Seq',
		nodes = {
			'DisableFrontCloseStop',
			'DisableRearCloseStop',
			'MoveForward10',
			}
		})
});

ActivityList[NextActivity] =  'TestMove10'
NextActivity = NextActivity + 1
