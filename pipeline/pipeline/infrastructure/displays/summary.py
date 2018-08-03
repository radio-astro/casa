from __future__ import absolute_import

import datetime
import math
import operator
import os

import matplotlib.dates as dates
import matplotlib.pyplot as pyplot
import matplotlib.ticker as ticker
import numpy as np
import pylab

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from pipeline.domain.measures import FrequencyUnits, DistanceUnits
from pipeline.infrastructure import casa_tasks
from . import plotmosaic
from . import plotpwv
from . import plotweather

LOG = infrastructure.get_logger(__name__)
DISABLE_PLOTMS = False

ticker.TickHelper.MAXTICKS = 10000


class AzElChart(object):
    def __init__(self, context, ms):
        self.context = context
        self.ms = ms
        self.figfile = self._get_figfile()

        # Plot the first channel for all the science spws.
        self.spwlist = ''
        for spw in ms.get_spectral_windows(science_windows_only=True):
            if self.spwlist == '':
                self.spwlist += '%d:0~0' % spw.id
            else:
                self.spwlist += ',%d:0~0' % spw.id

    def plot(self):
        if DISABLE_PLOTMS:
            LOG.debug('Disabling AzEl plot due to problems with plotms')
            return None

        # inputs based on analysisUtils.plotElevationSummary
        task_args = {
            'vis': self.ms.name,
            'xaxis': 'azimuth',
            'yaxis': 'elevation',
            'title': 'Elevation vs Azimuth for %s' % self.ms.basename,
            'coloraxis': 'field',
            'avgchannel': '9000',
            'avgtime': '10',
            'antenna': '0&&*',
            'spw': self.spwlist,
            'plotfile': self.figfile,
            'clearplots': True,
            'showgui': False,
            'customflaggedsymbol': True,
            'flaggedsymbolshape': 'autoscaling'}

        task = casa_tasks.plotms(**task_args)

        if not os.path.exists(self.figfile):
            task.execute()
        
        return self._get_plot_object(task)

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'azel.png')

    def _get_plot_object(self, task):
        return logger.Plot(self.figfile,
                           x_axis='Azimuth',
                           y_axis='Elevation',
                           parameters={'vis': self.ms.basename},
                           command=str(task))


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
        try:
            # Based on the analysisUtils method
            # analysisUtils.plotWeather(vis=self.ms.name, figfile=self.figfile)
            plotweather.plotWeather(vis=self.ms.name, figfile=self.figfile)
        except:
            return None
        finally:
            # plot weather does not close the plot! work around that here rather
            # than editing the code as we might lose the fix (again..)
            try:
                pylab.close()
            except:
                pass

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

        # Plot the first channel for all the science spws.
        self.spwlist = ''
        for spw in ms.get_spectral_windows(science_windows_only=True):
            if self.spwlist == '':
                self.spwlist = self.spwlist + '%d:0~0' % spw.id
            else:
                self.spwlist = self.spwlist + ',%d:0~0' % spw.id

    def plot(self):
        if DISABLE_PLOTMS:
            LOG.debug('Disabling ElVsTime plot due to problems with plotms')
            return None
    
        # Inputs based on analysisUtils.plotElevationSummary
        task_args = {'vis': self.ms.name,
                     'xaxis': 'time',
                     'yaxis': 'elevation',
                     'title': 'Elevation vs Time for %s' % self.ms.basename,
                     'coloraxis': 'field',
                     'avgchannel': '9000',
                     'avgtime': '10',
                     'antenna': '0&&*',
                     'spw': self.spwlist,
                     'plotfile': self.figfile,
                     'clearplots': True,
                     'showgui': False,
                     'customflaggedsymbol': True,
                     'flaggedsymbolshape': 'autoscaling'}

        task = casa_tasks.plotms(**task_args)
        if not os.path.exists(self.figfile):
            task.execute()

        return self._get_plot_object(task)

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, 'el_vs_time.png')

    def _get_plot_object(self, task):
        return logger.Plot(self.figfile,
                           x_axis='Time',
                           y_axis='Elevation',
                           parameters={'vis': self.ms.basename},
                           command=str(task))


