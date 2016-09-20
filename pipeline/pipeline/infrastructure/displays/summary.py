from __future__ import absolute_import
import datetime
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
from . import plotpwv
from pipeline.extern import analysis_scripts
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.utils as utils
import casa

LOG = infrastructure.get_logger(__name__)
DISABLE_PLOTMS = False

ticker.TickHelper.MAXTICKS = 10000


class SummaryDisplayInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None, output=None):
        self._init_properties(vars())

    @property
    def output(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        return os.path.join(self.context.report_dir, session_part, ms_part)
        

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
            'flaggedsymbolshape': 'autoscaling' }

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
                           parameters={'vis' : self.ms.basename},
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
            analysisUtils.plotWeather(vis=self.ms.name, figfile=self.figfile)
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
    
        #inputs based on analysisUtils.plotElevationSummary
        task_args = {'vis'             : self.ms.name,
                     'xaxis'           : 'time',
                     'yaxis'           : 'elevation',
                     'title'           : 'Elevation vs Time for %s' % self.ms.basename,
                     'coloraxis'       : 'field',
                     'avgchannel'      : '9000',
                     'avgtime'         : '10',
                     'antenna'         : '0&&*',
                     'spw'             : self.spwlist,
                     'plotfile'        : self.figfile,
                     'clearplots'      : True,
                     'showgui'         : False,
                     'customflaggedsymbol': True,
                     'flaggedsymbolshape': 'autoscaling' }

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
                           parameters={'vis' : self.ms.basename},
                           command=str(task))


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
                       'CHECK'      : 'purple',
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
            minorLocator = ticker.MultipleLocator(1)
            ax.yaxis.set_minor_locator(minorLocator)
        else:
            majorLocator = ticker.FixedLocator(np.arange(0, nfield+1, 400))
        ax.yaxis.set_major_locator(majorLocator)
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

    _intent_colours = {'AMPLITUDE'  : ('green', 20),
                       'ATMOSPHERE' : ('magenta', 25),
                       'BANDPASS'   : ('red', 15),
                       'CHECK'      : ('purple',10),
                       'PHASE'      : ('cyan', 5),
                       'POINTING'   : ('yellow', 30),
                       'SIDEBAND'   : ('orange', 35),
                       'TARGET'     : ('blue', 0),
                       'WVR'        : ('lime', 40)}
    
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
        ax.set_yticks([2.5,7.5,12.5,17.5,22.5,27.5,32.5,37.5,42.5])
        ax.set_yticklabels(['SCIENCE', 'PHASE', 'CHECK', 'BANDPASS', 'AMPLITUDE',
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
        if self.polarlog:
            self.draw_polarlog_ant_map_in_subplot(plf1, self.ms.antennas)
        else:
            self.draw_pad_map_in_subplot(plf1, self.ms.antennas)
        pylab.title('Antenna Positions for %s' % self.ms.basename)
        pylab.savefig(self.figfile, format='png', density=108)
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
                           parameters={'vis' : self.ms.basename})

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
#        if self.ms.antenna_array.name == 'ALMA':
#            # Arbitrarily shift ALMA coord so that central cluster comes 
#            # around (0, 0).
#            pos = (pos[0]+480., pos[1]-14380., pos[2])

        pos = [[antenna.offset['longitude offset']['value']],
               [antenna.offset['latitude offset']['value']],
               [antenna.offset['elevation offset']['value']]]

        return pylab.array(pos)

    # This plot is adapted from the "plotPositionsLogarithmic" function
    # in the Analysis Utils written by Todd Hunter.
    def draw_polarlog_ant_map_in_subplot(self, plf, antennas):
        """
        Draw a polar-log map of antennas.
        
        plf: a pylab.figure instance
        antennas: a list of dictionaries, each representing an antenna
        """

        # Get longitude and latitude offsets in meters for antennas.
        xoffsets = np.array([ant.offset['longitude offset']['value'] for ant in antennas])
        yoffsets = np.array([ant.offset['latitude offset']['value'] for ant in antennas])

        # Take median of antenna offsets as the actual center for the plot.
        xcenter = np.median(xoffsets)
        ycenter = np.median(yoffsets)
        
        # Derive radial offset and angle w.r.t. center position.
        theta = np.arctan2(xoffsets-xcenter, yoffsets-ycenter)
        r = ((xoffsets-xcenter)**2 + (yoffsets-ycenter)**2)**0.5
        
        # Set rmin, clamp between a min and max value, ignore station at r=0 if one is there.
        rmin_min, rmin_max = 7, 200
        rmin = min(rmin_max, max(rmin_min, np.min(r[r>0])))
        
        # Update r to move any points below rmin to r=rmin.
        r[r<=rmin] = rmin
        rmin = np.log(rmin)
        
        # Set rmax.
        rmax = np.log(1.5*np.max(r))

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
        angles = np.arange(0,2.01*np.pi,0.01*np.pi)
        show_circle = True
        for cr in [30,100,300,1000,3000,10000]:

            # Only draw circles outside rmin.
            if cr > np.min(r) and show_circle:

                # Draw the circle.
                radius = np.ones(len(angles))*np.log(cr)
                subpl.plot(angles, radius, 'k:')

                # Draw tick marks on the circle at 1 km intervals.
                inc = 0.1*10000/cr
                if cr > 100:
                    for angle in np.arange(inc/2., 2*np.pi+0.05, inc):
                        subpl.plot([angle,angle], [np.log(0.95*cr), np.log(1.05*cr)], 'k-') 

                # Add text label to circle to denote distance from center.
                va = 'top'
                circle_label_angle = -20.0 * np.pi / 180.
                if cr >= 1000:
                    if (np.log(cr) < rmax):
                        subpl.text(circle_label_angle, np.log(cr), '%d km'%(cr/1000), size=8, va=va)
                else:
                    subpl.text(circle_label_angle, np.log(cr), '%dm'%(cr), size=8, va=va)

            # Find out if most recently drawn circle was outside all antennas, 
            # if so, no more circles will be drawn.
            if np.log(cr) > rmax:
                show_circle = False

        # For each antenna:
        for i, antenna in enumerate(antennas):

            # Draw the antenna position.
            subpl.plot(theta[i], np.log(r[i]), 'ko', ms=5, mfc='k')
    
            # Draw label for the antenna.
            subpl.text(theta[i], np.log(r[i]), ' '+antenna.name,
              size=8, color='k', ha='left', va='bottom', weight='bold')        
    
            # Create label for legend
            if max(r) < 100:
                label = r'{}: {:2.0f} m, {:4.0f}$^\circ$'.format(antenna.name, r[i], np.degrees(theta[i]))                
            if max(r) < 1000:
                label = r'{}: {:3.0f} m, {:4.0f}$^\circ$'.format(antenna.name, r[i], np.degrees(theta[i]))                
            elif max(r) < 3000:
                label = r'{}: {:3.2f} km, {:4.0f}$^\circ$'.format(antenna.name, 0.001*r[i], np.degrees(theta[i]))   
            else:
                label = r'{}: {:3.1f} km, {:4.0f}$^\circ$'.format(antenna.name, 0.001*r[i], np.degrees(theta[i]))

            # Draw a key in the legend for finding the antenna.
            subpl.annotate(label, xy=(0.5,0.5), xytext=(0.02,0.925-0.90*i/len(antennas)), 
              xycoords='figure fraction', textcoords='figure fraction', weight='bold', 
              arrowprops=None, color='black', ha='left', va='center', size=8)

        # Set minimum and maximum radius.
        subpl.set_rmax(rmax)
        subpl.set_rmin(rmin)


