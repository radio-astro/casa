from __future__ import absolute_import

import collections
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
from pipeline.h.tasks.common import commonresultobjects

LOG = infrastructure.get_logger(__name__)


class WvrgcalResult(basetask.Results):

    def __init__(self, vis, final=None, pool=None, preceding=None,
                 wvrflag=None):
        """
        Construct and return a new WvrgcalResult.
        """
        super(WvrgcalResult, self).__init__()

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []
        if wvrflag is None:
            wvrflag = []

        self.vis = vis
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

        # section for qa results
        self.qa_wvr = commonresultobjects.QaResult()

        # results used to calculate the qa results
        self.qa_wvr.bandpass_result = None
        self.qa_wvr.nowvr_result = None
        self.qa_wvr.qa_spw = None

        # views and flag operations
        self.flagging = []
        self.wvrflag = wvrflag
        self.view = collections.defaultdict(list)
        
        # record wvrgcal tie arguments for weblog 
        self.tie = ''

    def merge_with_context(self, context):
        if not self.final:
            LOG.info('No results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

        if self.wvrflag:
            ms = context.observing_run.get_ms(name=self.vis)
            if (hasattr(ms, 'reference_antenna')
                    and isinstance(ms.reference_antenna, str)):
                refant = ms.reference_antenna.split(',')
                bad_antennas = set(self.wvrflag).intersection(refant)
                if bad_antennas:
                    ant_msg = utils.commafy(bad_antennas, quotes=False,
                                            multi_prefix='s')
                    LOG.warning('Antenna%s with bad WVR removed from refant list for '
                                '%s' % (ant_msg, os.path.basename(self.vis)))
                    for antenna in list(bad_antennas):
                        refant.remove(antenna)
                    ms.reference_antenna = ','.join(refant)

    def __repr__(self):

        # Format the Wvrgcal results.
        s = 'WvrgcalResult:\n'
        if not self.final:
            s += '\tNo wvr caltables will be applied\n'

        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} field {field} in {vis} is {name}\n'.format(
              spw=calapplication.spw, field=calapplication.field,
              vis=os.path.basename(calapplication.vis),
              name=calapplication.gaintable)
        s += '\twvrflag is {wvrflag}'.format(wvrflag=self.wvrflag)

        return s
