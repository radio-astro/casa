from __future__ import absolute_import

import os
import math
import pylab as pl
import numpy

from matplotlib.ticker import FuncFormatter, MultipleLocator, AutoLocator

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from pipeline.domain.datatable import DataTableImpl as DataTable
from pipeline.domain.datatable import OnlineFlagIndex
#from . import common

LOG = infrastructure.get_logger(__name__)

RArotation = 90
DECrotation = 0

DPISummary = 90

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

def HHMM(x, pos):
    # HHMM format
    (h, m, s) = Deg2HMS(x, 1/40.0)
    #return '%02dh%02dm' % (h, m)
    return '%02d%s%02d' % (h, hsyb, m)

def HHMMSS(x, pos):
    # HHMMSS format
    (h, m, s) = Deg2HMS(x, 1/2400.0)
    #return '%02dh%02dm%02ds' % (h, m, s)
    return '%02d%s%02d%s%02d' % (h, hsyb, m, msyb, s)

def HHMMSSs(x, pos):
    # HHMMSS.s format
    (h, m, s) = Deg2HMS(x, 1/24000.0)
    #return '%02dh%02dm%04.1fs' % (h, m, s)
    return '%02d%s%02d%s%04.1f' % (h, hsyb, m, msyb, s)

def HHMMSSss(x, pos):
    # HHMMSS.ss format
    (h, m, s) = Deg2HMS(x, 1/240000.0)
    #return '%02dh%02dm%05.2fs' % (h, m, s)
    return '%02d%s%02d%s%05.2f' % (h, hsyb, m, msyb, s)

def HHMMSSsss(x, pos):
    # HHMMSS.sss format
    (h, m, s) = Deg2HMS(x, 1/2400000.0)
    #return '%02dh%02dm%06.3fs' % (h, m, s)
    return '%02d%s%02d%s%06.3f' % (h, hsyb, m, msyb, s)


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

def DDMM(x, pos):
    # +DDMM format
    (d, m, s) = Deg2DMS(x, 1/600.0)
    #return '%+02dd%02dm' % (d, m)
    return '%+02d%s%02d\'' % (d, dsyb, m)

def DDMMSS(x, pos):
    # +DDMMSS format
    (d, m, s) = Deg2DMS(x, 1/36000.0)
    #return '%+02dd%02dm%02ds' % (d, m, s)
    return '%+02d%s%02d\'%02d\"' % (d, dsyb, m, s)

def DDMMSSs(x, pos):
    # +DDMMSS.s format
    (d, m, s) = Deg2DMS(x, 1/360000.0)
    #return '%+02dd%02dm%04.1fs' % (d, m, s)
    sint = int(s)
    sstr = ('%3.1f'%(s-int(s))).lstrip('0')
    return '%+02d%s%02d\'%02d\"%s' % (d, dsyb, m, sint, sstr)

def DDMMSSss(x, pos):
    # +DDMMSS.ss format
    (d, m, s) = Deg2DMS(x, 1/3600000.0)
    #return '%+02dd%02dm%05.2fs' % (d, m, s)
    sint = int(s)
    sstr = ('%4.2f'%(s-int(s))).lstrip('0')
    return '%+02d%s%02d\'%02d\"%s' % (d, dsyb, m, sint, sstr)

