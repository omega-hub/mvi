#include "WorkspaceAllocator.h"

#include <omegaToolkit.h>

using namespace omegaToolkit;
using namespace omegaToolkit::ui;

WorkspaceAllocator* sWAInstance = NULL;

///////////////////////////////////////////////////////////////////////////////
WorkspaceAllocator* WorkspaceAllocator::instance()
{
    return sWAInstance;
}

///////////////////////////////////////////////////////////////////////////////
WorkspaceAllocator::WorkspaceAllocator(WorkspaceManager* wm) : 
    myWorkspaceManager(wm),
    myAppId(1)
{
    sWAInstance = this;
}

///////////////////////////////////////////////////////////////////////////////
int WorkspaceAllocator::allocateAppId()
{
    return myAppId++;
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceAllocator::setAppWorkspace(const String& appid, Workspace* w)
{
    myAllocatedWorkspaces[appid] = w;
}

///////////////////////////////////////////////////////////////////////////////
bool WorkspaceAllocator::canAllocate(Workspace* w, const String& client)
{
    foreach(WorkspaceDictionary::Item i, myAllocatedWorkspaces)
    {
        // Skip comparison with workspace currently allocated to target client
        // since it will be freed if this allocation is succesfull.
        if(i.getKey() != client)
        {
            // If workspaces overlap, we cannot allocate the new one.
            if(i.getValue() != NULL && i.getValue()->overlaps(w)) return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
Workspace* WorkspaceAllocator::findFreeWorkspace(int x, int y)
{
    // Find tile containing specified point
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();
    DisplayTileConfig* tile = dcfg.getTileFromPixel(x, y);

    // Find all the workspaces containing the specified tile.
    List<Workspace*> workspaces;
    myWorkspaceManager->getWorkspacesContainingTile(tile, &workspaces);
    // From the list of potential workspaces, find the largest one that has.
    // Not been allocated yet.
    int workspaceArea = 0;
    Workspace* result = NULL;
    foreach(Workspace* w, workspaces)
    {
        if(canAllocate(w, "UNDEFINED CLIENT"))
        {
            int area = w->getWorkspaceRect().width() * w->getWorkspaceRect().height();
            if(area > workspaceArea)
            {
                workspaceArea = area;
                result = w;
            }
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceAllocator::requestWorkspace(const String& client, const String& layout, const String& workspace)
{
    ofmsg("WorkspaceAllocator::requestWorkspace %1% %2% %3%",
        %client %layout %workspace);

    // Process special workspace requests
    if(layout == "SPECIAL")
    {
        if(workspace == "MINIMIZED")
        {
            // Free client workspace
            freeWorkspace(client);
            // Tell the client the workspace allocation request has been granted.
            MissionControlClient* mcc = SystemManager::instance()->getMissionControlClient();
            mcc->postCommand(ostr(
                "@%1%:"
                "WorkspaceManager.instance().setActiveWorkspace('SPECIAL MINIMIZED')", %client));
        }
    }
    else
    {
        // Find the workspace
        Workspace* w = myWorkspaceManager->getWorkspace(layout, workspace);
        if(canAllocate(w, client))
        {
            // Allocate the selected workspace to this client.
            allocateWorkspace(w, client);

            // Tell the client the workspace allocation request has been granted.
            MissionControlClient* mcc = SystemManager::instance()->getMissionControlClient();
            mcc->postCommand(ostr(
                "@%1%:"
                "WorkspaceManager.instance().setActiveWorkspace('%2% %3%')", %client %layout %workspace));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceAllocator::allocateWorkspace(Workspace* w, const String& client)
{
    myAllocatedWorkspaces[client] = w;
    updateLocalActiveTiles();
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceAllocator::freeWorkspace(const String& client)
{
    myAllocatedWorkspaces[client] = NULL;
    updateLocalActiveTiles();
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceAllocator::updateLocalActiveTiles()
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();

    // NOTE: we can't do this easily with workspace rects and no tile info.
    // First enable all tiles
    //foreach(DisplayConfig::Tile t, dcfg.tiles) t->enabled = true;

    //foreach(WorkspaceDictionary::Item i, myAllocatedWorkspaces)
    //{
    //    if(i.getValue() != NULL)
    //    {
    //        // Then disable tiles used by any allocated workspace.
    //        foreach(DisplayTileConfig* dtc, i->myTiles) dtc->enabled = false;
    //    }
    //}
}
