"""
ASAP plotting class based on matplotlib.
"""

from asap.asaplotbase import *
# Force use of the newfangled toolbar.
import gtk
import matplotlib
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
from matplotlib.backends.backend_gtkagg import FigureManagerGTKAgg
matplotlib.use("GTkAgg")
matplotlib.rcParams['toolbar'] = 'toolbar2'
from matplotlib.backends.backend_gtk import NavigationToolbar2GTK as NavigationToolbar

class asaplotgui(asaplotbase):
    """
    ASAP plotting class based on matplotlib.
    """

    def __init__(self, rows=1, cols=0, title='', size=(8,6), buffering=False):
        """
        Create a new instance of the ASAPlot plotting class.

        If rows < 1 then a separate call to set_panels() is required to define
        the panel layout; refer to the doctext for set_panels().
        """
        v = vars()
        del v['self']

        asaplotbase.__init__(self, **v)
        matplotlib.interactive = True
        self.canvas = FigureCanvas(self.figure)
        # Simply instantiating this is enough to get a working toolbar.
        self.figmgr = FigureManagerGTKAgg(self.canvas, 1)
        def dest_callback(val):
            self.is_dead = True
            self.figmgr.window.destroy()
        self.window = self.figmgr.window
        self.window.connect("destroy", dest_callback )
        self.window.set_title('ASAP Plotter - GTK')
        self.events = {'button_press':None,
                       'button_release':None,
                       'motion_notify':None}

        self.buffering = buffering
        matplotlib.rcParams['interactive'] = True
        #self.canvas.set_size_request(800,600)

        #self.canvas.show()

    def map(self):
        """
        Reveal the ASAPlot graphics window and bring it to the top of the
        window stack.
        """
        self.window.deiconify()
        #self.window.lift()

    def position(self):
        """
        Use the mouse to get a position from a graph.
        """

        def position_disable(event):
            self.register('button_press', None)
            print '%.4f, %.4f' % (event.xdata, event.ydata)

        print 'Press any mouse button...'
        self.register('button_press', position_disable)


    def quit(self):
        """
        Destroy the ASAPlot graphics window.
        """
        self.window.destroy()


    def region(self):
        """
        Use the mouse to get a rectangular region from a plot.

        The return value is [x0, y0, x1, y1] in world coordinates.
        """

        def region_start(event):
            height = self.canvas.figure.bbox.height()
            self.rect = {'fig': None, 'height': height,
                         'x': event.x, 'y': height - event.y,
                         'world': [event.xdata, event.ydata,
                                   event.xdata, event.ydata]}
            self.register('button_press', None)
            self.register('motion_notify', region_draw)
            self.register('button_release', region_disable)

        def region_draw(event):
            self.canvas._tkcanvas.delete(self.rect['fig'])
            self.rect['fig'] = self.canvas._tkcanvas.create_rectangle(
                                self.rect['x'], self.rect['y'],
                                event.x, self.rect['height'] - event.y)

        def region_disable(event):
            self.register('motion_notify', None)
            self.register('button_release', None)

            self.canvas._tkcanvas.delete(self.rect['fig'])

            self.rect['world'][2:4] = [event.xdata, event.ydata]
            print '(%.2f, %.2f)  (%.2f, %.2f)' % (self.rect['world'][0],
                self.rect['world'][1], self.rect['world'][2],
                self.rect['world'][3])

        self.register('button_press', region_start)

        # This has to be modified to block and return the result (currently
        # printed by region_disable) when that becomes possible in matplotlib.

        return [0.0, 0.0, 0.0, 0.0]


    def register(self, type=None, func=None):
        """
        Register, reregister, or deregister events of type 'button_press',
        'button_release', or 'motion_notify'.

        The specified callback function should have the following signature:

            def func(event)

        where event is an MplEvent instance containing the following data:

            name                # Event name.
            canvas              # FigureCanvas instance generating the event.
            x      = None       # x position - pixels from left of canvas.
            y      = None       # y position - pixels from bottom of canvas.
            button = None       # Button pressed: None, 1, 2, 3.
            key    = None       # Key pressed: None, chr(range(255)), shift,
                                  win, or control
            inaxes = None       # Axes instance if cursor within axes.
            xdata  = None       # x world coordinate.
            ydata  = None       # y world coordinate.

        For example:

            def mouse_move(event):
                print event.xdata, event.ydata

            a = asaplot()
            a.register('motion_notify', mouse_move)

        If func is None, the event is deregistered.

        Note that in TkAgg keyboard button presses don't generate an event.
        """

        if not self.events.has_key(type): return

        if func is None:
            if self.events[type] is not None:
                # It's not clear that this does anything.
                self.canvas.mpl_disconnect(self.events[type])
                self.events[type] = None

                # It seems to be necessary to return events to the toolbar.
                if type == 'motion_notify':
                    self.canvas.mpl_connect(type + '_event',
                        self.figmgr.toolbar.mouse_move)
                elif type == 'button_press':
                    self.canvas.mpl_connect(type + '_event',
                        self.figmgr.toolbar.press)
                elif type == 'button_release':
                    self.canvas.mpl_connect(type + '_event',
                        self.figmgr.toolbar.release)

        else:
            self.events[type] = self.canvas.mpl_connect(type + '_event', func)


    def show(self, hardrefresh=True):
        """
        Show graphics dependent on the current buffering state.
        """
        if not self.buffering:
            if hardrefresh:
                asaplotbase.show(self, hardrefresh)
            self.window.deiconify()
            self.canvas.draw()
            self.window.show_all()

    def terminate(self):
        """
        Clear the figure.
        """
        self.window.destroy()

    def unmap(self):
        """
        Hide the ASAPlot graphics window.
        """
        self.window.wm_withdraw()
