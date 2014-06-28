#include "WandInputFilter.h"
#include "WandPointerSwitcher.h"
#include "WorkspaceManager.h"
#include "WorkspaceAllocator.h"
#include "AppLauncher.h"
#include "AppControlOverlay.h"

///////////////////////////////////////////////////////////////////////////////
// Python API follows
// this fuction registers the Wand Pointer Switcher service with the omicron
// service manager
void setup()
{
    ServiceManager* sm = SystemManager::instance()->getServiceManager();
    sm->registerService("WandPointerSwitcher", (ServiceAllocator)WandPointerSwitcher::New);
}

///////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#ifdef OMEGA_USE_PYTHON
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(mvi)
{
	PYAPI_REF_BASE_CLASS(WandInputFilter)
		PYAPI_STATIC_REF_GETTER(WandInputFilter, createAndInitialize)
		;

    PYAPI_REF_BASE_CLASS(WorkspaceManager)
        PYAPI_STATIC_REF_GETTER(WorkspaceManager, create)
        PYAPI_STATIC_REF_GETTER(WorkspaceManager, instance)
        PYAPI_REF_GETTER(WorkspaceManager, createLayout)
        PYAPI_REF_GETTER(WorkspaceManager, findLayout)
        PYAPI_METHOD(WorkspaceManager, setActiveWorkspace)
        PYAPI_METHOD(WorkspaceManager, requestWorkspace)
        ;

    PYAPI_REF_BASE_CLASS(WorkspaceAllocator)
        PYAPI_STATIC_REF_GETTER(WorkspaceAllocator, instance)
        PYAPI_METHOD(WorkspaceAllocator, requestWorkspace)
        PYAPI_METHOD(WorkspaceAllocator, freeWorkspace)
        ;

    PYAPI_REF_BASE_CLASS(Workspace)
        PYAPI_METHOD(Workspace, setTiles)
        PYAPI_METHOD(Workspace, activate)
        ;

    PYAPI_REF_BASE_CLASS(WorkspaceLayout)
        PYAPI_REF_GETTER(WorkspaceLayout, createWorkspace)
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
    PYAPI_REF_BASE_CLASS(AppControlOverlay)
        PYAPI_STATIC_REF_GETTER(AppControlOverlay, create)
        ;

    def("setup", setup);
}
#endif
