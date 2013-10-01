#include "WandInputFilter.h"

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
}
#endif
