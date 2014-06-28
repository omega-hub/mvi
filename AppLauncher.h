#ifndef __APP_DRAWER__
#define __APP_DRAWER__

#include "omega/Engine.h"
#include "omega/Application.h"
#include "omega/PythonInterpreter.h"
#include "omegaToolkit/UiModule.h"
#include "omegaToolkit/ui/Container.h"
#include "omegaToolkit/ui/Button.h"
#include "omegaToolkit/ui/WidgetFactory.h"
#include "omegaToolkit/ui/Widget.h"
#include "omegaToolkit/UiScriptCommand.h"
#include "WorkspaceManager.h"
#include "AppInfo.h"
#include "WorkspaceAllocator.h"

class AppLauncher;

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
class IAppLauncherListener
{
public:
	virtual void startApp(AppInfo* app) = 0;
};

///////////////////////////////////////////////////////////////////////////////
class AppLauncher : public EngineModule
{
public:
    static AppLauncher* create();
    static AppLauncher* instance();

public:
	AppLauncher(PythonInterpreter* interp, UiModule* ui, WorkspaceManager* wm, WorkspaceAllocator* a);
	virtual ~AppLauncher() {}

	void initialize();
	ui::Container* getContainer() { return myContainer; }
	UiModule* getUi() { return myUi; }
	void update(const UpdateContext& context);
	void handleEvent(const Event& evt);
	void show();
	void hide();
	bool isVisible() { return myVisible; }
	void addApp(AppInfo* app);
	float getDrawerScale() { return myDrawerScale; }
	void setDrawerScale(float value) { myDrawerScale = value; }
	int getIconSize() { return myIconSize; }
	void setIconSize(int value) { myIconSize = value; }

	void setListener(IAppLauncherListener* listener) { myListener = listener; }
	IAppLauncherListener* getListener() { return myListener; }

    void removeAppInstance(const String& clientid);
    void run(AppInfo* app);

    void onAppConnected(const String& client);
    void onAppDisconnected(const String& client);

private:
	Ref<UiModule> myUi;
	List < Ref<AppInfo> > myAppList;
	Ref<ui::Container> myContainer;
    Ref<MissionControlClient> myMissionControlClient;
	PythonInterpreter* myInterpreter;
	bool myVisible;
	float myDrawerScale;
	int myIconSize;

	IAppLauncherListener* myListener;
    Ref<WorkspaceManager> myWorkspaceManager;
    Ref<WorkspaceAllocator> myWorkspaceAllocator;
};
#endif