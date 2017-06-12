from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.bandpass import common
from pipeline.hif.tasks.correctedampflag import resultobjects

LOG = infrastructure.get_logger(__name__)


class BandpassflagResults(basetask.Results):

    def __init__(self):
        super(BandpassflagResults, self).__init__()
        self.bpresult = common.BandpassResults()
        self.cafresult = resultobjects.CorrectedampflagResults()
        self.plots = {}

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.api.Results.merge_with_context`
        """
        if not self.bpresult.final:
            LOG.error('No results to merge')
            return

        for calapp in self.bpresult.final:
            LOG.debug('Adding calibration to callibrary:\n%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):
        s = 'BandpassflagResults'
        for calapplication in self.bpresult.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s
