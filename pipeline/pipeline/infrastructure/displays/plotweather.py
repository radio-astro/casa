import math
import numpy as np
import pylab as pb
import datetime
import matplotlib
from casac import casac

def plotweather(vis='', figfile='', station=[], help=False):
    """
    Compiles and plots the major weather parameters for the specified ms. 
    Station can be a single integer or integer string, or a list of two integers.
    The default empty list means to plot all data from up to 2 of the stations
    present in the data.  The default plot file name will be 'vis'.weather.png.
    """
    return(plotWeather(vis, figfile, station, help))

def plotWeather(vis='', figfile='', station=[], help=False):
    """
    Compiles and plots the major weather parameters for the specified ms.
    Station can be a single integer or integer string, or a list of two integers.
    The default empty list means to plot all data from up to 2 of the stations
    present in the data.  The default plot file name will be 'vis'.weather.png.
    """
    if (help):
        print "plotWeather(vis='', figfile='', station=[])"
        print "  Plots pressure, temperature, relative humidity, wind speed and direction."
        print "Station can be a single integer or integer string, or a list of two integers."
        print "The default empty list means to plot the data form up to 2 of the stations"
        print "present in the data.  The default plot file name will be 'vis'.weather.png."
        return

    myfontsize = 8

    try:
        mytb = casac.table()
        mytb.open("%s/WEATHER" % vis)
    except:
        print "Could not open WEATHER table.  Did you importasdm with asis='*'?"
        return

    available_cols = mytb.colnames()
    mjdsec = mytb.getcol('TIME')
    mjdsec1 = mjdsec
    vis = vis.split('/')[-1]
    pressure = mytb.getcol('PRESSURE')
    relativeHumidity = mytb.getcol('REL_HUMIDITY')
    temperature = mytb.getcol('TEMPERATURE')
    # Nobeyama does not have DEW_POINT and NS_WX_STATION_ID
    dewPoint = mytb.getcol('DEW_POINT') if 'DEW_POINT' in available_cols else None
    windDirection = (180 / math.pi) * mytb.getcol('WIND_DIRECTION')
    windSpeed = mytb.getcol('WIND_SPEED')
    stations = mytb.getcol('NS_WX_STATION_ID') if 'NS_WX_STATION_ID' in available_cols else []
    uniqueStations = np.unique(stations)

    if (station != []):
        if (type(station) == int):
            if (station not in uniqueStations):
                print "Station %d is not in the data.  Present are: "%station, uniqueStations
                return
            uniqueStations = [station]
        elif (type(station) == list):
            if (len(station) > 2):
                print "Only 2 stations can be overlaid."
                return
            if (station[0] not in uniqueStations):
                print "Station %d is not in the data.  Present are: "%station[0], uniqueStations
                return
            if (station[1] not in uniqueStations):
                print "Station %d is not in the data.  Present are: "%station[1], uniqueStations
                return
            uniqueStations = station
        elif (type(station) == str):
            if (station.isdigit()):
                if (int(station) not in uniqueStations):
                    print "Station %s is not in the data.  Present are: "%station, uniqueStations
                    return
                uniqueStations = [int(station)]
            else:
                print "Invalid station ID, it must be an integer, or list of integers."
                return
            
    if (len(uniqueStations) > 1):
        firstStationRows = np.where(stations == uniqueStations[0])[0]
        secondStationRows = np.where(stations == uniqueStations[1])[0]

        pressure2 = pressure[secondStationRows]
        relativeHumidity2 = relativeHumidity[secondStationRows]
        temperature2 = temperature[secondStationRows]
        dewPoint2 = dewPoint[secondStationRows] if dewPoint is not None else None
        windDirection2 = windDirection[secondStationRows]
        windSpeed2 = windSpeed[secondStationRows]
        mjdsec2 = mjdsec[secondStationRows]

        pressure = pressure[firstStationRows]
        relativeHumidity = relativeHumidity[firstStationRows]
        temperature = temperature[firstStationRows]
        dewPoint = dewPoint[firstStationRows] if dewPoint is not None else None
        windDirection = windDirection[firstStationRows]
        windSpeed = windSpeed[firstStationRows]
        mjdsec1 = mjdsec[firstStationRows]
        if (np.mean(temperature2) > 100):
            # convert to Celsius
            temperature2 -= 273.15        
        if (dewPoint2 is not None and np.mean(dewPoint2) > 100):
            dewPoint2 -= 273.15        
        
    if (np.mean(temperature) > 100):
        # convert to Celsius
        temperature -= 273.15        
    if (dewPoint is not None and np.mean(dewPoint) > 100):
        dewPoint -= 273.15        
    if (dewPoint is not None and np.mean(dewPoint) == 0):
        # assume it is not measured and use NOAA formula to compute from humidity:
        dewPoint = ComputeDewPointCFromRHAndTempC(relativeHumidity, temperature)
    if (np.mean(relativeHumidity) < 0.001):
        if dewPoint is None or np.count_nonzero(dewPoint) == 0:
            # dew point is all zero so it was not measured, so cap the rH at small non-zero value
            relativeHumidity = 0.001 * np.ones(len(relativeHumidity))
        else:
            print "Replacing zeros in relative humidity with value computed from dew point and temperature."
            dewPointWVP = computeWVP(dewPoint)
            ambientWVP = computeWVP(temperature)
            print "dWVP=%f, aWVP=%f" % (dewPointWVP[0],ambientWVP[0])
            relativeHumidity = 100*(dewPointWVP/ambientWVP)

    mytb.close()

    mysize = 'small'
    pb.clf()
    adesc = pb.subplot(321)
    myhspace = 0.25
    mywspace = 0.25
    markersize = 3
    pb.subplots_adjust(hspace=myhspace, wspace=mywspace)
    pb.title(vis)
    list_of_date_times = mjdSecondsListToDateTime(mjdsec1)
    timeplot = pb.date2num(list_of_date_times)
    pb.plot_date(timeplot, pressure, markersize=markersize)
    if (len(uniqueStations) > 1):
        pb.hold(True)
        list_of_date_times = mjdSecondsListToDateTime(mjdsec2)
        timeplot2 = pb.date2num(list_of_date_times)
        pb.plot_date(timeplot2, pressure2, markersize=markersize, color='r')
        
    resizeFonts(adesc,myfontsize)
    #pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
    #pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Pressure (mb)',size=mysize)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

    adesc = pb.subplot(322)
    pb.plot_date(timeplot, temperature, markersize=markersize)
    if (len(uniqueStations) > 1):
        pb.hold(True)
        list_of_date_times = mjdSecondsListToDateTime(mjdsec2)
        timeplot2 = pb.date2num(list_of_date_times)
        pb.plot_date(timeplot2, temperature2, markersize=markersize, color='r')
    resizeFonts(adesc,myfontsize)
    #pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
    #pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Temperature (C)',size=mysize)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')
    if (len(uniqueStations) > 1):
        pb.title('blue = station %d,  red = station %d'%(uniqueStations[0],uniqueStations[1]))
    elif (len(uniqueStations) > 0):
        pb.title('blue = station %d'%(uniqueStations[0]))

    adesc = pb.subplot(323)
