from __future__ import absolute_import

import collections
import copy

from pipeline.hifa.tasks.tsyscal import resultobjects 

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)

class TsysflagResults(resultobjects.TsyscalResults):
    def __init__(self):
        """
        Construct and return a new TsysflagResults.
        """
        # the results to be merged back into the context
        super(TsysflagResults, self).__init__()

        # component results
        self.components = collections.OrderedDict()

    def merge_with_context(self, context):
        # do nothing, the tsys cal files should already be in the context
        # and we don't want to insert them twice.
        pass

    def add(self, name, result):
        self.components[name] = result

    def __repr__(self):
        s = super(TsysflagResults, self).__repr__()
        s = 'TsysflagResults incorporating %s' % s
        components = self.components.keys()
        if not components:
            s += '\n Containing no component results'
        else:
            s += '\n Containing component results:'
            for component in components:
                s += '\n   %s' % component
        return s
