from __future__ import absolute_import

import collections
import copy

from pipeline.hifa.tasks.tsyscal import resultobjects 

import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


class TsysflagspectraResults(resultobjects.TsyscalResults):
    def __init__(self, final=[], pool=[], preceding=[]):
        """
        Construct and return a new TsysflagspectraResults.
        """
        # the results to be merged back into the context
        super(TsysflagspectraResults, self).__init__(final, pool, preceding)

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
        s = super(TsysflagspectraResults, self).__repr__()
        s = 'TsysflagspectraResults incorporating %s' % s
        return s
