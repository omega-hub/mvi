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

private:
    void setAppCanvas(const Rect& canvasRect);

    struct Shortcut : public ReferenceType
    {
        Event::Flags button;
        Ref<PixelData> icon;
        Workspace* target;
        String command;
    };

    Shortcut* getOrCreateSortcut(Event::Flags button);

private:
    Ref<UiModule> myUi;
    
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

    Event::Flags myModeSwitchButton;
    Event::Flags myMoveButton;
    Event::Flags myResizeButton;

    // When set to true, pointer events controlling the window are happening on
    // the window itself. This flag is needed to apply pointer delta correction
    // after window is moved.
    bool myUsingLocalPointer;
    bool myModifyingCanvas;
    bool myMovingCanvas;
    bool mySizingCanvas;
    // Pointer deltas used to compute canvas movement / resize
    Vector2i myLastPointerPos;
    Vector2i myPointerDelta;

    // Shortcuts
    List< Ref<Shortcut> > myShortcuts;

    int myActiveUserId;
};
#endif