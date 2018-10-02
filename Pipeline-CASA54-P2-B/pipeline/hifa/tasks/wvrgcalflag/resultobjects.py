from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.h.tasks.common import flaggableviewresults

LOG = infrastructure.get_logger(__name__)


class WvrgcalflagResults(basetask.Results):

    def __init__(self, vis, flaggerresult=None, too_few_wvr=False,
                 too_few_wvr_post_flagging=False):
        """
        Construct and return a new WvrgcalflagResults.
        """
        super(WvrgcalflagResults, self).__init__()

        self.vis = vis
        self.flaggerresult = flaggerresult
        self.too_few_wvr = too_few_wvr
        self.too_few_wvr_post_flagging = too_few_wvr_post_flagging

    def merge_with_context(self, context):
        # The results from the data task (Wvrgcal) are the only items to
        # consider for acceptance into the context:
        if not self.flaggerresult.dataresult:
            LOG.info("No results from Wvrgcal, nothing to merge.")
        else:
            self.flaggerresult.dataresult.merge_with_context(context)

    def __repr__(self):
        s = 'WvrgcalflagResults:\n'
        if self.flaggerresult.dataresult:
            for calapplication in self.flaggerresult.dataresult.final:
                s += '\tBest caltable for {} is {}\n'.format(
                    os.path.basename(self.vis),
                    calapplication.gaintable)
        else:
            s += '\tNo caltable available for {}\n'.format(
                os.path.basename(self.vis))
        return s


class WvrgcalflagViewResults(flaggableviewresults.FlaggableViewResults):

    def __init__(self, vis):
        """
        Construct and return a new WvrgcalflagViewResults.
        """
        super(WvrgcalflagViewResults, self).__init__()
        self.vis = vis
