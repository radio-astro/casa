from __future__ import absolute_import
import collections
import copy
import os

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from pipeline.hif.tasks.common import commonresultobjects

LOG = logging.get_logger(__name__)


class WvrgcalflagResult(basetask.Results):

    def __init__(self, final=[], pool=[], preceding=[], wvrflag=[]):
        """
        Construct and return a new WvrgcalflagResult.
        """
        super(WvrgcalflagResult, self).__init__()

        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

        # section for qa2 results
        self.qa2 = commonresultobjects.Qa2Result()

        # views and flag operations
        self.flagging = []
        self.wvrflag = wvrflag
        self.view = collections.defaultdict(list)

    def merge_with_context(self, context):
        if not self.final:
            LOG.warning('No results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
              '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def addflags(self, flags):
        self.flagging += flags

    def addview(self, description, viewresult):
        self.view[description].append(viewresult)

    def descriptions(self):
        return self.view.keys()

    def first(self, description):
        return copy.deepcopy(self.view[description][0])

    def flagops(self):
        return copy.deepcopy(self.flagging)

    def last(self, description):
        return copy.deepcopy(self.view[description][-1])

    def __repr__(self):

        # Format the Wvrgcal results.
        s = 'WvrgcalResult:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} field {field} in {vis} is {name}\n'.format(
              spw=calapplication.spw, field=calapplication.field,
              vis=os.path.basename(calapplication.vis),
              name=calapplication.gaintable)
        s += '\twvrflag is {wvrflag}'.format(wvrflag=self.wvrflag)

        return s



