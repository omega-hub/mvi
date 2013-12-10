#ifndef __MVI_WAND_POINTER_SWITCHER__
#define __MVI_WAND_POINTER_SWITCHER__

#include <omega.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
// Module declaration
class WandPointerSwitcher: public Service
{
public:
	// Allocator function
	static WandPointerSwitcher* New() { return new WandPointerSwitcher(); }

public:
	WandPointerSwitcher();

	virtual void setup(Setting& settings);
	virtual void initialize();
	virtual void poll();
	virtual void dispose();

private:
};

#endif
