from __future__ import absolute_import

import os
import math
import pylab as pl
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from . import common
from .utils import RADEClabel, RArotation, DECrotation

LOG = infrastructure.get_logger(__name__)

class PointingAxesManager(object):
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

class SDPointingDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8905
    AxesManager = PointingAxesManager

    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        vis = parent_ms.basename
        spws = parent_ms.get_spectral_windows(science_windows_only=True)
        spwid = spws[0].id
        beam_size = casatools.quanta.convert(self.context.observing_run[idx].beam_size[spwid], 'deg')
        obs_pattern = st.pattern[spwid].values()[0]
        rows = self.datatable.get_row_index(idx, spwid)
        datatable = self.datatable
        
        plots = []
        
        #ROW = datatable.getcol('ROW')
        tRA = datatable.getcol('RA')
        tDEC = datatable.getcol('DEC')
        tNCHAN = datatable.getcol('NCHAN')
        tSRCTYPE = datatable.getcol('SRCTYPE')
        
        RA = []
        DEC = []
        for row in rows:
            if tNCHAN[row] > 1:
                RA.append(tRA[row])
                DEC.append(tDEC[row])
        plotfile = os.path.join(stage_dir, 'pointing_full_%s.png'%(st.basename))
        self.draw_radec(RA, DEC, plotfile, circle=[0.5*beam_size['value']], ObsPattern=obs_pattern)
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'full pointing'
        parameters['file'] = st.basename
        parameters['vis'] = vis
        plots.append(logger.Plot(plotfile,
                                 x_axis='R.A.', y_axis='Dec.',
                                 field=parent_ms.fields[0].name,
                                 parameters=parameters))
        RA = []
        DEC = []
        srctype = st.calibration_strategy['srctype']
        for row in rows:
            if tNCHAN[row] > 1 and tSRCTYPE[row] == srctype:
                RA.append(tRA[row])
                DEC.append(tDEC[row])
        plotfile = os.path.join(stage_dir, 'pointing_onsource_%s.png'%(st.basename))
        self.draw_radec(RA, DEC, plotfile, circle=[0.5*beam_size['value']], ObsPattern=obs_pattern)
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'on source pointing'
        parameters['file'] = st.basename
        parameters['vis'] = vis
        plots.append(logger.Plot(plotfile,
                                 x_axis='R.A.', y_axis='Dec.',
                                 field=parent_ms.fields[0].name,
                                 parameters=parameters))
        return plots
        
    def draw_radec(self, RA, DEC, plotfile, connect=True, circle=[], ObsPattern=False):
        """
        Draw loci of the telescope pointing
        xaxis: extension header keyword for RA
        yaxis: extension header keyword for DEC
        connect: connect points if True
        """
        span = max(max(RA) - min(RA), max(DEC) - min(DEC))
        xmax = min(RA) - span / 10.0
        xmin = max(RA) + span / 10.0
        ymax = max(DEC) + span / 10.0
        ymin = min(DEC) - span / 10.0
        (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

        # 2008/9/20 DEC Effect
        Aspect = 1.0 / math.cos(DEC[0] / 180.0 * 3.141592653)

        # Plotting routine
        #if common.ShowPlot: pl.ion()
        #else: pl.ioff()
        #pl.figure(self.MATPLOTLIB_FIGURE_ID)
        #if common.ShowPlot: pl.ioff()
        if connect is True: Mark = 'g-o'
        else: Mark = 'bo'
        self.axes_manager.init_axes(RAlocator, DEClocator,
                                    RAformatter, DECformatter,
                                    RArotation, DECrotation,
                                    Aspect,
                                    xlim=(xmin,xmax),
                                    ylim=(ymin,ymax))
        a = self.axes_manager.axes
        if ObsPattern == False:
            a.title.set_text('Telescope Pointing on the Sky')
        else:
            a.title.set_text('Telescope Pointing on the Sky\nPointing Pattern = %s' % ObsPattern)
        plot_objects = []
        plot_objects.extend(
            pl.plot(RA, DEC, Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
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
        pl.savefig(plotfile, format='png', dpi=common.DPISummary)

        for obj in plot_objects:
            obj.remove()
        
        return
