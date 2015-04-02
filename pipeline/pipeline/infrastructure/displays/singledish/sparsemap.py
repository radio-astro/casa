from __future__ import absolute_import

import os
import time
import abc
import numpy
import math
import pylab as pl

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
from .utils import DDMMSSs, HHMMSSss
from .utils import sd_polmap as polmap
from .utils import PlotObjectHandler
from .common import DPIDetail, SDImageDisplay, ShowPlot

LOG = infrastructure.get_logger(__name__)

NoData = -32767.0
NoDataThreshold = NoData + 10000.0

class SparseMapAxesManager(object):
    def __init__(self, nh, nv, brightnessunit, ticksize):
        self.nh = nh
        self.nv = nv
        self.ticksize = ticksize
        self.brightnessunit = brightnessunit
        self.numeric_formatter = pl.FormatStrFormatter('%.2f')
        
        self._axes_integsp = None
        self._axes_spmap = None
        
    @property
    def axes_integsp(self):
        if self._axes_integsp is None:
            axes = pl.subplot((self.nv+3)/2+3, 1, 1)
            axes.xaxis.set_major_formatter(self.numeric_formatter)
            pl.xlabel('Frequency(GHz)', size=(self.ticksize+1))
            pl.ylabel('Intensity(%s)'%(self.brightnessunit), size=(self.ticksize+1))
            pl.xticks(size=self.ticksize)
            pl.yticks(size=self.ticksize)
            pl.title('Spatially Integrated Spectrum', size=(self.ticksize + 1))

            self._axes_integsp = axes
        return self._axes_integsp

    @property
    def axes_spmap(self):
        if self._axes_spmap is None:
            self._axes_spmap = list(self.__axes_spmap())

        return self._axes_spmap

    def __axes_spmap(self):
        for x in xrange(self.nh):
            for y in xrange(self.nv):
                axes = pl.subplot(self.nv+3, self.nh+1, (self.nv - 1 - y + 2) * (self.nh + 1) + (self.nh - 1 - x) + 2)
                axes.yaxis.set_major_locator(pl.NullLocator())
                axes.xaxis.set_major_locator(pl.NullLocator())

                yield axes
        

    def setup_labels(self, label_ra, label_dec):
        for x in xrange(self.nh):
            a1 = pl.subplot(self.nv+3, self.nh+1, (self.nv + 2) * (self.nh + 1) + self.nh - x + 1)
            a1.set_axis_off()
            pl.text(0.5, 0.5, HHMMSSss((label_ra[x][0]+label_ra[x][1])/2.0, 0), horizontalalignment='center', verticalalignment='center', size=self.ticksize)
        for y in xrange(self.nv):
            a1 = pl.subplot(self.nv+3, self.nh+1, (self.nv + 1 - y) * (self.nh + 1) + 1)
            a1.set_axis_off()
            pl.text(0.5, 0.5, DDMMSSs((label_dec[y][0]+label_dec[y][1])/2.0, 0), horizontalalignment='center', verticalalignment='center', size=self.ticksize)
        a1 = pl.subplot(self.nv+3, self.nh+1, (self.nv + 2) * (self.nh + 1) + 1)
        a1.set_axis_off()
        pl.text(0.5, 1, 'Dec', horizontalalignment='center', verticalalignment='bottom', size=(self.ticksize+1))
        pl.text(1, 0.5, 'RA', horizontalalignment='center', verticalalignment='center', size=(self.ticksize+1))


class SDSparseMapPlotter(object):
    def __init__(self, nh, nv, step, brightnessunit):
        self.step = step
        ticksize = 10 - int(max(nh, nv) * step / (step - 1)) / 2
        self.axes = SparseMapAxesManager(nh, nv, brightnessunit, ticksize)
        
    @property
    def nh(self):
        return self.axes.nh
    
    @property
    def nv(self):
        return self.axes.nv
        
    @property
    def TickSize(self):
        return self.axes.ticksize
    
    def setup_labels(self, refpix_list, refval_list, increment_list):
        LabelRA = numpy.zeros((self.nh, 2), numpy.float32) + NoData
        LabelDEC = numpy.zeros((self.nv, 2), numpy.float32) + NoData
        refpix = refpix_list[0]
        refval = refval_list[0]
        increment = increment_list[0]
        #LOG.debug('axis 0: refpix,refval,increment=%s,%s,%s'%(refpix,refval,increment))
        for x in xrange(self.nh):
            x0 = (self.nh - x - 1) * self.step
            x1 = (self.nh - x - 2) * self.step + 1
            LabelRA[x][0] = refval + (x0 - refpix) * increment
            LabelRA[x][1] = refval + (x1 - refpix) * increment
        refpix = refpix_list[1]
        refval = refval_list[1]
        increment = increment_list[1]
        #LOG.debug('axis 1: refpix,refval,increment=%s,%s,%s'%(refpix,refval,increment))
        for y in xrange(self.nv):
            y0 = y * self.step
            y1 = (y + 1) * self.step - 1
            LabelDEC[y][0] = refval + (y0 - refpix) * increment
            LabelDEC[y][1] = refval + (y1 - refpix) * increment
        self.axes.setup_labels(LabelRA, LabelDEC)
        
    def plot(self, map_data, integrated_data, frequency, figfile):
        plot_helper = PlotObjectHandler()
        
        spmin = integrated_data.min()
        spmax = integrated_data.max()
        dsp = spmax - spmin
        spmin -= dsp * 0.1
        spmax += dsp * 0.1
        LOG.debug('spmin=%s, spmax=%s'%(spmin,spmax))
        
        xmin = min(frequency[0], frequency[-1])
        xmax = max(frequency[0], frequency[-1])

        # Auto scaling
        # to eliminate max/min value due to bad pixel or bad fitting,
        #  1/10-th value from max and min are used instead
        valid_index = numpy.where(map_data.min(axis=2) > NoDataThreshold)
        valid_data = map_data[valid_index[0],valid_index[1],:]
        ListMax = valid_data.max(axis=1)
        ListMin = valid_data.min(axis=1)
        LOG.debug('ListMax=%s'%(list(ListMax)))
        LOG.debug('ListMin=%s'%(list(ListMin)))
        if len(ListMax) == 0: 
            return False
        ymax = numpy.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
        ymin = numpy.sort(ListMin)[len(ListMin)/10]
        ymax = ymax + (ymax - ymin) * 0.2
        ymin = ymin - (ymax - ymin) * 0.1
        del ListMax, ListMin

        LOG.info('ymin=%s, ymax=%s'%(ymin,ymax))

        pl.gcf().sca(self.axes.axes_integsp)
        plot_helper.plot(frequency, integrated_data, color='b', linestyle='-', linewidth=0.4)
        (_xmin,_xmax,_ymin,_ymax) = pl.axis()
        pl.axis((_xmin,_xmax,spmin,spmax))

        for x in xrange(self.nh):
            for y in xrange(self.nv):
                pl.gcf().sca(self.axes.axes_spmap[y+(self.nh-x-1)*self.nv])
                if map_data[x][y].min() > NoDataThreshold:
                    plot_helper.plot(frequency, map_data[x][y], color='b', linestyle='-', linewidth=0.2)
                else:
                    plot_helper.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'NO DATA', ha='center', va='center', 
                                     size=(self.TickSize + 1))
                pl.axis([xmin, xmax, ymin, ymax])

        if ShowPlot: pl.draw()

        pl.savefig(figfile, format='png', dpi=DPIDetail)
        LOG.debug('figfile=\'%s\''%(figfile))
        
        plot_helper.clear()

        return True

        
