from __future__ import absolute_import
import os

import pylab as pl
import numpy

from matplotlib.ticker import NullFormatter

from asap.scantable import is_ms

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from . import common
from . import utils

class WeatherAxesManager(common.TimeAxesManager):
    def __init__(self):
        super(WeatherAxesManager,self).__init__()
        self._temp = None
        self._humi = None
        self._pres = None
        self._wind = None

    @property
    def axes_temperature(self):
        if self._temp is None:
            self._temp = self.__temp()
        return self._temp

    @property
    def axes_humidity(self):
        if self._humi is None:
            self._humi = self.__humi()
        return self._humi

    @property
    def axes_pressure(self):
        if self._pres is None:
            self._pres = self.__pres()
        return self._pres

    @property
    def axes_wind(self):
        if self._wind is None:
            self._wind = self.__wind()
        return self._wind

    def __temp(self):
        a = pl.subplot(211)
        #a.set_xlabel('MJD')
        #a.set_xlabel('Time (UT)')
        a.set_ylabel('Temperature (degC)', color='r')
        #a.set_title('Weather (Temperature & Humidity) versus MJD')
        #a.set_title('Weather (Temperature & Humidity) versus Time (UT)')
        a.set_title('Weather versus Time\nTop: Temperature & Humidity\nBottom: Pressure & Wind Speed')
        a.xaxis.set_major_locator(self.locator)
        #a.xaxis.set_major_formatter(utils.utc_formatter())
        a.xaxis.set_major_formatter(NullFormatter())
        for tl in a.get_yticklabels():
            tl.set_color('r')

        pos = a.get_position()
        left = pos.x0
        bottom = pos.y0 - 0.05
        width = pos.x1 - pos.x0
        height = pos.y1 - pos.y0
        a.set_position([left, bottom, width, height])

        return a

    def __humi(self):
        if self._temp is None:
            self._temp = self.__temp()

        a = self._temp.twinx()
        a.set_ylabel('Humidity (%)', color='b')
        a.xaxis.set_major_locator(self.locator)
        #a.xaxis.set_major_formatter(utils.utc_formatter())
        a.xaxis.set_major_formatter(NullFormatter())
        for tl in a.get_yticklabels():
            tl.set_color('b')
        return a

    def __pres(self):
        a = pl.subplot(212)
        #a.set_xlabel('MJD')
        a.set_xlabel('Time (UT)')
        a.set_ylabel('Pressure (hPa)', color='r')
        #a.set_title('Weather (Pressure & Wind Speed) versus MJD')
        #a.set_title('Weather (Pressure & Wind Speed) versus Time (UT)')
        a.xaxis.set_major_locator(self.locator)
        a.xaxis.set_major_formatter(utils.utc_formatter())
        for tl in a.get_yticklabels():
            tl.set_color('r')

        pos = a.get_position()
        left = pos.x0
        bottom = pos.y0 + 0.025
        width = pos.x1 - pos.x0
        height = pos.y1 - pos.y0
        a.set_position([left, bottom, width, height])

        return a

    def __wind(self):
        if self._pres is None:
            self._pres = self.__pres()

        a = self._pres.twinx()
        a.set_ylabel('Wind Speed (m/s)', color='b')
        a.xaxis.set_major_locator(self.locator)
        a.xaxis.set_major_formatter(utils.utc_formatter())
        for tl in a.get_yticklabels():
            tl.set_color('b')
        return a
        

class SDWeatherDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8907
    AxesManager = WeatherAxesManager
    WEATHER_KEYS = ['TEMPERATURE', 'PRESSURE', 'REL_HUMIDITY', 'WIND_SPEED']
        
    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        vis = parent_ms.basename
        spws = parent_ms.get_spectral_windows(science_windows_only=True)
        spwid = spws[0].id
        rows = self.datatable.get_row_index(idx, spwid, 0)
        plotfile = os.path.join(stage_dir, 'weather_%s.png'%(st.basename))
        weather_dict = self.get_weather(idx)
        self.draw_weather(weather_dict, rows, idx, plotfile)
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'sd'
        parameters['file'] = st.basename
        parameters['vis'] = vis
        plot = logger.Plot(plotfile,
          x_axis='Time', y_axis='Weather',
          field=parent_ms.fields[0].name,
          parameters=parameters)
        return plot

    def draw_weather(self, WeatherDic, rows, vAnt, plotfile):
        """
        Plot Weather information v.s. Time
        Table: DataTable
        """
        datatable = self.datatable
        
        # Plotting routine
        Fig = pl.gcf()

        # get Weather info from the table
        #print WeatherDic['TIME']
        #print WeatherDic['TEMPERATURE']
        #print WeatherDic['PRESSURE']
        #print WeatherDic['REL_HUMIDITY']
        #print WeatherDic['WIND_SPEED']

        # Convert MJD sec to MJD date for WeatherDic
        # K -> degC
        for i in range(len(WeatherDic['TIME'])):
            WeatherDic['TIME'][i] = WeatherDic['TIME'][i]/3600./24.
            WeatherDic['TEMPERATURE'][i] = WeatherDic['TEMPERATURE'][i] - 273.16

        MJD = WeatherDic['TIME']
        time_for_plot = utils.mjd_to_plotval(MJD)

        MJDmin = time_for_plot.min()
        MJDmax = time_for_plot.max()
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

        plot_objects = []

        self.axes_manager.init(MJDmin, MJDmax)
        
        # Plot Temperature (degC)
        Ax1 = self.axes_manager.axes_temperature
        if len(WeatherDic['TIME']) == 1:
            plot_objects.append(
                Ax1.axhline(y = WeatherDic['TEMPERATURE'][0])
                )
        else:
            plot_objects.extend(
                Ax1.plot(time_for_plot, WeatherDic['TEMPERATURE'], 'r-')
                )
        Ax1.axis([MJDmin, MJDmax, Tempmin, Tempmax])

        # Plot Humidity (%)
        Ax2 = self.axes_manager.axes_humidity
        if len(WeatherDic['TIME']) == 1:
            plot_objects.append(
                Ax2.axhline(y = WeatherDic['REL_HUMIDITY'][0])
                )
        else:
            plot_objects.extend(
                Ax2.plot(time_for_plot, WeatherDic['REL_HUMIDITY'], 'b-')
                )
        Ax2.axis([MJDmin, MJDmax, Humimin, Humimax])

        # Plot Pressure (hPa)
        Ax1 = self.axes_manager.axes_pressure
        if len(WeatherDic['TIME']) == 1:
            plot_objects.append(
                Ax1.axhline(y = WeatherDic['PRESSURE'][0])
                )
        else:
            plot_objects.extend(
                Ax1.plot(time_for_plot, WeatherDic['PRESSURE'], 'r-')
                )
        Ax1.axis([MJDmin, MJDmax, Presmin, Presmax])

        # Plot Wind speed (m/s)
        Ax2 = self.axes_manager.axes_wind
        if len(WeatherDic['TIME']) == 1:
            plot_objects.append(
                Ax2.axhline(y = WeatherDic['WIND_SPEED'][0])
                )
        else:
            plot_objects.extend(
                Ax2.plot(time_for_plot, WeatherDic['WIND_SPEED'], 'b-')
                )
        Ax2.axis([MJDmin, MJDmax, Windmin, Windmax])

        if common.ShowPlot: pl.draw()
        pl.savefig(plotfile, format='png', dpi=common.DPISummary)

        for obj in plot_objects:
            obj.remove()
            
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
        with casatools.TableReader(atable) as tb:
            antennaId = tb.getcol('NAME').tolist().index(antenna)
            antpos = tb.getcell( 'POSITION', antennaId )

        # get weather
        with casatools.TableReader(wtable) as tb:
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
        with casatools.TableReader(wtable) as tb:
            tmpwed = {}
            wkeys = tb.getcol( 'ID' )
            for col in colnames:
                tmpwed[col] = {}
                wed = tb.getcol( col )
                for k in xrange(len(wkeys)):
                    tmpwed[col][wkeys[k]] = wed[k]

        # then match up with MAIN rows
        with casatools.TableReader(filename) as tb:
            # should be converted to 'sec' since TIME is stored as 'day' 
            winfo['TIME'] = tb.getcol( 'TIME' ) * 86400.0
            wid = tb.getcol( 'WEATHER_ID' )
            for icol in xrange(len(weather)):
                wlist = []
                for jcol in xrange(tb.nrows()):
                    wlist.append( tmpwed[colnames[icol]][wid[jcol]] )
                winfo[weather[icol]] = numpy.array( wlist )

        return winfo
