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
        self.window = Tk.Tk()
        #def dest_callback():
        #    print "dest_callback"
        #    self.is_dead = True
        #    self.window.destroy()

        self.window.protocol("WM_DELETE_WINDOW", self.quit)
        self.canvas = FigureCanvasTkAgg(self.figure, master=self.window)
        self.canvas.get_tk_widget().pack(side=Tk.TOP, fill=Tk.BOTH, expand=1)
        # Simply instantiating this is enough to get a working toolbar.
        self.figmgr = FigureManagerTkAgg(self.canvas, 0, self.window)
        # Register this plot to matplotlib without activating it
        #_pylab_helpers.Gcf.set_active(self.figmgr)
        _pylab_helpers.Gcf.figs[self.figmgr.num] = self.figmgr
        self._set_window_title('ASAP Plotter - Tk')

	self.events = {'button_press':None,
		       'button_release':None,
		       'motion_notify':None}

	matplotlib.rcParams["interactive"] = True
	#self.buffering = buffering

	self.canvas.show()

    def map(self):
        """
        Reveal the ASAPlot graphics window and bring it to the top of the
        window stack.
        """
        self.window.wm_deiconify()
        self.window.lift()

    def quit(self):
        """
        Destroy the ASAPlot graphics window.
        """
        self.is_dead = True
        #self.window.destroy()
        _pylab_helpers.Gcf.destroy(self.figmgr.num)
        del self.window, self.canvas
        self.window = None
        self.canvas = None
        

    def show(self, hardrefresh=True):
        """
        Show graphics dependent on the current buffering state.
        """
        if not self.buffering:
            if hardrefresh:
                asaplotbase.show(self)
            self.window.wm_deiconify()
            self.canvas.show()

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

    def _set_window_title(self,title):
        # Set title to main window title bar
        self.window.wm_title(title)
