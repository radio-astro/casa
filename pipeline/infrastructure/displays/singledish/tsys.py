import os
import numpy
import pylab as pl

import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger
from . import common

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
    

class SDTsysDisplay(common.SDCalibrationDisplay):
    MATPLOTLIB_FIGURE_ID = 8908

    def __init__(self, inputs):
        super(SDTsysDisplay, self).__init__(inputs)

    def doplot(self, result, stage_dir):
        
        plots = []
        plot_list = self._plot_tsys_vs_freq(result, stage_dir)
        plots.extend(plot_list)

        plot_list = self._plot_tsys_vs_el(result, stage_dir)
        plots.extend(plot_list)

        return plots

    def _plot_tsys_vs_freq(self, result, stage_dir):
        table = result.outcome.applytable
        calto = result.outcome.calto
        antenna_name = calto.antenna
        parent_ms = self.inputs.context.observing_run.get_ms(calto.vis)
        # here, assumed that all rows have same FREQ_ID, which 
        # is true in almost case, at least for ALMA.
        with utils.open_table(table) as tb:
            freq_ids = numpy.unique(tb.getcol('FREQ_ID'))
        nchans = {}
        with utils.open_table(table) as tb:
            for freq_id in freq_ids:
                tsel = tb.query('FREQ_ID == %s'%(freq_id))
                nchans[freq_id] = len(tsel.getcell('TSYS',0))
        base_freqs = {}
        for freq_id in freq_ids:
            base_freqs[freq_id] = common.get_base_frequency(table, freq_id,
                                                            nchans[freq_id])
        base_frame = common.get_base_frame(table)

        plots = []
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        ylabel = 'Tsys [K]'
        xlabel = '%s Frequency [MHz]'%(base_frame)
        #print xlabel, ylabel
        parameters = {}
        parameters['intent'] = 'ATMCAL'
        parameters['spw'] = ''
        parameters['pol'] = ''
        parameters['ant'] = antenna_name
        parameters['type'] = 'sd'
        parameters['file'] = os.path.basename(table)
        with utils.open_table(table) as tb:
            tsel = tb.query('',sortlist='TIME')
            nrow = tsel.nrows()
            timestamps = tsel.getcol('TIME')
            time_group = [0]
            for row in xrange(nrow-1):
                if timestamps[row] != timestamps[row+1]:
                    time_group.append(row+1)
            time_group.append(nrow)
            for i in xrange(len(time_group)-1):
                pl.clf()
                start = time_group[i]
                end = time_group[i+1]
                title = 'Tsys vs Frequency (MJD=%.1fsec)'%(timestamps[start]*86400.0)
                #print title
                #print 'range=',range(start,end)
                for row in xrange(start,end):
                    spw = tsel.getcell('IFNO',row)
                    pol = tsel.getcell('POLNO',row)
                    tsys = tsel.getcell('TSYS',row)
                    freq_id = tsel.getcell('FREQ_ID',row)
                    freq = base_freqs[freq_id] * 1.0e-6
                    pl.plot(freq, tsys, '.-', label='spw%s, pol%s'%(spw,pol))
                pl.title(title)
                pl.xlabel(xlabel)
                pl.ylabel(ylabel)
                pl.legend(loc='best', numpoints=1, prop={'size':'small'})
                plotfile='tsys_vs_freq_%s_time%s.png'%(os.path.basename(table),i)
                plotfile=os.path.join(stage_dir, plotfile)
                plot = logger.Plot(plotfile,
                                   x_axis='Frequency', y_axis='Tsys',
                                   field=parent_ms.fields[0].name,
                                   parameters=parameters)
                pl.savefig(plotfile)
                plots.append(plot)
        return plots 

    def _plot_tsys_vs_el(self, result, stage_dir):
        table = result.outcome.applytable
        calto = result.outcome.calto
        antenna_name = calto.antenna
        parent_ms = self.inputs.context.observing_run.get_ms(calto.vis)
        plots = []
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        ylabel = 'Channel-Averaged Tsys [K]'
        xlabel = 'Elevation [deg]'
        #print xlabel, ylabel
        parameters_base = {}
        parameters_base['intent'] = 'ATMCAL'
        parameters_base['spw'] = ''
        parameters_base['pol'] = ''
        parameters_base['ant'] = antenna_name
        parameters_base['file'] = os.path.basename(table)
        with utils.open_table(table) as tb:
            spwlist = numpy.unique(tb.getcol('IFNO'))
            pollist = numpy.unique(tb.getcol('POLNO'))
            pl.clf()
            title = 'Tsys vs Elevation (Averaged Full Channel)'
            for spw in spwlist:
                for pol in pollist:
                    tsel = tb.query('IFNO==%s && POLNO==%s'%(spw,pol), sortlist='ELEVATION')
                    tsys = tsel.getcol('TSYS')
                    el = tsel.getcol('ELEVATION') * 180.0 / numpy.pi
                    atsys = tsys.mean(axis=0)
                    pl.plot(el, atsys, '.-', label='spw=%s, pol=%s'%(spw,pol))
            pl.title(title)
            pl.xlabel(xlabel)
            pl.ylabel(ylabel)
            pl.legend(loc='best', numpoints=1, prop={'size':'small'})
            plotfile='tsys_vs_el_%s.png'%(os.path.basename(table))
            plotfile=os.path.join(stage_dir, plotfile)
            pl.savefig(plotfile)
            parameters = parameters_base.copy()
            parameters['type'] = 'full channel'
            plot = logger.Plot(plotfile,
                               x_axis='Elevation', y_axis='Tsys',
                               field=parent_ms.fields[0].name,
                               parameters=parameters)
            plots.append(plot)

            pl.clf()
            title = 'Tsys vs Elevation (Average Excluding Edge)'
            lines = []
            for spw in spwlist:
                for pol in pollist:
                    tsel = tb.query('IFNO==%s && POLNO==%s'%(spw,pol), sortlist='ELEVATION')
                    tsys = tsel.getcol('TSYS')
                    el = tsel.getcol('ELEVATION') * 180.0 / numpy.pi
                    etsys = common.drop_edge(tsys)
                    if etsys is not None:
                        atsys = etsys.mean(axis=0)
                        lines.append(pl.plot(el, atsys, '.-', label='spw=%s, pol=%s'%(spw,pol)))
            if len(lines) > 0:
                pl.title(title)
                pl.xlabel(xlabel)
                pl.ylabel(ylabel)
                pl.legend(loc='best', numpoints=1, prop={'size':'small'})
                plotfile='tsys_vs_el_%s_noedge.png'%(os.path.basename(table))
                plotfile=os.path.join(stage_dir, plotfile)
                pl.savefig(plotfile)
                parameters = parameters_base.copy()
                parameters['type'] = 'without edge'
                plot = logger.Plot(plotfile,
                                   x_axis='Elevation', y_axis='Tsys',
                                   field=parent_ms.fields[0].name,
                                   parameters=parameters)
                plots.append(plot)
        return plots
                

