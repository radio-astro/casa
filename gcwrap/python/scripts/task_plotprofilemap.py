import os
import numpy
import pylab as pl

from taskinit import casalog, gentools, qa

def plotprofilemap(imagename=None, figfile=None, overwrite=None, 
                   title=None,
                   linecolor=None, linestyle=None, linewidth=None,
                   separatepanel=None, plotmasked=None, maskedcolor=None, 
                   showaxislabel=None, showtick=None, showticklabel=None,
                   horizontalbind=None, verticalbind=None, spectralrange=None, trasnparent=None):
    casalog.origin('plotprofilemap')
    
    try:
        if len(figfile) > 0 and os.path.exists(figfile) and overwrite == False:
            raise RuntimeError('overwrite is False and output file exists: \'%s\''%(figfile))
    
        image = SpectralImage(imagename)
        plot_profile_map(image, figfile, title, linecolor, linestyle, linewidth,
                         separatepanel, plotmasked, maskedcolor,
                         showaxislabel, showtick, showticklabel)
    except Exception, e:
        casalog.post('Error: %s'%(str(e)), priority='SEVERE')
        import traceback
        casalog.post(traceback.format_exc(), priority='DEBUG')
        raise e
    finally:
        pass
    
NoData = -32767.0
NoDataThreshold = NoData + 10000.0
LightSpeedQuantity = qa.constants('c')
LightSpeed = qa.convert(LightSpeedQuantity, 'km/s')['value'] # speed of light in km/s
DPIDetail = 130

dsyb = '$^\circ$'
hsyb = ':'
msyb = ':'

def Deg2HMS(x, arrowance):
    # Transform degree to HHMMSS.sss format
    xx = x % 360 + arrowance
    h = int(xx / 15)
    m = int((xx % 15) * 4)
    s = ((xx % 15) * 4 - m) * 60.0
    return (h, m, s)

def HHMMSSss(x, pos):
    # HHMMSS.ss format
    (h, m, s) = Deg2HMS(x, 1/240000.0)
    #return '%02dh%02dm%05.2fs' % (h, m, s)
    return '%02d%s%02d%s%05.2f' % (h, hsyb, m, msyb, s)

def Deg2DMS(x, arrowance):
    # Transform degree to +ddmmss.ss format
    xxx = (x + 90) % 180 - 90
    xx = abs(xxx) + arrowance
    if xxx < 0: sign = -1
    else: sign = 1
    d = int(xx * sign)
    m = int((xx % 1) * 60)
    s = ((xx % 1) * 60 - m) * 60.0
    return (d, m, s)

def DDMMSSs(x, pos):
    # +DDMMSS.s format
    (d, m, s) = Deg2DMS(x, 1/360000.0)
    #return '%+02dd%02dm%04.1fs' % (d, m, s)
    sint = int(s)
    sstr = ('%3.1f'%(s-int(s))).lstrip('0')
    return '%+02d%s%02d\'%02d\"%s' % (d, dsyb, m, sint, sstr)


