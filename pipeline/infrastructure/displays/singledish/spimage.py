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
from .common import DPISummary, DPIDetail, SDImageDisplay, ShowPlot, draw_beam
LOG = infrastructure.get_logger(__name__)

NoData = -32767.0
NoDataThreshold = NoData + 10000.0
LightSpeed = 29972.458 # km/s

class SDSpectralImageDisplay(SDImageDisplay):
    MATPLOTLIB_FIGURE_ID = 8910
    MaxPanel = 8
    NumChannelMap = 15
    NhPanel = 5
    NvPanel = 3
    #NumChannelMap = 12
    #NhPanel = 4
    #NvPanel = 3
    
    @property
    def num_valid_spectrum(self):
        flat_data = self.inputs.num_valid_spectrum
        return self.__reshape2d(flat_data)

    @property
    def rms(self):
        flat_data = self.inputs.rms
        return self.__reshape2d(flat_data)

    def __reshape2d(self, array2d):
        array3d = array2d.reshape((self.npol,self.ny,self.nx)).transpose()
        return numpy.flipud(array3d)

    def plot(self):
        qa = casatools.quanta

        self.init()
        
        plot_list = []
        plot_list.extend(self.__plot_sparse_map())
        plot_list.extend(self.__plot_channel_map())

        return plot_list

    def __get_lines(self):
        reduction_group = self.context.observing_run.reduction_group
        ant_index = [i for i in xrange(len(self.context.observing_run))
                     if self.context.observing_run[i].antenna.name == self.antenna]
        mygroup = None
        for (k,v) in reduction_group.items():
            if v[0].spw == self.spw:
                mygroup = v
                break

        line_list = []
        for g in mygroup:
            if g.antenna in ant_index:
                for ll_p in g.linelist:
                    for ll in ll_p:
                        if not ll in line_list:
                            line_list.append(ll)
        return line_list
                
    def __gen_line_list(self, line_list):
        for line_window in line_list:
            if line_window[2] is True:
                yield line_window
        
    def __plot_channel_map(self):
        colormap = 'color'
        scale_max = False
        scale_min = False
        
        plot_list = []
        
        # nrow is number of grid points for image
        nrow = self.nx * self.ny

        # retrieve line list from reduction group
        # key is antenna and spw id
        line_list = self.__get_lines()

        # Number of the clusters determined in Process4
        Ncluster = len(line_list)
        # 2010/6/9 in the case of non-detection of the lines
        if Ncluster == 0: return plot_list

        # Set data
        Map = numpy.zeros((self.NumChannelMap, (self.y_max - self.y_min + 1), (self.x_max - self.x_min + 1)), dtype=numpy.float32)
        RMSMap = numpy.zeros(((self.y_max - self.y_min + 1), (self.x_max - self.x_min + 1)), dtype=numpy.float32)
        #Total = numpy.zeros(((self.y_max - self.y_min + 1), (self.x_max - self.x_min + 1)), dtype=numpy.float32)
        # ValidSp: SQRT of Number of combined spectra for the weight
        LOG.todo('ValidSp must be transferred from hsd_imaging')
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
        if ShowPlot: PL.ion()
        else: PL.ioff()
        PL.figure(self.MATPLOTLIB_FIGURE_ID)
        if ShowPlot: PL.ioff()
        # 2008/9/20 Dec Effect has been taken into account
        Aspect = 1.0 / math.cos(0.5 * (self.dec_min + self.dec_max) / 180.0 * 3.141592653)

        # Check the direction of the Velocity axis
        Reverse = (self.velocity[0] < self.velocity[1])
 
        # loop over detected lines
        ValidCluster = 0
        #for Nc in range(Ncluster):
        for line_window in self.__gen_line_list(line_list):            
            ChanC = int(line_window[0] + 0.5)
            if float(ChanC) == line_window[0]:
                VelC = self.velocity[ChanC]
            else:
                 VelC = 0.5 * ( self.velocity[ChanC] + self.velocity[ChanC-1] )
            if ChanC > 0:
                ChanVelWidth = abs(self.velocity[ChanC] - self.velocity[ChanC - 1])
            else:
                ChanVelWidth = abs(self.velocity[ChanC] - self.velocity[ChanC + 1])

            # 2007/9/13 Change the magnification factor 1.2 to your preference (to Dirk)
            # be sure the width of one channel map is integer
            ChanW = max(int(line_window[1] * 1.4 / self.NumChannelMap + 0.5), 1)
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

            # loop over polarizations
            for pol in xrange(self.npol):
                data = self.data[:,:,:,pol]
                masked_data = data * self.mask[:,:,:,pol]
                flattened_data = masked_data.reshape((nrow,self.nchan))
                valid = ValidSp[:,pol]
                
                # Integrated Spectrum
                Sp = numpy.sum(numpy.transpose((valid * numpy.transpose(flattened_data))),axis=0)/numpy.sum(valid,axis=0)
                (F0, F1) = (min(self.frequency[0], self.frequency[-1]), max(self.frequency[0], self.frequency[-1]))

                Title = []
                N = 0

                # Draw Total Intensity Map
                Total = masked_data.sum(axis=2) * ChanVelWidth
                Total = numpy.flipud(Total.transpose())
                PL.cla()
                PL.clf()

                x0 = 2.0 / 3.0 + 0.15 / 3.0
                x1 = 1.0 / 3.0 * 0.8
                y0 = 2.0 / 3.0 + 0.2 / 3.0
                y1 = 1.0 / 3.0 * 0.7
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
                    if not ((self.y_max == self.y_min) and (self.x_max == self.x_min)): 
                       cb=PL.colorbar(shrink=0.8)
                       for t in cb.ax.get_yticklabels():
                           newfontsize = t.get_fontsize()*0.5
                           t.set_fontsize(newfontsize)
                       cb.ax.set_title('[K km/s]')
                       lab = cb.ax.title
                       lab.set_fontsize(newfontsize)

                # draw beam pattern
                draw_beam(a, self.beam_radius, Aspect, self.ra_min, self.dec_min)

                PL.title('Total Intensity: CenterFreq.= %.3f GHz' % self.frequency[ChanC], size=TickSize)

                Format = PL.FormatStrFormatter('%.2f')
                # Plot Integrated Spectrum #1
                x0 = 1.0 / 3.0 + 0.1 / 3.0
                a = PL.axes([x0, y0, x1, y1])
                a.xaxis.set_major_formatter(Format)
                PL.plot(self.frequency, Sp, '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
                PL.axvline(x = self.frequency[ChanB], linewidth=0.3, color='r')
                PL.axvline(x = self.frequency[ChanB + self.NumChannelMap * ChanW], linewidth=0.3, color='r')
                #print 'DEBUG: Freq0, Freq1', self.frequency[ChanB], self.frequency[ChanB + self.NumChannelMap * ChanW]
                PL.xticks(size=TickSize)
                PL.yticks(size=TickSize)
                PL.xlabel('Frequency (GHz)', size=TickSize)
                PL.ylabel('Intensity (K)', size=TickSize)
                #PL.setp(xlabels, 'rotation', 45, fontsize=TickSize)
                Range = PL.axis()
                PL.axis([F0, F1, Range[2], Range[3]])
                PL.title('Integrated Spectrum', size=TickSize)

                # Plot Integrated Spectrum #2
                x0 = 0.1 / 3.0
                a = PL.axes([x0, y0, x1, y1])
                PL.plot(self.velocity[chan0:chan1] - VelC, Sp[chan0:chan1], '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
                for i in range(self.NumChannelMap + 1):
                    ChanL = int(ChanB + i*ChanW)
                    #if 0 <= ChanL and ChanL < nchan:
                    #    PL.axvline(x = Abcissa[2][ChanL] - VelC, linewidth=0.3, color='r')
                    if 0 < ChanL and ChanL < self.nchan:
        ##                 PL.axvline(x = 0.5*(Abcissa[2][ChanL]+Abcissa[2][ChanL-1]) - VelC, linewidth=0.3, color='r')
                        PL.axvline(x = 0.5*(self.velocity[ChanL]+self.velocity[ChanL-1]) - VelC, linewidth=0.3, color='r')
                    elif ChanL == 0:
        ##                 PL.axvline(x = 0.5*(Abcissa[2][ChanL]-Abcissa[2][ChanL+1]) - VelC, linewidth=0.3, color='r')
                        PL.axvline(x = 0.5*(self.velocity[ChanL]-self.velocity[ChanL+1]) - VelC, linewidth=0.3, color='r')
                    #print 'DEBUG: Vel[ChanL]', i, (self.velocity[ChanL]+self.velocity[ChanL-1])/2.0 - VelC
                PL.xticks(size=TickSize)
                PL.yticks(size=TickSize)
                Range = PL.axis()
                PL.axis([V0, V1, Range[2], Range[3]])
                PL.xlabel('Relative Velocity w.r.t. Window Center (km/s)', size=TickSize)
                PL.ylabel('Intensity (K)', size=TickSize)
                PL.title('Integrated Spectrum (zoom)', size=TickSize)

                # Draw Channel Map
                NMap = 0
                Vmax0 = Vmin0 = 0
                for i in range(self.NumChannelMap):
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
                    x = i % self.NhPanel
                    y = self.NvPanel - int(i / self.NhPanel) - 1
                    x0 = 1.0 / float(self.NhPanel) * (x + 0.05)
                    x1 = 1.0 / float(self.NhPanel) * 0.9
                    y0 = 1.0 / float((self.NvPanel+2)) * (y + 0.05)
                    y1 = 1.0 / float((self.NvPanel+2)) * 0.85
                    a = PL.axes([x0, y0, x1, y1])
                    a.set_aspect('equal')
                    a.xaxis.set_major_locator(PL.NullLocator())
                    a.yaxis.set_major_locator(PL.NullLocator())
                    #im = PL.imshow(Map[i], vmin=Vmin, vmax=Vmax, interpolation='bilinear', aspect='equal', extent=Extent)
                    if not (Vmax==Vmin):
                        im = PL.imshow(Map[i], vmin=Vmin, vmax=Vmax, interpolation='nearest', aspect='equal', extent=ExtentCM)
                        if colormap == 'gray': PL.gray()
                        else: PL.jet()
                        #if x == (self.NhPanel - 1): PL.colorbar()
                        if x == (self.NhPanel - 1):
                            cb=PL.colorbar()
                            for t in cb.ax.get_yticklabels():
                                newfontsize = t.get_fontsize()*0.5
                                t.set_fontsize(newfontsize)
                            cb.ax.set_title('[K km/s]')
                            lab=cb.ax.title
                            lab.set_fontsize(newfontsize)

                        PL.title(Title[i], size=TickSize)

                if ShowPlot: PL.draw()
                FigFileRoot = self.inputs.imagename + '.pol%s'%(pol)
                plotfile = os.path.join(self.stage_dir, FigFileRoot+'_ChannelMap_%s.png'%(ValidCluster))
                PL.savefig(plotfile, format='png', dpi=DPIDetail)

                parameters = {}
                parameters['intent'] = 'TARGET'
                parameters['spw'] = self.spw
                parameters['pol'] = pol
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


        # Draw RMS Map
        for pol in xrange(self.npol):
        
            #for row in range(nrow):
                #if Table[row][6] > 0:
                #    RMSMap[self.y_max-(Table[row][3]-self.y_min)][self.x_max-(Table[row][2]-self.x_min)] = Table[row][8]
                #else:
                #    RMSMap[self.y_max-1-(Table[row][3]-self.y_min)][self.x_max-1-(Table[row][2]-self.x_min)] = 0.0
            RMSMap = self.rms[:,:,pol] * (self.num_valid_spectrum[:,:,pol] > 0)
            RMSMap = numpy.flipud(RMSMap.transpose())
            LOG.debug('RMSMap=%s'%(RMSMap))
            LOG.todo('RMS must be transferred from imaging results')
            PL.cla()
            PL.clf()
            a = PL.axes([0.25, 0.25, 0.5, 0.5])
            # 2008/9/20 DEC Effect
            im = PL.imshow(RMSMap, interpolation='nearest', aspect=Aspect, extent=Extent)
            #im = PL.imshow(RMSMap, interpolation='nearest', aspect='equal', extent=Extent)

            a.xaxis.set_major_formatter(RAformatter)
            a.yaxis.set_major_formatter(DECformatter)
            a.xaxis.set_major_locator(RAlocator)
            a.yaxis.set_major_locator(DEClocator)
            xlabels = a.get_xticklabels()
            PL.setp(xlabels, 'rotation', RArotation, fontsize=8)
            ylabels = a.get_yticklabels()
            PL.setp(ylabels, 'rotation', DECrotation, fontsize=8)

            PL.xlabel('RA', size=12)
            PL.ylabel('DEC', size=12)
            if colormap == 'gray': PL.gray()
            else: PL.jet()

            # colorbar
            if not (RMSMap.min() == RMSMap.max()): 
                if not ((self.y_max == self.y_min) and (self.x_max == self.x_min)): 
                   cb=PL.colorbar(shrink=0.8)
                   cb.ax.set_title('[K]')
                   lab = cb.ax.title

            # draw beam pattern
            draw_beam(a, self.beam_radius, Aspect, self.ra_min, self.dec_min)

            PL.title('Baseline RMS Map', size=12)

            if ShowPlot: PL.draw()
            FigFileRoot = self.inputs.imagename + '.pol%s'%(pol)
            plotfile = os.path.join(self.stage_dir, FigFileRoot+'_rmsmap.png')
            PL.savefig(plotfile, format='png', dpi=DPISummary)

            parameters = {}
            parameters['intent'] = 'TARGET'
            parameters['spw'] = self.spw
            parameters['pol'] = pol
            parameters['ant'] = self.antenna
            parameters['type'] = 'channel_map'
            parameters['file'] = self.inputs.imagename
            parameters['type'] = 'rms_map'

            plot2 = logger.Plot(plotfile,
                                x_axis='R.A.',
                                y_axis='Dec.',
                                field=self.inputs.source,
                                parameters=parameters)
            plot_list.append(plot2)

        return plot_list
        
    def __plot_sparse_map(self):
        #x_max = self.nx - 1
        #x_min = 0
        #y_max = self.ny - 1
        #y_min = 0
        num_panel = min(max(self.x_max - self.x_min + 1, self.y_max - self.y_min + 1), self.MaxPanel)
        STEP = int((max(self.x_max - self.x_min + 1, self.y_max - self.y_min + 1) - 1) / num_panel) + 1
        NH = (self.x_max - self.x_min) / STEP + 1
        NV = (self.y_max - self.y_min) / STEP + 1

        LOG.info('num_panel=%s, STEP=%s, NH=%s, NV=%s'%(num_panel,STEP,NH,NV))

        chan0 = 0
        chan1 = self.nchan

        #(refpix, refval, increment) = self.image.spectral_axis(unit='GHz')
        #self.frequency = numpy.array([refval+increment*(i-refpix) for i in xrange(nchan)])
        xmin = min(self.frequency[chan0], self.frequency[chan1-1])
        xmax = max(self.frequency[chan0], self.frequency[chan1-1])

        TickSize = 10 - num_panel/2
        Format = PL.FormatStrFormatter('%.2f')

        masked_data = self.data * self.mask

        plot_list = []

        LabelRA = numpy.zeros((NH, 2), numpy.float32) + NoData
        LabelDEC = numpy.zeros((NV, 2), numpy.float32) + NoData
        LabelRADEC = numpy.zeros((num_panel, 2, 2), numpy.float32) + NoData
        (refpix, refval, increment) = self.image.direction_axis(0, unit='deg')
        LOG.debug('refpix,refval,increment=%s,%s,%s'%(refpix,refval,increment))
        for x in xrange(NH):
            x0 = (NH - x - 1) * STEP
            x1 = (NH - x - 2) * STEP + 1
            LabelRA[x][0] = refval + (x0 - refpix) * increment
            LabelRA[x][1] = refval + (x1 - refpix) * increment
        (refpix, refval, increment) = self.image.direction_axis(1, unit='deg')
        LOG.debug('refpix,refval,increment=%s,%s,%s'%(refpix,refval,increment))
        for y in xrange(NV):
            y0 = y * STEP
            y1 = (y + 1) * STEP - 1
            LabelDEC[y][0] = refval + (y0 - refpix) * increment
            LabelDEC[y][1] = refval + (y1 - refpix) * increment
        LOG.info('LabelDEC=%s'%(LabelDEC))
        LOG.info('LabelRA=%s'%(LabelRA))

        # loop over pol
        for pol in xrange(self.npol):
            masked_data_p = masked_data[:,:,:,pol]
            Plot = numpy.zeros((num_panel, num_panel, (chan1 - chan0)), numpy.float32) + NoData
            TotalSP = masked_data_p.sum(axis=0).sum(axis=0)
            mask_p = self.mask[:,:,:,pol]
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
            Mark = '-b'
            if ShowPlot: PL.ion()
            else: PL.ioff()
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
            PL.plot(self.frequency[chan0:chan1], TotalSP, color='b', linestyle='-', linewidth=0.4)
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
                        PL.plot(self.frequency[chan0:chan1], Plot[x][y], color='b', linestyle='-', linewidth=0.2)
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

            FigFileRoot = self.inputs.imagename+'.pol%s_Sparse'%(pol)
            plotfile = os.path.join(self.stage_dir, FigFileRoot+'_0.png')
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

