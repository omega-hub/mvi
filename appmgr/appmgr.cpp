#include <omega.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
// Stored data about a single application instance.
struct AppInstance : public ReferenceType
{
    AppInstance():
        currentCanvas(0,0,0,0),
        z(0),
        slot(0),
        dirtyCanvas(false) {}
        
    String id;
    // slot is the allocation number of this app instance. it is used by the omegalib
    // runtime to determine poer allocation for the application (using the -I 
    // command line argument)
    int slot;
    Rect currentCanvas;
    int z;
    bool dirtyCanvas;
    Ref<MissionControlConnection> connection;

    // Active tiles for this app
    List< DisplayTileConfig* > activeTiles;
};

///////////////////////////////////////////////////////////////////////////////
// Stores per-tile information used to allocate tile rendering
struct TileAllocation: public ReferenceType
{
    DisplayTileConfig* tile;
    struct LocalAppRect: public ReferenceType
    {
        AppInstance* app;
        Rect localRect;
    };
    List< Ref<LocalAppRect> > apps;
};

///////////////////////////////////////////////////////////////////////////////
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
    void update(const UpdateContext& context);

    // From IMissionControlListener
    void onClientConnected(const String& clientId);
    void onClientDisconnected(const String& clientId);
    bool handleCommand(const String& cmd);
    
    // Called by connected clients
    void setDisplaySize(const String& clientid, int width, int height);
    void onAppCanvasChange(const String& appid, int x, int y, int w, int h);
    void setLauncherApp(const String& appid);

    // Run a script using orun
    void run(const String& script);

private:
    void loadDisplayConfig(String configFileName);

    // Get a 2D pointer out of a pointer or wand event
    bool get2DPointer(const Event& evt, Vector2i& out);
    InputInfo* getOrCreateInputInfo(const Event& evt);
    AppInstance* getAppAt(Vector2i pos);
    
    AppInstance* allocAppInstance(String appName);
    void releaseAppInstance(AppInstance* ai);

    void updateTileAllocation();
    void sendActiveTileUpdates();

