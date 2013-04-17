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
from .common import DPISummary, DPIDetail

LOG = infrastructure.get_logger(__name__)

class SDChannelAveragedImageDisplayInputs(object):
    def __init__(self, context, result):
        self.context = context
        self.imagename = result.outcome.imagename
        self.spw = result.outcome.spwlist
        self.antenna = result.outcome.antenna
        self.stage = result.stage_number
        self.source = result.outcome.sourcename
        LOG.info('inpus object created')

class SDChannelAveragedImageDisplay(object):
    Inputs = SDChannelAveragedImageDisplayInputs
    MATPLOTLIB_FIGURE_ID = 8911
    
    def __init__(self, inputs):
        self.inputs = inputs
        self.context = self.inputs.context
        LOG.info('task class created')

    def execute(self, dry_run=False, **parameters):
        return self.plot()

    def plot(self):
        LOG.info('START PLOT')
        ShowPlot = True
        qa = casatools.quanta
        to_deg = lambda q: qa.convert(q,'deg')['value']
        # Read data
        #NROW = len(Table)
    ##     NCHAN = 1
    ##     data = NP.zeros((NROW, NCHAN), dtype=NP.float32)
    ##     _tbtool = casac.homefinder.find_home_by_name('tableHome')
    ##     _tb = _tbtool.create()
    ##     _tb.open(DataIn)
    ##     for x in range(NROW):
    ##         data[x] = _tb.getcell('SPECTRA', x)
    ##     _tb.close()
    ##     del _tbtool, _tb
        #(NCHAN,data,Abcissa,Velocity) = GetDataFromFile( DataIn, NROW, False )
        NCHAN = 1
        stage_dir = os.path.join(self.context.report_dir,
                                 'stage%d'%(self.inputs.stage))
        antenna_names = [st.antenna.name for st in self.context.observing_run]
        try:
            idx = antenna_names.index(self.inputs.antenna)
        except:
            idx = 0
        beam_size = self.context.observing_run[idx].beam_size[self.inputs.spw]
        beamsize = to_deg(beam_size)
        gridsize = beamsize / 3.0
        #gridsize = 0.5 * beamsize 
        LOG.info('imagename=%s (%s)'%(self.inputs.imagename,type(self.inputs.imagename)))
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
            bottom = ia.toworld(numpy.zeros(len(image_shape),dtype=int), 'q')['quantity']
            top = ia.toworld(image_shape-1, 'q')['quantity']
            key = lambda x: '*%s'%(x+1)
            ra_min = bottom[key(id_direction[0])]
            ra_max = top[key(id_direction[0])]
            if ra_min > ra_max:
                ra_min,ra_max = ra_max,ra_min
            dec_min = bottom[key(id_direction[1])]
            dec_max = top[key(id_direction[1])]
            LOG.todo('mask must be properly handled')

        # Determine Plotting Area Size
        #(Xmin, Xmax, Ymin, Ymax) = (Table[0][2], Table[0][2], Table[0][3], Table[0][3])
        #(RAmin, RAmax, DECmin, DECmax) = (Table[0][4], Table[0][4], Table[0][5], Table[0][5])
        #for row in range(1, NROW):
        #    if   Xmin > Table[row][2]: (Xmin, RAmin) = (Table[row][2], Table[row][4])
        #    elif Xmax < Table[row][2]: (Xmax, RAmax) = (Table[row][2], Table[row][4])
        #    if   Ymin > Table[row][3]: (Ymin, DECmin) = (Table[row][3], Table[row][5])
        #    elif Ymax < Table[row][3]: (Ymax, DECmax) = (Table[row][3], Table[row][5])

        # Set data
        #Total = NP.zeros((Ymax - Ymin + 1, Xmax - Xmin + 1), dtype=NP.float32)
        ## If the data is valid, set the value. Otherwise set Zero
        #for row in range(NROW):
        #    if Table[row][6] > 0:
        #        Total[int(Ymax-(Table[row][3]-Ymin))][int(Xmax-(Table[row][2]-Xmin))] = data[row][0]
        #    else:
        #        Total[int(Ymax-1-(Table[row][3]-Ymin))][int(Xmax-1-(Table[row][2]-Xmin))] = 0.0

        # Swap (x,y) to match the clustering result
        #ExtentCM = ((Xmax+0.5)*gridsize*3600., (Xmin-0.5)*gridsize*3600., (Ymin-0.5)*gridsize*3600., (Ymax+0.5)*gridsize*3600.)
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

        for pol in xrange(npol):
            Total = data[:,:,0,pol]
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
            if beamsize > 0:
                Mark = 'r-'
                #R = beamsize / gridsize
                #R = beamsize
                R = 0.5 * beamsize
                x = []
                y = []
                for t in range(50):
                    # 2008/9/20 DEC Effect
                    x.append(R * (math.sin(t * 0.13) + 1.0) * Aspect + RAmin)
                    #x.append(R * (math.sin(t * 0.13) + 1.0) + RAmin)
                    y.append(R * (math.cos(t * 0.13) + 1.0) + DECmin)
                PL.plot(x, y, Mark)

            PL.title('Total Power', size=TickSize)

            if ShowPlot != False: PL.draw()
            FigFileDir = stage_dir
            FigFileRoot = self.inputs.imagename+'.pol%s'%(pol)
            plotfile = os.path.join(FigFileDir,FigFileRoot+'_TP.png')
            if FigFileDir != False:
                PL.savefig(plotfile, format='png', dpi=DPIDetail)
                #if os.access(FigFileDir+'listofplots.txt', os.F_OK):
                #    BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
                #else:
                #    BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
                #    print >> BrowserFile, 'TITLE: Gridding'
                #    print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
                #    print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
                #    print >> BrowserFile, FigFileRoot+'_TP.png'
                #BrowserFile.close()

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


