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
        matplotlib.rcParams["interactive"] = True

        _pylab_helpers.Gcf.destroy(0)
        self.canvas = FigureCanvasQTAgg(self.figure)
        # Simply instantiating this is enough to get a working toolbar.
        self.figmgr = FigureManagerQTAgg(self.canvas, 0)
        self.window = self.figmgr.window
        self._set_window_title('ASAP Plotter - Qt4')
        # Register this plot to matplotlib without activating it
        #_pylab_helpers.Gcf.set_active(self.figmgr)
        _pylab_helpers.Gcf.figs[self.figmgr.num] = self.figmgr

        #############
        ### DO WE HAVE TO DO SOMETHING FOR WINDOW CLOSE CALL?
        #############
        def dest_callback():
            try:
                self.is_dead = True
            except NameError:
                pass

        qt.QtCore.QObject.connect(self.window, qt.QtCore.SIGNAL('destroyed()'),dest_callback)

        self.unmap()
	#self.canvas.show()

    def map(self):
	"""
	Reveal the ASAPlot graphics window and bring it to the top of the
	window stack.
	"""
        if self.is_dead:
            raise RuntimeError( "No plotter to show. Not yet plotted or plotter is closed." )
        self.window.activateWindow()
        #To raise this window to the top of the stacking order
        self.window.raise_()
        self.window.show()

    def quit(self):
	"""
	Destroy the ASAPlot graphics window.
	"""
        self.is_dead = True
        if not self.figmgr:
            return
        try:
            #self.window.close()
            # TODO destroy casabar
            _pylab_helpers.Gcf.destroy(self.figmgr.num)
            del self.window, self.canvas, self.figmgr
            self.window = None
            self.canvas = None
            self.figmgr = None
        except RuntimeError: pass # the window may already be closed by user

    def show(self, hardrefresh=True):
	"""
	Show graphics dependent on the current buffering state.
	"""
        if self.is_dead:
            raise RuntimeError( "No plotter to show (not yet plotted or closed)." )
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
        if not self.window:
            asaplog.push( "No plotter window to terminate." )
            asaplog.post( "WARN" )
            return
	self.window.close()

    def unmap(self):
	"""
	Hide the ASAPlot graphics window.
	"""
        if not self.window:
            asaplog.push( "No plotter window to unmap." )
            asaplog.post( "WARN" )
            return
        self.window.hide()

    def _set_window_title(self,title):
        # Set title to main window title bar
        self.window.setWindowTitle(title)
