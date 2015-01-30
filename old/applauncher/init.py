# Load the workspace configuration script.
#orun('mvi/config/split2.py')

# App launcher always works with stereo disabled.
getDisplayConfig().forceMono = True

# Set the background picture
backgroundPicture = 'mvi/applauncher/backgrounds/blue-voron.jpg'
queueCommand('setBackground()')

#-------------------------------------------------------------------------------
def setBackground():
    b = Image.create(UiModule.createAndInitialize().getUi())
    i = loadImage(backgroundPicture)
    if(i != None):
        b.setData(i)
        b.tile(True)
        b.setLayer(WidgetLayer.Back)
        b.setSizeAnchor(Vector2(0,0))
        b.setSizeAnchorEnabled(True)
        b.setAutosize(False)
