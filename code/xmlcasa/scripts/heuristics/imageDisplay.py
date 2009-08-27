"""Module to plot images."""

# History:
# 16-Jul-2007 jfl First version.
#  9-Aug-2007 jfl facecolor parameter named in calls to .fill()
# 16-Aug-2007 jfl Change profiling, modify to handle empty dataset.
# 30-Aug-2007 jfl Flux calibrating version.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
# 25-Jun-2008 jfl regression release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 18-Sep-2008 jfl bytes parameter added to __call__.
# 10-Oct-2008 jfl complex display release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import os
from numpy import *
from numpy import ma
import pickle
import pylab
from matplotlib.colors import ColorConverter, Colormap, Normalize

# alma modules

from baseDisplay import *


class _SentinelMap(Colormap):
    """Utility class for plotting sentinel pixels in colours."""

    def __init__(self, cmap, sentinels={}):
        """Constructor.
 
        Keyword arguments:
        """

#        print '_SentinelMap.__init__ called'
        self.name = 'SentinelMap'
        cmap._init()
        self.cmap = cmap
        self._lut = cmap._lut
        self.N = cmap.N
        self.sentinels = sentinels


    def __call__(self, scaledData, alpha=1.0, bytes=False):
        """Utility method.
        """

#        print '_SentinelMap.__call__ called'

        rgbaValues = self.cmap(scaledData)

        for sentinel,rgb in self.sentinels.items():
            r,g,b = rgb 
            if ndim(rgbaValues) == 3:
                rgbaValues[:,:,0] = where(scaledData==sentinel, r,
                 rgbaValues[:,:,0])
                rgbaValues[:,:,1] = where(scaledData==sentinel, g,
                 rgbaValues[:,:,1])
                rgbaValues[:,:,2] = where(scaledData==sentinel, b,
                 rgbaValues[:,:,2])
                rgbaValues[:,:,3] = alpha
            elif ndim(rgbaValues) == 2:
                rgbaValues[:,0] = where(scaledData==sentinel, r,
                 rgbaValues[:,0])
                rgbaValues[:,1] = where(scaledData==sentinel, g,
                 rgbaValues[:,1])
                rgbaValues[:,2] = where(scaledData==sentinel, b,
                 rgbaValues[:,2])
                rgbaValues[:,3] = alpha

        return rgbaValues


class _SentinelNorm(Normalize):
    """Normalise but leave sentinel values unchanged.
    """

    def __init__(self, vmin=None, vmax=None, clip=True, sentinels=[]):
#        print '_SentinelNorm.__init__ called'
        self.vmin = vmin
        self.vmax = vmax
        self.clip = clip
        self.sentinels = sentinels

 
    def __call__(self, value, clip=None):
#        print '_SentinelNorm.__call__ called'

# remove sentinels, keeping a mask of where they were.

        sentinel_mask = zeros(shape(value), bool)
        for sentinel in self.sentinels:
            sentinel_mask += (value==sentinel)
        sentinel_values = value[sentinel_mask]

        actual_data = value[logical_not(sentinel_mask)]
        if len(actual_data):
            value[sentinel_mask] = actual_data.min()
        value = ma.asarray(value)
        value = Normalize.__call__(self, value, clip)

# restore sentinels

        value[sentinel_mask] = sentinel_values
        return value


class ImageDisplay(BaseDisplay):
    """Class to plot images."""

    def _plot_panel(self, nplots, plot_number, description,
     data_in, xtitle, x, ytitle, y, chunks, dataType, dataUnits, flag,
     flagVersions, flagging_in, flaggingApplied_in, sub_title, stageName):
        """Plot the 2d data into one panel.

        Keyword arguments:
        nplots             -- The number of sub-plots on the page.
        plot_number        -- The index of this sub-plot.
        description        -- Dictionary whose entries describe the data to be
                           -- plotted.
        data_in            -- The 2d data.
        xtitle             -- The name of the x-axis.
        x                  -- The values of the x-axis.
        ytitle             -- The name of the y-axis.
        y                  -- The values of the y-axis.
        chunks             -- If the y-axis is time, then the chunks are a list
                              of contiguous sets of integrations.
        dataType           -- Data type.
        dataUnits          -- Data units.
        flag               -- List of flags for data, one for each flag version.
        flagVersions       -- Flagging version name for each item in flag list.
        flagging_in        -- List of flagging commands applied to data.
        flaggingApplied_in -- True if corresponding 'flagging_in' has been
                              been applied to the data.
        sub_title          -- The title to be given to this subplot.
        stageName          -- The name of the recipe stage using the object.
        """  

