from __future__ import absolute_import

import copy

from ..wvrgcal import resultobjects as wvrgcalresults
from pipeline.hif.tasks.common import flaggableviewresults

class WvrgcalflagResult(wvrgcalresults.WvrgcalResult,
  flaggableviewresults.FlaggableViewResults):

    def __init__(self, vis, final=[], pool=[], preceding=[],
      wvrflag=[]):
        wvrgcalresults.WvrgcalResult.__init__(self, vis, final, pool, 
          preceding, wvrflag)
        flaggableviewresults.FlaggableViewResults.__init__(self)
