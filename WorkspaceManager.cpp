#include "WorkspaceManager.h"

#include <omegaToolkit.h>

using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
Workspace::Workspace(const String& name):
myName(name)
{
}

///////////////////////////////////////////////////////////////////////////////
void Workspace::setTiles(const String& tiles)
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();
    Vector<String> tileNames = StringUtils::split(tiles, " ");
    foreach(String tilename, tileNames)
    {
        if(dcfg.tiles.find(tilename) != dcfg.tiles.end())
        {
            myTiles.push_back(dcfg.tiles[tilename]);
        }
        else
        {
            ofwarn("Workspace::setTiles: could not find tile %1%", %tilename);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void Workspace::activate()
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();

    // First disable all tiles
    foreach(DisplayConfig::Tile t, dcfg.tiles) t->enabled = false;

    // The enable tiles in this workspace.
    foreach(DisplayTileConfig* dtc, myTiles) dtc->enabled = true;
}

///////////////////////////////////////////////////////////////////////////////
WorkspaceManager* WorkspaceManager::create()
{
    WorkspaceManager* wm = new WorkspaceManager();
    ModuleServices::addModule(wm);
    return wm;
}

///////////////////////////////////////////////////////////////////////////////
WorkspaceManager::WorkspaceManager()
{
    setPriority(EngineModule::PriorityHigh);
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceManager::initialize() 
{
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceManager::dispose() 
{
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceManager::update(const UpdateContext& context) 
{
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceManager::handleEvent(const Event& evt) 
{
}

///////////////////////////////////////////////////////////////////////////////
bool WorkspaceManager::handleCommand(const String& cmd) 
{
    Vector<String> args = StringUtils::split(cmd);
    if(args[0] == "ws")
    {
        ofmsg("WORKSPACE NOW %1%", %args[1]);
        Workspace* w = findWorkspace(args[1]);
        if(w != NULL)
        {
            w->activate();
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
Workspace* WorkspaceManager::createWorkspace(const String& name)
{
    Workspace* w = new Workspace(name);
    myWorkspaces.push_back(w);
    return w;
}

///////////////////////////////////////////////////////////////////////////////
Workspace* WorkspaceManager::findWorkspace(const String& name)
{
    foreach(Workspace* w, myWorkspaces)
    {
        if(w->getName() == name) return w;
    }
    return NULL;
}
