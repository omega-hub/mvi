#include "WorkspaceLayout.h"

#include <omegaToolkit.h>

using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
WorkspaceLayout::WorkspaceLayout(const String& name, WorkspaceLibrary* mgr) :
myName(name), myManager(mgr)
{
}

///////////////////////////////////////////////////////////////////////////////
Workspace* WorkspaceLayout::createWorkspace(const String& name, const String& icon, const String& tiles)
{
    Workspace* w = new Workspace(name, this, icon);
    w->setTiles(tiles);
    myWorkspaces[name] = w;
    return w;
}

///////////////////////////////////////////////////////////////////////////////
Workspace* WorkspaceLayout::createWorkspace(const String& name, const String& icon, float x, float y, float w, float h)
{
    if(x <= 1 || y <= 1 || w <= 1 || h <= 1)
    {
        DisplayConfig& dc = SystemManager::instance()->getDisplaySystem()->getDisplayConfig();
        x *= dc.displayResolution[0];
        w *= dc.displayResolution[0];
        y *= dc.displayResolution[1];
        h *= dc.displayResolution[1];
    }
    Workspace* wk = new Workspace(name, this, icon);
    wk->setWorkspaceRect(Rect((int)x, (int)y, (int)w, (int)h));
    myWorkspaces[name] = wk;
    return wk;
}

///////////////////////////////////////////////////////////////////////////////
Workspace* WorkspaceLayout::findWorkspace(const String& name)
{
    if(myWorkspaces.find(name) == myWorkspaces.end()) return NULL;
    return myWorkspaces[name];
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceLayout::getWorkspacesContainingTile(DisplayTileConfig* tile, List<Workspace*>* outWorkspaces)
{
    foreach(WorkspaceDictionary::Item w, myWorkspaces)
    {
        if(w->containsTile(tile)) outWorkspaces->push_back(w.getValue());
    }
}

///////////////////////////////////////////////////////////////////////////////
void WorkspaceLayout::createUi(Container* parent)
{
    String activeStyle = "alpha: 1.0; scale: 1.0;";
    String inactiveStyle = "alpha: 0.2; scale: 0.8;";

    Vector2f iconSize(16, 16);

    Container* c = Container::create(Container::LayoutHorizontal, parent);
    c->setHorizontalAlign(Container::AlignLeft);
    foreach(WorkspaceDictionary::Item i, myWorkspaces)
    {
        Button* b = Button::create(c);
        b->setIcon(i->getIcon());
        b->getImage()->setSize(iconSize);
        b->setTextEnabled(false);
        b->setActiveStyle(activeStyle);
        b->setInactiveStyle(inactiveStyle);
        b->setStyle(inactiveStyle);
        b->setAutosize(false);
        b->setSize(iconSize);
        b->setUIEventCommand(ostr(
            "WorkspaceLibrary.instance().requestWorkspace('%1%', '%2%')",
            %myName %i->getName()));
    }
}