class ProfileMapAxesManager(object):
    label_map = {'Right Ascension': 'RA',
                 'Declination': 'Dec'}
    def __init__(self, nh, nv, brightnessunit, direction_label, direction_reference, 
                 spectral_label, spectral_unit, ticksize, title='', separatepanel=True, 
                 showaxislabel=False, showtick=False, showticklabel=False,
                 
                 clearpanel=True):
        self.nh = nh
        self.nv = nv
        self.ticksize = ticksize
        self.brightnessunit = brightnessunit
        self.numeric_formatter = pl.FormatStrFormatter('%.2f')
        self.direction_label = direction_label
        self.direction_reference = direction_reference
        self.separatepanel = separatepanel
        self.spectral_label = spectral_label
        self.spectral_unit = spectral_unit
        self.showaxislabel = showaxislabel
        self.showtick = showtick
        self.showticklabel = showticklabel
        self.title = title
        
        self._axes_spmap = None
        
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        if clearpanel:
            pl.clf()
            
    @property
    def MATPLOTLIB_FIGURE_ID(self):
        return 8910

    @property
    def axes_spmap(self):
        if self._axes_spmap is None:
            self._axes_spmap = list(self.__axes_spmap())

        return self._axes_spmap
    
    @property
    def nrow(self):
        return self.nv
    
    @property
    def ncolumn(self):
        return self.nh + 1

    @property
    def left_margin(self):
        return 0.01 + 0.2 / self.ncolumn

    @property
    def right_margin(self):
        return 0.01

    @property
    def bottom_margin(self):
        return 0.01 + 0.5 / self.nrow

    @property
    def top_margin(self):
        return 0.01
        
    @property
    def horizontal_space(self):
        if self.separatepanel:
            return self.horizontal_subplot_size * 0.1
        else:
            return 0.
    
    @property
    def vertical_space(self):
        if self.separatepanel:
            return self.vertical_subplot_size * 0.1
        else:
            return 0.
        
    @property
    def xlabel_area(self):
        if self.showaxislabel or (self.showtick and self.showticklabel):
            return 0.02
        else:
            return 0.
    
    @property
    def ylabel_area(self):
        if self.showaxislabel or (self.showtick and self.showticklabel):
            return 0.02
        else:
            return 0.
        
    @property
    def title_area(self):
        if isinstance(self.title, str) and len(self.title) > 0:
            return 0.04 * (self.title.count('\n') + 1)
        else:
            return 0.

    @property
    def horizontal_subplot_size(self):
        return (1.0 - self.left_margin - self.right_margin - self.ylabel_area) / self.ncolumn 

    @property
    def vertical_subplot_size(self):
        return (1.0 - self.bottom_margin - self.top_margin - self.xlabel_area - self.title_area) / self.nrow 

    def __axes_spmap(self):
        for x in xrange(self.nh):
            for y in xrange(self.nv):
                w = self.horizontal_subplot_size
                h = self.vertical_subplot_size
                l = 1.0 - self.right_margin - w * (x + 1) + 0.5 * self.horizontal_space
                b = self.bottom_margin + self.ylabel_area + h * y + 0.5 * self.vertical_space
                axes = pl.axes([l, b, w - self.horizontal_space, h - self.vertical_space])
                axes.cla()
                if self.showaxislabel and y == 0 and x == self.nh - 1:
                    axes.xaxis.set_label_text('%s [%s]'%(self.spectral_label,self.spectral_unit),
                                              size=self.ticksize)
                    axes.yaxis.set_label_text('Intensity [%s]'%(self.brightnessunit), 
                                              size=self.ticksize, rotation='vertical')
                if self.showtick:
                    axes.xaxis.tick_bottom()
                    axes.yaxis.tick_left()
                    if self.showticklabel: 
                        locator = pl.LinearLocator(numticks=4)
                        axes.xaxis.set_major_locator(locator)
                        axes.yaxis.set_major_locator(locator)
                        axes.xaxis.set_tick_params(labelsize=max(self.ticksize-2,1))
                        axes.yaxis.set_tick_params(labelsize=max(self.ticksize-2,1))
                        if y != 0 or x != self.nh - 1:
                            axes.xaxis.set_major_formatter(pl.NullFormatter())
                            axes.yaxis.set_major_formatter(pl.NullFormatter())
                    else:
                        axes.xaxis.set_major_formatter(pl.NullFormatter())
                        axes.yaxis.set_major_formatter(pl.NullFormatter())
                    
                else:
                    axes.yaxis.set_major_locator(pl.NullLocator())
                    axes.xaxis.set_major_locator(pl.NullLocator())

                yield axes
        

    def setup_labels(self, label_ra, label_dec):
        for x in xrange(self.nh):
            w = self.horizontal_subplot_size
            l = 1.0 - self.right_margin - w * (x + 1)
            h = self.bottom_margin * 0.5
            b = self.bottom_margin - h
            a1 = pl.axes([l, b, w, h])
            a1.set_axis_off()
            if len(a1.texts) == 0:
                pl.text(0.5, 0.2, HHMMSSss((label_ra[x][0]+label_ra[x][1])/2.0, 0), 
                        horizontalalignment='center', verticalalignment='center', size=self.ticksize)
            else:
                a1.texts[0].set_text(HHMMSSss((label_ra[x][0]+label_ra[x][1])/2.0, 0))
        for y in xrange(self.nv):
            l = self.left_margin
            w = self.horizontal_subplot_size
            h = self.vertical_subplot_size
            b = self.bottom_margin + y * h
            a1 = pl.axes([l, b, w, h])
            a1.set_axis_off()
            if len(a1.texts) == 0:
                pl.text(0.5, 0.5, DDMMSSs((label_dec[y][0]+label_dec[y][1])/2.0, 0), 
                        horizontalalignment='center', verticalalignment='center', size=self.ticksize)
            else:
                a1.texts[0].set_text(DDMMSSs((label_dec[y][0]+label_dec[y][1])/2.0, 0))
                
        # longitude label
        l = self.left_margin + self.xlabel_area
        h = self.bottom_margin * 0.5 
        b = 0.
        w = 1.0 - l - self.right_margin
        a1 = pl.axes([l, b, w, h])
        a1.set_axis_off()
        xpos = (1.0 + 0.5 * self.nh) / self.ncolumn
        casalog.post('xpos=%s'%(xpos), priority='DEBUG')
        pl.text(xpos, 0.5, '%s (%s)'%(self.direction_label[0],self.direction_reference),
                horizontalalignment='center', verticalalignment='center',
                size=(self.ticksize+2))

        # latitude label
        l = 0.0
        w = self.left_margin
        h = self.vertical_subplot_size
        b = self.bottom_margin + 0.5 * (h * self.nrow - self.vertical_subplot_size)
        a1 = pl.axes([l, b, w, h])
        a1.set_axis_off()
        pl.text(1.0, 0.5, '%s (%s)'%(self.direction_label[1],self.direction_reference),
                horizontalalignment='right', verticalalignment='center', 
                rotation='vertical', size=(self.ticksize+2))
        
        # title
        if self.title_area > 0.:
            left = self.left_margin + self.xlabel_area
            bottom = 1.0 - self.title_area - self.top_margin
            width = 1.0 - left - self.right_margin
            height = self.title_area
            a1 = pl.axes([left, bottom, width, height])
            a1.set_axis_off()
            xpos = (1.0 + 0.5 * self.nh) / self.ncolumn
            pl.text(xpos, 0.1, self.title, 
                    horizontalalignment='center', verticalalignment='bottom',
                    size=self.ticksize+4)

