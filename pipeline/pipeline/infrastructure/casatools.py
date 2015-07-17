from __future__ import absolute_import
import copy_reg
import contextlib
import sys

from casac import casac
from taskinit import casalog

from . import logging

LOG = logging.get_logger(__name__)

imager = casac.imager()
measures = casac.measures()
quanta = casac.quanta()
table = casac.table()
ms = casac.ms()
tableplot = casac.table()
calibrater = casac.calibrater()
calanalysis = casac.calanalysis()
msplot = casac.msplot()
calplot = casac.calplot()
#flagger = casac.flagger()
agentflagger = casac.agentflagger()
image = casac.image()
imagepol = casac.imagepol()
simulator = casac.simulator()
componentlist = casac.componentlist()
coordsys = casac.coordsys()
regionmanager = casac.regionmanager()
spectralline = casac.spectralline()
utils = casac.utils()
deconvolver = casac.deconvolver()
vpmanager = casac.vpmanager()
vlafillertask = casac.vlafillertask()
atmosphere = casac.atmosphere()
utils = casac.utils()
msmd = casac.msmetadata()

# Unless we set the log file, output will be sent to casapy.log rather than the
# timestamped log file, which is where standard CASA output is sent.
#log =  casac.homefinder.find_home_by_name('logsinkHome').create()
#log.setlogfile(casalog.logfile())
log = casalog

def post_to_log(comment='', echo_to_screen=True):
    log.post(comment)
    if echo_to_screen:
        sys.stdout.write('{0}\n'.format(comment))

def set_log_origin(fromwhere=''):
    log.origin(fromwhere)


def context_manager_factory(tool):
    '''
    Create a context manager function that wraps the given CASA tool.

    The returned context manager function takes one argument: a filename. The
    function opens the file using the CASA tool, returning the tool so that it
    may be used for queries or other operations pertaining to the tool. The
    tool is closed once it falls out of scope or an exception is raised.
    '''
    tool_name = tool.__class__.__name__

    @contextlib.contextmanager
    def f(filename, **kwargs):
        LOG.trace('%s tool: opening \'%s\'' % (tool_name, filename))
        tool.open(filename, **kwargs)
        try:
            yield tool
        finally:
            LOG.trace('%s tool: closing \'%s\'' % (tool_name, filename))
            tool.close()
    return f

# context managers for frequently used CASA tools
CalAnalysis = context_manager_factory(calanalysis)
ImageReader = context_manager_factory(image)
ImagerReader = context_manager_factory(imager)
MSReader = context_manager_factory(ms)
TableReader = context_manager_factory(table)
MSMDReader = context_manager_factory(msmd)

# C extensions cannot be pickled, so ignore the CASA logger on pickle and
# replace with it with the current CASA logger on unpickle
__tools = ['imager', 'measures', 'quanta', 'table', 'ms', 'tableplot', 
           'calibrater', 'msplot', 'calplot', 'agentflagger',
           'image', 'imagepol', 'simulator', 'componentlist', 'coordsys',
           'regionmanager', 'spectralline', 'utils', 'deconvolver',
           'vpmanager', 'vlafillertask', 'atmosphere', 'log', 'utils']
#           'calibrater', 'msplot', 'calplot', 'flagger', 'agentflagger',

for tool in __tools:
    tool_type = type(globals()[tool])
    unpickler = lambda data: globals()[tool]
    pickler = lambda _: (unpickler, (tool, ))
    copy_reg.pickle(tool_type, pickler, unpickler)
