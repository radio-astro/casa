from __future__ import absolute_import

import os
import abc
import numpy
import math
import pylab as PL

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.renderer.logger as logger
from . import SDChannelAveragedImageDisplay
from . import SDSpectralImageDisplay

LOG = infrastructure.get_logger(__name__)

def SDImageDisplayFactory(mode):
    LOG.debug('MODE=%s'%(mode))
    if mode == 'TP':
        return SDChannelAveragedImageDisplay

    else:
        # mode should be 'SP'
        return SDSpectralImageDisplay
