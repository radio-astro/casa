from __future__ import absolute_import

import os
import pylab as pl
import numpy

from asap.scantable import is_ms

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger
from . import common

LOG = infrastructure.get_logger(__name__)

class WvrAxesManager(object):
    def __init__(self):
        self._axes = None

    @property
    def axes(self):
        if self._axes is None:
            self._axes = self.__axes()
        return self._axes

    def __axes(self):
        a = pl.subplot(111)
        a.set_xlabel('MJD')
        a.set_ylabel('WVR reading')
        a.set_title('WVR reading versus MJD')
        # draw dummy lines
        colors = ['r', 'g', 'b', 'c']
        for i in xrange(4):
            a.plot(0, 0, '%so'%(colors[i]),
                   markersize=3, markeredgecolor=colors[i],
                   markerfacecolor=colors[i]) 
        return a
        
class SDWvrDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8907
    AxesManager = WvrAxesManager

    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        spws = self.context.observing_run.get_spw_for_wvr(st.basename)
        plotfile = os.path.join(stage_dir, 'wvr_%s.png'%(st.basename))
        wvr_data = self.get_wvr_data(st.name, spws)
        wvr_frequency = self.get_wvr_frequency(st, spws)
        if len(wvr_data) == 0:
            return 
        self.draw_wvr(wvr_data, wvr_frequency, plotfile)
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spws[0]
        parameters['pol'] = 'I'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'sd'
        parameters['file'] = st.basename
        plot = logger.Plot(plotfile,
          x_axis='Time', y_axis='WVR Reading',
          field=parent_ms.fields[0].name,
          parameters=parameters)
        return plot

    def draw_wvr(self, wvr_data, wvr_frequency, plotfile):
        # Plotting routine
        #if common.ShowPlot: pl.ion()
        #else: pl.ioff()
        #Fig = pl.figure(self.MATPLOTLIB_FIGURE_ID)
        #if common.ShowPlot: pl.ioff()
        #pl.clf()
        Fig = pl.gcf()

        # Convert MJD sec to MJD date for wvr_data
        mjd = wvr_data[0]/3600.0/24.0

        # Convert wvr_frequency in Hz to GHz
        wvr_frequency = wvr_frequency * 1.0e-9

        wvr = wvr_data[1:,:]

        xmin = mjd.min()
        xmax = mjd.max()
        dx = (xmax - xmin) * 0.1
        xmin -= dx
        xmax += dx
        ymin = wvr.min()
        ymax = wvr.max()
        if ymin == ymax:
            dy = 0.06
        else:
            dy = 0.1 * (ymax - ymin)
        ymin -= dy
        ymax += dy
        
        # Plot WVR data
        Ax1 = self.axes_manager.axes
        lines = Ax1.get_lines()
        if len(wvr_data[0]) == 1:
            for i in xrange(4):
                lines[i].set_data([xmin,xmax],[wvr[i][0],wvr[i][0]])
        else:
            for i in xrange(4):
                lines[i].set_data(mjd,wvr[i])                
        Ax1.legend(['%.2fGHz'%(f) for f in wvr_frequency],
                   loc=0, numpoints=1, prop={'size': 'smaller'})
        Ax1.set_xlim(xmin, xmax)
        Ax1.set_ylim(ymin, ymax)

        if common.ShowPlot != False: pl.draw()
        pl.savefig(plotfile, format='png', dpi=common.DPISummary)

        return


    def get_wvr_frequency(self, st, spwids):
        if len(spwids) == 0:
            return []

        spwid = spwids[0]

        if is_ms(st.ms.name):
            # take frequency from MS
            table_name = os.path.join(st.ms.name, 'SPECTRAL_WINDOW')
            with utils.open_table(table_name) as tb:
                chanfreq = tb.getcell('CHAN_FREQ', spwid)
        else:
            # take frequency from Scantable
            table_name = os.path.join(st.name, 'FREQUENCIES')
            with utils.open_table(table_name) as tb:
                refpix = tb.getcell('REFPIX', spwid)
                refval = tb.getcell('REFVAL', spwid)
                increm = tb.getcell('INCREMENT', spwid)
            chanfreq = [refval + (i - refpix) * increm for i in xrange(4)]

        return chanfreq
                

    def get_wvr_data(self, name, spwids):
        if len(spwids) == 0:
            return []
        
        with utils.open_table(name) as tb:
            tsel = tb.query('IFNO IN %s'%(list(spwids)))
            timecol = tsel.getcol('TIME') * 86400.0 # Day -> Sec
            wvrdata = tsel.getcol('SPECTRA')
            tsel.close()
            timecol = timecol.reshape(1, timecol.shape[0])
            data = numpy.concatenate([timecol,wvrdata])

        return data
