from mvi import *

wl = WorkspaceLibrary.create()

center = wl.createLayout('center')
center.createWorkspace('full', 'mvi/icons/center-full.png', 0, 0, 1, 1)

split = wl.createLayout('split')
split.createWorkspace('left',  'mvi/icons/left-12.png', 0, 0, 0.5, 1)
split.createWorkspace('right', 'mvi/icons/right-12.png', 0.5, 0, 0.5, 1)

ac = AppController.create()
ac.setShortcut(EventFlags.ButtonLeft, "split left")
ac.setShortcut(EventFlags.ButtonRight, "split right")
ac.setShortcut(EventFlags.ButtonUp, "center full")
ac.setShortcut(EventFlags.ButtonDown, "split right")