from threading import Timer
import porthole
webServerPort = 4080

# Launch the web server
porthole.initialize(webServerPort, './index.html')
webServer = porthole.getService()

# launch the views
views = []
def runView():
    global views
    views.append(appmgr.run('icecloud/apps/endurance.py'))

Timer(0, runView).start()
Timer(10, runView).start()
Timer(20, runView).start()

def canvas(viewid, name):
    canvasString = ""
    if(name == "left-third"): canvasString = "0,0,0.33,1"
    if(name == "left-half"): canvasString = "0,0,0.5,1"
    if(name == "center"): canvasString = "0.33,0,0.33,1"
    if(name == "right-third"): canvasString = "0.66,0,0.33,1"
    if(name == "right-half"): canvasString = "0.5,0,0.5,1"
    if(name == "full"): canvasString = "0,0,1,1"
    if(name == "hide"): canvasString = "0,0,0.001,0.01"
    
    mcc = getMissionControlClient()
    cmd = "@{0}: canvas({1})".format(views[viewid], canvasString)
    print(cmd)
    mcc.postCommand(cmd)
    