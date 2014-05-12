from mvi import *

wm = WorkspaceManager.create()

left = wm.createWorkspace('left')
left.setTiles('t0x0')

center = wm.createWorkspace('center')
center.setTiles('t1x0')

right = wm.createWorkspace('right')
right.setTiles('t2x0')

full = wm.createWorkspace('full')
full.setTiles('t0x0 t1x0 t2x0')
