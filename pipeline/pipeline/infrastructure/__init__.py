from __future__ import absolute_import
import sys

from . import api
from . import callibrary
from . import casatools
from . import filenamer
from . import imagelibrary
from . import jobrequest
from . import logging
from . import utils

from .callibrary import CalLibrary, CalTo, CalFrom, CalApplication, CalState, SDCalLibrary 
from .launcher import Context, Pipeline
from .logging import get_logger
from .jobrequest import JobRequest, casa_tasks

LOG = logging.get_logger(__name__)
PLOT_LEVEL = 'all'

def set_plot_level(plotlevel):
    if plotlevel not in ('all', 'summary'):
        LOG.warning('Unknown plot level: %r', plotlevel)
        plotlevel = 'all'
    LOG.info('Setting plot level to %r', plotlevel)
    module = sys.modules[__name__]
    setattr(module, 'PLOT_LEVEL', plotlevel)
    
def generate_detail_plots():
    return PLOT_LEVEL == 'all'