def RADEClabel(span):
    """
    return (RAlocator, DEClocator, RAformatter, DECformatter)
    """
    RAtick = [15.0, 5.0, 2.5, 1.25, 1/2.0, 1/4.0, 1/12.0, 1/24.0, 1/48.0, 1/120.0, 1/240.0, 1/480.0, 1/1200.0, 1/2400.0, 1/4800.0, 1/12000.0, 1/24000.0, 1/48000.0, -1.0]
    DECtick = [20.0, 10.0, 5.0, 2.0, 1.0, 1/3.0, 1/6.0, 1/12.0, 1/30.0, 1/60.0, 1/180.0, 1/360.0, 1/720.0, 1/1800.0, 1/3600.0, 1/7200.0, 1/18000.0, 1/36000.0, -1.0]
    for RAt in RAtick:
        if span > (RAt * 3.0) and RAt > 0:
            RAlocator = MultipleLocator(RAt)
            break
    #if RAt < 0: RAlocator = MultipleLocator(1/96000.)
    if RAt < 0: RAlocator = AutoLocator()
    for DECt in DECtick:
        if span > (DECt * 3.0) and DECt > 0:
            DEClocator = MultipleLocator(DECt)
            break
    #if DECt < 0: DEClocator = MultipleLocator(1/72000.0)
    if DECt < 0: DEClocator = AutoLocator()
            
    if span < 0.0001:
        RAformatter=FuncFormatter(HHMMSSsss)
        DECformatter=FuncFormatter(DDMMSSss)
    elif span < 0.001:
        RAformatter=FuncFormatter(HHMMSSss)
        DECformatter=FuncFormatter(DDMMSSs)
    elif span < 0.01:
        RAformatter=FuncFormatter(HHMMSSs)
        DECformatter=FuncFormatter(DDMMSS)
    elif span < 1.0:
        RAformatter=FuncFormatter(HHMMSS)
        #DECformatter=FuncFormatter(DDMM)
        DECformatter=FuncFormatter(DDMMSS)
    else:
        RAformatter=FuncFormatter(HHMM)
        DECformatter=FuncFormatter(DDMM)

    return (RAlocator, DEClocator, RAformatter, DECformatter)

class PointingAxesManager(object):
    MATPLOTLIB_FIGURE_ID = 9005
    
    @property
    def direction_reference(self):
        return self._direction_reference
    
    @direction_reference.setter
    def direction_reference(self, value):
        if isinstance(value, str):
            self._direction_reference = value
    
    def __init__(self):
        self._axes = None
        self.is_initialized = False
        self._direction_reference = None

    def init_axes(self, xlocator, ylocator, xformatter, yformatter, xrotation, yrotation, aspect, xlim=None, ylim=None, reset=False):
        if self._axes is None:
            self._axes = self.__axes()

        if xlim is not None:
            self._axes.set_xlim(xlim)

        if ylim is not None:
            self._axes.set_ylim(ylim)
            
        if self.is_initialized == False or reset:
            # 2008/9/20 DEC Effect
            self._axes.set_aspect(aspect)
            self._axes.xaxis.set_major_formatter(xformatter)
            self._axes.yaxis.set_major_formatter(yformatter)
            self._axes.xaxis.set_major_locator(xlocator)
            self._axes.yaxis.set_major_locator(ylocator)
            xlabels = self._axes.get_xticklabels()
            pl.setp(xlabels, 'rotation', xrotation, fontsize=8)
            ylabels = self._axes.get_yticklabels()
            pl.setp(ylabels, 'rotation', yrotation, fontsize=8)
            
    @property
    def axes(self):
        if self._axes is None:
            self._axes = self.__axes()
        return self._axes

    def __axes(self):
        a = pl.axes([0.15, 0.2, 0.7, 0.7])
        if self.direction_reference is None:
            pl.xlabel('RA')
            pl.ylabel('Dec')
        else:
            pl.xlabel('RA ({0})'.format(self.direction_reference))
            pl.ylabel('Dec ({0})'.format(self.direction_reference))
        pl.title('')
        return a

def draw_beam(axes, r, aspect, x_base, y_base, offset=1.0):
    xy = numpy.array([[r * (math.sin(t * 0.13) + offset) * aspect + x_base,
                       r * (math.cos(t * 0.13) + offset) + y_base]
                      for t in xrange(50)])
    pl.gcf().sca(axes)
    line = pl.plot(xy[:,0], xy[:,1], 'r-')
    return line[0]

