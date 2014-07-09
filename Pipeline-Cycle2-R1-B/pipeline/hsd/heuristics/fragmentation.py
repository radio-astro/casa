import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.api as api
import math

import pipeline.infrastructure as infrastructure
LOG = infrastructure.get_logger(__name__)


class FragmentationHeuristics(api.Heuristic):
    """
    Determine fragmentation from given total number of channels,
    edge channels to be dropped, and polynomial order for baseline
    fit. 
    """
    MaxOrder = 9
    MaxFragmentation = 3
    MinChannels = 512
    
    def calculate(self, polyorder, nchan, edge, modification=0):
        """
        Determine fragmentation from given total number of channels,
        edge channels to be dropped, and polynomial order for baseline
        fit. 

        Inputs:
           polyorder -- polynomial order for baseline fit
           nchan -- number of channels
           edge -- edge channels to be dropped given by tuple, (left, right)
           modification -- modification factor for polyorder

        Returns:
           fragment -- fragmentation parameter
           num_segment -- number of segments
           segment_polyorder -- polynomial order for baseline fit of segments
        """
        _polyorder = polyorder + modification
        _nchan = nchan - sum(edge)
        fragment = int(min(_polyorder / self.MaxOrder + 1,
                           max(_nchan / self.MinChannels, 1)))
        fragment = min(fragment, self.MaxFragmentation)
        num_segment = fragment * 2 - 1
        segment_polyorder = min(int(_polyorder / fragment) + fragment - 1,
                                 self.MaxOrder)
        return (fragment, num_segment, segment_polyorder)
