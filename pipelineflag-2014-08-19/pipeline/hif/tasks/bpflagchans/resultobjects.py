from __future__ import absolute_import
import collections
import copy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


LOG = infrastructure.get_logger(__name__)


class BandpassflagResults(basetask.Results):
    def __init__(self):
        """
        Construct and return a new BandpassflagResults.
        """
        super(BandpassflagResults, self).__init__()
        # views and associated results
        self.flagging = []
        # following are used instead of standard dictionaries so that
        # missing keys are created automatically as needed
        self.view = collections.defaultdict(list)
        self.children = collections.defaultdict(dict)

    def merge_with_context(self, context):
        # do nothing, the tsys cal files should already be in the context
        # and we don't want to insert them twice.
        pass

    def addview(self, description, viewresult):
        self.view[description].append(viewresult)

    def addflags(self, flags):
        self.flagging += flags

    def add_flag_reason_plane(self, flag_reason_plane, flag_reason_key):
        for description in self.descriptions():
            self.view[description][-1].flag_reason_plane = \
              flag_reason_plane[description]
            self.view[description][-1].flag_reason_key = \
              flag_reason_key

    def descriptions(self):
        return self.view.keys()

    def first(self, description):
        return copy.deepcopy(self.view[description][0])

    def flagcmds(self):
        return copy.deepcopy(self.flagging)

    def add_flag_reason_plane(self, flag_reason_plane, flag_reason_key):
        for description in self.descriptions():
            self.view[description][-1].flag_reason_plane = \
              flag_reason_plane[description]
	    self.view[description][-1].flag_reason_key = \
              flag_reason_key

    def flagged(self):
        return len(self.flagging) > 0

    def last(self, description):
        return copy.deepcopy(self.view[description][-1])

    def __repr__(self):
        s = 'BandpassflagResults'
        return s
