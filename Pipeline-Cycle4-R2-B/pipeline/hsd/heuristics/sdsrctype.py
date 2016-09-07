from __future__ import absolute_import

import pipeline.infrastructure.api as api
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure
from pipeline.hsd.heuristics import DataTypeHeuristics
LOG = infrastructure.get_logger(__name__)

from asap import srctype

class SrcTypeHeuristics(api.Heuristic):
    """
    """
    TypeMap = {'ps': 'pson',
               'otf': 'pson',
               'otfraster': 'pson'}
    def calculate(self, calmode):
        if self.TypeMap.has_key(calmode):
            return int(getattr(srctype, self.TypeMap[calmode]))
        else:
            #return int(srctype.notype)
            return -1
