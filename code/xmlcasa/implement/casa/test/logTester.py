
import os
import re
import shutil
import sys
import traceback
import types

import casac

class LogTester:
    """Class to test casa logger tool.
    """
    def __init__(self, tasks={}):

        self._stage = {}

# create the log tool

        self._log = {}
        self._logTool = casac.homefinder.find_home_by_name('logsinkHome')
        self._log['logTool'] = self._logTool.create()
        self._log['logTool'].origin('LogTester')
        self._log['logTool'].setlogfile('casapy.log')
        if len(tasks) == 0:
            self._log['logTool'].setglobal(True)

    #define this function or call setlogfile directly
    #whenever whereever you want a new logfile
    #
    #change the default fname if you want
    def newlogfile(self, fname='casapy.py'):
        self._log['logTool'].setlogfile(fname)

    def reduce(self, start=None):

        self._log['logTool'].postLocally('<pre>')
        self._log['logTool'].postLocally('<whatever>')
        self._log['logTool'].postLocally('</pre>')