def plot_profile_map(image, figfile, title=None, linecolor=None, linestyle=None, linewidth=None,
                     separatepanel=None, plotmasked=None, maskedcolor=None,
                     showaxislabel=None, showtick=None, showticklabel=None):
    """
    image 
    figfile
    linecolor
    linestyle
    linewidth
    separatepanel
    plotmasked -- 'none': show neither lines nor panels
                  'empty': show empty panel
                  'text': show 'NO DATA' symbol
                  'zero': plot zero level
                  'plot': plot masked data with different color
    """
    if linecolor is None:
        linecolor = 'b'
    if linestyle is None:
        linestyle = '-'
    if linewidth is None:
        linewidth = 0.2
    if separatepanel is None:
        separatepanel = True
    if plotmasked is None:
        plotmasked = 'none'
    if maskedcolor is None:
        maskedcolor = 'gray' if linecolor != 'gray' else 'black'
    if showaxislabel is None:
        showaxislabel = False
    if showtick is None:
        showtick = False
    if showticklabel is None:
        showticklabel = False

    x_max = image.nx - 1
    x_min = 0
    y_max = image.ny - 1
    y_min = 0
    MaxPanel = 8
    num_panel = min(max(x_max - x_min + 1, y_max - y_min + 1), MaxPanel)
    STEP = int((max(x_max - x_min + 1, y_max - y_min + 1) - 1) / num_panel) + 1
    NH = (x_max - x_min) / STEP + 1
    NV = (y_max - y_min) / STEP + 1

    casalog.post('num_panel=%s, STEP=%s, NH=%s, NV=%s'%(num_panel,STEP,NH,NV))

    chan0 = 0
    chan1 = image.nchan
    
    direction_label = image.direction_label
    direction_reference = image.direction_reference
    spectral_label = image.spectral_label
    spectral_unit = image.spectral_unit
    plotter = SDProfileMapPlotter(NH, NV, STEP, image.brightnessunit, 
                                  direction_label, direction_reference,
                                  spectral_label, spectral_unit,
                                  title=title,
                                  separatepanel=separatepanel, 
                                  showaxislabel=showaxislabel,
                                  showtick=showtick,
                                  showticklabel=showticklabel,
                                  clearpanel=True)

    masked_data = image.data * image.mask

    plot_list = []

    refpix = [0,0]
    refval = [0,0]
    increment = [0,0]
    refpix[0], refval[0], increment[0] = image.direction_axis(0, unit='deg')
    refpix[1], refval[1], increment[1] = image.direction_axis(1, unit='deg')
    plotter.setup_labels(refpix, refval, increment)
    
    
    # loop over pol
    npol = 1
    for pol in xrange(npol):
        
        masked_data_p = masked_data.take([pol], axis=image.id_stokes).squeeze()
        Plot = numpy.zeros((num_panel, num_panel, (chan1 - chan0)), numpy.float32) + NoData
        mask_p = image.mask.take([pol], axis=image.id_stokes).squeeze()
        isvalid = numpy.any(mask_p, axis=2)
        Nsp = sum(isvalid.flatten())
        casalog.post('Nsp=%s'%(Nsp))

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

        status = plotter.plot(figfile, Plot, 
                              image.frequency[chan0:chan1], 
                              linecolor=linecolor,
                              linestyle=linestyle,
                              linewidth=linewidth,
                              plotmasked=plotmasked,
                              maskedcolor=maskedcolor)
        
    plotter.done()
    
