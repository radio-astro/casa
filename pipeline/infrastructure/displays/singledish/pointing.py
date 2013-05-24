from __future__ import absolute_import

import os
import math
import pylab as pl
import numpy

from matplotlib.font_manager import FontProperties 

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from . import common
from .utils import RADEClabel, RArotation, DECrotation

class SDPointingDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8905

    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        spws = parent_ms.get_spectral_windows(science_windows_only=True)
        spwid = spws[0].id
        beam_size = casatools.quanta.convert(self.context.observing_run[idx].beam_size[spwid], 'deg')
        obs_pattern = st.pattern[spwid].values()[0]
        rows = self.datatable.get_row_index(idx, spwid)
        plotfile = os.path.join(stage_dir, 'pointing_%s.png'%(st.basename))
        self.draw_radec(rows, plotfile, circle=[0.5*beam_size['value']], ObsPattern=obs_pattern)
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'sd'
        parameters['file'] = st.basename
        plot = logger.Plot(plotfile,
          x_axis='R.A.', y_axis='Dec.',
          field=parent_ms.fields[0].name,
          parameters=parameters)
        return plot
        
    def draw_radec(self, rows, plotfile, connect=True, circle=[], ObsPattern=False):
        """
        Draw loci of the telescope pointing
        xaxis: extension header keyword for RA
        yaxis: extension header keyword for DEC
        connect: connect points if True
        """
        datatable = self.datatable

        ROW = datatable.getcol('ROW')
        tRA = datatable.getcol('RA')
        tDEC = datatable.getcol('DEC')
        tNCHAN = datatable.getcol('NCHAN')
        
        # Extract RA and DEC
        RA = []
        DEC = []
        for row in rows:
            if tNCHAN[row] > 1:
                RA.append(tRA[row])
                DEC.append(tDEC[row])
        span = max(max(RA) - min(RA), max(DEC) - min(DEC))
        xmax = min(RA) - span / 10.0
        xmin = max(RA) + span / 10.0
        ymax = max(DEC) + span / 10.0
        ymin = min(DEC) - span / 10.0
        (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

        # 2008/9/20 DEC Effect
        Aspect = 1.0 / math.cos(tDEC[0] / 180.0 * 3.141592653)

        # Plotting routine
        if common.ShowPlot: pl.ion()
        else: pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        if common.ShowPlot: pl.ioff()
        if connect is True: Mark = 'g-o'
        else: Mark = 'bo'
        pl.cla()
        pl.clf()
        a = pl.axes([0.15, 0.2, 0.7, 0.7])
        # 2008/9/20 DEC Effect
        a.set_aspect(Aspect)
        #a.set_aspect('equal')
        pl.xlabel('RA')
        pl.ylabel('Dec')
        if ObsPattern == False:
            pl.title('Telescope Pointing on the Sky')
        else:
            pl.title('Telescope Pointing on the Sky\nPointing Pattern = %s' % ObsPattern)
        pl.plot(RA, DEC, Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
        a.xaxis.set_major_formatter(RAformatter)
        a.yaxis.set_major_formatter(DECformatter)
        a.xaxis.set_major_locator(RAlocator)
        a.yaxis.set_major_locator(DEClocator)
        xlabels = a.get_xticklabels()
        pl.setp(xlabels, 'rotation', RArotation, fontsize=8)
        ylabels = a.get_yticklabels()
        pl.setp(ylabels, 'rotation', DECrotation, fontsize=8)
        # plot starting position with beam and end position 
        if len(circle) != 0:
            for R in circle:
                Mark = 'r-'
                x = []
                y = []
                for t in range(50):
                    # 2008/9/20 DEC Effect
                    x.append(RA[0] + R * math.sin(t * 0.13)  * Aspect)
                    #x.append(RA[0] + R * math.sin(t * 0.13))
                    y.append(DEC[0] + R * math.cos(t * 0.13))
                pl.plot(x, y, Mark)
                #Mark = 'm-'
                Mark = 'ro'
                x = []
                y = []
                x.append(RA[-1])
                y.append(DEC[-1])
                pl.plot(x, y, Mark, markersize=4, markeredgecolor='r', markerfacecolor='r')
        pl.axis([xmin, xmax, ymin, ymax])
        if common.ShowPlot != False: pl.draw()
        pl.savefig(plotfile, format='png', dpi=common.DPISummary)

        return
