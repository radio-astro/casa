from __future__ import absolute_import

import os
import pylab as PL
import numpy
from matplotlib.font_manager import FontProperties 

from . import common
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger

class SDAzElDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8906

    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        spws = parent_ms.get_spectral_windows(science_windows_only=True)
        spwid = spws[0].id
        rows = self.datatable.get_row_index(idx, spwid, 0)
        timegap = self.datatable.get_timegap(idx, spwid, 0, asrow=False)
        plotfile = os.path.join(stage_dir, 'azel_%s.png'%(st.basename))
        self.draw_azel(timegap, rows, plotfile=plotfile)
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

    def draw_azel(self, TimeGapList, rows, plotfile=False):
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
        if common.ShowPlot == False and plotfile == False: return
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
        if common.ShowPlot: PL.ion()
        else: PL.ioff()
        PL.figure(self.MATPLOTLIB_FIGURE_ID)
        if common.ShowPlot: PL.ioff()
        PL.cla()
        PL.clf()

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

            PL.axes([0.1, 0.55, 0.8, 0.35])
            PL.ylabel('Elevation (deg)')
            PL.title('Elevation Plot v.s. Time (with Detected large Gaps)')
            PL.xlabel('Time (UT)')
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
                #PL.plot(UTdata, ElArr[nd], 'bo', markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)

                PL.plot(UTdata, ElArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                PL.legend(prop=FontProperties(size='smaller'),markerscale=1.0,numpoints=1)
                for Time in TGap:
                    if int(Time) == int(MJDArr[nd][0]):
                        modTime = (Time - int(Time))*24
                        PL.axvline(x=modTime, linewidth=0.5, color='c',label='_nolegend_')
            Extend = (UTmax - UTmin) * 0.05
            UTmin -= Extend
            UTmax += Extend
            if ELmin < 0: PL.axis([UTmin, UTmax, -90, 90])
            else: PL.axis([UTmin, UTmax, 0, 90])

            PL.axes([0.1, 0.1, 0.8, 0.35])
            PL.ylabel('Azimuth (deg)')
            PL.title('Azimuth Plot v.s. Time (with Detected large Gaps)')
            PL.xlabel('Time (UT)')
            for nd in range(ndays):
                UTdata = (numpy.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
                date = qa.quantity(str(MJDArr[nd][0])+'d')
                (datelab,rest) = qa.time(date,form='dmy')[0].split('/')  
                #PL.plot(UTdata, AzArr[nd], 'bo', markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                PL.plot(UTdata, AzArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                PL.legend(prop=FontProperties(size='smaller'),markerscale=0.8,numpoints=1)
                for Time in PGap:
                    if int(Time) == int(MJDArr[nd][0]):
                        modTime = (Time - int(Time))*24
                        PL.axvline(x=modTime, linewidth=0.5, color='g', label='_nolegend_')

            PL.axis([UTmin, UTmax, 0, 360])
        else:
            PL.axes([0.1, 0.55, 0.8, 0.35])
            PL.ylabel('Elevation (deg)')
            PL.title('Elevation Plot v.s. Time (with Detected large Gaps)')
            PL.xlabel('MJD (Day)')
            for Time in TGap:
                PL.axvline(x=Time, linewidth=0.5, color='c')
            PL.plot(MJD, El, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
            if ELmin < 0: PL.axis([MJDmin, MJDmax, -90, 90])
            else: PL.axis([MJDmin, MJDmax, 0, 90])

            PL.axes([0.1, 0.1, 0.8, 0.35])
            PL.ylabel('Azimuth (deg)')
            PL.title('Azimuth Plot v.s. Time (with Detected large Gaps)')
            PL.xlabel('MJD (Day)')
            for Time in PGap:
                PL.axvline(x=Time, linewidth=0.5, color='g')
            PL.plot(MJD, Az, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
            PL.axis([MJDmin, MJDmax, 0, 360])

        if common.ShowPlot != False: PL.draw()
        if plotfile != False: PL.savefig(plotfile, format='png', dpi=common.DPISummary)

        del MJD, Az, El, TGap, PGap
        return
