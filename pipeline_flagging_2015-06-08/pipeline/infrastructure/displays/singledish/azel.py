from __future__ import absolute_import

import os
import pylab as pl
import numpy
from matplotlib.ticker import NullFormatter, MultipleLocator

from . import common
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from pipeline.domain.datatable import OnlineFlagIndex

from . import utils
from . import common

LOG = infrastructure.get_logger(__name__)

class AzElAxesManager(common.TimeAxesManager):
    def __init__(self):
        super(AzElAxesManager,self).__init__()
        self._az = None
        self._el = None
        
    @property
    def elevation_axes(self):
        if self._el is None:
            self._el = self.__el()
        return self._el

    @property
    def azimuth_axes(self):
        if self._az is None:
            self._az = self.__az()
        return self._az

    def __az(self):
        #a = pl.axes([0.1, 0.1, 0.8, 0.35])
        a = pl.axes([0.1, 0.15, 0.8, 0.38])
        pl.ylabel('Azimuth (deg)')
        #pl.title('Azimuth Plot v.s. Time (with Detected large Gaps)')
        pl.xlabel('Time (UT)')
        a.xaxis.set_major_locator(self.locator)
        a.xaxis.set_major_formatter(utils.utc_formatter())
        return a

    def __el(self):
        #a = pl.axes([0.1, 0.55, 0.8, 0.35])
        a = pl.axes([0.1, 0.53, 0.8, 0.38])
        pl.ylabel('Elevation (deg)')
        #pl.title('Elevation Plot v.s. Time (with Detected large Gaps)')
        pl.title('Elevation/Azimuth Plot v.s. Time (with Detected large Gaps)')
        #pl.xlabel('Time (UT)')
        a.xaxis.set_major_locator(self.locator)
        #a.xaxis.set_major_formatter(utils.utc_formatter())
        a.xaxis.set_major_formatter(NullFormatter())
        return a
            
        
        
class SDAzElDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8906
    AxesManager = AzElAxesManager

    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        vis = parent_ms.basename
        # target_spws = [spwid for (spwid, spwobj) in st.spectral_window.items()
        #                if spwobj.is_target and spwobj.nchan != 4]
        target_spws = self.context.observing_run.get_spw_for_science(st.basename)
        spwid = target_spws[0]
        rows = self.datatable.get_row_index(idx, spwid, 0)
        timegap = self.datatable.get_timegap(idx, spwid, 0, asrow=False)
        plotfile = os.path.join(stage_dir, 'azel_%s.png'%(st.basename))
        self.draw_azel(timegap, rows, plotfile, plotpolicy='greyed')
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'sd'
        parameters['file'] = st.basename
        parameters['vis'] = vis
        plot = logger.Plot(plotfile,
          x_axis='Time', y_axis='Azimuth/Elevation',
          field=parent_ms.fields[0].name,
          parameters=parameters)
        return plot

    def draw_azel(self, TimeGapList, rows, plotfile, plotpolicy='ignore'):
        """
        Plot Az El v.s. Time
        Table: DataTable
        TimeGapList: [[rowX1, rowX2,...,rowXN],[rowX1, rowX2,...,rowXN]]
        plotpolicy: plot policy for flagged data
                    'ignore' -- ignore flagged data
                    'plot' -- plot flagged data with same color as unflagged
                    'greyed' -- plot flagged data with grey color
        """
        qa = casatools.quanta
        datatable = self.datatable
        
        # Select large time gap (equivalent to one set of raster scan)
        TimeGap = TimeGapList[1]

        # if DoStack is true plot will be stacked with different dates.
        ###DoStack = True
        DoStack = False
        
        # Extract Az, El, and MJD
        AzArr = []
        ElArr = []
        MJDArr = []
        TGap = []
        PGap = []
        TmpArr = []
        FlagArr = []

        tTIME = datatable.tb1.getcol('TIME')
        Az = datatable.tb1.getcol('AZ').take(rows)
        El = datatable.tb1.getcol('EL').take(rows)
        Flag = datatable.tb2.getcol('FLAG_PERMANENT').take(rows, axis=1)[OnlineFlagIndex]

        for gap in TimeGap:
            if gap > rows[-1]: break
            if gap == 0: continue
            TGap.append((tTIME[gap - 1] + tTIME[gap]) / 2.)
        for gap in TimeGap:
            if gap > rows[-1]: break
            if gap == 0: continue
            PGap.append((tTIME[gap - 1] + tTIME[gap]) / 2.)
        TGapTmp = utils.mjd_to_plotval(numpy.array(TGap)) if len(TGap) > 0 \
                  else []
        PGapTmp = utils.mjd_to_plotval(numpy.array(PGap)) if len(PGap) > 0 \
                  else []
        MJD = tTIME.take(rows)
        time_for_plot = utils.mjd_to_plotval(MJD)
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
                    MJDArr.append([MJD[0]])
                    AzArr.append([Az[0]])
                    ElArr.append([El[0]])
                    TmpArr.append([time_for_plot[0]])
                    FlagArr.append([Flag[0]])
                else:
                    delt = int(MJD[n]) - int(MJD[n-1])
                    if delt >= 1:
                        ndays += 1
                        MJDArr.append([])
                        AzArr.append([])
                        ElArr.append([])
                        TmpArr.append([])
                        FlagArr.append([])

                    MJDArr[ndays-1].append(MJD[n])
                    AzArr[ndays-1].append(Az[n])
                    ElArr[ndays-1].append(El[n])
                    TmpArr[ndays-1].append(time_for_plot[n])
                    FlagArr[ndays-1].append(Flag[n])

        # Plotting routine
        UTmin = time_for_plot.min()
        UTmax = time_for_plot.max()
        Extend = (UTmax - UTmin) * 0.05
        UTmin -= Extend
        UTmax += Extend
        
        self.axes_manager.init(UTmin, UTmax)

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
                UTdata = TmpArr[nd]

                #date = qa.quantity(MJDArr[nd][0],'d')
                date = qa.quantity(str(MJDArr[nd][0])+'d')
                (datelab,rest) = qa.time(date,form='dmy')[0].split('/')  

                if plotpolicy == 'plot':
                    plot_objects.extend(
                        pl.plot(UTdata, ElArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                        )
                elif plotpolicy == 'ignore':
                    NpElArr = numpy.array(ElArr[nd])
                    NpUTdata = numpy.array(UTdata)
                    NpFlagArr = numpy.array(FlagArr[nd]) 
                    filter = NpFlagArr == 1
                    plot_objects.extend(
                        pl.plot(NpUTdata[filter], NpElArr[filter], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                        )
                elif plotpolicy == 'greyed':
                    NpElArr = numpy.array(ElArr[nd])
                    NpUTdata = numpy.array(UTdata)
                    NpFlagArr = numpy.array(FlagArr[nd]) 
                    filter = NpFlagArr == 1
                    plot_objects.extend(
                        pl.plot(NpUTdata[filter], NpElArr[filter], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                        )
                    filter = NpFlagArr == 0
                    if numpy.any(filter == True):
                        plot_objects.extend(
                            pl.plot(NpUTdata[filter], NpElArr[filter], markers[nd], markersize=2, markeredgecolor='grey', markerfacecolor='grey')
                            )
                    
                pl.legend(prop={'size': 'smaller'},markerscale=1.0,numpoints=1)
                for Time in TGap:
                    if int(Time) == int(MJDArr[nd][0]):
                        modTime = (Time - int(Time))*24
                        plot_objects.append(
                            pl.axvline(x=modTime, linewidth=0.5, color='c',label='_nolegend_')
                            )
            if ELmin < 0: pl.axis([UTmin, UTmax, -90, 90])
            else: pl.axis([UTmin, UTmax, 0, 90])

            pl.gcf().sca(self.axes_manager.azimuth_axes)
            for nd in range(ndays):
                UTdata = TmpArr[nd]
                date = qa.quantity(str(MJDArr[nd][0])+'d')
                (datelab,rest) = qa.time(date,form='dmy')[0].split('/')  
                
                if plotpolicy == 'plot':
                    plot_objects.extend(
                        pl.plot(UTdata, AzArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                        )
                elif plotpolicy == 'ignore':
                    NpAzArr = numpy.array(AzArr[nd])
                    NpUTdata = numpy.array(UTdata)
                    NpFlagArr = numpy.array(FlagArr[nd]) 
                    filter = NpFlagArr == 1
                    plot_objects.extend(
                        pl.plot(NpUTdata[filter], NpAzArr[filter], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                        )
                elif plotpolicy == 'greyed':
                    NpAzArr = numpy.array(AzArr[nd])
                    NpUTdata = numpy.array(UTdata)
                    NpFlagArr = numpy.array(FlagArr[nd]) 
                    filter = NpFlagArr == 1
                    plot_objects.extend(
                        pl.plot(NpUTdata[filter], NpAzArr[filter], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                        )
                    filter = NpFlagArr == 0
                    if numpy.any(filter == True):
                        plot_objects.extend(
                            pl.plot(NpUTdata[filter], NpAzArr[filter], markers[nd], markersize=2, markeredgecolor='grey', markerfacecolor='grey')
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
            UTdata = time_for_plot
            self.axes_manager.init(UTmin, UTmax)
            pl.gcf().sca(self.axes_manager.elevation_axes)
            for Time in TGapTmp:
                plot_objects.append(
                    pl.axvline(x=Time, linewidth=0.5, color='c')
                    )
                
            if plotpolicy == 'plot':
                plot_objects.extend(
                    pl.plot(UTdata, El, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
                    )
            elif plotpolicy == 'ignore':
                filter = Flag == 1
                plot_objects.extend(
                    pl.plot(UTdata[filter], El[filter], 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b' )
                    )
            elif plotpolicy == 'greyed':
                filter = Flag == 1
                plot_objects.extend(
                    pl.plot(UTdata[filter], El[filter], 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b' )
                    )
                filter = Flag == 0
                if numpy.any(filter == True):
                    plot_objects.extend(
                        pl.plot(UTdata[filter], El[filter], 'o', markersize=2, markeredgecolor='grey', markerfacecolor='grey' )
                        )
                
            if ELmin < 0:
                pl.axis([UTmin, UTmax, -90, 90])
            else:
                pl.axis([UTmin, UTmax, 0, 90])

            pl.gcf().sca(self.axes_manager.azimuth_axes)
            for Time in PGapTmp:
                plot_objects.append(
                    pl.axvline(x=Time, linewidth=0.5, color='g')
                    )
                
            if plotpolicy == 'plot':
                plot_objects.extend(
                    pl.plot(UTdata, Az, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
                    )
            elif plotpolicy == 'ignore':
                filter = Flag == 1
                plot_objects.extend(
                    pl.plot(UTdata[filter], Az[filter], 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
                    )
            elif plotpolicy == 'greyed':
                filter = Flag == 1
                plot_objects.extend(
                    pl.plot(UTdata[filter], Az[filter], 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b' )
                    )
                filter = Flag == 0
                if numpy.any(filter == True):
                    plot_objects.extend(
                        pl.plot(UTdata[filter], Az[filter], 'o', markersize=2, markeredgecolor='grey', markerfacecolor='grey' )
                        )
                
            pl.axis([UTmin, UTmax, 0, 380])

        if common.ShowPlot != False: pl.draw()
        pl.savefig(plotfile, format='png', dpi=common.DPISummary)

        for obj in plot_objects:
            obj.remove()
        
        return