class SDProfileMapPlotter(object):
    def __init__(self, nh, nv, step, brightnessunit, direction_label, direction_reference, 
                 spectral_label, spectral_unit, title=None, separatepanel=True, 
                 showaxislabel=False, showtick=False, showticklabel=False,
                 clearpanel=True):
        self.step = step
        if step > 1:
            ticksize = 10 - int(max(nh, nv) * step / (step - 1)) / 2
        elif step == 1:
            ticksize = 10 - int(max(nh, nv)) / 2
        self.axes = ProfileMapAxesManager(nh, nv, brightnessunit, 
                                          direction_label, direction_reference,
                                          spectral_label, spectral_unit,
                                          ticksize, title=title, 
                                          separatepanel=separatepanel, 
                                          showaxislabel=showaxislabel,
                                          showtick=showtick,
                                          showticklabel=showticklabel,
                                          clearpanel=clearpanel)
        self.lines_averaged = None
        self.lines_map = None
        self.reference_level = None
        self.global_scaling = True
        self.deviation_mask = None
        
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
        #casalog.post('axis 0: refpix,refval,increment=%s,%s,%s'%(refpix,refval,increment))
        for x in xrange(self.nh):
            x0 = (self.nh - x - 1) * self.step
            x1 = (self.nh - x - 2) * self.step + 1
            LabelRA[x][0] = refval + (x0 - refpix) * increment
            LabelRA[x][1] = refval + (x1 - refpix) * increment
        refpix = refpix_list[1]
        refval = refval_list[1]
        increment = increment_list[1]
        #casalog.post('axis 1: refpix,refval,increment=%s,%s,%s'%(refpix,refval,increment))
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
        
    def plot(self, figfile, map_data, frequency, 
             linecolor='b', linestyle='-', linewidth=0.2,
             plotmasked='none', maskedcolor='gray'):        
        global_xmin = min(frequency[0], frequency[-1])
        global_xmax = max(frequency[0], frequency[-1])
        casalog.post('global_xmin=%s, global_xmax=%s'%(global_xmin,global_xmax))

        # Auto scaling
        # to eliminate max/min value due to bad pixel or bad fitting,
        #  1/10-th value from max and min are used instead
        valid_index = numpy.where(map_data.min(axis=2) > NoDataThreshold)
        valid_data = map_data[valid_index[0],valid_index[1],:]
        ListMax = valid_data.max(axis=1)
        ListMin = valid_data.min(axis=1)
        casalog.post('ListMax=%s'%(list(ListMax)))
        casalog.post('ListMin=%s'%(list(ListMin)))
        if len(ListMax) == 0: 
            return False
        global_ymax = numpy.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
        global_ymin = numpy.sort(ListMin)[len(ListMin)/10]
        global_ymax = global_ymax + (global_ymax - global_ymin) * 0.2
        global_ymin = global_ymin - (global_ymax - global_ymin) * 0.1
        del ListMax, ListMin

        casalog.post('global_ymin=%s, global_ymax=%s'%(global_ymin,global_ymax))

        pl.ioff()
        

        no_data = numpy.zeros(len(frequency), dtype=numpy.float32)
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
                    casalog.post('Per panel scaling turned on: ymin=%s, ymax=%s (global ymin=%s, ymax=%s)'%(ymin,ymax,global_ymin,global_ymax))
                pl.gcf().sca(self.axes.axes_spmap[y+(self.nh-x-1)*self.nv])
                if map_data[x][y].min() > NoDataThreshold:
                    pl.plot(frequency, map_data[x][y], color=linecolor, linestyle=linestyle, 
                            linewidth=linewidth)
                else:
                    if plotmasked == 'empty':
                        pass
                    elif plotmasked == 'text':
                        pl.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'NO DATA', ha='center', va='center', 
                                size=(self.TickSize + 1))
                    elif plotmasked == 'zero':
                        pl.plot(frequency, no_data, 
                                color=maskedcolor, linestyle=linestyle, linewidth=linewidth)
                    elif plotmasked == 'none':
                        a = pl.gcf().gca()
                        if self.axes.xlabel_area > 0. and y == 0 and x == 0:
                            pass
                        else:
                            a.set_axis_off()
                    elif plotmasked == 'plot':
                        m = map_data[x][y] == NoDataThreshold
                        if not all(m):
                            ma = pl.ma.masked_array(map_data[x][y], m)
                            pl.plot(frequency, ma, 
                                    color=maskedcolor, linestyle=linestyle, linewidth=linewidth)
                        
                pl.axis((xmin, xmax, ymin, ymax))

        pl.ion()
        pl.draw()

        casalog.post('figfile=\'%s\''%(figfile), priority='DEBUG')
        if figfile is not None and len(figfile) > 0:
            casalog.post('Output profile map to %s'%(figfile))
            pl.savefig(figfile, format='png', dpi=DPIDetail)
        
        return True
    
    def done(self):
        #pl.close()
        del self.axes

