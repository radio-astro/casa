"""Module to plot sky images."""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Change profiling.
# 30-Aug-2007 jfl Flux calibration version.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
#  9 Jan 2008 jfl Can now plot multiple clean boxes. Plots description ['TITLE']
#                 if present.
#  1-May-2008 jfl Removed plot log for java browser.
# 13-May-2008 jfl 13 release.
# 25-Jun-2008 jfl regression release.
# 25-Jun-2008 jfl more info on plots.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 26-Sep-2008 jfl mosaic release.
# 10-Oct-2008 jfl complex display release.
# 14-Nov-2008 jfl documentation upgrade release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import os
from numpy import *
import pickle
import pylab

# alma modules

from baseDisplay import *


class SkyDisplay(BaseDisplay):
    """Class to plot sky images."""

    def description(self):
        """Method to return a brief description of the class."""

        description = '''SkyDisplay - display an image of the sky'''
        return description


    def display(self, stageDescription, dataView, dataOperator, logName):
        """Method to display the data 'view' as sky images.

        Keyword arguments:
        stageDescription -- A dictionary with a description of the calling
                              reduction stage.
        dataView         -- The object giving the data 'view'.
        dataOperator     -- The object that has 'operated' on the data in
                              this stage.
        logName          -- The name of the file containing the casapy log.
        """

#        print 'SkyDisplay.display called'
        self._htmlLogger.timing_start('SkyDisplay.display')

# get results to display. This needs to be done before the view object is
# asked to describe itself in HTML

        viewResults = dataView.getData(logName=logName)

# write general description of the data view, operation
                              
        flagMessage,colour = self.writeBaseHTMLDescriptionHead(stageDescription,
         dataView, dataOperator)

# display results

        self._htmlLogger.logHTML('<h3>Displays</h3>')

        collection = viewResults['data']
        history = viewResults['parameters']['history']

        keys = collection.keys()
        keys.sort()
        for k in keys:
            description = pickle.loads(k)
            for result in collection[k]:

# iterate through collected items and display them in turn, check for an
# error result first

                if result['error'] != None:
                     plotName = '%s - error' % description['TITLE']
                     self._htmlLogger.appendNode(plotName, None)
                     continue

                pylab.ioff()
                pylab.figure(1)
                pylab.clf()

                dataType = result['dataType']
                dataUnits = result['dataUnits']
                xtitle = result['xtitle']
                xunits = result['xunits']
                x = result['x']
                ytitle = result['ytitle']
                yunits = result['yunits']
                y = result['y']
                data = transpose(result['data'])
                flag = transpose(result['flag'])
                clean_box = None
                if result.has_key('clean_box'):
                    clean_box = result['clean_box']
                clean_limit_box = None
                if result.has_key('clean_limit_box'):
                    clean_limit_box = result['clean_limit_box']
                image_ref = []
                if result.has_key('image_ref'):
                    image_ref = result['image_ref']

# image reference pixel
                
#                yoff = 0.20
                yoff = 0.10
                yoff = self._plotText(1.05, yoff, 'Reference position:', 40)
                cname = ['RA:', 'Dec:', 'Stokes:', 'Freq:']
                for i,k in enumerate(image_ref):
                    yoff = self._plotText(1.05, yoff, cname[i]+k, 40, mult=0.8)

# threshold and image rms info, if present.

                if result.has_key('threshold'):
                    yoff = self._plotText(1.05, yoff,
                     'clean threshold: %6.2e' %
                     result['threshold'], 40, mult=0.8)
                if result.has_key('rms'):
                    yoff = self._plotText(1.05, yoff,
                     'residual rms: %6.2e' % result['rms'], 40, mult=0.8)

# plot data

                pylab.gray()
#                pylab.imshow(data, interpolation='nearest', origin='lower',
#                 extent=[x[0], x[-1], y[0], y[-1]])
                pylab.imshow(data, interpolation='nearest', origin='lower',
                 aspect='equal', extent=[x[0], x[-1], y[0], y[-1]])

                pylab.axis('image')
#                pylab.axis('auto')
                lims = pylab.axis()

# ..plot wedge

                cb = pylab.colorbar(shrink=0.5)
                cb.set_label(dataUnits)
                for label in cb.ax.get_yticklabels():
                    label.set_fontsize(8)

# plot contours

                if result.has_key('rms2d'):
                    rms2d = result['rms2d']
                    contlevels = array([3, 6, 9, 12]) * rms2d
                    contTitle = \
                     'contours at [3, 6, 9, 12] * 2d residual rms (%s)' % \
                     rms2d 
                else:
                    imageMax = max(ravel(data))
                    contlevels = array([0.01, 0.03, 0.05, 0.07]) * imageMax
                    contTitle = \
                     'contours at [1, 3, 5, 7] per cent of image max. (%s)' % \
                     imageMax 
                pylab.contour(data, contlevels, origin='lower',
                 extent=[x[0], x[-1], y[0], y[-1]])

# print title

                pylab.xlabel('%s (%s)' % (xtitle, xunits))
                pylab.ylabel('%s (%s)' % (ytitle, yunits))
                title = 'notitle'
                if description.has_key('TITLE'):
                    title = description['TITLE']
                    pylab.figtext(0.1, 0.95, title, ha='left')
                pylab.figtext(0.1, 0.91, contTitle, ha='left')

# plot clean box if any

                if clean_box != None:
                    for box in clean_box:
                        pylab.plot(
                         [box[0], box[2], box[2], box[0], box[0]], 
                         [box[1], box[1], box[3], box[3], box[1]], 'g-')

# and clean limit box

                if clean_limit_box != None:
                    box = clean_limit_box
                    pylab.plot(
                     [box[0], box[2], box[2], box[0], box[0]], 
                     [box[1], box[1], box[3], box[3], box[1]], 'r-')

# make axis fit snugly around image
  
                pylab.axis([lims[0], lims[1], lims[2], lims[3]])

# save the image (remove odd characters from filename to cut down length)

                plotFile = '%s_%s_%s_v_%s_%s.png' % (stageDescription['name'],
                 dataType, ytitle, xtitle, title) 
                plotFile = self._pruneFilename(plotFile)
                pylab.savefig(os.path.join(self._plotDirectory, plotFile))
                plotName = description['TITLE']
                self._htmlLogger.appendNode(plotName, os.path.join(
                 self._plotDirectory, plotFile))

                pylab.clf()
                pylab.close(1)

        self._htmlLogger.flushNode()

# write HTML info that goes after the displays
 
        self.writeBaseHTMLDescriptionTail(stageDescription, dataView,
         dataOperator, logName)

# pass out parameters from data view, if present
        
        if viewResults.has_key('parameters'):
            dataParameters = viewResults['parameters']
        else:
            dataParameters = None

        self._htmlLogger.timing_stop('SkyDisplay.display')
        return flagMessage, colour, dataParameters


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the display.
   
        Keyword arguments:
        stageName -- The name of the recipe stage using the object.
        """

        self._htmlLogger.logHTML("""The data view was displayed as a tangent
         plane projection on the sky.""")


    def writeDetailedHTMLDescription(self, stageName):
        """Write a detailed description of the display.
   
        Keyword arguments:
        stageName -- The name of the recipe stage using the object.
        """

        self._htmlLogger.logHTML("""<h3>Display</h3>
         The data view was displayed as a tangent plane projection on the sky.
         """)
#         <p>The display was generated by Python class SkyDisplay.""")
