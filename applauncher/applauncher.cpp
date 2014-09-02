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
class AppLauncher: public EngineModule, public IMissionControlListener
{
public:
    AppLauncher() : EngineModule("AppLauncher") { omegaToolkitPythonApiInit(); }
    virtual void initialize();

private:
    void addApp(const String& appfile);
    PixelData* getOrCreateIcon(const String& iconfile);
    Menu*   getOrCreateGroup(const String& groupname);

    void onClientConnected(const String& str);
    void onClientDisconnected(const String& str);

    void handleEvent(const Event& evt);
    void update(const UpdateContext& ctx);

private:
    unsigned int myAppId;

    Dictionary<String, Ref<PixelData> > myIcons;

    Dictionary<String, Ref<Menu> > myGroups;

    List< Ref<AppInfo> > myApps;

    Ref<Button> myLaunchingItem;
    float myLaunchingItemAnim;

    Container* myGroup;

    Dictionary<int, String> myLaunchCommands;
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
// Menu* AppLauncher::getOrCreateGroup(const String& groupname)
// {
    // if(myGroups.find(groupname) != myGroups.end()) return myGroups[groupname];

    // // Create new menu.
    // Menu* root = myMenuManager->getMainMenu();
    // Menu* group = root->addSubMenu(groupname);
    // myGroups[groupname] = group;
    // return group;
// }

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

        String appname;
        String appdir;
        StringUtils::splitFilename(appfile, appname, appdir);

        myApps.push_back(ai);

        String mcarg = "";
        if(sMcAddr != "") mcarg = "--mc " + sMcAddr;

        String cmd = ostr("olaunch('%1% %2% %3% %4% -N %5%-$appid$ -I $appid$ $canvasdef$')", 
            %execdir 
            %appfile 
            %ai->args
            %mcarg
            %appname
            );
            
        Button* b = Button::create(myGroup);
        b->setText(ai->label);
        //mi->getWidget()->setUIEventHandler(this);
        b->setUIEventHandler(this);
        myLaunchCommands[b->getId()] = cmd;

        myGroup->setSizeAnchorEnabled(true);
        myGroup->setSizeAnchor(Vector2f(0, 0));
        myGroup->setAutosize(false);

        // FOrce a layout to get the label size.
        b->getLabel()->autosize();
        int h = b->getLabel()->getHeight();

        PixelData* icon = getOrCreateIcon(ai->iconFile);
        if(icon != NULL)
        {
            b->setIcon(getOrCreateIcon(ai->iconFile));
            b->getImage()->setSize(Vector2f(h * 2, h * 2));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::initialize()
{
    myAppId = 1;

    // Register myself as a mission control listener
    if(SystemManager::instance()->isMaster())
    {
        MissionControlClient* mcc = SystemManager::instance()->getMissionControlClient();
        oassert(mcc);
        mcc->setListener(this);
        mcc->postCommand(ostr(
            "@server:"
            "AppManager.instance().setLauncherApp('%1%')",
            %mcc->getName()));
    }

    myGroup = Container::create(Container::LayoutGridHorizontal, UiModule::instance()->getUi());
    myGroup->setGridColumns(4);
    myGroup->setGridRows(4);

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
void AppLauncher::handleEvent(const Event& evt)
{
    if(evt.getType() == Event::Click)
    {
        // Save the current button, so we can animate it while we wait for the 
        // app to start.
        myLaunchingItem = Widget::getSource<Button>(evt);
        myLaunchingItemAnim = 5.0f;
        PythonInterpreter* i = SystemManager::instance()->getScriptInterpreter();

        DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
        DisplayConfig& dc = ds->getDisplayConfig();
        
        if(SystemManager::instance()->isMaster())
        {
            String cmd = myLaunchCommands[myLaunchingItem->getId()];
            // Run the launch command for this button.
            // Add the canvas definition to the command line
            Rect c = dc.getCanvasRect();

            String canvasDef = ostr("-w %1%,%2%,%3%,%4%", %c.x() %c.y() %c.width() %c.height());
            cmd = StringUtils::replaceAll(cmd, "$canvasdef$", canvasDef);

            cmd = StringUtils::replaceAll(cmd, "$appid$", ostr("%1%", %myAppId));
            myAppId++;

            ofmsg("Launching %1%", %cmd);
            
            i->queueCommand(cmd, true);
            
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::update(const UpdateContext& ctx)
{
    if(myLaunchingItem != NULL)
    {
        myLaunchingItemAnim -= ctx.dt;
        myLaunchingItem->getImage()->setScale(1.0f + Math::abs(Math::cos(5.0f - myLaunchingItemAnim) * 0.5));
        if(myLaunchingItemAnim <= 0) myLaunchingItem = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::onClientConnected(const String& str)
{
}

///////////////////////////////////////////////////////////////////////////////
void AppLauncher::onClientDisconnected(const String& str)
{
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