#        print 'ImageDisplay._plot_panel called'
        self._htmlLogger.timing_start('imageDisplay._plot_panel')

# work on a copy of the data

        data = array(data_in)

        cc = ColorConverter()
        sentinels = {}

# ..set sentinels at points with no data/violet. These should be
#   overwritten by other flag colours in a moment.

        if flagVersions.count('Current') > 0:
            i_current = flagVersions.index('Current')
            data[flag[i_current]!=0] = 2.0
            sentinels[2.0] = cc.to_rgb('violet')

# ..sentinels to mark flagging.

# look at the flags in reverse so that the first time a point is
# flagged is what appears on the plot. Work on local copies to avoid
# unpredictable effects on global data.

        flagging = list(flagging_in)
        flaggingApplied = list(flaggingApplied_in)
        flagging.reverse()
        flaggingApplied.reverse()
        y_range = arange(len(y))
        x_range = arange(len(x))
        ant_v_ant = -ones([len(x), len(y)])
        data_desc_id = int(description['DATA_DESC_ID'])
        field_id = int(description['FIELD_ID'])
        if description.has_key('ANTENNA1'):
            plot_antenna = int(description['ANTENNA1'])
        else:
            plot_antenna = None

        sentinel_value = 10.0
        pixelsFlagged = False
        for row,flag_row in enumerate(flagging):
            if not(flaggingApplied[row]):
                continue
            for rule,flag_list in flag_row.iteritems():

# do these flags apply to this source, data_desc?

                for flag_cmd in flag_list:

# flags set in stages before this one are not different colours

                    if flag_cmd['stageName'] != stageName:
                        continue

# ignore if flag does not have colour associated with it

                    if not(flag_cmd.has_key('colour')):
                        continue

# basic checks of flag applicability

                    if flag_cmd.has_key('FIELD_ID'):
                        if not(flag_cmd['FIELD_ID'].count(int(field_id))):
                            continue

                    if flag_cmd.has_key('DATA_DESC_ID'):
                        if flag_cmd['DATA_DESC_ID'] != data_desc_id:
                            continue

# ready to set flags

                    flag_x = []
                    flag_y = []
                    sentinel_value += 1.0

# check axis combinations in turn

                    if xtitle.upper() == 'ANTENNA' and ytitle.upper() == "TIME":
                        if flag_cmd.has_key('CHANNELS'):
                            continue

                        if flag_cmd.has_key('TIME'):
                            flag_y = flag_cmd['TIME']
                        else:
                            flag_y = y
                        if flag_cmd.has_key('TIME_INTERVAL'):
                            flag_y_interval = flag_cmd['TIME_INTERVAL']
                        else:
                            flag_y_interval = ones(shape(flag_y), float) * 0.002

                        if flag_cmd.has_key('ANTENNA'):
                            flag_x = flag_cmd['ANTENNA']
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

                        if flag_cmd.has_key('ANTENNA1'):
                            flag_y = flag_cmd['ANTENNA1']
                            flag_x = flag_cmd['ANTENNA2']

                            if list(flag_cmd['ANTENNA1']).count(plot_antenna) > 0:
                                flag_x = flag_cmd['ANTENNA2']
                                fset = self._setSentinel(x_range, y_range, x, y, data,
                                 flag_x, flag_y, flag_y_interval,
                                 sentinel_value) 
                                pixelsFlagged = pixelsFlagged or fset

                            if list(flag_cmd['ANTENNA2']).count(plot_antenna) > 0:
                                flag_x = flag_cmd['ANTENNA1']
                                fset = self._setSentinel(x_range, y_range, x, y, data,
                                 flag_x, flag_y, flag_y_interval,
                                 sentinel_value) 
                                pixelsFlagged = pixelsFlagged or fset

                    elif xtitle.upper() == 'ANTENNA2' and \
                     ytitle.upper() == "TIME":
                        if flag_cmd.has_key('CHANNELS'):
                            continue

                        if flag_cmd.has_key('TIME'):
                            flag_y = flag_cmd['TIME']
                        else:
                            flag_y = y
                        if flag_cmd.has_key('TIME_INTERVAL'):
                            flag_y_interval = flag_cmd['TIME_INTERVAL']
                        else:
                            flag_y_interval = ones(shape(flag_y), float) * 0.002

                        if flag_cmd.has_key('ANTENNA'):
                            flag_x = flag_cmd['ANTENNA']
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

                        if flag_cmd.has_key('ANTENNA1'):
                            if list(flag_cmd['ANTENNA1']).count(plot_antenna) > 0:
                                flag_x = flag_cmd['ANTENNA2']
                                fset = self._setSentinel(x_range, y_range, x, y,
                                 data, flag_x, flag_y, flag_y_interval,
                                 sentinel_value) 
                                pixelsFlagged = pixelsFlagged or fset

                            if list(flag_cmd['ANTENNA2']).count(plot_antenna) > 0:
                                flag_x = flag_cmd['ANTENNA1']
                                fset = self._setSentinel(x_range, y_range, x, y,
                                 data, flag_x, flag_y, flag_y_interval,
                                 sentinel_value) 
                                pixelsFlagged = pixelsFlagged or fset

                    elif xtitle.upper() == 'CHANNEL' and ytitle.upper() == \
                     'ANTENNA2':
                        if flag_cmd.has_key('TIME'):
                            continue