class SDSparseMapDisplay(SDImageDisplay):
    #MATPLOTLIB_FIGURE_ID = 8910
    MaxPanel = 8
    
    @property
    def num_valid_spectrum(self):
        return self.inputs.num_valid_spectrum

    def plot(self):

        self.init()
        
        return self.__plot_sparse_map()

    def __plot_sparse_map(self):

        # Plotting routine
        #Mark = '-b'
        #if ShowPlot: pl.ion()
        #else: pl.ioff()
        #pl.figure(self.MATPLOTLIB_FIGURE_ID)
        #if ShowPlot: pl.ioff()

        pl.clf()
        
        num_panel = min(max(self.x_max - self.x_min + 1, self.y_max - self.y_min + 1), self.MaxPanel)
        STEP = int((max(self.x_max - self.x_min + 1, self.y_max - self.y_min + 1) - 1) / num_panel) + 1
        NH = (self.x_max - self.x_min) / STEP + 1
        NV = (self.y_max - self.y_min) / STEP + 1

        LOG.info('num_panel=%s, STEP=%s, NH=%s, NV=%s'%(num_panel,STEP,NH,NV))

        chan0 = 0
        chan1 = self.nchan
        
        plotter = SDSparseMapPlotter(NH, NV, STEP, self.brightnessunit)

        masked_data = self.data * self.mask

        plot_list = []

        refpix = [0,0]
        refval = [0,0]
        increment = [0,0]
        refpix[0], refval[0], increment[0] = self.image.direction_axis(0, unit='deg')
        refpix[1], refval[1], increment[1] = self.image.direction_axis(1, unit='deg')
        plotter.setup_labels(refpix, refval, increment)
        
        
        # loop over pol
        for pol in xrange(self.npol):
            
            masked_data_p = masked_data.take([pol], axis=self.id_stokes).squeeze()
            Plot = numpy.zeros((num_panel, num_panel, (chan1 - chan0)), numpy.float32) + NoData
            TotalSP = masked_data_p.sum(axis=0).sum(axis=0)
            mask_p = self.mask.take([pol], axis=self.id_stokes).squeeze()
            #isvalid = mask_p.prod(axis=2)
            isvalid = numpy.any(mask_p, axis=2)
            Nsp = sum(isvalid.flatten())
            LOG.info('Nsp=%s'%(Nsp))
            TotalSP /= Nsp

            for x in xrange(NH):
                x0 = x * STEP
                x1 = (x + 1) * STEP
                for y in xrange(NV):
                    y0 = y * STEP
                    y1 = (y + 1) * STEP
                    valid_index = isvalid[x0:x1,y0:y1].nonzero()
                    chunk = masked_data_p[x0:x1,y0:y1]
                    valid_sp = chunk[valid_index[0],valid_index[1],:]
                    Plot[x][y] = valid_sp.mean(axis=0)
 
            FigFileRoot = self.inputs.imagename+'.pol%s_Sparse'%(pol)
            plotfile = os.path.join(self.stage_dir, FigFileRoot+'_0.png')

            status = plotter.plot(Plot, TotalSP, self.frequency[chan0:chan1], 
                                  plotfile)
            
            if status:
                parameters = {}
                parameters['intent'] = 'TARGET'
                parameters['spw'] = self.inputs.spw
                parameters['pol'] = polmap[pol]
                parameters['ant'] = self.inputs.antenna
                parameters['type'] = 'sd_sparse_map'
                parameters['file'] = self.inputs.imagename

                plot = logger.Plot(plotfile,
                                   x_axis='Frequency',
                                   y_axis='Intensity',
                                   field=self.inputs.source,
                                   parameters=parameters)
                plot_list.append(plot)
            

        return plot_list