private:
    static AppManager* mysInstance;

    SystemManager* mySys;

    // Name of the app configuration file to use with applications launched
    // with :run
    String myAppConfig;

    Ref<MissionControlServer> myServer;
    Ref<MissionControlConnection> myServerConnection;

    EventRoutingTable myEventRoutingTable;

    Event::Flags myModeSwitchButton;
    Event::Flags myMoveButton;
    Event::Flags myResizeButton;
    InputInfoTable myInputInfoTable;
    float myTileUpdateInterval;
    float myLastTileUpdate;

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

    // Maximum number of instances that can run simultaneously. Each app needs a 
    // separate port allocated when running on a cluster, so this number is limited.
    static const int MaxAppInstances = 256;
    AppInstance* myAppInstanceIds[MaxAppInstances];

    // Display config
    DisplayConfig myDisplayConfig;
    List< Ref<TileAllocation> > myTileAllocation;
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
    myResizeButton(Event::Button2),
    myCurrentTopZ(1),
    myTileUpdateInterval(1),
    myLastTileUpdate(0)
{
    mysInstance = this;
    memset(myAppInstanceIds, 0, MaxAppInstances * sizeof(AppInstance*));
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::initialize()
{
    String displayConfigFile = "DEFAULT";

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

        myAppConfig = Config::getStringValue("appConfig", s, "mvi/mvi.cfg");
        displayConfigFile = Config::getStringValue("displayConfig", s, "DEFAULT");

        // By default recompute tile allocation every second.
        myTileUpdateInterval = Config::getFloatValue("tileUpdateInterval", s, 1.0);
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
    //interpreter->eval("import porthole");
    //interpreter->eval("porthole.initialize('mvi/appmgr/pointer.xml')");

    // Initialize python API and create a variable 'appmgr' storing the
    // AppManager instance
    interpreter->lockInterpreter();
    initappmgr();
    interpreter->eval("from appmgr import *");
    interpreter->eval("appmgr = AppManager.instance()");
    interpreter->unlockInterpreter();

    // Load the display configuration
    loadDisplayConfig(displayConfigFile);

    omsg("Application Manager initialization complete!");
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::loadDisplayConfig(String displayConfigFile)
{
    if(displayConfigFile == "DEFAULT")
    {
        Config* defaultCfg = new Config("default.cfg");
        if(defaultCfg->load())
        {
            displayConfigFile = (const char*)defaultCfg->lookup("config/systemConfig");
            ofmsg("Default system configuration file: %1%", %displayConfigFile);
        }
        else
        {
            oerror("SystemManager::setup: FATAL - coult not load default.cfg");
        }
    }

    Ref<Config> displayConfig = new Config(displayConfigFile);
    bool loaded = displayConfig->load();

    if(!loaded)
    {
        oferror("loading %1% failed", %displayConfigFile);
    }
    else
    {
        if(displayConfig->exists("config/display"))
        {
            Setting& s = displayConfig->lookup("config/display");
            DisplayConfig::LoadConfig(s, myDisplayConfig);
        }
        else
        {
            oferror("Config file %1% is missing a display configuration section", %displayConfigFile);
        }

        // Initialize the objects needed for tile allocation.
        foreach(DisplayConfig::Tile tile, myDisplayConfig.tiles)
        {
            TileAllocation* ta = new TileAllocation();
            ta->tile = tile.second;
            myTileAllocation.push_back(ta);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::onClientConnected(const String& clientId)
{
    ofmsg("Application connected: %1%", %clientId);
    MissionControlConnection* conn = myServer->findConnection(clientId);

    // Setup the connected app controller: configure buttons
    String cmd = ostr("AppController.configPhysicalButtons(%1%, %2%, %3%)",
        %myModeSwitchButton %myMoveButton %myResizeButton);
    conn->sendMessage(MissionControlMessageIds::ScriptCommand, (void*)cmd.c_str(), cmd.size());

    if(myAppInstances.find(clientId) == myAppInstances.end())
    {
        oferror("AppManager::onClientConnected: could not find app instance %1%", %clientId);
    }
    else
    {
        AppInstance* ai = myAppInstances[clientId];
        ai->connection = conn;
        myZSortedAppInstances.push_front(ai);
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::onClientDisconnected(const String& clientId)
{
    ofmsg("Application disconnected: %1%", %clientId);
    
    AppInstance* ai = myAppInstances[clientId];
    releaseAppInstance(ai);

    myZSortedAppInstances.remove(ai);
    myAppInstances.erase(clientId);
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
    
    ai->currentCanvas = Rect(x, y, w, h);
    ai->z = myCurrentTopZ++;

    // Place app on top of z sorted app instance list.
    myZSortedAppInstances.remove(ai);
    myZSortedAppInstances.push_front(ai);
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
            // Disable the focus border around the currently focused application
            if(ii->target != NULL)
            {
                String cmd = "AppController.setFocus(False)";
                ii->target->connection->sendMessage(
                    MissionControlMessageIds::ScriptCommand, 
                    (void*)cmd.c_str(), cmd.size());
            }
        }
        else if(evt.isButtonUp(myModeSwitchButton))
        {
            ii->controlMode = false;
            ii->lockedMode = false;
            // Enable the focus border for the focused application
            if(ii->target != NULL)
            {
                String cmd = "AppController.setFocus(True)";
                ii->target->connection->sendMessage(
                    MissionControlMessageIds::ScriptCommand, 
                    (void*)cmd.c_str(), cmd.size());
                    
                // Also place the application in front of the Z sorted instance list
                myZSortedAppInstances.remove(ii->target);
                myZSortedAppInstances.push_front(ii->target);
            }
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
                
                // Convert to a pointer event before broadcasting: AppController
                // expect pointer events for windows size/move.
                // Convert the event to a pointer event with the set position.
                Event& mutableEvent = const_cast<Event&>(evt);
                mutableEvent.setServiceType(Service::Pointer);
                myServer->broadcastEvent(mutableEvent, myServerConnection);
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
void AppManager::update(const UpdateContext& context)
{
    if(context.time - myLastTileUpdate > myTileUpdateInterval)
    {
        myLastTileUpdate = context.time;
        updateTileAllocation();
    }
}

///////////////////////////////////////////////////////////////////////////////
bool AppManager::handleCommand(const String& cmd)
{
    // :q exits app.
    if(cmd == "q") mySys->postExitRequest();
    // :run runs a script through orun
    else if(StringUtils::startsWith(cmd, "run"))
    {
        String scriptName = cmd.substr(3);
        StringUtils::trim(scriptName);
        run(scriptName);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::run(const String& script)
{
    // get an app name based on the script
    String appname;
    String path;
    String ext;
    StringUtils::splitFullFilename(script, appname, ext, path);

    // Allocate an app instance.
    AppInstance* ai = allocAppInstance(appname);
    if(ai == NULL)
    {
        ofwarn("AppManager::run: failed to allocate new app instance for %1%", %script);
    }
    else
    {
        String cmd = ostr("orun %1% -s %2% -N %3% -I %4% --mc @localhost",
            %myAppConfig
            %script
            %ai->id
            %ai->slot);

        myAppInstances[ai->id] = ai;
        ofmsg("Running %1%: %2%", %ai->id %script);

        olaunch(cmd);
    }
}

///////////////////////////////////////////////////////////////////////////////
AppInstance* AppManager::allocAppInstance(String appName)
{
    for(int i = 0; i < MaxAppInstances; i++)
    {
        if(myAppInstanceIds[i] == NULL)
        {
            AppInstance* ai = new AppInstance();
            ai->slot = i;
            ai->id = ostr("%1%-%2%", %appName %ai->slot);
            myAppInstanceIds[i] = ai;
            return ai;
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::releaseAppInstance(AppInstance* ai)
{
    for(int i = 0; i < MaxAppInstances; i++)
    {
        if(myAppInstanceIds[i] == ai)
        {
            myAppInstanceIds[i] = NULL;
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::updateTileAllocation()
{
    // Clean app rects in current tile allocation
    foreach(TileAllocation* ta, myTileAllocation)
    {
        ta->apps.clear();
    }

    // Clean enabled tiles for each app
    foreach(AppInstanceDictionary::Item ai, myAppInstances)
    {
        ai->activeTiles.clear();
    }

    // Loop over applications front-to-back
    foreach(AppInstance* ai, myZSortedAppInstances)
    {
        foreach(TileAllocation* ta, myTileAllocation)
        {
            // Compute the interection between the tile and application canvas.
            Rect tileRect(ta->tile->offset, ta->tile->offset + ta->tile->pixelSize);
            pair<bool, Rect> localIntersection = ai->currentCanvas.getIntersection(tileRect);

            if(localIntersection.first)
            {
                List< Ref<TileAllocation::LocalAppRect> > localAppsToRemove;
                // Loop over the other applications that share this canvas.
                // NOTE: since we are looping front-to-back, these apps will be
                // in front of the current one.
                // Remove this app rect from their local rect. If an app rect is
                // completely covered by other apps, remove it from the list
                // of applications active for this tile.
                foreach(TileAllocation::LocalAppRect* lar, ta->apps)
                {
                    pair<bool, Rect> newLocalRect;
                    lar->localRect.subtract(localIntersection.second);

                    if(newLocalRect.first)
                    {
                        if(newLocalRect.second.size() == Vector2i::Zero())
                        {
                            localAppsToRemove.push_back(lar);
                        }
                        lar->localRect = newLocalRect.second;
                    }
                }

                // Remove apps with empty local rects from the apps enabled
                // for this tile.
                foreach(TileAllocation::LocalAppRect* lar, localAppsToRemove)
                {
                    ta->apps.remove(lar);
                }

                // Add the current app to the list of apps enabled for this tile.
                TileAllocation::LocalAppRect* lar = new TileAllocation::LocalAppRect();
                lar->app = ai;
                lar->localRect = localIntersection.second;
                ta->apps.push_back(lar);
            }
        }
    }

    // Now we have an updated list of display tiles, each with a list of apps
    // that should render on those tiles. Save the list of enabled tiles with
    // each app instance.
    foreach(TileAllocation* ta, myTileAllocation)
    {
        foreach(TileAllocation::LocalAppRect* lar, ta->apps)
        {
            lar->app->activeTiles.push_back(ta->tile);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::sendActiveTileUpdates()
{
    foreach(AppInstanceDictionary::Item ai, myAppInstances)
    {
        String tileNames = "";
        foreach(DisplayTileConfig* dtc, ai->activeTiles)
        {
            tileNames.append(dtc->name);
            tileNames.append(" ");
        }
        ofmsg("APP %1% TILES %2%", %ai->id %tileNames);
    }
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // This app name will make app use /mvi/appmgr/appmgr.cfg as the default
    // config file.
    Application<AppManager> app("mvi/appmgr/appmgr");
    return omain(app, argc, argv);
}
