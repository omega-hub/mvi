#include "AppControlOverlay.h"
#include "WorkspaceManager.h"

///////////////////////////////////////////////////////////////////////////////
AppControlOverlay* AppControlOverlay::create()
{
    PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();

    AppControlOverlay* ad = new AppControlOverlay(pi);
    ModuleServices::addModule(ad);
    return ad;
}

///////////////////////////////////////////////////////////////////////////////
AppControlOverlay::AppControlOverlay(PythonInterpreter* interp) :
myUi(NULL), myInterpreter(interp), myModifyingCanvas(false),
myMovingCanvas(false), mySizingCanvas(false), myPointerDelta(Vector2i::Zero())
{
    setPriority(EngineModule::PriorityHigh);
}

///////////////////////////////////////////////////////////////////////////////
void AppControlOverlay::initialize()
{
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
}

///////////////////////////////////////////////////////////////////////////////
void AppControlOverlay::update(const UpdateContext& context)
{
    // Control canvas size/position if we are in canvas move/size mode.
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dc = ds->getDisplayConfig();
    Rect canvas = dc.getCanvasRect();

    if(myMovingCanvas)
    {
        canvas.min += myPointerDelta;
        canvas.max += myPointerDelta;
        dc.setCanvasRect(canvas);
        myLastPointerPos -= myPointerDelta;
    }
    else if(mySizingCanvas)
    {
        canvas.max += myPointerDelta;
        dc.setCanvasRect(canvas);
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
void AppControlOverlay::handleEvent(const Event& evt)
{
    // See if this event happens inside the limits of the AppLauncher container, and convert it to a pointer event.
    if(evt.getServiceType() == Service::Pointer)
    {
        const Vector3f& pos3 = evt.getPosition();
        Vector2i pos(pos3[0], pos3[1]);

        if(myModifyingCanvas)
        {
            if(evt.getType() == Event::Down)
            {
                if(evt.isFlagSet(Event::Left)) myMovingCanvas = true;
                else if(evt.isFlagSet(Event::Right)) mySizingCanvas = true;

            }
            else if(evt.getType() == Event::Up)
            {
                mySizingCanvas = false;
                myMovingCanvas = false;
            }

            if(mySizingCanvas || myMovingCanvas)
            {
                myPointerDelta += pos - myLastPointerPos;
            }

            evt.setProcessed();
        }
        myLastPointerPos = pos;
    }

    if(evt.isKeyDown(KC_HOME))
    {
        myModifyingCanvas = !myModifyingCanvas;

        if(myModifyingCanvas) show();
        else hide();
    }
    else if(myModifyingCanvas && evt.getType() == Event::Down)
    {
        foreach(Shortcut* s, myShortcuts)
        {
            if(evt.isButtonDown(s->button))
            {
                // Activate a target workspace
                if(s->target != NULL)
                {
                    s->target->requestActivation();
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppControlOverlay::show()
{
    myVisible = true;
    myBackground->setEnabled(false);
    myBackground->setVisible(true);

    Container* root = myBackground->getContainer();
    root->setStyleValue("border", "2 #FFB638");
}

///////////////////////////////////////////////////////////////////////////////
void AppControlOverlay::hide()
{
    //omsg("Menu hide");

    myVisible = false;
    myBackground->setEnabled(false);
    myBackground->setVisible(false);

    Container* root = myBackground->getContainer();
    root->setStyleValue("border", "1 #D119FF");
}

///////////////////////////////////////////////////////////////////////////////
AppControlOverlay::Shortcut* AppControlOverlay::getOrCreateSortcut(Event::Flags button)
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
void AppControlOverlay::setShortcut(Event::Flags button, const String& target, const String& command)
{
    Shortcut* s = getOrCreateSortcut(button);
    WorkspaceManager* wm = WorkspaceManager::instance();
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
