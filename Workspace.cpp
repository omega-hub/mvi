#include "Workspace.h"

#include <omegaToolkit.h>

using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
Workspace::Workspace(const String& name, WorkspaceLayout* layout, const String& icon):
myName(name),
myLayout(layout),
myIconName(icon)
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
            myTiles.push_back(dcfg.tiles[tilename]);
        }
        else
        {
            ofwarn("Workspace::setTiles: could not find tile %1%", %tilename);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void Workspace::activate()
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();

    // First disable all tiles
    foreach(DisplayConfig::Tile t, dcfg.tiles) t->enabled = false;

    // The enable tiles in this workspace.
    foreach(DisplayTileConfig* dtc, myTiles) dtc->enabled = true;
}

///////////////////////////////////////////////////////////////////////////////
void Workspace::deactivate()
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dcfg = ds->getDisplayConfig();

    // DIsable tiles in this workspace.
    foreach(DisplayTileConfig* dtc, myTiles) dtc->enabled = false;
}

///////////////////////////////////////////////////////////////////////////////
bool Workspace::containsTile(DisplayTileConfig* tile)
{
    foreach(DisplayTileConfig* t, myTiles)
    {
        if(t == tile) return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
bool Workspace::overlaps(Workspace* other)
{
    // Simple case first, are workspaces the same?
    if(other == this) return true;
    foreach(DisplayTileConfig* t, myTiles)
    {
        if(other->containsTile(t)) return true;
    }
    return false;
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
