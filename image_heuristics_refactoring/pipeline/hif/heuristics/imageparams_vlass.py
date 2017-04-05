import os.path
import decimal
import math
import numpy as np
import re
import types
import collections

import cleanhelper

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.contfilehandler as contfilehandler
import pipeline.domain.measures as measures
from .imageparams_base import ImageParamsHeuristics

LOG = infrastructure.get_logger(__name__)


class ImageParamsHeuristicsVLASS(ImageParamsHeuristics):

    def __init__(self, context, vislist, spw, contfile=None, linesfile=None):
        ImageParamsHeuristics.__init__(self, context, vislist, spw, contfile, linesfile)
        self.imaging_mode = 'VLASS'


    def niter_correction(self, niter, cell, imsize, residual_max, threshold):

        '''Adjustment of number of cleaning iterations due to mask size.'''

        new_niter = 1000

        return new_niter
