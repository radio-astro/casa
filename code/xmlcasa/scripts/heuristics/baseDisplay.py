"""Module with Base Class for displays."""

# History:
# 16-Jul-2007 jfl First version.
# 30-Aug-2007 jfl Flux calibrating release.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
# 25-Jun-2008 jfl regression release.
# 10-Oct-2008 jfl complex display release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import os
from numpy import *
import pickle
import pylab
from types import *

#import numarray
#import ppgplot

# alma modules


class BaseDisplay:
    """Base Class for displays."""

    def __init__(self, tasks, htmlLogger, msName, layout='landscape'):
        """Constructor.

        Keyword arguments:
        tasks      -- Dictionary of casapy tasks.
        htmlLogger -- Route for logging to html structure.
        msName     -- Name of MeasurementSet.
        layout     -- If 'landscape' sub-plots will be horizontal bands
                      across the page. If 'portrait' they will be vertical
                      stripes.
        """

#        print 'BaseDisplay constructor called'
        self._tasks = tasks
        self._htmlLogger = htmlLogger
        if msName != 'noDisplay':
            self._plotDirectory = 'html'

            if not(os.path.exists(self._plotDirectory)):
                os.mkdir(self._plotDirectory)
            self._layout = layout
            self._descriptionWritten = False


    def _expandValue(self, key, value, summary):
        """Utility method to 'expand' a value. For example, if the value is
        a FIELD_ID it will have the source name appended to it.

        Keyword arguments:
        key     -- The 'name' of the value.
        value   -- The value to be expanded.
        summary -- Results summary for the stage using this class.
        """

        field_names = summary['field_names']
        field_types = summary['field_types']
        expandedValue = value
        if key == 'FIELD_ID':
             expandedValue = '%s (%s)' % (value, field_types[int(value)])
        return expandedValue


    def _plotText(self, xoff, yoff, text, maxchars, ny_subplot=1, mult=1):
        """Utility method to plot text and put line breaks in to keep the
        text within a given limit.

        Keyword arguments:
        xoff       -- world x coord where text is to start.
        yoff       -- world y coord where text is to start.
        text       -- Text to print.
        maxchars   -- Maximum number of characters before a newline is
                      inserted.
        ny_subplot -- Number of sub-plots along the y-axis of the page.
        mult       -- Factor by which the text fontsize is to be multiplied.
        """

        words = text.rsplit()
        words_in_line = 0
        line = ''
        ax = pylab.gca()
        for i in range(len(words)):
            temp = line + words[i] + ' '
            words_in_line += 1
            if len(temp) > maxchars:
                if words_in_line == 1:
                    ax.text(xoff, yoff, temp, va='center', fontsize=mult*8,
                     transform=ax.transAxes, clip_on=False)
                    yoff -= 0.03 * ny_subplot * mult
                    words_in_line = 0
                else:
                    ax.text(xoff, yoff, line, va='center', fontsize=mult*8,
                     transform=ax.transAxes, clip_on=False)
                    yoff -= 0.03 * ny_subplot * mult
                    line = words[i] + ' '
                    words_in_line = 1
            else:
                line = temp
        if len(line) > 0:
            ax.text(xoff, yoff, line, va='center', fontsize=mult*8,
             transform=ax.transAxes, clip_on=False)
            yoff -= 0.03 * ny_subplot * mult
        yoff -= 0.01 * ny_subplot * mult
        return yoff


    def _pad(self, id):
        """Utility method to insert spaces into the string version of id
        so that a sort of _pad'ed ids comes out 1,2,3 instead of 1,10,11,.. 2

        Keyword arguments:
        id    -- The number to be padded.
        """
        if id < 10:
             result = ' %s' % id
        else:
             result = str(id)
        return result


    def _pruneFilename(self, filename):
        """Method to remove characters from filenames that add no value
        and may upset the filesystem. Will break path names because / is
        removed.

        Keyword arguments:
        filename  -- the filename to be checked.
        """
        result = filename
        result = result.replace('/', '')
        result = result.replace(' ', '')
        result = result.replace('-', '')
        result = result.replace(':', '')
        result = result.replace(';', '')
        result = result.replace('{', '')
        result = result.replace('}', '')
        result = result.replace('[', '')
        result = result.replace(']', '')
        result = result.replace(',', '')
        result = result.replace("'", "")
        result = result.replace('\n', '')

        return result


    def display(self, stageName, dataView, dataOperator, logName):
        """Does nothing.
        """

#        print 'BaseDisplay.display called'
        return


    def writeBaseHTMLDescriptionHead(self, stageDescription, dataView,
     dataOperator):
        """Write description of view and operator to HTML.
        """

        self._htmlLogger.logHTML('<h2>Description</h2>')

# write recipe description of stage if available

        if stageDescription.has_key('description'):
            self._htmlLogger.logHTML('<p> %s' % stageDescription['description'])

# brief description

        stageName = stageDescription['name']
        viewMessage = dataView.writeGeneralHTMLDescription(stageName)
        if dataOperator != None:
            dataOperator.writeGeneralHTMLDescription(stageName)
        self.writeGeneralHTMLDescription(stageName)

        self._htmlLogger.logHTML('<h2>Results</h2>')

# flagging stats
        
        flagMessage = None
        flagMessageColour = None
        if dataOperator != None:
            flagMessage,flagMessageColour = dataView.writeFlaggingStatistics(
             stageDescription, dataOperator.rules(),
             dataOperator.potentiallyFlaggedTargetIDs())

        return flagMessage, flagMessageColour


    def writeBaseHTMLDescriptionTail(self, stageDescription, dataView,
     dataOperator, logName):
        """Write description of view and operator to HTML.
        """

# now automatic descriptions. The try/finally block is to make sure
# that once a new node is opened it is closed again whatever happens

        self._htmlLogger.logHTML('<h3>Logs</h3>')

# the casapy log.

        stageName = stageDescription['name']
        self._htmlLogger.logHTML('The relevant section of the casapy log is ')
        self._htmlLogger.logHTML('<a href="%s#%s">here</a>' %
          (logName, stageName))
        self._htmlLogger.logHTML('.')

        self._htmlLogger.logHTML('<h2>Algorithms</h2>')
        dataView.writeDetailedHTMLDescription(stageName, topLevel=True)
        if dataOperator != None:
            dataOperator.writeDetailedHTMLDescription(stageName)
        self.writeDetailedHTMLDescription(stageName)
