#ifndef __WORKSPACE_MANAGER__
#define __WORKSPACE_MANAGER__

#include <omega.h>
#include <omegaToolkit.h>

#include "WorkspaceLayout.h"

using namespace omega;
using namespace omegaToolkit;

///////////////////////////////////////////////////////////////////////////////
class WorkspaceLibrary : public ReferenceType
{
public:
    static WorkspaceLibrary* create();
    static WorkspaceLibrary* instance();

public:
    WorkspaceLibrary();

    WorkspaceLayout* createLayout(const String& name);
    WorkspaceLayout* findLayout(const String& name);
    Workspace* getWorkspace(const String& layout, const String& name);
    //void setActiveWorkspace(const String& fullname);
    //void requestWorkspace(const String& layout, const String& workspace);

    //! Returns all the workspaces containing the specified tile.
    void getWorkspacesContainingTile(DisplayTileConfig* tile, List<Workspace*>* outWorkspaces);

    //void createUi(ui::Container* parent);

private:
    typedef Dictionary< String, Ref<WorkspaceLayout> > LayoutDictionary;
    LayoutDictionary myLayouts;
    //Ref<Workspace> myActiveWorkspace;
};
#endif