def draw_pointing(axes_manager, RA, DEC, FLAG=None, plotfile=None, connect=True, circle=[], ObsPattern=False, plotpolicy='ignore'):
    span = max(max(RA) - min(RA), max(DEC) - min(DEC))
    xmax = min(RA) - span / 10.0
    xmin = max(RA) + span / 10.0
    ymax = max(DEC) + span / 10.0
    ymin = min(DEC) - span / 10.0
    (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

    Aspect = 1.0 / math.cos(DEC[0] / 180.0 * 3.141592653)

    # Plotting routine
    if connect is True: Mark = 'g-o'
    else: Mark = 'bo'
    axes_manager.init_axes(RAlocator, DEClocator,
                                RAformatter, DECformatter,
                                RArotation, DECrotation,
                                Aspect,
                                xlim=(xmin,xmax),
                                ylim=(ymin,ymax))
    a = axes_manager.axes
    if ObsPattern == False:
        a.title.set_text('Telescope Pointing on the Sky')
    else:
        a.title.set_text('Telescope Pointing on the Sky\nPointing Pattern = %s' % ObsPattern)
    plot_objects = []
    
    if plotpolicy == 'plot':
        # Original
        plot_objects.extend(
            pl.plot(RA, DEC, Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
            )
    elif plotpolicy == 'ignore':
        # Ignore Flagged Data
        filter = FLAG == 1
        plot_objects.extend(
            pl.plot(RA[filter], DEC[filter], Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
            )
    elif plotpolicy == 'greyed':
        # Change Color 
        if connect is True:
            plot_objects.extend(pl.plot(RA, DEC, 'g-'))
        filter = FLAG == 1
        plot_objects.extend(
            pl.plot(RA[filter], DEC[filter], 'o', markersize=2, markeredgecolor='b', markerfacecolor='b')
            )
        filter = FLAG == 0
        if numpy.any(filter == True):
            plot_objects.extend(
                pl.plot(RA[filter], DEC[filter], 'o', markersize=2, markeredgecolor='grey', markerfacecolor='grey')
                )
    # plot starting position with beam and end position 
    if len(circle) != 0:
        plot_objects.append(
           	draw_beam(a, circle[0], Aspect, RA[0], DEC[0], offset=0.0)
            )
        Mark = 'ro'
        plot_objects.extend(
            pl.plot(RA[-1], DEC[-1], Mark, markersize=4, markeredgecolor='r', markerfacecolor='r')
            )
    pl.axis([xmin, xmax, ymin, ymax])
    if plotfile is not None:
        pl.savefig(plotfile, format='png', dpi=DPISummary)

    for obj in plot_objects:
        obj.remove()
    
class SingleDishPointingChart(object):    
    def __init__(self, context, ms, antenna, target_field_id=None, reference_field_id=None, target_only=True,
                 shift_coord=False):
        self.context = context
        self.ms = ms
        self.antenna = antenna
        self.target_field = self.__get_field(target_field_id)
        self.reference_field = self.__get_field(reference_field_id) 
        self.target_only = target_only
        self.shift_coord = shift_coord
        self.figfile = self._get_figfile()
        self.axes_manager = PointingAxesManager()
        
    def __get_field(self, field_id):
        if field_id is not None:
            fields = self.ms.get_fields(field_id)
            assert len(fields) == 1
            field = fields[0]
            LOG.debug('found field domain for %s'%(field_id))
            return field
        else:
            return None
        
    def plot(self, revise_plot=False):
        if revise_plot == False and os.path.exists(self.figfile):
            return self._get_plot_object()
        
        ms = self.ms
        antenna_id = self.antenna.id
        
        datatable_name = os.path.join(self.context.observing_run.ms_datatable_name, ms.basename)
        datatable = DataTable()
        datatable.importdata(datatable_name, minimal=False, readonly=True)

        target_spws = ms.get_spectral_windows(science_windows_only=True)
        # Search for the first available SPW, antenna combination
        # observing_pattern is None for invalid combination.
        spw_id = None
        for s in target_spws:
            field_patterns = ms.observing_pattern[antenna_id][s.id].values()
            if field_patterns.count(None) < len(field_patterns):
                # at least one valid field exists.
                spw_id = s.id
                break
        if spw_id is None:
            LOG.info('No data with antenna=%d and spw=%s found in %s' % (antenna_id, str(target_spws), ms.basename))
            LOG.info('Skipping pointing plot')
            return None
        else: LOG.debug('Generate pointing plot using antenna=%d and spw=%d of %s' % (antenna_id, spw_id, ms.basename))
        beam_size = casatools.quanta.convert(ms.beam_sizes[antenna_id][spw_id], 'deg')
        beam_size_in_deg = casatools.quanta.getvalue(beam_size)
        obs_pattern = ms.observing_pattern[antenna_id][spw_id]
        antenna_ids = datatable.getcol('ANTENNA')
        spw_ids = datatable.getcol('IF')
        if self.target_field is None or self.reference_field is None:
            # plot pointings regardless of field
            if self.target_only == True:
                srctypes = datatable.getcol('SRCTYPE')
                func = lambda j, k, l: j == antenna_id and k == spw_id and l == 0
                vfunc = numpy.vectorize(func)
                dt_rows = vfunc(antenna_ids, spw_ids, srctypes)
            else:
                func = lambda j, k: j == antenna_id and k == spw_id
                vfunc = numpy.vectorize(func)
                dt_rows = vfunc(antenna_ids, spw_ids)
        else:
            field_ids = datatable.getcol('FIELD_ID')
            if self.target_only == True:
                srctypes = datatable.getcol('SRCTYPE')
                field_id = [self.target_field.id]
                func = lambda f, j, k, l: f in field_id and j == antenna_id and k == spw_id and l == 0
                vfunc = numpy.vectorize(func)
                dt_rows = vfunc(field_ids, antenna_ids, spw_ids, srctypes)
            else:
                field_id = [self.target_field.id, self.reference_field.id]
                func = lambda f, j, k: f in field_id and j == antenna_id and k == spw_id 
                vfunc = numpy.vectorize(func)
                dt_rows = vfunc(field_ids, antenna_ids, spw_ids)
        
        if self.shift_coord == True:
            racol = 'SHIFT_RA'
            deccol = 'SHIFT_DEC'
        else:
            racol = 'RA'
            deccol = 'DEC'
        LOG.debug('column names: {}, {}'.format(racol, deccol))
        if racol not in datatable.colnames() or deccol not in datatable.colnames():
            return None
        
        RA = datatable.getcol(racol)[dt_rows]
        if len(RA) == 0: # no row found
            LOG.warn('No data found with antenna=%d, spw=%d, and field=%s in %s.' % (antenna_id, spw_id, str(field_id), ms.basename))
            LOG.warn('Skipping pointing plots.')
            return None
        DEC = datatable.getcol(deccol)[dt_rows]
        FLAG = numpy.zeros(len(RA), dtype=int)
        rows = numpy.where(dt_rows == True)[0]
        assert len(RA) == len(rows)
        for (i, row) in enumerate(rows):
            pflags = datatable.getcell('FLAG_PERMANENT', row)
            # use flag for pol 0
            FLAG[i] = pflags[0][OnlineFlagIndex]
                
        self.axes_manager.direction_reference = datatable.direction_ref
        
        pl.clf()
        draw_pointing(self.axes_manager, RA, DEC, FLAG, self.figfile, circle=[0.5*beam_size_in_deg], ObsPattern=obs_pattern, plotpolicy='greyed')
        pl.close()

        return self._get_plot_object()

    def _get_figfile(self):
        session_part = self.ms.session
        ms_part = self.ms.basename
        antenna_part = self.antenna.name
        if self.target_field is None or self.reference_field is None:
            identifier = antenna_part
        else:
            clean_name = self.target_field.clean_name
            identifier = antenna_part + '.%s'%(clean_name)
        if self.target_only == True:
            if self.shift_coord == True:
                basename = 'shifted_target_pointing.%s'%(identifier)
            else:
                basename = 'target_pointing.%s'%(identifier)
        else:
            basename = 'whole_pointing.%s'%(identifier)
        figfile = os.path.join(self.context.report_dir, 
                               'session%s' % session_part, 
                               ms_part,
                               '%s.png'%(basename))
        return figfile

    def _get_plot_object(self):
        intent = 'target' if self.target_only == True else 'target,reference'
        if self.target_field is None or self.reference_field is None:
            field_name = ''
        else:
            if self.target_only or self.target_field.name == self.reference_field.name:
                field_name = self.target_field.name
            else:
                field_name = self.target_field.name + ',' + self.reference_field.name
        if self.shift_coord == True:
            xaxis = 'Shifted R.A.'
            yaxis = 'Shifted Declination'
        else:
            xaxis = 'R.A.'
            yaxis = 'Declination'
        return logger.Plot(self.figfile,
                           x_axis=xaxis,
                           y_axis=yaxis,
                           parameters={'vis' : self.ms.basename,
                                       'antenna': self.antenna.name,
                                       'field': field_name,
                                       'intent': intent})
