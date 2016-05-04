from __future__ import absolute_import

import os
import math
import pylab as pl
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from pipeline.domain.datatable import DataTableImpl as DataTable
from pipeline.domain.datatable import OnlineFlagIndex
from . import common
from .utils import RADEClabel, RArotation, DECrotation

LOG = infrastructure.get_logger(__name__)

class PointingAxesManager(object):
    MATPLOTLIB_FIGURE_ID = 9005
    def __init__(self):
        self._axes = None
        self.is_initialized = False

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
        pl.xlabel('RA')
        pl.ylabel('Dec')
        pl.title('')
        return a

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
            common.draw_beam(a, circle[0], Aspect, RA[0], DEC[0], offset=0.0)
            )
        Mark = 'ro'
        plot_objects.extend(
            pl.plot(RA[-1], DEC[-1], Mark, markersize=4, markeredgecolor='r', markerfacecolor='r')
            )
    pl.axis([xmin, xmax, ymin, ymax])
    if common.ShowPlot != False: pl.draw()
    if plotfile is not None:
        pl.savefig(plotfile, format='png', dpi=common.DPISummary)

    for obj in plot_objects:
        obj.remove()
    
class SingleDishPointingChart(object):    
    def __init__(self, context, ms, antenna, target_field_id=None, reference_field_id=None, target_only=True):
        self.context = context
        self.ms = ms
        self.antenna = antenna
        self.target_field = self.__get_field(target_field_id)
        self.reference_field = self.__get_field(reference_field_id) 
        self.target_only = target_only
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
        
    def plot(self):
        if os.path.exists(self.figfile):
            return self._get_plot_object()
        
        ms = self.ms
        ms_id = self.context.observing_run.measurement_sets.index(ms)
        antenna_id = self.antenna.id
        
        datatable_name = self.context.observing_run.ms_datatable_name
        datatable = DataTable()
        datatable.importdata(datatable_name, minimal=False, readonly=True)

        target_spws = ms.get_spectral_windows(science_windows_only=True)
        spw_id = target_spws[0].id
        beam_size = casatools.quanta.convert(ms.beam_sizes[antenna_id][spw_id], 'deg')
        beam_size_in_deg = casatools.quanta.getvalue(beam_size)
        obs_pattern = ms.observing_pattern[antenna_id][spw_id]
        ms_ids = datatable.tb1.getcol('MS')
        antenna_ids = datatable.tb1.getcol('ANTENNA')
        spw_ids = datatable.tb1.getcol('IF')
        if self.target_field is None or self.reference_field is None:
            # plot pointings regardless of field
            if self.target_only == True:
                srctypes = datatable.tb1.getcol('SRCTYPE')
                func = lambda i, j, k, l: i == ms_id and j == antenna_id and k == spw_id and l == 0
                vfunc = numpy.vectorize(func)
                dt_rows = vfunc(ms_ids, antenna_ids, spw_ids, srctypes)
            else:
                func = lambda i, j, k: i == ms_id and j == antenna_id and k == spw_id
                vfunc = numpy.vectorize(func)
                dt_rows = vfunc(ms_ids, antenna_ids, spw_ids)
        else:
            field_ids = datatable.tb1.getcol('FIELD_ID')
            if self.target_only == True:
                srctypes = datatable.tb1.getcol('SRCTYPE')
                field_id = [self.target_field.id]
                func = lambda i, f, j, k, l: i == ms_id and f in field_id and j == antenna_id and k == spw_id and l == 0
                vfunc = numpy.vectorize(func)
                dt_rows = vfunc(ms_ids, field_ids, antenna_ids, spw_ids, srctypes)
            else:
                field_id = [self.target_field.id, self.reference_field.id]
                func = lambda i, f, j, k: i == ms_id and f in field_id and j == antenna_id and k == spw_id 
                vfunc = numpy.vectorize(func)
                dt_rows = vfunc(ms_ids, field_ids, antenna_ids, spw_ids)
        
        RA = datatable.tb1.getcol('RA')[dt_rows]
        DEC = datatable.tb1.getcol('DEC')[dt_rows]
        FLAG = datatable.tb2.getcol('FLAG_PERMANENT')[:,dt_rows][OnlineFlagIndex]
                
        pl.clf()
        draw_pointing(self.axes_manager, RA, DEC, FLAG, self.figfile, circle=[0.5*beam_size_in_deg], ObsPattern=obs_pattern, plotpolicy='greyed')

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
        return logger.Plot(self.figfile,
                           x_axis='R.A.',
                           y_axis='Declination',
                           parameters={'vis' : self.ms.basename,
                                       'antenna': self.antenna.name,
                                       'field': field_name,
                                       'intent': intent})