class FieldVsTimeChartInputs(vdp.StandardInputs):

    @vdp.VisDependentProperty
    def output(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        output = os.path.join(self.context.report_dir,
                              'session%s' % session_part,
                              ms_part, 'field_vs_time.png')
        return output

    def __init__(self, context, vis=None, output=None):
        super(FieldVsTimeChartInputs, self).__init__()

        self.context = context
        self.vis = vis

        self.output = output


class FieldVsTimeChart(object):
    Inputs = FieldVsTimeChartInputs

    # http://matplotlib.org/examples/color/named_colors.html
    _intent_colours = {'AMPLITUDE': 'green',
                       'ATMOSPHERE': 'magenta',
                       'BANDPASS': 'red',
                       'CHECK': 'purple',
                       'PHASE': 'cyan',
                       'POINTING': 'yellow',
                       'SIDEBAND': 'orange',
                       'TARGET': 'blue',
                       'WVR': 'lime',
                       'POLARIZATION': 'navy',
                       'POLANGLE': 'mediumslateblue',
                       'POLLEAKAGE': 'plum',
                       'UNKNOWN': 'grey',
                       }
    
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
                               parameters={'vis': ms.basename})
            return plot
        
        f = pylab.figure()
        pylab.clf()
        pylab.axes([0.1, 0.15, 0.8, 0.7])
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
            major_locator = ticker.FixedLocator(np.arange(0, nfield+1))
            minor_locator = ticker.MultipleLocator(1)
            ax.yaxis.set_minor_locator(minor_locator)
        else:
            major_locator = ticker.FixedLocator(np.arange(0, nfield+1, 400))
        ax.yaxis.set_major_locator(major_locator)
        ax.grid(True)

        pylab.ylabel('Field ID')
        major_formatter = ticker.FormatStrFormatter('%d')
        ax.yaxis.set_major_formatter(major_formatter)

        # plot key
        self._plot_key()

        pylab.savefig(filename=filename)
        pylab.clf()
        pylab.close()

        plot = logger.Plot(filename,
                           x_axis='Time',
                           y_axis='Field',
                           parameters={'vis': ms.basename})
        
        return plot

    def _plot_key(self):
        pylab.axes([0.1, 0.8, 0.8, 0.2])
        lims = pylab.axis()
        pylab.axis('off')

        x = 0.00
        size = [0.4, 0.4, 0.6, 0.6]
        for intent, colour in sorted(self._intent_colours.items(),
                                     key=operator.itemgetter(0)):
            if (intent in self.inputs.ms.intents) or 'UNKNOWN' in intent:
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
            hours = dates.HourLocator(np.arange(0, 25, 6))
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


class IntentVsTimeChartInputs(vdp.StandardInputs):

    @vdp.VisDependentProperty
    def output(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        output = os.path.join(self.context.report_dir,
                              'session%s' % session_part,
                              ms_part, 'intent_vs_time.png')
        return output

    def __init__(self, context, vis=None, output=None):
        super(IntentVsTimeChartInputs, self).__init__()

        self.context = context
        self.vis = vis

        self.output = output


class IntentVsTimeChart(object):
    Inputs = IntentVsTimeChartInputs

    # http://matplotlib.org/examples/color/named_colors.html
    _intent_colours = {'AMPLITUDE': ('green', 20),
                       'ATMOSPHERE': ('magenta', 25),
                       'BANDPASS': ('red', 15),
                       'CHECK': ('purple', 10),
                       'PHASE': ('cyan', 5),
                       'POINTING': ('yellow', 30),
                       'SIDEBAND': ('orange', 35),
                       'TARGET': ('blue', 0),
                       'WVR': ('lime', 40),
                       'POLARIZATION': ('navy', 45),
                       'POLANGLE': ('mediumslateblue', 50),
                       'POLLEAKAGE': ('plum', 55),
                       }
    
    def __init__(self, inputs):
        self.inputs = inputs

    def plot(self):
        if os.path.exists(self.inputs.output):
            return self._get_plot_object()

        fig = pyplot.figure(figsize=(14, 9))
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
    
        ax.set_ylim(0, 57.5)
        ax.set_yticks([2.5, 7.5, 12.5, 17.5, 22.5, 27.5, 32.5, 37.5, 42.5, 47.5, 52.5, 57.5])
        ax.set_yticklabels(['SCIENCE', 'PHASE', 'CHECK', 'BANDPASS',
                            'AMPLITUDE', 'ATMOSPHERE', 'POINTING', 'SIDEBAND',
                            'WVR', 'POLARIZATION', 'POLANGLE', 'POLLEAKAGE'])

        # set the labelling of the time axis
        FieldVsTimeChart._set_time_axis(
            figure=fig, ax=ax, datemin=obs_start, datemax=obs_end)
        ax.grid(True)
    
        pyplot.title(
            'Measurement set: ' + ms.basename + ' - Start time:' +
            obs_start.strftime('%Y-%m-%dT%H:%M:%S') + ' End time:' +
            obs_end.strftime('%Y-%m-%dT%H:%M:%S'), fontsize=12)
    
        fig.savefig(self.inputs.output)
        pylab.clf()
        pylab.close()

        return self._get_plot_object()
        
    @staticmethod
    def _in_minutes(dt):
        return (dt.days * 86400 + dt.seconds + dt.microseconds * 1e-6) / 60.0 

    def _get_plot_object(self):
        filename = self.inputs.output
        return logger.Plot(filename,
                           x_axis='Time',
                           y_axis='Intent',
                           parameters={'vis': self.inputs.ms.basename})


class PWVChart(object):
    def __init__(self, context, ms):
        self.context = context
        self.ms = ms
        self.figfile = self._get_figfile()

    def plot(self):
        if os.path.exists(self.figfile):
            return self._get_plot_object()

        try:
            plotpwv.plotPWV(self.ms.name, figfile=self.figfile)
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
                           parameters={'vis': self.ms.basename})


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
            # based on the analysisUtils method
            # analysisUtils.plotMosaic(self.ms.name,
            #                          sourceid=self.source.id,
            #                          coord='rel',
            #                          figfile=self.figfile)
            plotmosaic.plotMosaic(self.ms.name, 
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
                           parameters={'vis': self.ms.basename})


