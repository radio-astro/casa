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
        def dest_callback():
            self.is_dead = True
            self.window.destroy()

        self.window.protocol("WM_DELETE_WINDOW", dest_callback)
        self.canvas = FigureCanvasTkAgg(self.figure, master=self.window)
        self.canvas.get_tk_widget().pack(side=Tk.TOP, fill=Tk.BOTH, expand=1)
        # Simply instantiating this is enough to get a working toolbar.
        self.figmgr = FigureManagerTkAgg(self.canvas, 1, self.window)
        self.window.wm_title('ASAP Plotter - Tk')

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
        self.window.destroy()

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
