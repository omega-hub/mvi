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
}

///////////////////////////////////////////////////////////////////////////////
void AppControlOverlay::initialize()
{
    myUi = UiModule::createAndInitialize();

    String activeStyle = "alpha: 1.0; scale: 1.0;";
    String inactiveStyle = "alpha: 0.2; scale: 0.8;";

    myDrawerScale = 1.0f;
    myIconSize = 16;
    myContainer = Container::create(Container::LayoutVertical, myUi->getUi());
    myContainer->setFillColor(Color::Black);
    myContainer->setFillEnabled(true);
    myContainer->setHorizontalAlign(Container::AlignLeft);
    myContainer->setPosition(Vector2f(10, 10));

    Container* titleBar = Container::create(Container::LayoutHorizontal, myContainer);
    Label* l = Label::create(titleBar);
    l->setText(getEngine()->getApplication()->getName());

    myMinimizeButton = Button::create(titleBar);
    myMinimizeButton->setIcon(ImageUtils::loadImage("mvi/icons/minimize.png"));
    myMinimizeButton->setText("");
    myMinimizeButton->getImage()->setSize(Vector2f(myIconSize, myIconSize));
    myMinimizeButton->setActiveStyle(activeStyle);
    myMinimizeButton->setInactiveStyle(inactiveStyle);
    myMinimizeButton->setStyle(inactiveStyle);
    myMinimizeButton->setUIEventCommand("WorkspaceManager.instance().requestWorkspace('SPECIAL', 'MINIMIZED')");

    myCloseButton = Button::create(titleBar);
    myCloseButton->setIcon(ImageUtils::loadImage("mvi/icons/close.png"));
    myCloseButton->setText("");
    myCloseButton->getImage()->setSize(Vector2f(myIconSize, myIconSize));
    myCloseButton->setActiveStyle(activeStyle);
    myCloseButton->setInactiveStyle(inactiveStyle);
    myCloseButton->setStyle(inactiveStyle);
    myCloseButton->setUIEventCommand("oexit()");

    WorkspaceManager* wm = WorkspaceManager::instance();
    if(wm != NULL)
    {
        wm->createUi(myContainer);
    }

    show();
    //hide();
}

///////////////////////////////////////////////////////////////////////////////
void AppControlOverlay::update(const UpdateContext& context)
{
    float speed = context.dt * 10;

    ui::Container3dSettings& c3ds = myContainer->get3dSettings();

    if(myContainer->isVisible())
    {
        if(c3ds.alpha <= 0.1f)
        {
            myContainer->setVisible(false);
            myVisible = false;
        }
    }
    else
    {
        if(c3ds.alpha > 0.1f)
        {
            myContainer->setVisible(true);
            myVisible = true;
        }
    }

    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dc = ds->getDisplayConfig();
    if(myMovingCanvas)
    {
        Rect canvas = dc.getCanvasRect();
        canvas.min += myPointerDelta;
        canvas.max += myPointerDelta;
        dc.setCanvasRect(canvas);
        myLastPointerPos -= myPointerDelta;
    }
    else if(mySizingCanvas)
    {
        Rect canvas = dc.getCanvasRect();
        canvas.max += myPointerDelta;
        dc.setCanvasRect(canvas);
        //ofmsg("Sizing: %1%", %myPointerDelta);
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
        Container* root = myContainer->getContainer();
        if(myModifyingCanvas)
        {
            root->setStyleValue("border", "2 #FFB638");
        }
        else
        {
            root->setStyleValue("border", "1 #D119FF");
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppControlOverlay::show()
{
    myVisible = true;
    myContainer->setEnabled(true);
    
    UiModule::instance()->activateWidget(myContainer);
}

///////////////////////////////////////////////////////////////////////////////
void AppControlOverlay::hide()
{
    //omsg("Menu hide");

    myVisible = false;
    myContainer->setEnabled(false);
    //myContainer->setDebugModeEnabled(true);

    UiModule::instance()->activateWidget(NULL);
}
