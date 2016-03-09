import decimal
import math
import numpy as np
import os.path
import re
import types

import pipeline.infrastructure.filenamer as filenamer
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import cleanhelper
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
from pipeline.extern.findContinuum import findContinuum

LOG = infrastructure.get_logger(__name__)


class FindContHeuristics(object):

    def __init__(self, context):
        self.context = context

    def find_continuum(self, dirty_cube):
        channel_selection, png_name, aggregate_bw = findContinuum(dirty_cube)
        frequency_ranges_GHz = utils.selection_to_frequencies(dirty_cube, channel_selection, 'GHz')
        return(frequency_ranges_GHz, png_name)
