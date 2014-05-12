#ifndef __WORKSPACE_MANAGER__
#define __WORKSPACE_MANAGER__

#include <omega.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
class Workspace : public ReferenceType
{
public:
    Workspace(const String& name);

    const String& getName() { return myName; }
    //! Sets the tiles associated to this workspace as a space
    // separated string of tile names.
    void setTiles(const String& tiles);

    void activate();

private:
    String myName;
    Vector<DisplayTileConfig*> myTiles;
};

///////////////////////////////////////////////////////////////////////////////
class WorkspaceManager: public EngineModule
{
public:
    static WorkspaceManager* create();

public:
    WorkspaceManager();

    virtual void initialize();
    virtual void dispose();
    virtual void update(const UpdateContext& context);
    virtual void handleEvent(const Event& evt);
    virtual bool handleCommand(const String& cmd);

    Workspace* createWorkspace(const String& name);
    Workspace* findWorkspace(const String& name);

private:
    Vector<Ref<Workspace> > myWorkspaces;
};

#endif
