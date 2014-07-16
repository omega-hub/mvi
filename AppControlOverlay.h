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
class AppControlOverlay: public EngineModule
{
public:
    static AppControlOverlay* create();

public:
    AppControlOverlay(PythonInterpreter* interp);
    virtual ~AppControlOverlay() {}

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

private:
    Ref<UiModule> myUi;
    
    Ref<Container> myContainer;
    Ref<Button> myMinimizeButton;
    Ref<Button> myCloseButton;
    Ref<Button> myExpandLeftButton;
    Ref<Button> myExpandRightButton;
    Ref<Button> myShrinkRightButton;
    Ref<Button> myShrinkLeftButton;

    PythonInterpreter* myInterpreter;
    bool myVisible;
    float myDrawerScale;
    int myIconSize;

    bool myModifyingCanvas;
    bool myMovingCanvas;
    bool mySizingCanvas;
    // Pointer deltas used to compute canvas movement / resize
    Vector2i myLastPointerPos;
    Vector2i myPointerDelta;
};
#endif