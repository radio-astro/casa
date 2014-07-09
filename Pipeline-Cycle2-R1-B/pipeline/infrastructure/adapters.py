from __future__ import absolute_import
from . import api
import pipeline.h.heuristics as heuristics


class Adapter(api.Heuristic):
    """
    Adapter is a base class for heuristic adapters.
    """
    def __init__(self, heuristic):
        # if the heuristic is not callable, such as when it's a Python
        # primitive, wrap it in an EchoHeuristic
        if not callable(heuristic):
            heuristic = heuristics.EchoHeuristic(heuristic)
            
        self._adaptee = heuristic 


# Not clear why this routine is located here.

class GaincurveAdapter(Adapter):
    """
    GaincurveAdapter adapts the 
    :class:`~pipeline.heuristics.gaincurve.Gaincurve` heuristic to a pipeline
    interface. It converts the incoming pipeline domain arguments to Python
    primitives suitable for 
    :func:`~pipeline.heuristics.gaincurve.Gaincurve.calculate`.    
    """
    def __init__(self, heuristic):
        super(GaincurveAdapter, self).__init__(heuristic)
    
    def calculate(self, ms):
        # gaincurve is a simple switch that should depend purely on whether
        # the observation is a VLA observation or not, so the name of the
        # antenna array should be sufficient to calculate the appropriate
        # value
        return self._adaptee(ms.antenna_array.name)
