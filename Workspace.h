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