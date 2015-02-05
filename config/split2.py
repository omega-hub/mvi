from mvi import *



def initWorkspace():
    wl = WorkspaceLibrary.create()

    center = wl.createLayout('center')
    print(center)
    center.createWorkspace('full', 'mvi/icons/center-full.png', 0, 0, 1, 1)

    split = wl.createLayout('split')
    split.createWorkspace('left',  'mvi/icons/left-12.png', 0, 0, 0.5, 1)
    split.createWorkspace('right', 'mvi/icons/right-12.png', 0.5, 0, 0.5, 1)
    split.createWorkspace('close', 'mvi/icons/close.png', 0, 0, 0, 0)

    ac = AppController.create()
    ac.setShortcut(EventFlags.ButtonLeft, "split left")
    ac.setShortcut(EventFlags.ButtonRight, "split right")
    ac.setShortcut(EventFlags.ButtonUp, "center full")
    ac.setShortcut(EventFlags.ButtonDown, "split close", "oexit()")

    ac.setButton(0, loadImage('mvi/icons/close.png'), 'oexit()')


if(mviInit()):    
    queueCommand('initWorkspace()')