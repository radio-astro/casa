from __future__ import absolute_import

import copy

import pipeline.infrastructure as infrastructure
from pipeline.hif.tasks.bandpass import common

LOG = infrastructure.get_logger(__name__)


class BandpassflagResults(common.BandpassResults):

    def __init__(self):
        super(BandpassflagResults, self).__init__()
        self.flagging = []

    def import_bpresult(self, bpresult):
        self.final = copy.deepcopy(bpresult.final)
        self.preceding = copy.deepcopy(bpresult.preceding)
        self.pool = copy.deepcopy(bpresult.pool)
        self.error = copy.deepcopy(bpresult.error)
        self.qa = copy.deepcopy(bpresult.qa)

    def import_cafresult(self, cafresult):
        self.flagging = cafresult.flagcmds()

    def flagcmds(self):
        return copy.deepcopy(self.flagging)

    def __repr__(self):
        s = 'BandpassflagResults'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s
