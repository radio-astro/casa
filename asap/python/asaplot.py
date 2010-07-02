"""
ASAP plotting class based on matplotlib.
"""

from asap.asaplotbase import *

from matplotlib.backends.backend_agg import FigureCanvasAgg
from matplotlib.backend_bases import FigureManagerBase

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
        self.canvas = FigureCanvasAgg(self.figure)
        self.figmgr = FigureManagerBase(self.canvas,1)
