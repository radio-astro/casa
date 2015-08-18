"""This module presents a logging abstraction layer on top of casa.
"""
__all__ = ["asaplog", "asaplog_post_dec", "AsapLogger"]

import inspect
import sys
from asap.env import is_casapy
from asap.parameters import rcParams
from asap._asap import LogSink, set_global_sink
try:
    from functools import wraps as wraps_dec
except ImportError:
    from asap.compatibility import wraps as wraps_dec


class AsapLogger(object):
    """Wrapper object to allow for both casapy and asap logging.

    Inside casapy this will connect to `taskinit.casalog`. Otherwise it will
    create its own casa log sink.

    .. note:: Do not instantiate a new one - use the :obj:`asaplog` instead.

    """
    def __init__(self):
        self._enabled = True
        self._log = ""
        if is_casapy():
            from taskinit import casalog
            self.logger = casalog
        else:
            self.logger = LogSink()
            set_global_sink(self.logger)

    def post(self, level='INFO', origin=""):
        """Post the messages to the logger. This will clear the buffered
        logs.

        Parameters:

            level:  The log level (severity). One of INFO, WARN, ERROR.

        """
        if not self._enabled:
            return

        if not origin:
            origin = inspect.getframeinfo(inspect.currentframe().f_back)[2]
        logs = self._log.strip()
        if len(logs) > 0:
            if isinstance(self.logger, LogSink):
                #can't handle unicode in boost signature
                logs = str(logs)
            self.logger.post(logs, priority=level, origin=origin)
        if isinstance(self.logger, LogSink):
            logs = self.logger.pop().strip()
            if len(logs) > 0:
                if rcParams['verbose']:
                    print >>sys.stdout, logs
                    if hasattr(sys.stdout, "flush"):
                        sys.stdout.flush()
        self._log = ""

    def clear(self):
        if isinstance(self.logger, LogSink):
            logs = self.logger.pop()
            
    def push(self, msg, newline=True):
        """Push logs into the buffer. post needs to be called to send them.

        Parameters:

            msg:        the log message (string)

            newline:    should we terminate with a newline (default yes)

        """
        if self._enabled:
            sep = ""
            self._log = sep.join([self._log, msg])
            if newline:
                self._log += "\n"

    def enable(self, flag=True):
        """Enable (or disable) logging."""
        self._enabled = flag

    def disable(self, flag=False):
        """Disable (or enable) logging"""
        self._enabled = flag

    def is_enabled(self):
        return self._enabled

asaplog = AsapLogger()
"""Default asap logger"""

def asaplog_post_dec(f):
    """Decorator which posts log at completion of the wrapped method.

    Example::

        @asaplog_post_dec
        def test(self):
            do_stuff()
            asaplog.push('testing...', False)
            do_more_stuff()
            asaplog.push('finished')
    """
    @wraps_dec(f)
    def wrap_it(*args, **kw):
        level = "INFO"
        try:
            try:
                val = f(*args, **kw)
                return val
            except Exception, ex:
                level = "ERROR"
                asaplog.push(str(ex))
                if rcParams['verbose']:
                    pass
                else:
                    raise
        finally:
            asaplog.post(level, f.func_name)
    return wrap_it

