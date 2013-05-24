from __future__ import absolute_import

import os
import abc
import numpy
import math
import pylab as PL

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from .utils import RADEClabel, RArotation, DECrotation
from .common import DPISummary, DPIDetail, SDImageDisplay, ShowPlot, draw_beam

LOG = infrastructure.get_logger(__name__)

class SDChannelAveragedImageDisplay(SDImageDisplay):
    MATPLOTLIB_FIGURE_ID = 8911
    
    def plot(self):
        self.init()
        
        stage_dir = self.inputs.stage_dir

        Extent = (self.ra_max+self.grid_size/2.0, self.ra_min-self.grid_size/2.0, self.dec_min-self.grid_size/2.0, self.dec_max+self.grid_size/2.0)
        span = max(self.ra_max - self.ra_min + self.grid_size, self.dec_max - self.dec_min + self.grid_size)
        (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

        # Plotting
        TickSize = 6
        if ShowPlot: PL.ion()
        else: PL.ioff()
        PL.figure(self.MATPLOTLIB_FIGURE_ID)
        if ShowPlot: PL.ioff()
        # 2008/9/20 Dec Effect has been taken into account
        #Aspect = 1.0 / math.cos(Table[0][5] / 180. * 3.141592653)
        Aspect = 1.0 / math.cos(0.5 * (self.dec_max + self.dec_min) / 180. * 3.141592653)

        (x0, x1, y0, y1) = (0.25, 0.5, 0.25, 0.5)

        colormap = 'color'

        plot_list = []

        masked_data = self.data * self.mask
        for pol in xrange(self.npol):
            Total = masked_data[:,:,0,pol]
            Total = numpy.flipud(Total.transpose())

            PL.cla()
            PL.clf()

            a = PL.axes([x0, y0, x1, y1])
            # 2008/9/20 DEC Effect
            im = PL.imshow(Total, interpolation='nearest', aspect=Aspect, extent=Extent)
            #im = PL.imshow(Total, interpolation='nearest', aspect='equal', extent=Extent)

            a.xaxis.set_major_formatter(RAformatter)
            a.yaxis.set_major_formatter(DECformatter)
            a.xaxis.set_major_locator(RAlocator)
            a.yaxis.set_major_locator(DEClocator)
            xlabels = a.get_xticklabels()
            PL.setp(xlabels, 'rotation', RArotation, fontsize=TickSize)
            ylabels = a.get_yticklabels()
            PL.setp(ylabels, 'rotation', DECrotation, fontsize=TickSize)

            PL.xlabel('RA', size=TickSize)
            PL.ylabel('DEC', size=TickSize)
            if colormap == 'gray': PL.gray()
            else: PL.jet()

            # colorbar
            #print "min=%s, max of Total=%s" % (Total.min(),Total.max())
            if not (Total.min() == Total.max()): 
                #if not ((Ymax == Ymin) and (Xmax == Xmin)): 
                #if not all(image_shape[id_direction] <= 1):
                if self.nx > 1 or self.ny > 1:
                    cb=PL.colorbar(shrink=0.8)
                    for t in cb.ax.get_yticklabels():
                        newfontsize = t.get_fontsize()*0.5
                        t.set_fontsize(newfontsize)
                    #cb.ax.set_title('[K km/s]')
                    cb.ax.set_title('[K]')
                    lab = cb.ax.title
                    lab.set_fontsize(newfontsize)

            # draw beam pattern
            draw_beam(a, 0.5 * self.beam_size, Aspect, self.ra_min, self.dec_min)

            PL.title('Total Power', size=TickSize)

            if ShowPlot != False: PL.draw()
            FigFileDir = stage_dir
            FigFileRoot = self.inputs.imagename+'.pol%s'%(pol)
            plotfile = os.path.join(FigFileDir,FigFileRoot+'_TP.png')
            if FigFileDir != False:
                PL.savefig(plotfile, format='png', dpi=DPIDetail)

            parameters = {}
            parameters['intent'] = 'TARGET'
            parameters['spw'] = self.inputs.spw
            parameters['pol'] = pol
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


