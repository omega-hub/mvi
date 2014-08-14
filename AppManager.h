#ifndef __WORKSPACE_ALLOCATOR__
#define __WORKSPACE_ALLOCATOR__

#include <omega.h>
#include <omegaToolkit.h>

#include "WorkspaceLibrary.h"

using namespace omega;
using namespace omegaToolkit;

class WorkspaceLayout;
class Workspace;

///////////////////////////////////////////////////////////////////////////////
class AppManager : public EngineModule
{
public:
    static AppManager* instance();
    static AppManager* create();

public:
    AppManager();

    void initialize();
    void handleEvent(const Event& evt);

    //void setAppWorkspace(const String& appid, Workspace* w);
    //! Find the largest workspace that could be allocated, containing the 
    //! specified pixel.
    //Workspace* findFreeWorkspace(int x, int y);
    //bool canAllocate(Workspace* w, const String& client);

    //void requestWorkspace(const String& client, const String& layout, const String& workspace);
    //void allocateWorkspace(Workspace* w, const String& client);
    //void freeWorkspace(const String& client);
    int allocateAppId();

    void onAppCanvasChange(const String& appid, int x, int y, int w, int h);

private:
    void computeAppCanvases();
    void updateAppCanvases();

    //void updateLocalActiveTiles();

private:
    int myAppId;

    Ref<MissionControlClient> myMC;

    //WorkspaceLibrary* myWorkspaceLibrary;
    //typedef Dictionary<String, Workspace*> WorkspaceDictionary;
    //WorkspaceDictionary myAllocatedWorkspaces;

    struct AppInstance: public ReferenceType
    {
        String id;
        Rect targetCanvas;
        Rect currentCanvas;
        int z;
        bool dirtyCanvas;
    };
    typedef Dictionary<String, Ref<AppInstance> > AppInstanceDictionary;
    AppInstanceDictionary myAppInstances;

    typedef List< Ref<AppInstance> > AppInstanceList;
    AppInstanceList myZSortedAppInstances;
    unsigned int myCurrentTopZ;
};
#endif
