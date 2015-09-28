from mvi import *

# Sets the canvas rect using normalized coordinates.
def canvas(x, y, w, h):
    margin = 2
    ps = getDisplayPixelSize()
    dc = getDisplayConfig()
    
    x = int(x * ps[0]) + margin
    y = int(y * ps[1]) + margin
    w = int(w * ps[0]) - margin * 2
    h = int(h * ps[1]) - margin * 2
    
    r = (x, y, w, h)
    dc.setCanvasRect(r)
    dc.bringToFront()
    
    mcc = getMissionControlClient()
    mcc.postCommand("@server: appmgr.onAppCanvasChange('{0}', {1}, {2}, {3}, {4})".format(mcc.getName(), x, y, w, h))

def onEvent():
    e = getEvent()
    #if(e.getServiceType() == ServiceType.Pointer):
    #    if(e.isButtonDown(EventFlags.ButtonLeft)): canvas(0, 0, 0.5, 1)
    #    elif(e.isButtonDown(EventFlags.ButtonRight)): canvas(0.5, 0, 0.5, 1)
    #    elif(e.isButtonDown(EventFlags.ButtonUp)): canvas(0, 0, 1, 1)


if(mviInit()):    
    setEventFunction(onEvent)