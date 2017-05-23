from __future__ import absolute_import

import os
import time
import abc
import numpy
import math
import string
import pylab as pl
from matplotlib.ticker import MultipleLocator

from taskinit import gentools

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from .utils import RADEClabel, RArotation, DECrotation, DDMMSSs, HHMMSSss
from .common import DPISummary, DPIDetail, SingleDishDisplayInputs, ShowPlot, draw_beam, LightSpeed
LOG = infrastructure.get_logger(__name__)

class SDBaselineAxesManager(object):
    def __init__(self, nh, nv, formatter, locator, ticksize):
        self.nh = nh
        self.nv = nv
        self.formatter = formatter
        self.locator = locator
        self.ticksize = ticksize
        self.npanel = nh * nv

        self._axes = None
        
    @property
    def axes_list(self):
        if self._axes is None:
            self._axes = list(self._axes_list())

        return self._axes

    def _axes_list(self):
        title_yposition = 0.95
        if self.nv == 1 and self.nh == 1:
            pl.subplots_adjust(hspace=0.3)
            a0 = pl.subplot(121)
            a0.title.set_y(title_yposition)
            pl.xlabel('Channel', size=10)
            #pl.ylabel('Flux Density', size=10)
            pl.ylabel('Intensity (K)', size=10)

            yield a0

            a1 = pl.subplot(122)
            a1.title.set_y(title_yposition)
            pl.xlabel('Channel', size=10)
            #pl.ylabel('Flux Density', size=10)
            pl.ylabel('Intensity (arbitrary)', size=10)

            yield a1

        else:
            nhinv = 1.0 / float(self.nh)
            nvinv = 1.0 / float(self.nv)
            width = 1.0 / float(self.nh) * 0.4
            height = 1.0 / float(self.nv) * 0.8
            for y in xrange(self.nv-1,-1,-1):
                bottom = nvinv * (y + 0.1)
                for x in xrange(self.nh):
                    left0 = nhinv * (x + 0.1 + 0.05)
                    left1 = nhinv * (x + 0.5 + 0.05)
                    a0 = pl.axes([left0, bottom, width, height])
                    pl.ylabel('Intensity (arbitrary)', size=self.ticksize)
                    a0.xaxis.set_major_formatter(self.formatter)
                    a0.xaxis.set_major_locator(self.locator)
                    #pl.xticks(size=self.ticksize)
                    for t in a0.get_xticklabels():
                        t.set_fontsize((self.ticksize-1))
                    pl.yticks(size=self.ticksize-1)
                    a0.yaxis.set_label_coords(-0.3,0.5)
                    a0.xaxis.set_label_text('(GHz)', size=(self.ticksize-1))
                    a0.xaxis.set_label_coords(-0.05,-0.07)
                    a0.title.set_y(title_yposition)
                    a0.title.set_size(self.ticksize)


                    yield a0

                    a1 = pl.axes([left1, bottom, width, height])
                    a1.xaxis.set_major_formatter(self.formatter)
                    a1.xaxis.set_major_locator(self.locator)
                    for t in a1.get_xticklabels():
                        #tt = t.get_text()
                        #newlabs.append(tt)
                        t.set_fontsize((self.ticksize-1))
                    a1.yaxis.set_major_locator(pl.NullLocator())
                    a1.title.set_y(title_yposition)
                    a1.title.set_size(self.ticksize)

                    yield a1

