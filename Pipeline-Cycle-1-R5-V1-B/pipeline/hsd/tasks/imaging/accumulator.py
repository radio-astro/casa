from __future__ import absolute_import

import os
import math
import numpy

import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)

class GaussianKernel(object):
    def __init__(self, width):
        self.width = width
        self.width_squared = width * width

    def get_weight(self, radius):
        #return math.exp(-0.69314718055994529 * (radius * radius) / (self.width * self.width))
        return math.exp(-0.69314718055994529 * (radius * radius) / self.width_squared)


class LinearKernel(object):
    def __init__(self, width):
        self.width = width

    def get_weight(self, radius):
        return (1.0 - 0.5 * radius / self.width)


class Accumulator(object):
    KernelType = {'gauss': GaussianKernel,
                  'linear': LinearKernel}
    def __init__(self, minmaxclip, weight_rms, weight_tintsys,
                 kernel_type, kernel_width):
        self.minmaxclip = minmaxclip
        self.weight_rms = weight_rms
        self.weight_tintsys = weight_tintsys
        self.kernel = self.KernelType[kernel_type.lower()](kernel_width)

    def init(self, data):
        self.data = data
        self.data_shape = data.shape
        self.channel_weight = numpy.ones(self.data_shape, dtype=numpy.float32)
        self.row_weight = numpy.ones(self.data_shape[0], dtype=numpy.float32)

        # make sure that minmaxclip doesn't clip whole data
        self.minmaxclip = self.minmaxclip and self.data_shape[0] > 2

    def accumulate(self, indexlist, rmslist, deltalist, tsys_base, exposure_base):
        num_axis0 = self.data_shape[0]
        num_axis1 = self.data_shape[1]

        # 2013/06/03 TN
        # Calculate channel-independent weights first, then distribute it
        # to each channel, and finally calculate channel-dependent weights

        # Channel-independent weights
        if self.weight_rms:
            for m in xrange(num_axis0):
                if rmslist[m] != 0.0:
                    factor = rmslist[m] * rmslist[m]
                    self.row_weight[m] /= factor
                else:
                    self.row_weight[m] = 0.0

        if self.weight_tintsys:
            for m in xrange(num_axis0):
                # 2008/9/21 Bug fix
                # 2013/05/30 TN Bug fix
                # using rowlist as index is invalid
                if tsys_base[indexlist[m]] > 0.5:
                    factor = (exposure_base[indexlist[m]]/(tsys_base[indexlist[m]]*tsys_base[indexlist[m]]))
                    self.row_weight[m] *= factor
                else:
                    self.row_weight[m] = 0.0

        # Weight by Radius
        for m in xrange(num_axis0):
            factor = self.kernel.get_weight(deltalist[m])
            self.row_weight[m] *= factor

        # Channel-dependent weights
        self.channel_weight *= self.row_weight.reshape((num_axis0,1))

        # Currently only channel-dependent weight is minmax clipping
        if self.minmaxclip:
            self.channel_weight[numpy.argmin(self.data, axis=0), xrange(num_axis1)] = 0.0
            self.channel_weight[numpy.argmax(self.data, axis=0), xrange(num_axis1)] = 0.0
        
        # Combine Spectra
        if self.channel_weight.sum() != 0:
            self.accumulated = (numpy.sum(self.data * self.channel_weight, axis=0) / numpy.sum(self.channel_weight, axis=0))
        else:
            self.accumulated = numpy.zeros(num_axis1, dtype=numpy.float32)

        # Calculate RMS of the spectrum
        r0 = ((rmslist * self.row_weight) * (rmslist * self.row_weight)).sum()
        r1 = self.row_weight.sum()
        self.rms = math.sqrt(r0) / r1

