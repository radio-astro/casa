from __future__ import absolute_import

import copy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class CorrectedampflagResults(basetask.Results):

    def __init__(self):
        super(CorrectedampflagResults, self).__init__()
        self.flagging = []

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'CorrectedampflagResults'
        return s

    def addflags(self, flags):
        self.flagging += flags

    def flagcmds(self):
        return copy.deepcopy(self.flagging)
