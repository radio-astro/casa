from __future__ import absolute_import
import logging
import sys
import types

import pipeline.extern.logutils as logutils
import pipeline.extern.logutils.colorize as colorize
from . import casatools

from logging import CRITICAL, WARNING, ERROR, INFO, DEBUG

# Register two new logging levels with the standard logger
TRACE = 5
logging.addLevelName(TRACE, 'TRACE')
colorize.ColorizingStreamHandler.level_map[TRACE] = (None, 'blue', False)

TODO = 13
logging.addLevelName(TODO, 'TODO')
colorize.ColorizingStreamHandler.level_map[TODO] = ('black', 'yellow', True)

LOGGING_LEVELS = {'critical' : CRITICAL,
                  'error'    : ERROR,
                  'warning'  : WARNING,
                  'info'     : INFO,
                  'debug'    : DEBUG,
                  'todo'     : TODO,
                  'trace'    : TRACE}

# Begin with a default log level of NOTSET. All loggers created at module level
# import time will use this logging level. 
logging_level = logging.NOTSET

# initialise the root logger so that module-level messages can be logged.
# Change this to see lower level messages from module-level statements at import
# time
logging.getLogger().setLevel(INFO)
_loggers = []


class CASALogHandler(logging.Handler):
    """
    A handler class which writes logging records, appropriately formatted,
    to the CASA log.
    """

    def __init__(self, log=None):
        """
        Initialize the handler.

        If log is not specified, the current CASA global logger is used.
        """
        logging.Handler.__init__(self)
        if log is None:
            log = casatools.log
        self._log = log
        self.setFormatter(logging.Formatter('%(message)s', ''))

    def flush(self):
        """
        Flushes the stream.
        """
        pass

    def emit(self, record):
        """
        Emit a record.

        If a formatter is specified, it is used to format the record. The 
        record is then written to the stream with a trailing newline. If
        exception information is present, it is formatted using
        traceback.print_exception and appended to the stream.
        """
        try:
            msg = self.format(record)
            priority = CASALogHandler.get_casa_priority(record.levelno)
            origin = record.name
            log = self._log            
            log.post(msg, priority, origin)
            self.flush()
        except (KeyboardInterrupt, SystemExit):
            raise
        except:
            self.handleError(record)

    @staticmethod
    def get_casa_priority(lvl):
        if lvl >= ERROR:
            return 'ERROR'
        if lvl >= WARNING:
            return 'WARN'
        if lvl >= INFO:
            return 'INFO'
        if lvl >= DEBUG:
            return 'DEBUG1'
        if lvl >= TRACE:
            return 'DEBUG2'
        return 'INFO'


# This format is more useful when debugging as it gives more details on who
# emitted the message. Try it!
#               format='%(asctime)s %(name)s %(funcName)s\n%(levelname)s: %(message)s',

