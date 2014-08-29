from __future__ import absolute_import

import copy

from ..wvrgcal import resultobjects as wvrgcalresults

class WvrgcalflagResult(wvrgcalresults.WvrgcalResult):

    def addflags(self, flags):
        self.flagging += flags

    def add_flag_reason_plane(self, flag_reason_plane, flag_reason_key):
        for description in self.descriptions():
            self.view[description][-1].flag_reason_plane = \
              flag_reason_plane[description]
            self.view[description][-1].flag_reason_key = \
              flag_reason_key

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