class PlotAntsChart(object):
    def __init__(self, context, ms, polarlog=False):
        self.context = context
        self.ms = ms
        self.polarlog = polarlog
        self.figfile = self._get_figfile()
        self.site = casatools.measures.observatory(ms.antenna_array.name)

    def plot(self):
        if os.path.exists(self.figfile):
            return self._get_plot_object()

        # map: with pad names
        plf1 = pylab.figure(1)
        pylab.clf()
        if self.polarlog:
            self.draw_polarlog_ant_map_in_subplot(plf1)
        else:
            self.draw_pad_map_in_subplot(plf1, self.ms.antennas)
        pylab.title('Antenna Positions for %s' % self.ms.basename)
        pylab.savefig(self.figfile, format='png', density=108)
        pylab.clf()
        pylab.close()

        return self._get_plot_object()

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        if self.polarlog:
            figfilename = 'plotants_polarlog.png'
        else:
            figfilename = 'plotants.png'
        return os.path.join(self.context.report_dir, 
                            'session%s' % session_part, 
                            ms_part, figfilename)

    def _get_plot_object(self):
        return logger.Plot(self.figfile,
                           x_axis='Antenna Longitude',
                           y_axis='Antenna Latitude',
                           parameters={'vis': self.ms.basename})

    def draw_pad_map_in_subplot(self, plf, antennas, xlimit=None, 
                                ylimit=None, showemptypads=True):
        """
        Draw a map of pads and antennas on them.

        plf: a pylab.figure instance
        pads: a dictionary of antennas {"Name": (X, Y, Z), ...}
        antennas: a dictionary of antennas {"AntennaName": "PadName", ...}
        xlimit, ylimit: lists (or tuples, arrays) for the x and y axis limits.
                        if not given, automatically adjusted.
        showemptypads: set False not to draw pads and their names
        """
        subpl = plf.add_subplot(1, 1, 1, aspect='equal')

        if showemptypads:
            for antenna in antennas:
                padpos = self.get_position(antenna)
                circ = pylab.Circle(padpos[:2], antenna.diameter/2.0)
                subpl.add_artist(circ)
                circ.set_alpha(0.5)
                circ.set_facecolor([1.0, 1.0, 1.0])
                tt = subpl.text(padpos[0]+antenna.diameter/2.0*1.3, padpos[1]-4., antenna.station)
                pylab.setp(tt, size='small', alpha=0.5)

        (xmin, xmax, ymin, ymax) = (9e9, -9e9, 9e9, -9e9)
        for antenna in antennas:
            padpos = self.get_position(antenna)
            circ = pylab.Circle(padpos[:2], radius=antenna.diameter/2.0)
            subpl.add_artist(circ)
            circ.set_alpha(1.0)
            circ.set_facecolor([0.8, 0.8, 0.8])
            subpl.text(padpos[0]+antenna.diameter/2.0*1.3, padpos[1]+1, antenna.name)
            if padpos[0] < xmin:
                xmin = padpos[0]
            if padpos[0] > xmax:
                xmax = padpos[0]
            if padpos[1] < ymin:
                ymin = padpos[1]
            if padpos[1] > ymax:
                ymax = padpos[1]

        subpl.set_xlabel('X [m]')
        subpl.set_ylabel('Y [m]')
        plotwidth = max(xmax-xmin, ymax-ymin) * 6./10.  # extra 1/10 is the margin
        (xcenter, ycenter) = ((xmin+xmax)/2., (ymin+ymax)/2.)
        if xlimit is None:
            # subpl.set_xlim(xcenter-plotwidth, xcenter+plotwidth)
            subpl.set_xlim(xcenter[0]-plotwidth[0], xcenter[0]+plotwidth[0])
        else:
            subpl.set_xlim(xlimit[0], xlimit[1])
        if ylimit is None:
            # subpl.set_ylim(ycenter-plotwidth, ycenter+plotwidth)
            subpl.set_ylim(ycenter[0]-plotwidth[0], ycenter[0]+plotwidth[0])
        else:
            subpl.set_ylim(ylimit[0], ylimit[1])

    @staticmethod
    def get_position(antenna):
        # if self.ms.antenna_array.name == 'ALMA':
        #    # Arbitrarily shift ALMA coord so that central cluster comes
        #    # around (0, 0).
        #    pos = (pos[0]+480., pos[1]-14380., pos[2])

        pos = [[antenna.offset['longitude offset']['value']],
               [antenna.offset['latitude offset']['value']],
               [antenna.offset['elevation offset']['value']]]

        return pylab.array(pos)

    # This plot is adapted from the "plotPositionsLogarithmic" function
    # in the Analysis Utils written by Todd Hunter.
    def draw_polarlog_ant_map_in_subplot(self, plf):
        """
        Draw a polar-log map of antennas.
        
        plf: a pylab.figure instance
        """

        # Get longitude and latitude offsets in meters for antennas.
        xoffsets = np.array([ant.offset['longitude offset']['value']
                             for ant in self.ms.antennas])
        yoffsets = np.array([ant.offset['latitude offset']['value']
                             for ant in self.ms.antennas])

        # Set center of plot and min/max rmin as appropriate for observatory.
        if self.context.project_summary.telescope in ('VLA', 'EVLA'):
            # For (E)VLA, set a fixed local center position that has been
            # tuned to work well for its array configurations (CAS-7479).
            xcenter, ycenter = -32, 0
            rmin_min, rmin_max = 12.5, 350
        else:
            # For non-(E)VLA, take the median of antenna offsets as the
            # center for the plot.
            xcenter = np.median(xoffsets)
            ycenter = np.median(yoffsets)
            rmin_min, rmin_max = 3, 350

        # Derive radial offset w.r.t. center position.
        r = ((xoffsets-xcenter)**2 + (yoffsets-ycenter)**2)**0.5

        # Set rmin, clamp between a min and max value, ignore station
        # at r=0 if one is there.
        rmin = min(rmin_max, max(rmin_min, 0.8*np.min(r[r > 0])))
        
        # Update r to move any points below rmin to r=rmin.
        r[r <= rmin] = rmin
        rmin = np.log(rmin)
        
        # Set rmax.
        rmax = np.log(1.5*np.max(r))

        # Derive angle of offset w.r.t. center position.
        theta = np.arctan2(xoffsets-xcenter, yoffsets-ycenter)

        # Set up subplot.
        subpl = plf.add_subplot(1, 1, 1, polar=True, projection='polar')

        # Set zero point and direction for theta angle.
        subpl.set_theta_zero_location('N')
        subpl.set_theta_direction(-1)
        
        # Do not show azimuth labels.
        subpl.set_xticklabels([])
        subpl.set_yticklabels([])
        
        # Do not show grid.
        subpl.grid(False)
        
        # Draw circles at specific distances from the center.
        angles = np.arange(0, 2.01*np.pi, 0.01*np.pi)
        show_circle = True
        for cr in [30, 100, 300, 1000, 3000, 10000]:

            # Only draw circles outside rmin.
            if cr > np.min(r) and show_circle:

                # Draw the circle.
                radius = np.ones(len(angles))*np.log(cr)
                subpl.plot(angles, radius, 'k:')

                # Draw tick marks on the circle at 1 km intervals.
                inc = 0.1*10000/cr
                if cr > 100:
                    for angle in np.arange(inc/2., 2*np.pi+0.05, inc):
                        subpl.plot([angle, angle],
                                   [np.log(0.95*cr), np.log(1.05*cr)], 'k-')

                # Add text label to circle to denote distance from center.
                va = 'top'
                circle_label_angle = -20.0 * np.pi / 180.
                if cr >= 1000:
                    if np.log(cr) < rmax:
                        subpl.text(circle_label_angle, np.log(cr),
                                   '%d km' % (cr/1000), size=8, va=va)
                        subpl.text(circle_label_angle + np.pi, np.log(cr),
                                   '%d km' % (cr / 1000), size=8, va=va)
                else:
                    subpl.text(circle_label_angle, np.log(cr), '%dm' % (cr),
                               size=8, va=va)
                    subpl.text(circle_label_angle + np.pi, np.log(cr), '%dm' % (cr),
                               size=8, va=va)

            # Find out if most recently drawn circle was outside all antennas, 
            # if so, no more circles will be drawn.
            if np.log(cr) > rmax:
                show_circle = False

        # For each antenna:
        for i, antenna in enumerate(self.ms.antennas):

            # Draw the antenna position.
            subpl.plot(theta[i], np.log(r[i]), 'ko', ms=5, mfc='k')
    
            # Draw label for the antenna.
            subpl.text(theta[i], np.log(r[i]), '   '+antenna.name, size=8,
                       color='k', ha='left', va='bottom', weight='bold')
    
            # Create label for legend
            if max(r) < 100:
                label = r'{}: {:2.0f} m, {:4.0f}$^\circ$'.format(
                    antenna.name, r[i], np.degrees(theta[i]))
            elif max(r) < 1000:
                label = r'{}: {:3.0f} m, {:4.0f}$^\circ$'.format(
                    antenna.name, r[i], np.degrees(theta[i]))
            elif max(r) < 3000:
                label = r'{}: {:3.2f} km, {:4.0f}$^\circ$'.format(
                    antenna.name, 0.001*r[i], np.degrees(theta[i]))
            else:
                label = r'{}: {:3.1f} km, {:4.0f}$^\circ$'.format(
                    antenna.name, 0.001*r[i], np.degrees(theta[i]))

            # Draw a key in the legend for finding the antenna.
            subpl.annotate(label, xy=(0.5, 0.5),
                           xytext=(0.02, 0.925-0.90*i/len(self.ms.antennas)),
                           xycoords='figure fraction',
                           textcoords='figure fraction', weight='bold',
                           arrowprops=None, color='black', ha='left',
                           va='center', size=8)

        # Set minimum and maximum radius.
        subpl.set_rmax(rmax)
        subpl.set_rmin(rmin)


