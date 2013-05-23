from __future__ import absolute_import

import os
import pylab as PL
import numpy
from matplotlib.font_manager import FontProperties

from asap.scantable import is_ms

import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger
from . import common

class SDWvrDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8907

    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        spws = self.context.observing_run.get_spw_for_wvr(st.basename)
        plotfile = os.path.join(stage_dir, 'wvr_%s.png'%(st.basename))
        wvr_data = self.get_wvr_data(st.name, spws)
        wvr_frequency = self.get_wvr_frequency(st, spws)
        if len(wvr_data) == 0:
            return 
        self.draw_wvr(wvr_data, wvr_frequency, plotfile=plotfile)
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

    def draw_wvr(self, wvr_data, wvr_frequency, show_plot=True, plotfile=False):
        
        if show_plot == False and plotfile == False: return

        # Plotting routine
        if show_plot: PL.ion()
        Fig = PL.figure(self.MATPLOTLIB_FIGURE_ID)
        if show_plot: PL.ioff()
        PL.clf()

        # Convert MJD sec to MJD date for wvr_data
        wvr_data[0] = wvr_data[0]/3600.0/24.0

        # Convert wvr_frequency in Hz to GHz
        wvr_frequency = wvr_frequency / 1.0e9

        # Plot WVR data
        Ax1 = Fig.add_subplot(111)
        if len(wvr_data[0]) == 1:
            Ax1.axhline(y = wvr_data[1][0], color='r', label='%.2fGHz'%wvr_frequency[0])
            Ax1.axhline(y = wvr_data[2][0], color='g', label='%.2fGHz'%wvr_frequency[1])
            Ax1.axhline(y = wvr_data[3][0], color='b', label='%.2fGHz'%wvr_frequency[2])
            Ax1.axhline(y = wvr_data[4][0], color='c', label='%.2fGHz'%wvr_frequency[3])
        else:
            Ax1.plot(wvr_data[0], wvr_data[1], 'ro', markersize=3, markeredgecolor='r', markerfacecolor='r', label='%.2fGHz'%wvr_frequency[0])
            Ax1.plot(wvr_data[0], wvr_data[2], 'go', markersize=3, markeredgecolor='g', markerfacecolor='g', label='%.2fGHz'%wvr_frequency[1])
            Ax1.plot(wvr_data[0], wvr_data[3], 'bo', markersize=3, markeredgecolor='b', markerfacecolor='b', label='%.2fGHz'%wvr_frequency[2])
            Ax1.plot(wvr_data[0], wvr_data[4], 'co', markersize=3, markeredgecolor='c', markerfacecolor='c', label='%.2fGHz'%wvr_frequency[3])
        Ax1.legend(loc=0,numpoints=1,prop=FontProperties(size='smaller'))
        Ax1.set_xlabel('MJD')
        Ax1.set_ylabel('WVR reading')
        Ax1.set_title('WVR reading versus MJD')

        if show_plot != False: PL.draw()
        if plotfile != False: PL.savefig(plotfile, format='png', dpi=common.DPISummary)
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
