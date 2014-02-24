from __future__ import absolute_import
import datetime
import math
import os
import operator

import matplotlib.dates as dates
import matplotlib.pyplot as pyplot
import matplotlib.ticker as ticker
import pylab
import numpy as np

import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.extern.analysis_scripts.analysisUtils as analysisUtils
from pipeline.extern import analysis_scripts
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.utils as utils
import shutil

LOG = infrastructure.get_logger(__name__)
DISABLE_PLOTMS = True

ticker.TickHelper.MAXTICKS = 10000

class SummaryDisplayInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None, output=None):
        self._init_properties(vars())

    @property
    def output(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, session_part, ms_part)
        

class PlotAntsChart(object):
    def __init__(self, context, ms):
        self.context = context
        self.ms = ms
        self.figfile = self._get_figfile()

    def plot(self):
        if os.path.exists(self.figfile):
            return self._get_plot_object()

        c = analysis_scripts.analysisUtils.stuffForScienceDataReduction()
        c.plotAntennas(self.ms, figfile=self.figfile)

        return self._get_plot_object()

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'plotants.png')

    def _get_plot_object(self):
        return logger.Plot(self.figfile,
                           x_axis='Antenna Longitude',
                           y_axis='Antenna Latitude',
                           parameters={'vis' : self.ms.basename})

class AzElChart(object):
    def __init__(self, context, ms):
        self.context = context
        self.ms = ms
        self.figfile = self._get_figfile()

    def plot(self):
        if DISABLE_PLOTMS:
            LOG.debug('Disabling AzEl plot due to problems with plotms')
            return None

        if os.path.exists(self.figfile):
            return self._get_plot_object()

        casa_tasks.plotms(vis=self.ms.name,
                          xaxis='azimuth',
                          yaxis='elevation',
                          title='Elevation vs Azimuth for %s' % self.ms.basename,                           
                          plotfile=self.figfile,
                          highres=True,
                          interactive=False,
                          overwrite=True).execute()

        return self._get_plot_object()

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'azel.png')

    def _get_plot_object(self):
        return logger.Plot(self.figfile,
                           x_axis='Azimuth',
                           y_axis='Elevation',
                           parameters={'vis' : self.ms.basename})


class WeatherChart(object):
    def __init__(self, context, ms):
        self.context = context
        self.ms = ms
        self.figfile = self._get_figfile()

    def plot(self):
        if os.path.exists(self.figfile):
            LOG.debug('Returning existing Weather plot')
            return self._get_plot_object()

        LOG.debug('Creating new Weather plot')
        analysis_scripts.analysisUtils.plotWeather(vis=self.ms.name,
                                                   figfile=self.figfile)

        # plot weather does not close the plot! work around that here rather
        # than editing the code as we might lose the fix (again..)
        try:
            pylab.close()
        except Exception:
            pass
            
        #Use plotweather task output for the VLA
        try:
            origfile = self.ms.basename+'.plotweather.png'
            shutil.copy2(origfile, self.figfile)
        except:
            LOG.debug('Not a VLA dataset')

        return self._get_plot_object()

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'weather.png')

    def _get_plot_object(self):
        return logger.Plot(self.figfile,
                           x_axis='Time',
                           y_axis='Weather',
                           parameters={'vis': self.ms.basename})


class ElVsTimeChart(object):    
    def __init__(self, context, ms):
        self.context = context
        self.ms = ms
        self.figfile = self._get_figfile()

    def plot(self):
        if DISABLE_PLOTMS:
            LOG.debug('Disabling ElVsTime plot due to problems with plotms')
            return None
    
        if os.path.exists(self.figfile):
            return self._get_plot_object()

        casa_tasks.plotms(vis=self.ms.name,
                          xaxis='time',
                          yaxis='elevation',
                          title='%%yaxis%% vs. %%xaxis%% for %s' % self.ms.basename,                           
                          plotfile=self.figfile,
                          highres=True,
                          interactive=False,
                          overwrite=True).execute()

        return self._get_plot_object()

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'el_vs_time.png')

    def _get_plot_object(self):
        return logger.Plot(self.figfile,
                           x_axis='Time',
                           y_axis='Elevation',
                           parameters={'vis' : self.ms.basename})


class FieldVsTimeChartInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None, output=None):
        self._init_properties(vars())

    @property
    def output(self):
        if self._output is not None:
            return self._output

        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'field_vs_time.png')
        
    @output.setter
    def output(self, value):
        self._output = value


class FieldVsTimeChart(object):
    Inputs = FieldVsTimeChartInputs

    _intent_colours = {'AMPLITUDE'  : 'green',
                       'ATMOSPHERE' : 'magenta',
                       'BANDPASS'   : 'red',
                       'PHASE'      : 'cyan',
                       'POINTING'   : 'yellow',
                       'SIDEBAND'   : 'orange',
                       'TARGET'     : 'blue',
                       'WVR'        : 'lime',
                       'UNKNOWN'    : 'grey',  }
    
    def __init__(self, inputs):
        self.inputs = inputs

    def plot(self):
        ms = self.inputs.ms

        obs_start = utils.get_epoch_as_datetime(ms.start_time)
        obs_end = utils.get_epoch_as_datetime(ms.end_time)
        
        filename = self.inputs.output
        if os.path.exists(filename):
            plot = logger.Plot(filename,
                               x_axis='Time',
                               y_axis='Field',
                               parameters={'vis' : ms.basename})
            return plot
        
        f = pylab.figure()
        pylab.clf()
        pylab.axes([0.1,0.15,0.8,0.7])
        ax = pylab.gca()

        for nfield, field in enumerate(ms.fields):
            for scan in [scan for scan in ms.scans
                         if field in scan.fields]:
                colours = self._get_colours(scan.intents)

                # all 'datetime' objects are in UTC.
                x0 = utils.get_epoch_as_datetime(scan.start_time)
                x1 = utils.get_epoch_as_datetime(scan.end_time)
                
                y0 = nfield-0.5
                y1 = nfield+0.5

                height = (y1 - y0) / float(len(colours))
                ys = y0
                ye = y0 + height
                for colour in colours:
                    ax.fill([x0, x1, x1, x0], 
                            [ys, ys, ye, ye],
                            facecolor=colour, 
                            edgecolor=colour)
                    ys += height
                    ye += height

        # set the labelling of the time axis
        self._set_time_axis(figure=f, ax=ax, datemin=obs_start, datemax=obs_end)

        # set FIELD_ID axis ticks etc.
        if nfield < 11:
            majorLocator = ticker.FixedLocator(np.arange(0, nfield+1))
        else:
            majorLocator = ticker.FixedLocator(np.arange(0, nfield+1, 5))
        ax.yaxis.set_major_locator(majorLocator)
        ax.yaxis.set_minor_locator(ticker.MultipleLocator(1))
        ax.grid(True)

        pylab.ylabel('Field ID')
        majorFormatter = ticker.FormatStrFormatter('%d')
        ax.yaxis.set_major_formatter(majorFormatter)

        # plot key
        self._plot_key()

        pylab.savefig(filename=filename)
        pylab.clf()
        pylab.close()

        plot = logger.Plot(filename,
                           x_axis='Time',
                           y_axis='Field',
                           parameters={ 'vis' : ms.basename })
        
        return plot

    def _plot_key(self):
        pylab.axes([0.1,0.8,0.8,0.2])
        lims = pylab.axis()
        pylab.axis('off')

        x = 0.00
        size = [0.4, 0.4, 0.6, 0.6]
        for intent, colour in sorted(self._intent_colours.items(),
                                     key=operator.itemgetter(0)):
            pylab.gca().fill([x, x+0.05, x+0.05, x], size, facecolor=colour, 
                             edgecolor=colour)
            pylab.text(x+0.06, 0.4, intent, size=9, va='bottom', rotation=45)
            x += 0.12

        pylab.axis(lims)

    def _get_colours(self, user_intents):
        colours = [colour 
                   for (intent, colour) in sorted(self._intent_colours.items(),
                                                  key=operator.itemgetter(0))
                   if intent in user_intents]
        if not colours:
            colours.append(self._intent_colours['UNKNOWN'])
        return colours

    @staticmethod
    def _set_time_axis(figure, ax, datemin, datemax):
        border = datetime.timedelta(minutes=5)
        ax.set_xlim(datemin-border, datemax+border)

        if datemax - datemin < datetime.timedelta(seconds=7200):
            # scales if observation spans less than 2 hours
            quarterhours = dates.MinuteLocator(interval=15)
            minutes = dates.MinuteLocator(interval=5)
            ax.xaxis.set_major_locator(quarterhours)
            ax.xaxis.set_major_formatter(dates.DateFormatter('%Hh%Mm'))
            ax.xaxis.set_minor_locator(minutes)
        elif datemax - datemin < datetime.timedelta(seconds=21600):
            # scales if observation spans less than 6 hours
            halfhours = dates.MinuteLocator(interval=30)
            minutes = dates.MinuteLocator(interval=10)
            ax.xaxis.set_major_locator(halfhours)
            ax.xaxis.set_major_formatter(dates.DateFormatter('%Hh%Mm'))
            ax.xaxis.set_minor_locator(minutes)
        elif datemax - datemin < datetime.timedelta(days=1):
            # scales if observation spans less than a day
            hours = dates.HourLocator(interval=1)
            minutes = dates.MinuteLocator(interval=10)
            ax.xaxis.set_major_locator(hours)
            ax.xaxis.set_major_formatter(dates.DateFormatter('%Hh%Mm'))
            ax.xaxis.set_minor_locator(minutes)
        elif datemax - datemin < datetime.timedelta(days=7):
            # spans more than a day, less than a week
            days = dates.DayLocator()
            hours = dates.HourLocator(np.arange(0,25,6))
            ax.xaxis.set_major_locator(days)
            ax.xaxis.set_major_formatter(dates.DateFormatter('%Y-%m-%d:%Hh'))
            ax.xaxis.set_minor_locator(hours)
        else:    
            # spans more than a week
            months = dates.MonthLocator(bymonthday=1, interval=3)
            mondays = dates.WeekdayLocator(dates.MONDAY)
            ax.xaxis.set_major_locator(months)
            ax.xaxis.set_major_formatter(dates.DateFormatter('%Y-%m'))
            ax.xaxis.set_minor_locator(mondays)

        ax.set_xlabel('Time')
        figure.autofmt_xdate()


class IntentVsTimeChartInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None, output=None):
        self._init_properties(vars())

    @property
    def output(self):
        if self._output is not None:
            return self._output

        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'intent_vs_time.png')
        
    @output.setter
    def output(self, value):
        self._output = value


class IntentVsTimeChart(object):
    Inputs = IntentVsTimeChartInputs

    _intent_colours = {'AMPLITUDE'  : ('green', 15),
                       'ATMOSPHERE' : ('magenta', 20),
                       'BANDPASS'   : ('red', 10),
                       'PHASE'      : ('cyan', 5),
                       'POINTING'   : ('yellow', 25),
                       'SIDEBAND'   : ('orange', 30),
                       'TARGET'     : ('blue', 0),
                       'WVR'        : ('lime', 35)}
    
    def __init__(self, inputs):
        self.inputs = inputs

    def plot(self):
        if os.path.exists(self.inputs.output):
            return self._get_plot_object()

        fig = pyplot.figure(figsize=(14,9))
        ax = fig.add_subplot(111)
    
        ms = self.inputs.ms        
        obs_start = utils.get_epoch_as_datetime(ms.start_time)
        obs_end = utils.get_epoch_as_datetime(ms.end_time)
    
        for scan in ms.scans:
            scan_start = utils.get_epoch_as_datetime(scan.start_time)
            scan_end = utils.get_epoch_as_datetime(scan.end_time)
            for intent in scan.intents:
                if intent not in IntentVsTimeChart._intent_colours:
                    continue
                (colour, scan_y) = IntentVsTimeChart._intent_colours[intent]
                ax.fill([scan_start, scan_end, scan_end, scan_start], 
                        [scan_y, scan_y, scan_y+5, scan_y+5],
                        facecolor=colour)

                ax.annotate('%s' % scan.id, (scan_start, scan_y+6))
    
        ax.set_ylim(0, 42.5)
        ax.set_yticks([2.5,7.5,12.5,17.5,22.5,27.5,32.5,37.5])
        ax.set_yticklabels(['SCIENCE', 'PHASE', 'BANDPASS', 'AMPLITUDE',
          'ATMOSPHERE', 'POINTING', 'SIDEBAND', 'WVR'])

        # set the labelling of the time axis
        FieldVsTimeChart._set_time_axis(figure=fig, ax=ax, datemin=obs_start,
          datemax=obs_end)
        ax.grid(True)
    
        pyplot.title('Measurement set: ' + ms.basename + ' - Start time:' +
          obs_start.strftime('%Y-%m-%dT%H:%M:%S') + ' End time:' + 
          obs_end.strftime('%Y-%m-%dT%H:%M:%S'), fontsize = 12)
    
        fig.savefig(self.inputs.output)
        pylab.close()

        return self._get_plot_object()
        
    def _in_minutes(self, dt):
        return (dt.days * 86400 + dt.seconds + dt.microseconds * 1e-6) / 60.0 

    def _get_plot_object(self):
        filename = self.inputs.output
        return logger.Plot(filename,
                           x_axis='Time',
                           y_axis='Intent',
                           parameters={'vis' : self.inputs.ms.basename})


