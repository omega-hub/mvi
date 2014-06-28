from mvi import *

wm = WorkspaceManager.create()

center = wm.createLayout('center')
center.createWorkspace('full', 'mvi/icons/center-full.png', 't0x0 t1x0')

split = wm.createLayout('split')
split.createWorkspace('left',  'mvi/icons/left-12.png', 't0x0')
split.createWorkspace('right', 'mvi/icons/right-12.png', 't1x0')

wm.setActiveWorkspace('center full')