def get_logger(name, 
               format='%(asctime)s %(levelname)s: %(message)s',
               datefmt='%Y-%m-%d %H:%M:%S',
               stream=sys.stdout, level=None,
               filename=None, filemode='w', filelevel=None,  
               propagate=False):  
    """Do basic configuration for the logging system. Similar to 
    logging.basicConfig but the logger ``name`` is configurable and both a 
    file output and a stream output can be created. Returns a logger object. 
 
    The default behaviour is to create a StreamHandler which writes to 
    sys.stdout, set a formatter using the "%(message)s" format string, and add
    the handler to the ``name`` logger. 
 
    A number of optional keyword arguments may be specified, which can alter
    the default behaviour. 
 
    :param name: Logger name 
    :param format: handler format string 
    :param datefmt: handler date/time format specifier 
    :param stream: initialize the StreamHandler using ``stream`` 
        (None disables the stream, default=sys.stdout) 
    :param level: logger level (default=current pipeline log level). 
    :param filename: create FileHandler using ``filename`` (default=None) 
    :param filemode: open ``filename`` with specified filemode ('w' or 'a') 
    :param filelevel: logger level for file logger (default=``level``) 
    :param propagate: propagate message to parent (default=False) 
 
    :returns: logging.Logger object 
    """  
    if level is None:
        level = logging_level
    
    # Get a logger for the specified name  
    logger = logging.getLogger(name)
    
    def trace(self, msg, *args, **kwargs):
        """
        Log 'msg % args' with severity 'TRACE'.

        To pass exception information, use the keyword argument exc_info with
        a true value, e.g.

        logger.info("Houston, we have a %s", "interesting problem", exc_info=1)
        """
        if self.isEnabledFor(TRACE):
            self._log(TRACE, msg, args, **kwargs)

    def todo(self, msg, *args, **kwargs):
        """
        Log 'msg % args' with severity 'TODO'.

        To pass exception information, use the keyword argument exc_info with
        a true value, e.g.

        logger.info("Houston, we have a %s", "interesting problem", exc_info=1)
        """
        if self.isEnabledFor(TODO):
            self._log(TODO, msg, args, **kwargs)

    logger.trace = types.MethodType(trace, logger)
    logger.todo = types.MethodType(todo, logger)
    
    logger.setLevel(logging_level)
    fmt = logging.Formatter(format, datefmt)  
    logger.propagate = propagate  
  
    # Remove existing handlers, otherwise multiple handlers can accrue  
    for hdlr in logger.handlers[:]:  
        logger.removeHandler(hdlr)  
  
    # Add handlers. Add NullHandler if no file or stream output so that  
    # modules don't emit a warning about no handler.  
    if not (filename or stream):  
        logger.addHandler(logutils.NullHandler())  
  
    if filename:  
        hdlr = logging.FileHandler(filename, filemode)  
        if filelevel is None:  
            filelevel = level  
        hdlr.setLevel(filelevel)  
        hdlr.setFormatter(fmt)  
        logger.addHandler(hdlr)  
  
    if stream:
        hdlr = colorize.ColorizingStreamHandler(stream)
        hdlr.setLevel(level)  
        hdlr.setFormatter(fmt)  
        logger.addHandler(hdlr)  

    hdlr = CASALogHandler()
#     hdlr.setLevel(level)
    logger.addHandler(hdlr)

    _loggers.append(logger)  
    
    return logger

def set_logging_level(logger=None, level='info'):
    level_no = LOGGING_LEVELS.get(level, logging.NOTSET)

    if logger is not None:
        logger.setLevel(level_no)
        
    else:
        module = sys.modules[__name__]
        setattr(module, 'logging_level', level_no)
    
        for logger in _loggers:
            logger.setLevel(level_no)
            
    #     casa_level = CASALogHandler.get_casa_priority(level_no)
    #     casatools.log.filter(casa_level)
    

def add_handler(handler):
    '''
    Add given handler to all registered loggers.
    '''
    for l in _loggers:
        l.addHandler(handler)

        
def remove_handler(handler):
    '''
    Remove specified handler from all registered loggers. 
    '''
    for l in _loggers:
        l.removeHandler(handler)
       
       
class CapturingHandler(logging.Handler):
    """
    A handler class which buffers logging records above a certain threshold
    in memory.
    """
    
    def __init__(self, level=WARNING):
        '''
        Initialize the handler.
        '''
        logging.Handler.__init__(self, level)
        self.buffer = []

    def emit(self, record):
        '''
        Emit a record.

        Append the record to the buffer.
        '''
        # The Traceback object attached to the LogRecord exception tuple is
        # not serializable. Replace it with a substitute wrapper object that 
        # is.
        if record.exc_info is not None:
            exc_type, exc_value, tb = record.exc_info
            record.exc_info = (exc_type, exc_value, Traceback(tb))
        
        self.buffer.append(record)

    def flush(self):
        '''
        Override to implement custom flushing behaviour.

        This version just zaps the buffer to empty.
        '''
        self.buffer = []

    def close(self):
        '''
        Close the handler.

        This version just flushes and chains to the parent class' close().
        '''
        self.flush()
        logging.Handler.close(self)


# Code, Frame and Traceback are serializable substitutes for the Traceback
# logged with exceptions
class Code(object):
    def __init__(self, code):
        self.co_filename = code.co_filename
        self.co_name = code.co_name

class Frame(object):
    def __init__(self, frame):
        self.f_globals = {"__file__": frame.f_globals["__file__"]}
        self.f_code = Code(frame.f_code)

class Traceback(object):
    def __init__(self, tb):
        self.tb_frame = Frame(tb.tb_frame)
        self.tb_lineno = tb.tb_lineno
        if tb.tb_next is None:
            self.tb_next = None
        else:
            self.tb_next = Traceback(tb.tb_next)


LOG = get_logger(__name__)
