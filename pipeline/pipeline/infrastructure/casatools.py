from __future__ import absolute_import
import copy_reg
import sys

from casac import casac
from taskinit import casalog

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


class TableReader(object):
    '''
    TableReader is a context manager for CASA tables.
    
    TableReader uses the CASA table tool to open the given table, closing the
    table automatically once this object is out of scope or if an exception is 
    raised.
    '''   
    num_instances = 0

    def __init__(self, table_name, nomodify=True):
        self._table_name = table_name
        self._nomodify = nomodify
        
    def __enter__(self):        
        if TableReader.num_instances != 0:
            raise IOError('Cannot open multiple tables simultaneously')
        # increment the counter *after* opening the file, otherwise the
        # counter could be left non-zero should the table.open raise an 
        # exception
        table.open(self._table_name, nomodify=self._nomodify)
        TableReader.num_instances += 1
        return table

    def __exit__(self, thetype, value, traceback):
        table.close()
        TableReader.num_instances -= 1


# class MSReader(object):
#     '''
#     MSReader is a context manager for measurement sets.
#     
#     MSReader uses the CASA measurement set tool to open the given measurement
#     set, closing it automatically once this object is out of scope or if an
#     exception is raised.
#     ''' 
#     
#     num_instances = 0
# 
#     def __init__(self, ms_name):
#         self._ms_name = ms_name
#         
#     def __enter__(self):        
#         if MSReader.num_instances != 0:
#             raise IOError('Cannot open multiple measurement sets simultaneously')
#         MSReader.num_instances += 1
#         ms.open(self._ms_name)
#         return ms
# 
#     def __exit__(self, thetype, value, traceback):
#         ms.close()
#         MSReader.num_instances -= 1


# class ImageReader(object):
#     """Uses the CASA image tool to open the given image,
#     closing it automatically once this object is out of scope or if an
#     exception is raised.""" 
#     
#     num_instances = 0
# 
#     def __init__(self, image_name):
#         self._image_name = image_name
#         
#     def __enter__(self):        
#         if ImageReader.num_instances != 0:
#             raise IOError('Cannot open multiple images simultaneously')
#         ImageReader.num_instances += 1
#         image.open(self._image_name)
#         return image
# 
#     def __exit__(self, thetype, value, traceback):
#         image.close()
#         ImageReader.num_instances -= 1


def createContextManager(tool):
    '''
    ToolContextManager is a context manager for CASA tools. It closes the 
    tool automatically once the object goes out of scope or when an exception
    is raised. 
    '''     
    # variables in the outer scope of python closures are read-only, so we
    # must make this a mutable type
    num_instances = [0]

    class t(object):
        def __init__(self, filename):
            self.filename = filename
                
        def __enter__(self):        
            if num_instances[0] != 0:
                raise IOError('Cannot open multiple files simultaneously')
            num_instances[0] += 1
            tool.open(self.filename)
            return tool
        
        def __exit__(self, thetype, value, traceback):
            tool.close()
            num_instances[0] -= 1

    return t

ImageReader = createContextManager(image)
CalAnalysis = createContextManager(calanalysis)
MSReader = createContextManager(ms)

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
