from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.h.tasks.common import flaggableviewresults

LOG = infrastructure.get_logger(__name__)


class RawflagchansResults(basetask.Results,
                          flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new RawflagchansResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'RawflagchansResults'
        return s


class RawflagchansDataResults(basetask.Results):
    def __init__(self):
        """
        Construct and return a new RawflagchansDataResults.
        """
        basetask.Results.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'RawflagchansDataResults'
        return s


class RawflagchansViewResults(basetask.Results,
                              flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new RawflagchansViewResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'RawflagchansViewResults'
        return s
