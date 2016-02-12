from __future__ import absolute_import

import collections
import types

import pipeline.infrastructure.basetask as basetask
from pipeline.hifa.tasks.tsyscal import resultobjects 
from pipeline.hif.tasks.common import flaggableviewresults

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils

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
        
        # list of entirely flagged antennas (to be removed from refants)
        self.bad_antennas = []

    def merge_with_context(self, context):
        
        # If any antennas were found to be fully flagged,
        # remove them from the list of reference antennas.
        if self.bad_antennas:
            # Get the MS, and proceed if it contains a list of 
            # reference antennas.
            ms = context.observing_run.get_ms(name=self.vis)
            if hasattr(ms, 'reference_antenna') and \
              type(ms.reference_antenna) == types.StringType:

                # Create list of current refants
                refant = ms.reference_antenna.split(',')
                
                # Find intersection between refants and fully flagged antennas
                refants_to_remove = set(self.bad_antennas).intersection(refant)
                
                if refants_to_remove:
                    # Log warning
                    ant_msg = utils.commafy(refants_to_remove, quotes=False,
                                            multi_prefix='s')
                    LOG.warning('Antenna%s that are fully flagged in all Tsys '
                      'spws in the "BANDPASS", "PHASE", and/or "AMPLITUDE" '
                      'intents removed from refant list for '
                      '%s' % (ant_msg, ms.basename))
                    
                    # Remove fully flagged ants from refants, and store back in MS
                    for antenna in refants_to_remove:
                        refant.remove(antenna)
                    ms.reference_antenna = ','.join(refant)
 
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
