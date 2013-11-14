from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class SwpowcalResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(SwpowcalResults, self).__init__()
        
        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):

	# Format the Swpowcal results.
        s = 'SwpowResults:\n'
        for calapplication in self.final:
            s += '\tBest switched power (swpow) priorcals caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
	return s
