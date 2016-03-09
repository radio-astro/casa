# Copyright (C) 2010 Vinay Sajip. All Rights Reserved.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies and that
# both that copyright notice and this permission notice appear in
# supporting documentation, and that the name of Vinay Sajip
# not be used in advertising or publicity pertaining to distribution
# of the software without specific, written prior permission.
# VINAY SAJIP DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
# ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
# VINAY SAJIP BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
# ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
# IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

import logging


class LoggerAdapter(object):
    """
    An adapter for loggers which makes it easier to specify contextual
    information in logging output.
    """

    def __init__(self, logger, extra):
        """
        Initialize the adapter with a logger and a dict-like object which
        provides contextual information. This constructor signature allows
        easy stacking of LoggerAdapters, if so desired.

        You can effectively pass keyword arguments as shown in the
        following example:

        adapter = LoggerAdapter(someLogger, dict(p1=v1, p2="v2"))
        """
        self.logger = logger
        self.extra = extra

    def process(self, msg, kwargs):
        """
        Process the logging message and keyword arguments passed in to
        a logging call to insert contextual information. You can either
        manipulate the message itself, the keyword args or both. Return
        the message and kwargs modified (or not) to suit your needs.

        Normally, you'll only need to override this one method in a
        LoggerAdapter subclass for your specific needs.
        """
        kwargs["extra"] = self.extra
        return msg, kwargs

    #
    # Boilerplate convenience methods
    #
    def debug(self, msg, *args, **kwargs):
        """
        Delegate a debug call to the underlying logger.
        """
        self.log(logging.DEBUG, msg, *args, **kwargs)

    def info(self, msg, *args, **kwargs):
        """
        Delegate an info call to the underlying logger.
        """
        self.log(logging.INFO, msg, *args, **kwargs)

    def warning(self, msg, *args, **kwargs):
        """
        Delegate a warning call to the underlying logger.
        """
        self.log(logging.WARNING, msg, *args, **kwargs)

    warn = warning

    def error(self, msg, *args, **kwargs):
        """
        Delegate an error call to the underlying logger.
        """
        self.log(logging.ERROR, msg, *args, **kwargs)

    def exception(self, msg, *args, **kwargs):
        """
        Delegate an exception call to the underlying logger.
        """
        kwargs["exc_info"] = 1
        self.log(logging.ERROR, msg, *args, **kwargs)

    def critical(self, msg, *args, **kwargs):
        """
        Delegate a critical call to the underlying logger.
        """
        self.log(logging.CRITICAL, msg, *args, **kwargs)

    def log(self, level, msg, *args, **kwargs):
        """
        Delegate a log call to the underlying logger, after adding
        contextual information from this adapter instance.
        """
        if self.isEnabledFor(level):
            msg, kwargs = self.process(msg, kwargs)
            self.logger._log(level, msg, args, **kwargs)

    def isEnabledFor(self, level):
        """
        Is this logger enabled for level 'level'?
        """
        if self.logger.manager.disable >= level:
            return False
        return level >= self.getEffectiveLevel()

    def setLevel(self, level):
        """
        Set the specified level on the underlying logger.
        """
        self.logger.setLevel(level)

    def getEffectiveLevel(self):
        """
        Get the effective level for the underlying logger.
        """
        return self.logger.getEffectiveLevel()

    def hasHandlers(self):
        """
        See if the underlying logger has any handlers.
        """
        return logutils.hasHandlers(self.logger)

