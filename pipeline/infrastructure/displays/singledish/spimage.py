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
from .utils import RADEClabel, RArotation, DECrotation, DDMMSSs, HHMMSSss
from .common import DPISummary, DPIDetail, SDImageDisplayInputs

LOG = infrastructure.get_logger(__name__)

NoData = -32767.0
NoDataThreshold = NoData + 10000.0

class SDSpectralImageDisplay(object):
    Inputs = SDImageDisplayInputs
    MATPLOTLIB_FIGURE_ID = 8910
    MaxPanel = 8

    def __init__(self, inputs):
        self.inputs = inputs
        self.context = self.inputs.context

    def plot(self):

        qa = casatools.quanta

        stage_dir = self.inputs.stage_dir

        # read data to storage
        with utils.open_image(self.inputs.imagename) as ia:
            image_shape = ia.shape()
            coordsys = ia.coordsys()
            coord_types = coordsys.axiscoordinatetypes()
            units = coordsys.units()
            id_direction = coord_types.index('Direction')
            id_direction = [id_direction, id_direction+1]
            id_spectral = coord_types.index('Spectral')
            id_stokes = coord_types.index('Stokes')
            LOG.info('id_direction=%s'%(id_direction))
            LOG.info('id_spectral=%s'%(id_spectral))
            LOG.info('id_stokes=%s'%(id_stokes))
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

        x_max = nx - 1
        x_min = 0
        y_max = ny - 1
        y_min = 0
        num_panel = min(max(x_max - x_min + 1, y_max - y_min + 1), self.MaxPanel)
        STEP = int((max(x_max - x_min + 1, y_max - y_min + 1) - 1) / num_panel) + 1
        NH = (x_max - x_min) / STEP + 1
        NV = (y_max - y_min) / STEP + 1

        LOG.info('num_panel=%s, STEP=%s, NH=%s, NV=%s'%(num_panel,STEP,NH,NV))

        chan0 = 0
        chan1 = nchan

        refpix = coordsys.referencepixel()['numeric'][id_spectral]
        refval = coordsys.referencevalue()['numeric'][id_spectral]
        increment = coordsys.increment()['numeric'][id_spectral]
        unit = units[id_spectral]
        if unit != 'GHz':
            refval = qa.convert(qa.quantity(refval,unit),'GHz')['value']
            increment = qa.convert(qa.quantity(increment,unit),'GHz')['value']
        Frequency = numpy.array([refval+increment*(i-refpix) for i in xrange(nchan)]) 
        xmin = min(Frequency[chan0], Frequency[chan1-1])
        xmax = max(Frequency[chan0], Frequency[chan1-1])

        TickSize = 10 - num_panel/2
        Format = PL.FormatStrFormatter('%.2f')

        masked_data = data * mask

        plot_list = []

        LabelRA = numpy.zeros((NH, 2), numpy.float32) + NoData
        LabelDEC = numpy.zeros((NV, 2), numpy.float32) + NoData
        LabelRADEC = numpy.zeros((num_panel, 2, 2), numpy.float32) + NoData
        refpix = coordsys.referencepixel()['numeric'][id_direction[0]]
        refval = coordsys.referencevalue()['numeric'][id_direction[0]]
        increment = coordsys.increment()['numeric'][id_direction[0]]
        unit = units[id_direction[0]]
        if unit != 'deg':
            refval = qa.convert(qa.quantity(refval,unit),'deg')['value']
            increment = qa.convert(qa.quantity(increment,unit),'deg')['value']
        LOG.info('refpix,refval,increment=%s,%s,%s'%(refpix,refval,increment))
        for x in xrange(NH):
            x0 = (NH - x - 1) * STEP
            x1 = (NH - x - 2) * STEP + 1
            LabelRA[x][0] = refval + (x0 - refpix) * increment
            LabelRA[x][1] = refval + (x1 - refpix) * increment
        refpix = coordsys.referencepixel()['numeric'][id_direction[1]]
        refval = coordsys.referencevalue()['numeric'][id_direction[1]]
        increment = coordsys.increment()['numeric'][id_direction[1]]
        unit = units[id_direction[1]]
        if unit != 'deg':
            refval = qa.convert(qa.quantity(refval,unit),'deg')['value']
            increment = qa.convert(qa.quantity(increment,unit),'deg')['value']
        LOG.info('refpix,refval,increment=%s,%s,%s'%(refpix,refval,increment))
        for y in xrange(NV):
            y0 = y * STEP
            y1 = (y + 1) * STEP - 1
            LabelDEC[y][0] = refval + (y0 - refpix) * increment
            LabelDEC[y][1] = refval + (y1 - refpix) * increment
        LOG.info('LabelDEC=%s'%(LabelDEC))
        LOG.info('LabelRA=%s'%(LabelRA))

        # loop over pol
        for pol in xrange(npol):
            masked_data_p = masked_data[:,:,:,pol]
            Plot = numpy.zeros((num_panel, num_panel, (chan1 - chan0)), numpy.float32) + NoData
            TotalSP = masked_data_p.sum(axis=0).sum(axis=0)
            mask_p = mask[:,:,:,pol]
            isvalid = mask_p.prod(axis=2)
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

            # Plotting routine
            ShowPlot = True
            Mark = '-b'
            if ShowPlot: PL.ion()
            PL.figure(self.MATPLOTLIB_FIGURE_ID)
            if ShowPlot: PL.ioff()

            AutoScale = True
            if AutoScale: 
                # to eliminate max/min value due to bad pixel or bad fitting,
                #  1/10-th value from max and min are used instead
                ListMax = []
                ListMin = []
                for x in range(num_panel):
                    for y in range(num_panel):
                        if Plot[x][y].min() > NoDataThreshold:
                            ListMax.append(Plot[x][y].max())
                            ListMin.append(Plot[x][y].min())
                if len(ListMax) == 0: return
                ymax = numpy.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
                ymin = numpy.sort(ListMin)[len(ListMin)/10]
                ymax = ymax + (ymax - ymin) * 0.2
                ymin = ymin - (ymax - ymin) * 0.1
                del ListMax, ListMin
            else:
                ymin = scale_min
                ymax = scale_max

            LOG.info('ymin=%s, ymax=%s'%(ymin,ymax))

            PL.cla()
            PL.clf()
            a0 = PL.subplot((NV+3)/2+3, 1, 1)
            a0.xaxis.set_major_formatter(Format)
        ##     PL.plot(Abcissa[1][chan0:chan1], TotalSP, color='b', linestyle='-', linewidth=0.4)
            PL.plot(Frequency[chan0:chan1], TotalSP, color='b', linestyle='-', linewidth=0.4)
            PL.xlabel('Frequency(GHz)', size=(TickSize+1))
            PL.ylabel('Intensity(K)', size=(TickSize+1))
            PL.xticks(size=TickSize)
            PL.yticks(size=TickSize)
            PL.title('Spatially Integrated Spectrum', size=(TickSize + 1))

            for x in range(NH):
                for y in range(NV):
                    a1 = PL.subplot(NV+3, NH+1, (NV - 1 - y + 2) * (NH + 1) + (NH - 1 - x) + 2)
                    if Plot[x][y].min() > NoDataThreshold:
        ##                 PL.plot(Abcissa[1][chan0:chan1], Plot[x][y], color='b', linestyle='-', linewidth=0.2)
                        PL.plot(Frequency[chan0:chan1], Plot[x][y], color='b', linestyle='-', linewidth=0.2)
                    else:
                        PL.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'NO DATA', horizontalalignment='center', verticalalignment='center', size=(TickSize + 1))
                    a1.yaxis.set_major_locator(PL.NullLocator())
                    a1.xaxis.set_major_locator(PL.NullLocator())
                    PL.axis([xmin, xmax, ymin, ymax])
            for x in range(NH):
                a1 = PL.subplot(NV+3, NH+1, (NV + 2) * (NH + 1) + NH - x + 1)
                a1.set_axis_off()
                #PL.text(0.5, 0.5, HHMMSSss((LabelRADEC[x][0][0]+LabelRADEC[x][0][1])/2.0, 0), horizontalalignment='center', verticalalignment='center', size=TickSize)
                PL.text(0.5, 0.5, HHMMSSss((LabelRA[x][0]+LabelRA[x][1])/2.0, 0), horizontalalignment='center', verticalalignment='center', size=TickSize)
            for y in range(NV):
                a1 = PL.subplot(NV+3, NH+1, (NV + 1 - y) * (NH + 1) + 1)
                a1.set_axis_off()
                #PL.text(0.5, 0.5, DDMMSSs((LabelRADEC[y][1][0]+LabelRADEC[y][1][1])/        2.0, 0), horizontalalignment='center', verticalalignment='center', size=TickSize)
                PL.text(0.5, 0.5, DDMMSSs((LabelDEC[y][0]+LabelDEC[y][1])/        2.0, 0), horizontalalignment='center', verticalalignment='center', size=TickSize)
            a1 = PL.subplot(NV+3, NH+1, (NV + 2) * (NH + 1) + 1)
            a1.set_axis_off()
            PL.text(0.5, 1, 'Dec', horizontalalignment='center', verticalalignment='bottom', size=(TickSize+1))
            PL.text(1, 0.5, 'RA', horizontalalignment='center', verticalalignment='center', size=(TickSize+1))
            if ShowPlot: PL.draw()

            FigFileDir = stage_dir
            FigFileRoot = self.inputs.imagename+'pol%s_Sparse'%(pol)
            plotfile = os.path.join(FigFileDir, FigFileRoot+'_0.png')
            if FigFileDir != False:
                PL.savefig(plotfile, format='png', dpi=DPIDetail)

            parameters = {}
            parameters['intent'] = 'TARGET'
            parameters['spw'] = self.inputs.spw
            parameters['pol'] = pol
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

