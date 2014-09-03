from __future__ import absolute_import
import collections
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
PLOT_LEVEL = '20'

# temporary data structure to map plot level to detail plotting decision.
# This should be replaced with a hierarchical log level.
_PLOT_LEVELS = {'all'     : 30,
                'default' : 20,
                'summary' : 10}
# detail plots will be generated if PLOT_LEVEL is greater than this
_PLOT_DETAIL_THRESHOLDS = {'default'     :20,
                           'hif_applycal':30,
                           'hifv_applycals':30}

def set_plot_level(plotlevel):
    if plotlevel not in ('all', 'summary', 'default'):
        LOG.warning('Unknown plot level: %r', plotlevel)
        plotlevel = 'default'
    LOG.info('Setting plot level to %r', plotlevel)
    module = sys.modules[__name__]
    setattr(module, 'PLOT_LEVEL', _PLOT_LEVELS[plotlevel])

    
def generate_detail_plots(result=None): 
    if result is None:
        LOG.warning('Calling generate_detail_plots without a result argument '
                    'is deprecated.')
        return PLOT_LEVEL >= _PLOT_DETAIL_THRESHOLDS['default']
    
    task = getattr(result, 'pipeline_casa_task', '')
    if task.startswith('hif_applycal'):
        return PLOT_LEVEL >= _PLOT_DETAIL_THRESHOLDS['hif_applycal'] 
    elif task.startswith('hifv_applycals'):
        LOG.info('Using plot level for the VLA as: '+str(_PLOT_DETAIL_THRESHOLDS['hifv_applycals']))
        return PLOT_LEVEL >= _PLOT_DETAIL_THRESHOLDS['hifv_applycals'] 
    else:
        return PLOT_LEVEL >= _PLOT_DETAIL_THRESHOLDS['default']
