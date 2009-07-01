"""This module provides a 'null' operation on the data 'view'."""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Change to profiling.
# 30-Aug-2007 jfl Flux calibrating version.
#  5-Sep-2007 jfl Removed () after class declaration.
# 28-Sep-2007 jfl Recipe release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release. 
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.

from baseFlagger import *


class NoFlagger(BaseFlagger):
    """This class provides a 'null' operation on the data 'view'."""

    def __init__(self, htmlLogger):
        """Constructor.

        Keyword arguments:
        htmlLogger -- Object giving access to the html structure.
        """
#        print 'NoFlagger constructor called'
        self._htmlLogger = htmlLogger
        self._rules = []
        self._potentially_flagged_target_ids = []


    def description(self):
        description = None
        return description


    def operate(self, stageDescription, dataView):
        """Public method to ask for the 'operation' to be performed on the
        data 'view'. In this case nothing is done.

        Keyword arguments:
        stageDescription -- Dictionary with a description of the reduction 
                            stage that is using this object.
        dataView         -- Object giving access to the data 'view'.
        """

#        print 'NoFlagger.operate called'
        self._htmlLogger.timing_start('noFlagger.operate')

# record this stage in the FLAG_CMD sub-table even though no flags
# have been set. 

        dataView.setFlags(stageDescription, [], {})

        self._htmlLogger.timing_stop('noFlagger.operate')
  

    def writeGeneralHTMLDescription(self, stageName):
        """Write description of this object to HTML.
        """
        self._htmlLogger.logHTML('No flagging was performed.')


    def writeDetailedHTMLDescription(self, stageName):
        """Write description of this object to HTML.
        """
        self._htmlLogger.logHTML('''<h3>Flagging</h3>
         No flagging was performed.''')