class SDBaselineAllDisplay(object):
    Inputs = SingleDishDisplayInputs
    MaxHPanel = 5
    MaxVPanel = 5
    MATPLOTLIB_FIGURE_ID = 8907

    def __init__(self, inputs):
        self.inputs = inputs

    @property
    def context(self):
        return self.inputs.context

    @property
    def result(self):
        return self.inputs.result

    @property
    def stage_dir(self):
        return os.path.join(self.context.report_dir,'stage%s'%(self.result.stage_number))

    @property
    def datatable(self):
        return self.context.observing_run.datatable_instance

    def plot(self):
        start_time = time.time()
        
        if ShowPlot:
            pl.ion()
        else:
            pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        if ShowPlot:
            pl.ioff()

        pl.clf()

        baselined = self.result.outcome['baselined']
        edge = self.result.outcome['edge']
        plot_list = []
        for b in baselined:
            spw = b['spw']
            antennas = b['index']
            for ant in antennas:
                scantable = self.context.observing_run[ant]
                pre_baseline = scantable.name
                #post_baseline = b['name'][ant]
                post_baseline = scantable.baselined_name
                srctype = scantable.calibration_strategy['srctype']
                nchan = scantable.spectral_window[spw].nchan
                index_list = list(self.__index_list(ant, spw, srctype))
                t0 = time.time()
                plot_list.append(
                    list(self.doplot(ant,
                                     spw,
                                     pre_baseline,
                                     post_baseline,
                                     index_list,
                                     nchan,
                                     edge))
                    )
                t1 = time.time()
                LOG.debug('PROFILE doplot %s %s: elapsed time is %s sec'%(ant,spw,t1-t0))

        end_time = time.time()
        LOG.debug('PROFILE plot %s: elapsed time is %s sec'%(spw,end_time-start_time))
        return plot_list

    def __index_list(self, ant, spw, srctype):
        antennas = self.datatable.getcol('ANTENNA')
        ifnos = self.datatable.getcol('IF')
        srctypes = self.datatable.getcol('SRCTYPE')
        for irow in xrange(self.datatable.nrow):
            if antennas[irow] == ant and ifnos[irow] == spw and srctypes[irow] == srctype:
                yield irow

    def __spectra(self, name, rows):
        # dummy table object which is opened
        dummy = self.datatable.tb1
        taqlstring = 'USING STYLE PYTHON SELECT SPECTRA FROM "%s" WHERE ROWNUMBER() IN %s'%(name,list(rows))
        tx = dummy.taql(taqlstring)
        spectra = tx.getcol('SPECTRA')
        tx.close()
        return spectra.transpose()

    def doplot(self, ant, spw, pre_baseline, post_baseline, index_list, nchan, edge):
        pl.clf()
        
        #LOG.info('index_list=%s'%(index_list))
        nrow = len(index_list)
        # Variables for Panel
        TickSizeList = [12, 12, 10, 8, 6, 5, 5, 5, 5, 5, 5]
        if ((nrow-1) / (self.MaxHPanel*self.MaxVPanel)+1) > 1:
            (nh, nv) = (self.MaxHPanel, self.MaxVPanel)
        elif nrow == 1: (nh, nv) = (1, 1)
        elif nrow == 2: (nh, nv) = (1, 2)
        elif nrow <= 4: (nh, nv) = (2, 2)
        elif nrow <= 6: (nh, nv) = (2, 3)
        elif nrow <= 9: (nh, nv) = (3, 3)
        elif nrow <=12: (nh, nv) = (3, 4)
        elif nrow <=15: (nh, nv) = (3, 5)
        else: (nh, nv) = (self.MaxHPanel, self.MaxVPanel)
        NSpFit = nh * nv

        # fitparam: no use since 2010/6/12

        rows = self.datatable.tb1.getcol('ROW').take(index_list)
        net_flags = self.datatable.tb2.getcol('FLAG_SUMMARY').take(index_list)
        stats = self.datatable.tb2.getcol('STATISTICS').take(index_list,axis=1)
        nochanges = self.datatable.tb2.getcol('NOCHANGE').take(index_list)

        # Set edge mask region (not used?)
        EdgeL, EdgeR = edge

        vis = self.context.observing_run[ant].ms.basename
        antenna_name = self.context.observing_run[ant].antenna.name
        spwobj = self.context.observing_run[ant].spectral_window[spw]
        startval = (spwobj.refval - spwobj.refpix * spwobj.increment) * 1.0e-9
        increment = spwobj.increment * 1.0e-9
        endval = startval + increment * spwobj.nchan
        #Abcissa = numpy.arange(startval,endval,increment,dtype=numpy.float64)
        Abcissa = numpy.array([startval + i * increment for i in xrange(spwobj.nchan)], dtype=numpy.float64)
        LOG.debug('startval=%s, endval=%s, increment=%s'%(startval,endval,increment))
        pre_sp = self.__spectra(pre_baseline, rows)
        post_sp = self.__spectra(post_baseline, rows)

        # Setup Plot range, fontsize, ticks
        Xrange = [min(Abcissa[0], Abcissa[-1]), max(Abcissa[0], Abcissa[-1])]
        #TickSize = 12 - nh * 2
        TickSize = TickSizeList[nh]
        xtick = abs(Xrange[1] - Xrange[0]) / 3.0
        Order = int(math.floor(math.log10(xtick)))
        NewTick = int(xtick / (10**Order) + 1) * (10**Order)
        FreqLocator = MultipleLocator(NewTick)
        if Order < 0: FMT = '%%.%df' % (-Order)
        else: FMT = '%.2f'
        Format = pl.FormatStrFormatter(FMT)

        axes_manager = SDBaselineAxesManager(nh, nv,
                                             Format, FreqLocator,
                                             TickSize)
        axes_list = axes_manager.axes_list

        bgcolors = ['w' for i in xrange(nh*nv)]
        title_position = (0.5,0.95)
        
        # Main loop to plot all spectra (raw + reduced)
        counter = 0
        Npanel = 0
        plot_objects = []
        for index in xrange(nrow):
            idx = index_list[index]
            row = rows[index]
            SpIn = pre_sp[index]
            SpOut = post_sp[index]
            NoChange = nochanges[index]

            YMIN = min(SpIn.min(), SpOut.min())
            YMAX = max(SpIn.max(), SpOut.max())
            Yrange = [YMIN-(YMAX-YMIN)/10.0, YMAX+(YMAX-YMIN)/10.0]
            Mask = []

            tMASKLIST = self.datatable.tb2.getcell('MASKLIST',idx)
            if tMASKLIST[0][0] < 0:
                tMASKLIST = []

            Mask = [[Abcissa[int(mask[0])],Abcissa[int(mask[1])]]
                    for mask in tMASKLIST]

            if NoChange < 0:
                TitleColor = 'k'
                AddTitle = ''
            else:
                TitleColor = 'g'
                AddTitle = 'No Change since Cycle%d' % NoChange
    
            fitdata = SpIn - SpOut
            if net_flags[index] == 0: BackgroundColor = '#ff8888'
            else: BackgroundColor = 'w'
    
            if nh == 1 and nv == 1:
                pl.gcf().sca(axes_list[0])
                axes_list[0].title.set_text('%s\nRaw and Fit data : row = %d' % (AddTitle, row))
                axes_list[0].title.set_color(TitleColor)
                plot_objects.extend(
                    pl.plot(Abcissa, SpIn, color='b', linestyle='-', linewidth=0.2)
                    )
                plot_objects.extend(
                    pl.plot(Abcissa, fitdata, color='r', linestyle='-', linewidth=0.8)
                    )
                for x in range(len(Mask)):
                    plot_objects.extend(
                        pl.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.8)
                        )
                    plot_objects.extend(
                        pl.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.8)
                        )
                pl.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
                statistics = 'Pre-Fit RMS=%.2f, Post-Fit RMS=%.2f' % (stats[2][index], stats[1][index])
                plot_objects.append(
                    pl.text(0.05, 0.015, statistics, size=10)
                    )
                #pl.figtext(0.05, 0.015, statistics, size=10)

                pl.gcf().sca(axes_list[1])
                axes_list[1].title.set_text('%s\nReduced data : row = %d' % (AddTitle, row))
                axes_list[1].title.set_color(TitleColor)
                plot_objects.extend(
                    pl.plot(Abcissa, SpOut, color='b', linestyle='-', linewidth=0.2)
                    )
                for x in range(len(Mask)):
                    plot_objects.extend(
                        pl.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.8)
                        )
                    plot_objects.extend(
                        pl.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.8)
                        )
                pl.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
            else:
                # Axes objects
                a0 = axes_list[2*counter]
                a1 = axes_list[2*counter+1]

                # Change background color if necessary
                if bgcolors[counter] != BackgroundColor:
                    a0.set_axis_bgcolor(BackgroundColor)
                    a1.set_axis_bgcolor(BackgroundColor)
                    bgcolors[counter] = BackgroundColor
                
                # Plot spectrum before baseline fit + fitted result
                pl.gcf().sca(a0)
                a0.title.set_text('Fit: row = %d' % row)
                a0.title.set_color(TitleColor)
                plot_objects.extend(
                    pl.plot(Abcissa, SpIn, color='b', linestyle='-', linewidth=0.2)
                    )
                plot_objects.extend(
                    pl.plot(Abcissa, fitdata, color='r', linestyle='-', linewidth=0.8)
                    )
                for x in range(len(Mask)):
                    plot_objects.extend(
                        pl.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.2)
                        )
                    plot_objects.extend(
                        pl.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.2)
                        )
                pl.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])

                # Plot spectrum after baseline
                pl.gcf().sca(a1)
                if type(NoChange) == int:
                    a1.title.set_text(AddTitle)
                else:
                    a1.title.set_text('Reduced: row = %d' % row)
                a1.title.set_color(TitleColor)
                plot_objects.extend(
                    pl.plot(Abcissa, SpOut, color='b', linestyle='-', linewidth=0.2)
                    )
                for x in range(len(Mask)):
                    plot_objects.extend(
                        pl.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.2)
                        )
                    plot_objects.extend(
                        pl.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.2)
                        )
                pl.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])

            counter += 1
            if counter == NSpFit or idx == index_list[-1]:
                if counter < NSpFit:
                    for ipanel in xrange(counter*2, NSpFit*2):
                        axes_list[ipanel].clear()
                        axes_list[ipanel].set_axis_off()
                plotfile = os.path.join(self.stage_dir,'baseline_ant%s_spw%s_%s.png'%(ant,spw,Npanel))
                LOG.debug('plotfile=%s'%(plotfile))

                if ShowPlot:
                    pl.draw()

                # clear plot objects
                pl.savefig(plotfile, format='png', dpi=DPIDetail)
                for obj in plot_objects:
                    obj.remove()
                plot_objects = []

                # increment panel id
                Npanel += 1
                
                parameters = {'intent': 'TARGET',
                              'spw': spw,
                              'pol': [0,1],
                              'ant': antenna_name,
                              'vis': vis}
                plot_obj = logger.Plot(plotfile,
                                       x_axis='Frequency',
                                       y_axis='Intensity',
                                       field='',
                                       parameters=parameters)

                counter = 0
                
                yield plot_obj

