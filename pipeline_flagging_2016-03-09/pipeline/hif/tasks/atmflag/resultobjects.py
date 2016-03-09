from __future__ import absolute_import
import collections
import copy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class AtmflagResults(basetask.Results):
    def __init__(self, vis=None):
        """
        Construct and return a new AtmflagResults.
        """
        super(AtmflagResults, self).__init__()
        self.vis = vis

        # views and associated results
        self.flagging = []
        self.view = collections.defaultdict(list)
        self.children = collections.defaultdict(dict)

    def merge_with_context(self, context):
        # do nothing, this stage flags the ms
        pass

    def addview(self, description, viewresult):
        self.view[description].append(viewresult)

    def addflags(self, flags):
        self.flagging += flags

    def descriptions(self):
        return self.view.keys()

    def first(self, description):
        return copy.deepcopy(self.view[description][0])

    def flagcmds(self):
        return copy.deepcopy(self.flagging)

    def flagged(self):
        return len(self.flagging) > 0

    def last(self, description):
        return copy.deepcopy(self.view[description][-1])

    def __repr__(self):
        s = 'AtmflagResults'
        return s
