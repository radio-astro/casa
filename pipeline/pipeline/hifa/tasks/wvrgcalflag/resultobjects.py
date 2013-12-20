from __future__ import absolute_import

import copy

from ..wvrgcal import resultobjects as wvrgcalresults

class WvrgcalflagResult(wvrgcalresults.WvrgcalResult):

    def addflags(self, flags):
        self.flagging += flags

    def addview(self, description, viewresult):
        self.view[description].append(viewresult)

    def descriptions(self):
        return self.view.keys()

    def first(self, description):
        return copy.deepcopy(self.view[description][0])

    def flagcmds(self):
        return copy.deepcopy(self.flagging)

    def last(self, description):
        return copy.deepcopy(self.view[description][-1])

