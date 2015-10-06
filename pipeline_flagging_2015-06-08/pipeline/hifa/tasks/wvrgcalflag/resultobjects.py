from __future__ import absolute_import

from ..wvrgcal import resultobjects as wvrgcalresults
from pipeline.hif.tasks.common import flaggableviewresults

class WvrgcalflagResult(wvrgcalresults.WvrgcalResult,
                        flaggableviewresults.FlaggableViewResults):

    def __init__(self, vis, final=None, pool=None, preceding=None,
                 wvrflag=None):
        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []
        if wvrflag is None:
            wvrflag = []

        wvrgcalresults.WvrgcalResult.__init__(self, vis, final, pool,
                                              preceding, wvrflag)
        flaggableviewresults.FlaggableViewResults.__init__(self)
