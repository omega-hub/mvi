from mvi import *

# Create App drawer
launcher = AppLauncher.create()
launcher.setIconSize(64)

# Setup apps
app = AppInfo()
app.file = "cyclops/examples/python/spincube.py"
app.iconFile = "app.png"
app.label = "SpinCube"
app.id = "spincube"
launcher.addApp(app)

app = AppInfo()
app.file = "cyclops/examples/python/fallingBricks.py"
app.iconFile = "app.png"
app.label = "Falling Bricks"
app.id = "fallingBricks"
launcher.addApp(app)

app = AppInfo()
app.file = "cyclops/examples/python/planes.py"
app.iconFile = "app.png"
app.label = "Planes"
app.id = "planes"
launcher.addApp(app)

# Set background
backgroundImage = loadImage('mvi/backgrounds/blue-texture-wallpaper.jpg')
ui = UiModule.createAndInitialize().getUi()
bkg = Image.create(ui)
bkg.setAutosize(False)
bkg.setData(backgroundImage)
#bkg.setSize(ui.getSize())
    