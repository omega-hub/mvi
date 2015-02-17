#include "WorkspaceLibrary.h"
#include "AppController.h"

using namespace omega;

bool mviInit();

///////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#ifdef OMEGA_USE_PYTHON
#include "omega/PythonInterpreterWrapper.h"

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(AppController_setShortcut, setShortcut, 2, 3)
BOOST_PYTHON_MODULE(mvi)
{
    PYAPI_REF_BASE_CLASS(WorkspaceLibrary)
        PYAPI_STATIC_REF_GETTER(WorkspaceLibrary, create)
        PYAPI_STATIC_REF_GETTER(WorkspaceLibrary, instance)
        PYAPI_REF_GETTER(WorkspaceLibrary, createLayout)
        PYAPI_REF_GETTER(WorkspaceLibrary, findLayout)
        ;

    PYAPI_REF_BASE_CLASS(Workspace)
        PYAPI_METHOD(Workspace, setTiles)
        PYAPI_PROPERTY(Workspace, onActivated)
        ;

    Workspace* (WorkspaceLayout::*createWorkspace1)(const String&, const String&, const String&) = &WorkspaceLayout::createWorkspace;
    Workspace* (WorkspaceLayout::*createWorkspace2)(const String&, const String&, float, float, float, float) = &WorkspaceLayout::createWorkspace;
    PYAPI_REF_BASE_CLASS(WorkspaceLayout)
        .def("createWorkspace", createWorkspace1, PYAPI_RETURN_REF)
        .def("createWorkspace", createWorkspace2, PYAPI_RETURN_REF)
        PYAPI_REF_GETTER(WorkspaceLayout, findWorkspace)
        ;

    // App drawer
    PYAPI_REF_BASE_CLASS(AppController)
        PYAPI_STATIC_REF_GETTER(AppController, create)
        PYAPI_STATIC_METHOD(AppController, configPhysicalButtons)
        PYAPI_STATIC_METHOD(AppController, setActiveUser)
        PYAPI_METHOD(AppController, setButton)
        .def("setShortcut", &AppController::setShortcut, AppController_setShortcut())
        ;

    def("mviInit", mviInit);
}
#endif

///////////////////////////////////////////////////////////////////////////////
// This is the message handler used to receive the application slot from
// the app manager.
class MviMessageHandler : public IMissionControlMessageHandler
{
public:
    virtual bool handleMessage(
        MissionControlConnection* sender,
        const char* header, char* data, int size)
    {
        if(!strncmp(header, "slot", 4))
        {
            MultiInstanceConfig& mic = SystemManager::instance()->getMultiInstanceConfig();
            mic.id = (int32_t)*data;
            mic.enabled = true;
            // Setting all the tile entries to -1 will use the full tile
            // set specified in the system configuration. We are using this
            // here because we are aonly interested in setting the application
            // instance id, not an initial tile set.
            mic.tilex = -1;
            mic.tiley = -1;
            mic.tilew = -1;
            mic.tileh = -1;

            // Reset the app name to include the slot.
            ApplicationBase* app = SystemManager::instance()->getApplication();
            String appname = app->getName();
            app->setName(ostr("%1%-%2%", %appname %mic.id));

            ofmsg("MVI: Application slot allocated: %1%", %mic.id);
        }
        return true;
    }
};

///////////////////////////////////////////////////////////////////////////////
bool mviInit()
{
    // If we have already been allocated a multi-instance id, nothing needs to
    // be done here.
    MultiInstanceConfig& mic = SystemManager::instance()->getMultiInstanceConfig();
    if(mic.enabled || !SystemManager::instance()->isMaster()) return true;

    // Multi-instance not initialized yet. That means we are an external application
    // connecting to app manager (i.e. we have not been launched from inside 
    // app manager using :run).
    // Connect to the appmanager through a mission control connection and tell
    // it to allocate an app instance for us. Get app instance data through
    // 
    asio::io_service ioService;
    MviMessageHandler mviMessageHandler;

    Ref<MissionControlConnection> conn = new MissionControlConnection(
        ConnectionInfo(ioService), 
        &mviMessageHandler, 
        NULL);

    conn->open("127.0.0.1", MissionControlServer::DefaultPort);
    if(conn->getState() == TcpConnection::ConnectionOpen)
    {
        ApplicationBase* app = SystemManager::instance()->getApplication();

        // Send the app name to appmgr. This will prompt appmgr to allocate a 
        // new application instance / slot to this app (see AppManager::onClientConnected) 
        // The slot is multi-instance id we should use for this app. The slot
        // is sent back in a mission control message with header 'slot'.
        conn->sendMessage(MissionControlMessageIds::MyNameIs, 
            (void*)app->getName(), 
            strlen(app->getName()));

        // Wait for 2 seconds max.
        int timeout = 2000;

        owarn("mviInit: Waiting for application slot.");

        while(!mic.enabled && timeout > 0)
        {
            conn->poll();
            osleep(100);
            timeout -= 100;
        }
        conn->goodbyeServer();

        if(!mic.enabled)
        {
            owarn("mviInit: no response from appmanager. MVI disabled.");
        }
    }
    return mic.enabled;
}

