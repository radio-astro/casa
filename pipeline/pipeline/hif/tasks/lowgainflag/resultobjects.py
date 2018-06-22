from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.h.tasks.common import flaggableviewresults

LOG = infrastructure.get_logger(__name__)


class LowgainflagResults(basetask.Results,
                         flaggableviewresults.FlaggableViewResults):
    def __init__(self, vis=None):
        """
        Construct and return a new LowgainflagResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)
        self.vis = vis

        # list of antennas that should be moved to the end
        # of the refant list
        self.refants_to_demote = set()

    def merge_with_context(self, context):
        # Update reference antennas for MS.
        ms = context.observing_run.get_ms(name=self.vis)
        ms.update_reference_antennas(ants_to_demote=self.refants_to_demote)

    def __repr__(self):
        s = 'LowgainflagResults'
        return s


class LowgainflagDataResults(basetask.Results):
    def __init__(self):
        """
        Construct and return a new LowgainflagDataResults.
        """
        basetask.Results.__init__(self)

    def merge_with_context(self, context):
        # do nothing, none of the gain cals used for the flagging
        # views should be used elsewhere
        pass

    def __repr__(self):
        s = 'LowgainflagDataResults'
        return s


class LowgainflagViewResults(basetask.Results,
                             flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new LowgainflagViewResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'LowgainflagViewResults'
        return s
