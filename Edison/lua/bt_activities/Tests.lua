

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

ActivityList[NextActivity] =  'TurnLeft90'
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

ActivityList[NextActivity] =  'TurnNorth'
NextActivity = NextActivity + 1

Forward10 = BT:new({
  name = 'Test.Forward.10',
  tree = BT.Sequence:new({
	name = 'Test.Seq',
		nodes = {
			'DisableFrontCloseStop',
			'DisableRearCloseStop',
			'MoveForward10',
			}
		})
});

ActivityList[NextActivity] =  'Forward10'
NextActivity = NextActivity + 1

Backward10 = BT:new({
  name = 'Test.Backward.10',
  tree = BT.Sequence:new({
	name = 'Test.Seq',
		nodes = {
			'DisableFrontCloseStop',
			'DisableRearCloseStop',
			'MoveBackward10',
			}
		})
});

ActivityList[NextActivity] =  'Backward10'
NextActivity = NextActivity + 1