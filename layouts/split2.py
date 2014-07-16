from mvi import *

wm = WorkspaceManager.create()

center = wm.createLayout('center')
center.createWorkspace('full', 'mvi/icons/center-full.png', 0, 0, 1, 1)

split = wm.createLayout('split')
split.createWorkspace('left',  'mvi/icons/left-12.png', 0, 0, 0.5, 1)
split.createWorkspace('right', 'mvi/icons/right-12.png', 0.5, 0, 0.5, 1)

wm.setActiveWorkspace('center full')

_co = AppControlOverlay.create()