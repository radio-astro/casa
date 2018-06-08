from __future__ import absolute_import

import abc
import datetime
import itertools
import math
import matplotlib.gridspec as gridspec
import numpy
import os
import pylab as pl
import time
from matplotlib.dates import date2num, DateFormatter, MinuteLocator
from matplotlib.ticker import FuncFormatter, MultipleLocator, AutoLocator

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays.pointing as pointing
import pipeline.infrastructure.renderer.logger as logger
from . import atmutil

LOG = infrastructure.get_logger(__name__)

#ShowPlot = True
ShowPlot = False

DPISummary = 90
#DPIDetail = 120
#DPIDetail = 130
DPIDetail = 260
LightSpeedQuantity = casatools.quanta.constants('c')
LightSpeed = casatools.quanta.convert(LightSpeedQuantity, 'km/s')['value']  # speed of light in km/s

sd_polmap = {0: 'XX', 1: 'YY', 2: 'XY', 3: 'YX'}

NoData = -32767.0
NoDataThreshold = NoData + 10000.0


def mjd_to_datedict(val, unit='d'):
    mjd = casatools.quanta.quantity(val, unit)
    return casatools.quanta.splitdate(mjd)


def mjd_to_datetime(val):
    mjd = mjd_to_datedict(val, unit='d')
    date_time = datetime.datetime(mjd['year'], mjd['month'],
                                  mjd['monthday'], mjd['hour'],
                                  mjd['min'], mjd['sec'])
    return date_time


def mjd_to_datestring(val, fmt='%Y/%m/%d'):
    date_time = mjd_to_datetime(val)
    return date_time.strftime(fmt)


# vectorized version
mjd_to_datetime_vectorized = numpy.vectorize(mjd_to_datetime)


def mjd_to_plotval(mjd_list):
    datetime_list = mjd_to_datetime_vectorized(mjd_list)
    return date2num(datetime_list)
    

class CustomDateFormatter(DateFormatter):
    """
    Customized date formatter that puts YYYY/MM/DD under usual
    tick labels at the beginning of tick and when date is changed.
    """
    def __call__(self, x, pos=0):
        fmt_saved = self.fmt
        if pos == 0 or x % 1.0 == 0.0:
            self.fmt = '%H:%M\n%Y/%m/%d'
        tick = DateFormatter.__call__(self, x, pos)
        self.fmt = fmt_saved
        return tick


def utc_formatter(fmt='%H:%M'):
    return CustomDateFormatter(fmt)


def utc_locator(start_time=None, end_time=None):
    if start_time is None or end_time is None:
        return MinuteLocator()
    else:
        dt = abs(end_time - start_time) * 1440.0 # day -> minutes
        #print dt
        tick_interval = int(dt/10) + 1
        #print tick_interval
        return MinuteLocator(byminute=range(0,60,tick_interval))


class PlotObjectHandler(object):
    def __init__(self):
        self.storage = []
        
    def __del__(self):
        self.clear()
    
    def plot(self, *args, **kwargs):
        object_list = pl.plot(*args, **kwargs)
        self.storage.extend(object_list)
        return object_list
    
    def text(self, *args, **kwargs):
        object_list = pl.text(*args, **kwargs)
        self.storage.append(object_list)
        return object_list
    
    def axvspan(self, *args, **kwargs):
        object_list = pl.axvspan(*args, **kwargs)
        self.storage.append(object_list)
        return object_list
    
    def axhline(self, *args, **kwargs):
        object_list = pl.axhline(*args, **kwargs)
        self.storage.append(object_list)
        return object_list
    
    def clear(self):
        for obj in self.storage:
            obj.remove()
        self.storage = []


