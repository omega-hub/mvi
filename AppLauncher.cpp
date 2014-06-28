#include "AppLauncher.h"
#include "WorkspaceAllocator.h"
#include "omegaToolkit/UiScriptCommand.h"
#include "omega/MissionControl.h"

AppLauncher* sADInstance = NULL;

///////////////////////////////////////////////////////////////////////////////
AppLauncher* AppLauncher::instance()
{
    return sADInstance;
}

///////////////////////////////////////////////////////////////////////////////
AppLauncher* AppLauncher::create()
{
    PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();
    UiModule* uim = UiModule::createAndInitialize();

    WorkspaceAllocator* wa = new WorkspaceAllocator(WorkspaceManager::instance());

    AppLauncher* ad = new AppLauncher(pi, uim, WorkspaceManager::instance(), wa);
    ModuleServices::addModule(ad);
    ad->doInitialize(Engine::instance());

    sADInstance = ad;

    return ad;
}

///////////////////////////////////////////////////////////////////////////////
AppLauncher::AppLauncher(PythonInterpreter* interp, UiModule* ui, WorkspaceManager* wm, WorkspaceAllocator* a) :
myUi(ui), myInterpreter(interp), myListener(NULL), myWorkspaceManager(wm), myWorkspaceAllocator(a)
{
    myMissionControlClient = SystemManager::instance()->getMissionControlClient();
    oassert(myMissionControlClient);

    myMissionControlClient->setClientConnectedCommand("AppLauncher.instance().onAppConnected('%clientId%')");
    myMissionControlClient->setClientDisconnectedCommand("AppLauncher.instance().onAppDisconnected('%clientId%')");
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::initialize()
{
	myDrawerScale = 1.0f;
	myIconSize = 16;
	myContainer = myUi->getWidgetFactory()->createContainer("AppLauncher", myUi->getUi(), Container::LayoutHorizontal);
    myContainer->setLayer(Widget::Front);
	hide();
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::onAppConnected(const String& client)
{
    if(client != "server")
    {
        myMissionControlClient->postCommand(ostr(
            "@%1%:_aco = AppControlOverlay.create()", %client));
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::onAppDisconnected(const String& client)
{
    removeAppInstance(client);
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::addApp(AppInfo* info)
{
	info->initialize(this);
	myAppList.push_back(info);
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::update(const UpdateContext& context)
{
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::handleEvent(const Event& evt)
{
    if(evt.getServiceType() == Service::Pointer &&
        evt.isButtonDown(Event::Middle))
    {
        show();
        const Vector3f& pos = evt.getPosition();

        // Convert pointer position to UI root reference frame.
        const Vector2f& pt = myUi->getUi()->transformPoint(Vector2f(pos[0], pos[1]));
        myContainer->setPosition(pt);
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::run(AppInfo* app)
{
    SystemManager* sys = getEngine()->getSystemManager();

    int mcport = sys->getMissionControlServer()->getPort();

    String orunPath = ogetexecpath();

    String cmd = app->command;
    cmd = StringUtils::replaceAll(cmd, "%orun", orunPath);
    cmd = StringUtils::replaceAll(cmd, "%file", app->file);
    cmd = StringUtils::replaceAll(cmd, "%cfg", sys->getSystemConfig()->getFilename());
    cmd = StringUtils::replaceAll(cmd, "%mchost", "127.0.0.1");
    cmd = StringUtils::replaceAll(cmd, "%mcport", boost::lexical_cast<String>(mcport));
    cmd = StringUtils::replaceAll(cmd, "%workspace", "split left");

    int appid = myWorkspaceAllocator->allocateAppId();
    String appname = ostr("%1%-%2%", %app->id %appid);

    cmd = StringUtils::replaceAll(cmd, "%appname", appname);
    cmd = StringUtils::replaceAll(cmd, "%appid", boost::lexical_cast<String>(appid));

    app->addInstance(appname);

    olaunch(cmd);
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::show()
{
	myVisible = true;
	myContainer->setEnabled(true);
    myContainer->setVisible(true);
    UiModule::instance()->activateWidget(myContainer);
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::hide()
{
	//omsg("Menu hide");

	myVisible = false;
	myContainer->setEnabled(false);
    myContainer->setVisible(false);
    //myContainer->setDebugModeEnabled(true);

	UiModule::instance()->activateWidget(NULL);
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::removeAppInstance(const String& clientid)
{
    // Get app name
    Vector<String> args = StringUtils::split(clientid, "-");
    String appname = args[0];

    // Find AppInfo
    foreach(AppInfo* ai, myAppList)
    {
        if(ai->id == appname)
        {
            ai->removeInstance(clientid);
            break;
        }
    }

    // Free workspace used by this app
    myWorkspaceAllocator->freeWorkspace(clientid);
}
