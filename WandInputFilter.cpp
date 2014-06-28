#include "WandInputFilter.h"

///////////////////////////////////////////////////////////////////////////////
WandInputFilter* WandInputFilter::createAndInitialize()
{
	WandInputFilter* instance = new WandInputFilter();
	ModuleServices::addModule(instance);
	instance->doInitialize(Engine::instance());
	return instance;
}

///////////////////////////////////////////////////////////////////////////////
WandInputFilter::WandInputFilter():
	EngineModule("WandInputFilter")
{
	// This module runs before every other, to be able to filter input events.
	setPriority(EngineModule::PriorityHighest);
}

///////////////////////////////////////////////////////////////////////////////
void WandInputFilter::initialize()
{
}

///////////////////////////////////////////////////////////////////////////////
void WandInputFilter::update(const UpdateContext& context)
{
}

///////////////////////////////////////////////////////////////////////////////
void WandInputFilter::handleEvent(const Event& evt)
{
    if(SystemManager::instance()->isMaster())
    {
        DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
        DisplayConfig& dcfg = ds->getDisplayConfig();

        // If the event is a wand event, get the ray and see if it intersects any
        // active tile.
        if(evt.getServiceType() == Event::ServiceTypeWand)
        {
            // Get sensor-space wand ray
            Ray ray;
            ray.setOrigin(evt.getPosition());
            ray.setDirection(evt.getOrientation() * -Vector3f::UnitZ());

            // Loop through enabled tiles.
            typedef KeyValue<String, DisplayTileConfig*> TileItem;
            foreach(TileItem dtc, dcfg.tiles)
            {
                // If we found an intersection, we are done.
                if(dtc->enabled && dtc->rayIntersects(ray)) 
                {
                    return;
                }
            }
            // No intersection: mark the wand event as processed so it will not
            // be dispatched to other modules.
            evt.setProcessed();
            EventSharingModule::markLocal(evt);
        }
    }
}
