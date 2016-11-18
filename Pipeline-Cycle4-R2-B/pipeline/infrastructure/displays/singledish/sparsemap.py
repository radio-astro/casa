from __future__ import absolute_import

import os
import time
import abc
import numpy
import math
import pylab as pl
import matplotlib.gridspec as gridspec

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.casatools as casatools
from .utils import DDMMSSs, HHMMSSss
#from .utils import sd_polmap as polmap
from .utils import PlotObjectHandler
from .common import DPIDetail, SDImageDisplay, ShowPlot
from . import atmutil

LOG = infrastructure.get_logger(__name__)

NoData = -32767.0
NoDataThreshold = NoData + 10000.0

def form3(n):
    if n <= 4:
        return 4
    elif n == 5:
        return 5
    elif n < 8:
        return 7
    else:
        return 8

def form4(n):
    if n <= 4:
        return 4
    elif n < 8:
        return 5
    else:
        return 5.5

class SparseMapAxesManager(object):
    def __init__(self, nh, nv, brightnessunit, ticksize, clearpanel=True, figure_id=None):
        self.nh = nh
        self.nv = nv
        self.ticksize = ticksize
        self.brightnessunit = brightnessunit
        self.numeric_formatter = pl.FormatStrFormatter('%.2f')
        
        self._axes_integsp = None
        self._axes_spmap = None
        self._axes_atm = None
        
        if figure_id is None:
            self.figure_id = self.MATPLOTLIB_FIGURE_ID()
        else:
            self.figure_id = figure_id
        self.figure = pl.figure(self.figure_id)
        if clearpanel:
            pl.clf()
            
        _f = form4(self.nv)
        self.gs_top = gridspec.GridSpec(1, 1,
                                        left=0.08,
                                        bottom=1.0 - 1.0/_f, top=0.96)
        self.gs_bottom = gridspec.GridSpec(self.nv+1, self.nh+1,
                                           hspace=0, wspace=0,
                                           left=0, right=0.95,
                                           bottom=0.01, top=1.0 - 1.0/_f-0.07)
#         self.gs_top = gridspec.GridSpec(1, 1,
#                                         bottom=1.0 - 1.0/form3(self.nv), top=0.96)
#         self.gs_bottom = gridspec.GridSpec(self.nv+1, self.nh+1,
#                                            hspace=0, wspace=0,
#                                            left=0, right=0.95,
#                                            bottom=0.01, top=1.0 - 1.0/form3(self.nv)-0.07)
        
    @staticmethod
    def MATPLOTLIB_FIGURE_ID():
        return 8910
        
    @property
    def axes_integsp(self):
        if self._axes_integsp is None:
            pl.figure(self.figure_id)
            axes = pl.subplot(self.gs_top[:,:])
            axes.cla()
            axes.xaxis.set_major_formatter(self.numeric_formatter)
            pl.xlabel('Frequency(GHz)', size=(self.ticksize+1))
            pl.ylabel('Intensity(%s)'%(self.brightnessunit), size=(self.ticksize+1))
            pl.xticks(size=self.ticksize)
            pl.yticks(size=self.ticksize)
            #pl.title('Spatially Integrated Spectrum', size=(self.ticksize + 1))
            pl.title('Spatially Averaged Spectrum', size=(self.ticksize + 1))

            self._axes_integsp = axes
        return self._axes_integsp

    @property
    def axes_spmap(self):
        if self._axes_spmap is None:
            pl.figure(self.figure_id)
            self._axes_spmap = list(self.__axes_spmap())

        return self._axes_spmap
    
    @property
    def axes_atm(self):
        if self._axes_atm is None:
            pl.figure(self.figure_id)
            self._axes_atm = self.axes_integsp.twinx()
            ylabel = self._axes_atm.set_ylabel('ATM Transmission', size=self.ticksize)
            ylabel.set_color('m')
            self._axes_atm.yaxis.set_tick_params(colors='m', labelsize=self.ticksize-1)
            self._axes_atm.yaxis.set_ticks([0.5, 0.8, 0.9, 0.95, 0.98, 1.0])
            self._axes_atm.yaxis.set_major_formatter(
                pl.FuncFormatter(lambda x, pos: '{}%'.format(int(x*100)))
                )
        return self._axes_atm

    def __axes_spmap(self):
        for x in xrange(self.nh):
            for y in xrange(self.nv):
                axes = pl.subplot(self.gs_bottom[self.nv - y - 1, self.nh - x])
                axes.cla()
                axes.yaxis.set_major_locator(pl.NullLocator())
                axes.xaxis.set_major_locator(pl.NullLocator())

                yield axes
        

    def setup_labels(self, label_ra, label_dec):
        for x in xrange(self.nh):
            a1 = pl.subplot(self.gs_bottom[-1, self.nh - x])
            a1.set_axis_off()
            if len(a1.texts) == 0:
                pl.text(0.5, 0.5, HHMMSSss((label_ra[x][0]+label_ra[x][1])/2.0, 0), horizontalalignment='center', verticalalignment='center', size=self.ticksize)
            else:
                a1.texts[0].set_text(HHMMSSss((label_ra[x][0]+label_ra[x][1])/2.0, 0))
        for y in xrange(self.nv):
            a1 = pl.subplot(self.gs_bottom[self.nv - y - 1, 0])
            a1.set_axis_off()
            if len(a1.texts) == 0:
                pl.text(0.5, 0.5, DDMMSSs((label_dec[y][0]+label_dec[y][1])/2.0, 0), horizontalalignment='center', verticalalignment='center', size=self.ticksize)
            else:
                a1.texts[0].set_text(DDMMSSs((label_dec[y][0]+label_dec[y][1])/2.0, 0))
        a1 = pl.subplot(self.gs_bottom[-1, 0])
        a1.set_axis_off()
        pl.text(0.5, 1, 'Dec', horizontalalignment='center', verticalalignment='bottom', size=(self.ticksize+1))
        pl.text(1, 0.5, 'RA', horizontalalignment='center', verticalalignment='center', size=(self.ticksize+1))