class PWVChart(object):
    def __init__(self, context, ms):
        self.context = context
        self.ms = ms
        self.figfile = self._get_figfile()

    def plot(self):
        if os.path.exists(self.figfile):
            return self._get_plot_object()

        try:
            analysisUtils.plotPWV(self.ms.name, self.figfile)
        except:
            LOG.debug('Could not create PWV plot')
            return None

        return self._get_plot_object()

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'pwv.png')

    def _get_plot_object(self):
        return logger.Plot(self.figfile,
                           x_axis='Time',
                           y_axis='PWV',
                           parameters={'vis' : self.ms.basename})


class MosaicChart(object):
    def __init__(self, context, ms, source):
        self.context = context
        self.ms = ms
        self.source = source
        self.figfile = self._get_figfile()

    def plot(self):
        if os.path.exists(self.figfile):
            return self._get_plot_object()

        try:
            analysisUtils.plotMosaic(self.ms.name, 
                                     sourceid=self.source.id,
                                     coord='rel',
                                     figfile=self.figfile)

            # plotMosaic does not close the plot! work around that
            # here rather than editing the code as we might lose the fix
            try:
                pylab.close()
            except Exception:
                pass

        except:
            LOG.debug('Could not create mosaic plot')
            return None

        return self._get_plot_object()

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'mosaic_source%s.png' % self.source.id)

    def _get_plot_object(self):
        return logger.Plot(self.figfile,
                           x_axis='RA Offset',
                           y_axis='Dec Offset',
                           parameters={'vis' : self.ms.basename})


class PlotAntsChart2(object):
    def __init__(self, context, ms):
        self.context = context
        self.ms = ms
        self.figfile = self._get_figfile()
        self.site = casatools.measures.observatory(ms.antenna_array.name)

    def plot(self):
