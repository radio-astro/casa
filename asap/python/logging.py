__all__ = ["asaplog", "print_log", "print_log_dec"]

from asap.env import is_casapy
from asap._asap import LogSink, set_global_sink
try:
    from functools import wraps as wraps_dec
except ImportError:
    from asap.compatibility import wraps as wraps_dec


class _asaplog(object):
    """Wrapper object to allow for both casapy and asap logging"""
    def __init__(self):
        self._enabled = False
        self._log = ""
        if is_casapy():
            from taskinit import casalog
            self.logger = casalog
        else:
            self.logger = LogSink()
            set_global_sink(self.logger)

    def post(self, level):
        """Post the messages to the logger. This will clear the buffered
        logs.
        """
        if len(self._log) > 0:
            self.logger.post(self._log, priority=level)
        if isinstance(self.logger, LogSink):
            logs = self.logger.pop()
            if logs > 0:
                print logs
        self._log = ""

    def push(self, msg, newline=True):
        """Push logs into the buffer. post needs to be called to send them."""
        from asap import rcParams
        if self._enabled:
            if rcParams["verbose"]:
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

asaplog = _asaplog()

def print_log_dec(f, level='INFO'):
    @wraps_dec(f)
    def wrap_it(*args, **kw):
        val = f(*args, **kw)
        print_log(level)
        return val
    return wrap_it

def print_log(level='INFO'):
    """Alias for asaplog.post."""
    asaplog.post(level)