class SingleDishDisplayInputs(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result


class SpectralImage(object):
    def __init__(self, imagename):
        # read data to storage
        with casatools.ImageReader(imagename) as ia:
            self.image_shape = ia.shape()
            self.coordsys = ia.coordsys()
            coord_types = self.coordsys.axiscoordinatetypes()
            self.units = self.coordsys.units()
            self.id_direction = coord_types.index('Direction')
            self.id_direction = [self.id_direction, self.id_direction+1]
            self.id_spectral = coord_types.index('Spectral')
            self.id_stokes = coord_types.index('Stokes')
            LOG.debug('id_direction=%s'%(self.id_direction))
            LOG.debug('id_spectral=%s'%(self.id_spectral))
            LOG.debug('id_stokes=%s'%(self.id_stokes))
            self.data = ia.getchunk()
            self.mask = ia.getchunk(getmask=True)
            bottom = ia.toworld(numpy.zeros(len(self.image_shape), dtype=int), 'q')['quantity']
            top = ia.toworld(self.image_shape-1, 'q')['quantity']
            key = lambda x: '*%s'%(x+1)
            ra_min = bottom[key(self.id_direction[0])]
            ra_max = top[key(self.id_direction[0])]
            if ra_min > ra_max:
                ra_min, ra_max = ra_max, ra_min
            self.ra_min = ra_min
            self.ra_max = ra_max
            self.dec_min = bottom[key(self.id_direction[1])]
            self.dec_max = top[key(self.id_direction[1])]
            self._brightnessunit = ia.brightnessunit()
            beam = ia.restoringbeam()
            self.direction_reference = self.coordsys.referencecode('dir')[0]
        qa = casatools.quanta
        self._beamsize_in_deg = qa.convert(qa.sqrt(qa.mul(beam['major'], beam['minor'])), 'deg')['value']
        
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
#         if self._brightnessunit.find('Jy') != -1:
#             return 'Jy'
#         else:
#             return 'K'

    @property
    def beam_size(self):
        return self._beamsize_in_deg

    def to_velocity(self, frequency, freq_unit='GHz'):
        qa = casatools.quanta
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
        qa = casatools.quanta
        refpix = self.coordsys.referencepixel()['numeric'][idx]
        refval = self.coordsys.referencevalue()['numeric'][idx]
        increment = self.coordsys.increment()['numeric'][idx]
        _unit = self.units[idx]
        if _unit != unit:
            refval = qa.convert(qa.quantity(refval, _unit), unit)['value']
            increment = qa.convert(qa.quantity(increment, _unit), unit)['value']
        #return numpy.array([refval+increment*(i-refpix) for i in xrange(self.nchan)])
        return (refpix, refval, increment)
        

class SDImageDisplayInputs(SingleDishDisplayInputs):
    def __init__(self, context, result):
        super(SDImageDisplayInputs,self).__init__(context, result)
        
    @property
    def imagename(self):
        return self.result.outcome['image'].imagename

    @property
    def spw(self):
        spwlist = self.result.outcome['image'].spwlist
        if isinstance(spwlist, list):
            return spwlist[0]
        else:
            return spwlist

    @property
    def vis(self):
        if 'vis' in self.result.outcome:
            return self.result.outcome['vis']
        else:
            return None

    @property
    def antenna(self):
        return self.result.outcome['image'].antenna

    @property
    def reduction_group(self):
        """
        Retruns ReductionGroupDesc instance corresponding to the reduction group
        associated to the image
        """
        group_id = self.result.outcome['reduction_group_id']
        return self.context.observing_run.ms_reduction_group[group_id]
    
    @property
    def msid_list(self):
        return self.result.outcome['file_index']

    @property
    def antennaid_list(self):
        return self.result.outcome['assoc_antennas']
    
    @property
    def fieldid_list(self):
        return self.result.outcome['assoc_fields']

    @property
    def spwid_list(self):
        return self.result.outcome['assoc_spws']

    @property
    def stage_number(self):
        return self.result.stage_number

    @property
    def stage_dir(self):
        return os.path.join(self.context.report_dir,
                            'stage%d'%(self.stage_number))

    @property
    def source(self):
        return self.result.outcome['image'].sourcename


class SDCalibrationDisplay(object):
    __metaclass__ = abc.ABCMeta
    Inputs = SingleDishDisplayInputs

    def __init__(self, inputs):
        self.inputs = inputs

    def plot(self):
        results = self.inputs.result
        report_dir = self.inputs.context.report_dir
        stage_dir = os.path.join(report_dir, 'stage%d'%(results.stage_number))
        plots = []
        for result in results:
            if result is None or result.outcome is None:
                plot = None
            else:
                plot = self.doplot(result, stage_dir)
                
            if plot is not None:
                plots.append(plot)
        return plots

    @abc.abstractmethod
    def doplot(self, result, stage_dir):
        raise NotImplementedError()

        
class SDImageDisplay(object):
    __metaclass__ = abc.ABCMeta
    Inputs = SDImageDisplayInputs

    def __init__(self, inputs):
        self.inputs = inputs
        self.context = self.inputs.context
        self.stage_dir = self.inputs.stage_dir
        self.image = None
        self.imagename = self.inputs.imagename
        self.spw = self.inputs.spw
        self.antenna = self.inputs.antenna
        self.vis = self.inputs.vis

    def init(self):
        self.image = SpectralImage(self.imagename)
        qa = casatools.quanta
        self.nchan = self.image.nchan
#         self.data = self.image.data
#         self.mask = self.image.mask
        self.nx = self.image.nx
        self.ny = self.image.ny
        self.npol = self.image.npol
        self.brightnessunit = self.image.brightnessunit
        self.direction_reference = self.image.direction_reference
        (refpix, refval, increment) = self.image.spectral_axis(unit='GHz')
        self.frequency = numpy.array([refval+increment*(i-refpix) for i in xrange(self.nchan)])
        self.velocity = self.image.to_velocity(self.frequency, freq_unit='GHz')
        self.frequency_frame = self.image.coordsys.getconversiontype('spectral')
        self.x_max = self.nx - 1
        self.x_min = 0
        self.y_max = self.ny - 1
        self.y_min = 0
        self.ra_min = qa.convert(self.image.ra_min, 'deg')['value']
        self.ra_max = qa.convert(self.image.ra_max, 'deg')['value']
        self.dec_min = qa.convert(self.image.dec_min, 'deg')['value']
        self.dec_max = qa.convert(self.image.dec_max, 'deg')['value']

        LOG.debug('(ra_min,ra_max)=(%s,%s)' % (self.ra_min, self.ra_max))
        LOG.debug('(dec_min,dec_max)=(%s,%s)' % (self.dec_min, self.dec_max))

        self.beam_size = self.image.beam_size
        self.beam_radius = self.beam_size / 2.0
        self.grid_size = self.beam_size / 3.0
        LOG.debug('beam_radius=%s'%(self.beam_radius))
        LOG.debug('grid_size=%s'%(self.grid_size))

        # 2008/9/20 Dec Effect has been taken into account
        self.aspect = 1.0 / math.cos(0.5 * (self.dec_min + self.dec_max) / 180.0 * 3.141592653)

    @property
    def data(self):
        return self.image.data if self.image is not None else None

    @property
    def mask(self):
        return self.image.mask if self.image is not None else None

    @property
    def id_spectral(self):
        return self.image.id_spectral if self.image is not None else None
    
    @property
    def id_stokes(self):
        return self.image.id_stokes if self.image is not None else None

    @property
    def num_valid_spectrum(self):
        return self.__reshape2d(self.inputs.result.outcome['validsp'], int)

    @property
    def rms(self):
        return self.__reshape2d(self.inputs.result.outcome['rms'], float)

    @property
    def edge(self):
        return self.inputs.result.outcome['edge']

    def __reshape2d(self, array2d, dtype=None):
        array3d = numpy.zeros((self.npol, self.ny, self.nx), dtype=dtype)
        if len(array2d) == self.npol:
            each_len = numpy.array(map(len, array2d))
            if numpy.all(each_len == 0):
                # no valid data in the pixel
                array3d = numpy.zeros((self.npol, self.ny, self.nx), dtype=dtype)
            elif numpy.all(each_len == self.ny * self.nx):
                # all polarizations has valid data in each pixel
                array3d = numpy.array(array2d).reshape((self.npol, self.ny, self.nx))
            elif numpy.any(each_len == self.ny * self.nx):
                # probably one of the polarization components has no valid data
                invalid_pols = numpy.where(each_len == 0)[0]
                _array2d = []
                for i in xrange(self.npol):
                    if i in invalid_pols:
                        _array2d.append(numpy.zeros((self.ny * self.nx), dtype=dtype))
                    else:
                        _array2d.append(array2d[i])
                array3d = numpy.array(_array2d).reshape((self.npol, self.ny, self.nx))
        return numpy.flipud(array3d.transpose())


def get_base_frequency(table, freqid, nchan):
    freq_table = os.path.join(table, 'FREQUENCIES')
    with casatools.TableReader(freq_table) as tb:
        refpix = tb.getcell('REFPIX', freqid)
        refval = tb.getcell('REFVAL', freqid)
        increment = tb.getcell('INCREMENT', freqid)
        chan_freq = numpy.array([refval + (i - refpix) * increment 
                                 for i in xrange(nchan)])
    return chan_freq


def get_base_frame(table):
    freq_table = os.path.join(table, 'FREQUENCIES')
    with casatools.TableReader(freq_table) as tb:
        base_frame = tb.getkeyword('BASEFRAME')
    return base_frame


def drop_edge(array):
    # array should be two-dimensional (nchan,nrow)
    nchan = array.shape[0]
    a = None
    if nchan > 2:
        echan = max(1, int(nchan * 0.05))
        a = array[echan:-echan, ::]
    return a


class TimeAxesManager(object):
    def __init__(self):
        self.locator = utc_locator()
        
    def init(self, start_time=None, end_time=None):
        self.locator = utc_locator(start_time, end_time)


# sparse profile map
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


class MapAxesManagerBase(object):
    @property
    def direction_reference(self):
        return self._direction_reference
    
    @direction_reference.setter
    def direction_reference(self, value):
        if isinstance(value, str):
            self._direction_reference = value
    
    def __init__(self):
        self._direction_reference = None
        
    def __get_axis_label(self, base_label):
        if self.direction_reference is None:
            return base_label
        else:
            return '{0} ({1})'.format(base_label,
                                      self.direction_reference)
        
    def get_horizontal_axis_label(self):
        return self.__get_axis_label('RA')
    
    def get_vertical_axis_label(self):
        return self.__get_axis_label('Dec')
        

class SparseMapAxesManager(MapAxesManagerBase):
    def __init__(self, nh, nv, brightnessunit, ticksize, clearpanel=True, figure_id=None):
        super(SparseMapAxesManager, self).__init__()
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
        self.figure = pl.figure(self.figure_id, dpi=DPIDetail)
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
                pl.text(0.5, 0.5, pointing.HHMMSSss((label_ra[x][0]+label_ra[x][1])/2.0, 0),
                        horizontalalignment='center', verticalalignment='center', size=self.ticksize)
            else:
                a1.texts[0].set_text(pointing.HHMMSSss((label_ra[x][0]+label_ra[x][1])/2.0, 0))
        for y in xrange(self.nv):
            a1 = pl.subplot(self.gs_bottom[self.nv - y - 1, 0])
            a1.set_axis_off()
            if len(a1.texts) == 0:
                pl.text(0.5, 0.5, pointing.DDMMSSs((label_dec[y][0]+label_dec[y][1])/2.0, 0),
                        horizontalalignment='center', verticalalignment='center', size=self.ticksize)
            else:
                a1.texts[0].set_text(pointing.DDMMSSs((label_dec[y][0]+label_dec[y][1])/2.0, 0))
        a1 = pl.subplot(self.gs_bottom[-1, 0])
        a1.set_axis_off()
        ralabel = self.get_horizontal_axis_label()
        declabel = self.get_vertical_axis_label()
        pl.text(0.5, 1, declabel, horizontalalignment='center', verticalalignment='bottom', size=(self.ticksize+1))
        pl.text(1, 0.5, ralabel, horizontalalignment='right', verticalalignment='center', size=(self.ticksize+1))


class SDSparseMapPlotter(object):
    def __init__(self, nh, nv, step, brightnessunit, clearpanel=True, figure_id=None):
        self.step = step
        if step > 1:
            ticksize = 10 - int(max(nh, nv) * step / (step - 1)) / 2
        elif step == 1:
            ticksize = 10 - int(max(nh, nv)) / 2
        ticksize = max(ticksize, 3)
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
    
    @property
    def direction_reference(self):
        return self.axes.direction_reference
    
    @direction_reference.setter
    def direction_reference(self, value):
        self.axes.direction_reference = value
    
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
        LOG.debug('spmin=%s, spmax=%s' % (spmin, spmax))
        
        global_xmin = min(frequency[0], frequency[-1])
        global_xmax = max(frequency[0], frequency[-1])
        LOG.debug('global_xmin=%s, global_xmax=%s' % (global_xmin, global_xmax))

        # Auto scaling
        # to eliminate max/min value due to bad pixel or bad fitting,
        #  1/10-th value from max and min are used instead
        valid_index = numpy.where(map_data.min(axis=2) > NoDataThreshold)
        valid_data = map_data[valid_index[0], valid_index[1], :]
        LOG.debug('valid_data.shape={shape}'.format(shape=valid_data.shape))
        del valid_index
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
        del valid_data
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
            for (t, f) in itertools.izip(self.atm_transmission, self.atm_frequency):
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
                    LOG.debug('Per panel scaling turned on: ymin=%s, ymax=%s (global ymin=%s, ymax=%s)' %
                              (ymin, ymax, global_ymin, global_ymax))
                pl.gcf().sca(self.axes.axes_spmap[y+(self.nh-x-1)*self.nv])
                if map_data[x][y].min() > NoDataThreshold:
                    plot_helper.plot(frequency, map_data[x][y], color='b', linestyle='-', linewidth=0.2)
                    if self.lines_map is not None and self.lines_map[x][y] is not None:
                        for chmin, chmax in self.lines_map[x][y]:
                            fmin = ch_to_freq(chmin, frequency)
                            fmax = ch_to_freq(chmax, frequency)
                            LOG.debug('plotting line range for %s, %s: [%s, %s]' % (x, y, chmin, chmax))
                            plot_helper.axvspan(fmin, fmax, color='cyan')
                    elif self.lines_averaged is not None:
                        for chmin, chmax in self.lines_averaged:
                            fmin = ch_to_freq(chmin, frequency)
                            fmax = ch_to_freq(chmax, frequency)
                            LOG.debug('plotting line range for %s, %s (reuse lines_averaged): [%s, %s]' %
                                      (x, y, chmin, chmax))
                            plot_helper.axvspan(fmin, fmax, color='cyan')
                    if is_valid_fit_result:
                        plot_helper.plot(frequency, fit_result[x][y], color='r', linewidth=0.4)
                    elif self.reference_level is not None and ymin < self.reference_level and self.reference_level < ymax:
                        plot_helper.axhline(self.reference_level, color='r', linewidth=0.4) 
                else:
                    plot_helper.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'NO DATA', ha='center', va='center', 
                                     size=(self.TickSize + 1))
                pl.axis((xmin, xmax, ymin, ymax))

        if ShowPlot:
            pl.draw()

        if figfile is not None:
            pl.savefig(figfile, format='png', dpi=DPIDetail)
        LOG.debug('figfile=\'%s\''%(figfile))
        
        plot_helper.clear()

        return True
    
    def done(self):
        pl.close()
        del self.axes


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
