from __future__ import absolute_import

import collections
import copy

from pipeline.hifa.tasks.tsyscal import resultobjects 
from pipeline.hif.tasks.common import flaggableviewresults

import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


class TsysflagspectraResults(resultobjects.TsyscalResults,
  flaggableviewresults.FlaggableViewResults):
    def __init__(self, final=[], pool=[], preceding=[]):
        """
        Construct and return a new TsysflagspectraResults.
        """
        # the results to be merged back into the context
        resultobjects.TsyscalResults.__init__(self, final, pool, preceding)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        # do nothing, the tsys cal files should already be in the context
        # and we don't want to insert them twice.
        pass

    def __repr__(self):
        s = super(TsysflagspectraResults, self).__repr__()
        s = 'TsysflagspectraResults incorporating %s' % s
        return s
