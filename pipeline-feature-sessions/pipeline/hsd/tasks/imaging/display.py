from __future__ import absolute_import

import os
import numpy
import pylab as pl
import time
import itertools
import math
from matplotlib.ticker import MultipleLocator

import pipeline.infrastructure as infrastructure
from pipeline.infrastructure.jobrequest import casa_tasks
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.displays.pointing as pointing
#from ..common.display import RADEClabel, RArotation, DECrotation
from ..common.display import DPIDetail, DPISummary, SDImageDisplay, SDImageDisplayInputs, ShowPlot
import pipeline.infrastructure.casatools as casatools
from ..common.display import sd_polmap as polmap
#from ..common.display import DDMMSSs, HHMMSSss
from ..common.display import SDSparseMapDisplay
from ..common.display import NoData, NoDataThreshold

# NoData = -32767.0
# NoDataThreshold = NoData + 10000.0

RADEClabel = pointing.RADEClabel
RArotation = pointing.RArotation
DECrotation = pointing.DECrotation
DDMMSSs = pointing.DDMMSSs
HHMMSSss = pointing.HHMMSSss

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
        
#         masked_data = self.data * self.mask
        for pol in xrange(self.npol):
#             Total = masked_data.take([pol], axis=self.id_stokes).squeeze()
            Total = (self.data.take([pol], axis=self.id_stokes) * self.mask.take([pol], axis=self.id_stokes)).squeeze()
            Total = numpy.flipud(Total.transpose())
            tmin = Total.min()
            tmax = Total.max()

            # 2008/9/20 DEC Effect
            im = pl.imshow(Total, interpolation='nearest', aspect=self.aspect, extent=Extent)
            #im = pl.imshow(Total, interpolation='nearest', aspect='equal', extent=Extent)
            del Total

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
#                         #tpmap_colorbar.ax.set_title('[K km/s]')
#                         tpmap_colorbar.ax.set_title('[%s]'%(self.image.brightnessunit))
#                         lab = tpmap_colorbar.ax.title
#                         lab.set_fontsize(newfontsize)
                        tpmap_colorbar.ax.set_ylabel('[%s]'%(self.image.brightnessunit), fontsize=newfontsize)
                    else:
                        tpmap_colorbar.set_clim((tmin,tmax))
                        tpmap_colorbar.draw_all()
                        
            # draw beam pattern
            if beam_circle is None:
                beam_circle = pointing.draw_beam(axes_tpmap, 0.5 * self.beam_size, self.aspect, self.ra_min, self.dec_min)

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
            parameters['pol'] = self.image.coordsys.stokes()[pol]#polmap[pol]
            parameters['ant'] = self.inputs.antenna
            #parameters['type'] = 'sd_channel-averaged'
            parameters['type'] = 'sd_integrated_map'
            parameters['file'] = self.inputs.imagename
            if self.inputs.vis is not None:
                parameters['vis'] = self.inputs.vis

            plot = logger.Plot(plotfile,
                               x_axis='R.A.',
                               y_axis='Dec.',
                               field=self.inputs.source,
                               parameters=parameters)
            plot_list.append(plot)

        return plot_list

class SDIntegratedImageDisplayInputs(SDImageDisplayInputs):
    def __init__(self, context, result):
        super(SDIntegratedImageDisplayInputs,self).__init__(context, result)
        # obtain integrated image using immoments task
        print self.imagename
        #job = casa_tasks.immoments(imagename=self.imagename, moments=[0], outfile=self.integrated_imagename)

    @property
    def integrated_imagename(self):
        return self.result.outcome['image'].imagename.rstrip('/') + '.integ'
    
