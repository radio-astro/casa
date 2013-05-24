from __future__ import absolute_import
import os

import pylab as PL
import numpy

from asap.scantable import is_ms

import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger
from . import common


class SDWeatherDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8907
    WEATHER_KEYS = ['TEMPERATURE', 'PRESSURE', 'REL_HUMIDITY', 'WIND_SPEED']
        
    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        spws = parent_ms.get_spectral_windows(science_windows_only=True)
        spwid = spws[0].id
        rows = self.datatable.get_row_index(idx, spwid, 0)
        plotfile = os.path.join(stage_dir, 'weather_%s.png'%(st.basename))
        weather_dict = self.get_weather(idx)
        self.draw_weather(weather_dict, rows, idx, plotfile=plotfile)
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'sd'
        parameters['file'] = st.basename
        plot = logger.Plot(plotfile,
          x_axis='Time', y_axis='Weather',
          field=parent_ms.fields[0].name,
          parameters=parameters)
        return plot

    def draw_weather(self, WeatherDic, rows, vAnt, plotfile=False):
        """
        Plot Weather information and Tsys v.s. Time
        Table: DataTable
        """
        datatable = self.datatable
        
        if common.ShowPlot == False and plotfile == False: return

        # Plotting routine
        if common.ShowPlot: PL.ion()
        else: PL.ioff()
        Fig = PL.figure(self.MATPLOTLIB_FIGURE_ID)
        if common.ShowPlot: PL.ioff()
        PL.clf()

        # get Weather info from the table
        #print WeatherDic['TIME']
        #print WeatherDic['TEMPERATURE']
        #print WeatherDic['PRESSURE']
        #print WeatherDic['REL_HUMIDITY']
        #print WeatherDic['WIND_SPEED']

        # Extract MJD and Tsys
        MJD = numpy.take(datatable.getcol('TIME'),rows)
        #Tsys = numpy.take(datatable.getcol('TSYS'),rows)

        # Convert MJD sec to MJD date for WeatherDic
        # K -> degC
        for i in range(len(WeatherDic['TIME'])):
            WeatherDic['TIME'][i] = WeatherDic['TIME'][i]/3600./24.
            WeatherDic['TEMPERATURE'][i] = WeatherDic['TEMPERATURE'][i] - 273.16

        MJDmin = numpy.array(MJD).min()
        MJDmax = numpy.array(MJD).max()
        #Tsysmax = numpy.array(Tsys).max()
        #if Tsysmax > 0:
        #    for i in range(len(Tsys)):
        #        Tsys[i] = Tsys[i] * 100.0 / Tsysmax
        #else:
        #    for i in range(len(Tsys)):
        #        Tsys[i] = 0.0
        Tempmin = WeatherDic['TEMPERATURE'].min() - 3.0
        Tempmax = WeatherDic['TEMPERATURE'].max() + 2.0
        dTemp = Tempmax - Tempmin
        Humimin = 0.0
        Humimax = min(WeatherDic['REL_HUMIDITY'].max() + 5.0, 100.0)
        Presmin = WeatherDic['PRESSURE'].min() - 2.0
        Presmax = WeatherDic['PRESSURE'].max() + 1.0
        dPres = Presmax - Presmin
        Windmin = 0.0
        Windmax = WeatherDic['WIND_SPEED'].max() + 5
        #dTsys = max(Tsys) - min(Tsys)
        #if dTsys < 0.1:
        #    (Tsysmin, Tsysmax) = (min(Tsys) - 1.0, max(Tsys) + 1.0)
        #else:
        #    (Tsysmin, Tsysmax) = (min(Tsys) - dTsys*0.1, max(Tsys) + dTsys*0.1)
        #dTsys = Tsysmax - Tsysmin
        # Scale Tsys to fit in the Weather plotting window
        # Overplot on the Temperature plot and Pressure plot
        #TsysT = Tsys[:]
        #TsysP = Tsys[:]
        #for i in range(len(TsysT)):
        #    TsysT[i] = Tempmin + dTemp * (Tsys[i] - Tsysmin) / dTsys
        #    TsysP[i] = Presmin + dPres * (Tsys[i] - Tsysmin) / dTsys
        
        # Plot Temperature (degC)
        Ax1 = Fig.add_subplot(211)
        #Ax1.plot(MJD, TsysT, 'go', markersize=3, markeredgecolor='g', markerfacecolor='g')
        if len(WeatherDic['TIME']) == 1:
            Ax1.axhline(y = WeatherDic['TEMPERATURE'][0])
        else:
            Ax1.plot(WeatherDic['TIME'], WeatherDic['TEMPERATURE'], 'r-')
        Ax1.axis([MJDmin, MJDmax, Tempmin, Tempmax])
        Ax1.set_xlabel('MJD')
        Ax1.set_ylabel('Temperature (degC)', color='r')
        Ax1.set_title('Weather (Temperature & Humidity) and Tsys(green) versus MJD')
        for tl in Ax1.get_yticklabels():
            tl.set_color('r')

        # Plot Humidity (%)
        Ax2 = Ax1.twinx()
        if len(WeatherDic['TIME']) == 1:
            Ax2.axhline(y = WeatherDic['REL_HUMIDITY'][0])
        else:
            Ax2.plot(WeatherDic['TIME'], WeatherDic['REL_HUMIDITY'], 'b-')
        Ax2.axis([MJDmin, MJDmax, Humimin, Humimax])
        Ax2.set_ylabel('Humidity (%)', color='b')
        for tl in Ax2.get_yticklabels():
            tl.set_color('b')

        # Plot Pressure (hPa)
        Ax1 = Fig.add_subplot(212)
        #Ax1.plot(MJD, TsysP, 'go', markersize=3, markeredgecolor='g', markerfacecolor='g')
        if len(WeatherDic['TIME']) == 1:
            Ax1.axhline(y = WeatherDic['PRESSURE'][0])
        else:
            Ax1.plot(WeatherDic['TIME'], WeatherDic['PRESSURE'], 'r-')
        Ax1.axis([MJDmin, MJDmax, Presmin, Presmax])
        Ax1.set_xlabel('MJD')
        Ax1.set_ylabel('Pressure (hPa)', color='r')
        Ax1.set_title('Weather (Pressure & Wind Speed) and Tsys(green) versus MJD')
        for tl in Ax1.get_yticklabels():
            tl.set_color('r')

        # Plot Wind speed (m/s)
        Ax2 = Ax1.twinx()
        if len(WeatherDic['TIME']) == 1:
            Ax2.axhline(y = WeatherDic['WIND_SPEED'][0])
        else:
            Ax2.plot(WeatherDic['TIME'], WeatherDic['WIND_SPEED'], 'b-')
        Ax2.axis([MJDmin, MJDmax, Windmin, Windmax])
        Ax2.set_ylabel('Wind Speed (m/s)', color='b')
        for tl in Ax2.get_yticklabels():
            tl.set_color('b')

        if common.ShowPlot != False: PL.draw()
        if plotfile != False: PL.savefig(plotfile, format='png', dpi=common.DPISummary)

        del MJD
        return

    def get_weather(self, idx):
        """
        Return weather information for current antenna.
        Types of weather information should be specified as a string list.

        Example:
            self.get_weather( ['TEMPERATURE','PRESSURE'] )

        Possible weather type:
            TEMPERATURE
            PRESSURE
            REL_HUMIDITY
            WIND_DIRECTION
            WIND_SPEED

        Return:
            Dictionary that contains TIME and weather informations.
            List elements given as argument will be keys for weather info.
            {'TIME': [...],
             '<weather[0]>': [...],
             '<weather[1]>': [...],
             ...}
        """
        st = self.context.observing_run[idx]
        filename = st.ms.name

        if is_ms(filename):
            return self._get_weather_from_ms(filename, st.antenna.name)
        else:
            return self._get_weather_from_scantable(st.name)

    def _get_weather_from_ms(self, filename, antenna):
        weather = self.WEATHER_KEYS

        # read MS
        #atable = self._tb.getkeyword('ANTENNA').split()[-1]
        atable = os.path.join(filename, 'ANTENNA')
        wtable = os.path.join(filename, 'WEATHER')
        winfo = {}
        
        colnames = [col.upper() for col in weather]

        # get antenna position
        with utils.open_table(atable) as tb:
            antennaId = tb.getcol('NAME').tolist().index(antenna)
            antpos = tb.getcell( 'POSITION', antennaId )

        # get weather
        with utils.open_table(wtable) as tb:
            antid = tb.getcol( 'ANTENNA_ID' )
            if antennaId in antid:
                # get rows with ANTENNA_ID == self.iterAnt
                tbsel = tb.query( 'ANTENNA_ID == %s'%antennaId )
                for i in xrange(len(colnames)):
                    winfo[weather[i]] = tbsel.getcol( colnames[i] )
                tbsel.close()
                del tbsel
            else:
                # get rows for nearest weather station
                wxids = numpy.unique( tb.getcol( 'NS_WX_STATION_ID' ) )
                wxpos = []
                for id in wxids:
                    tbsel = tb.query( 'NS_WX_STATION_ID == %s'%id )
                    wxpos.append( tbsel.getcell( 'NS_WX_STATION_POSITION', 0 ) )
                    tbsel.close()
                    del tbsel
                sep = None
                nearest = -1
                for id in xrange(len(wxids)):
                    dist = 0.0
                    dpos = numpy.array(antpos) - numpy.array(wxpos[id])
                    dist = numpy.sqrt((numpy.square(dpos)).sum())
                    #self.LogMessage( 'DEBUG', Origin=origin, Msg='wxid[%s]=%s: dist = %s'%(id,wxids[id],dist) )
                    if sep == None or dist < sep:
                        sep = dist
                        nearest = wxids[id]
                #self.LogMessage( 'DEBUG', Origin=origin, Msg='nearest = %s'%(nearest) )
                tbsel = tb.query( 'NS_WX_STATION_ID == %s'%nearest )
                winfo['TIME'] = tbsel.getcol( 'TIME' )
                for iwed in xrange(len(weather)):
                    winfo[weather[iwed]] = tbsel.getcol( colnames[iwed] )
                tbsel.close()
                del tbsel
   
        return winfo

    def _get_weather_from_scantable(self, filename):
        weather = self.WEATHER_KEYS

        wtable = os.path.join(filename,'WEATHER')

        colnames = []
        winfo = {}

        # read Scantable
        for col in weather:
            if col.upper() == 'REL_HUMIDITY':
                colnames.append( 'HUMIDITY' )
            elif col.upper() == 'WIND_SPEED':
                colnames.append( 'WINDSPEED' )
            elif col.upper() == 'WIND_DIRECTION':
                colnames.append( 'WINDAZ' )
            else:
                colnames.append( col.upper() )

        # first read WEATHER subtable
        with utils.open_table(wtable) as tb:
            tmpwed = {}
            wkeys = tb.getcol( 'ID' )
            for col in colnames:
                tmpwed[col] = {}
                wed = tb.getcol( col )
                for k in xrange(len(wkeys)):
                    tmpwed[col][wkeys[k]] = wed[k]

        # then match up with MAIN rows
        with utils.open_table(filename) as tb:
            # should be converted to 'sec' since TIME is stored as 'day' 
            winfo['TIME'] = tb.getcol( 'TIME' ) * 86400.0
            wid = tb.getcol( 'WEATHER_ID' )
            for icol in xrange(len(weather)):
                wlist = []
                for jcol in xrange(tb.nrows()):
                    wlist.append( tmpwed[colnames[icol]][wid[jcol]] )
                winfo[weather[icol]] = numpy.array( wlist )

        return winfo
