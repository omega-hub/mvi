#include <omega/MouseService.h>
#include <omega/Platform.h>
#include <omega/MissionControl.h>
#include <omega/DisplaySystem.h>

#include "AppController.h"

Event::Flags AppController::mysModeSwitchButton = Event::Alt;
Event::Flags AppController::mysMoveButton = Event::Button1;
Event::Flags AppController::mysResizeButton = Event::Button2;
int AppController::mysActiveUserId = false;

///////////////////////////////////////////////////////////////////////////////
void AppController::configPhysicalButtons(uint modeSwitch, uint move, uint resize)
{
    mysModeSwitchButton = (Event::Flags)modeSwitch;
    mysMoveButton = (Event::Flags)move;
    mysResizeButton = (Event::Flags)resize;
}

///////////////////////////////////////////////////////////////////////////////
void AppController::setActiveUser(int userId)
{
    mysActiveUserId = userId;
}

///////////////////////////////////////////////////////////////////////////////
AppController* AppController::create(const String& name)
{
    PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();

    AppController* ad = new AppController(name, pi);
    ModuleServices::addModule(ad);
    return ad;
}

///////////////////////////////////////////////////////////////////////////////
AppController::AppController(const String& name, PythonInterpreter* interp) :
EngineModule(name),
myUi(NULL), myInterpreter(interp), myModifyingCanvas(false),
myActiveUserId(-1),
myMovingCanvas(false), mySizingCanvas(false), myPointerDelta(Vector2i::Zero()),
myUsingLocalPointer(false),
myBorderSize(2),
myAbsoluteMode(false)
{
    setPriority(EngineModule::PriorityHighest);
}

