from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class OpcalResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None, opacities=None, spw=None,
                 center_frequencies=None, seasonal_weight=None):

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []
        if opacities is None:
            opacities = []
        if spw is None:
            spw = []
        if center_frequencies is None:
            center_frequencies = []

        super(OpcalResults, self).__init__()
        
        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.opacities = opacities
        self.spw = spw
        self.center_frequencies = center_frequencies
        self.seasonal_weight = seasonal_weight

    def merge_with_context(self, context):
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):
        # Format the Tsyscal results.
        s = 'OpcalResults:\n'
        for calapplication in self.final:
            s += '\tOpacities caltable written to {name}\n'.format(
                    name=calapplication.gaintable)
        return s
