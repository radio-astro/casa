"""
ASAP plotting class based on matplotlib.
"""

from asap.asaplotbase import *
import Tkinter as Tk
import matplotlib
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, \
	FigureManagerTkAgg
# Force use of the newfangled toolbar.
matplotlib.rcParams['toolbar'] = 'toolbar2'
from matplotlib import _pylab_helpers
from asap.logging import asaplog, asaplog_post_dec

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
        #matplotlib.rcParams["interactive"] = True

        _pylab_helpers.Gcf.destroy(0)
        self.window = Tk.Tk()
        self.window.protocol("WM_DELETE_WINDOW", self.quit)
        self.canvas = FigureCanvasTkAgg(self.figure, master=self.window)
        self.canvas.get_tk_widget().pack(side=Tk.TOP, fill=Tk.BOTH, expand=1)
        # Simply instantiating this is enough to get a working toolbar.
        self.figmgr = FigureManagerTkAgg(self.canvas, 0, self.window)
        self.figmgr._cidgcf = None
        # Register this plot to matplotlib without activating it
        #_pylab_helpers.Gcf.set_active(self.figmgr)
        _pylab_helpers.Gcf.figs[self.figmgr.num] = self.figmgr
        self._set_window_title('ASAP Plotter - Tk')
        self.canvas.show()


    def map(self):
        """
        Reveal the ASAPlot graphics window and bring it to the top of the
        window stack.
        """
        if self.is_dead:
            raise RuntimeError( "No plotter to show. Not yet plotted or plotter is closed." )
        self.window.wm_deiconify()
        self.window.lift()

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
                asaplotbase.show(self)
            self.window.wm_deiconify()
            self.canvas.show()

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

    def _set_window_title(self,title):
        # Set title to main window title bar
        self.window.wm_title(title)