class SDSparseMapPlotter(object):
    def __init__(self, nh, nv, step, brightnessunit, clearpanel=True, figure_id=None):
        self.step = step
        if step > 1:
            ticksize = 10 - int(max(nh, nv) * step / (step - 1)) / 2
        elif step == 1:
            ticksize = 10 - int(max(nh, nv)) / 2
        self.axes = SparseMapAxesManager(nh, nv, brightnessunit, ticksize, clearpanel, figure_id)
        self.lines_averaged = None
        self.lines_map = None
        self.reference_level = None
        self.global_scaling = True
        self.deviation_mask = None
        self.atm_transmission = None
        self.atm_frequency = None
        
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
        
    def setup_lines(self, lines_averaged, lines_map=None):
        self.lines_averaged = lines_averaged
        self.lines_map = lines_map
        
    def setup_reference_level(self, level=0.0):
        self.reference_level = level
        
    def set_global_scaling(self):
        self.global_scaling = True
        
    def unset_global_scaling(self):
        self.global_scaling = False
        
    def set_deviation_mask(self, mask):
        self.deviation_mask = mask
        
    def set_atm_transmission(self, transmission, frequency):
        if self.atm_transmission is None:
            self.atm_transmission = [transmission]
            self.atm_frequency = [frequency]
        else:
            self.atm_transmission.append(transmission)
            self.atm_frequency.append(frequency)
    
    def unset_atm_transmission(self):
        self.atm_transmission = None
        self.atm_frequency = None
        
    def plot(self, map_data, averaged_data, frequency, fit_result=None, figfile=None):
        plot_helper = PlotObjectHandler()
        
        overlay_atm_transmission = self.atm_transmission is not None
        
        spmin = averaged_data.min()
        spmax = averaged_data.max()
        dsp = spmax - spmin
        spmin -= dsp * 0.1
        if overlay_atm_transmission:
            spmax += dsp * 0.4
        else:
            spmax += dsp * 0.1
        LOG.debug('spmin=%s, spmax=%s'%(spmin,spmax))
        
        global_xmin = min(frequency[0], frequency[-1])
        global_xmax = max(frequency[0], frequency[-1])
        LOG.debug('global_xmin=%s, global_xmax=%s'%(global_xmin,global_xmax))

        # Auto scaling
        # to eliminate max/min value due to bad pixel or bad fitting,
        #  1/10-th value from max and min are used instead
        valid_index = numpy.where(map_data.min(axis=2) > NoDataThreshold)
        valid_data = map_data[valid_index[0],valid_index[1],:]
        LOG.debug('valid_data.shape={shape}'.format(shape=valid_data.shape))
        if isinstance(map_data, numpy.ma.masked_array):
            def stat_per_spectra(spectra, oper):
                for v in spectra:
                    unmasked = v.data[v.mask == False]
                    if len(unmasked) > 0:
                        yield oper(unmasked)
            ListMax = numpy.fromiter(stat_per_spectra(valid_data, numpy.max), dtype=numpy.float64)
            ListMin = numpy.fromiter(stat_per_spectra(valid_data, numpy.min), dtype=numpy.float64)
