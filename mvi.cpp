#include "WandInputFilter.h"
#include "WandPointerSwitcher.h"
#include "WorkspaceManager.h"

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
        PYAPI_REF_GETTER(WorkspaceManager, createWorkspace)
        PYAPI_REF_GETTER(WorkspaceManager, findWorkspace)
        ;

    PYAPI_REF_BASE_CLASS(Workspace)
        PYAPI_METHOD(Workspace, setTiles)
        PYAPI_METHOD(Workspace, activate)
        ;

    def("setup", setup);
}
#endif
