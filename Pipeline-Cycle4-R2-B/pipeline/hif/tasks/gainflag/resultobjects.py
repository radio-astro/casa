from __future__ import absolute_import

import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.common import flaggableviewresults

LOG = infrastructure.get_logger(__name__)


class GainflagResults(basetask.Results):
    def __init__(self):
        """
        Construct and return a new GainflagResults.
        """
        basetask.Results.__init__(self)

        # component results
        self.components = collections.OrderedDict()

    def merge_with_context(self, context):
        pass

    def add(self, name, result):
        self.components[name] = result

    def __repr__(self):
        s = 'GainflagResults'
        components = self.components.keys()
        if not components:
            s += '\n Containing no component results'
        else:
            s += '\n Containing component results:'
            for component in components:
                s += '\n   %s' % component
        return s


class GainflaggerResults(basetask.Results,
  flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new GainflaggerResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'GainflaggerResults'
        return s


class GainflaggerDataResults(basetask.Results):
    def __init__(self):
        """
        Construct and return a new GainflaggerDataResults.
        """
        basetask.Results.__init__(self)

    def merge_with_context(self, context):
        # do nothing, none of the gain cals used for the flagging
        # views should be used elsewhere
        pass

    def __repr__(self):
        s = 'GainflaggerDataResults'
        return s


class GainflaggerViewResults(basetask.Results,
  flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new GainflaggerViewResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'GainflaggerViewResults'
        return s
