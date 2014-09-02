#include <omega.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
// Stored data about a single application instance.
struct AppInstance : public ReferenceType
{
    AppInstance():
        targetCanvas(0,0,0,0),
        currentCanvas(0,0,0,0),
        z(0),
        dirtyCanvas(false) {}
        
    String id;
    Rect targetCanvas;
    Rect currentCanvas;
    int z;
    bool dirtyCanvas;
    Ref<MissionControlConnection> connection;
};

// Information about input data coming from a specific user
struct InputInfo: ReferenceType
{
    InputInfo():
        controlMode(false), lockedMode(false) {}
        
    // When set to true input from this user is in control mode
    // i.e. user is controlling layout instead of sending input to a specific app
    bool controlMode;
    // When set to true, we are in control mode and "locked-on" on a single
    // application, moving or resizing its canvas. when this flag is set, events
    // will be forwarded to this app regardless of where the pointer is, to avoid
    // losing control of the app if we are moving the pointer too fast or out
    // of bounds.
    bool lockedMode;
    
    // Target application for this input source;
    Ref<AppInstance> target;
};

///////////////////////////////////////////////////////////////////////////////
// Some useful typedefs.
typedef Dictionary<String, Ref<AppInstance> > AppInstanceDictionary;
typedef Dictionary<uint, Ref<MissionControlConnection> > EventRoutingTable;
typedef Dictionary<uint, Ref<InputInfo> > InputInfoTable;

///////////////////////////////////////////////////////////////////////////////
class AppManager : public EngineModule, public IMissionControlListener
{
public:
    static AppManager* instance();

    AppManager();
    void initialize();
    void handleEvent(const Event& evt);

    // From IMissionControlListener
    void onClientConnected(const String& clientId);
    void onClientDisconnected(const String& clientId);
    bool handleCommand(const String& cmd);
    
    // Called by connected clients
    void setDisplaySize(const String& clientid, int width, int height);
    void onAppCanvasChange(const String& appid, int x, int y, int w, int h);
    void setLauncherApp(const String& appid);

private:
    // Get a 2D pointer out of a pointer or wand event
    bool get2DPointer(const Event& evt, Vector2i& out);
    InputInfo* getOrCreateInputInfo(const Event& evt);
    AppInstance* getAppAt(Vector2i pos);
    
private:
    static AppManager* mysInstance;

    SystemManager* mySys;

    Ref<MissionControlServer> myServer;
    Ref<MissionControlConnection> myServerConnection;

    EventRoutingTable myEventRoutingTable;

    Event::Flags myModeSwitchButton;
    Event::Flags myMoveButton;
    Event::Flags myResizeButton;
    InputInfoTable myInputInfoTable;

    // Pixel size of the connected display
    Vector2i myDisplaySize;

    // App instance data
    AppInstanceDictionary myAppInstances;
    // Refs to special 'system' applications, typically launchers, system apps
    // or desktop apps. They can be launched when in control mode pointing to
    // an area where no other applications are active.
    Ref<AppInstance> myLauncherApp;

    typedef List< Ref<AppInstance> > AppInstanceList;
    AppInstanceList myZSortedAppInstances;
    unsigned int myCurrentTopZ;
};

///////////////////////////////////////////////////////////////////////////////
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(appmgr)
{
    // The python API is mainly used to support communication with connected
    // clients through Mission Control.
    PYAPI_REF_BASE_CLASS(AppManager)
        PYAPI_STATIC_REF_GETTER(AppManager, instance)
        PYAPI_METHOD(AppManager, setDisplaySize)
        PYAPI_METHOD(AppManager, onAppCanvasChange)
        PYAPI_METHOD(AppManager, setLauncherApp)
        ;
}


///////////////////////////////////////////////////////////////////////////////
AppManager* AppManager::mysInstance = NULL;
AppManager* AppManager::instance()
{ 
    return mysInstance; 
}

