#ifndef __APP_CONTROL_OVERLAY__
#define __APP_CONTROL_OVERLAY__

#include "omega/Engine.h"
#include "omega/Application.h"
#include "omega/PythonInterpreter.h"
#include "omegaToolkit/UiModule.h"
#include "omegaToolkit/ui/Container.h"
#include "omegaToolkit/ui/Button.h"
#include "omegaToolkit/ui/WidgetFactory.h"
#include "omegaToolkit/ui/Widget.h"
#include "omegaToolkit/UiScriptCommand.h"

class AppLauncher;

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
class AppController: public EngineModule
{
public:
    static AppController* create(const String& name);
    //! Sets the button flags for the mode switch, mode and resize physical buttons.
    //! This method is called by the remote application manager to setup
    //! application buttons based on a global shared configuration.
    static void configPhysicalButtons(uint modeSwitch, uint move, uint resize);
    static void setActiveUser(int userId);

public:
    AppController(const String& name, PythonInterpreter* interp);
    virtual ~AppController() {}

    void initialize();
    void update(const UpdateContext& context);
    void handleEvent(const Event& evt);

private:
    void parseConfig(Config* cfg);
    void setAppCanvas(const Rect& canvasRect);

private:
    static Event::Flags mysModeSwitchButton;
    static Event::Flags mysMoveButton;
    static Event::Flags mysResizeButton;
    static int mysActiveUserId;

    Ref<UiModule> myUi;
    
    int myBorderSize;

    PythonInterpreter* myInterpreter;

    // When set to true, pointer events controlling the window are happening on
    // the window itself. This flag is needed to apply pointer delta correction
    // after window is moved.
    bool myUsingLocalPointer;
    // When absolute mode is set, moving and resizing will work differently.
    // Resizing, instead of extending/shrinking the existing canvas, will let the
    // user brush a rectangle to re-define both the canvas size and position.
    bool myAbsoluteMode;
    bool myModifyingCanvas;
    bool myMovingCanvas;
    bool mySizingCanvas;
    // Pointer deltas used to compute canvas movement / resize
    Vector2i myLastPointerPos;
    Vector2i myPointerDelta;

    int myActiveUserId;
};
#endif