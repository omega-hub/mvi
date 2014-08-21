#include "WorkspaceLibrary.h"
#include "AppManager.h"
#include "AppLauncher.h"
#include "AppController.h"

///////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#ifdef OMEGA_USE_PYTHON
#include "omega/PythonInterpreterWrapper.h"

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(AppController_setShortcut, setShortcut, 2, 3)
BOOST_PYTHON_MODULE(mvi)
{
    PYAPI_REF_BASE_CLASS(WorkspaceLibrary)
        PYAPI_STATIC_REF_GETTER(WorkspaceLibrary, create)
        PYAPI_STATIC_REF_GETTER(WorkspaceLibrary, instance)
        PYAPI_REF_GETTER(WorkspaceLibrary, createLayout)
        PYAPI_REF_GETTER(WorkspaceLibrary, findLayout)
        ;

    PYAPI_REF_BASE_CLASS(AppManager)
        PYAPI_STATIC_REF_GETTER(AppManager, instance)
        PYAPI_STATIC_REF_GETTER(AppManager, create)
        PYAPI_STATIC_REF_GETTER(AppManager, onAppCanvasChange)
        //PYAPI_METHOD(AppManager, requestWorkspace)
        //PYAPI_METHOD(AppManager, freeWorkspace)
        ;

    PYAPI_REF_BASE_CLASS(Workspace)
        PYAPI_METHOD(Workspace, setTiles)
        PYAPI_PROPERTY(Workspace, onActivated)
        ;

    Workspace* (WorkspaceLayout::*createWorkspace1)(const String&, const String&, const String&) = &WorkspaceLayout::createWorkspace;
    Workspace* (WorkspaceLayout::*createWorkspace2)(const String&, const String&, float, float, float, float) = &WorkspaceLayout::createWorkspace;
    PYAPI_REF_BASE_CLASS(WorkspaceLayout)
        .def("createWorkspace", createWorkspace1, PYAPI_RETURN_REF)
        .def("createWorkspace", createWorkspace2, PYAPI_RETURN_REF)
        PYAPI_REF_GETTER(WorkspaceLayout, findWorkspace)
        ;

    // App drawer
    PYAPI_REF_BASE_CLASS(AppLauncher)
        PYAPI_STATIC_REF_GETTER(AppLauncher, create)
        PYAPI_STATIC_REF_GETTER(AppLauncher, instance)
        PYAPI_METHOD(AppLauncher, show)
        PYAPI_METHOD(AppLauncher, hide)
        PYAPI_METHOD(AppLauncher, isVisible)
        PYAPI_METHOD(AppLauncher, addApp)
        PYAPI_METHOD(AppLauncher, setDrawerScale)
        PYAPI_METHOD(AppLauncher, getDrawerScale)
        PYAPI_METHOD(AppLauncher, getIconSize)
        PYAPI_METHOD(AppLauncher, setIconSize)
        PYAPI_METHOD(AppLauncher, onAppConnected)
        PYAPI_METHOD(AppLauncher, onAppDisconnected)
        PYAPI_REF_GETTER(AppLauncher, getContainer)
        PYAPI_METHOD(AppLauncher, removeAppInstance)
        ;

    // AppInfo
    PYAPI_REF_BASE_CLASS_WITH_CTOR(AppInfo)
        PYAPI_PROPERTY(AppInfo, command)
        PYAPI_PROPERTY(AppInfo, file)
        PYAPI_PROPERTY(AppInfo, id)
        PYAPI_PROPERTY(AppInfo, label)
        PYAPI_PROPERTY(AppInfo, iconFile)
        ;

    // App drawer
    PYAPI_REF_BASE_CLASS(AppController)
        PYAPI_STATIC_REF_GETTER(AppController, create)
        PYAPI_METHOD(AppController, setButton)
        .def("setShortcut", &AppController::setShortcut, AppController_setShortcut())
        ;
}
#endif