#    pb.plot(mjdFraction,relativeHumidity)
    pb.plot_date(timeplot,relativeHumidity, markersize=markersize)
    if (len(uniqueStations) > 1):
        pb.hold(True)
        list_of_date_times = mjdSecondsListToDateTime(mjdsec2)
        timeplot2 = pb.date2num(list_of_date_times)
        pb.plot_date(timeplot2, relativeHumidity2, markersize=markersize, color='r')
    resizeFonts(adesc,myfontsize)
#    pb.xlabel('Universal Time (%s)'%utdatestring(mjdsec[0]),size=mysize)
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
    pb.ylabel('Relative Humidity (%)',size=mysize)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

    pid = 4
    if dewPoint is not None:
        adesc = pb.subplot(3,2,pid)
        pb.plot_date(timeplot,dewPoint, markersize=markersize)
        if (len(uniqueStations) > 1):
            pb.hold(True)
            list_of_date_times = mjdSecondsListToDateTime(mjdsec2)
            timeplot2 = pb.date2num(list_of_date_times)
            pb.plot_date(timeplot2, dewPoint2, markersize=markersize, color='r')
        resizeFonts(adesc,myfontsize)
#        pb.xlabel('Universal Time (%s)'%utdatestring(mjdsec[0]),size=mysize)
        pb.ylabel('Dew point (C)',size=mysize)
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
        RescaleXAxisTimeTicks(pb.xlim(), adesc)
        adesc.xaxis.grid(True,which='major')
        adesc.yaxis.grid(True,which='major')
        pid += 1

    adesc = pb.subplot(3,2,pid)
    pb.plot_date(timeplot, windSpeed, markersize=markersize)
    if (len(uniqueStations) > 1):
        pb.hold(True)
        list_of_date_times = mjdSecondsListToDateTime(mjdsec2)
        timeplot2 = pb.date2num(list_of_date_times)
        pb.plot_date(timeplot2, windSpeed2, markersize=markersize, color='r')
    resizeFonts(adesc,myfontsize)
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
    pb.xlabel('Universal Time (%s)'%utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Wind speed (m/s)',size=mysize)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')
    pid += 1

    adesc= pb.subplot(3,2,pid)
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
    pb.xlabel('Universal Time (%s)'%utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Wind direction (deg)',size=mysize)
#    pb.plot(mjdFraction,windDirection)
    pb.plot_date(timeplot,windDirection, markersize=markersize)
    if (len(uniqueStations) > 1):
        pb.hold(True)
        list_of_date_times = mjdSecondsListToDateTime(mjdsec2)
        timeplot2 = pb.date2num(list_of_date_times)
        pb.plot_date(timeplot2, windDirection2, markersize=markersize, color='r')
    resizeFonts(adesc,myfontsize)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')
    if (len(figfile) < 1):
        weatherFile = vis+'.weather.png'
    else:
        weatherFile = figfile
    pb.savefig(weatherFile)
    pb.draw()
    print "Wrote file = %s" % (weatherFile)

def mjdSecondsListToDateTime(mjdsecList):
    """
    Takes a list of mjd seconds and converts it to a list of datetime structures.
    """

    myqa = casac.quanta()
    myme = casac.measures()

    dt = []
    typelist = type(mjdsecList)
    if not (typelist == list or typelist == np.ndarray):
        mjdsecList = [mjdsecList]
    for mjdsec in mjdsecList:
        today = myme.epoch('utc','today')
        mjd = mjdsec / 86400.
        today['m0']['value'] =  mjd
        hhmmss = call_qa_time(today['m0'])
        date = myqa.splitdate(today['m0'])  # date is now a dict
        mydate = datetime.datetime.strptime('%d-%d-%d %d:%d:%d'%(date['monthday'],date['month'],date['year'],date['hour'],date['min'],date['sec']),'%d-%m-%Y %H:%M:%S')
        dt.append(mydate)
    myme.done()

    return(dt)

def mjdSecondsToMJDandUT(mjdsec):
    """
    Converts a value of MJD seconds into MJD, and into a UT date/time string.
    For example:  2011-01-04 13:10:04 UT
    Caveat: only works for a scalar input value
    """

    myme = casac.measures()
    myqa = casac.quanta()

    today = myme.epoch('utc','today')
    mjd = mjdsec / 86400.
    today['m0']['value'] =  mjd
    hhmmss = call_qa_time(today['m0'])
    date = myqa.splitdate(today['m0'])
    utstring = "%s-%02d-%02d %s UT" % (date['year'],date['month'],date['monthday'],hhmmss)
    myme.done()

    return(mjd, utstring)

def call_qa_time(arg, form='', prec=0):
    """
    This is a wrapper for qa.time(), which in casa 3.5 returns a list of strings instead
    of just a scalar string.  
    """
    myqa = casac.quanta()
    result = myqa.time(arg, form=form, prec=prec)
    if (type(result) == list or type(result) == np.ndarray):
        return(result[0])
    else:
        return(result)

def utdatestring(mjdsec):
    (mjd, dateTimeString) = mjdSecondsToMJDandUT(mjdsec)
    tokens = dateTimeString.split()
    return(tokens[0])

def ComputeDewPointCFromRHAndTempC(relativeHumidity, temperature):
    """
    inputs:  relativeHumidity in percentage, temperature in C
    output: in degrees C
    Uses formula from http://en.wikipedia.org/wiki/Dew_point#Calculating_the_dew_point
    """
    es = 6.112*np.exp(17.67*temperature/(temperature+243.5))
    e = relativeHumidity*0.01*es
    dewPoint = 243.5*np.log(e/6.112)/(17.67-np.log(e/6.112))
    return(dewPoint)

def computeWVP(d):
    """
    This simply converts the specified temperature (in Celsius) to water vapor
    pressure, which can be used to estimate the relative humidity from the
    measured dew point.
    """
    # d is in Celsius
    t = d+273.15
    w = np.exp(-6096.9385/t +21.2409642-(2.711193e-2)*t +(1.673952e-5)*t**2 +2.433502*np.log(t))
    return(w)


def resizeFonts(adesc, fontsize):
    """
    Plotting utility routine
    """
    yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    adesc.xaxis.set_major_formatter(yFormat)
    pb.setp(adesc.get_xticklabels(), fontsize=fontsize)
    pb.setp(adesc.get_yticklabels(), fontsize=fontsize)

def RescaleXAxisTimeTicks(xlim, adesc):
    """
    Plotting utility routine
    """
    if (xlim[1] - xlim[0] < 10/1440.):
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,1)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,30)))
    elif (xlim[1] - xlim[0] < 0.5/24.):
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,5)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,1)))
    elif (xlim[1] - xlim[0] < 1/24.):
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,2)))
                

