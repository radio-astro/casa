"""
ASAP plotting class based on matplotlib.
"""

from asap.asaplotbase import *

from matplotlib.backends.backend_agg import FigureCanvasAgg
from matplotlib.backend_bases import FigureManagerBase
from matplotlib import _pylab_helpers

class asaplot(asaplotbase):
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
        asaplotbase.__init__(self,**v)
        _pylab_helpers.Gcf.destroy(0)
        self.window = None
        self.canvas = FigureCanvasAgg(self.figure)
        self.figmgr = FigureManagerBase(self.canvas,0)
        # This seems necessary when using Gcf, as this is done in
        # pyplot.figure. Otherwise this can cause atexit errors at cleanup
        
        self.figmgr._cidgcf = None
        _pylab_helpers.Gcf.figs[self.figmgr.num] = self.figmgr

    def quit(self):
        # never dextroy non-GUI plotter.
        pass

    def map(self):
        """ This method is not available in non-GUI plotting class"""
        pass

    def unmap(self):
        """ This method is not available in non-GUI plotting class"""
        pass