class SDIntegratedImageDisplay(SDImageDisplay):
    MATPLOTLIB_FIGURE_ID = 8911

    def __init__(self, inputs):
        super(self.__class__, self).__init__(inputs)
        if hasattr(self.inputs, 'integrated_imagename'):
            self.imagename = self.inputs.integrated_imagename
        else:
            self.imagename = self.inputs.result.outcome['image'].imagename.rstrip('/') + '.integ'

    def init(self):
        if os.path.exists(self.imagename):
            os.system('rm -rf %s'%(self.imagename))
        job = casa_tasks.immoments(imagename=self.inputs.imagename, moments=[0], outfile=self.imagename)
        job.execute(dry_run=False)
        assert os.path.exists(self.imagename)
        super(self.__class__, self).init()
    
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
        
        for pol in xrange(self.npol):
            masked_data = (self.data.take([pol], axis=self.id_stokes) * self.mask.take([pol], axis=self.id_stokes)).squeeze()
            Total = numpy.flipud(masked_data.transpose())
            del masked_data

            # 2008/9/20 DEC Effect
            im = pl.imshow(Total, interpolation='nearest', aspect=self.aspect, extent=Extent)
            #im = pl.imshow(Total, interpolation='nearest', aspect='equal', extent=Extent)
            tmin = Total.min()
            tmax = Total.max()
            del Total

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
                        newfontsize = None
                        for t in tpmap_colorbar.ax.get_yticklabels():
                            newfontsize = t.get_fontsize()*0.5
                            t.set_fontsize(newfontsize)
                        #if newfontsize is None: # no ticks in colorbar likely invalid TP map
                        tpmap_colorbar.ax.set_ylabel('[%s]'%(self.brightnessunit), fontsize=newfontsize)
                    else:
                        tpmap_colorbar.set_clim((tmin,tmax))
                        tpmap_colorbar.draw_all()
                        
            # draw beam pattern
            if beam_circle is None:
                beam_circle = pointing.draw_beam(axes_tpmap, 0.5 * self.beam_size, self.aspect, self.ra_min, self.dec_min)

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
            parameters['pol'] = self.image.coordsys.stokes()[pol]#polmap[pol]
            parameters['ant'] = self.inputs.antenna
            parameters['type'] = 'sd_integrated_map'
            parameters['file'] = self.inputs.imagename

            plot = logger.Plot(plotfile,
                               x_axis='R.A.',
                               y_axis='Dec.',
                               field=self.inputs.source,
                               parameters=parameters)
            
            plot_list.append(plot)

        return plot_list
 


class ChannelMapAxesManager(ChannelAveragedAxesManager):
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
            left = 1.0 / float(self.nh) * x #(x + 0.05)
            width = 1.0 / float(self.nh) * 0.85 #0.9
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
    
    def plot(self):

        self.init()

        return self.__plot_channel_map()

    def __valid_lines(self):
        group_desc = self.inputs.reduction_group
        ant_index = self.inputs.antennaid_list
        spwid_list = self.inputs.spwid_list
        msid_list = self.inputs.msid_list
        fieldid_list = self.inputs.fieldid_list

        line_list = []
