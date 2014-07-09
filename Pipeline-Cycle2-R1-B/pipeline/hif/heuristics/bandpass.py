import numpy

import pipeline.infrastructure.api as api

class MedianSNR(api.Heuristic):
    def calculate(self, snr):
        if len(snr) > 0:
            return 1.0 / numpy.median(snr)
        else:
            return 0
