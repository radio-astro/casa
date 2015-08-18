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
from matplotlib import _pylab_helpers

class asaplotgui(asaplotbase):
    """
    ASAP plotting class based on matplotlib.
    """

    def __init__(self, rows=1, cols=0, title='', size=None, buffering=False):
        """
        Create a new instance of the ASAPlot plotting class.

        If rows < 1 then a separate call to set_panels() is required to define
        the panel layout; refer to the doctext for set_panels().
        """
        v = vars()
        del v['self']

        asaplotbase.__init__(self, **v)
        matplotlib.rcParams['interactive'] = True
        matplotlib.interactive = True

        _pylab_helpers.Gcf.destroy(0)
        self.canvas = FigureCanvas(self.figure)
        # Simply instantiating this is enough to get a working toolbar.
        self.figmgr = FigureManagerGTKAgg(self.canvas, 1)
        def dest_callback(val):
            self.is_dead = True
            self.figmgr.window.destroy()
        self.window = self.figmgr.window
        self.window.connect("destroy", dest_callback )
        self.window.set_title('ASAP Plotter - GTK')
        #self.canvas.set_size_request(800,600)
        _pylab_helpers.Gcf.figs[self.figmgr.num] = self.figmgr

        #self.canvas.show()

    def map(self):
        """
        Reveal the ASAPlot graphics window and bring it to the top of the
        window stack.
        """
        if self.is_dead:
            raise RuntimeError( "No plotter to show. Not yet plotted or plotter is closed." )
        self.window.deiconify()
        #self.window.lift()

    def quit(self):
        """
        Destroy the ASAPlot graphics window.
        """
        self.is_dead = True
        if not self.figmgr:
            return
        #self.window.destroy()
        _pylab_helpers.Gcf.destroy(self.figmgr.num)
        del self.window, self.canvas, self.figmgr
        self.window = None
        self.canvas = None
        self.figmgr = None

    def show(self, hardrefresh=True):
        """
        Show graphics dependent on the current buffering state.
        """
        if self.is_dead:
            raise RuntimeError( "No plotter to show (not yet plotted or closed)." )
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
        if not self.window:
            asaplog.push( "No plotter window to terminate." )
            asaplog.post( "WARN" )
            return
        self.window.destroy()

    def unmap(self):
        """
        Hide the ASAPlot graphics window.
        """
        if not self.window:
            asaplog.push( "No plotter window to unmap." )
            asaplog.post( "WARN" )
            return
        self.window.wm_withdraw()
