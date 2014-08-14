#ifndef __WORKSPACE__
#define __WORKSPACE__

#include <omega.h>
#include <omegaToolkit.h>

using namespace omega;
using namespace omegaToolkit;

class WorkspaceLayout;

///////////////////////////////////////////////////////////////////////////////
class Workspace : public ReferenceType
{
friend class AppManager;
public:
    // Public fields (accessible from python)
    String onActivated;

public:
    Workspace(const String& name, WorkspaceLayout* layout, const String& icon);

    const String& getName() { return myName; }
    //! Sets the tiles associated to this workspace as a space
    //! separated string of tile names. The list of tiles is used to compute
    //! the size and position of this workspace.
    void setTiles(const String& tiles);

    //! Sets the position and size of this workspace in pixels
    void setWorkspaceRect(const Rect& r) { myRect = r;  }


    //! Requests activation of this workspace. This call returns immediately.
    //! If activation succeeds (i.e. the workspace area is not allocated to
    //! other applications), the workspace will be activated at a later time
    //! Immediately activates the workspace.
    //void requestActivation();

    //void activate();
    //! Immediately deactivated the workspace.
    //void deactivate();

    bool containsTile(DisplayTileConfig*);

    //! Returns true if workspaces have overlapping tiles.
    bool overlaps(Workspace* other);

    //int getNumTiles() { return myTiles.size(); }
    WorkspaceLayout* getLayout() { return myLayout; }

    PixelData* getIcon();

    const Rect& getWorkspaceRect() { return myRect; }

private:
    WorkspaceLayout* myLayout;
    String myName;
    Rect myRect;
    //Vector<DisplayTileConfig*> myTiles;

    String myIconName;
    Ref<PixelData> myIcon;
};
#endif