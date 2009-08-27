"""Module to plot slices through 2-d data.
"""

# History:
# 16-Jul-2007 jfl First version.
#  9-Aug-2007 jfl facecolor parameter name added to .fill() calls.
# 16-Aug-2007 jfl Removed redundant version of _plot_panel, changed 
#                 profiling.
# 30-Aug-2007 jfl Flux calibrating version.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
# 25-Jun-2008 jfl regression release.
# 26-Jun-2008 jfl order calculation history.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 10-Oct-2008 jfl complex display release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 12-Dec-2008 jfl 15-dec datetime.fromtimestamp fix.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import datetime
import matplotlib.dates as mdates
import matplotlib.ticker as ticker
from numpy import *
import os
import pickle
import pylab

# alma modules

from baseDisplay import *

class BaseSlice(BaseDisplay):

    def _plot_panel(self, layout, nplots, plot_number, description, xtitle, x, 
     dataType, dataUnits, data, data_mad, flag, flagVersions, stageDescription,
     flagging_in, flagReason_in, flaggingApplied_in, sub_title,
     noisy_channels=None):
        """Plot the data into one panel.
        
        Keyword arguments:
        layout             -- If 'landscape' sub-plots are horizontal stripes
                              across the page. If 'portrait' sub-plots are
                              vertical stripes.
        nplots             -- The number of sub-plots on the page.
        plot_number        -- The index of this sub-plot.
        description        -- Dictionary whose entries describe the data to be
                           -- plotted.
        xtitle             -- The name of the x-axis.
        x                  -- The values of the x-axis.
        dataType           -- String description of the data.
        dataUnits          -- String description of the data units.
        data               -- The 1d data.
        data_mad           -- MAD of the 1d data.
        flag               -- List of flags for data, one for each flag version.
        flagVersions       -- Flagging version name for each item in flag list.
        stageDescription   -- Dictionary with elements describing the reducer
                              object calling this method.
        flagging_in        -- List of flagging commands applied to data.
        flagReason_in      -- Matching list of dictionaries describing at
                              what stage and why the flagging was applied. 
        flaggingApplied_in -- Matching list whose entries are True if 
                              'flagging_in' has been applied to the data.
        sub_title          -- The title to be given to this subplot.
        noisy_channels     -- Channels designated as at 'edge' of bandpass.
        """
#        print 'BaseSlice._plot_panel called'
        self._htmlLogger.timing_start('BaseSlice._plot_panel')

# find which layer in flags has current flagging

        if flagVersions.count('Current') > 0:
            i_current = flagVersions.index('Current')

# get time axis in sensible units (hours since beginning of day) - if x-axis
# is TIME

        if xtitle == 'TIME':
            day = floor(x[0]/86400.0)
            xaxis = x - day * 86400.0
        else:
            xaxis = x

# set the font sizes etc.

        if layout == 'landscape':
            pylab.rc('axes', titlesize=10)
            pylab.rc('axes', labelsize=8)
            pylab.rc('xtick', labelsize=8)
            pylab.rc('ytick', labelsize=8)
            pylab.subplots_adjust(hspace=0.35)

# set the plot layout

        if layout == 'portrait':
            pylab.subplot(1, nplots, plot_number)
        elif layout == 'landscape':
            pylab.subplot(nplots, 1, plot_number)
        else:
            raise NameError, 'bad layout: %s' % layout

