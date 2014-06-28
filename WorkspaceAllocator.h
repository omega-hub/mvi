#ifndef __WORKSPACE_ALLOCATOR__
#define __WORKSPACE_ALLOCATOR__

#include <omega.h>
#include <omegaToolkit.h>

#include "WorkspaceManager.h"

using namespace omega;
using namespace omegaToolkit;

class WorkspaceLayout;
class Workspace;

///////////////////////////////////////////////////////////////////////////////
class WorkspaceAllocator : public ReferenceType
{
public:
    static WorkspaceAllocator* instance();

public:
    WorkspaceAllocator(WorkspaceManager* wm);

    void setAppWorkspace(const String& appid, Workspace* w);
    //! Find the largest workspace that could be allocated, containing the 
    //! specified pixel.
    Workspace* findFreeWorkspace(int x, int y);
    bool canAllocate(Workspace* w, const String& client);

    void requestWorkspace(const String& client, const String& layout, const String& workspace);
    void allocateWorkspace(Workspace* w, const String& client);
    void freeWorkspace(const String& client);
    int allocateAppId();

private:
    void updateLocalActiveTiles();

private:
    int myAppId;
    WorkspaceManager* myWorkspaceManager;
    typedef Dictionary<String, Workspace*> WorkspaceDictionary;
    WorkspaceDictionary myAllocatedWorkspaces;
};
#endif
