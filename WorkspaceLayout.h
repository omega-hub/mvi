#ifndef __WORKSPACE_LAYOUT__
#define __WORKSPACE_LAYOUT__

#include <omega.h>
#include <omegaToolkit.h>

#include "Workspace.h"

using namespace omega;
using namespace omegaToolkit;

class WorkspaceLibrary;

///////////////////////////////////////////////////////////////////////////////
class WorkspaceLayout : public ReferenceType
{
public:
    WorkspaceLayout(const String& name, WorkspaceLibrary* mgr);

    const String& getName() { return myName; }

    //! Create a workspace with area defined by the rectangle containing all 
    //! passed tiles.
    Workspace* createWorkspace(const String& name, const String& icon, const String& tiles);
    //! Create a workspace with area defined by the passes position, width and height.
    //! If any of the passed values is les than 1, values are considered to be normalized
    //! over the full display size. Otherwise, they are interpreted as pixel values.
    Workspace* createWorkspace(const String& name, const String& icon, float x, float y, float w, float h);
    Workspace* findWorkspace(const String& name);

    //! Returns all the workspaces containing the specified tile.
    void getWorkspacesContainingTile(DisplayTileConfig* tile, List<Workspace*>* outWorkspaces);

    void createUi(ui::Container* parent);

    WorkspaceLibrary* getWorkspaceLibrary() { return myManager; }

private:
    String myName;
    typedef Dictionary< String, Ref<Workspace> > WorkspaceDictionary;
    WorkspaceDictionary myWorkspaces;
    WorkspaceLibrary* myManager;
};

#endif