# plot good data as circles

        full_set = set(range(len(data)))
        if noisy_channels == None:
            noisy_set = set()
        else:
            noisy_set = set(list(noisy_channels))
        not_noisy_set = full_set.difference(noisy_set)
        good_data = compress(logical_not(flag[i_current]
         [array(list(not_noisy_set))]), data[array(list(not_noisy_set))])

        datemin = None
        datemax = None
        datamin = None
        datamax = None
        if len(good_data) > 0:
            good_data_mad = compress(
             logical_not(flag[i_current][array(list(not_noisy_set))]),
             data_mad[array(list(not_noisy_set))])
            datamin = min(good_data - good_data_mad)
            datamax = max(good_data + good_data_mad)
            good_x = compress(
             logical_not(flag[i_current][array(list(not_noisy_set))]),
             xaxis[array(list(not_noisy_set))])
            if xtitle == 'TIME':
                temp = []
                for item in good_x:
                    temp.append(datetime.datetime.utcfromtimestamp(item))
                good_x = temp
                datemin = min(good_x)
                datemax = max(good_x)
            pylab.errorbar(good_x, good_data, good_data_mad, fmt='o',
             color='black') 

        pylab.title(sub_title, fontsize='small')
        pylab.xlabel(xtitle)
        if plot_number == 1:
            if dataUnits != '':
                pylab.ylabel('%s (%s)' % (dataType, dataUnits))
            else:
                pylab.ylabel(dataType)

# ..overplot points not valid in blue. Often these will be zero having
# simply not been calculated because the underlying data are bad. Hence plot
# them floating at the correct x position above the bottom of the plot, to
# suggest that the y value is unimportant.

        ymin, ymax = pylab.ylim()
        yflag = ymin + (ymax - ymin) / 10.0
        datamin = min(yflag, datamin)
        bad_data = compress(flag[i_current], data)
        if len(bad_data) > 0:
            bad_data[:] = yflag 
            bad_x = compress(flag[i_current], xaxis)
            if xtitle == 'TIME':
                temp = []
                for item in bad_x:
                    temp.append(datetime.datetime.utcfromtimestamp(item))
                bad_x = temp
                if datemin == None:
                    datemin = min(bad_x)
                    datemax = max(bad_x)
                else:
                    datemin = min(datemin, min(bad_x))
                    datemax = max(datemax, max(bad_x))
            pylab.errorbar(bad_x, bad_data, fmt='^', color='blue')

# overplot 'edge' channels in grey

        if xtitle.upper() == 'CHANNEL' and noisy_channels != None:
            edge_data = compress(logical_not(flag[i_current]
             [array(noisy_channels, int)]), data[array(noisy_channels, int)])
            if len(edge_data) > 0:
                edge_data_mad = compress(logical_not(flag[i_current]
                 [array(noisy_channels, int)]), data_mad[array(noisy_channels,int)])
                edge_x = compress(logical_not(flag[i_current]
                 [array(noisy_channels,int)]), xaxis[array(noisy_channels,int)])
                pylab.errorbar(edge_x, edge_data, edge_data_mad, fmt='o',
                 color='grey') 

# ..overplot 'flagging'.

# look at the flags in reverse so that the first time a point is
# flagged is what appears on the plot - work on local copies to
# avoid unpredictable effects on global data.

        flagging = list(flagging_in)
        flagReason = list(flagReason_in)
        flaggingApplied = list(flaggingApplied_in)
        flagging.reverse()
        flagReason.reverse()
        flaggingApplied.reverse()
        x_range = arange(len(x))
        data_desc_id = int(description['DATA_DESC_ID'])
        if description.has_key('FIELD_ID'):
            field_id = int(description['FIELD_ID'])
        else:
            field_id = None
        if description.has_key('ANTENNA1'):
            plot_antenna = int(description['ANTENNA1'])
        else:
            plot_antenna = None

        flagsPlotted = False
        for i,val in enumerate(flagging):

# ignore the flags if they are not currently applied to the data

            if not(flaggingApplied[i]):
                continue
            for rule, flag_list in val.iteritems():

# do these flags apply to this source, data_desc?

                for flag_cmd in flag_list:
                    if flag_cmd['stageName'] != stageDescription['name']:
                        continue

                    if flag_cmd.has_key('FIELD_ID'):
                        if not(flag_cmd['FIELD_ID'].count(field_id)):
                            continue

                    if flag_cmd.has_key('DATA_DESC_ID'):
                        if flag_cmd['DATA_DESC_ID'] != data_desc_id:
                            continue

                    flag_x = []
                    if xtitle.upper().find('ANTENNA') > -1:  

