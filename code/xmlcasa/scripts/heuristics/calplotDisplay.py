"""Module to plot calibration solutions using the cal plot tool.
"""

# History:
# 16-Jul-2007 jfl First version.

# package modules

import datetime
import matplotlib.dates as mdates
import matplotlib.ticker as ticker
from numpy import *
import os
import pickle
import pylab

# alma modules

import casac
from baseDisplay import *

class CalplotDisplay(BaseDisplay):
    """
    Class to display gain solutions using the calplot tool.
    """

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

        print 'CalplotDisplay constructor called'

        BaseDisplay.__init__(self, tasks, htmlLogger, msName, layout)

# get a calplot tool

        calplotTool = casac.homefinder.find_home_by_name('calplotHome')
        self._calplot = calplotTool.create()
        tableTool = casac.homefinder.find_home_by_name('tableHome')
        self._table = tableTool.create()


    def description(self):
        """Method to return a brief description of the class."""

        description = 'CalplotDisplay - display gain solutions using calplot.'
        return description


    def display(self, stageDescription, dataView, dataOperator, logName):
        """Method to have the gains displayed.

        Keyword arguments:
        stageDescription   -- Dictionary holding a description of the calling
                              object.
        dataView           -- The object giving access to the data 'view'.
        dataOperator       -- The object that has 'operated' on the data in 
                              this stage.
        logName            -- The name of the file containing the casapy log.
        """

        print 'CalplotDisplay.display called'
        self._htmlLogger.timing_start('CalplotDisplay.display')

# get results to display. This needs to be done before the view object is
# asked to describe itself in HTML

        dataResult = dataView.getData()

# write general description of the data view, operation

        flagMessage,colour = self.writeBaseHTMLDescriptionHead(stageDescription,
         dataView, dataOperator)

# display results

        self._htmlLogger.logHTML('<h3>Displays</h3>')

        history = dataResult['parameters']['history']
        flagging = dataResult['flagging']
        flaggingReason = dataResult['flaggingReason']
        flaggingApplied = dataResult['flaggingApplied']

        keys = dataResult['data'].keys()

        if len(keys) == 0:
            self._htmlLogger.logHTML('Nothing to display')

        for key in keys:
            results = dataResult['data'][key]
            description = pickle.loads(key)

# how many versions of the data are there

            if len(results) > 1:
                nstages = 2
                stages = [results[0], results[-1]]
                stage_flagging = [flagging[0], flagging[-1]]
                stage_flaggingReason = [flaggingReason[0], flaggingReason[-1]]
                stage_flaggingApplied = [flaggingApplied[0],
                 flaggingApplied[-1]]
            else:
                nstages = 1
                stages = [results[0]]
                stage_flagging = [flagging[0]]
                stage_flaggingReason = [flaggingReason[0]]
                stage_flaggingApplied = [flaggingApplied[0]]

# iterate through results

            data = stages[0]['data']
            print 'title', description['TITLE']
            print 'cal group name', description['CALIBRATION_GROUP_NAME']
            print 'cal group spw', description['CALIBRATION_GROUP_SPWS']
            print 'data to plot', data
            flagVersions = stages[0]['flagVersions']
            print 'flagversions', flagVersions

# now plot the results for each antenna

            original = flagVersions.index('BeforeHeuristics')
            stageEntry = flagVersions.index('StageEntry')
            current = flagVersions.index('Current')

            self._table.open(data[current])
            antenna1 = self._table.getcol('ANTENNA1')
            self._table.close()
            antenna_range = []
            for antenna in antenna1:
                if antenna_range.count(antenna) == 0:
                    antenna_range.append(antenna)
            antenna_range.sort()
            print antenna_range

            self._calplot.open(data[current])
            for yaxis in ['AMP', 'PHASE']:
                subplot = 221
                plotted_antennas = []
                for antenna in antenna_range:
                    plotted_antennas.append(antenna)
                    self._calplot.selectcal(antenna=str(antenna))
                    self._calplot.plotoptions(subplot=subplot, overplot=False,
                     iteration='', plotrange=[], showflags=False,
                     plotsymbol='.', plotcolor='blue', markersize=2.0,
                     fontsize=5.0) 
                    self._calplot.plot(xaxis='time', yaxis=yaxis)
                    pylab.text(0.7, 0.8, 'Antenna %s' % antenna, 
                     transform=pylab.gca().transAxes, fontsize=10.0)
                    subplot += 1
                    if subplot > 224 or antenna==antenna_range[-1]:

# construct a suitable filename and save the image

                        title = 'Group %s; gain %s for antennas: %s' % (
                         description['CALIBRATION_GROUP_NAME'], yaxis,
                         plotted_antennas)
                        dataTitle = 'gains'
                        xtitle = 'time' 
                        plotFile = '%s-%s-v-%s.png' % (
                         stageDescription['name'], title, xtitle) 
                        plotFile = self._pruneFilename(plotFile)
                        self._calplot.savefig(filename=os.path.join(
                         self._plotDirectory, plotFile))
                        subplot = 221
                        plotted_antennas = []
                        self._calplot.clearplot()

# add link information to the list

                        plotName = title
                        self._htmlLogger.appendNode(plotName, os.path.join(
                         self._plotDirectory, plotFile))

            self._calplot.close()

        self._htmlLogger.flushNode()

# write HTML info that goes after the displays

        self.writeBaseHTMLDescriptionTail(stageDescription, dataView,
         dataOperator, logName)

        self._htmlLogger.timing_stop('CalplotDisplay.display')
        return flagMessage, colour, dataResult['parameters']


    def writeGeneralHTMLDescription(self, stageName):
        """Write general description of display.

        Keyword arguments:
        stageName -- The name of the stage using this class.
        """
 
        self._htmlLogger.logHTML('''The data view was a gain solution; it was 
         displayed using the calplot tool.''')


    def writeDetailedHTMLDescription(self, stageName):

        """Write detailed description of display.

        Keyword arguments:
        stageName -- The name of the stage using this class.
        """
 
        self._htmlLogger.logHTML('''<h3>Display</h3>
         The data to be displayed was a file containing gain solutions.
         This display of it using the calplot tool.''')
