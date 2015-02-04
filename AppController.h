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

#include "Workspace.h"

class AppLauncher;

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
class AppController: public EngineModule
{
public:
    static AppController* create();
    //! Sets the button flags for the mode switch, mode and resize physical buttons.
    //! This method is called by the remote application manager to setup
    //! application buttons based on a global shared configuration.
    static void configPhysicalButtons(uint modeSwitch, uint move, uint resize);
    //! Sets the focused or unfocused mode for this application. Applications in
    //! focused mode are receiving input from an input controller. The application
    //! manager (appmgr) takes care of calling this method. Focused mode only sets
    //! a visual property (i.e. a colored border) of the application.
    static void setFocus(bool value);

public:
    AppController(PythonInterpreter* interp);
    virtual ~AppController() {}

    void initialize();
    ui::Container* getContainer() { return myContainer; }
    UiModule* getUi() { return myUi; }
    void update(const UpdateContext& context);
    void handleEvent(const Event& evt);
    void show();
    void hide();
    bool isVisible() { return myVisible; }
    float getDrawerScale() { return myDrawerScale; }
    void setDrawerScale(float value) { myDrawerScale = value; }
    int getIconSize() { return myIconSize; }
    void setIconSize(int value) { myIconSize = value; }

    void setShortcut(Event::Flags button, const String& target, const String& command = "");
    void setButton(uint index, PixelData* icon, const String& command);

private:
    void parseConfig(Config* cfg);
    void updateButton(uint index);
    void setAppCanvas(const Rect& canvasRect);

    struct Shortcut : public ReferenceType
    {
        Event::Flags button;
        Ref<PixelData> icon;
        Ref<Workspace> target;
        String command;
    };

    Shortcut* getOrCreateSortcut(Event::Flags button);

private:
    static Event::Flags mysModeSwitchButton;
    static Event::Flags mysMoveButton;
    static Event::Flags mysResizeButton;
    static bool mysFocused;

    Ref<UiModule> myUi;
    
    bool myCurrentFocus;
    int myBorderSize;
    Ref<Container> myContainer;
    Ref<Container> myBackground;
    Ref<Button> myMinimizeButton;
    Ref<Button> myCloseButton;
    Ref<Button> myExpandLeftButton;
    Ref<Button> myExpandRightButton;
    Ref<Button> myShrinkRightButton;
    Ref<Button> myShrinkLeftButton;

    PythonInterpreter* myInterpreter;
    bool myShowOverlay;
    bool myVisible;
    float myDrawerScale;
    int myIconSize;

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

    // Shortcuts
    List< Ref<Shortcut> > myShortcuts;

    // Buttons
    Ref<Container> myButtonContainer;
    Ref<PixelData> myButtonIcons[4];
    String myButtonCommands[4];
    Ref<Button> myButtons[4];

    int myActiveUserId;
};
#endif