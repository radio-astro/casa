"""Module to use casapy tasks"""

# History:
# 16-Jul-2007 jfl First version.

# package modules

import os
from numpy import *
import pickle
import pylab

# alma modules

from baseDisplay import *


class plotcal(BaseDisplay):
    """Class to plot bandpass."""

    def __init__(self, tasks, htmlLogger, msName, layout='portrait',
     yaxis='amp'):
        """Constructor.

        Keyword arguments:
        tasks      -- Dictionary of casapy tasks.
        htmlLogger -- Route for logging to html structure.
        msName     -- Name of MeasurementSet
        """
        BaseDisplay.__init__(self, tasks, htmlLogger, msName, layout)
        if tasks.has_key('plotcal') and tasks.has_key('default'):
            self._plotcal = tasks['plotcal'] 
            self._default = tasks['default'] 
        else:
            self._plotcal = None 
            self._default = None 
        self._yaxis = yaxis


    def description(self):
        description = 'Display using the casapy plotcal task'
        return description


    def display(self, reducerDescription, dataView):

        if self._plotcal == None:
            return

        bp = dataView.calculate()

# iterate through spectral windows, plotcal for each.

        for spw,cal in bp['data'].iteritems():
            title = 'bandpass calibration for SpW %s' % spw
            plotFile = '%s-%s.png' % (reducerDescription['name'], title)
            plotFile = self._pruneFilename(plotFile)
            plotFile = os.path.join(self._plotDirectory, plotFile)

            pylab.figure()
            pylab.clf()
            self._default(self._plotcal)
            self._plotcal(caltable=cal['table'], xaxis='chan', 
             yaxis=self._yaxis, figfile=plotFile)
            pylab.clf()
            pylab.close()

# link the plot to the html output

            plotName = title
            self._htmlLogger.appendNode(plotName, os.path.join(
             self._plotDirectory, plotFile))
