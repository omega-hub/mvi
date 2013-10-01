#ifndef __MVI_WAND_INPUT_FILTER__
#define __MVI_WAND_INPUT_FILTER__

#include <omega.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Module declaration
class WandInputFilter: public EngineModule
{
public:
	static WandInputFilter* createAndInitialize();
	WandInputFilter();

	virtual void initialize();
	virtual void update(const UpdateContext& context);
	virtual void handleEvent(const Event& evt);

private:
};

#endif