# currently same channels are flagged in all antennas

                        if flag_cmd.has_key('CHANNELS'):
                            flag_x = list(flag_cmd['CHANNELS'])
                            flag_y = list(y)
                            flag_y_interval = ones(shape(flag_y), float) * 0.1
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

# all data for a range of antennas

                        if flag_cmd.has_key('ANTENNA'):
                            flag_y = list(flag_cmd['ANTENNA'])
                            flag_y_interval = ones(shape(flag_y), float) * 0.1
                            flag_x = list(x)
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

# all data for some baselines

                        if flag_cmd.has_key('ANTENNA1'):
                            if list(flag_cmd['ANTENNA1']).count(plot_antenna) > 0:
                                flag_y = list(flag_cmd['ANTENNA2'])
                            else:
                                flag_y = []
                            flag_y_interval = ones(shape(flag_y), float) * 0.1
                            flag_x = list(x)
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

                        if flag_cmd.has_key('ANTENNA2'):
                            if list(flag_cmd['ANTENNA2']).count(plot_antenna) > 0:
                                flag_y = list(flag_cmd['ANTENNA1'])
                            else:
                                flag_y = []
                            flag_y_interval = ones(shape(flag_y), float) * 0.1
                            flag_x = list(x)
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

                    elif xtitle.upper() == 'ANTENNA2' and ytitle.upper() == \
                     'ANTENNA1':

# don't overplot if flag is for a channel subset

                        if flag_cmd.has_key('CHANNELS'):
                            continue

# do overplot if an ANTENNA is flagged for all TIMEs
 
                        if flag_cmd.has_key('ANTENNA') and not(
                         flag_cmd.has_key('TIME')):
                            flag_x = flag_cmd['ANTENNA']
                            flag_y = y
                            flag_y_interval = ones(shape(flag_y), float) * 0.1
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

                            flag_y = flag_cmd['ANTENNA']
                            flag_x = x
                            flag_y_interval = ones(shape(flag_y), float) * 0.1
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

# overplot baseline flags

                        if flag_cmd.has_key('ANTENNA1') and \
                         not(flag_cmd.has_key('TIME')):
                            flag_y = flag_cmd['ANTENNA1']
                            flag_x = flag_cmd['ANTENNA2']
                            flag_y_interval = ones(shape(flag_y), float) * 0.1
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

                            flag_x = flag_cmd['ANTENNA1']
                            flag_y = flag_cmd['ANTENNA2']
                            flag_y_interval = ones(shape(flag_y), float) * 0.1
                            fset = self._setSentinel(x_range, y_range, x, y, data,
                             flag_x, flag_y, flag_y_interval, sentinel_value) 
                            pixelsFlagged = pixelsFlagged or fset

                            flag_x = []
                            flag_y = []

                    colour = flag_cmd['colour']
                    sentinels[sentinel_value] = cc.to_rgb(colour)

