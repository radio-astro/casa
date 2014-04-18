import os
import numpy
import pylab as pl
import string

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from . import common
from . import utils
from .utils import sd_polmap as polmap

LOG = logging.get_logger(__name__)

def get_ylim(tsys):
    tshape = tsys.shape
    nchan = tshape[0] 
    nrow = tshape[-1] if len(tshape) > 1 else 1
    echan = max(1,int(nchan * 0.05))
    if len(tshape) == 1:
        ctsys = tsys[echan:-echan]
    else:
        ctsys = tsys[echan:-echan,::]
    stats = lambda x: (numpy.median(x), numpy.std(x), numpy.min(x), numpy.max(x))
    (med,std,tmin,tmax) = stats(tsys)
    (cmed,cstd,ctmin,ctmax) = stats(ctsys)
    fmin = 3 * int(tmin/ctmin)
    fmax = 3 * int(tmax/ctmax)
    ymin = med - fmin * cstd
    ymax = med + fmax * cstd
    return (ymin,ymax)


class TsysFreqAxesManager(object):
    def __init__(self, figure_id=8908, baseframe=None):
        self._axes = None
        self._baseframe = baseframe
        self._xlabel = string.Template('${frame} Frequency [MHz]')
        if pl.rcParams.has_key('axes.color_cycle'):
            self.color_cycle = pl.rcParams['axes.color_cycle']
        else:
            self.color_cycle = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
        if common.ShowPlot:
            pl.ion()
        else:
            pl.ioff()
        pl.figure(figure_id)
        pl.clf()

    @property
    def baseframe(self):
        return self._baseframe

    @baseframe.setter
    def baseframe(self, value):
        if self._baseframe != value:
            self._baseframe = value

            # update axes
            if self._axes is not None:
                self.axes.set_xlabel(self._xlabel.safe_substitute(frame=self.baseframe))
        
    @property
    def axes(self):
        if self._axes is None:
            self._axes = self.__axes()
        return self._axes

    def reset_color_cycle(self):
        if self._axes is None:
            return

        if hasattr(self._axes._get_lines, 'count'):
            self._axes._get_lines.count = 0
        else:
            self._axes.set_color_cycle(self.color_cycle)

    def __axes(self):
        a = pl.subplot(111)
        a.set_xlabel(self._xlabel.safe_substitute(frame=self.baseframe))
        a.set_ylabel('Tsys [K]')
        
        return a

class TsysTimeAxesManager(common.TimeAxesManager):
    def __init__(self):
        super(TsysTimeAxesManager, self).__init__()
        self._axes = None
        if pl.rcParams.has_key('axes.color_cycle'):
            self.color_cycle = pl.rcParams['axes.color_cycle']
        else:
            self.color_cycle = ['b', 'g', 'r', 'c', 'm', 'y', 'k']

    @property
    def axes(self):
        if self._axes is None:
            self._axes = self.__axes()
        return self._axes

    def reset_color_cycle(self):
        if self._axes is None:
            return

        if hasattr(self._axes._get_lines, 'count'):
            self._axes._get_lines.count = 0
        else:
            self._axes.set_color_cycle(self.color_cycle)

    def __axes(self):
        a = pl.subplot(111)
        a.set_xlabel('Time (UT)')
        a.set_ylabel('Channel Averaged Tsys [K]')
        a.xaxis.set_major_locator(self.locator)
        a.xaxis.set_major_formatter(utils.utc_formatter())

        # shift axes upward
        pos = a.get_position()
        left = pos.x0
        bottom = pos.y0 + 0.02
        width = pos.x1 - pos.x0
        height = pos.y1 - pos.y0
        a.set_position([left, bottom, width, height])
        
        return a
    
        
