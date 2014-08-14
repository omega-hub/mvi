#include "AppInfo.h"
#include "AppLauncher.h"
#include "WorkspaceLibrary.h"
#include "AppManager.h"
#include "omegaToolkit/UiScriptCommand.h"
#include "omega/MissionControl.h"

///////////////////////////////////////////////////////////////////////////////
void AppInfo::initialize(AppLauncher* drawer)
{
    myDrawer = drawer;

    command = "%orun %env -D - -s %file -N %appname -I %appid --mc @%mchost:%mcport";

    myUi = myDrawer->getUi();
    WidgetFactory* wf = myUi->getWidgetFactory();

    myContainer = wf->createContainer(label, drawer->getContainer(), Container::LayoutVertical);

    myContainer->setActiveStyle("fill: #505080");
    myContainer->setInactiveStyle("fill: #000000");

    //myContainer->setUserData(this);
    myContainer->setUIEventHandler(this);

    myImage = wf->createImage(label + "Icon", myContainer);
    myImage->setData(ImageUtils::loadImage(iconFile));
    myImage->setSize(Vector2f(myDrawer->getIconSize(), myDrawer->getIconSize()));

    myLabel = wf->createLabel(label + "Label", myContainer, label);
}

///////////////////////////////////////////////////////////////////////////////
void AppInfo::addInstance(const String& appid)
{
    InstanceInfo* ii = new InstanceInfo(appid, this);
    myInstances[appid] = ii;
}

///////////////////////////////////////////////////////////////////////////////
void AppInfo::removeInstance(const String& appid)
{
    Ref<InstanceInfo> ii = myInstances[appid];
    myContainer->removeChild(ii->uiContainer);
    myInstances[appid] = NULL;
}

///////////////////////////////////////////////////////////////////////////////
AppInfo::InstanceInfo::InstanceInfo(const String& appid, AppInfo* parent)
{
    id = appid;
    uiContainer = Container::create(Container::LayoutHorizontal, parent->myContainer);
    uiContainer->setStyleValue("fill", "#000000");
    uiLabel = Label::create(uiContainer);
    uiLabel->setText(appid);

    Vector2f iconSize(16, 16);

    uiCloseButton = Button::create(uiContainer);
    uiCloseButton->setIcon(ImageUtils::loadImage("mvi/icons/close.png"));
    uiCloseButton->getImage()->setSize(iconSize);
    //uiCloseButton->setUIEventHandler(this);
    uiCloseButton->setUIEventCommand(ostr(
        "getMissionControlClient().postCommand('@%1%:oexit()')", %appid));
    uiCloseButton->setTextEnabled(false);
    uiCloseButton->setAutosize(false);
    uiCloseButton->setSize(iconSize);

    uiRestoreButton = Button::create(uiContainer);
    uiRestoreButton->setIcon(ImageUtils::loadImage("mvi/icons/empty.png"));
    uiRestoreButton->getImage()->setSize(iconSize);
    uiRestoreButton->setTextEnabled(false);
    uiRestoreButton->setAutosize(false);
    uiRestoreButton->setSize(iconSize);
    uiRestoreButton->setUIEventHandler(this);
}

///////////////////////////////////////////////////////////////////////////////
void AppInfo::InstanceInfo::handleEvent(const Event& evt)
{
    if(uiRestoreButton->isButtonDown(evt, UiModule::getClickButton()))
    {
        AppManager* wa = AppManager::instance();
        // Get a new workspace for this application.
        const Vector3f& pos = evt.getPosition();
        //Workspace* w = wa->findFreeWorkspace(pos[0], pos[1]);
        //if(w != NULL)
        //{
        //    wa->requestWorkspace(id, w->getLayout()->getName(), w->getName());
        //}
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppInfo::handleEvent(const Event& evt)
{
    if(myContainer->isButtonDown(evt, UiModule::getClickButton()))
    {
        omsg("Container clicked");
        myDrawer->run(this);
    }
}
