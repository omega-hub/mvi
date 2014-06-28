#ifndef __WORKSPACE_LAYOUT__
#define __WORKSPACE_LAYOUT__

#include <omega.h>
#include <omegaToolkit.h>

#include "Workspace.h"

using namespace omega;
using namespace omegaToolkit;

///////////////////////////////////////////////////////////////////////////////
class WorkspaceLayout : public ReferenceType
{
public:
    WorkspaceLayout(const String& name);

    const String& getName() { return myName; }

    Workspace* createWorkspace(const String& name, const String& icon, const String& tiles);
    Workspace* findWorkspace(const String& name);

    //! Returns all the workspaces containing the specified tile.
    void getWorkspacesContainingTile(DisplayTileConfig* tile, List<Workspace*>* outWorkspaces);

    void createUi(ui::Container* parent);

private:
    String myName;
    typedef Dictionary< String, Ref<Workspace> > WorkspaceDictionary;
    WorkspaceDictionary myWorkspaces;
};

#endif