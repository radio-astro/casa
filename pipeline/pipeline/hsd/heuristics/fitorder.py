import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.api as api
import math

import pipeline.infrastructure as infrastructure
LOG = infrastructure.get_logger(__name__)


class FitOrderHeuristics(api.Heuristic):
    """
    Determine fitting order from a set of spectral data.
    """
    MaxDominantFreq = 15
    
    def calculate(self, data, mask=None, edge=(0,0)):
        """
        Determine fitting order from a set of spectral data, data,
        with masks for each spectral data, mask, and number of edge
        channels to be excluded, edge.
        
        First, manipulate each spectral data by the following procedure:

           1) mask regions specified by mask and edge,
           2) subtract average from spectral data,
           3) compute one-dimensional discrete Fourier Transform.

        Then, Fourier power spectrum is averaged and averaged power
        spectrum is analyzed to determine optimal polynomial order
        for input data array. The heuristics returns one representative
        polynomial order per input data array.

        data: two-dimensional data array with shape (nrow, nchan).
        mask: list of mask regions. Value should be a list of
              [[start0,end0],[start1,end1],...] for each spectrum.
              [[-1,-1]] indicates no mask. Default is None.
        edge: number of edge channels to be dropped. Default is (0,0).
        """
        (nrow,nchan) = data.shape
        effective_nchan = nchan - sum(edge)
        power_spectrum = []
        if mask is not None:
            mask_maker = MaskMaker(nchan, mask, edge)
        else:
            mask_maker = MaskMakerNoLine(nchan, edge)
        for irow in xrange(nrow):
            spectrum = data[irow]
            flag = mask_maker.get_mask(irow)
            average = (spectrum * flag).sum() / float(flag.sum())
            spectrum = (spectrum - average) * flag

            # Apply FFT to the spectrum
            power_spectrum.append(numpy.abs(numpy.fft.rfft(spectrum)))

        # Average seems to be better than median
        #power = numpy.median(power_spectrum, axis=0)
        power = numpy.average(power_spectrum, axis=0)
        
        max_freq = max(int(self.MaxDominantFreq * effective_nchan / 2048.0), 1)

        # 2007/09/01 Absolute value of power should be taken into account
        # If the power is low, it should be ignored
        # Normalize the power
        power2 = power / power.mean()
        max_power = power2[:max_freq].max()
        if max_power < 3.0: poly_order = 1.0
        elif max_power < 5.0: poly_order = 1.5
        elif max_power < 10.0: poly_order = 2.0
        else:
            flag = False
            for i in xrange(max_freq, -1, -1):
                if power2[i] > 10.0: break
                if power2[i] > 5.0: flag = True
            if flag == True: poly_order = float(max(2.0, i)) + 0.5
            else: poly_order = float(max(2.0, i))

        # Finally, convert to polynomial order
        #poly_order = int(poly_order * 3.0)
        #poly_order = int(poly_order + 1) * 2.0 + 0.5)
        poly_order = int(poly_order * 2.0 + 1.5)

        return poly_order

        
class MaskMakerNoLine(object):
    def __init__(self, nchan, edge):
        self.flag = numpy.ones(nchan)
        self.flag[:edge[0]] = 0
        self.flag[(nchan-edge[1]):] = 0

    def get_mask(self, row):
        return self.flag
        
class MaskMaker(MaskMakerNoLine):
    def __init__(self, nchan, lines, edge):
        super(MaskMaker,self).__init__(nchan, edge)
        self.lines = lines

    def get_mask(self, row):
        flag = self.flag.copy()
        for line in self.lines[row]:
            if line[0] != -1:
                flag[line[0]:line[1]] = 0
        return flag

