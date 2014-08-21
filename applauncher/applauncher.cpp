#include <omega.h>
#include <omegaToolkit.h>

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
struct AppInfo : public ReferenceType
{
public:
    String command;
    String file;
    String label;
    String id;
    String iconFile;
    String group;
    String args;
};

///////////////////////////////////////////////////////////////////////////////
class AppLauncher : public EngineModule
{
public:
    AppLauncher() : EngineModule("AppLauncher") { }
    virtual void initialize();

private:
    void addApp(const String& appfile);
    PixelData* getOrCreateIcon(const String& iconfile);
    Menu*   getOrCreateGroup(const String& groupname);

private:
    unsigned int myAppId;
    Ref<MenuManager> myMenuManager;
    Ref<Menu> myRootMenu;

    Dictionary<String, Ref<PixelData> > myIcons;

    Dictionary<String, Ref<Menu> > myGroups;

    List< Ref<AppInfo> > myApps;

    // Host:port of the app manager (if we are using one).
    String myAppMgrHost;
};

///////////////////////////////////////////////////////////////////////////////
PixelData* AppLauncher::getOrCreateIcon(const String& iconfile)
{
    if(myIcons.find(iconfile) != myIcons.end()) return myIcons[iconfile];

    Ref<PixelData> icon = ImageUtils::loadImage(iconfile);
    myIcons[iconfile] = icon;
    return icon;
}

///////////////////////////////////////////////////////////////////////////////
Menu* AppLauncher::getOrCreateGroup(const String& groupname)
{
    if(myGroups.find(groupname) != myGroups.end()) return myGroups[groupname];

    // Create new menu.
    Menu* root = myMenuManager->getMainMenu();
    Menu* group = root->addSubMenu(groupname);
    myGroups[groupname] = group;
    return group;
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::addApp(const String& appfile)
{
    AppInfo* ai = new AppInfo();

    Config* cfg = new Config(appfile);
    if(cfg->load())
    {
        if(cfg->exists("config/app"))
        {
            Setting& s = cfg->lookup("config/app");
            ai->iconFile = Config::getStringValue("icon", s, "mvi/apps/icon.png");
            ai->group = Config::getStringValue("group", s, "root");
            ai->label = Config::getStringValue("label", s, cfg->getFilename());
            ai->args = Config::getStringValue("args", s, "");

            // For the icon file, replace ./ with the.
            String cfgdir;
            String cfgfilename;
            String fullpath;
            DataManager::findFile(appfile, fullpath);
            StringUtils::splitFilename(fullpath, cfgfilename, cfgdir);
            ai->iconFile = StringUtils::replaceAll(ai->iconFile, "./", cfgdir);
        }
        else
        {
            ofwarn("%1%: missing app section, app will be ignored.", %appfile);
            return;
        }

        String execpath = ogetexecpath();
        String execname;
        String execdir;
        StringUtils::splitFilename(execpath, execname, execdir);
        execdir = execdir + "orun";

        myApps.push_back(ai);
        Menu* mnu = getOrCreateGroup(ai->group);

        String cmd = ostr("olaunch('%1% %2% %3%')", %execdir %appfile %ai->args);

        MenuItem* mi = mnu->addButton(
            ai->label, 
            cmd);

        // FOrce a layout to get the label size.
        mi->getButton()->getLabel()->autosize();
        int h = mi->getButton()->getLabel()->getHeight();

        PixelData* icon = getOrCreateIcon(ai->iconFile);
        if(icon != NULL)
        {
            mi->setImage(getOrCreateIcon(ai->iconFile));
            mi->getImage()->setSize(Vector2f(h * 2, h * 2));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::initialize()
{
    myAppId = 0;

    myMenuManager = MenuManager::createAndInitialize();
    Menu* m = myMenuManager->createMenu("Root");

    m->addLabel("Application Launcher");
    myGroups["root"] = m;

    myMenuManager->setMainMenu(m);

    addApp("mvi/apps/spincube.oapp");
    addApp("mvi/apps/browser.oapp");

    SystemManager::instance()->getDisplaySystem()->setBackgroundColor(Color("black"));
}

///////////////////////////////////////////////////////////////////////////////
// ApplicationBase entry point
int main(int argc, char** argv)
{
    Application<AppLauncher> app("mvi/applauncher/applauncher");
    return omain(app, argc, argv);
}