///////////////////////////////////////////////////////////////////////////////
AppManager::AppManager(): 
    EngineModule("AppManager"),
    mySys(SystemManager::instance()),
    myModeSwitchButton(Event::Alt),
    myMoveButton(Event::Button1),
    myResizeButton(Event::Button2)
{
    mysInstance = this;
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::initialize()
{
    // Read in configuration
    Config* cfg = SystemManager::instance()->getAppConfig();
    if(cfg->exists("config/appmgr"))
    {
        Setting& s = cfg->lookup("config/appmgr");
        String sModeSwitchButton = Config::getStringValue("modeSwitchButton", s, "");
        String sMoveButton = Config::getStringValue("moveButton", s, "");
        String sResizeButton = Config::getStringValue("resizeButton", s, "");
        if(sModeSwitchButton != "") myModeSwitchButton = Event::parseButtonName(sModeSwitchButton);
        if(sMoveButton != "") myMoveButton = Event::parseButtonName(sMoveButton);
        if(sResizeButton != "") myResizeButton = Event::parseButtonName(sResizeButton);
    }

    myServer = mySys->getMissionControlServer();
    oassert(myServer != NULL);

    // Find the server-side connection (used later when broadcasting event to
    // avoid sending the event back to ourselves)
    MissionControlClient* c = mySys->getMissionControlClient();
    oassert(c != NULL);
    myServerConnection = myServer->findConnection(c->getName());
    oassert(myServerConnection != NULL);


    myServer->setListener(this);

    // Use the python interpreter to load and initialize the porthole web server.
    PythonInterpreter* interpreter = mySys->getScriptInterpreter();
    interpreter->eval("import porthole");
    interpreter->eval("porthole.initialize('mvi/appmgr/pointer.xml')");

    // Initialize python API and create a variable 'appmgr' storing the
    // AppManager instance
    initappmgr();
    interpreter->eval("from appmgr import *");
    interpreter->eval("appmgr = AppManager.instance()");


    omsg("Application Manager initialization complete!");
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::onClientConnected(const String& clientId)
{
    ofmsg("Application connected: %1%", %clientId);
    // New client connected: setup a new AppInstance.
    MissionControlConnection* conn = myServer->findConnection(clientId);

    // Setup the connected app controller: configure buttons
    String cmd = ostr("AppController.configPhysicalButtons(%1%, %2%, %3%)",
        %myModeSwitchButton %myMoveButton %myResizeButton);
    conn->sendMessage(MissionControlMessageIds::ScriptCommand, (void*)cmd.c_str(), cmd.size());
    
    AppInstance* ai = new AppInstance();
    ai->id = clientId;
    ai->connection = conn;
    
    myAppInstances[clientId] = ai;
    myZSortedAppInstances.push_front(ai);
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::onClientDisconnected(const String& clientId)
{
    ofmsg("Application disconnected: %1%", %clientId);
    
    AppInstance* ai = myAppInstances[clientId];
    myZSortedAppInstances.remove(ai);
    myAppInstances[clientId] = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::setDisplaySize(const String& clientid, int width, int height)
{
    myDisplaySize = Vector2i(width, height);
    ofmsg("Display size set to %1%", %myDisplaySize);
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::onAppCanvasChange(const String& appid, int x, int y, int w, int h)
{
    AppInstance* ai = myAppInstances[appid];
    oassert(ai != NULL);
    
    ofmsg("CANVAS UPDATE: %1% - (%2% %3% %4% %5%)",
        %appid %x %y %w %h);
    
    ai->targetCanvas = Rect(x, y, w, h);
    ai->currentCanvas = Rect(x, y, w, h);

    // TODO: Update other app canvases here.
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::setLauncherApp(const String& appid)
{
    ofmsg("Setting launcher application to: %1%", %appid);
    AppInstance* ai = myAppInstances[appid];
    oassert(ai != NULL);
    myLauncherApp = ai;
}

///////////////////////////////////////////////////////////////////////////////
InputInfo* AppManager::getOrCreateInputInfo(const Event& evt)
{
    InputInfo* ii = NULL;
    // If we do not have an entry from this user in the input info table,
    // create it now.
    if(myInputInfoTable.find(evt.getUserId()) == myInputInfoTable.end())
    {
        ii = new InputInfo();
        myInputInfoTable[evt.getUserId()] = ii;
    }
    else
    {
        ii = myInputInfoTable[evt.getUserId()];
    }
    return ii;
}

///////////////////////////////////////////////////////////////////////////////
bool AppManager::get2DPointer(const Event& evt, Vector2i& out)
{
    const Vector3f& pos3 = evt.getPosition();
    out[0] = pos3[0];
    out[1] = pos3[1];
    
    if(evt.getServiceType() == Service::Wand && 
        !evt.isExtraDataNull(2) && !evt.isExtraDataNull(3))
    {
        out[0] = evt.getExtraDataFloat(2) * myDisplaySize[0];
        out[1] = evt.getExtraDataFloat(3) * myDisplaySize[1];
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
AppInstance* AppManager::getAppAt(Vector2i pos)
{
    // Find which app contains this pointer.
    foreach(AppInstance* ai, myZSortedAppInstances)
    {
        Rect& cc = ai->currentCanvas;
        if(pos[0] >= cc.min[0] && pos[0] < cc.max[0] && 
            pos[1] >= cc.min[1] && pos[1] <= cc.max[1])
        {
            return ai;
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::handleEvent(const Event& evt)
{
    if(evt.getServiceType() == Service::Pointer || 
        evt.getServiceType() == Service::Wand)
    {
        InputInfo* ii = getOrCreateInputInfo(evt);
        
        if(evt.isButtonDown(myModeSwitchButton))
        {
            ii->controlMode = true;
        }
        else if(evt.isButtonUp(myModeSwitchButton))
        {
            ii->controlMode = false;
            ii->lockedMode = false;
            myServer->broadcastEvent(evt, myServerConnection);
        }
        
        if(ii->controlMode)
        {
            Vector2i pos;
            get2DPointer(evt, pos);
            
            if(evt.isButtonDown(myMoveButton) || evt.isButtonDown(myResizeButton))
            {
                // Go in locked-on mode: find which application we are pointing at.
                ii->lockedMode = true;
                AppInstance* ai = getAppAt(pos);
                if(ai != NULL)
                {
                    ii->target = ai;
                }
                else if(myLauncherApp != NULL)
                {
                    // We are in control locked down mode but we are not
                    // pointing at any app. Forward events to the registered 
                    // launcher application.
                    ii->target = myLauncherApp;
                }
            }
            else if(evt.isButtonUp(myMoveButton) || evt.isButtonUp(myResizeButton))
            {
                ii->lockedMode = false;
                myServer->broadcastEvent(evt, myServerConnection);
            }
            
            // If we are not in locked mode (moving or resizing an app), find
            // out which app we are pointing at and send event to that one. If
            // we are not pointing at any app, send events to the registered app 
            // launcher.
            AppInstance* ai = ii->target;
            if(!ii->lockedMode || ai == NULL)
            {
                ai = getAppAt(pos);
                ii->target = ai;
            }
            if(ai == NULL && myLauncherApp != NULL) ai = myLauncherApp;
            
            // Send pointer event to identfied target app
            if(ai != NULL)
            {
                Rect& cc = ai->currentCanvas;
                pos[0] -= cc.min[0];
                pos[1] -= cc.min[1];
                
                // Convert the event to a pointer event with the set position.
                Event& mutableEvent = const_cast<Event&>(evt);
                mutableEvent.setServiceType(Service::Pointer);
                mutableEvent.setPosition(pos[0], pos[1], 0);
                myServer->sendEventTo(evt, ai->connection);
            }
        }
        else
        {
            // We have a wand or pointer event and we are not in control mode:
            // send the event to the application registered to this event user
            if(ii->target != NULL)
            {
                //ofmsg("send %1% to %2%", %evt.getPosition() %ii->target->id);
                myServer->sendEventTo(evt, ii->target->connection);
            }
        }
    }
    else
    {
        myServer->broadcastEvent(evt, myServerConnection);
    }
}

///////////////////////////////////////////////////////////////////////////////
bool AppManager::handleCommand(const String& cmd)
{
    // :q exits app.
    if(cmd == "q") mySys->postExitRequest();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // This app name will make app use /mvi/appmgr/appmgr.cfg as the default
    // config file.
    Application<AppManager> app("mvi/appmgr/appmgr");
    return omain(app, argc, argv);
}
