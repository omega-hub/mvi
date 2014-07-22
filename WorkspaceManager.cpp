#include "WorkspaceManager.h"

#include <omegaToolkit.h>

using namespace omegaToolkit;
using namespace omegaToolkit::ui;

WorkspaceManager* sWMInstance = NULL;

///////////////////////////////////////////////////////////////////////////////
WorkspaceManager* WorkspaceManager::create()
{
    WorkspaceManager* wm = new WorkspaceManager();
    sWMInstance = wm;
    return wm;
}

///////////////////////////////////////////////////////////////////////////////
WorkspaceManager* WorkspaceManager::instance()
{
    return sWMInstance;
}

///////////////////////////////////////////////////////////////////////////////
WorkspaceManager::WorkspaceManager()
{
}

///////////////////////////////////////////////////////////////////////////////
WorkspaceLayout* WorkspaceManager::createLayout(const String& name)
{
    WorkspaceLayout* w = new WorkspaceLayout(name, this);
    myLayouts[name] = w;
    return w;
}

///////////////////////////////////////////////////////////////////////////////
WorkspaceLayout* WorkspaceManager::findLayout(const String& name)
{
    if(myLayouts.find(name) == myLayouts.end()) return NULL;
    return myLayouts[name];
}

///////////////////////////////////////////////////////////////////////////////
Workspace* WorkspaceManager::getWorkspace(const String& layout, const String& name)
{
    WorkspaceLayout* wl = findLayout(layout);
    if(wl != NULL)
    {
        Workspace* ws = wl->findWorkspace(name);
        if(wl == NULL)
        {
            ofwarn("Could not find workspace '%1%' in layout '%2%'", %name %layout);
        }
        return ws;
    }
    else
    {
        ofwarn("Could not find workspace layout '%1%'", %layout);
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceManager::setActiveWorkspace(const String& fullname)
{
    // Process special workspace requests
    if(fullname == "SPECIAL MINIMIZED")
    {
        // Whatever the current active workspace is, de-activate it.
        if(myActiveWorkspace != NULL) myActiveWorkspace->deactivate();
    }
    else
    {
        Vector<String> args = StringUtils::split(fullname);
        Workspace* ws = getWorkspace(args[0], args[1]);
        if(ws != NULL)
        {
            myActiveWorkspace = ws;
            ws->activate();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceManager::requestWorkspace(const String& layout, const String& workspace)
{
    MissionControlClient* cli = SystemManager::instance()->getMissionControlClient();

    if(cli->isConnected() && cli->getName() != "server")
    {
        // Send a request to the workspace server to see if we can allocate this workspace.
        // If successful, the workspace server will send us a :ws command with the workspace
        // id to confirm allocation and we will activate it.
        cli->postCommand(ostr(
            "@server:"
            "WorkspaceAllocator.instance().requestWorkspace('%1%', '%2%', '%3%')",
            %cli->getName() % layout %workspace));
    }
    else
    {
        // If we are not connected to a server, always confirm and apply the 
        // workspace request.
        setActiveWorkspace(layout + " " + workspace);
    }
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceManager::getWorkspacesContainingTile(DisplayTileConfig* tile, List<Workspace*>* outWorkspaces)
{
    foreach(LayoutDictionary::Item wl, myLayouts)
    {
        wl->getWorkspacesContainingTile(tile, outWorkspaces);
    }
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceManager::createUi(Container* parent)
{
    foreach(LayoutDictionary::Item i, myLayouts)
    {
        i->createUi(parent);
    }
}
