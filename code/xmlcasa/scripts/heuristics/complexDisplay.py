"""Module to plot data on the complex plane."""

# History:
# 10-Oct-2008 jfl First version.
# 14-Nov-2008 jfl documentation upgrade release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import os
from numpy import *
import pickle
import pylab

# alma modules

from baseDisplay import *


class ComplexDisplay(BaseDisplay):
    """Class to plot data on the complex plane."""

    def _plot_panel(self, nplots, plot_number, data, flag, 
     flagVersions, flagging_in, flaggingApplied_in, sub_title, stageName):
        """Plot the data into one panel.

        Keyword arguments:
        nplots             -- The number of sub-plots on the page.
        plot_number        -- The index of this sub-plot.
        data               -- The complex data.
        flag               -- List of flags for data, one for each flag version.
        flagVersions       -- Flagging version name for each item in flag list.
        flagging_in        -- List of flagging commands applied to data.
        flaggingApplied_in -- True if corresponding 'flagging_in' has been
                              been applied to the data.
        sub_title          -- The title to be given to this subplot.
        stageName          -- The name of the recipe stage using the object.
        """  

#        print 'ComplexDisplay._plot_panel called'
        self._htmlLogger.timing_start('ComplexDisplay._plot_panel')

        if flagVersions.count('Current') > 0:
            i_current = flagVersions.index('Current')
        else:
            raise ValueError, 'flag has no Current version'

        data_good = compress(ravel(logical_not(flag[i_current])), ravel(data))

        x = data_good.imag
        y = data_good.real

        lim = 0.0
        if len(data_good) > 0:
            xmax = max(abs(x))
            ymax = max(abs(y))
            lim = 1.1 * max(xmax,ymax)
        if lim < 1.0e-6:
            lim = 1.0

# plot data

        pylab.subplot(1, nplots, plot_number)
        pylab.xlabel('Imaginary part')
        pylab.ylabel('Real part')
        pylab.axvline(c='k')
        pylab.axhline(c='k')

# set axes beforehand in effort to stop markers changing size during
# scatter plot on Linux machines

        pylab.axis([-lim, lim, -lim, lim])
        if len(x) > 0:
            pylab.scatter(x, y, s=1, marker='x')
        else:
            pylab.annotate('no data', (0.5,0.5), xycoords='axes fraction')

# 30-Sep-2008 casapy 5654 bug means 'equal' axes corrupt limits in x.
# So, arrange to plot expected limit values in y.

        pylab.title(sub_title)
        pylab.axis('equal')
        pylab.axis([-lim, lim, -lim, lim])

        self._htmlLogger.timing_stop('ComplexDisplay._plot_panel')
        return


    def description(self):
        """Method to return a brief description of the class."""

        description = '''ComplexDisplay - display a complex dataset on the
         complex plane'''
        return description


    def display(self, stageDescription, dataView, dataOperator, logName):
        """Method to have the images displayed.

        Keyword arguments:
        stageDescription -- Dictionary holding a description of the calling
                            object.
        dataView         -- The object giving access to the data 'view'.
        dataOperator     -- The object that has 'operated' on the data in
                            this stage.
        logName          -- The name of the file containing the casapy log.
        """

#        print 'ComplexDisplay.display called'

        self._htmlLogger.timing_start('ComplexDisplay.display')

# get results to display. This needs to be done before the view object is
# asked to describe itself in HTML

        dataResult = dataView.getData(topLevel=True)

# write general description of the data view, operation

        flagMessage,colour = self.writeBaseHTMLDescriptionHead(stageDescription,
         dataView, dataOperator)

# display results

        self._htmlLogger.logHTML('<h3>Displays</h3>')

# do the work

        summary = dataResult['summary']
        collection = dataResult['data']
        history = dataResult['parameters']['history']

        flagging = dataResult['flagging']
        flaggingReason = dataResult['flaggingReason']
        flaggingApplied = dataResult['flaggingApplied']

        keys = collection.keys()
        keys.sort()
        for key in keys:
            pickled_description = key
            results = collection[key]
            description = pickle.loads(pickled_description)

# convert integer descriptors to strings, padded with space so that string
# ordering of numbers works for ..9,10,... etc. Also just uses first
# element of lists.

            for k,v in description.iteritems():
                if type(v) == ListType:
                    if len(v) > 1:
                        print 'WARNING - list longer than 1 element in description'
                    description[k] = v[0]  
            for k,v in description.iteritems():
                if type(v) == IntType:
                    description[k] = self._pad(v)

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
            
            plot_number = 1
            pylab.ioff()
            pylab.figure(1)
            pylab.clf()

# iterate through collected items and display them in turn

            for index, result in enumerate(stages):
                sub_title = ''
                if nstages > 1:
                    if index == 0:
                        sub_title = 'IN' 
                    else:
                        sub_title = 'OUT'

                if len(result) ==  0:
                    dataTitle = 'no data'
                    continue
 
                dataTitle = result['dataType']
                data = result['data']
                flag = result['flag']
                flagVersions = result['flagVersions']
                self._plot_panel(nstages+1, plot_number, data, flag,
                 flagVersions, stage_flagging[index],
                 stage_flaggingApplied[index],
                 sub_title, stageDescription['name'])
                plot_number += 1

# plot the titles and k

            pylab.subplot(1, nstages+1, plot_number)
            pylab.axis('off')
            yoff = 1.1
            yoff = self._plotText(0.1, yoff, 'STAGE: %s' % 
             stageDescription['name'], 40)
            title = 'notitle'
            if description.has_key('TITLE'):
                title = description['TITLE']
                yoff = self._plotText(0.1, yoff, title, 35)

# history
            
            yoff -= 0.1
            yoff = self._plotText(0.1, yoff, 'Calculation History:', 50)
            yoff = self._plotText(0.1, yoff,
                 'Origin:%s' % history, 50, mult=0.8)
 
            pylab.axis([0.0,1.0,0.0,1.0])

# save the image (remove odd characters from filename to cut down length)

            plotFile = '%s_%s_%s.png' % (stageDescription['name'],
             dataTitle, title) 
            plotFile = self._pruneFilename(plotFile)
            pylab.savefig(os.path.join(self._plotDirectory, plotFile))
            plotName = title
            self._htmlLogger.appendNode(plotName, os.path.join(
             self._plotDirectory, plotFile))

            pylab.clf()
            pylab.close(1)

        self._htmlLogger.flushNode()

# write HTML info that goes after the displays

        self.writeBaseHTMLDescriptionTail(stageDescription, dataView,
         dataOperator, logName)

# pass out parameters from data view, if present
        
        if dataResult.has_key('parameters'):
            dataParameters = dataResult['parameters']
        else:
            dataParameters = None

        self._htmlLogger.timing_stop('ComplexDisplay.display')
        return flagMessage, colour, dataParameters


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the display.

        Keyword arguments:
        stageName -- The name of the recipe stage using the object.
        """

        self._htmlLogger.logHTML('''The data view was shown as a set of
         scatter plots of the data points on the complex plane.
         ''')


    def writeDetailedHTMLDescription(self, stageName):
        """Write a detailed description of the display.

        Keyword arguments:
        stageName -- The name of the recipe stage using the object.
        """

        self._htmlLogger.logHTML('''<h3>Display</h3>
         The data view was shown as a set of 
         scatter plots of data points on the complex plane.
         <p>If data were flagged as part of this reduction stage then
         each display contains 2 images; one displaying the data view before
         flagging, the other after flagging. The reasons for data having
         been flagged are colour coded.''')
#         <p>The display was produced by Python class ComplexDisplay.
#         ''')