# flag all data for antennas

                        if flag_cmd.has_key('ANTENNA'):
                            if list(flag_cmd['ANTENNA']).count(plot_antenna):
                                flag_x = list(x)
                            else:
                                flag_x = list(flag_cmd['ANTENNA'])
                        else:

# flag baselines

                            if flag_cmd.has_key('ANTENNA1') and \
                             list(flag_cmd['ANTENNA1']).count(plot_antenna):
                                flag_x = list(flag_cmd['ANTENNA2'])
                            if flag.has_key('ANTENNA2') and \
                             list(flag_cmd['ANTENNA2']).count(plot_antenna):
                                flag_x += list(flag_cmd['ANTENNA1'])

                    if xtitle == 'CHANNEL':  
                        if flag_cmd.has_key('CHANNELS'):
                            flag_x = list(flag_cmd['CHANNELS'])

                    if not(flag_cmd.has_key('colour')):
                        continue
                    colour = flag_cmd['colour']

                    for xval in flag_x:
                        xf = compress(abs(xval-x) < 0.001, x_range)
                        if xtitle == 'TIME':
                            xf = datetime.datetime.utcfromtimestamp(xf)
                        yf = array(xf, float)
                        yf[:] = yflag
                        pylab.errorbar(xf, yf, fmt='^', color=colour,
                         markeredgecolor=colour)
                        print yf
                        if len(yf) > 0:
                            flagsPlotted = True

# ..overplot points flagged on stage entry in violet

        if flagVersions.count('StageEntry') > 0:
            i_stageentry = flagVersions.index('StageEntry')
            bad_data = compress(flag[i_stageentry], data)
            if len(bad_data) > 0:
                bad_x = compress(flag[i_stageentry], xaxis)
                if xtitle == 'TIME':
                    temp = []
                    for item in bad_x:
                        temp.append(datetime.datetime.utcfromtimestamp(item))
                    bad_x = temp
                bad_data[:] = yflag
                pylab.errorbar(bad_x, bad_data, fmt='^', color='violet')

# ..overplot points flagged on recipe entry in green. 

        if flagVersions.count('BeforeHeuristics') > 0:
            i_original = flagVersions.index('BeforeHeuristics')
            bad_data = compress(flag[i_original], data)
            if len(bad_data) > 0:
                bad_x = compress(flag[i_original], xaxis)
                if xtitle == 'TIME':
                    temp = []
                    for item in bad_x:
                        temp.append(datetime.datetime.utcfromtimestamp(item))
                    bad_x = temp
                bad_data[:] = yflag
                pylab.errorbar(bad_x, bad_data, fmt='^', color='g')

# ..overplot points with no data in indigo

        if flagVersions.count('NoData') > 0:
            i_nodata = flagVersions.index('NoData')
            bad_data = compress(flag[i_nodata], data)
            if len(bad_data) > 0:
                bad_x = compress(flag[i_nodata], xaxis)
                if xtitle == 'TIME':
                    temp = []
                    for item in bad_x:
                        temp.append(datetime.datetime.utcfromtimestamp(item))
                    bad_x = temp
                bad_data[:] = yflag
                pylab.errorbar(bad_x, bad_data, fmt='^', color='indigo')

# set plot y scale to include all 'good' points.

        if datamin != None:
            room = (datamax - datamin) / 10.0
            pylab.gca().set_ylim(datamin - room, datamax + room)

# turn off possible use of offset in ScalarFormatter on y axis - otherwise 
# if the y range is much smaller than the abs value then the default ticks are 
# relative to an offset printed at the top of the axis, which has led to 
# confusion

        pylab.gca().yaxis.set_major_formatter(ticker.ScalarFormatter(
         useOffset=False))

        if xtitle == 'TIME':

# set the limits of the plot to whole hours

            datemin = datemin.replace(minute=0, second=0, microsecond=0)

