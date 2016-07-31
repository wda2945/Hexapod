--local BT = require('/root/lua/behaviortree/behaviour_tree')

--[[
	isPilotReady,
	ComputeHomePosition,
	ComputeRandomExplorePosition,
	ComputeRandomClosePosition,
	Orient,
	Engage,
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
