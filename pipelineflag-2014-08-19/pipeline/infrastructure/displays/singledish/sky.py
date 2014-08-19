import os
import numpy
import pylab as pl
import string
from math import sqrt

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from . import common
from .utils import sd_polmap as polmap

LOG = logging.get_logger(__name__)

class SDSkyAxesManager(object):
    def __init__(self, figure_id=8909, baseframe=None):
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
        a.set_ylabel('Sky Level')
        
        return a


class SDSkyDisplay(common.SDCalibrationDisplay):
    MATPLOTLIB_FIGURE_ID = 8909

    def __init__(self, inputs):
        LOG.debug('instantiate...')
        super(SDSkyDisplay, self).__init__(inputs)

    def doplot(self, result, stage_dir):
        
        #plots = []
        #plot_list = self._plot_sky_vs_el(result, stage_dir)
        #plots.extend(plot_list)
        plots = self._plot_sky(result, stage_dir)

        return plots

    def _plot_sky(self, result, stage_dir):
        context = self.inputs.context
        caltable = result.outcome.applytable
        infile = result.outcome.infile
        vis = result.outcome.calto.vis
        frequencies = os.path.join(caltable, 'FREQUENCIES')
        calto = result.outcome.calto
        calfrom = result.outcome.calfrom[0]

        # get base frame
        with utils.open_table(frequencies) as tb:
            base_frame = tb.getkeyword('BASEFRAME')

        # set up axes
        axes_manager = SDSkyAxesManager(figure_id=self.MATPLOTLIB_FIGURE_ID,
                                        baseframe=base_frame)
        axes = axes_manager.axes

        # get scantable
        ms_name = os.path.join(context.output_dir, calto.vis)
        mses = context.observing_run.get_measurement_sets(names=[ms_name])
        scantable = None
        if len(mses) > 0:
            scantables = mses[0].scantables
            sts = [st for st in scantables if st.antenna.name == calto.antenna]
            scantable = sts[0]
        else:
            return []


        target_fields = [f for f in mses[0].fields if f.intents.issuperset(['TARGET'])]
        if len(target_fields) == 0:
            field_name = mses[0].fields[0].name
        else:
            field_name = target_fields[0].name

        # get base frequency
        base_freqs = {}
        with utils.open_table(caltable) as tb:
            spwlist = numpy.unique(tb.getcol('IFNO'))
            pollist = numpy.unique(tb.getcol('POLNO'))

            freq_ids = numpy.unique(tb.getcol('FREQ_ID'))
            nchans = {}
            for freq_id in freq_ids:
                tsel = tb.query('FREQ_ID==%s'%(freq_id))
                if tsel.nrows() > 0:
                    nchans[freq_id] = len(tsel.getcell('SPECTRA', 0))
                tsel.close()
        base_freqs = {}
        for freq_id in freq_ids:
            base_freqs[freq_id] = common.get_base_frequency(caltable, freq_id,
                                                            nchans[freq_id]) 
            
        # create time-direction map
        on_source = scantable.calibration_strategy['srctype']
        if on_source == 0:
            off_source = 1
        else:
            off_source = 99
        with utils.open_table(scantable.name) as tb:
            if calfrom.caltype == 'ps':
                tsel = tb.query('SRCTYPE==%s'%(off_source))
                if tsel.nrows() == 0:
                    time_stamps = tb.getcol('TIME')
                    direction_list = tb.getcol('DIRECTION')
                else:
                    time_stamps = tsel.getcol('TIME')
                    direction_list = tsel.getcol('DIRECTION')
                tsel.close()
            else:
                time_stamps = tb.getcol('TIME')
                direction_list = tb.getcol('DIRECTION')
            
        # generate plots
        plots = []
        qa = casatools.quanta
        #polmap = {0: 'XX', 1: 'YY', 2: 'XY', 3: 'YX'}
        with utils.open_table(caltable) as tb:
            for spw in spwlist:
                beam_size = scantable.beam_size[spw]
                beam_size_rad = qa.convert(beam_size, 'rad')['value']
                allowance = beam_size_rad * 0.1
                #LOG.debug('spw%s: allowance=%srad'%(spw, allowance))
                for pol in pollist:
                    tsel = tb.query('IFNO==%s && POLNO==%s'%(spw,pol))
                    freq_id = tsel.getcell('FREQ_ID', 0)
                    spectra = tsel.getcol('SPECTRA')
                    time_list = tsel.getcol('TIME')
                    tsel.close()

                    if nchans[freq_id] == 1:
                        LOG.debug('Skip channel averaged spw %s'%(spw))
                        continue

                    nearest_time_index = numpy.array([numpy.argmin(abs(time_stamps - t))
                                                      for t in time_list])

                    
                    # grouping
                    direction_group = []
                    for i in xrange(len(time_list)):
                        group_id = -1
                        for j in xrange(len(direction_group)):
                            index_list = nearest_time_index.take(direction_group[j] + [i])
                            ra_list = direction_list[0].take(index_list)
                            dec_list = direction_list[1].take(index_list)
                            wra = max(ra_list) - min(ra_list)
                            wdec = max(dec_list) - min(dec_list)
                            distance = sqrt(wra*wra + wdec*wdec)
                            if distance < allowance:
                                direction_group[j].append(i)
                                group_id = j
                        if group_id < 0:
                            direction_group.append([i])

                    #LOG.debug('direction_group=%s'%(direction_group))
                    
                    mean_direction = [direction_list.take(members,axis=1).mean(axis=1)
                                      for members in direction_group]

                    #LOG.debug('mean_direction=%s'%(mean_direction))
                    
                    direction_group_dict = {}
                    for i in xrange(len(direction_group)):
                        for v in direction_group[i]:
                            direction_group_dict[v] = i

                            
                    for i in xrange(len(direction_group)):
                        index_list = direction_group[i]
                        if len(index_list) > 0:
                            sky_level = spectra.take(index_list,axis=1).mean(axis=1)
                            base_freq = base_freqs[freq_id] * 1.0e-6

                            lines = pl.plot(base_freq, sky_level, '.-')
                            dirstring = 'J2000 %s %s'%(qa.formxxx('%srad'%(mean_direction[i][0]), 'hms'),qa.formxxx('%srad'%(mean_direction[i][1]), 'dms'))
                            title = '%s (spw %s pol %s)\n%s'%(os.path.basename(caltable), spw, pol, dirstring)
                            pl.title(title)
                            xmin = min(base_freq)
                            xmax = max(base_freq)
                            dx = (xmax - xmin) * 0.1
                            xmin -= dx
                            xmax += dx
                            ymin = min(sky_level)
                            ymax = max(sky_level)
                            dy = (ymax - ymin) * 0.1
                            ymin -= dy
                            ymax += dy
                            axes.set_xlim(xmin, xmax)
                            axes.set_ylim(ymin, ymax)
                            plotfile='%s_spw%s_pol%s_%s.png'%(os.path.basename(caltable),spw,pol,i)
                            LOG.debug('created %s'%(plotfile))
                            plotfile = os.path.join(stage_dir, plotfile)
                            parameters = {'spw': '%s'%(spw),
                                          'pol': '%s'%(polmap[pol]),
                                          'ant': calto.antenna,
                                          'type': 'sd',
                                          'intent': 'SKYCAL',
                                          'file': infile,
                                          'vis': vis}
                            plot = logger.Plot(plotfile,
                                               x_axis='Frequency',
                                               y_axis='Sky Level',
                                               field=field_name,
                                               parameters=parameters)
                            pl.savefig(plotfile)
                            if common.ShowPlot: pl.draw()
                            plots.append(plot)
                            
                            for line in lines:
                                line.remove()

                            #axes.set_color_cycle(pl.rcParams['axes.color_cycle'])
                            axes_manager.reset_color_cycle()
                        
                                    
        return plots
        
    def _plot_sky_vs_el(self, result, stage_dir):
        table = result.outcome.applytable
        calto = result.outcome.calto
        antenna_name = calto.antenna
        parent_ms = self.inputs.context.observing_run.get_ms(calto.vis)
        plots = []
        if common.ShowPlot: pl.ion()
        else: pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        if common.ShowPlot: pl.ioff()
        ylabel1 = 'Relative Sky Level'
        ylabel2 = 'Relative Sky Standard Deviation'
        xlabel = 'Elevation [deg]'
        #print xlabel, ylabel
        parameters_base = {}
        parameters_base['intent'] = 'SKY'
        parameters_base['ant'] = antenna_name
        parameters_base['type'] = 'sd'
        parameters_base['file'] = os.path.basename(table)
        with utils.open_table(table) as tb:
            spwlist = numpy.unique(tb.getcol('IFNO'))
            pollist = numpy.unique(tb.getcol('POLNO'))
            pl.clf()
            for spw in spwlist:
                for pol in pollist:
                    tsel = tb.query('IFNO==%s && POLNO==%s'%(spw,pol), sortlist='ELEVATION')
                    if tsel.nrows() == 0:
                        tsel.close()
                        continue

                    sky = tsel.getcol('SPECTRA')
                    el = tsel.getcol('ELEVATION') * 180.0 / numpy.pi
                    tsel.close()
                    if sky.shape[0] > 1:
                        title = 'Sky RMS vs Elevation'
                        sky_level = (sky * sky).mean(axis=0)
                        sky_variance = sky.var(axis=0)
                    else:
                        title = 'Sky Level vs Elevation'
                        sky_level = sky[0,:]
                        sky_variance = None
                    if common.ShowPlot: pl.ioff()
                    pl.clf()
                    sky_level /= sky_level[0]
                    pl.plot(el, sky_level, '.-', label='spw=%s, pol=%s'%(spw,pol))

                    pl.title(title)
                    pl.xlabel(xlabel)
                    pl.ylabel(ylabel1)
                    pl.legend(loc='best', numpoints=1, prop={'size':'small'})
                    plotfile='skylevel_vs_el_%s_spw%s_pol%s.png'%(os.path.basename(table),spw,pol)
                    plotfile=os.path.join(stage_dir, plotfile)
                    if common.ShowPlot: pl.draw()
                    pl.savefig(plotfile)
                    parameters = parameters_base.copy()
                    parameters['spw'] = spw
                    parameters['pol'] = pol
                    plot = logger.Plot(plotfile,
                                       x_axis='Elevation', y_axis='Sky Level',
                                       field=parent_ms.fields[0].name,
                                       parameters=parameters)
                    plots.append(plot)

                    if sky_variance is not None:
                        if common.ShowPlot: pl.ioff()
                        pl.clf()
                        title = 'Sky Standard Deviation vs Elevation'
                        sky_variance /= sky_variance[0]
                        pl.plot(el, sky_variance, '.-', label='spw=%s, pol=%s'%(spw,pol))
                        pl.title(title)
                        pl.xlabel(xlabel)
                        pl.ylabel(ylabel2)
                        pl.legend(loc='best', numpoints=1, prop={'size': 'small'})
                        plotfile='skystd_vs_el_%s_spw%s_pol%s.png'%(os.path.basename(table),spw,pol)
                        plotfile=os.path.join(stage_dir, plotfile)
                        if common.ShowPlot: pl.draw()
                        pl.savefig(plotfile)
                        plot = logger.Plot(plotfile,
                                           x_axis='Elevation', y_axis='Sky Standard Deviation',
                                           field=parent_ms.fields[0].name,
                                           parameters=parameters)
                        plots.append(plot)

        return plots
                