# following will have problems at very end of month

            if datemax.hour < 23:
                datemax = datemax.replace(hour=datemax.hour + 1, minute=0,
                 second=0, microsecond=0)
            else:
                datemax = datemax.replace(day=datemax.day + 1, hour=0,
                 minute=0, second=0, microsecond=0) 
            pylab.gca().set_xlim(datemin, datemax)

# set the major ticks to lie each hour

            hours = mdates.HourLocator(interval=1)
            minutes = mdates.MinuteLocator(interval=10)
            pylab.gca().xaxis.set_major_locator(hours)
            pylab.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Hh%Mm'))
            pylab.gca().xaxis.set_minor_locator(minutes)

# override default x-axis title

            pylab.gca().set_xlabel('Time (hours after MJD start)')

# formats time axis better by tilting tick strings

            pylab.gcf().autofmt_xdate()

        self._htmlLogger.timing_stop('BaseSlice._plot_panel')
        print flagsPlotted
        return flagsPlotted


    def _plotTitleAndKey(self, stageDescription, history,
     description, flaggingReason, flaggingApplied, ny_subplot,
     noisyChannelsPlotted, bandpassFlaggingStage):
        """Method to plot the plot title, description and key.

        Keyword arguments:
        stageDescription   -- Dictionary with elements describing the 
                              reduction stage.
        history            -- Info on which stage calculated the data.
        description        -- Dictionary with elements describing the data.
        flaggingReason     -- List of dictionaries describing at
                              what stage and why flagging was applied. 
        flaggingApplied    -- Matching list whose entries are True if 
                              the flags from flaggingReason has been applied
                              to the data.
        ny_subplot         -- The number of sub-plots up the y axis of the 
                              page.
        noisyChannelsPlotted  -- True if noisy channels were plotted.
        bandpassFlaggingStage -- Name of the stage used to detect the 'noisy'
                                 channels that have been plotted. 

        """
        self._htmlLogger.timing_start('BaseSlice._plotTitleAndKey')

        pylab.axis('off')

# history

        if self._layout == 'landscape':
            yoff = 0.9
        else:
            yoff = 1.0

        if self._layout == 'landscape':
            yoff = 0.9
            xoff = 0.5
        else:
            xoff = 0.0

# noisy channels

        if noisyChannelsPlotted:
            pylab.errorbar(xoff, yoff, 0.0, fmt='o', color='grey')
            yoff = self._plotText(xoff+0.05, yoff, 
             "noisy channels from stage '%s'" % bandpassFlaggingStage, 35, 
             ny_subplot=ny_subplot, mult=0.8)

# and flagging

        yoff = self._plotText(xoff, yoff, 'Flag key', 30)
        yoff -= 0.02

        pylab.plot([xoff], [yoff], marker='^', markerfacecolor='indigo')
        yoff = self._plotText(xoff+0.05, yoff, 'no data', 35, 
         ny_subplot=ny_subplot, mult=0.8)

        pylab.plot([xoff], [yoff], marker='^', markerfacecolor='green')
        yoff = self._plotText(xoff+0.05, yoff, 'cannot calculate on recipe entry', 35, 
         ny_subplot=ny_subplot, mult=0.8)

        pylab.plot([xoff], [yoff], marker='^', markerfacecolor='violet')
        yoff = self._plotText(xoff+0.05, yoff, 
         'cannot calculate at stage start', 35, ny_subplot=ny_subplot, mult=0.8)

        pylab.plot([xoff], [yoff], marker='^', markerfacecolor='blue')
        yoff = self._plotText(xoff+0.05, yoff, 'cannot calculate now', 35,
         ny_subplot=ny_subplot, mult=0.8)

        for row,flagReason in enumerate(flaggingReason):
            stageName = flagReason['stageDescription']['name']
            if stageName == stageDescription['name']:

# flagged at this reduction stage so give full information on the rules

                if len(flagReason['rules']) == 0:
                    continue
                yoff = self._plotText(xoff, yoff, 'Flagged here:', 35, 
                 ny_subplot=ny_subplot, mult=0.9)
                yoff = self._plotText(xoff+0.02, yoff, 'rules:', 35,
                 ny_subplot=ny_subplot, mult=0.8)
                for rule in flagReason['rules']:
                    if rule.has_key('colour'):
                        pylab.plot([xoff], [yoff], marker='^', markerfacecolor=
                         rule['colour'])
                    if rule.has_key('axis'):
                        yoff = self._plotText(xoff+0.05, yoff,
                         '%s axis - %s' % (rule['axis'], rule['rule']), 35,
                         ny_subplot=ny_subplot, mult=0.8)
                    else:
                        yoff = self._plotText(xoff+0.05, yoff, rule['rule'], 35,
                         ny_subplot=ny_subplot, mult=0.8)
                break

# reset axis limits which otherwise will have been pulled around
# by the plotting of the key

        pylab.axis([0.0,1.0,0.0,1.0])
        self._htmlLogger.timing_stop('BaseSlice._plotTitleAndKey')


    def description(self):
        """Method to return a brief description of the class."""

        description = 'SliceX/Y - display slices across image'
        return description


class SliceX(BaseSlice):
    """Class to plot 2-d data as a sequence of slices in X."""

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

#        print 'SliceX.display called'
        self._htmlLogger.timing_start('SliceX.display')

# get results to display. This needs to be done before the view object is
# asked to describe itself in HTML

        dataResult = dataView.getData()

# write general description of the data view, operation

        flagMessage,colour = self.writeBaseHTMLDescriptionHead(stageDescription,
         dataView, dataOperator)

# display results

        self._htmlLogger.logHTML('<h3>Displays</h3>')

        collection = dataResult['data']
        history = dataResult['parameters']['history']
        flagging = dataResult['flagging']
        flaggingReason = dataResult['flaggingReason']
        flaggingApplied = dataResult['flaggingApplied']

        keys = collection.keys()

        if len(keys) == 0:
            self._htmlLogger.logHTML('Nothing to display')

        for key in keys:
            results = collection[key]
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

# iterate along slices through data

            data = stages[0]['data']
            for i in range(shape(data)[0]):
                pylab.ioff()
                pylab.figure(1)
                pylab.clf()

# plot title

                if description.has_key('TITLE'):
                    title = description['TITLE']

# if more than 1 slice modify title to include id of slice.

                    if shape(data)[0] > 1:
                        if title.find(' - ') > -1:
                            title = title.replace(' - ',
                             ' %s:%s - ' % (stages[0]['ytitle'], self._pad(i)),
                             1)
                        else:
                            title = '%s %s:%s - ' % (title,
                             stages[0]['ytitle'], self._pad(i))

                    pylab.figtext(0.5, 0.95, title, ha='center',
                     fontsize='smaller')

# iterate through collected items and display them in turn

                flagsPlotted = False
                for index, result in enumerate(stages):
                    sub_title = ''
                    if nstages > 1:
                        if index == 0:
                            sub_title = 'IN' 
                        else:
                            sub_title = 'OUT'
                        description[result['ytitle']] = self._pad(i)

                    dataTitle = result['dataType']
                    xtitle = result['xtitle']
                    x = result['x']
                    dataType = result['dataType']
                    dataUnits = result['dataUnits']
                    data = result['data']
                    data_mad = result['mad_floor']
                    flagVersions = result['flagVersions']
                    temp = result['flag']
                    flag = []
                    for fi,fs in enumerate(flagVersions):
                        flag.append(temp[fi][i,:])
                    if result.has_key('noisyChannels'):
                        noisyChannels = result['noisyChannels']
                        bandpassFlaggingStage = result['bandpassFlaggingStage']
                    else:
                        noisyChannels = None
                        bandpassFlaggingStage = None

                    flagsPlotted = self._plot_panel(self._layout, nstages+1,
                     index+1, description, 
                     xtitle, x, dataType, dataUnits, data[i,:], data_mad[i,:],
                     flag, flagVersions, stageDescription,
                     stage_flagging[index], stage_flaggingReason[index],
                     stage_flaggingApplied[index], sub_title, noisyChannels)

