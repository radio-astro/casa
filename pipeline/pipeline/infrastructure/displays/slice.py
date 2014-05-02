from __future__ import absolute_import
import re
import os
import string

import matplotlib.ticker as ticker
from matplotlib.colors import ColorConverter, Colormap, Normalize
import numpy as np
from numpy import ma
import pylab as plt

import pipeline.infrastructure as infrastructure
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.renderer.logger as logger

LOG = infrastructure.get_logger(__name__)

_valid_chars = "_.%s%s" % (string.ascii_letters, string.digits)
def _char_replacer(s):
    '''A small utility function that echoes the argument or returns '_' if the
    argument is in a list of forbidden characters.
    '''
    if s not in _valid_chars:
        return '_'
    return s

def sanitize(text):
    filename = ''.join(_char_replacer(c) for c in text)
    return filename

flag_color = {'edges':'lightblue',
              'high outlier':'orange',
              'low outlier':'yellow',
              'max abs':'pink',
              'min abs':'darkcyan',
              'nmedian':'darkred',
              'outlier': 'red',
              'sharps':'green',
              'sharps2':'green',
              'diffmad': 'red',
              'tmf': 'aqua'}

class SliceDisplay(object):

    def plot(self, context, results, reportdir, description_to_plot=None,
      overplot_spectrum=None, plotbad=True, plot_only_flagged=False,
      prefix=''):

        if not results:
            return []

        stagenumber = context.stage

        plots = []        

        if description_to_plot is None:
            # plot all results
            descriptionlist = results.descriptions()
            descriptionlist.sort()
        else:
            # plot just this one
            descriptionlist = [description_to_plot]

        flagcmds = results.flagcmds()

        for description in descriptionlist:
            # decide if we want to plot this result
            plot_result = True
            if plot_only_flagged:
                plot_result = False
                for flagcmd in flagcmds:
                    if flagcmd.match(results.last(description)):
                        plot_result = True
                        break

            if not plot_result:
                continue

            # bug here somewhere - pol has 3 possible values for Tsyscal;
            # None, Pol1 or Pol2 but only None and Pol1 get displayed as
            # buttons.
            # Not fixed as reporting is to be rewritten.
            # save the image (remove odd characters from filename to cut
            # down length)
            xtitle = results.first(description).axis.name
            ytitle = results.first(description).datatype
            plotfile = '%s_%s_%s_v_%s_%s.png' % (prefix,
              results.first(description).datatype, ytitle, xtitle,
              description)
            plotfile = sanitize(plotfile)
            plotfile = os.path.join(reportdir, plotfile)

            ant = results.first(description).ant
            if ant is not None:
                ant = ant[1]

            plot = logger.Plot(plotfile, x_axis=xtitle, y_axis=ytitle,
              field=results.first(description).fieldname,
              parameters={ 'spw': results.first(description).spw,
              'pol': results.first(description).pol,
              'ant': ant,
              'type': results.first(description).datatype,
              'file': os.path.basename(results.first(description).filename)})
            plots.append(plot)

            if os.path.exists(plotfile):
                LOG.trace('Not overwriting existing image at %s' % plotfile)
                continue

            # do the plot
            if results.flagging:
                nsubplots = 3
                ignore = self._plot_panel(nsubplots, 1,
                  description, results.first(description),
                  'Before Flagging', stagenumber,
                  overplot_spectrum=overplot_spectrum,
                  plotbad=plotbad)

                # and 'after flagging'
                flagsSet = self._plot_panel(nsubplots, 2,
                  description, results.last(description),
                  'After', stagenumber, plotbad=plotbad,
                  flagcmds=flagcmds)
            else:
                nsubplots = 2
                ignore = self._plot_panel(nsubplots, 1,
                  description, results.first(description),
                  '', stagenumber,
                  overplot_spectrum=overplot_spectrum,
                  plotbad=plotbad, flagcmds=flagcmds)

            # plot the titles and key
            plt.subplot(nsubplots, 1, nsubplots)
            yoff = 0.9
            xoff = 0.5

            yoff = self.plottext(xoff, yoff, 'STAGE: %s' %
              stagenumber, 40, ny_subplot=nsubplots, mult=1.8)
            yoff = self.plottext(xoff, yoff, description, 35,
              ny_subplot=nsubplots, mult=1.8)

            yoff -= 0.1
            yoff = self.plottext(xoff, yoff, 'Key', 30, mult=2.0)
            yoff -= 0.015

            if overplot_spectrum is not None:
                # overplot key
                overplot_datatype = overplot_spectrum.datatype

                plt.plot([xoff], [yoff], marker='o', markersize=5,
                  markerfacecolor='lightblue', markeredgecolor='lightblue')
                yoff = self.plottext(xoff+0.05, yoff, overplot_datatype, 35,
                  ny_subplot=nsubplots, mult=1.6)

            # flagging
            plt.plot([xoff], [yoff], marker='o', markerfacecolor='indigo',
              markeredgecolor='indigo', markersize=5, clip_on=False)
            yoff = self.plottext(xoff+0.05, yoff, 'no data', 35,
              ny_subplot=nsubplots, mult=1.6)

            plt.plot([xoff], [yoff], marker='o', markerfacecolor='blue',
              markeredgecolor='blue', markersize=5, clip_on=False)
            yoff = self.plottext(xoff+0.05, yoff, 'already flagged', 35,
              ny_subplot=nsubplots, mult=1.6)

            # key for data flagged during this stage
            if len(flagcmds) > 0:
                ax = plt.gca()
                rulesplotted = set()
                
                for flagcmd in flagcmds:
                    if flagcmd.rulename == 'ignore':
                        continue

                    if (flagcmd.rulename, flagcmd.ruleaxis,
                      flag_color[flagcmd.rulename]) not in rulesplotted:

                        plt.plot([xoff], [yoff], linestyle='None', marker='o',
                          markersize=5, alpha=0.5,
                          markerfacecolor=flag_color[flagcmd.rulename],
                          markeredgecolor=flag_color[flagcmd.rulename],
                          clip_on=False)

                        if flagcmd.ruleaxis is not None:
                            yoff = self.plottext(xoff+0.05, yoff,
                              '%s axis - %s' %
                              (flagcmd.ruleaxis, flagcmd.rulename), 45,
                              ny_subplot=nsubplots, mult=1.6)
                        else:
                            yoff = self.plottext(xoff+0.05, yoff,
                              flagcmd.rulename, 45,
                              ny_subplot=nsubplots, mult=1.6)

                        rulesplotted.update([(flagcmd.rulename, flagcmd.ruleaxis,
                          flag_color[flagcmd.rulename])])

            # do not print axes, turn off autoscaling
            plt.axis([0, 1, 0, 1])
            plt.axis('off')

            plt.savefig(plotfile)

            plt.clf()
            plt.close(1)

        return plots

    def _plot_panel(self, nplots, plotnumber, description,
      spectrum, subtitle, stagenumber, layout='landscape',
      lineregions=None, spectrumlineregions=None,
      plotbad=True, overplot_description=None, overplot_spectrum=None,
      flagcmds=[]):
        """Plot the 2d data into one panel.

        Keyword arguments:
        nplots             -- The number of sub-plots on the page.
        plotnumber         -- The index of this sub-plot.
        description        -- Title associated with image.
        spectrum           -- The 1d data.
        subtitle           -- The title to be given to this subplot.
        stagenumber        -- The number of the recipe stage using the object.
        plotbad            -- True to plot flagged data, False to 
                              plot a 'floating' symbol at the x position.
        flagcmds           -- List of FlagCmd flagging operations done
                              to the data in this stage.
        """  

        data = spectrum.data
        data_mad = spectrum.data_mad
        flag = spectrum.flag
        nodata = spectrum.nodata
        noisy = spectrum.noisychannels
        xtitle = spectrum.axis.name
        xunits = spectrum.axis.units
        x = spectrum.axis.data
        dataunits = spectrum.units
        datatype = spectrum.datatype
        spw = spectrum.spw

        if overplot_spectrum is not None:
            overplot_data = overplot_spectrum.data
            overplot_data_mad = overplot_spectrum.data_mad
            overplot_flag = overplot_spectrum.flag

        # get time axis in sensible units (hours since beginning of day) -
        # if x-axis is TIME
        if xtitle.upper() == 'TIME':
            day = np.floor(x[0]/86400.0)
            xaxis = np.array(x - day * 86400.0)
        else:
            xaxis = np.array(x)

        # set the font sizes etc.
        if layout == 'landscape':
            plt.rc('axes', titlesize=12)
            plt.rc('axes', labelsize=10)
            plt.rc('xtick', labelsize=10)
            plt.rc('ytick', labelsize=10)
            plt.subplots_adjust(hspace=0.35)

        # set the plot layout
        if layout == 'portrait':
            plt.subplot(1, nplots, plotnumber)
        elif layout == 'landscape':
            plt.subplot(nplots, 1, plotnumber)
        else:
            raise NameError, 'bad layout: %s' % layout

        not_noisy = np.logical_not(noisy)

        # plot 'not noisy' points first 
        if overplot_spectrum is not None:
            # plot overplot data as light blue circles first
            plot_data = overplot_data[not_noisy]
            plot_data_mad = overplot_data_mad[not_noisy]
            plot_flag = overplot_flag[not_noisy]
            plot_axis = xaxis[not_noisy]
            if len(plot_axis[np.logical_not(plot_flag)]) > 0:
                plt.errorbar(plot_axis[np.logical_not(plot_flag)],
                  plot_data[np.logical_not(plot_flag)],
                  plot_data_mad[np.logical_not(plot_flag)],
                  linestyle='None',
                  marker='o', markersize=5, color='lightblue') 

        datemin = None
        datemax = None

        # plot good data as circles
        plot_data = data[not_noisy]
        plot_data_mad = data_mad[not_noisy]
        plot_flag = flag[not_noisy]
        plot_axis = xaxis[not_noisy]
        
        good_data = plot_data[np.logical_not(plot_flag)]
        good_data_mad = plot_data_mad[np.logical_not(plot_flag)]
        if len(good_data) > 0:
            good_x = plot_axis[np.logical_not(plot_flag)]
            if xtitle.upper() == 'TIME':
                temp = []
                for item in good_x:
                    temp.append(datetime.datetime.utcfromtimestamp(item))
                good_x = temp
                datemin = min(good_x)
                datemax = max(good_x)
            plt.errorbar(good_x, good_data, good_data_mad, linestyle='None',
              marker='o', markersize=5, color='black') 

        plt.title(subtitle, fontsize=10)
        if xunits is None:
            plt.xlabel(xtitle, fontsize=10)
        else:
            plt.xlabel('%s (%s)' % (xtitle, xunits), fontsize=10)
        if plotnumber == 1:
            if dataunits != None:
                plt.ylabel('%s (%s)' % (datatype, dataunits), fontsize=10)
            else:
                plt.ylabel(datatype, fontsize=10)

        # plot points not valid in blue. Often these will be zero having
        # simply not been calculated because the underlying data are bad; this
        # screws up the autoscaling. Hence if plotbad is False, plot them 
        # floating at the correct x position above the bottom of the plot, to 
        # suggest that the y value is unimportant. If plotbad is True then
        # plot the actual value.
        ymin, ymax = plt.ylim()
        yflag = ymin + (ymax - ymin) / 10.0
        bad_data = plot_data[plot_flag==True]
        if len(bad_data) > 0:
            if not plotbad:
                bad_data[:] = yflag 
            bad_x = plot_axis[plot_flag==True]
            if xtitle.upper() == 'TIME':
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
            plt.errorbar(bad_x, bad_data, linestyle='None', marker='o',
              markersize=5, markerfacecolor='blue', markeredgecolor='blue')

        # fix the y-scale at the current values, defined by the data we want
        # to be sure of seeing
        ymin, ymax = plt.ylim()

        # overplot noisy channels as triangles of the appropriate colour
        if overplot_spectrum is not None:
            plot_data = overplot_data[noisy]
            plot_data_mad = overplot_data_mad[noisy]
            plot_flag = overplot_flag[noisy]
            plot_axis = xaxis[noisy]
            if len(plot_flag) > 0:
                plt.errorbar(plot_axis[np.logical_not(plot_flag)],
                  plot_data[np.logical_not(plot_flag)],
                  plot_data_mad[np.logical_not(plot_flag)],
                  linestyle='None',
                  marker='^', markersize=5, color='lightblue') 

        # noisy channel data
        plot_data = data[noisy]
        plot_data_mad = data_mad[noisy]
        plot_flag = flag[noisy]
        plot_axis = xaxis[noisy]
        if len(plot_data[np.logical_not(plot_flag)]) > 0:
            plt.errorbar(plot_axis[np.logical_not(plot_flag)],
              plot_data[np.logical_not(plot_flag)],
              plot_data_mad[np.logical_not(plot_flag)],
              linestyle='None',
              marker='^', markersize=5, color='black') 

        # flagged noisy channel data
        if len(plot_data[plot_flag]) > 0:
            plt.errorbar(plot_axis[plot_flag],
              plot_data[plot_flag],
              plot_data_mad[plot_flag],
              linestyle='None',
              marker='^', markersize=5, color='blue') 
        
        if xtitle.upper() == 'CHANNEL' and lineregions is not None:
            lineRanges = []
            for lineregion in lineregions:
                lineRanges.append((lineregion[0],
                  lineregion[1]-lineregion[0]))
            plt.broken_barh(lineRanges, (ymin, (ymax-ymin)/20.0),
              facecolors='pink', edgecolors='pink')

        if spectrumlineregions is not None:
            lineRanges = []
            for lineregion in spectrumlineregions:
                qlr0 = casatools.quanta.quantity(lineregion[0])
                qlr0 = casatools.quanta.convert(qlr0, 'Hz')
                qlr0 = casatools.quanta.getvalue(qlr0) 
                qlr1 = casatools.quanta.quantity(lineregion[1])
                qlr1 = casatools.quanta.convert(qlr1, 'Hz')
                qlr1 = casatools.quanta.getvalue(qlr1) 
                lineRanges.append((qlr0, qlr1-qlr0))
            plt.broken_barh(lineRanges, (ymin, (ymax-ymin)/20.0),
              facecolors='blue', edgecolors='blue')  

        # overplot points flagged in this stage
        pointsflagged = bool(len(flagcmds))
        for flagcmd in flagcmds:
            if flagcmd.match(spectrum):
                bad_data = data[flagcmd.flagchannels]
                if len(bad_data) and not plotbad:
                    bad_data[:] = yflag 
                bad_x = xaxis[flagcmd.flagchannels]
                plt.errorbar(bad_x, bad_data, linestyle='None', marker='o',
                  markersize=5, alpha=0.5, 
                  markerfacecolor=flag_color[flagcmd.rulename],
                  markeredgecolor=flag_color[flagcmd.rulename])

        # overplot points with no data in indigo
        bad_data = data[nodata==True]
        if len(bad_data) > 0:
            bad_x = xaxis[nodata==True]
            if xtitle.upper() == 'TIME':
                temp = []
                for item in bad_x:
                    temp.append(datetime.datetime.utcfromtimestamp(item))
                bad_x = temp
            bad_data[:] = yflag
            plt.errorbar(bad_x, bad_data, linestyle='None', marker='o',
              markerfacecolor='indigo', markeredgecolor='indigo',
              markersize=5)

        # set plot y scale to saved values
        plt.gca().set_ylim(ymin, ymax)

        # turn off possible use of offset in ScalarFormatter on y axis -
        # otherwise if the y range is much smaller than the abs value then
        # the default ticks are relative to an offset printed at the top of
        # the axis, which has led to confusion
        plt.gca().yaxis.set_major_formatter(ticker.ScalarFormatter(
          useOffset=False))

        if xtitle.upper() == 'TIME':

            # set the limits of the plot to whole hours
            datemin = datemin.replace(minute=0, second=0, microsecond=0)

            # following will have problems at very end of month
            if datemax.hour < 23:
                datemax = datemax.replace(hour=datemax.hour + 1, minute=0,
                  second=0, microsecond=0)
            else:
                datemax = datemax.replace(day=datemax.day + 1, hour=0,
                  minute=0, second=0, microsecond=0) 
            plt.gca().set_xlim(datemin, datemax)

            # set major ticks at each hour
            hours = mdates.HourLocator(interval=1)
            minutes = mdates.MinuteLocator(interval=10)
            plt.gca().xaxis.set_major_locator(hours)
            plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Hh%Mm'))
            plt.gca().xaxis.set_minor_locator(minutes)

            # override default x-axis title
            plt.gca().set_xlabel('Time (hours after MJD start)')

        elif 'ANTENNA' in xtitle.upper():
            plt.gca().xaxis.set_major_locator(ticker.IndexLocator(1,0))

        return pointsflagged

    def plottext(self, xoff, yoff, text, maxchars, ny_subplot=1, mult=1):
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
        ax = plt.gca()
        for i in range(len(words)):
            temp = line + words[i] + ' '
            words_in_line += 1
            if len(temp) > maxchars:
                if words_in_line == 1:
                    ax.text(xoff, yoff, temp, va='center', fontsize=mult*6,
                      transform=ax.transAxes, clip_on=False)
                    yoff -= 0.02 * ny_subplot * mult
                    words_in_line = 0
                else:
                    ax.text(xoff, yoff, line, va='center', fontsize=mult*6,
                      transform=ax.transAxes, clip_on=False)
                    yoff -= 0.02 * ny_subplot * mult
                    line = words[i] + ' '
                    words_in_line = 1
            else:
                line = temp
        if len(line) > 0:
            ax.text(xoff, yoff, line, va='center', fontsize=mult*6,
              transform=ax.transAxes, clip_on=False)
            yoff -= 0.02 * ny_subplot * mult
        yoff -= 0.01 * ny_subplot * mult
        return yoff
        
    def _get_symbol_and_colour(self, pol, bandtype='B'):
        """Get the plot symbol and colour for this polarization and bandtype.
        """
        d = {'B'     : {'L' : ('3', 'orange'),
                        'R' : ('4', 'sandybrown'),
                        'X' : ('2', 'lime'),
                        'Y' : ('1', 'olive')  },
             'BPOLY' : {'L' : ('<', 'lightsteelblue'),
                        'R' : ('>', 'steelblue'),
                        'X' : ('^', 'lightslategray'),
                        'Y' : ('v', 'slategray') }}

        return d.get(bandtype, {}).get(pol, ('x', 'grey'))
     
    def _plot_data(self, caltable, data_description, antenna, y_axis):
        for pol in data_description.polarizations:
            bandtype = caltable.job.kw.get('bandtype', 'B')
            (plotsymbol, plotcolor) = self._get_symbol_and_colour(pol, 
                                                                  bandtype)
        
            plotcal_args = { 'caltable'   : caltable.name,
                             'antenna'    : str(antenna.id),
                             'spw'        : str(data_description.spw.id),
                             'poln'       : pol,
                             'xaxis'      : 'freq',
                             'yaxis'      : y_axis,
                             'overplot'   : self.overplot,
                             'showflags'  : False,
                             'plotsymbol' : plotsymbol,
                             'plotcolor'  : plotcolor,
                             'markersize' : 5.0, 
                             'fontsize'   : 10,
                             'showgui'    : False                     }
    
            casa_tasks.plotcal(**plotcal_args).execute()
            self.overplot = True

            # channel plots can have crazy y limits due to poor fitting at the
            # bandpass edges, so make a memo of reasonable limits set by
            # channel plots
            if bandtype is 'B':
                self.y_limits = plt.ylim()

            self.y_offset = self._plot_key(plotcal_args, self.y_offset, 
                                           bandtype)

        # stop ticks being marked relative to an offset which can be confusing
        xaxis = plt.gca().xaxis
        xaxis.set_major_formatter(ticker.ScalarFormatter(useOffset=False))
        
    def _plot_key(self, plotcal_args, y_offset, bandtype='B'):
        # pylab calls seem to screw up x-axis limits so read them here and
        # restore them later
        x_limits = plt.xlim()
            
        plot_args = { 'linestyle'  : 'None',
                      'marker'     : plotcal_args['plotsymbol'],
                      'markersize' : 1.5 * plotcal_args['markersize'],
                      'color'      : plotcal_args['plotcolor'],
                      'transform'  : plt.gca().transAxes             }
        plt.plot([0.42], [y_offset+0.01], **plot_args)

        bandtype = 'channel' if bandtype is 'B' else 'polynomial'

        text = "{0} {1} result".format(plotcal_args['poln'], bandtype)
        text_args = { 'fontsize'  : 10.0,
                      'transform' : plt.gca().transAxes }
        plt.text(0.45, y_offset, text, **text_args)

        if x_limits:
            plt.xlim(x_limits)
        
        return y_offset - 0.045