#             ListMax = numpy.fromiter((numpy.max(v.data[v.mask == False]) for v in valid_data), 
#                                      dtype=numpy.float64)
#             ListMin = numpy.fromiter((numpy.min(v.data[v.mask == False]) for v in valid_data), 
#                                      dtype=numpy.float64)
            LOG.debug('ListMax from masked_array=%s'%(ListMax))
            LOG.debug('ListMin from masked_array=%s'%(ListMin))
        else:
            ListMax = valid_data.max(axis=1)
            ListMin = valid_data.min(axis=1)
        if len(ListMax) == 0 or len(ListMin) == 0: 
            return False
        #if isinstance(ListMin, numpy.ma.masked_array):
        #    ListMin = ListMin.data[ListMin.mask == False]
        #if isinstance(ListMax, numpy.ma.masked_array):
        #    ListMax = ListMax.data[ListMax.mask == False]
        LOG.debug('ListMax=%s'%(list(ListMax)))
        LOG.debug('ListMin=%s'%(list(ListMin)))            
        global_ymax = numpy.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
        global_ymin = numpy.sort(ListMin)[len(ListMin)/10]
        global_ymax = global_ymax + (global_ymax - global_ymin) * 0.2
        global_ymin = global_ymin - (global_ymax - global_ymin) * 0.1
        del ListMax, ListMin

        LOG.info('global_ymin=%s, global_ymax=%s'%(global_ymin,global_ymax))

        pl.gcf().sca(self.axes.axes_integsp)
        plot_helper.plot(frequency, averaged_data, color='b', linestyle='-', linewidth=0.4)
        (_xmin,_xmax,_ymin,_ymax) = pl.axis()
        #pl.axis((_xmin,_xmax,spmin,spmax))
        pl.axis((global_xmin, global_xmax, spmin, spmax))
        if self.lines_averaged is not None:
            for chmin, chmax in self.lines_averaged:
                fmin = ch_to_freq(chmin, frequency)
                fmax = ch_to_freq(chmax, frequency)
                LOG.debug('plotting line range for mean spectrum: [%s, %s]'%(chmin,chmax))
                plot_helper.axvspan(fmin, fmax, color='cyan')
        if self.deviation_mask is not None:
            LOG.debug('plotting deviation mask %s'%(self.deviation_mask))
            for chmin, chmax in self.deviation_mask:
                fmin = ch_to_freq(chmin, frequency)
                fmax = ch_to_freq(chmax, frequency)
                plot_helper.axvspan(fmin, fmax, ymin=0.95, ymax=1, color='red')
        if overlay_atm_transmission:
            pl.gcf().sca(self.axes.axes_atm)
            amin = 1.0
            amax = 0.0
            for (t, f) in zip(self.atm_transmission, self.atm_frequency):
                plot_helper.plot(f, t, color='m', linestyle='-', linewidth=0.4)           
                amin = min(amin, t.min())
                amax = max(amax, t.max())
            Y = 0.6
            ymin = (amin - Y) / (1.0 - Y)
            ymax = amax + (1.0 - amax) * 0.1
            pl.axis((global_xmin, global_xmax, ymin, ymax))
                    
        is_valid_fit_result = (fit_result is not None and fit_result.shape == map_data.shape)

        for x in xrange(self.nh):
            for y in xrange(self.nv):
                if self.global_scaling is True:
                    xmin = global_xmin
                    xmax = global_xmax
                    ymin = global_ymin
                    ymax = global_ymax
                else:
                    xmin = global_xmin
                    xmax = global_xmax
                    if map_data[x][y].min() > NoDataThreshold:
                        median = numpy.median(map_data[x][y])
                        mad = numpy.median(map_data[x][y] - median)
                        sigma = map_data[x][y].std()
                        ymin = median - 2.0 * sigma
                        ymax = median + 5.0 * sigma
                    else:
                        ymin = global_ymin
                        ymax = global_ymax
                    LOG.debug('Per panel scaling turned on: ymin=%s, ymax=%s (global ymin=%s, ymax=%s)'%(ymin,ymax,global_ymin,global_ymax))
                pl.gcf().sca(self.axes.axes_spmap[y+(self.nh-x-1)*self.nv])
                if map_data[x][y].min() > NoDataThreshold:
                    plot_helper.plot(frequency, map_data[x][y], color='b', linestyle='-', linewidth=0.2)
                    if self.lines_map is not None and self.lines_map[x][y] is not None:
                        for chmin, chmax in self.lines_map[x][y]:
                            fmin = ch_to_freq(chmin, frequency)
                            fmax = ch_to_freq(chmax, frequency)
                            LOG.debug('plotting line range for %s, %s: [%s, %s]'%(x,y,chmin,chmax))
                            plot_helper.axvspan(fmin, fmax, color='cyan')
                    elif self.lines_averaged is not None:
                        for chmin, chmax in self.lines_averaged:
                            fmin = ch_to_freq(chmin, frequency)
                            fmax = ch_to_freq(chmax, frequency)
                            LOG.debug('plotting line range for %s, %s (reuse lines_averaged): [%s, %s]'%(x,y,chmin,chmax))
                            plot_helper.axvspan(fmin, fmax, color='cyan')
                    if is_valid_fit_result:
                        plot_helper.plot(frequency, fit_result[x][y], color='r', linewidth=0.4)
                    elif self.reference_level is not None and ymin < self.reference_level and self.reference_level < ymax:
                        plot_helper.axhline(self.reference_level, color='r', linewidth=0.4) 
                else:
                    plot_helper.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'NO DATA', ha='center', va='center', 
                                     size=(self.TickSize + 1))
                pl.axis((xmin, xmax, ymin, ymax))

        if ShowPlot: pl.draw()

        if figfile is not None:
            pl.savefig(figfile, format='png', dpi=DPIDetail)
        LOG.debug('figfile=\'%s\''%(figfile))
        
        plot_helper.clear()

        return True
    
    def done(self):
        pl.close()
        del self.axes

        
