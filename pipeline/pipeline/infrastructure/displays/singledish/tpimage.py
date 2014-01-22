from __future__ import absolute_import

import os
import abc
import numpy
import math
import pylab as pl

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from .utils import RADEClabel, RArotation, DECrotation
from .utils import sd_polmap as polmap
from .common import DPISummary, DPIDetail, SDImageDisplay, ShowPlot, draw_beam

LOG = infrastructure.get_logger(__name__)

class ChannelAveragedAxesManager(object):
    def __init__(self, xformatter, yformatter, xlocator, ylocator, xrotation, yrotation, ticksize, colormap):
        self.xformatter = xformatter
        self.yformatter = yformatter
        self.xlocator = xlocator
        self.ylocator = ylocator
        self.xrotation = xrotation
        self.yrotation = yrotation
        self.isgray = (colormap == 'gray')
        
        self.ticksize = ticksize
        
        self._axes_tpmap = None

    @property
    def axes_tpmap(self):
        if self._axes_tpmap is None:
            axes = pl.axes([0.25,0.25,0.5,0.5])
            axes.xaxis.set_major_formatter(self.xformatter)
            axes.yaxis.set_major_formatter(self.yformatter)
            axes.xaxis.set_major_locator(self.xlocator)
            axes.yaxis.set_major_locator(self.ylocator)
            xlabels = axes.get_xticklabels()
            pl.setp(xlabels, 'rotation', self.xrotation, fontsize=self.ticksize)
            ylabels = axes.get_yticklabels()
            pl.setp(ylabels, 'rotation', self.yrotation, fontsize=self.ticksize)
            pl.title('Baseline RMS Map', size=self.ticksize)
            pl.xlabel('RA', size=self.ticksize)
            pl.ylabel('DEC', size=self.ticksize)

            if self.isgray:
                pl.gray()
            else:
                pl.jet()

            self._axes_tpmap = axes
            
        return self._axes_tpmap

class SDChannelAveragedImageDisplay(SDImageDisplay):
    MATPLOTLIB_FIGURE_ID = 8911
    
    def plot(self):
        self.init()
        
        Extent = (self.ra_max+self.grid_size/2.0, self.ra_min-self.grid_size/2.0, self.dec_min-self.grid_size/2.0, self.dec_max+self.grid_size/2.0)
        span = max(self.ra_max - self.ra_min + self.grid_size, self.dec_max - self.dec_min + self.grid_size)
        (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

        # Plotting
        if ShowPlot: pl.ion()
        else: pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        if ShowPlot: pl.ioff()
        pl.clf()
        
        # 2008/9/20 Dec Effect has been taken into account
        #Aspect = 1.0 / math.cos(0.5 * (self.dec_max + self.dec_min) / 180. * 3.141592653)

        colormap = 'color'
        TickSize = 6

        axes_manager = ChannelAveragedAxesManager(RAformatter, DECformatter,
                                                  RAlocator, DEClocator,
                                                  RArotation, DECrotation,
                                                  TickSize, colormap)
        axes_tpmap = axes_manager.axes_tpmap
        tpmap_colorbar = None
        beam_circle = None

        pl.gcf().sca(axes_tpmap)

        
        plot_list = []
        
        masked_data = self.data * self.mask
        for pol in xrange(self.npol):
            Total = masked_data.take([pol], axis=self.id_stokes).squeeze()
            Total = numpy.flipud(Total.transpose())
            tmin = Total.min()
            tmax = Total.max()

            # 2008/9/20 DEC Effect
            im = pl.imshow(Total, interpolation='nearest', aspect=self.aspect, extent=Extent)
            #im = pl.imshow(Total, interpolation='nearest', aspect='equal', extent=Extent)

            xlim = axes_tpmap.get_xlim()
            ylim = axes_tpmap.get_ylim()

            # colorbar
            #print "min=%s, max of Total=%s" % (tmin,tmax)
            if not (tmin == tmax): 
                #if not ((Ymax == Ymin) and (Xmax == Xmin)): 
                #if not all(image_shape[id_direction] <= 1):
                if self.nx > 1 or self.ny > 1:
                    if tpmap_colorbar is None:
                        tpmap_colorbar = pl.colorbar(shrink=0.8)
                        for t in tpmap_colorbar.ax.get_yticklabels():
                            newfontsize = t.get_fontsize()*0.5
                            t.set_fontsize(newfontsize)
                        #tpmap_colorbar.ax.set_title('[K km/s]')
                        tpmap_colorbar.ax.set_title('[K]')
                        lab = tpmap_colorbar.ax.title
                        lab.set_fontsize(newfontsize)
                    else:
                        tpmap_colorbar.set_clim((tmin,tmax))
                        tpmap_colorbar.draw_all()
                        
            # draw beam pattern
            if beam_circle is None:
                beam_circle = draw_beam(axes_tpmap, 0.5 * self.beam_size, self.aspect, self.ra_min, self.dec_min)

            pl.title('Total Power', size=TickSize)
            axes_tpmap.set_xlim(xlim)
            axes_tpmap.set_ylim(ylim)

            if ShowPlot: pl.draw()
            FigFileRoot = self.inputs.imagename+'.pol%s'%(pol)
            plotfile = os.path.join(self.stage_dir,FigFileRoot+'_TP.png')
            pl.savefig(plotfile, format='png', dpi=DPIDetail)

            im.remove()
            
            parameters = {}
            parameters['intent'] = 'TARGET'
            parameters['spw'] = self.inputs.spw
            parameters['pol'] = polmap[pol]
            parameters['ant'] = self.inputs.antenna
            parameters['type'] = 'sd_channel-averaged'
            parameters['file'] = self.inputs.imagename

            plot = logger.Plot(plotfile,
                               x_axis='R.A.',
                               y_axis='Dec.',
                               field=self.inputs.source,
                               parameters=parameters)
            plot_list.append(plot)

        return plot_list


