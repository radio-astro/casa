from __future__ import absolute_import
import collections
import itertools
import os
import string
import re
import types

import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.extern.pyparsing as pyparsing
import pipeline.infrastructure.logging as logging
from . import spectralwindow as spectralwindow
from . import measurementset as measurementset
import pipeline.domain.singledish as singledish

LOG = logging.get_logger(__name__)

class VirtualMeasurementSet(measurementset.MeasurementSet):    
    def __init__(self, scantable_list):
        if isinstance(scantable_list, singledish.ScantableRep):
            name = scantable_list.name
            session = scantable_list.session
            scantable_list = [scantable_list]
        else:
            name = ':'.join([s.name for s in scantable_list])
            session = scantable_list[0].session
        super(VirtualMeasurementSet,self).__init__(name, session)
        self.scantables = scantable_list
