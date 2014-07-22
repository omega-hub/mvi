#include "Workspace.h"
#include "WorkspaceManager.h"

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
void Workspace::requestActivation()
{
    WorkspaceManager* wm = myLayout->getWorkspaceManager();
    wm->requestWorkspace(myLayout->getName(), myName);
}

///////////////////////////////////////////////////////////////////////////////
void Workspace::activate()
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();

    dcfg.setCanvasRect(myRect);

    // First disable all tiles
    //foreach(DisplayConfig::Tile t, dcfg.tiles) t->enabled = false;

    // The enable tiles in this workspace.
    //foreach(DisplayTileConfig* dtc, myTiles) dtc->enabled = true;
}

///////////////////////////////////////////////////////////////////////////////
void Workspace::deactivate()
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();

    dcfg.setCanvasRect(Rect(0, 0, 0, 0));

    // DIsable tiles in this workspace.
    //foreach(DisplayTileConfig* dtc, myTiles) dtc->enabled = false;
}

///////////////////////////////////////////////////////////////////////////////
bool Workspace::containsTile(DisplayTileConfig* tile)
{
    return myRect.intersects(Rect(tile->offset, tile->offset + tile->pixelSize));
    //foreach(DisplayTileConfig* t, myTiles)
    //{
    //    if(t == tile) return true;
    //}
    //return false;
}

///////////////////////////////////////////////////////////////////////////////
bool Workspace::overlaps(Workspace* other)
{
    // Simple case first, are workspaces the same?
    if(other == this) return true;
    //foreach(DisplayTileConfig* t, myTiles)
    //{
    //    if(other->containsTile(t)) return true;
    //}
    return myRect.intersects(other->getWorkspaceRect());
    //return false;
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
