#ifndef __APPMGR_H__
#define __APPMGR_H__
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
    int32_t slot;
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
    void onAppCanvasChange(const String& appid, int x, int y, int w, int h);
    void setLauncherApp(const String& appid);

    // Run a script using orun
    String run(const String& script);

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
    
    bool processControlMode(const Event& evt, InputInfo* ii);
    void processLockedMode(const Event& evt, const Vector2i& pos, InputInfo* ii);
    void sendPointerEvent(const Event& evt, Vector2i& pos, AppInstance* ai);

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
    bool myTileAllocatorEnabled;
};
#endif