#include "AppManager.h"

#include <omegaToolkit.h>

using namespace omegaToolkit;
using namespace omegaToolkit::ui;

AppManager* sWAInstance = NULL;

///////////////////////////////////////////////////////////////////////////////
AppManager* AppManager::instance()
{
    return sWAInstance;
}

///////////////////////////////////////////////////////////////////////////////
AppManager* AppManager::create()
{
    sWAInstance = new AppManager();
    return sWAInstance;
}

///////////////////////////////////////////////////////////////////////////////
AppManager::AppManager() : 
    myAppId(1),
    myCurrentTopZ(0)
{
    ModuleServices::addModule(this);
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::initialize()
{
    SystemManager* sys = getEngine()->getSystemManager();

    // Set the mission control callbacks
    myMC->setClientConnectedCommand("AppManager.instance().onAppConnected('%clientId%')");
    myMC->setClientDisconnectedCommand("AppManager.instance().onAppDisconnected('%clientId%')");

    // Use the python interpreter to load and initialize the porthole web server.
    PythonInterpreter* interpreter = sys->getScriptInterpreter();
    interpreter->eval("import porthole");
    interpreter->eval("porthole.initialize('pointer.xml')");
    interpreter->eval("print('------------------\\nweb server ready')");
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::handleEvent(const Event& evt)
{
    if(evt.getServiceType() == Event::ServiceTypePointer)
    {
        ofmsg("Pointer at %1%", %evt.getPosition());
    }
}

///////////////////////////////////////////////////////////////////////////////
int AppManager::allocateAppId()
{
    return myAppId++;
}

///////////////////////////////////////////////////////////////////////////////
//void AppManager::setAppWorkspace(const String& appid, Workspace* w)
//{
//    myAllocatedWorkspaces[appid] = w;
//}

///////////////////////////////////////////////////////////////////////////////
//bool AppManager::canAllocate(Workspace* w, const String& client)
//{
//    foreach(WorkspaceDictionary::Item i, myAllocatedWorkspaces)
//    {
//        // Skip comparison with workspace currently allocated to target client
//        // since it will be freed if this allocation is succesfull.
//        if(i.getKey() != client)
//        {
//            // If workspaces overlap, we cannot allocate the new one.
//            if(i.getValue() != NULL && i.getValue()->overlaps(w)) return false;
//        }
//    }
//    return true;
//}

///////////////////////////////////////////////////////////////////////////////
//Workspace* AppManager::findFreeWorkspace(int x, int y)
//{
//    // Find tile containing specified point
//    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
//    DisplayConfig& dcfg = ds->getDisplayConfig();
//    DisplayTileConfig* tile = dcfg.getTileFromPixel(x, y);
//
//    // Find all the workspaces containing the specified tile.
//    List<Workspace*> workspaces;
//    myWorkspaceLibrary->getWorkspacesContainingTile(tile, &workspaces);
//    // From the list of potential workspaces, find the largest one that has.
//    // Not been allocated yet.
//    int workspaceArea = 0;
//    Workspace* result = NULL;
//    foreach(Workspace* w, workspaces)
//    {
//        if(canAllocate(w, "UNDEFINED CLIENT"))
//        {
//            int area = w->getWorkspaceRect().width() * w->getWorkspaceRect().height();
//            if(area > workspaceArea)
//            {
//                workspaceArea = area;
//                result = w;
//            }
//        }
//    }
//
//    return result;
//}

///////////////////////////////////////////////////////////////////////////////
//void AppManager::requestWorkspace(const String& client, const String& layout, const String& workspace)
//{
//    ofmsg("AppManager::requestWorkspace %1% %2% %3%",
//        %client %layout %workspace);
//
//    // Process special workspace requests
//    if(layout == "SPECIAL")
//    {
//        if(workspace == "MINIMIZED")
//        {
//            // Free client workspace
//            freeWorkspace(client);
//            // Tell the client the workspace allocation request has been granted.
//            MissionControlClient* mcc = SystemManager::instance()->getMissionControlClient();
//            mcc->postCommand(ostr(
//                "@%1%:"
//                "WorkspaceLibrary.instance().setActiveWorkspace('SPECIAL MINIMIZED')", %client));
//        }
//    }
//    else
//    {
//        // Find the workspace
//        Workspace* w = myWorkspaceLibrary->getWorkspace(layout, workspace);
//        if(canAllocate(w, client))
//        {
//            // Allocate the selected workspace to this client.
//            allocateWorkspace(w, client);
//
//            // Tell the client the workspace allocation request has been granted.
//            MissionControlClient* mcc = SystemManager::instance()->getMissionControlClient();
//            mcc->postCommand(ostr(
//                "@%1%:"
//                "WorkspaceLibrary.instance().setActiveWorkspace('%2% %3%')", %client %layout %workspace));
//        }
//    }
//}

///////////////////////////////////////////////////////////////////////////////
//void AppManager::allocateWorkspace(Workspace* w, const String& client)
//{
//    myAllocatedWorkspaces[client] = w;
//    updateLocalActiveTiles();
//}

///////////////////////////////////////////////////////////////////////////////
//void AppManager::freeWorkspace(const String& client)
//{
//    myAllocatedWorkspaces[client] = NULL;
//    updateLocalActiveTiles();
//}

///////////////////////////////////////////////////////////////////////////////
//void AppManager::updateLocalActiveTiles()
//{
//    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
//    DisplayConfig& dcfg = ds->getDisplayConfig();
//
//    // NOTE: we can't do this easily with workspace rects and no tile info.
//    // First enable all tiles
//    //foreach(DisplayConfig::Tile t, dcfg.tiles) t->enabled = true;
//
//    //foreach(WorkspaceDictionary::Item i, myAllocatedWorkspaces)
//    //{
//    //    if(i.getValue() != NULL)
//    //    {
//    //        // Then disable tiles used by any allocated workspace.
//    //        foreach(DisplayTileConfig* dtc, i->myTiles) dtc->enabled = false;
//    //    }
//    //}
//}

///////////////////////////////////////////////////////////////////////////////
void AppManager::onAppCanvasChange(const String& appid, int x, int y, int w, int h)
{
    if(myAppInstances.find(appid) == myAppInstances.end())
    {
        AppInstance* ai = new AppInstance();
        ai->id = appid;
        ai->dirtyCanvas = false;
        myAppInstances[appid] = ai;
    }

    AppInstance* ai = myAppInstances[appid];
    ai->z = myCurrentTopZ++;

    // Place app on top of z sorted app instance list.
    myZSortedAppInstances.remove(ai);
    myZSortedAppInstances.push_front(ai);

    ai->targetCanvas = Rect(x, y, w, h);

    computeAppCanvases();
    updateAppCanvases();
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::computeAppCanvases()
{
    // Step 1: stash current canvas rects and reset them to their target values
    Dictionary<String, Rect> currentRects;

    foreach(AppInstance* ai, myZSortedAppInstances)
    {
        currentRects[ai->id] = ai->currentCanvas;
        ai->currentCanvas = ai->targetCanvas;
    }

    // Step 2: resize rects using sorted convex subtraction.
    for(AppInstanceList::iterator i = myZSortedAppInstances.begin();
        i != myZSortedAppInstances.end(); i++)
    {
        Rect& r = i->get()->currentCanvas;
        for(AppInstanceList::iterator j = i;
            j != myZSortedAppInstances.end(); j++)
        {
            if(i != j)
            {
                Rect& k = i->get()->currentCanvas;
                std::pair<bool, Rect> res = k.subtract(r);
                if(res.first) i->get()->currentCanvas = res.second;
            }
        }
    }

    // Step 3: mark all changed rects as dirty.
    foreach(AppInstance* ai, myZSortedAppInstances)
    {
        Rect& prevRect = currentRects[ai->id];
        if(prevRect != ai->currentCanvas)
        {
            ai->dirtyCanvas = true;
        }
        
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppManager::updateAppCanvases()
{
    MissionControlClient* mcc = SystemManager::instance()->getMissionControlClient();

    foreach(AppInstanceDictionary::Item a, myAppInstances)
    {
        // Update canvas for this app, if needed
        if(a->dirtyCanvas)
        {
            a->dirtyCanvas = false;
            mcc->postCommand(ostr(
                "@%1%:"
                "getDisplayConfig().setCanvasRect(Rect(%2%, %3%, %4%, %5%))", 
                %a->id 
                %a->currentCanvas.x() % a->currentCanvas.y()
                % a->currentCanvas.width() % a->currentCanvas.height()));
        }
    }
}
