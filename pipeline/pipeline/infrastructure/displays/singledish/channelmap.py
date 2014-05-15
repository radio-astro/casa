from __future__ import absolute_import

import os
import time
import abc
import numpy
import math
import pylab as pl
from matplotlib.ticker import MultipleLocator

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
from .utils import RADEClabel, RArotation, DECrotation, DDMMSSs, HHMMSSss
from .utils import sd_polmap as polmap
from .common import DPIDetail, SDImageDisplay, ShowPlot, draw_beam
from . import tpimage

LOG = infrastructure.get_logger(__name__)

class ChannelMapAxesManager(tpimage.ChannelAveragedAxesManager):
    def __init__(self, xformatter, yformatter, xlocator, ylocator, xrotation, yrotation, ticksize, colormap, nh, nv, brightnessunit):
        super(ChannelMapAxesManager,self).__init__(xformatter, yformatter,
                                                   xlocator, ylocator,
                                                   xrotation, yrotation,
                                                   ticksize, colormap)
        self.nh = nh
        self.nv = nv
        self.brightnessunit = brightnessunit
        self.nchmap = nh * nv
        self.left = 2.15 / 3.0
        self.width = 1.0 / 3.0 * 0.8
        self.bottom = 2.0 / 3.0 + 0.2 / 3.0
        self.height = 1.0 / 3.0 * 0.7

        self.numeric_formatter = pl.FormatStrFormatter('%.2f')
        
        self._axes_integmap = None
        self._axes_integsp_full = None
        self._axes_integsp_zoom = None
        self._axes_chmap = None
        
    @property
    def axes_integmap(self):
        if self._axes_integmap is None:
            axes = pl.axes([self.left, self.bottom, self.width, self.height])

            axes.xaxis.set_major_formatter(self.xformatter)
            axes.yaxis.set_major_formatter(self.yformatter)
            axes.xaxis.set_major_locator(self.xlocator)
            axes.yaxis.set_major_locator(self.ylocator)
            xlabels = axes.get_xticklabels()
            pl.setp(xlabels, 'rotation', self.xrotation, fontsize=self.ticksize)
            ylabels = axes.get_yticklabels()
            pl.setp(ylabels, 'rotation', self.yrotation, fontsize=self.ticksize)
            
            pl.xlabel('RA', size=self.ticksize)
            pl.ylabel('DEC', size=self.ticksize)
            if self.isgray:
                pl.gray()
            else:
                pl.jet()

            self._axes_integmap = axes
            
        return self._axes_integmap

    @property
    def axes_integsp_full(self):
        if self._axes_integsp_full is None:
            left = 1.0 / 3.0 + 0.1 / 3.0
            axes = pl.axes([left, self.bottom, self.width, self.height])
            axes.xaxis.set_major_formatter(self.numeric_formatter)
            pl.xticks(size=self.ticksize)
            pl.yticks(size=self.ticksize)
            pl.xlabel('Frequency (GHz)', size=self.ticksize)
            pl.ylabel('Intensity (%s)'%(self.brightnessunit), size=self.ticksize)
            pl.title('Integrated Spectrum', size=self.ticksize)

            self._axes_integsp_full = axes

        return self._axes_integsp_full

    @property
    def axes_integsp_zoom(self):
        if self._axes_integsp_zoom is None:
            left = 0.1 / 3.0
            axes = pl.axes([left, self.bottom, self.width, self.height])
            pl.xticks(size=self.ticksize)
            pl.yticks(size=self.ticksize)
            pl.xlabel('Relative Velocity w.r.t. Window Center (km/s)', size=self.ticksize)
            pl.ylabel('Intensity (%s)'%(self.brightnessunit), size=self.ticksize)
            pl.title('Integrated Spectrum (zoom)', size=self.ticksize)

            self._axes_integsp_zoom = axes

        return self._axes_integsp_zoom

    @property
    def axes_chmap(self):
        if self._axes_chmap is None:
            self._axes_chmap = list(self.__axes_chmap())

        return self._axes_chmap

    def __axes_chmap(self):
        for i in xrange(self.nchmap):
            x = i % self.nh
            y = self.nv - int(i / self.nh) - 1
            left = 1.0 / float(self.nh) * (x + 0.05)
            width = 1.0 / float(self.nh) * 0.9
            bottom = 1.0 / float((self.nv+2)) * (y + 0.05)
            height = 1.0 / float((self.nv+2)) * 0.85
            a = pl.axes([left, bottom, width, height])
            a.set_aspect('equal')
            a.xaxis.set_major_locator(pl.NullLocator())
            a.yaxis.set_major_locator(pl.NullLocator())
            if self.isgray:
                pl.gray()
            else:
                pl.jet()

            yield a
        


        
