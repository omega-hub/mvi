#include <omega.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
// Stored data about a single application instance.
struct AppInstance : public ReferenceType
{
    String id;
    Rect targetCanvas;
    Rect currentCanvas;
    int z;
    bool dirtyCanvas;
    Ref<MissionControlConnection> connection;
};

///////////////////////////////////////////////////////////////////////////////
// Some useful typedefs.
typedef Dictionary<String, Ref<AppInstance> > AppInstanceDictionary;
typedef Dictionary<uint, Ref<MissionControlConnection> > EventRoutingTable;

///////////////////////////////////////////////////////////////////////////////
class ApplicationManager : public EngineModule, public IMissionControlListener
{
public:
    static ApplicationManager* instance();

    ApplicationManager();
    void initialize();
    void handleEvent(const Event& evt);

    // From IMissionControlListener
    void onClientConnected(const String& clientId);
    void onClientDisconnected(const String& clientId);
    bool handleCommand(const String& cmd);

    // Called by connected clients
    void setDisplaySize(const String& clientid, int width, int height);

private:
    static ApplicationManager* mysInstance;

    SystemManager* mySys;

    Ref<MissionControlServer> myServer;
    Ref<MissionControlConnection> myServerConnection;

    EventRoutingTable myEventRoutingTable;

    Event::Flags myAquireInputButton;
    Event::Flags myExclusiveInputButton;

    // Pixel size of the connected display
    Vector2i myDisplaySize;

    // App instance data
    AppInstanceDictionary myAppInstances;

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
    PYAPI_REF_BASE_CLASS(ApplicationManager)
        PYAPI_STATIC_REF_GETTER(ApplicationManager, instance)
        PYAPI_METHOD(ApplicationManager, setDisplaySize)
        ;
}


///////////////////////////////////////////////////////////////////////////////
ApplicationManager* ApplicationManager::mysInstance = NULL;
ApplicationManager* ApplicationManager::instance()
{ 
    return mysInstance; 
}

///////////////////////////////////////////////////////////////////////////////
ApplicationManager::ApplicationManager(): 
    EngineModule("ApplicationManager"),
    mySys(SystemManager::instance()),
    myAquireInputButton(Event::Button1)
{
    mysInstance = this;
}

///////////////////////////////////////////////////////////////////////////////
void ApplicationManager::initialize()
{
    // Read in configuration
    Config* cfg = SystemManager::instance()->getAppConfig();
    if(cfg->exists("config/appmgr"))
    {
        Setting& s = cfg->lookup("config/appmgr");
        String aquireInputButtonName = Config::getStringValue("aquireInputButton", s, "Button1");
        myAquireInputButton = Event::parseButtonName(aquireInputButtonName);
        String exclusiveInputButtonName = Config::getStringValue("exclusiveInputButton", s, "Button2");
        myExclusiveInputButton = Event::parseButtonName(exclusiveInputButtonName);
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
    // ApplicationManager instance
    initappmgr();
    interpreter->eval("from appmgr import *");
    interpreter->eval("appmgr = ApplicationManager.instance()");


    omsg("Application Manager initialization complete!");
}

///////////////////////////////////////////////////////////////////////////////
void ApplicationManager::onClientConnected(const String& clientId)
{
    // Don't create app controller here - layout script takes care of that.
}

///////////////////////////////////////////////////////////////////////////////
void ApplicationManager::onClientDisconnected(const String& clientId)
{

}

///////////////////////////////////////////////////////////////////////////////
void ApplicationManager::setDisplaySize(const String& clientid, int width, int height)
{
    myDisplaySize = Vector2i(width, height);
    ofmsg("Display size set to %1%", %myDisplaySize);
}

///////////////////////////////////////////////////////////////////////////////
void ApplicationManager::handleEvent(const Event& evt)
{
    // If event has aquire button pressed:
    // - get event user id. If there is an entry in routing table, send event
    // to that app.
    // ELSE
    // if event is a pointer or wand event
    // - obtain 2D coords
    // get application pointed at (use new getAppAt(Vector2i) method, also
    // add AppInstance/AppInstanceDictionary code from AppManager (add MissionControlConnection to AppInstance)
    // add Application to event routing table with userId of event.
    // if event is not a pointer or wand
    // just look in routing table and send event to relative app.
    // if there are special apps (ie. apps that receive all events) send events
    // to those apps as well.
    // Simple version: forward all events to all clients.
    myServer->broadcastEvent(evt, myServerConnection);
    ofmsg("Sending%1%", %evt.getPosition());
}

///////////////////////////////////////////////////////////////////////////////
bool ApplicationManager::handleCommand(const String& cmd)
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
    Application<ApplicationManager> app("mvi/appmgr/appmgr");
    return omain(app, argc, argv);
}
