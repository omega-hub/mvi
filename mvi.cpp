#include "WorkspaceLibrary.h"
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
    PYAPI_REF_BASE_CLASS(AppController)
        PYAPI_STATIC_REF_GETTER(AppController, create)
        PYAPI_STATIC_METHOD(AppController, configPhysicalButtons)
        PYAPI_STATIC_METHOD(AppController, setFocus)
        PYAPI_METHOD(AppController, setButton)
        .def("setShortcut", &AppController::setShortcut, AppController_setShortcut())
        ;
}
#endif
