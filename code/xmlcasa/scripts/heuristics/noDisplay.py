"""Module containing the NULL display object."""

# History:
# 16-Jul-2007 jfl First version.
#  9-Aug-2007 jfl imports of numarray and ppgplot removed.
# 28-Aug-2007 jfl Recipe release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release.
# 25-Jun-2008 jfl regression release.
# 14-Nov-2008 jfl documentation upgrade release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import os
from numpy import *
import pickle
import pylab

# alma modules

from baseDisplay import *


class NoDisplay(BaseDisplay):
    """Null display class."""

    def __init__(self, htmlLogger):
        """Constructor.

        Keyword arguments:
        htmlLogger -- Conduit to HTML structure.
        """
        BaseDisplay.__init__(self, None, htmlLogger, 'noDisplay')

 
    def description(self):
        """Method to return a brief description of the class."""

        description = None
        return description


    def display(self, stageDescription, dataView, dataOperator, logName):
        """Method to have the slices displayed.

        Keyword arguments:
        stageDescription   -- Dictionary holding a description of the calling
                              object.
        dataView           -- The object giving access to the data 'view'.
        dataOperator       -- The object that has 'operated' on the data in
                              this stage.
        logName            -- The name of the file containing the casapy log. 

        """

# get results to display. This need to be done before the view object is
# asked to describe itself in HTML

        dataResult = dataView.getData()

# write descriptions of the data view, operation and display

        flagMessage,colour = self.writeBaseHTMLDescriptionHead(
         stageDescription, dataView, dataOperator)
        self.writeBaseHTMLDescriptionTail(stageDescription, dataView,
         dataOperator, logName)

# pass out parameters from data view, if present

        if dataResult.has_key('parameters'):
            dataParameters = dataResult['parameters']
        else:
            dataParameters = None

        return flagMessage, colour, dataParameters


    def writeGeneralHTMLDescription(self, stageName):
        """Write general description of display.

        Keyword arguments:
        stageName -- The name of the stage using this class.
        """
        self._htmlLogger.logHTML('No data were displayed.')


    def writeDetailedHTMLDescription(self, stageName):
        """Write detailed description of display.

        Keyword arguments:
        stageName -- The name of the stage using this class.
        """
        self._htmlLogger.logHTML('''<h3>Display</h3>
         No data were displayed.''')


