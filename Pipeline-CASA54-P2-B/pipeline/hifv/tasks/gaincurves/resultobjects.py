from __future__ import absolute_import


import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class GainCurvesResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None):
        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []
        super(GainCurvesResults, self).__init__()

        self.vis = None
        self.pool = pool
        self.final = final
        self.preceding = preceding
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
        # Format the GainCurve results.
        s = 'GainCurvesResults:\n'
        for calapplication in self.final:
            s += '\tGaincurves caltable written to {name}\n'.format(
                    name=calapplication.gaintable)
        return s