///////////////////////////////////////////////////////////////////////////////
void AppController::parseConfig(Config* cfg)
{
    if(cfg->exists("config/modules/" + getName()))
    {
        Setting& s = cfg->lookup("config/modules/" + getName());
        String sModeSwitchButton = Config::getStringValue("modeSwitchButton", s, "");
        String sMoveButton = Config::getStringValue("moveButton", s, "");
        String sResizeButton = Config::getStringValue("resizeButton", s, "");
        if(sModeSwitchButton != "") mysModeSwitchButton = Event::parseButtonName(sModeSwitchButton);
        if(sMoveButton != "") mysMoveButton = Event::parseButtonName(sMoveButton);
        if(sResizeButton != "") mysResizeButton = Event::parseButtonName(sResizeButton);
        
        myAbsoluteMode = Config::getBoolValue("absoluteMode", s, myAbsoluteMode);
        
        myBorderSize = Config::getIntValue("borderSize", s, myBorderSize);
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppController::initialize()
{
    // Read config options
    parseConfig(SystemManager::instance()->getSystemConfig());
    parseConfig(SystemManager::instance()->getAppConfig());
    myBorderSize *= Platform::scale * 2;

    myUi = UiModule::createAndInitialize();

    //myDrawerScale = 1.0f;

    // Send our display size to the application manager.
    MissionControlClient* cli = SystemManager::instance()->getMissionControlClient();
    if(cli != NULL && cli->isConnected() && cli->getName() != "server")
    {
        // Also do an initial setAppCanvas to let the server know our canvas information
        DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
        DisplayConfig& dc = ds->getDisplayConfig();
        Rect canvas = dc.getCanvasRect();
        setAppCanvas(canvas);
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
        dc.setCanvasRect(canvas);
        
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
        dc.setCanvasRect(canvas);
        //ofmsg("Sizing: %1%", %myPointerDelta);
    }
    
    if(myActiveUserId != mysActiveUserId)
    {
        myActiveUserId = mysActiveUserId;
        getEngine()->getDefaultCamera()->setTrackerUserId(myActiveUserId);
        ofmsg("switching to user %1%", %myActiveUserId);
        //if(mysFocused)
        {
            // Choose a color based on active user ID:
            String cs = Color::getColorByIndex(myActiveUserId).toString();
            myUi->getUi()->setStyleValue("border", 
                ostr("%1% %2%", 
                    %myBorderSize 
                    %cs));
            
            // We are focused, make sure the application canvas is on front of
            // all other app canvases on the display.
            dc.bringToFront();
        }
        /*else
        {
            Container* root = myBackground->getContainer();
            root->setStyleValue("border", "0 black");
        }*/
    }
    myPointerDelta = Vector2i::Zero();
}

///////////////////////////////////////////////////////////////////////////////
void AppController::handleEvent(const Event& evt)
{
    /*if(evt.isButtonDown(mysModeSwitchButton))
    {
        myModifyingCanvas = !myModifyingCanvas;
        
        if(myModifyingCanvas)
        {
            // The mode switch button s also used to set the active user, so for
            // instance we can start tracking the head of the user whose controller
            // generated this event.
            myActiveUserId = evt.getUserId();
            getEngine()->getDefaultCamera()->setTrackerUserId(myActiveUserId);
            if(myShowOverlay) show();
        }
        else
        {
            myMovingCanvas = false;
            mySizingCanvas = false;
            if(myShowOverlay) hide();
            
            // Done modifying canvas: send canvas update
            DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
            DisplayConfig& dc = ds->getDisplayConfig();
            Rect canvas = dc.getCanvasRect();
            setAppCanvas(canvas);
        }
    }*/

    // See if this event happens inside the limits of the AppLauncher container
    if(evt.getServiceType() == Service::Pointer)
    {
        const Vector3f& pos3 = evt.getPosition();
        Vector2i pos(pos3[0], pos3[1]);

        //if(myModifyingCanvas)
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

                if(evt.isFlagSet(mysMoveButton))
                {
                    myMovingCanvas = true;
                    dc.bringToFront();
                    
                    // Don't draw pointers while we move / resize the canvas.
                    // Since they will look to be drifting 
                    getEngine()->setDrawPointers(false);
                    evt.setProcessed();
                }
                else if(evt.isFlagSet(mysResizeButton))
                {
                    mySizingCanvas = true;
                    dc.bringToFront();

                    // Don't draw pointers while we move / resize the canvas.
                    // Since they will look to be drifting 
                    getEngine()->setDrawPointers(false);
                    
                    // If we are in absolute mode, reset the canvas size and position
                    if(myAbsoluteMode)
                    {
                        DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
                        DisplayConfig& dc = ds->getDisplayConfig();
                        Rect canvas = dc.getCanvasRect();
                        canvas.min += pos;
                        // give it some initial size...
                        int sz = 32 * Platform::scale;
                        canvas.max = canvas.min + Vector2i(sz,sz);
                        dc.setCanvasRect(canvas);
                    }
                    evt.setProcessed();
                }
                
            }
            else if(evt.getType() == Event::Up)
            {
                mySizingCanvas = false;
                myMovingCanvas = false;
                myUsingLocalPointer = false;
                
                // Done modifying canvas: send canvas update
                DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
                DisplayConfig& dc = ds->getDisplayConfig();
                Rect canvas = dc.getCanvasRect();
                setAppCanvas(canvas);
                
                //omsg("moving done");
                
                getEngine()->setDrawPointers(true);
            }

            if(mySizingCanvas || myMovingCanvas)
            {
                myPointerDelta += pos - myLastPointerPos;
            }
        }
        myLastPointerPos = pos;
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppController::setAppCanvas(const Rect& canvasRect)
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dc = ds->getDisplayConfig();
    dc.setCanvasRect(canvasRect);

    MissionControlClient* cli = SystemManager::instance()->getMissionControlClient();
    if(cli != NULL && cli->isConnected() && cli->getName() != "server")
    {
        ofmsg(">>> %1% sending canvas %2% %3% %4% %5%", %cli->getName() %canvasRect.x() %canvasRect.y()
            %canvasRect.width() %canvasRect.height());
            
        cli->postCommand(ostr(
            "@server:"
            "appmgr.onAppCanvasChange('%1%', %2%, %3%, %4%, %5%)",
            %cli->getName() 
            %canvasRect.x() %canvasRect.y()
            %canvasRect.width() %canvasRect.height()));
    }
}