class SDTsysDisplay(common.SDCalibrationDisplay):
    MATPLOTLIB_FIGURE_ID = 8908

    def __init__(self, inputs):
        super(SDTsysDisplay, self).__init__(inputs)

    def doplot(self, result, stage_dir):
        
        plots = []
        plot_list = self._plot_tsys_vs_freq_summary(result, stage_dir)
        plots.extend(plot_list)

        plot_list = self._plot_tsys_vs_freq(result, stage_dir)
        plots.extend(plot_list)
        
        #plot_list = self._plot_tsys_vs_time(result, stage_dir)
        #plots.extend(plot_list)

        return plots

    def _plot_tsys_vs_freq_summary(self, result, stage_dir):
        table = result.outcome.applytable
        calto = result.outcome.calto
        antenna_name = calto.antenna
        parent_ms = self.inputs.context.observing_run.get_ms(calto.vis)
        # here, assumed that all rows have same FREQ_ID, which 
        # is true in almost case, at least for ALMA.
        with casatools.TableReader(table) as tb:
            freq_ids = numpy.unique(tb.getcol('FREQ_ID'))
        nchans = {}
        with casatools.TableReader(table) as tb:
            for freq_id in freq_ids:
                tsel = tb.query('FREQ_ID == %s'%(freq_id))
                nchans[freq_id] = len(tsel.getcell('TSYS',0))
                tsel.close()
        base_freqs = {}
        for freq_id in freq_ids:
            base_freqs[freq_id] = common.get_base_frequency(table, freq_id,
                                                            nchans[freq_id])
        base_frame = common.get_base_frame(table)

        plots = []
        if common.ShowPlot: pl.ion()
        else: pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        pl.clf()

        axes_manager = TsysFreqAxesManager(baseframe=base_frame)
        axes = axes_manager.axes
        
        parameters = {}
        parameters['intent'] = 'ATMCAL'
        parameters['spw'] = 'all'
        parameters['pol'] = 'all'
        parameters['ant'] = antenna_name
        parameters['type'] = 'sd'
        parameters['file'] = os.path.basename(table)
        parameters['vis'] = calto.vis
        with casatools.TableReader(table) as tb:
            tsel = tb.query('',sortlist='TIME')
            nrow = tsel.nrows()
            timestamps = tsel.getcol('TIME')
            time_group = [0]
            for row in xrange(nrow-1):
                if timestamps[row] != timestamps[row+1]:
                    time_group.append(row+1)
            time_group.append(nrow)
            for i in xrange(len(time_group)-1):
                start = time_group[i]
                end = time_group[i+1]
                title = 'Tsys vs Frequency (%s)'%(utils.mjd_to_datestring(timestamps[start],fmt='%b %d %H:%M:%S %Y UT'))
                #print title
                #print 'range=',range(start,end)
                lines = []
                for row in xrange(start,end):
                    spw = tsel.getcell('IFNO',row)
                    pol = tsel.getcell('POLNO',row)
                    tsys = tsel.getcell('TSYS',row)
                    freq_id = tsel.getcell('FREQ_ID',row)
                    freq = base_freqs[freq_id] * 1.0e-6
                    lines.extend(
                        pl.plot(freq, tsys, '-', label='spw%s, pol%s'%(spw,pol))
                    )
                pl.title(title)
                axes.legend(loc='best', numpoints=1, prop={'size':'small'})
                plotfile='tsys_vs_freq_summary_%s_time%s.png'%(os.path.basename(table),i)
                plotfile=os.path.join(stage_dir, plotfile)
                plot = logger.Plot(plotfile,
                                   x_axis='Frequency', y_axis='Tsys',
                                   field=parent_ms.fields[0].name,
                                   parameters=parameters)
                pl.savefig(plotfile)
                if common.ShowPlot: pl.draw()
                plots.append(plot)

                for line in lines:
                    line.remove()

                axes_manager.reset_color_cycle()
                
            tsel.close()
        return plots 

    def _plot_tsys_vs_freq(self, result, stage_dir):
        table = result.outcome.applytable
        calto = result.outcome.calto
        antenna_name = calto.antenna
        parent_ms = self.inputs.context.observing_run.get_ms(calto.vis)
        # here, assumed that all rows have same FREQ_ID, which 
        # is true in almost case, at least for ALMA.
        with casatools.TableReader(table) as tb:
            freq_ids = numpy.unique(tb.getcol('FREQ_ID'))
        nchans = {}
        with casatools.TableReader(table) as tb:
            for freq_id in freq_ids:
                tsel = tb.query('FREQ_ID == %s'%(freq_id))
                nchans[freq_id] = len(tsel.getcell('TSYS',0))
                tsel.close()
            tsys = tb.getcell('TSYS', 0)
            min_tsys = tsys.min()
            max_tsys = tsys.max()
            for irow in xrange(1,tb.nrows()):
                tsys = tb.getcell('TSYS', irow)
                min_tsys = min(tsys.min(), min_tsys)
                max_tsys = max(tsys.max(), max_tsys)
            dt = max_tsys - min_tsys
            ylim = (min_tsys - 0.1 * dt, max_tsys + 0.1 * dt)
        base_freqs = {}
        for freq_id in freq_ids:
            base_freqs[freq_id] = common.get_base_frequency(table, freq_id,
                                                            nchans[freq_id])
        base_frame = common.get_base_frame(table)

        plots = []
        if common.ShowPlot: pl.ion()
        else: pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        pl.clf()

        axes_manager = TsysFreqAxesManager(baseframe=base_frame)
        axes = axes_manager.axes
        
        parameters = {}
        parameters['intent'] = 'ATMCAL'
        parameters['spw'] = ''
        parameters['pol'] = ''
        parameters['ant'] = antenna_name
        parameters['type'] = 'sd'
        parameters['file'] = os.path.basename(table)
        parameters['vis'] = calto.vis
        with casatools.TableReader(table) as tb:
            spw_list = numpy.unique(tb.getcol('IFNO'))
            pol_list = numpy.unique(tb.getcol('POLNO'))
            for spw in spw_list:
                for pol in pol_list:
                    tsel = tb.query('IFNO==%s&&POLNO==%s'%(spw,pol),sortlist='TIME')
                    nrow = tsel.nrows()
                    if nrow == 0:
                        tsel.close()
                        continue
                    
                    freq_id = tsel.getcell('FREQ_ID', 0)
                    freq = base_freqs[freq_id] * 1.0e-6
                    df = max(freq) - min(freq)
                    xlim = (min(freq) - 0.1 * df, max(freq) + 0.1 * df)
                    timestamps = tsel.getcol('TIME')
                    title = 'Tsys vs Frequency (spw %s pol %s)'%(spw,pol)
                    #title = 'Tsys vs Frequency (%s)'%(utils.mjd_to_datestring(timestamps[start],fmt='%b %d %H:%M:%S %Y UT'))
                        #print title
                        #print 'range=',range(start,end)
                    lines = []
                    for irow in xrange(nrow):
                        tsys = tsel.getcell('TSYS', irow)
                        lines.extend(
                            pl.plot(freq, tsys, '.-', label='%s'%(utils.mjd_to_datestring(timestamps[irow],fmt='%b %d %H:%M:%S %Y UT')))
                            )
                        pl.title(title)
                    axes.legend(loc='best', numpoints=1, prop={'size':'small'})
                    plotfile='tsys_vs_freq_%s_spw%s_pol%s_time%s.png'%(os.path.basename(table),spw,pol,irow)
                    axes.set_xlim(xlim)
                    axes.set_ylim(ylim)
                    plotfile=os.path.join(stage_dir, plotfile)
                    parameters['spw'] = spw
                    parameters['pol'] = polmap[pol]
                    plot = logger.Plot(plotfile,
                                       x_axis='Frequency', y_axis='Tsys',
                                       field=parent_ms.fields[0].name,
                                       parameters=parameters.copy())
                    pl.savefig(plotfile)
                    if common.ShowPlot: pl.draw()
                    plots.append(plot)
                    
                    for line in lines:
                        line.remove()

                    axes_manager.reset_color_cycle()

                    tsel.close()
        return plots 

    def _plot_tsys_vs_time(self, result, stage_dir):
        table = result.outcome.applytable
        calto = result.outcome.calto
        antenna_name = calto.antenna
        parent_ms = self.inputs.context.observing_run.get_ms(calto.vis)
        plots = []
        if common.ShowPlot: pl.ion()
        else: pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        parameters_base = {}
        parameters_base['intent'] = 'ATMCAL'
        parameters_base['spw'] = ''
        parameters_base['pol'] = ''
        parameters_base['ant'] = antenna_name
        parameters_base['file'] = os.path.basename(table)
        parameters_base['vis'] = calto.vis

        # setup axes
        axes_manager = TsysTimeAxesManager()
        
        with casatools.TableReader(table) as tb:
            spwlist = numpy.unique(tb.getcol('IFNO'))
            pollist = numpy.unique(tb.getcol('POLNO'))
            
            timelist = utils.mjd_to_plotval(numpy.unique(tb.getcol('TIME')))
            if len(timelist) < 2:
                start_time = timelist[0] - 0.0104
                end_time = timelist[0] + 0.0104
                axes_manager.init(start_time, end_time)
            else:
                interval = timelist[1:]-timelist[:-1]
                mean_interval = numpy.abs(timelist[1:] - timelist[:-1]).mean()
                start_time = min(timelist) - 0.5 * mean_interval
                end_time = max(timelist) + 0.5 * mean_interval
                axes_manager.init(start_time=start_time, end_time=end_time)
                
            if common.ShowPlot: pl.ioff()
            pl.clf()
            axes = axes_manager.axes
            title = 'Tsys vs Time (Averaged Full Channel)'
            lines = []
            noedge_data = {}
            for spw in spwlist:
                noedge_data[spw] = {}
                for pol in pollist:
                    tsel = tb.query('IFNO==%s && POLNO==%s'%(spw,pol), sortlist='TIME')
                    tsys = tsel.getcol('TSYS')
                    time = utils.mjd_to_plotval(tsel.getcol('TIME'))
                    tsel.close()
                    atsys = tsys.mean(axis=0)
                    etsys = common.drop_edge(tsys)
                    if etsys is not None:
                        noedge_data[spw][pol] = [time, etsys.mean(axis=0)]
                    lines.extend(
                        pl.plot(time, atsys, '.-', label='spw=%s, pol=%s'%(spw,pol))
                    )
            pl.title(title)
            axes.set_xlim(start_time, end_time)
            axes.legend(loc='best', numpoints=1, prop={'size':'small'})
            plotfile='tsys_vs_time_%s.png'%(os.path.basename(table))
            plotfile=os.path.join(stage_dir, plotfile)
            if common.ShowPlot: pl.draw()
            pl.savefig(plotfile)
            LOG.debug('created %s'%(plotfile))
            parameters = parameters_base.copy()
            parameters['type'] = 'full channel'
            plot = logger.Plot(plotfile,
                               x_axis='Time', y_axis='Tsys',
                               field=parent_ms.fields[0].name,
                               parameters=parameters)
            plots.append(plot)

            # reset lines
            for line in lines:
                line.remove()

            # reset colormap
            axes_manager.reset_color_cycle()

            title = 'Tsys vs Time (Average Excluding Edge)'
            lines = []
            for (spw, values) in noedge_data.items():
                for (pol, data) in values.items():
                    lines.extend(
                        pl.plot(data[0], data[1], '.-', label='spw=%s, pol=%s'%(spw,pol))
                    )
            if len(lines) > 0:
                pl.title(title)
                axes.legend(loc='best', numpoints=1, prop={'size':'small'})
                axes.set_xlim(start_time, end_time)
                plotfile='tsys_vs_time_%s_noedge.png'%(os.path.basename(table))
                plotfile=os.path.join(stage_dir, plotfile)
                if common.ShowPlot: pl.draw()
                pl.savefig(plotfile)
                LOG.debug('created %s'%(plotfile))
                parameters = parameters_base.copy()
                parameters['type'] = 'without edge'
                plot = logger.Plot(plotfile,
                                   x_axis='Time', y_axis='Tsys',
                                   field=parent_ms.fields[0].name,
                                   parameters=parameters)
                plots.append(plot)
                for line in lines:
                    line.remove()
        return plots
                