# ..plot points flagged on stage entry violet

        if flagVersions.count('StageEntry') > 0:
            i_stageentry = flagVersions.index('StageEntry')
            data[flag[i_stageentry]!=0] = 4.0
            sentinels[4.0] = cc.to_rgb('violet')

# ..plot points flagged on recipe entry green. 

        if flagVersions.count('BeforeHeuristics') > 0:
            i_original = flagVersions.index('BeforeHeuristics')
            data[flag[i_original]!=0] = 3.0
            sentinels[3.0] = cc.to_rgb('green')

# ..plot points with no data indigo. 

        if flagVersions.count('NoData') > 0:
            i_original = flagVersions.index('NoData')
            data[flag[i_original]!=0] = 5.0
            sentinels[5.0] = cc.to_rgb('indigo')

# ..set my own colormap and normalise to plot sentinels

        cmap = _SentinelMap(pylab.cm.gray, sentinels=sentinels)
        norm = _SentinelNorm(sentinels=sentinels.keys())

# ..calculate vmin, vmax without the sentinels. Leaving norm to do
#   this is not sufficient; the standard Normalize gets called by something
#   in matplotlib and initialises vmin and vmax incorrectly.
 
        sentinel_mask = zeros(shape(data), bool)
        for sentinel in sentinels.keys():
            sentinel_mask += (data==sentinel)
        actual_data = data[logical_not(sentinel_mask)]
        if len(actual_data):
            vmin = actual_data.min()
            vmax = actual_data.max()
        else:
            vmin = vmax = 0.0

# make antenna x antenna plots square

        aspect = 'auto'
        shrink = 0.6
        fraction = 0.15
        if (xtitle.upper().find('ANTENNA') > -1) and \
         (ytitle.upper().find('ANTENNA') > -1):
            aspect = 'equal'
            shrink = 0.4
            fraction = 0.1

# plot data - imshow requires indeces to be transposed

        pylab.subplot(1, nplots, plot_number)
        pylab.imshow(data, cmap=cmap, norm=norm, vmin=vmin, vmax=vmax,
         interpolation='nearest',
         origin='lower', extent=[-0.5, shape(data)[1]-0.5, -0.5,
         shape(data)[0]-0.5], aspect=aspect)
        lims = pylab.axis()
        pylab.xlabel(xtitle, size=10)
        for label in pylab.gca().get_xticklabels():
            label.set_fontsize(8)
        pylab.title(sub_title)
        if plot_number==1:
            pylab.ylabel(ytitle, size=10)
        for label in pylab.gca().get_yticklabels():
            label.set_fontsize(8)

# ..plot wedge, make tick numbers smaller, label with units

        if vmin==vmax:
            cb = pylab.colorbar(shrink=shrink, fraction=fraction,
             ticks=[-1,0,1])
        else:
            if (vmax - vmin > 0.001) and (vmax - vmin) < 10000:
                cb = pylab.colorbar(shrink=shrink, fraction=fraction)
            else:
                cb = pylab.colorbar(shrink=shrink, fraction=fraction,
                 format='%.1e')
        for label in cb.ax.get_yticklabels():
            label.set_fontsize(6)
        if plot_number==1:
            if dataUnits != '':
                cb.set_label('%s (%s)' % (dataType, dataUnits), fontsize=7)
            else:
                cb.set_label(dataType, fontsize=6)

# ..overplot chunk boundaries, do this last otherwise boundaries
#   can get overwritten. For now assumes that y axis is time.

        if plot_number==1:
            if ytitle == 'TIME':
                pylab.gca().yaxis.set_major_locator(pylab.NullLocator())
                base_time = 86400.0*floor (y[0]/86400.0)
                tim_plot = y - base_time
                for chunk in chunks:
                    t = tim_plot[chunk[0]]
                    h = int(floor(t/3600.0))
                    t -= h * 3600.0
                    m = int(floor(t/60.0))
                    t -= m * 60.0
                    s = int(floor(t))
                    tstring = '%sh%sm%ss' % (h,m,s)     
                    pylab.axhline(chunk[0]-0.5, color='white')
                    pylab.axhline(chunk[-1]+0.5, color='white')
                    pylab.text(lims[0]-0.25, chunk[0]-0.5, tstring, fontsize=7,
                      ha='right', va='bottom')

                if len(chunks) > 1:
                    if len(chunks[-1]) > 1:

# plot time for last chunk

                        t = tim_plot[chunks[-1][-1]]
                        h = int(floor(t/3600.0))
                        t -= h * 3600.0
                        m = int(floor(t/60.0))
                        t -= m * 60.0
                        s = int(floor(t))
                        tstring = '%sh%sm%ss' % (h,m,s)     
                        pylab.axhline(chunks[-1][-1]+0.5, color='white')
                        pylab.text(lims[0]-0.25, chunks[-1][-1]-0.5, tstring,
                         fontsize=7, ha='right', va='bottom')
        else:

# no y-axis ticks on 2nd plot.

            pylab.gca().yaxis.set_major_locator(pylab.NullLocator())

# reset lims to values for image, stops box being pulled off edge
# of image by other plotting commands.

        pylab.axis(lims)
        self._htmlLogger.timing_stop('imageDisplay._plot_panel')
        return pixelsFlagged


    def _setSentinel(self, x_range, y_range, x, y, data, flag_x, flag_y,
     flag_y_interval, sentinel_value): 
        """Set pixels to sentinel value. Returns True if any values
        are actually set.

        Keyword arguments:
        x_range         -- Precomputed range(len(x)). 
        y_range         -- Precomputed range(len(y)).
        x               -- x-axis.
        y               -- y-axis
        data            -- data array.
        flag_x          -- |data at these values of x,y to be flagged. 
        flag_y          -- |
        flag_y_interval -- The 'width' in y of each measurement.
        sentinel_value  -- The sentinel value to set in the target data.
        """
        sentinelSet = False

        for xval in flag_x:
            xf = compress(abs(xval-x) < 0.001, x_range)[0]

            y_flag_indeces = []
            for iy,yval in enumerate(flag_y):
                flag_index = compress(abs(yval-y) < 
                 flag_y_interval[iy]/2.0, y_range)
                if len(flag_index) == 0:
                    continue
                y_flag_indeces.append(flag_index[0])

                for yf in y_flag_indeces:
                    data[yf,xf] = sentinel_value
                    sentinelSet = True

        return sentinelSet


    def description(self):
        """Method to return a brief description of the class."""

        description = '''ImageDisplay - display a 2-D dataset as an image'''
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

#        print 'ImageDisplay.display called'

        self._htmlLogger.timing_start('imageDisplay.display')

# get results to display. This needs to be done before the view object is
# asked to describe itself in HTML

        dataResult = dataView.getData()

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

            flagsSet = False
            for index, result in enumerate(stages):
                sub_title = ''
                if nstages > 1:
                    if index == 0:
                        sub_title = 'IN' 
                    else:
                        sub_title = 'OUT'

                if len(result) ==  0:
                    dataType = 'no data'
                    continue
 
                dataType = result['dataType']
                xtitle = result['xtitle']
                x = result['x']
                ytitle = result['ytitle']
                y = result['y']
                chunks = []
                if result.has_key('chunks'):
                    chunks = result['chunks']
                data = result['data']
                dataUnits = result['dataUnits']
                flag = result['flag']
                flagVersions = result['flagVersions']
                flagsSet = self._plot_panel(nstages+1, plot_number,
                 description, 
                 data, xtitle, x, ytitle, y, chunks, 
                 dataType, dataUnits, flag,
                 flagVersions, stage_flagging[index],
                 stage_flaggingApplied[index], sub_title,
                 stageDescription['name'])
                plot_number += 1

# plot the titles and key

            pylab.subplot(1, nstages+1, plot_number)
            pylab.axis('off')
            yoff = 1.1
            yoff = self._plotText(0.1, yoff, 'STAGE: %s' % 
             stageDescription['name'], 40)
            title = 'notitle'
            if description.has_key('TITLE'):
                title = description['TITLE']
                yoff = self._plotText(0.1, yoff, title, 35)