#        if os.path.exists(self.figfile):
#            return self._get_plot_object()

        # map: with pad names
        plf1 = pylab.figure(1)
        subpl1 = plf1.add_subplot(1, 1, 1, aspect='equal')
        self.draw_pad_map_in_subplot(subpl1, self.ms.antennas)
        pylab.title('Antenna Positions for %s' % self.ms.basename)
        pylab.savefig(self.figfile, format='png', density=108)
        pylab.close()

        return self._get_plot_object()

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'plotants.png')

    def _get_plot_object(self):
        return logger.Plot(self.figfile,
                           x_axis='Antenna Longitude',
                           y_axis='Antenna Latitude',
                           parameters={'vis' : self.ms.basename})

    def geo_to_site(self, antenna):
        """
        Convert the geocentric coordinates into the local (horizontal) ones.
        """
        qt = casatools.quanta
        site_lon = qt.getvalue(self.site['m0'])
        site_lat = qt.getvalue(self.site['m1'])
        
        ant_lon = qt.getvalue(antenna.position['m0'])
        ant_lat = qt.getvalue(antenna.position['m1'])
        ant_z = qt.getvalue(antenna.position['m2'])
        
        local = [0, 0, 0]
        local[0] = - ant_lon * math.sin(site_lon) + \
                     ant_lat * math.cos(site_lon)
        local[1] = - ant_lon * math.cos(site_lon) * math.sin(site_lat) - \
                     ant_lat * math.sin(site_lon) * math.sin(site_lat) + \
                     ant_z * math.cos(site_lat)
        local[2] = ant_lon * math.cos(site_lon) * math.cos(site_lat) + \
                   ant_lat * math.sin(site_lon) * math.cos(site_lat) + \
                   ant_z * math.sin(site_lat)

        return local


    def draw_pad_map_in_subplot(self, subpl, antennas, xlimit=None, 
                                ylimit=None, showemptypads=True):
        """
        Draw a map of pads and antennas on them.

        subpl: a pylab.subplot instance
        pads: a dictionary of antennas {"Name": (X, Y, Z), ...}
        antennas: a dictionary of antennas {"AntennaName": "PadName", ...}
        xlimit, ylimit: lists (or tuples, arrays) for the x and y axis limits.
                        if not given, automatically adjusted.
        showemptypads: set False not to draw pads and their names
        showbaselinelength: set True to display baseline length
        """
        subpl.clear()

        if showemptypads:
            for antenna in antennas:
                padpos = self.get_position(antenna)
                circ = pylab.Circle(padpos[:2], antenna.diameter/2.0)
                subpl.add_artist(circ)
                circ.set_alpha(0.5)
                circ.set_facecolor([1.0, 1.0, 1.0])
                tt = subpl.text(padpos[0]+8., padpos[1]-4., antenna.station)
                pylab.setp(tt, size='small', alpha=0.5)

        (xmin, xmax, ymin, ymax) = (9e9, -9e9, 9e9, -9e9)
        for antenna in antennas:
            padpos = self.get_position(antenna)
            circ = pylab.Circle(padpos[:2], radius=antenna.diameter/2.0)
            subpl.add_artist(circ)
            circ.set_alpha(1.0)
            circ.set_facecolor([0.8, 0.8, 0.8])
            subpl.text(padpos[0]+8, padpos[1]+1, antenna.name)
            if padpos[0] < xmin: xmin = padpos[0]
            if padpos[0] > xmax: xmax = padpos[0]
            if padpos[1] < ymin: ymin = padpos[1]
            if padpos[1] > ymax: ymax = padpos[1]

        subpl.set_xlabel('X [m]')
        subpl.set_ylabel('Y [m]')
        plotwidth = max(xmax-xmin, ymax-ymin) * 6./10. # extra 1/10 is the margin
        (xcenter, ycenter) = ((xmin+xmax)/2., (ymin+ymax)/2.)
        if xlimit == None:
            #subpl.set_xlim(xcenter-plotwidth, xcenter+plotwidth)
            subpl.set_xlim(xcenter[0]-plotwidth[0], xcenter[0]+plotwidth[0])
        else:
            subpl.set_xlim(xlimit[0], xlimit[1])
        if ylimit == None:
            #subpl.set_ylim(ycenter-plotwidth, ycenter+plotwidth)
            subpl.set_ylim(ycenter[0]-plotwidth[0], ycenter[0]+plotwidth[0])
        else:
            subpl.set_ylim(ylimit[0], ylimit[1])

    def get_position(self, antenna):
        pos = self.geo_to_site(antenna)

#        if self.ms.antenna_array.name == 'ALMA':
#            # Arbitrarily shift ALMA coord so that central cluster comes 
#            # around (0, 0).
#            pos = (pos[0]+480., pos[1]-14380., pos[2])

        return pylab.array(pos)