class UVChart(object):
    def __init__(self, context, ms, customflagged=False, output_dir=None, title_prefix=None):
        self.context = context
        self.ms = ms
        self.customflagged = customflagged
        self.figfile = self._get_figfile(output_dir=output_dir)

        # Select which source to plot.
        src, spw_id = self._get_source_and_spwid()
        self.spw_id = spw_id

        # Determine which field to plot.
        self.field, self.field_name = self._get_field_for_source(src)

        # Determine number of channels in spw.
        self.nchan = self._get_nchan_for_spw(spw_id)

        # Set title of plot, modified by prefix if provided.
        self.title = 'UV coverage for {}'.format(self.ms.basename)
        if title_prefix:
            self.title = title_prefix + self.title

        # get max UV via unprojected baseline
        spw = ms.get_spectral_window(spw_id)
        wavelength_m = 299792458 / float(spw.max_frequency.to_units(FrequencyUnits.HERTZ))
        bl_max = float(ms.antenna_array.max_baseline.length.to_units(DistanceUnits.METRE))
        self.uv_max = math.ceil(1.05 * bl_max / wavelength_m)

    def plot(self):
        if DISABLE_PLOTMS:
            LOG.debug('Disabling UV coverage plot due to problems with plotms')
            return None

        # inputs based on analysisUtils.plotElevationSummary
        task_args = {
            'vis': self.ms.name,
            'xaxis': 'uwave',
            'yaxis': 'vwave',
            'title': self.title,
            'avgchannel': self.nchan,
            'antenna': '*&*',
            'spw': self.spw_id,
            'field': self.field,
            'intent': utils.to_CASA_intent(self.ms, 'TARGET'),
            'plotfile': self.figfile,
            'clearplots': True,
            'showgui': False,
            'customflaggedsymbol': self.customflagged,
            'plotrange': [-self.uv_max, self.uv_max, -self.uv_max, self.uv_max]
        }

        task = casa_tasks.plotms(**task_args)

        if not os.path.exists(self.figfile):
            task.execute()

        return self._get_plot_object(task)

    def _get_figfile(self, output_dir=None):
        # If output dir is specified, then store as <msname>-uv_coverage.png in output dir.
        if output_dir:
            figfile = os.path.join(output_dir, "{}-uv_coverage.png".format(self.ms.basename))
        # Otherwise, store under <sessionname>/<msname>/ directory as uv_coverage.png.
        else:
            session_part = self.ms.session
            ms_part = self.ms.basename
            figfile = os.path.join(self.context.report_dir, 'session%s' % session_part, ms_part, 'uv_coverage.png')
        return figfile

    def _get_plot_object(self, task):
        return logger.Plot(self.figfile,
                           parameters={'vis': self.ms.basename,
                                       'field': self.field,
                                       'field_name': self.field_name,
                                       'spw': self.spw_id},
                           command=str(task))

    def _get_source_and_spwid(self):
        # Attempt to get representative source and spwid.
        repr_src, repr_spw = self._get_representative_source_and_spwid()

        # Check that representative source was covered with TARGET intent,
        # otherwise reject.
        target_sources = [source for source in self.ms.sources
                          if source.name == repr_src
                          and 'TARGET' in source.intents]
        if not target_sources:
            repr_src = None

        # If both are defined, returned representative src and spw.
        if repr_src and repr_spw:
            return repr_src, str(repr_spw)
        elif repr_src and not repr_spw:
            spw = self._get_first_science_spw()
            return repr_src, spw

        # If no representative source was identified, then return first source
        # and first science spw.
        src, spw = self._get_first_target_source_and_science_spw()

        return src, spw

    def _get_representative_source_and_spwid(self):
        # Is the representative source in the context or not
        if not self.context.project_performance_parameters.representative_source:
            source_name = None
        else:
            source_name = self.context.project_performance_parameters.representative_source

        # Is the representative spw in the context or not
        if not self.context.project_performance_parameters.representative_spwid:
            source_spwid = None
        else:
            source_spwid = self.context.project_performance_parameters.representative_spwid

        # Determine the representative source name and spwid for the ms
        repsource_name, repsource_spwid = self.ms.get_representative_source_spw(source_name=source_name,
                                                                                source_spwid=source_spwid)

        return repsource_name, repsource_spwid

    def _get_first_science_spw(self):
        sci_spws = self.ms.get_spectral_windows(science_windows_only=True)
        try:
            spw = str(sci_spws[0].id)
        except IndexError:
            spw = None
        return spw

    def _get_first_target_source_and_science_spw(self):
        src = None
        for ms_src in self.ms.sources:
            if 'TARGET' in ms_src.intents:
                src = ms_src.name
                break

        spw = self._get_first_science_spw()

        return src, spw

    def _get_field_for_source(self, source_name):
        source = None
        for ms_source in self.ms.sources:
            if ms_source.name == source_name:
                source = ms_source
                break

        if not source:
            LOG.error("Source {} not found in MS.".format(source_name))
            return ''

        # Identify fields covered by TARGET intent.
        target_fields = [field for field in source.fields if 'TARGET' in field.intents]

        nfields = len(target_fields)
        if nfields == 0:
            LOG.error("Source {} has no fields with TARGET intent.".format(source.name))
            return '', ''
        elif nfields == 1:
            field = target_fields[0]
        else:
            field = self._get_center_field(target_fields)

        return str(field.id), field.name

    @staticmethod
    def _get_center_field(fields):
        # TODO: need algorithm to determine centermost field among
        # series of pointings for a mosaic.
        # For now, assume the first field is the centermost pointing.
        return fields[0]

    def _get_nchan_for_spw(self, spwid):
        if spwid is None:
            return None

        spw = self.ms.get_spectral_window(int(spwid))
        nchan = str(len(spw.channels))
        return nchan
