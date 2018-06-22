from __future__ import absolute_import

import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.h.tasks.common import flaggableviewresults
from pipeline.h.tasks.tsyscal import resultobjects

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

        # task completion status / reason for incompleteness
        self.task_incomplete_reason = ''

        # list of antennas that should be moved to the end
        # of the refant list
        self.refants_to_demote = set()

        # list of entirely flagged antennas that should be removed from refants
        self.refants_to_remove = set()

    def merge_with_context(self, context):
        # Update reference antennas for MS.
        ms = context.observing_run.get_ms(name=self.vis)
        ms.update_reference_antennas(ants_to_demote=self.refants_to_demote,
                                     ants_to_remove=self.refants_to_remove)

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


class TsysflagDataResults(basetask.Results):
    def __init__(self):
        """
        Construct and return a new TsysflagDataResults.
        """
        basetask.Results.__init__(self)

    def merge_with_context(self, context):
        # do nothing, tsys cal data table should already be in context
        pass

    def __repr__(self):
        s = 'TsysflagDataResults'
        return s


class TsysflagViewResults(basetask.Results,
                          flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new TsysflagViewResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'TsysflagViewResults'
        return s
