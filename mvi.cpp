#include "WandInputFilter.h"
#include "WandPointerSwitcher.h"

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
	// SceneLoader
	PYAPI_REF_BASE_CLASS(WandInputFilter)
		PYAPI_STATIC_REF_GETTER(WandInputFilter, createAndInitialize)
		;
    def("setup", setup);
}
#endif