class SDChannelMapDisplay(SDImageDisplay):
    #MATPLOTLIB_FIGURE_ID = 8910
    NumChannelMap = 15
    NhPanel = 5
    NvPanel = 3
    #NumChannelMap = 12
    #NhPanel = 4
    #NvPanel = 3
    
    @property
    def num_valid_spectrum(self):
        return self.inputs.num_valid_spectrum

    def plot(self):

        self.init()

        return self.__plot_channel_map()

    def __valid_lines(self):
        reduction_group = self.context.observing_run.reduction_group
        ant_index = self.inputs.antennaid_list
        spwid_list = self.inputs.spwid_list

        line_list = []
        for (gid, group_desc) in reduction_group.items():
            for g in group_desc:
                found = False
                for (ant,spw) in zip(ant_index, spwid_list):
                    if g.antenna == ant and g.spw == spw:
                        found = True
                        break
                if found:
                    for ll_p in g.channelmap_range:
                        for ll in ll_p:
                            if not ll in line_list and ll[2] is True:
                                line_list.append(ll)
        return line_list

    def __plot_channel_map(self):
        colormap = 'color'
        scale_max = False
        scale_min = False
        
        plot_list = []
        
        # nrow is number of grid points for image
        nrow = self.nx * self.ny

        # retrieve line list from reduction group
        # key is antenna and spw id
        line_list = self.__valid_lines()

        # 2010/6/9 in the case of non-detection of the lines
        if len(line_list) == 0:
            return plot_list
            
        # Set data
        Map = numpy.zeros((self.NumChannelMap, (self.y_max - self.y_min + 1), (self.x_max - self.x_min + 1)), dtype=numpy.float32)
        RMSMap = numpy.zeros(((self.y_max - self.y_min + 1), (self.x_max - self.x_min + 1)), dtype=numpy.float32)
        #Total = numpy.zeros(((self.y_max - self.y_min + 1), (self.x_max - self.x_min + 1)), dtype=numpy.float32)
        # ValidSp: SQRT of Number of combined spectra for the weight
        #ValidSp = numpy.ones(nrow, dtype=numpy.float32)
        ValidSp = self.num_valid_spectrum.reshape((nrow,self.npol))
        #ValidSp = numpy.zeros(nrow, dtype=numpy.float32)
        #for row in range(nrow): ValidSp[row] = math.sqrt(Table[row][6])

        # Swap (x,y) to match the clustering result
        grid_size_arcsec = self.grid_size * 3600.0
        ExtentCM = ((self.x_max+0.5)*grid_size_arcsec, (self.x_min-0.5)*grid_size_arcsec, (self.y_min-0.5)*grid_size_arcsec, (self.y_max+0.5)*grid_size_arcsec)
        Extent = (self.ra_max+self.grid_size/2.0, self.ra_min-self.grid_size/2.0, self.dec_min-self.grid_size/2.0, self.dec_max+self.grid_size/2.0)
        span = max(self.ra_max - self.ra_min + self.grid_size, self.dec_max - self.dec_min + self.grid_size)
        (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

        # How to coordinate the map
        TickSize = 6
        #if ShowPlot: pl.ion()
        #else: pl.ioff()
        #pl.figure(self.MATPLOTLIB_FIGURE_ID)
        #if ShowPlot: pl.ioff()
        
        # 2008/9/20 Dec Effect has been taken into account
        #Aspect = 1.0 / math.cos(0.5 * (self.dec_min + self.dec_max) / 180.0 * 3.141592653)

        # Check the direction of the Velocity axis
        Reverse = (self.velocity[0] < self.velocity[1])

        # Initialize axes
        pl.clf()
        axes_manager = ChannelMapAxesManager(RAformatter, DECformatter,
                                             RAlocator, DEClocator,
                                             RArotation, DECrotation,
                                             TickSize, colormap,
                                             self.NhPanel, self.NvPanel,
                                             self.inputs.brightnessunit)
        axes_integmap = axes_manager.axes_integmap
        integmap_colorbar = None
        beam_circle = None
        axes_integsp1 = axes_manager.axes_integsp_full
        axes_integsp2 = axes_manager.axes_integsp_zoom
        axes_chmap = axes_manager.axes_chmap
        chmap_colorbar = [None for v in xrange(self.NvPanel)]
        
        # loop over detected lines
        ValidCluster = 0
        for line_window in line_list:
            # shift channel according to the edge parameter
            ChanC = int(line_window[0] + 0.5 - self.inputs.edge[0])
            if float(ChanC) == line_window[0] - self.inputs.edge[0]:
                VelC = self.velocity[ChanC]
            else:
                VelC = 0.5 * ( self.velocity[ChanC] + self.velocity[ChanC-1] )
            if ChanC > 0:
                ChanVelWidth = abs(self.velocity[ChanC] - self.velocity[ChanC - 1])
            else:
                ChanVelWidth = abs(self.velocity[ChanC] - self.velocity[ChanC + 1])

            # 2007/9/13 Change the magnification factor 1.2 to your preference (to Dirk)
            # be sure the width of one channel map is integer
            # 2014/1/12 factor 1.4 -> 1.0 since velocity structure was taken into account for the range in validation.py
            #ChanW = max(int(line_window[1] * 1.4 / self.NumChannelMap + 0.5), 1)
            ChanW = max(int(line_window[1] / self.NumChannelMap + 0.5), 1)
            #ChanB = int(ChanC - self.NumChannelMap / 2.0 * ChanW)
            ChanB = int(ChanC - self.NumChannelMap / 2.0 * ChanW + 0.5)
            # 2007/9/10 remedy for 'out of index' error
            #print '\nDEBUG0: Nc, ChanB, ChanW, NchanMap', Nc, ChanB, ChanW, self.NumChannelMap
            if ChanB < 0:
                ChanW = int(ChanC * 2.0 / self.NumChannelMap)
                if ChanW == 0: continue
                ChanB = int(ChanC - self.NumChannelMap / 2.0 * ChanW)
            elif ChanB + ChanW * self.NumChannelMap > self.nchan:
                ChanW = int((self.nchan - 1 - ChanC) * 2.0 / self.NumChannelMap)
                if ChanW == 0: continue
                ChanB = int(ChanC - self.NumChannelMap / 2.0 * ChanW)
            #print 'DEBUG1: Nc, ChanB, ChanW, NchanMap', Nc, ChanB, ChanW, self.NumChannelMap, '\n'

            chan0 = max(ChanB-1, 0)
            chan1 = min(ChanB + self.NumChannelMap*ChanW, self.nchan-1)
            V0 = min(self.velocity[chan0], self.velocity[chan1]) - VelC
            V1 = max(self.velocity[chan0], self.velocity[chan1]) - VelC
            #print 'chan0, chan1, V0, V1, VelC =', chan0, chan1, V0, V1, VelC

            vertical_lines = []
            # vertical lines for integrated spectrum #1
            pl.gcf().sca(axes_integsp1)
            vertical_lines.append(pl.axvline(x = self.frequency[max(ChanB,0)], linewidth=0.3, color='r'))
            vertical_lines.append(pl.axvline(x = self.frequency[chan1], linewidth=0.3, color='r'))

            # vertical lines for integrated spectrum #2
            pl.gcf().sca(axes_integsp2)
            for i in xrange(self.NumChannelMap + 1):
                ChanL = int(ChanB + i*ChanW)
                #if 0 <= ChanL and ChanL < nchan:
                if 0 < ChanL and ChanL < self.nchan:
                    vertical_lines.append(pl.axvline(x = 0.5*(self.velocity[ChanL]+self.velocity[ChanL-1]) - VelC, linewidth=0.3, color='r'))
                elif ChanL == 0:
                    vertical_lines.append(pl.axvline(x = 0.5*(self.velocity[ChanL]-self.velocity[ChanL+1]) - VelC, linewidth=0.3, color='r'))
                #print 'DEBUG: Vel[ChanL]', i, (self.velocity[ChanL]+self.velocity[ChanL-1])/2.0 - VelC
            
            # loop over polarizations
            for pol in xrange(self.npol):
                plotted_objects = []
                
                data = self.data.take([pol], axis=self.id_stokes).squeeze()
                masked_data = data * self.mask.take([pol], axis=self.id_stokes).squeeze()
                flattened_data = masked_data.reshape((nrow,self.nchan))
                valid = ValidSp[:,pol]
                
                # Integrated Spectrum
                t0 = time.time()

                # Draw Total Intensity Map
                Total = masked_data.sum(axis=2) * ChanVelWidth
                Total = numpy.flipud(Total.transpose())

                # 2008/9/20 DEC Effect
                pl.gcf().sca(axes_integmap)
                plotted_objects.append(pl.imshow(Total, interpolation='nearest', aspect=self.aspect, extent=Extent))
                #im = pl.imshow(Total, interpolation='nearest', aspect='equal', extent=Extent)

                xlim = axes_integmap.get_xlim()
                ylim = axes_integmap.get_ylim()
                
                # colorbar
                #print "min=%s, max of Total=%s" % (Total.min(),Total.max())
                if not (Total.min() == Total.max()): 
                    if not ((self.y_max == self.y_min) and (self.x_max == self.x_min)):
                        if integmap_colorbar is None:
                            integmap_colorbar = pl.colorbar(shrink=0.8)
                            for t in integmap_colorbar.ax.get_yticklabels():
                                newfontsize = t.get_fontsize()*0.5
                                t.set_fontsize(newfontsize)
                            integmap_colorbar.ax.set_title('[%s km/s]'%(self.inputs.brightnessunit))
                            lab = integmap_colorbar.ax.title
                            lab.set_fontsize(newfontsize)
                        else:
                            integmap_colorbar.set_clim((Total.min(),Total.max()))
                            integmap_colorbar.draw_all()

                # draw beam pattern
                if beam_circle is None:
                    beam_circle = draw_beam(axes_integmap, self.beam_radius, self.aspect, self.ra_min, self.dec_min)
                    
                pl.title('Total Intensity: CenterFreq.= %.3f GHz' % self.frequency[ChanC], size=TickSize)
                axes_integmap.set_xlim(xlim)
                axes_integmap.set_ylim(ylim)

                t1 = time.time()

                # Plot Integrated Spectrum #1
                Sp = numpy.sum(numpy.transpose((valid * numpy.transpose(flattened_data))),axis=0)/numpy.sum(valid,axis=0)
                #Sp = numpy.sum(flattened_data * valid.reshape((nrow,1)), axis=0)/valid.sum()
                (F0, F1) = (min(self.frequency[0], self.frequency[-1]), max(self.frequency[0], self.frequency[-1]))
                spmin = Sp.min()
                spmax = Sp.max()
                dsp = spmax - spmin
                spmin -= dsp * 0.1
                spmax += dsp * 0.1

                pl.gcf().sca(axes_integsp1)
                plotted_objects.extend(pl.plot(self.frequency, Sp, '-b', markersize=2, markeredgecolor='b', markerfacecolor='b'))
                #print 'DEBUG: Freq0, Freq1', self.frequency[ChanB], self.frequency[ChanB + self.NumChannelMap * ChanW]
                pl.axis([F0, F1, spmin, spmax])

                t2 = time.time()
                
                # Plot Integrated Spectrum #2
                pl.gcf().sca(axes_integsp2)
                plotted_objects.extend(pl.plot(self.velocity[chan0:chan1] - VelC, Sp[chan0:chan1], '-b', markersize=2, markeredgecolor='b', markerfacecolor='b'))
                pl.axis([V0, V1, spmin, spmax])

                t3 = time.time()
                
                # Draw Channel Map
                NMap = 0
                Vmax0 = Vmin0 = 0
                Title = []
                for i in xrange(self.NumChannelMap):
                    if Reverse: ii = i
                    else: ii = self.NumChannelMap - i - 1
                    C0 = ChanB + ChanW*ii
                    C1 = C0 + ChanW
                    if C0 < 0 or C1 >= self.nchan - 1: continue
                    velo = (self.velocity[C0] + self.velocity[C1-1]) / 2.0 - VelC
                    width = abs(self.velocity[C0] - self.velocity[C1])
                    Title.append('(Vel,Wid) = (%.1f, %.1f) (km/s)' % (velo, width))
                    NMap += 1
                    tmp = masked_data[:,:,C0:C1].sum(axis=2) * ChanVelWidth
                    Map[i] = numpy.flipud(tmp.transpose())
                Vmax0 = Map.max()
                Vmin0 = Map.min()
                if type(scale_max) == bool: Vmax = Vmax0 - (Vmax0 - Vmin0) * 0.1
                else:                       Vmax = scale_max
                if type(scale_min) == bool: Vmin = Vmin0 + (Vmax0 - Vmin0) * 0.1
                else:                       Vmin = scale_min

                if Vmax == 0 and Vmin == 0: 
                    print "No data to create channel maps. Check the flagging criteria."
                    return plot_list

                for i in xrange(NMap):
                    #im = pl.imshow(Map[i], vmin=Vmin, vmax=Vmax, interpolation='bilinear', aspect='equal', extent=Extent)
                    if Vmax != Vmin:
                        #im = pl.imshow(Map[i], vmin=Vmin, vmax=Vmax, interpolation='nearest', aspect='equal', extent=ExtentCM)
                        pl.gcf().sca(axes_chmap[i])
                        plotted_objects.append(pl.imshow(Map[i], vmin=Vmin, vmax=Vmax, interpolation='nearest', aspect='equal', extent=ExtentCM))
                        x = i % self.NhPanel
                        if x == (self.NhPanel - 1):
                            y = int(i / self.NhPanel)
                            if chmap_colorbar[y] is None:
                                cb=pl.colorbar()
                                for t in cb.ax.get_yticklabels():
                                    newfontsize = t.get_fontsize()*0.5
                                    t.set_fontsize(newfontsize)
                                cb.ax.set_title('[%s km/s]'%(self.inputs.brightnessunit))
                                lab=cb.ax.title
                                lab.set_fontsize(newfontsize)
                                chmap_colorbar[y] = cb
                            else:
                                chmap_colorbar[y].set_clim(Vmin,Vmax)
                                chmap_colorbar[y].draw_all()
                        pl.title(Title[i], size=TickSize)

                t4 = time.time()
                LOG.debug('PROFILE: integrated intensity map: %s sec'%(t1-t0))
                LOG.debug('PROFILE: integrated spectrum #1: %s sec'%(t2-t1))
                LOG.debug('PROFILE: integrated spectrum #2: %s sec'%(t3-t2))
                LOG.debug('PROFILE: channel map: %s sec'%(t4-t3))

                if ShowPlot: pl.draw()
                FigFileRoot = self.inputs.imagename + '.pol%s'%(pol)
                plotfile = os.path.join(self.stage_dir, FigFileRoot+'_ChannelMap_%s.png'%(ValidCluster))
                pl.savefig(plotfile, format='png', dpi=DPIDetail)

                for obj in plotted_objects:
                    obj.remove()
                
                parameters = {}
                parameters['intent'] = 'TARGET'
                parameters['spw'] = self.spw
                parameters['pol'] = polmap[pol]
                parameters['ant'] = self.antenna
                parameters['type'] = 'channel_map'
                parameters['file'] = self.inputs.imagename

                plot = logger.Plot(plotfile,
                                   x_axis='R.A.',
                                   y_axis='Dec.',
                                   field=self.inputs.source,
                                   parameters=parameters)
                plot_list.append(plot)

            ValidCluster += 1

            for line in vertical_lines:
                line.remove()

        return plot_list
        
