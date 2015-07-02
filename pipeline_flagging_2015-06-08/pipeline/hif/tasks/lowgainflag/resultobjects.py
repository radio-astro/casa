from __future__ import absolute_import
import collections
import copy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.common import flaggableviewresults

LOG = infrastructure.get_logger(__name__)


class LowgainflagResults(basetask.Results,
  flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new LowgainflagResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

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
