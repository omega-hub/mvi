#ifndef __WORKSPACE_MANAGER__
#define __WORKSPACE_MANAGER__

#include <omega.h>
#include <omegaToolkit.h>

using namespace omega;
using namespace omegaToolkit;

class WorkspaceLayout;

///////////////////////////////////////////////////////////////////////////////
class Workspace : public ReferenceType
{
friend class WorkspaceAllocator;
public:
    Workspace(const String& name, WorkspaceLayout* layout, const String& icon);

    const String& getName() { return myName; }
    //! Sets the tiles associated to this workspace as a space
    // separated string of tile names.
    void setTiles(const String& tiles);

    void activate();
    void deactivate();

    bool containsTile(DisplayTileConfig*);

    //! Returns true if workspaces have overlapping tiles.
    bool overlaps(Workspace* other);

    int getNumTiles() { return myTiles.size(); }
    WorkspaceLayout* getLayout() { return myLayout; }

    PixelData* getIcon();

private:
    WorkspaceLayout* myLayout;
    String myName;
    Vector<DisplayTileConfig*> myTiles;

    String myIconName;
    Ref<PixelData> myIcon;
};

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

///////////////////////////////////////////////////////////////////////////////
class WorkspaceManager : public ReferenceType
{
public:
    static WorkspaceManager* create();
    static WorkspaceManager* instance();

public:
    WorkspaceManager();

    WorkspaceLayout* createLayout(const String& name);
    WorkspaceLayout* findLayout(const String& name);
    Workspace* getWorkspace(const String& layout, const String& name);
    void setActiveWorkspace(const String& fullname);
    void requestWorkspace(const String& layout, const String& workspace);

    //! Returns all the workspaces containing the specified tile.
    void getWorkspacesContainingTile(DisplayTileConfig* tile, List<Workspace*>* outWorkspaces);

    void createUi(ui::Container* parent);

private:
    typedef Dictionary< String, Ref<WorkspaceLayout> > LayoutDictionary;
    LayoutDictionary myLayouts;
    Ref<Workspace> myActiveWorkspace;
};
#endif