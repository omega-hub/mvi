#ifndef __APP_INFO__
#define __APP_INFO__

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

class AppLauncher;

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
struct AppInfo: public ReferenceType, public IEventListener
{
    friend class AppLauncher;
	String command;
    String file;
    String label;
    String id;
	String iconFile;

	void initialize(AppLauncher* drawer);
    void addInstance(const String& appid);
    void removeInstance(const String& appid);
    void handleEvent(const Event& evt);

private:
	AppLauncher* myDrawer;

	// ui stuff.
	Ref<UiModule> myUi;
	Ref<ui::Container> myContainer;
	Ref<ui::Label> myLabel;
	Ref<ui::Image> myImage;
	Ref<UiScriptCommand> myCommand;

    class InstanceInfo : public ReferenceType, public IEventListener
    {
    public:
        InstanceInfo(const String& appid, AppInfo* parent);
        void handleEvent(const Event& evt);
        String id;
        Ref<ui::Container> uiContainer;
        Ref<ui::Label> uiLabel;
        Ref<ui::Button> uiCloseButton;
        Ref<ui::Button> uiRestoreButton;
    };

    typedef Dictionary<String, Ref<InstanceInfo> > InstanceDictionary;
    InstanceDictionary myInstances;
};
#endif