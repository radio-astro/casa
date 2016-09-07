from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.common import flaggableviewresults

LOG = infrastructure.get_logger(__name__)


class FlagcorrectedResults(basetask.Results,
  flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new FlagcorrectedResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'FlagcorrectedResults'
        return s


class FlagcorrectedDataResults(basetask.Results):
    def __init__(self):
        """
        Construct and return a new FlagcorrectedDataResults.
        """
        basetask.Results.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'FlagcorrectedDataResults'
        return s


class FlagcorrectedViewResults(basetask.Results,
  flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new FlagcorrectedViewResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'FlagcorrectedViewResults'
        return s