# plot the key

                pylab.rcdefaults()
                if self._layout == 'portrait':
                    pylab.subplot(1, nstages+1, nstages+1)
                    ny_subplot = 1
                elif self._layout == 'landscape':
                    pylab.subplot(nstages+1, 1, nstages+1)
                    ny_subplot = nstages + 1

                noisyChannelsPlotted = xtitle.upper() == 'CHANNEL' and \
                 noisyChannels != None
                self._plotTitleAndKey(stageDescription, history,
                 description, flaggingReason[-1], flaggingApplied[-1],
                 ny_subplot, noisyChannelsPlotted, bandpassFlaggingStage)

# construct a suitable filename and save the image

                plotFile = '%s-%s-v-%s_%s_%s.png' % (stageDescription['name'],
                 dataTitle, xtitle, i, title) 
                plotFile = self._pruneFilename(plotFile)
                pylab.savefig(os.path.join(self._plotDirectory, plotFile))
                pylab.clf()
                pylab.close(1)

# add link information to the list

                plotName = title
                self._htmlLogger.appendNode(plotName, os.path.join(
                 self._plotDirectory, plotFile), flagsPlotted)

        self._htmlLogger.flushNode()

# write HTML info that goes after the displays

        self.writeBaseHTMLDescriptionTail(stageDescription, dataView,
         dataOperator, logName)

        self._htmlLogger.timing_stop('SliceX.display')
        return flagMessage, colour, dataResult['parameters']


    def writeGeneralHTMLDescription(self, stageName):
        """Write general description of display.

        Keyword arguments:
        stageName -- The name of the stage using this class.
        """
 
        self._htmlLogger.logHTML('''The data view was a 2-d array; it was 
         displayed as a series of 1-d slices.''')


    def writeDetailedHTMLDescription(self, stageName):

        """Write detailed description of display.

        Keyword arguments:
        stageName -- The name of the stage using this class.
        """
 
        self._htmlLogger.logHTML('''<h3>Display</h3>
         The data to be displayed was a 2-d array. This display of it
         was generated by iterating along its x-axis and at
         each position plotting as a graph a slice along the y-axis. 
         When the x-axis of each slice is 'channels', the slice y-axis is
         scaled to include unflagged data points that are not designated 
         as 'noisy'; this means that some unflagged but 'noisy' channels
         may not be plotted.''')

#         <p>The display was generated by Python class sliceDisplay.SliceX.''')
 

class SliceY(BaseSlice):
    """Class to plot 2-d data as a sequence of slices in Y."""

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

#        print 'SliceY.display called'
        self._htmlLogger.timing_start('SliceY.display')

# get results to display. This need to be done before the view object is
# asked to describe itself in HTML

        dataResult = dataView.getData()

# write general description of the data view, operation

        flagMessage, colour =self.writeBaseHTMLDescriptionHead(stageDescription,
         dataView, dataOperator)

# display results

        self._htmlLogger.logHTML('<h3>Displays</h3>')

        collection = dataResult['data']
        history = dataResult['parameters']['history']
        flagging = dataResult['flagging']
        flaggingReason = dataResult['flaggingReason']
        flaggingApplied = dataResult['flaggingApplied']

        keys = collection.keys()

        for key in keys:
            results = collection[key]
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

# iterate along slices through data

            data = stages[0]['data']
            for y in arange(shape(data)[1]):
                pylab.ioff()
                pylab.figure(1)
                pylab.clf()

# plot title

                if description.has_key('TITLE'):
                    title = description['TITLE']

# if more than 1 slice modify title to include id of slice.

                    if shape(data)[1] > 1:
                        if title.find(' - ') > -1:
                            title = title.replace(' - ',
                             ' %s:%s - ' % (stages[0]['xtitle'], self._pad(y)),
                             1)
                        else:
                            title = '%s %s:%s - ' % (title, 
                             stages[0]['xtitle'], self._pad(y))

# plot title

                    pylab.figtext(0.5, 0.95, title, ha='center',
                     fontsize='smaller')

