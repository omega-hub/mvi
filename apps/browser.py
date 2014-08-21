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

# Scale, max width, max height
S = Platform.scale
MW = 840
MH = 480
scale = S
    
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
    #frame.setSizeAnchorEnabled(True)
    #frame.setSizeAnchor(Vector2(5, 5))
    container.requestLayoutRefresh()
    
    ImageBroadcastModule.instance().addChannel(view, 'browser', ImageFormat.FormatNone)

#-------------------------------------------------------------------------------
def loadUrl(url):
    print('loading ' + url)
    if(isMaster()):
        view.loadUrl(url)

#-------------------------------------------------------------------------------
def rescale(newscale):
    global scale
    scale = newscale
    resize(width - 5, height - 5 - titleBar.getHeight())
    
#-------------------------------------------------------------------------------
def resize(width, height):
    sw = float(width) / scale
    sh = float(height) / scale
    view.resize(int(sw), int(sh))
    container.setSize(Vector2(sw, sh))
    
    ox = (width - sw) / 2
    oy = (height - sh) / 2
    frame.setSize(Vector2(sw, sh))
    frame.setScale(scale)
    frame.setPosition(Vector2(5 + ox, titleBar.getHeight() + 5 + oy))
    titleBar.setWidth(width)

#-------------------------------------------------------------------------------
def onCanvasChanged():
    global width, height, scale
    cr = getDisplayConfig().getCanvasRect()
    w = cr[2]
    h = cr[3]
    if(w != width or h != height):
        if(w > MW or h > MH):
            if(w > h):
                scale = S * w / MW
            else:
                scale = S * h / MH
        else:
            scale = S
        width = w
        height = h
        rescale(scale)
getDisplayConfig().canvasChangedCommand = 'onCanvasChanged()'
    
#-------------------------------------------------------------------------------
#setClearColor(Color('black'))
create(400,400)
loadUrl('http://youtube.com')
onCanvasChanged()