class SDSparseMapDisplay(SDImageDisplay):
    #MATPLOTLIB_FIGURE_ID = 8910
    MaxPanel = 8
    
#     @property
#     def num_valid_spectrum(self):
#         return self.inputs.num_valid_spectrum

    def enable_atm(self):
        self.showatm = True
        
    def disable_atm(self):
        self.showatm = False

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
        
        if hasattr(self, 'showatm') and self.showatm is True:
            msid_list = numpy.unique(self.inputs.msid_list)
            for ms_id in msid_list:
                ms = self.inputs.context.observing_run.measurement_sets[ms_id]
                vis = ms.name
                antenna_id = 0 # nominal
                spw_id = self.inputs.spw
                atm_freq, atm_transmission = atmutil.get_transmission(vis=vis, antenna_id=antenna_id,
                                                            spw_id=spw_id, doplot=False)
                frame = self.frequency_frame
                if frame != 'TOPO':
                    # do conversion
                    field_id = self.inputs.fieldid_list[0]
                    field = ms.fields[field_id]
                    direction_ref = field.mdirection
                    start_time = ms.start_time
                    end_time = ms.end_time
                    me = casatools.measures
                    qa = casatools.quanta
                    qmid_time = qa.quantity(start_time['m0'])
                    qa.add(qmid_time, end_time['m0'])
                    qa.div(qmid_time, 2.0)
                    time_ref = me.epoch(rf=start_time['refer'], 
                                        v0=qmid_time)
                    position_ref = ms.antennas[antenna_id].position
                    
                    # initialize
                    me.done()
                    me.doframe(time_ref)
                    me.doframe(direction_ref)
                    me.doframe(position_ref)
                    def _tolsrk(x):
                        m = me.frequency(rf=frame, v0=qa.quantity(x, 'GHz'))
                        converted = me.measure(v=m, rf='LSRK')
                        qout = qa.convert(converted['m0'], outunit='GHz')
                        return qout['value']
                    tolsrk = numpy.vectorize(_tolsrk)
                    atm_freq = tolsrk(atm_freq)
                plotter.set_atm_transmission(atm_transmission, atm_freq)
      
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
                                  figfile=plotfile)
            
            if status:
                parameters = {}
                parameters['intent'] = 'TARGET'
                parameters['spw'] = self.inputs.spw
                parameters['pol'] = self.image.coordsys.stokes()[pol]#polmap[pol]
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

def ch_to_freq(ch, frequency):
    ich = int(ch)
    offset_min = ch - float(ich)
    if offset_min == 0 or ich == len(frequency) -1:
        freq = frequency[ich]
    else:
        jch = ich + 1
        df = frequency[jch] - frequency[ich]
        freq = frequency[ich] + offset_min * df
    return freq
   
