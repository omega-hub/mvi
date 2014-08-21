from omega import *
from omegaToolkit import *
from webView import *
from euclid import *

#-------------------------------------------------------------------------------
view = None
container = None
titleBar = None
frame = None
draggable = False
width = 0
height = 0
    
#-------------------------------------------------------------------------------
def create(width, height):
    parent = UiModule.createAndInitialize().getUi()
    
    global container
    container = Container.create(ContainerLayout.LayoutFree, parent)
    container.setAlpha(1)
    #container.setFillColor(Color('black'))
    #container.setFillEnabled(True)
    container.setAutosize(False)
    container.setMargin(0)
    container.setSize(Vector2(width, height))
    
    global titleBar
    titleBar = Container.create(ContainerLayout.LayoutHorizontal, container)
    titleBar.setAutosize(False)
    titleBar.setSizeAnchorEnabled(True)
    titleBar.setSizeAnchor(Vector2(5, -1))
    titleBar.setPosition(Vector2(0, 0))
    titleBar.setAlpha(1)
    titleBar.setStyleValue('fill', '#000000ff')
    
    global urlbox
    urlbox = TextBox.create(titleBar)
    urlbox.setSizeAnchorEnabled(True)
    urlbox.setSizeAnchor(Vector2(0, -1))
    urlbox.setUIEventCommand('loadUrl("http://%value%")'.format(id))
    urlbox.updateSize()

    titleBar.setHeight(urlbox.getHeight() + titleBar.getMargin() * 2)
    
    global view, frame
    if(isMaster()):
        view = WebView.create(width, height)
        frame = WebFrame.create(container)
        frame.setView(view)
    else:
        # Needs to be RGBA since that's the image format used by WebView.
        view = PixelData.create(width, height, PixelFormat.FormatRgba)
        frame = Image.create(container)
        frame.setData(view)
        
    frame.setPosition(Vector2(5, titleBar.getHeight() + 5))
    frame.setSizeAnchorEnabled(True)
    frame.setSizeAnchor(Vector2(5, 5))
    container.requestLayoutRefresh()
    
    ImageBroadcastModule.instance().addChannel(view, 'browser', ImageFormat.FormatNone)

#-------------------------------------------------------------------------------
def loadUrl(url):
    print('loading ' + url)
    if(isMaster()):
        view.loadUrl(url)

#-------------------------------------------------------------------------------
def resize(width, height):
    view.resize(width, height)
    container.setSize(Vector2(width, height))
    titleBar.setWidth(width)

#-------------------------------------------------------------------------------
def onCanvasChanged():
    global width, height
    cr = getDisplayConfig().getCanvasRect()
    w = cr[2]
    h = cr[3]
    if(w != width or h != height):
        width = w
        height = h
        resize(w - 5, h - 5)
getDisplayConfig().canvasChangedCommand = 'onCanvasChanged()'
    
#-------------------------------------------------------------------------------
#setClearColor(Color('black'))
create(400,400)
loadUrl('http://youtube.com')
onCanvasChanged()

