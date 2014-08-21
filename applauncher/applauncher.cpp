#include <omega.h>
#include <omegaToolkit.h>

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

String sMcAddr;

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
};

///////////////////////////////////////////////////////////////////////////////
// Python script used to generate the application file list
const char* listFilesFunction =
    "import os\n" 
    "def listFiles(scriptDir):\n" 
    "   result = ''\n"
    "   for root,dirs,files in os.walk(scriptDir):\n"
    "      for f in files:\n"
    "         if(f.endswith('.oapp')):\n"
    "             result = result + root + '/' + f + ' '\n"
    "   return result\n";
    
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
        
        String mcarg = "";
        if(sMcAddr != "") mcarg = "--mc " + sMcAddr;

        String cmd = ostr("if(isMaster()): olaunch('%1% %2% %3% %4% -I %5%')", 
            %execdir 
            %appfile 
            %ai->args
            %mcarg
            %myAppId
            );
            
       myAppId++;

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

    // Run the python script to obtain the list of files in the script 
    // directory.
    char* fileList = NULL;
    String cmd = ostr("listFiles('%1%/modules')", %ogetdataprefix());
    PythonInterpreter* interpreter = SystemManager::instance()->getScriptInterpreter();
    interpreter->eval(listFilesFunction);
    interpreter->eval(cmd, "z", &fileList);
    omsg(fileList);
    Vector<String> fileVector = StringUtils::split(fileList, " ");
    
    foreach(String file, fileVector)
    {
        addApp(file);
    }
    
    SystemManager::instance()->getDisplaySystem()->setBackgroundColor(Color("black"));
}

///////////////////////////////////////////////////////////////////////////////
// ApplicationBase entry point
int main(int argc, char** argv)
{
    // TODO: Capture the mission control (appmanager) address. Will be used when
    // launching applications to make them connect to the same app manager.
    // for now we set it manually
    sMcAddr = "@localhost";
    
    Application<AppLauncher> app("mvi/applauncher/applauncher");
    return omain(app, argc, argv);
}
