from __future__ import absolute_import 

import sys

LogLevelMap = {'critical': 0,
               'error': 0,
               'warning': 1,
               'info': 2,
               'debug': 3,
               'todo': 4,
               'trace': 4}

####
# ProgressTimer
#
# Show the progress bar on the console if LogLevel is lower than or equal to 2.
#
####
class ProgressTimer(object):
    def __init__(self, length=80, maxCount=80, LogLevel='info'):
        """
        Constructor:
            length: length of the progress bar (default 80 characters)
        """
        self.currentLevel = 0
        self.maxCount = maxCount
        self.curCount = 0
        self.scale = float(length)/float(maxCount)
        self.LogLevel = LogLevelMap[LogLevel] if LogLevelMap.has_key(LogLevel) else 2
        if self.LogLevel <= 2:
            print '\n|' + '='*((length-8)/2) + ' 100% ' + '='*((length-8)/2) + '|'

    def __del__(self):
        if self.LogLevel <= 2:
            print '\n'

    def count(self, increment=1):
        if self.LogLevel <= 2:
            self.curCount += increment
            newLevel = int(self.curCount * self.scale)
            if newLevel != self.currentLevel:
                print '\b' + '*' * (newLevel - self.currentLevel),
                sys.stdout.flush()
                self.currentLevel = newLevel

# parse edge parameter to tuple
def parseEdge(edge):
    if len(edge) == 2:
        (EdgeL, EdgeR) = edge
    else:
        EdgeL = edge[0]
        EdgeR = edge[0]
    return(EdgeL, EdgeR)