class SpectralImage(object):
    def __init__(self, imagename):
        self.imagename = imagename
        # read data to storage
        ia = gentools(['ia'])[0]
        ia.open(self.imagename)
        try:
            self.image_shape = ia.shape()
            self.coordsys = ia.coordsys()
            coord_types = self.coordsys.axiscoordinatetypes()
            self.units = self.coordsys.units()
            self.names = self.coordsys.names()
            self.direction_reference = self.coordsys.referencecode('dir')[0]
            self.spectral_reference = self.coordsys.referencecode('spectral')[0]
            self.id_direction = coord_types.index('Direction')
            self.id_direction = [self.id_direction, self.id_direction+1]
            self.id_spectral = coord_types.index('Spectral')
            self.id_stokes = coord_types.index('Stokes')
            casalog.post('id_direction=%s'%(self.id_direction))
            casalog.post('id_spectral=%s'%(self.id_spectral))
            casalog.post('id_stokes=%s'%(self.id_stokes))
            self.data = ia.getchunk()
            self.mask = ia.getchunk(getmask=True)
            bottom = ia.toworld(numpy.zeros(len(self.image_shape),dtype=int), 'q')['quantity']
            top = ia.toworld(self.image_shape-1, 'q')['quantity']
            key = lambda x: '*%s'%(x+1)
            ra_min = bottom[key(self.id_direction[0])]
            ra_max = top[key(self.id_direction[0])]
            if ra_min > ra_max:
                ra_min,ra_max = ra_max,ra_min
            self.ra_min = ra_min
            self.ra_max = ra_max
            self.dec_min = bottom[key(self.id_direction[1])]
            self.dec_max = top[key(self.id_direction[1])]
            self._brightnessunit = ia.brightnessunit()
            refpix, refval, increment = self.spectral_axis(unit='GHz')
            self.frequency = numpy.array([refval+increment*(i-refpix) for i in xrange(self.nchan)])
        finally:
            ia.close()
        
    @property
    def nx(self):
        return self.image_shape[self.id_direction[0]]

    @property
    def ny(self):
        return self.image_shape[self.id_direction[1]]

    @property
    def nchan(self):
        return self.image_shape[self.id_spectral]

    @property
    def npol(self):
        return self.image_shape[self.id_stokes]
    
    @property
    def brightnessunit(self):
        return self._brightnessunit

    @property
    def direction_label(self):
        return [self.names[i] for i in self.id_direction]
    
    @property
    def spectral_label(self):
        return self.names[self.id_spectral]
    
    @property
    def spectral_unit(self):
        return self.units[self.id_spectral]
        
    def to_velocity(self, frequency, freq_unit='GHz'):
        rest_frequency = self.coordsys.restfrequency()
        if rest_frequency['unit'] != freq_unit:
            vrf = qa.convert(rest_frequency, freq_unit)['value']
        else:
            vrf = rest_frequency['value']
        return (1.0 - (frequency / vrf)) * LightSpeed

    def spectral_axis(self, unit='GHz'):
        return self.__axis(self.id_spectral, unit=unit)

    def direction_axis(self, idx, unit='deg'):
        return self.__axis(self.id_direction[idx], unit=unit)
    
    def __axis(self, idx, unit):
        refpix = self.coordsys.referencepixel()['numeric'][idx]
        refval = self.coordsys.referencevalue()['numeric'][idx]
        increment = self.coordsys.increment()['numeric'][idx]
        _unit = self.units[idx]
        if _unit != unit:
            refval = qa.convert(qa.quantity(refval,_unit),unit)['value']
            increment = qa.convert(qa.quantity(increment,_unit),unit)['value']
        #return numpy.array([refval+increment*(i-refpix) for i in xrange(self.nchan)])
        return (refpix, refval, increment)