#         for group_desc in reduction_group.values():
        for g in group_desc:
            found = False
            for (msid, ant, fid, spw) in itertools.izip(msid_list, ant_index,
                                             fieldid_list, spwid_list):
                msobj_list = self.inputs.context.observing_run.measurement_sets
                msname_list = [os.path.abspath(msobj_list[idx].name) \
                               for idx in xrange(len(msobj_list))]
                group_msid = msname_list.index(os.path.abspath(g.ms.name))
                del msobj_list, msname_list
                if group_msid==msid and g.antenna_id == ant and \
                    g.field_id == fid and g.spw_id == spw:
                    found = True
                    break
            if found:
                for ll in g.channelmap_range:
                    if not ll in line_list and ll[2] is True:
                        line_list.append(ll)
        return line_list

    def __get_integrated_spectra(self):
        imagename = self.inputs.imagename
        weightname = self.inputs.imagename + '.weight'
        new_id_stokes = 0 if self.id_stokes < self.id_spectral else 1
        # un-weighted image
        unweight_ia = casatools.image.imagecalc(outfile='', pixels='"%s" * "%s"' % (imagename, weightname))
        
        # if all pixels are masked, return fully masked array
        unweight_mask = unweight_ia.getchunk(getmask=True)
        if numpy.all(unweight_mask == False):
            unweight_ia.close()
            sp_ave = numpy.ma.masked_array(numpy.zeros((self.npol, self.nchan), dtype=numpy.float32), 
                                           mask=numpy.ones((self.npol, self.nchan), dtype=numpy.bool))
            return sp_ave
        
        # average image spectra over map area taking mask into account
        try:
            collapsed_ia = unweight_ia.collapse(outfile='', function='mean', axes=self.image.id_direction)
        finally:
            unweight_ia.close()
        try:
            data_integ = collapsed_ia.getchunk(dropdeg=True)
            mask_integ = collapsed_ia.getchunk(dropdeg=True, getmask=True)
        finally:
            collapsed_ia.close()
        # set mask to weight image
        with casatools.ImageReader(imagename) as ia:
            maskname = ia.maskhandler('get')[0]
        with casatools.ImageReader(weightname) as ia:
            if maskname!='T': #'T' is no mask (usually an image from completely flagged MSes)
                ia.maskhandler('delete', [maskname])
                ia.maskhandler('copy', ['%s:%s' % (imagename, maskname), maskname])
                ia.maskhandler('set', maskname)
            # average weight over map area taking the mask into account
            collapsed_ia = ia.collapse(outfile='', function='mean', axes=self.image.id_direction)
        try:
            weight_integ = collapsed_ia.getchunk(dropdeg=True)
        finally:
            collapsed_ia.close()
        # devive averaged image by averaged weight
        data_weight_integ = numpy.ma.masked_array((data_integ / weight_integ), [ not val for val in mask_integ ], fill_value=0.0)
        sp_ave = numpy.ma.masked_array(numpy.zeros((self.npol, self.nchan),dtype=numpy.float32))
        if self.npol == 1:
            if len(data_weight_integ) == self.nchan:
                sp_ave[0,:] = data_weight_integ
        else:
            for pol in xrange(self.npol):
                curr_sp= data_weight_integ.take([pol], axis=new_id_stokes).squeeze()
                if len(curr_sp) == self.nchan:
                    sp_ave[pol,:] = curr_sp
        return sp_ave

    def __plot_channel_map(self):
        colormap = 'color'
        scale_max = False
        scale_min = False
        
        plot_list = []
        
        # nrow is number of grid points for image
#         nrow = self.nx * self.ny

        # retrieve line list from reduction group
        # key is antenna and spw id
        line_list = self.__valid_lines()

        # 2010/6/9 in the case of non-detection of the lines
        if len(line_list) == 0:
            return plot_list
            
        # Set data
        Map = numpy.zeros((self.NumChannelMap, (self.y_max - self.y_min + 1), (self.x_max - self.x_min + 1)), dtype=numpy.float32)
