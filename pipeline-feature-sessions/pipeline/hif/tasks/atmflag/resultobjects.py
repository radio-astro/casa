from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.h.tasks.common import flaggableviewresults

LOG = infrastructure.get_logger(__name__)


class AtmflagResults(basetask.Results,
                     flaggableviewresults.FlaggableViewResults):
    def __init__(self, vis=None):
        """
        Construct and return a new AtmflagResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)
        self.vis = vis

    def merge_with_context(self, context):
        # do nothing, this stage flags the ms
        pass

    def __repr__(self):
        s = 'AtmflagResults'
        return s


class AtmflagDataResults(basetask.Results):
    def __init__(self, vis=None):
        """
        Construct and return a new AtmflagDataResults.
        """
        basetask.Results.__init__(self)
        self.vis = vis

    def merge_with_context(self, context):
        # do nothing, Atmflag does not produce any extra tables that need to
        # be merged.
        pass

    def __repr__(self):
        s = 'AtmflagDataResults'
        return s


class AtmflagViewResults(basetask.Results,
                         flaggableviewresults.FlaggableViewResults):
    def __init__(self, vis=None):
        """
        Construct and return a new AtmflagViewResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)
        self.vis = vis

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'AtmflagViewResults'
        return s
