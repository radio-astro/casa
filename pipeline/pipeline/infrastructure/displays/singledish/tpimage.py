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
from .common import DPISummary, DPIDetail, SDImageDisplayInputs, draw_beam

LOG = infrastructure.get_logger(__name__)

class SDChannelAveragedImageDisplay(object):
    Inputs = SDImageDisplayInputs
    MATPLOTLIB_FIGURE_ID = 8911
    
    def __init__(self, inputs):
        self.inputs = inputs
        self.context = self.inputs.context

    def plot(self):
        ShowPlot = True
        qa = casatools.quanta
        to_deg = lambda q: qa.convert(q,'deg')['value']

        stage_dir = self.inputs.stage_dir
        antenna_names = [st.antenna.name for st in self.context.observing_run]
        try:
            idx = antenna_names.index(self.inputs.antenna)
        except:
            idx = 0
        beam_size = self.context.observing_run[idx].beam_size[self.inputs.spw]
        beamsize = to_deg(beam_size)
        gridsize = beamsize / 3.0
        #gridsize = 0.5 * beamsize 
        with utils.open_image(self.inputs.imagename) as ia:
            image_shape = ia.shape()
            coordsys = ia.coordsys()
            coord_types = coordsys.axiscoordinatetypes()
            id_direction = coord_types.index('Direction')
            id_direction = [id_direction, id_direction+1]
            id_spectral = coord_types.index('Spectral')
            id_stokes = coord_types.index('Stokes')
            nx,ny = image_shape[id_direction]
            nchan = image_shape[id_spectral] # should be 1
            npol = image_shape[id_stokes]
            data = ia.getchunk()
            mask = ia.getchunk(getmask=True)
            bottom = ia.toworld(numpy.zeros(len(image_shape),dtype=int), 'q')['quantity']
            top = ia.toworld(image_shape-1, 'q')['quantity']
            key = lambda x: '*%s'%(x+1)
            ra_min = bottom[key(id_direction[0])]
            ra_max = top[key(id_direction[0])]
            if ra_min > ra_max:
                ra_min,ra_max = ra_max,ra_min
            dec_min = bottom[key(id_direction[1])]
            dec_max = top[key(id_direction[1])]

        RAmax = to_deg(ra_max)
        RAmin = to_deg(ra_min)
        DECmax = to_deg(dec_max)
        DECmin = to_deg(dec_min)
        Extent = (RAmax+gridsize/2.0, RAmin-gridsize/2.0, DECmin-gridsize/2.0, DECmax+gridsize/2.0)
        span = max(RAmax - RAmin + gridsize, DECmax - DECmin + gridsize)
        (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

        # Plotting
        TickSize = 6
        if ShowPlot: PL.ion()
        PL.figure(self.MATPLOTLIB_FIGURE_ID)
        if ShowPlot: PL.ioff()
        # 2008/9/20 Dec Effect has been taken into account
        #Aspect = 1.0 / math.cos(Table[0][5] / 180. * 3.141592653)
        Aspect = 1.0 / math.cos(0.5 * (DECmax + DECmin) / 180. * 3.141592653)

        (x0, x1, y0, y1) = (0.25, 0.5, 0.25, 0.5)

        colormap = 'color'

        plot_list = []

        masked_data = data * mask
        for pol in xrange(npol):
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
                if not all(image_shape[id_direction] <= 1):
                    cb=PL.colorbar(shrink=0.8)
                    for t in cb.ax.get_yticklabels():
                        newfontsize = t.get_fontsize()*0.5
                        t.set_fontsize(newfontsize)
                    #cb.ax.set_title('[K km/s]')
                    cb.ax.set_title('[K]')
                    lab = cb.ax.title
                    lab.set_fontsize(newfontsize)

            # draw beam pattern
            draw_beam(a, 0.5 * beamsize, Aspect, RAmin, DECmin)

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


