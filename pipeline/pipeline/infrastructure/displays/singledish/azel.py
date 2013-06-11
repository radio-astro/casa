from __future__ import absolute_import

import os
import pylab as pl
import numpy

from . import common
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger

class AzElAxesManager(object):
    def __init__(self):
        self._el = None
        self._az = None
        
    @property
    def elevation_axes(self):
        if self._el is None:
            a = pl.axes([0.1, 0.55, 0.8, 0.35])
            pl.ylabel('Elevation (deg)')
            pl.title('Elevation Plot v.s. Time (with Detected large Gaps)')
            pl.xlabel('Time (UT)')
            self._el = a
            
        return self._el

    @property
    def azimuth_axes(self):
        if self._az is None:
            a = pl.axes([0.1, 0.1, 0.8, 0.35])
            pl.ylabel('Azimuth (deg)')
            pl.title('Azimuth Plot v.s. Time (with Detected large Gaps)')
            pl.xlabel('Time (UT)')
            self._az = a

        return self._az

class SDAzElDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8906
    AxesManager = AzElAxesManager

    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        spws = parent_ms.get_spectral_windows(science_windows_only=True)
        spwid = spws[0].id
        rows = self.datatable.get_row_index(idx, spwid, 0)
        timegap = self.datatable.get_timegap(idx, spwid, 0, asrow=False)
        plotfile = os.path.join(stage_dir, 'azel_%s.png'%(st.basename))
        self.draw_azel(timegap, rows, plotfile)
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'sd'
        parameters['file'] = st.basename
        plot = logger.Plot(plotfile,
          x_axis='Time', y_axis='Azimuth/Elevation',
          field=parent_ms.fields[0].name,
          parameters=parameters)
        return plot

    def draw_azel(self, TimeGapList, rows, plotfile):
        """
        Plot Az El v.s. Time
        Table: DataTable
        TimeGapList: [[rowX1, rowX2,...,rowXN],[rowX1, rowX2,...,rowXN]]
        """
        qa = casatools.quanta
        datatable = self.datatable
        
        # Select large time gap (equivalent to one set of raster scan)
        TimeGap = TimeGapList[1]

        # if DoStack is true plot will be stacked with different dates.
        DoStack = True
        # Extract Az, El, and MJD
        Az = []
        AzArr = []
        El = []
        ElArr = []
        MJD = []
        MJDArr = []
        TGap = []
        PGap = []

        tTIME = datatable.getcol('TIME')
        tAZ = datatable.getcol('AZ')
        tEL = datatable.getcol('EL')

        for gap in TimeGap:
            if gap > rows[-1]: break
            if gap == 0: continue
            TGap.append((tTIME[gap - 1] + tTIME[gap]) / 2.)
        for gap in TimeGap:
            if gap > rows[-1]: break
            if gap == 0: continue
            PGap.append((tTIME[gap - 1] + tTIME[gap]) / 2.)
        for row in rows:
            Az.append(tAZ[row])
            El.append(tEL[row])
            MJD.append(tTIME[row])
        MJDmin = numpy.array(MJD).min()
        MJDmax = numpy.array(MJD).max()
        Extend = (MJDmax - MJDmin) * 0.05
        MJDmin -= Extend
        MJDmax += Extend
        ELmin = min(El)
        if min(Az) < 0:
            for row in range(len(Az)):
                if Az[row] < 0: Az[row] += 360.0

        plot_objects = []

        if DoStack:
            # find number of days 
            ndays = 0
            for n in range(len(MJD)):
                if n == 0:
                    ndays +=1
                    MJDArr.append([])
                    AzArr.append([])
                    ElArr.append([])
                    MJDArr[0].append(MJD[0])
                    AzArr[0].append(Az[0])
                    ElArr[0].append(El[0])
                else:
                    delt = int(MJD[n]) - int(MJD[n-1])
                    if delt >= 1:
                        ndays += 1
                        MJDArr.append([])
                        AzArr.append([])
                        ElArr.append([])

                    MJDArr[ndays-1].append(MJD[n])
                    AzArr[ndays-1].append(Az[n])
                    ElArr[ndays-1].append(El[n])

        # Plotting routine
        #if common.ShowPlot: pl.ion()
        #else: pl.ioff()
        #pl.figure(self.MATPLOTLIB_FIGURE_ID)
        #if common.ShowPlot: pl.ioff()
        #pl.cla()
        #pl.clf()

        if DoStack:
            markercolorbase = ['b', 'm', 'y', 'k', 'r']
            m=numpy.ceil(ndays*1.0/len(markercolorbase))
            markercolors = markercolorbase*int(m)
            markerbase = ['o','x','^','s','v']
            markers = []
            for mrk in markerbase:
                for i in range(len(markercolorbase)):
                    if len(markers)<len(markercolors): 
                        markers.append(markercolors[i]+mrk)

            pl.gcf().sca(self.axes_manager.elevation_axes)
            for nd in range(ndays):
                UTdata = (numpy.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
                if nd == 0:
                    UTmin = min(UTdata)
                    UTmax = max(UTdata)
                else:
                    if min(UTdata) < UTmin: UTmin = min(UTdata)
                    if max(UTdata) > UTmax: UTmax = max(UTdata)

                #date = qa.quantity(MJDArr[nd][0],'d')
                date = qa.quantity(str(MJDArr[nd][0])+'d')
                (datelab,rest) = qa.time(date,form='dmy')[0].split('/')  
                #pl.plot(UTdata, ElArr[nd], 'bo', markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)

                plot_objects.extend(
                    pl.plot(UTdata, ElArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                    )
                pl.legend(prop={'size': 'smaller'},markerscale=1.0,numpoints=1)
                for Time in TGap:
                    if int(Time) == int(MJDArr[nd][0]):
                        modTime = (Time - int(Time))*24
                        plot_objects.append(
                            pl.axvline(x=modTime, linewidth=0.5, color='c',label='_nolegend_')
                            )
            Extend = (UTmax - UTmin) * 0.05
            UTmin -= Extend
            UTmax += Extend
            if ELmin < 0: pl.axis([UTmin, UTmax, -90, 90])
            else: pl.axis([UTmin, UTmax, 0, 90])

            pl.gcf().sca(self.axes_manager.azimuth_axes)
            for nd in range(ndays):
                UTdata = (numpy.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
                date = qa.quantity(str(MJDArr[nd][0])+'d')
                (datelab,rest) = qa.time(date,form='dmy')[0].split('/')  
                #pl.plot(UTdata, AzArr[nd], 'bo', markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                plot_objects.extend(
                    pl.plot(UTdata, AzArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                    )
                pl.legend(prop={'size': 'smaller'},markerscale=0.8,numpoints=1)
                for Time in PGap:
                    if int(Time) == int(MJDArr[nd][0]):
                        modTime = (Time - int(Time))*24
                        plot_objects.append(
                            pl.axvline(x=modTime, linewidth=0.5, color='g', label='_nolegend_')
                        )
            pl.axis([UTmin, UTmax, 0, 360])
        else:
            pl.gcf().sca(self.axes_manager.elevation_axes)
            for Time in TGap:
                plot_objects.append(
                    pl.axvline(x=Time, linewidth=0.5, color='c')
                    )
            plot_objects.extend(
                pl.plot(MJD, El, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
                )
            if ELmin < 0: pl.axis([MJDmin, MJDmax, -90, 90])
            else: pl.axis([MJDmin, MJDmax, 0, 90])

            pl.gcf().sca(self.axes_manager.azimuth.axes)
            for Time in PGap:
                plot_objects.append(
                    pl.axvline(x=Time, linewidth=0.5, color='g')
                    )
            plot_objects.extend(
                pl.plot(MJD, Az, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
                )
            pl.axis([MJDmin, MJDmax, 0, 360])

        if common.ShowPlot != False: pl.draw()
        pl.savefig(plotfile, format='png', dpi=common.DPISummary)

        for obj in plot_objects:
            obj.remove()
        
        return