# flagging

            yoff -= 0.1

            ax = pylab.gca()
            ax.fill([0.1, 0.2, 0.2, 0.1], [yoff, yoff,
             yoff+1.0/80.0, yoff+1.0/80.0], facecolor='indigo', 
             edgecolor='indigo')
            yoff = self._plotText(0.25, yoff, 'No data', 35, mult=0.9)

            yoff = self._plotText(0.1, yoff, 'Flagged at previous stages:', 35,
             mult=0.9)
            ax = pylab.gca()
            ax.fill([0.1, 0.2, 0.2, 0.1], [yoff, yoff,
             yoff+1.0/80.0, yoff+1.0/80.0], facecolor='green', 
             edgecolor='green')
            yoff = self._plotText(0.25, yoff, 'cannot calculate on recipe entry',
             45, mult=0.8)

# stage entry flag version

            ax.fill([0.1, 0.2, 0.2, 0.1], [yoff, yoff, yoff+1.0/80.0,
             yoff+1.0/80.0], facecolor='violet', edgecolor='violet')
            yoff = self._plotText(0.25, yoff, 'cannot calculate at stage entry',
             45, mult=0.8)

# data that cannot be calculated

            ax.fill([0.1, 0.2, 0.2, 0.1], [yoff, yoff,
             yoff+1.0/80.0, yoff+1.0/80.0], facecolor='blue',
             edgecolor='blue')
            yoff = self._plotText(0.25, yoff, 'cannot calculate now', 45,
             mult=0.8)

# flagged during this stage

            for row,flagReason in enumerate(flaggingReason[-1]):
                stageName = flagReason['stageDescription']['name']
                if stageName == stageDescription['name']:
                    if not(flaggingApplied[-1][row]):
                        continue

                    if len(flagReason['rules']) == 0:
                        continue
                    yoff = self._plotText(0.1, yoff, 'Flagged here:', 35, 
                     mult=0.9)
                    yoff = self._plotText(0.1, yoff, 'rules:', 45, mult=0.8)
                    for rule in flagReason['rules']:
                        ax.fill([0.1, 0.2, 0.2, 0.1], [yoff, yoff,
                         yoff+1.0/80.0, yoff+1.0/80.0],
                         facecolor=rule['colour'], edgecolor=rule['colour'])
                        if rule.has_key('axis'):
                            yoff = self._plotText(0.25, yoff, '%s axis - %s' % 
                             (rule['axis'], rule['rule']), 45, mult=0.8)
                        else:
                            yoff = self._plotText(0.25, yoff, rule['rule'], 45,
                             mult=0.8)
                    break

# reset axis limits which otherwise will have been pulled around
# by the plotting of the key

            pylab.axis([0.0,1.0,0.0,1.0])

# save the image (remove odd characters from filename to cut down length)

            plotFile = '%s_%s_%s_v_%s_%s.png' % (stageDescription['name'],
             dataType, ytitle, xtitle, title) 
            plotFile = self._pruneFilename(plotFile)
            pylab.savefig(os.path.join(self._plotDirectory, plotFile))
            plotName = title
            self._htmlLogger.appendNode(plotName, os.path.join(
             self._plotDirectory, plotFile), flagsSet)

            pylab.clf()
            pylab.close(1)

        self._htmlLogger.flushNode()

# write the html info to go after the displays

        self.writeBaseHTMLDescriptionTail(stageDescription, dataView,
         dataOperator, logName)

# pass out parameters from data view, if present
        
        if dataResult.has_key('parameters'):
            dataParameters = dataResult['parameters']
        else:
            dataParameters = None

        self._htmlLogger.timing_stop('imageDisplay.display')
        return flagMessage, colour, dataParameters


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the display.
                 
        Keyword arguments:
        stageName -- The name of the recipe stage using the object.
        """
                
        self._htmlLogger.logHTML('''The data view was shown as a set of
         greyscale images. Flagged pixels were colour coded for reason.''')


    def writeDetailedHTMLDescription(self, stageName):
        """Write a detailed description of the display.

        Keyword arguments:
        stageName -- The name of the recipe stage using the object.
        """

        self._htmlLogger.logHTML("""<h3>Display</h3>
         The data view was shown as a set of greyscale images, scaled between 
         the maximum and minimum unflagged values.
         <p>If data were flagged during this reduction stage then
         each display contains 2 images; one displaying the data view before
         flagging, the other after flagging. The reasons for data having
         been flagged are colour coded.""")
#         <p>The display was produced by Python class ImageDisplay.
#         """)

