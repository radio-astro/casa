import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.api as api
import math

import pipeline.infrastructure as infrastructure
LOG = infrastructure.get_logger(__name__)


class ObservingPattern(api.Heuristic):
    """
    Analyze pointing pattern
    """
    def calculate(self, pos_dict):
        """
        Analyze pointing pattern from pos_dict which is calculated by
        GroupByPosition heuristic.
        Return (ret)
            ret: 'RASTER', 'SINGLE-POINT', or 'MULTI-POINT'
        # PosDict[row][0]: row, PosDict[][1]: index
        """
        LOG.info('Analyze Scan Pattern by Positions...')

        rows = pos_dict.keys()
        nrows = len(pos_dict)
        nPos = 0
        for row in rows:
            if pos_dict[row][0][0] != -1:
                nPos += 1
        if nPos == 0: nPos = 1
        LOG.debug('Number of Spectra: %d,   Number of independent position > %d' % (nrows, nPos))
        #if nPos > math.sqrt(len(rows)) or nPos > 10: ret = 'RASTER'
        if nPos > math.sqrt(nrows) or nPos > 3: ret = 'RASTER'
        elif nPos == 1: ret = 'SINGLE-POINT'
        else: ret = 'MULTI-POINT'
        LOG.info('Pattern is %s' % (ret))
        return ret

