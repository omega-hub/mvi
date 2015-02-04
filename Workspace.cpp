#include "Workspace.h"
#include "WorkspaceLibrary.h"

#include <omegaToolkit.h>

using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
Workspace::Workspace(const String& name, WorkspaceLayout* layout, const String& icon):
myName(name),
myLayout(layout),
myIconName(icon),
myRect(Vector2i(100000,100000),Vector2i::Zero())
{
}

///////////////////////////////////////////////////////////////////////////////
void Workspace::setWorkspaceRect(const Rect& r) 
{ 
    ofmsg("Workspace: %1% -- %2% %3%", %myName %r.min %r.max);
    myRect = r;  
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
            DisplayTileConfig* dtc = dcfg.tiles[tilename];
            // Expand the workspace rect to include the tile rect.
            myRect.min = myRect.min.cwiseMin(dtc->offset);
            myRect.max = myRect.max.cwiseMax(dtc->offset + dtc->pixelSize);
        }
        else
        {
            ofwarn("Workspace::setTiles: could not find tile %1%", %tilename);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
bool Workspace::containsTile(DisplayTileConfig* tile)
{
    return myRect.intersects(Rect(tile->offset, tile->offset + tile->pixelSize));
}

///////////////////////////////////////////////////////////////////////////////
bool Workspace::overlaps(Workspace* other)
{
    // Simple case first, are workspaces the same?
    if(other == this) return true;
    return myRect.intersects(other->getWorkspaceRect());
}

///////////////////////////////////////////////////////////////////////////////
PixelData* Workspace::getIcon()
{ 
    if(myIcon == NULL && myIconName != "")
    {
        myIcon = ImageUtils::loadImage(myIconName);
        if(myIcon == NULL)
        {
            ofwarn("Workspace::getIcon: could not create icon %1%.", %myIconName);
        }
    }
    return myIcon; 
}
