from __future__ import absolute_import
import collections
import copy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.common import flaggableviewresults

LOG = infrastructure.get_logger(__name__)


class BandpassflagResults(basetask.Results, 
  flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new BandpassflagResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        # do nothing, the tsys cal files should already be in the context
        # and we don't want to insert them twice.
        pass

    def __repr__(self):
        s = 'BandpassflagResults'
        return s