#         RMSMap = numpy.zeros(((self.y_max - self.y_min + 1), (self.x_max - self.x_min + 1)), dtype=numpy.float32)

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
                                             self.brightnessunit)
        axes_integmap = axes_manager.axes_integmap
        integmap_colorbar = None
        beam_circle = None
        axes_integsp1 = axes_manager.axes_integsp_full
        axes_integsp2 = axes_manager.axes_integsp_zoom
        axes_chmap = axes_manager.axes_chmap
        chmap_colorbar = [None for v in xrange(self.NvPanel)]
        
        Sp_integ = self.__get_integrated_spectra()
        # loop over detected lines
        ValidCluster = 0
        for line_window in line_list:
            # shift channel according to the edge parameter
            ChanC = int(line_window[0] + 0.5 - self.edge[0])
            if float(ChanC) == line_window[0] - self.edge[0]:
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
                
                masked_data = (self.data.take([pol], axis=self.id_stokes) * self.mask.take([pol], axis=self.id_stokes)).squeeze()
                
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
#                             integmap_colorbar.ax.set_title('[%s km/s]'%(self.brightnessunit))
#                             lab = integmap_colorbar.ax.title
#                             lab.set_fontsize(newfontsize)
                            integmap_colorbar.ax.set_ylabel('[%s km/s]'%(self.brightnessunit), fontsize=newfontsize)
                        else:
                            integmap_colorbar.set_clim((Total.min(),Total.max()))
                            integmap_colorbar.draw_all()

                # draw beam pattern
                if beam_circle is None:
                    beam_circle = pointing.draw_beam(axes_integmap, self.beam_radius, self.aspect, self.ra_min, self.dec_min)
                    
                pl.title('Total Intensity: CenterFreq.= %.3f GHz' % self.frequency[ChanC], size=TickSize)
                axes_integmap.set_xlim(xlim)
                axes_integmap.set_ylim(ylim)

                t1 = time.time()

                # Plot Integrated Spectrum #1
#                 Sp = numpy.sum(numpy.transpose((valid * numpy.transpose(flattened_data))),axis=0)/numpy.sum(valid,axis=0)
                #Sp = numpy.sum(flattened_data * valid.reshape((nrow,1)), axis=0)/valid.sum()
                Sp = Sp_integ[pol,:]
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
                # adjust Y-axis range to the current line
                spmin_zoom = Sp[chan0:chan1].min()
                spmax_zoom = Sp[chan0:chan1].max()
                dsp = spmax_zoom - spmin_zoom
                spmin_zoom -= dsp * 0.1
                spmax_zoom += dsp * 0.1
                pl.axis([V0, V1, spmin_zoom, spmax_zoom])

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
                del masked_data
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
#                                 cb.ax.set_title('[%s km/s]'%(self.brightnessunit))
#                                 lab=cb.ax.title
#                                 lab.set_fontsize(newfontsize)
                                cb.ax.set_ylabel('[%s km/s]'%(self.brightnessunit), fontsize=newfontsize)
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
                parameters['pol'] = self.image.coordsys.stokes()[pol]#polmap[pol]
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
        

class RmsMapAxesManager(ChannelAveragedAxesManager):
    @property
    def axes_rmsmap(self):
        return self.axes_tpmap


