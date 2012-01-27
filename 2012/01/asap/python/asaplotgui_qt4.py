"""
ASAP plotting class based on matplotlib.
"""

from asap.asaplotbase import *
import PyQt4 as qt
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg, \
	FigureManagerQTAgg
from matplotlib import _pylab_helpers
# Force use of the newfangled toolbar.
import matplotlib
#matplotlib.use("Qt4Agg")
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

        self.canvas = FigureCanvasQTAgg(self.figure)
        # Simply instantiating this is enough to get a working toolbar.
        self.figmgr = FigureManagerQTAgg(self.canvas, 1)
        self.window = self.figmgr.window
        self._set_window_title('ASAP Plotter - Qt4')
        # register this plot to matplotlib
        _pylab_helpers.Gcf.set_active(self.figmgr)

        #############
        ### DO WE HAVE TO DO SOMETHING FOR WINDOW CLOSE CALL?
        #############
        def dest_callback():
            try:
                self.is_dead = True
            except NameError:
                pass

        qt.QtCore.QObject.connect(self.window, qt.QtCore.SIGNAL('destroyed()'),dest_callback)

	self.events = {'button_press':None,
		       'button_release':None,
		       'motion_notify':None}

	matplotlib.rcParams["interactive"] = True
	self.buffering = buffering

        self.unmap()
	#self.canvas.show()

    def map(self):
	"""
	Reveal the ASAPlot graphics window and bring it to the top of the
	window stack.
	"""
        self.window.activateWindow()
        #To raise this window to the top of the stacking order
        self.window.raise_()
        self.window.show()

    def quit(self):
	"""
	Destroy the ASAPlot graphics window.
	"""
        self.is_dead = True
        try: self.window.close()
        except RuntimeError: pass # the window may already be closed by user

    def show(self, hardrefresh=True):
	"""
	Show graphics dependent on the current buffering state.
	"""
	if not self.buffering:
            if hardrefresh:
                asaplotbase.show(self)
            self.window.activateWindow()
            self.canvas.draw()
	    #self.canvas.show()
	    self.window.show()

    def terminate(self):
	"""
	Clear the figure.
	"""
	self.window.close()

    def unmap(self):
	"""
	Hide the ASAPlot graphics window.
	"""
        self.window.hide()

    def _set_window_title(self,title):
        # Set title to main window title bar
        self.window.setWindowTitle(title)
