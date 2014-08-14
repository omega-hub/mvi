#include <omega/MouseService.h>

#include "AppController.h"
#include "WorkspaceLibrary.h"

///////////////////////////////////////////////////////////////////////////////
AppController* AppController::create()
{
    PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();

    AppController* ad = new AppController(pi);
    ModuleServices::addModule(ad);
    return ad;
}

///////////////////////////////////////////////////////////////////////////////
AppController::AppController(PythonInterpreter* interp) :
myUi(NULL), myInterpreter(interp), myModifyingCanvas(false),
myActiveUserId(-1),
myMovingCanvas(false), mySizingCanvas(false), myPointerDelta(Vector2i::Zero()),
myModeSwitchButton(Event::Alt),
myMoveButton(Event::Button1),
myResizeButton(Event::Button2),
myUsingLocalPointer(false)
{
    setPriority(EngineModule::PriorityHighest);
}

///////////////////////////////////////////////////////////////////////////////
void AppController::initialize()
{
    // Read config options
    Config* cfg = SystemManager::instance()->getAppConfig();
    if(cfg->exists("config/appController"))
    {
        Setting& s = cfg->lookup("config/appController");
        String sModeSwitchButton = Config::getStringValue("modeSwitchButton", s, "");
        String sMoveButton = Config::getStringValue("moveButton", s, "");
        String sResizeButton = Config::getStringValue("resizeButton", s, "");
        if(sModeSwitchButton != "") myModeSwitchButton = Event::parseButtonName(sModeSwitchButton);
        if(sMoveButton != "") myMoveButton = Event::parseButtonName(sMoveButton);
        if(sResizeButton != "") myResizeButton = Event::parseButtonName(sResizeButton);
    }

    myUi = UiModule::createAndInitialize();

    //myDrawerScale = 1.0f;
    myIconSize = 24;
    int margin = 10;

    myBackground = Container::create(Container::LayoutFree, myUi->getUi());
    myBackground->setLayer(Widget::Front);
    myBackground->setFillColor(Color(0, 0, 0, 0.4f));
    myBackground->setFillEnabled(true);
    myBackground->setAutosize(false);

    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dc = ds->getDisplayConfig();
    myBackground->setSize(dc.getCanvasRect().size().cast<omicron::real>());

    myContainer = Container::create(Container::LayoutFree, myBackground);
    myContainer->setAutosize(false);
    myContainer->setSize(Vector2f(myIconSize * 6 + margin * 2, myIconSize * 6 + margin * 2));
    myContainer->setCenter(myBackground->getCenter());
    myContainer->setStyleValue("border", "1 white");
    myContainer->setFillColor(Color::Black);
    myContainer->setFillEnabled(true);

    Image* dpad = Image::create(myContainer);
    dpad->setData(ImageUtils::loadImage("mvi/icons/dpad.png"));
    dpad->setSize(Vector2f(myIconSize * 4, myIconSize * 4));
    dpad->setPosition(Vector2f(myIconSize + margin, myIconSize + margin));

    Vector2f dpadCenter = dpad->getPosition() + Vector2f(myIconSize * 1.5f, myIconSize * 1.5f) - Vector2f(4, 0);
    float dpadSize = (dpad->getSize() / 2).x() + myIconSize / 2.0f;

    foreach(Shortcut* s, myShortcuts)
    {
        if(s->icon != NULL)
        {
            Button* btn = Button::create(myContainer);
            btn->setIcon(s->icon);
            btn->getImage()->setSize(Vector2f(myIconSize, myIconSize));
            btn->setTextEnabled(false);

            if(s->button == Event::ButtonLeft) btn->setCenter(dpadCenter + Vector2f(-dpadSize, 0));
            else if(s->button == Event::ButtonRight) btn->setCenter(dpadCenter + Vector2f(dpadSize, 0));
            else if(s->button == Event::ButtonUp) btn->setCenter(dpadCenter + Vector2f(0, -dpadSize));
            else if(s->button == Event::ButtonDown) btn->setCenter(dpadCenter + Vector2f(0, dpadSize));
        }
    }

    hide();

    // Send our display size to the application manager.
    MissionControlClient* cli = SystemManager::instance()->getMissionControlClient();
    if(cli != NULL && cli->isConnected() && cli->getName() != "server")
    {
        cli->postCommand(ostr(
            "@server:"
            "appmgr.setDisplaySize('%1%', %2%, %3%)",
            %cli->getName()
            %dc.displayResolution[0] %dc.displayResolution[1]));
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppController::update(const UpdateContext& context)
{
    // Control canvas size/position if we are in canvas move/size mode.
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dc = ds->getDisplayConfig();
    Rect canvas = dc.getCanvasRect();

    if(myMovingCanvas)
    {
        canvas.min += myPointerDelta;
        canvas.max += myPointerDelta;
        setAppCanvas(canvas);
        
        // When set to true, pointer events controlling the window are happening on
        // the window itself. This flag is needed to apply pointer delta correction
        // after window is moved.
        if(myUsingLocalPointer)
        {
            myLastPointerPos -= myPointerDelta;
        }
    }
    else if(mySizingCanvas)
    {
        canvas.max += myPointerDelta;
        setAppCanvas(canvas);
        //ofmsg("Sizing: %1%", %myPointerDelta);
    }

    if(myModifyingCanvas)
    {
        // Resize/position the overlay.
        myBackground->setSize(canvas.size().cast<omicron::real>());
        myContainer->setCenter(myBackground->getCenter());
    }
    myPointerDelta = Vector2i::Zero();
}

///////////////////////////////////////////////////////////////////////////////
void AppController::handleEvent(const Event& evt)
{
    if(evt.isButtonDown(myModeSwitchButton))
    {
        // The mode switch button s also used to set the active user, so for
        // instance we can start tracking the head of the user whose controller
        // generated this event.
        myActiveUserId = evt.getUserId();
        myModifyingCanvas = true;
    }
    else if(evt.isButtonUp(myModeSwitchButton))
    {
        myModifyingCanvas = false;
    }

    // Head tracking management: if we get a mocap event whose id is the same
    // as the user id and it is marked as tracking a head, we want to use this
    // tracker as the head tracker for the application: change the event source
    // if to the source id that the main camera uses for head tracking.
    if(evt.getServiceType() == Service::Mocap && evt.getUserId() == myActiveUserId)
    {
        // By convention (as of omicron 3.0), if this mocap event has int extra data, 
        // the first field is a joint id. This will not break with previous versions
        // of omicron, but no joint data will be read here.
        if(!evt.isExtraDataNull(0) && 
            evt.getExtraDataType() == Event::ExtraDataIntArray &&
            evt.getExtraDataInt(0) == Event::OMICRON_SKEL_HEAD)
        {
            Event& mutableEvent = const_cast<Event&>(evt);
            int headTrackerId = getEngine()->getDefaultCamera()->getTrackerSourceId();
            mutableEvent.resetSourceId(headTrackerId);
        }
    }

    // See if this event happens inside the limits of the AppLauncher container, and convert it to a pointer event.
    if(evt.getServiceType() == Service::Pointer || 
        evt.getServiceType() == Service::Wand)
    {
        const Vector3f& pos3 = evt.getPosition();
        Vector2i pos(pos3[0], pos3[1]);
        
        if(evt.getServiceType() == Service::Wand && 
            !evt.isExtraDataNull(2) && !evt.isExtraDataNull(3))
        {
            DisplayConfig& dcfg = SystemManager::instance()->getDisplaySystem()->getDisplayConfig();
            pos[0] = evt.getExtraDataFloat(2) * dcfg.displayResolution[0];
            pos[1] = (1.0f - evt.getExtraDataFloat(3)) * dcfg.displayResolution[1];
            //ofmsg("pos: %1% %2%", 
            //    %evt.getExtraDataFloat(2) %evt.getExtraDataFloat(3));
        }

        if(myModifyingCanvas)
        {
            if(evt.getType() == Event::Down)
            {
                // When set to true, pointer events controlling the window are happening on
                // the window itself. This flag is needed to apply pointer delta correction
                // after window is moved.
                if(MouseService::instance() != NULL)
                {
                    if(evt.isFrom(MouseService::instance(), 0))
                    {
                        myUsingLocalPointer = true;
                    }
                }

                DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
                DisplayConfig& dc = ds->getDisplayConfig();

                if(evt.isFlagSet(myMoveButton))
                {
                    myMovingCanvas = true;
                    dc.bringToFront();
                }
                else if(evt.isFlagSet(myResizeButton))
                {
                    mySizingCanvas = true;
                    dc.bringToFront();
                }

            }
            else if(evt.getType() == Event::Up)
            {
                mySizingCanvas = false;
                myMovingCanvas = false;
                myUsingLocalPointer = false;
            }

            if(mySizingCanvas || myMovingCanvas)
            {
                myPointerDelta += pos - myLastPointerPos;
            }

            evt.setProcessed();
        }
        myLastPointerPos = pos;
    }

    //if(evt.isKeyDown(KC_HOME))
    //{
    //    myModifyingCanvas = !myModifyingCanvas;

    //    if(myModifyingCanvas) show();
    //    else hide();
    //}
    //else
    if(myModifyingCanvas && evt.getType() == Event::Down)
    {
        foreach(Shortcut* s, myShortcuts)
        {
            if(evt.isButtonDown(s->button))
            {
                // Activate a target workspace
                if(s->target != NULL)
                {
                    if(s->target->onActivated != "")
                    {
                        PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();
                        pi->queueCommand(s->target->onActivated);
                    }
                    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
                    DisplayConfig& dc = ds->getDisplayConfig();
                    dc.bringToFront();
                    const Rect& r = s->target->getWorkspaceRect();
                    setAppCanvas(r);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppController::show()
{
    myVisible = true;
    myBackground->setEnabled(false);
    myBackground->setVisible(true);

    Container* root = myBackground->getContainer();
    root->setStyleValue("border", "2 #FFB638");
}

///////////////////////////////////////////////////////////////////////////////
void AppController::hide()
{
    //omsg("Menu hide");

    myVisible = false;
    myBackground->setEnabled(false);
    myBackground->setVisible(false);

    Container* root = myBackground->getContainer();
    root->setStyleValue("border", "1 #D119FF");
}

///////////////////////////////////////////////////////////////////////////////
AppController::Shortcut* AppController::getOrCreateSortcut(Event::Flags button)
{
    foreach(Shortcut* s, myShortcuts)
    {
        if(s->button == button) return s;
    }
    Shortcut* s = new Shortcut();
    s->button = button;
    myShortcuts.push_back(s);
    return s;
}

///////////////////////////////////////////////////////////////////////////////
void AppController::setShortcut(Event::Flags button, const String& target, const String& command)
{
    Shortcut* s = getOrCreateSortcut(button);
    WorkspaceLibrary* wm = WorkspaceLibrary::instance();
    if(wm != NULL)
    {
        Vector<String> args = StringUtils::split(target, " ");
        if(args.size() == 2)
        {
            Workspace* w = wm->getWorkspace(args[0], args[1]);
            if(w != NULL)
            {
                s->target = w;
                s->icon = w->getIcon();
                return;
            }
        }
    }

    s->icon = ImageUtils::loadImage(target);
}

///////////////////////////////////////////////////////////////////////////////
void AppController::setAppCanvas(const Rect& canvasRect)
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dc = ds->getDisplayConfig();
    Rect canvas = dc.getCanvasRect();
    dc.setCanvasRect(canvasRect);

    MissionControlClient* cli = SystemManager::instance()->getMissionControlClient();
    if(cli != NULL && cli->isConnected() && cli->getName() != "server")
    {
        cli->postCommand(ostr(
            "@server:"
            "AppManager.instance().onAppCanvasChange('%1%', %2%, %3%, %4%, %5%)",
            %cli->getName() 
            %canvasRect.x() %canvasRect.y()
            %canvasRect.width() %canvasRect.height()));
    }
}