# iterate through collected items and display them in turn

                flagsPlotted = False
                for index,result in enumerate(stages):
                    sub_title = ''
                    if nstages > 1:
                        if index == 0:
                            sub_title = 'IN' 
                        else:
                            sub_title = 'OUT'
                        description[result['xtitle']] = self._pad(y)

                    dataTitle = result['dataType']
                    xtitle = result['ytitle']
                    x = result['y']

# handle case where each data point has an associated time, which may not
# all fall on a regular axis

                    if xtitle == 'TIME':
                        if index == 0:
                            if result.has_key('data_time_on_entry'):
                                x = result['data_time_on_entry'][:,y]
                        else:
                            if result.has_key('data_time'):
                                x = result['data_time'][:,y]
                    ytitle = result['xtitle']
                    dataType = result['dataType']
                    dataUnits = result['dataUnits']
                    chunks = result['chunks']
                    data = result['data']
                    data_mad = result['mad_floor']
                    flagVersions = result['flagVersions']
                    temp = result['flag']
                    flag = []
                    for fi,fs in enumerate(flagVersions):
                        flag.append(temp[fi][:,y])
                    if result.has_key('noisyChannels'):
                        noisyChannels = result['noisyChannels']
                        bandpassFlaggingStage = result['bandpassFlaggingStage']
                    else:
                        noisyChannels = None
                        bandpassFlaggingStage = None

                    flagsPlotted = self._plot_panel(self._layout, nstages+1,
                     index+1, description, 
                     xtitle, x, dataType, dataUnits, data[:,y], data_mad[:,y],
                     flag, flagVersions, stageDescription,
                     stage_flagging[index], stage_flaggingReason[index],
                     stage_flaggingApplied[index], sub_title, noisyChannels)

# plot the titles and key

                pylab.rcdefaults()
                if self._layout == 'portrait':
                    pylab.subplot(1, nstages+1, nstages+1)
                    ny_subplot = 1
                elif self._layout == 'landscape':
                    pylab.subplot(nstages+1, 1, nstages+1)
                    ny_subplot = nstages + 1

                noisyChannelsPlotted = xtitle.upper() == 'CHANNEL' and \
                 noisyChannels != None
                self._plotTitleAndKey(stageDescription, history,
                 description, flaggingReason[-1], flaggingApplied[-1],
                 ny_subplot, noisyChannelsPlotted, bandpassFlaggingStage)

# construct a suitable filename and save the image

                plotFile = '%s-%s-v-%s_%s_%s.png' % (stageDescription['name'],
                 dataTitle, xtitle, y, title) 
                plotFile = self._pruneFilename(plotFile)
                pylab.savefig(os.path.join(self._plotDirectory, plotFile))
                pylab.clf()
                pylab.close(1)

# add link information to the list

                plotName = title
                self._htmlLogger.appendNode(plotName, os.path.join(
                 self._plotDirectory, plotFile), flagsPlotted)

        self._htmlLogger.flushNode()

# write HTML info that goes after the displays

        self.writeBaseHTMLDescriptionTail(stageDescription, dataView,
         dataOperator, logName)

        self._htmlLogger.timing_stop('SliceY.display')
        return flagMessage, colour, dataResult['parameters']


    def writeGeneralHTMLDescription(self, stageName):
        """Write general description of display.

        Keyword arguments:
        stageName -- The name of the stage using this class.
        """
 
        self._htmlLogger.logHTML('''The data view was a 2-d array. It was 
         displayed as a series of 1-d slices.''')


    def writeDetailedHTMLDescription(self, stageName):

        """Write detailed description of display.

        Keyword arguments:
        stageName -- The name of the stage using this class.
        """
 
        self._htmlLogger.logHTML('''<h3>Display</h3>
         The data to be displayed was a 2-d array. This display of it
         was generated by iterating along its y-axis and at
         each position plotting as a graph a cut along the x-axis.''')

#         <p>The display was generated by Python class sliceDisplay.SliceY.''')
