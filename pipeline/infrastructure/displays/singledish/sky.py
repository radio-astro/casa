import os
import numpy
import pylab as pl

import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger
from . import common

class SDSkyDisplay(common.SDCalibrationDisplay):
    MATPLOTLIB_FIGURE_ID = 8909

    def __init__(self, inputs):
        super(SDSkyDisplay, self).__init__(inputs)

    def doplot(self, result, stage_dir):
        
        plots = []
        plot_list = self._plot_sky_vs_el(result, stage_dir)
        plots.extend(plot_list)

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
                        continue

                    sky = tsel.getcol('SPECTRA')
                    el = tsel.getcol('ELEVATION') * 180.0 / numpy.pi
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
                