class SDRmsMapDisplay(SDImageDisplay):
    #MATPLOTLIB_FIGURE_ID = 8910

    def plot(self):
        self.init()

        t1 = time.time()
        plot_list = self.__plot_channel_map()
        t2 = time.time()
        LOG.debug('__plot_channel_map: elapsed time %s sec'%(t2-t1))

        return plot_list

    def __get_rms(self):
        # reshape rms to a 3d array in shape, (nx_im, ny_im, npol_data)
        return self.__reshape_grid_table_values(self.inputs.result.outcome['rms'], float)

    def __get_num_valid(self):
        # reshape validsp to a 3d array in shape, (nx_im, ny_im, npol_data)
        return self.__reshape_grid_table_values(self.inputs.result.outcome['validsp'], int)

    def __reshape_grid_table_values(self, array2d, dtype=None):
        # reshape 2d array in shape, (npol, nx*ny), to (nx, ny, npol)
        npol_data = len(array2d)
        # retruned value will be transposed
        array3d = numpy.zeros((npol_data, self.ny, self.nx),dtype=dtype)
        for pol in xrange(npol_data):
            if len(array2d[pol]) == self.nx*self.ny:
                array3d[pol,:,:] = numpy.array(array2d[pol]).reshape((self.ny,self.nx))
        return numpy.flipud(array3d.transpose())
        

    
    def __plot_channel_map(self):
        pl.clf()

        colormap = 'color'
        plot_list = []

        # 2008/9/20 Dec Effect has been taken into account
        #Aspect = 1.0 / math.cos(0.5 * (self.dec_min + self.dec_max) / 180.0 * 3.141592653)

        # Draw RMS Map
        TickSize = 6

        Extent = (self.ra_max+self.grid_size/2.0, self.ra_min-self.grid_size/2.0, self.dec_min-self.grid_size/2.0, self.dec_max+self.grid_size/2.0)
        span = max(self.ra_max - self.ra_min + self.grid_size, self.dec_max - self.dec_min + self.grid_size)
        (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

        axes_manager = RmsMapAxesManager(RAformatter, DECformatter,
                                         RAlocator, DEClocator,
                                         RArotation, DECrotation,
                                         TickSize, colormap)
        rms_axes = axes_manager.axes_rmsmap
        rms_colorbar = None
        beam_circle = None

        rms = self.__get_rms()
        nvalid = self.__get_num_valid()
        npol_data = rms.shape[2]
#         for pol in xrange(self.npol):
        for pol in xrange(npol_data):
            rms_map = rms[:,:,pol] * (nvalid[:,:,pol] > 0)
            rms_map = numpy.flipud(rms_map.transpose())
            #LOG.debug('rms_map=%s'%(rms_map))
            # 2008/9/20 DEC Effect
            image = pl.imshow(rms_map, interpolation='nearest', aspect=self.aspect, extent=Extent)
            xlim = rms_axes.get_xlim()
            ylim = rms_axes.get_ylim()
            
            # colorbar
            rmsmin = rms_map.min()
            rmsmax = rms_map.max()
            if not (rmsmin == rmsmax): 
                if not ((self.y_max == self.y_min) and (self.x_max == self.x_min)):
                    if rms_colorbar is None:
                        rms_colorbar = pl.colorbar(shrink=0.8)
                        for t in rms_colorbar.ax.get_yticklabels():
                            newfontsize = t.get_fontsize()*0.5
                            t.set_fontsize(newfontsize)
#                         rms_colorbar.ax.set_title('[%s]' % self.brightnessunit)
#                         lab = rms_colorbar.ax.title
                        rms_colorbar.ax.set_ylabel('[%s]' % self.brightnessunit)
                    else:
                        rms_colorbar.set_clim((rmsmin,rmsmax))
                        rms_colorbar.draw_all()
            del rms_map

            # draw beam pattern
            if beam_circle is None:
                beam_circle = pointing.draw_beam(rms_axes, self.beam_radius, self.aspect, self.ra_min, self.dec_min)

            rms_axes.set_xlim(xlim)
            rms_axes.set_ylim(ylim)

            if ShowPlot: pl.draw()
            FigFileRoot = self.inputs.imagename + '.pol%s'%(pol)
            plotfile = os.path.join(self.stage_dir, FigFileRoot+'_rmsmap.png')
            pl.savefig(plotfile, format='png', dpi=DPISummary)

            image.remove()

            parameters = {}
            parameters['intent'] = 'TARGET'
            parameters['spw'] = self.spw
            parameters['pol'] = polmap[pol]
            parameters['ant'] = self.antenna
            parameters['file'] = self.inputs.imagename
            parameters['type'] = 'rms_map'

            plot2 = logger.Plot(plotfile,
                                x_axis='R.A.',
                                y_axis='Dec.',
                                field=self.inputs.source,
                                parameters=parameters)
            plot_list.append(plot2)

        return plot_list
        

class SpectralMapAxesManager(object):
    def __init__(self, nh, nv, brightnessunit, formatter, locator, ticksize):
        self.nh = nh
        self.nv = nv
        self.brightnessunit = brightnessunit
        self.formatter = formatter
        self.locator = locator
        self.ticksize = ticksize

        self._axes = None
        
    @property
    def axes_list(self):
        if self._axes is None:
            self._axes = list(self.__axes_list())

        return self._axes

    def __axes_list(self):
        npanel = self.nh * self.nv
        for ipanel in xrange(npanel):
            x = ipanel % self.nh
            y = int(ipanel / self.nh)
            #x0 = 1.0 / float(self.nh) * (x + 0.1)
            x0 = 1.0 / float(self.nh) * (x + 0.22)
            #x1 = 1.0 / float(self.nh) * 0.8
            x1 = 1.0 / float(self.nh) * 0.75
            y0 = 1.0 / float(self.nv) * (y + 0.15)
            #y1 = 1.0 / float(self.nv) * 0.7
            y1 = 1.0 / float(self.nv) * 0.65
            a = pl.axes([x0, y0, x1, y1])
            a.xaxis.set_major_formatter(self.formatter)
            a.xaxis.set_major_locator(self.locator)
            a.yaxis.set_label_coords(-0.22,0.5)
            a.title.set_y(0.95)
            a.title.set_size(self.ticksize)
            pl.ylabel('Intensity (%s)'%(self.brightnessunit), size=self.ticksize)
            pl.xticks(size=self.ticksize)
            pl.yticks(size=self.ticksize)

            yield a

        
class SDSpectralMapDisplay(SDImageDisplay):
    #MATPLOTLIB_FIGURE_ID = 8910
    MaxNhPanel = 5
    MaxNvPanel = 5
    
    def plot(self):
        self.init()
        return self.__plot_spectral_map()

    def __get_strides(self):
        qa = casatools.quanta
        increment = self.image.coordsys.increment()
        units = self.image.coordsys.units()
        factors = []
        for idx in self.image.id_direction:
            cell = qa.convert(qa.quantity(increment['numeric'][idx],units[idx]),'deg')['value']
            factors.append( int( numpy.round( abs(self.grid_size / cell) ) ) )
        return factors

    def __plot_spectral_map(self):
        pl.clf()

        (STEPX, STEPY) = self.__get_strides()

        # Raster Case: re-arrange spectra to match RA-DEC orientation
        mode = 'raster'
        if mode.upper() == 'RASTER':
            # the number of panels in each page
            NhPanel = min( max((self.x_max - self.x_min + 1)/STEPX, (self.y_max - self.y_min + 1)/STEPY), self.MaxNhPanel )
            NvPanel = min( max((self.x_max - self.x_min + 1)/STEPX, (self.y_max - self.y_min + 1)/STEPY), self.MaxNvPanel )
            # total number of pages in horizontal and vertical directions
            NH = int((self.x_max - self.x_min) / STEPX / NhPanel + 1)
            NV = int((self.y_max - self.y_min) / STEPY / NvPanel + 1)
            # an array with length of total number of spectra to be plotted (initialized by -1)
            ROWS = numpy.zeros(NH * NV * NhPanel * NvPanel, dtype=numpy.int) - 1
            # 2010/6/15 GK Change the plotting direction: UpperLeft->UpperRight->OneLineDown repeat...
            for x in xrange(0,self.nx,STEPX):
                posx = (self.x_max - x)/STEPX / NhPanel
                offsetx = ( (self.x_max - x)/STEPX ) % NhPanel
                for y in xrange(0,self.ny,STEPY):
                    posy = (self.y_max - y)/STEPY / NvPanel
                    offsety = NvPanel - 1 - (self.y_max - y)/STEPY % NvPanel
                    row = (self.nx - x - 1) * self.ny + y
                    ROWS[(posy*NH+posx)*NvPanel*NhPanel + offsety*NhPanel + offsetx] = row
        else: ### This block is currently broken (2016/06/23 KS)
            #ROWS = rows[:]
            #NROW = len(rows)
            #Npanel = (NROW - 1) / (self.MaxNhPanel * self.MaxNvPanel) + 1
            #if Npanel > 1:  (NhPanel, NvPanel) = (self.MaxNhPanel, self.MaxNvPanel)
            #else: (NhPanel, NvPanel) = (int((NROW - 0.1) ** 0.5) + 1, int((NROW - 0.1) ** 0.5) + 1)
            raise Exception, "non-Raster map is not supported yet."

        LOG.debug("Generating spectral map")
        LOG.debug("- Stride: [%d, %d]" % (STEPX, STEPY))
        LOG.debug("- Number of panels: [%d, %d]" % (NhPanel, NvPanel))
        LOG.debug("- Number of pages: [%d, %d]" % (NH, NV))
        LOG.debug("- Number of spcetra to be plotted: %d" % (len(ROWS)))
        
        Npanel = 0
        TickSize = 11 - NhPanel

        # Plotting routine
        connect = True
        if connect is True: Mark = '-b'
        else: Mark = 'bo'
        chan0 = 0
        chan1 = -1
        if chan1 == -1:
            chan0 = 0
            chan1 = self.nchan - 1
        xmin = min(self.frequency[chan0], self.frequency[chan1])
        xmax = max(self.frequency[chan0], self.frequency[chan1])


        NSp = 0
        xtick = abs(self.frequency[-1] - self.frequency[0]) / 4.0
        Order = int(math.floor(math.log10(xtick)))
        NewTick = int(xtick / (10**Order) + 1) * (10**Order)
        FreqLocator = MultipleLocator(NewTick)
        if Order < 0: FMT = '%%.%dfG' % (-Order)
        else: FMT = '%.2fG'
        Format = pl.FormatStrFormatter(FMT)


        (xrp,xrv,xic) = self.image.direction_axis(0)
        (yrp,yrv,yic) = self.image.direction_axis(1)

        plot_list = []

        axes_manager = SpectralMapAxesManager(NhPanel, NvPanel, self.brightnessunit,
                                              Format, FreqLocator,
                                              TickSize)
        axes_list = axes_manager.axes_list
        plot_objects = []

        # MS-based procedure
        reference_data = self.context.observing_run.measurement_sets[self.inputs.msid_list[0]]
        is_baselined = reference_data.work_data != reference_data.name
        
        for pol in xrange(self.npol):
            data = (self.data.take([pol], axis=self.id_stokes) * self.mask.take([pol], axis=self.id_stokes)).squeeze()
            Npanel = 0

            # to eliminate max/min value due to bad pixel or bad fitting,
            #  1/10-th value from max and min are used instead
#             valid_index = numpy.where(self.num_valid_spectrum[:,:,pol] > 0)
            mask2d = numpy.any(self.mask.take([pol], axis=self.id_stokes).squeeze(), axis=2)
            valid_index = mask2d.nonzero()
            valid_data = data[valid_index[0],valid_index[1],chan0:chan1]
            ListMax = valid_data.max(axis=1)
            ListMin = valid_data.min(axis=1)
            del valid_index, valid_data
            if len(ListMax) == 0: continue 
            if is_baselined:
                ymax = numpy.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
                ymin = numpy.sort(ListMin)[len(ListMin)/10]
            else:
                ymax = numpy.sort(ListMax)[-1]
                ymin = numpy.sort(ListMin)[1]
            ymax = ymax + (ymax - ymin) * 0.2
            ymin = ymin - (ymax - ymin) * 0.1
            LOG.debug('ymin=%s, ymax=%s'%(ymin,ymax))
            del ListMax, ListMin

            for irow in xrange(len(ROWS)):
                row = ROWS[irow]
                    
                _x = row / self.ny
                _y = row % self.ny
                
                prefix = self.inputs.imagename+'.pol%s_Result'%(pol)
                plotfile = os.path.join(self.stage_dir, prefix+'_%s.png'%(Npanel))

                if not os.path.exists(plotfile):
                    if 0 <= _x < self.nx and 0 <= _y < self.ny:
                        a = axes_list[NSp]
                        a.set_axis_on()
                        pl.gcf().sca(a)
                        world_x = xrv + (_x - xrp) * xic
                        world_y = yrv + (_y - yrp) * yic
                        title = '(IF, POL, X, Y) = (%s, %s, %s, %s)\n%s %s' % (self.spw, pol, _x, _y, HHMMSSss(world_x, 0), DDMMSSs(world_y, 0))
#                         if self.num_valid_spectrum[_x][_y][pol] > 0:
                        if mask2d[_x][_y]:
                            plot_objects.extend(
                                pl.plot(self.frequency, data[_x][_y], Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
                                )
                        else:
                            plot_objects.append(
                                pl.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'NO DATA', horizontalalignment='center', verticalalignment='center', size=TickSize)
                                )
                        a.title.set_text(title)
                        pl.axis([xmin, xmax, ymin, ymax])
                    else:
                        a = axes_list[NSp]
                        a.set_axis_off()
                        a.title.set_text('')
                        
                NSp += 1
                if NSp >= (NhPanel * NvPanel) or (irow == len(ROWS)-1 and mode.upper() != 'RASTER'):
                    NSp = 0
                    if ShowPlot:
                        pl.draw()

                    prefix = self.inputs.imagename+'.pol%s_Result'%(pol)
                    plotfile = os.path.join(self.stage_dir, prefix+'_%s.png'%(Npanel))
                    if not os.path.exists(plotfile):
                        LOG.debug('Regenerate plot: %s'%(plotfile))
                        pl.savefig(plotfile, format='png', dpi=DPIDetail)
                    else:
                        LOG.debug('Use existing plot: %s'%(plotfile))

                    for obj in plot_objects:
                        obj.remove()
                        del obj
                    plot_objects = []

                    parameters = {}
                    parameters['intent'] = 'TARGET'
                    parameters['spw'] = self.inputs.spw
                    parameters['pol'] = self.image.coordsys.stokes()[pol]#polmap[pol]
                    parameters['ant'] = self.inputs.antenna
                    parameters['type'] = 'sd_spectral_map'
                    parameters['file'] = self.inputs.imagename

                    plot = logger.Plot(plotfile,
                                       x_axis='Frequency',
                                       y_axis='Intensity',
                                       field=self.inputs.source,
                                       parameters=parameters)
                    plot_list.append(plot)
            

                    Npanel += 1
            del data, mask2d
        del ROWS
        print("Returning %d plots from spectralmap" % len(plot_list))
        return plot_list

        
class SDSpectralImageDisplay(SDImageDisplay):
    MATPLOTLIB_FIGURE_ID = 8910

    def plot(self):
        if ShowPlot: pl.ion()
        else: pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        if ShowPlot: pl.ioff()
        
#         self.init()
        
        plot_list = []
        t0 = time.time()
        worker = SDSparseMapDisplay(self.inputs)
        worker.enable_atm()
        plot_list.extend(worker.plot())
        t1 = time.time()
        worker = SDChannelMapDisplay(self.inputs)
        plot_list.extend(worker.plot())
        t2 = time.time()
        worker = SDSpectralMapDisplay(self.inputs)
        plot_list.extend(worker.plot())
        t3 = time.time()
        worker = SDRmsMapDisplay(self.inputs)
        plot_list.extend(worker.plot())
        t4 = time.time()
        worker = SDIntegratedImageDisplay(self.inputs)
        plot_list.extend(worker.plot())
        t5 = time.time()
        LOG.debug('sparse_map: elapsed time %s sec'%(t1-t0))
        LOG.debug('channel_map: elapsed time %s sec'%(t2-t1))
        LOG.debug('spectral_map: elapsed time %s sec'%(t3-t2))
        LOG.debug('rms_map: elapsed time %s sec'%(t4-t3))
        LOG.debug('integrated_map: elapsed time %s sec'%(t5-t4))
        return plot_list


def SDImageDisplayFactory(mode):
    LOG.debug('MODE=%s'%(mode))
    if mode == 'TP':
        return SDChannelAveragedImageDisplay

    else:
        # mode should be 'SP'
        return SDSpectralImageDisplay

