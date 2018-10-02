"""
Utilities for getting TMCDB data and plotting time-series data.
2013-09-16 thunter: added support for vector quantities, such as IFProc0 GAINS, including showing
                    the requested index in the title and in the plot filename
2013-01-08 pmerino: changes for the definitive structure of directories of monitordata.
                    changes on: def get_root_url_for_date(date) and def get_root_url_for_curl(date)
2012-12-27 pmerino: changes for the new monitordata URL
2011-04-11 tsawada: change the host name of the TMCDB web site (R8.0).
2011-01-25 tsawada: change the directory name in the TMCDB web site (R8.0).
2010-12-30 tsawada: bug fixes
2010-06-21 tsawada: cope with the change of the directory name in the TMCDB
                    web site.
2010-05-27 tsawada: changed symbol size (removed 'ms=0.5');
                    changed default 'removefile' to False in some methods;
                    added options to remove discontinuities and outliers.
2010-05-18 tsawada: added remove_outlier method
2010-05-17 tsawada: added remove_discontinuity method;
                    added get_available_antennas_on_date method;
                    added 'showxticklabels' option in some methods.
2010-05-12 tsawada: added a method to resample data;
                    show legends if 'label' option is given.
2010-05-11 tsawada: fixed a bug that 'ylabel' option was ignored
                    in some methods.
2010-04-30 tsawada

$Id: tmUtils.py,v 1.49 2015/10/15 20:47:29 thunter Exp $
"""
import datetime
import math
import os
import re
import urllib2

import numpy as np
import pylab as pl

allAntennas = ['DV%02d'%x for x in range(1,26)] + ['DA%02d'%x for x in range(40,66)] + \
              ['PM%02d'%x for x in range(1,5)] + ['CM%02d'%x for x in range(1,13)]

def get_host_name():
    """
    Returns  the  hostname
    """
    hostname = 'http://monitordata.osf.alma.cl'
    return  hostname


def get_root_url_for_date(date):
    """
    Returns the root URL of the TMCDB web I/F for the given date.

    The argument date should be an ISO-8601 date string (YYYY-MM-DD).
    The returned URL already contains the date.
    """
    year = date[:4]
    mm = date[5:7]
    hostname = get_host_name()
    return "%s/index.php?dir=%s/%s/%s/" % (hostname, year, mm, date)

def get_root_url_for_curl(date):
    """
    Returns the root URL of the new monitordata TMCDB web I/F for the given date.

    The argument date should be an ISO-8601 date string (YYYY-MM-DD).
    The returned URL already contains the date.
    """
    year = date[:4]
    mm = date[5:7]
    hostname = get_host_name()
    return "%s/monitordata/%s/%s/%s/" % (hostname, year, mm, date)

def get_available_antennas_on_date(date, device=None):
    """
    Return a list of antennas that were available on the date.

    The argument date should be an ISO-8601 date string (YYYY-MM-DD).
    This method searchs "CONTROL_XX99_" in TMCDB directory list via HTTP.
    If no antenna matched, returns an empty list.
    """
    rooturl = get_root_url_for_date(date)

    try:
        furl = urllib2.urlopen(rooturl)
    except:
        print 'Failed to open URL %s' % rooturl
        raise

    if device == None:
        regexp = 'CONTROL_([A-Z][A-Z][0-9][0-9])_'
    else:
        regexp = 'CONTROL_([A-Z][A-Z][0-9][0-9])_%s' % device

    result = []
    for line in furl:
        mount = re.search(regexp, line)
        if mount != None:
            result.append(mount.group(1))

    if len(result) > 0:
        result = list(set(result))
        result.sort()

    return result


def get_available_monitorpoints_in_device_on_date(date, antenna, device):
    """
    Return a list of MPs that were available on the date for a certain device.

    The argument date should be an ISO-8601 date string (YYYY-MM-DD).
    This method searchs "CONTROL_XX99_" in TMCDB directory list via HTTP.
    If no antenna matched, returns an empty list.
    """
    rooturl = get_root_url_for_date(date)

    targeturl = 'CONTROL_%s_%s' % (antenna, device)
    completeurl = '%s/%s' % (rooturl, targeturl)
    
    try:
        furl = urllib2.urlopen(completeurl)
    except:
        print 'Failed to open URL %s' % completeurl
        raise

    regexp = 'CONTROL_([A-Z][A-Z][0-9][0-9])'
    regexp = 'href=".*txt'

    result = []
    for line in furl:
        mount = re.search(regexp, line)
        if mount != None:
            monpoint_name=mount.group(0).split('.txt')[0].split('download=')[1]
            result.append(monpoint_name)

    if len(result) > 0:
        result = list(set(result))
        result.sort()

    return result


def retrieve_daily_tmc_data_file(antenna, device, monitorpoint, date,
                                 verbose = True, outpath='./'):
    """
    Retrieve TMC monitor data via HTTP.

    Parameters are something like:
    antenna = 'DV01'
    device = 'LLC'
    monitorpoint = 'CNTR_0'
    date = '2010-04-24'  # ISO-8601 date or datetime string

    outpath = set this if you don't want to write the result to the working directory

    Return the name of the file if succeeded, otherwise '_CURL_FAILED_'.
    """

    isodate = get_datetime_from_isodatetime(date).date().strftime('%Y-%m-%d')
    inputdate = datetime.datetime.strptime(date, '%Y-%m-%d')
    
    rooturl = get_root_url_for_curl(date)

    extension = 'txt'
    targeturl = 'CONTROL_%s_%s/%s.%s' % (antenna, device, monitorpoint, extension)
    completeurl = '%s/%s' % (rooturl, targeturl)
    outfile = '%s%s_%s_%s_%s.%s' % (outpath,isodate, antenna, device, monitorpoint, extension)
    if verbose == True: print date, rooturl,targeturl,completeurl
    exitcode = os.system('curl -s -f %s -o %s' % (completeurl, outfile))
        
    if exitcode == 0:
        return outfile
    else:
        extension = 'txt.bz2'
        targeturl = 'CONTROL_%s_%s/%s.%s' % (antenna, device, monitorpoint, extension)
        completeurl = '%s/%s' % (rooturl, targeturl)
        outfile = '%s%s_%s_%s_%s.%s' % (outpath,isodate, antenna, device, monitorpoint, extension)
        if verbose == True: print date, rooturl,targeturl,completeurl
        exitcode = os.system('curl -s -f %s -o %s' % (completeurl, outfile))

        if exitcode == 0:
            os.system('bunzip2 %s' %outfile)
            outfile = outfile[0:-4]
            return outfile
        else:
            if verbose == True: print 'Retrieval failed. Check permissions on directory and set outpath if necessary'
            return '_CURL_FAILED_'


def retrieve_daily_tmc_data_file_name_only(antenna, device, monitorpoint, 
                                           date, outpath='./'):
    """
    Retrieve name of file that would be created by retrieve_daily_tmc_data_file()

    Parameters are something like:
    antenna = 'DV01'
    device = 'LLC'
    monitorpoint = 'CNTR_0'
    date = '2010-04-24'  # ISO-8601 date or datetime string

    outpath = set this if you don't want to write the result to the working directory

    Return the name of the file if succeeded, otherwise '_CURL_FAILED_'.
    """

    isodate = get_datetime_from_isodatetime(date).date().strftime('%Y-%m-%d')
    inputdate = datetime.datetime.strptime(date, '%Y-%m-%d')
    rooturl = get_root_url_for_curl(date)
    extension = 'txt'
    targeturl = 'CONTROL_%s_%s/%s.%s' % (antenna, device, monitorpoint, extension)
    completeurl = '%s/%s' % (rooturl, targeturl)
    outfile = '%s%s_%s_%s_%s.%s' % (outpath,isodate, antenna, device, monitorpoint, extension)
    return(outfile)

def get_datetime_from_isodatetime(isodatetime):
    """
    Return a datetime.datetime object for given ISO-8601 date/datetime string.

    The argument isodatetime should be in YYYY-MM-DDThh:mm:ss or YYYY-MM-DD
    (in the latter case, 00:00:00 is assumed).
    Return 0001-01-01T00:00:00 if an invalid string is given.
    """

    datelist = isodatetime.split('T')
    if len(datelist) == 1:  # date only
        timelist = [0, 0, 0]
        datelist = datelist[0].split('-')
    elif len(datelist) == 2:  # date and time
        timelist = datelist[1].split(':')
        datelist = datelist[0].split('-')
    else:
        print "Date %s is invalid." % isodatetime
        return datetime.date(1, 1, 1)
    
    
    if (len(datelist) == 3) and (len(timelist) == 3):
        microsec = int(1e6 * (float(timelist[2]) - int(float(timelist[2]))))
        timelist[2] = int(float(timelist[2]))
        return datetime.datetime( \
            int(datelist[0]), int(datelist[1]), int(datelist[2]), \
            int(timelist[0]), int(timelist[1]), int(timelist[2]), microsec )
    else:
        print "Date '%s' is invalid." % isodatetime
        return datetime.date(1, 1, 1)


def retrieve_tmc_data_files(antenna, device, monitorpoint, startdate, enddate,
                            verbose = True, outpath='./'):
    """
    Retrieve TMC monitor data files for given antenna, device, and period.

    Parameters are something like:
    antenna = 'DV01'
    device = 'LLC'
    monitorpoint = 'CNTR_0'
    startdate = '2010-04-24'  # ISO-8601 date or datetime string
    enddate = '2010-04-24'    # ISO-8601 date or datetime string

    Return a list of filenames.
    """

    sdate = get_datetime_from_isodatetime(startdate).date()
    edate = get_datetime_from_isodatetime(enddate).date()
    y1900 = datetime.date(1900, 1, 1)

    if ((sdate-y1900).days < 0) or ((edate-y1900).days < 0):
        raise ValueError, "Startdate and/or enddate are invalid."

    if ((edate-sdate).days < 0):
        raise ValueError, "Startdate is later than enddate."

    ndays = (edate-sdate).days + 1
    files = []

    for i in range(ndays):
        targetdate = sdate + datetime.timedelta(i)
        filename = retrieve_daily_tmc_data_file( \
            antenna=antenna, \
            device=device, \
            monitorpoint=monitorpoint, \
            date=targetdate.strftime('%Y-%m-%d'),
            verbose = verbose, 
            outpath=outpath)
        if filename != '_CURL_FAILED_':
            files.append(filename)

    return files


def read_tmc_data_file(filename, removefile=False):
    """
    Read given TMC data file.

    Set removefile to False in order to keep files, otherwise removed.
    Return is a dictionary of lists of datetime.datetime and float values:
    {'datetime': [datetime], 'value': [value]}
    """
    # import os

    if not os.path.exists(filename):
        raise Exception, ("File %s not found." % filename)

    datetimelist = []
    valuelist = []

    for line in open(filename, 'r'):
        if line[0] == '#':
            continue
#        (strdatetime, value) = line.split()
        tokens = line.split()
        dt = get_datetime_from_isodatetime(tokens[0])
        # check that the date was valid
        if dt == datetime.date(1,1,1):
            continue
        datetimelist.append(get_datetime_from_isodatetime(tokens[0]))
        valuelist.append([float(x) for x in tokens[1:]])
            
    if removefile:
        os.system('rm %s' % filename)

    return {'datetime': datetimelist, 'value': valuelist}


def get_tmc_data(antenna, device, monitorpoint, startdate, enddate, \
    removefile=False, verbose = True, outpath='./'):
    """
    Obtain monitor values recorded in TMCDB for given set of parameters.

    Parameters are something like:
    antenna = 'DV01'
    device = 'LLC'
    monitorpoint = 'CNTR_0'
    startdate = '2010-04-24'  # ISO-8601 date or datetime string
    enddate = '2010-04-24'    # ISO-8601 date or datetime string
    removefile = False        # optional: set True to delete files once read
    
    Files are automatically retrieved via HTTP, read, and deleted.
    Return is a dictionary of lists of datetime.datetime and float values:
    {'datetime': [datetime], 'value': [value]}
    """
    # import os

    files = retrieve_tmc_data_files(antenna, device, monitorpoint, \
        startdate, enddate, verbose = verbose , outpath=outpath)
    if len(files) == 0:
        raise Exception, "Failed to retrieve data files."

    datetimelist = []
    valuelist = []
    for filename in files:
        tmcdata = read_tmc_data_file(filename, removefile)
        datetimelist += tmcdata['datetime']
        valuelist += tmcdata['value']
    return {'datetime': datetimelist, 'value': valuelist, 'files': files}


def show_time_series_in_subplot(subpl, datetimelist, datalist, \
    startdatetime, enddatetime=None, yrange=None, ylabel=None, tickmark='.',label=None, \
    title=None, showxticklabels=True, col='b', removediscontinuity=False, \
    removeoutlier=False, index=0):
    """
    Plot a time series of any data in a pylab.figure.subplot instance.

    You need to create a subplot instance in advance by, e.g.:
        import pylab as pl
        plf = pl.figure()
        subpl = plf.add_subplot(1, 1, 1)
    and to show it:
        plf.show()
    The other parameters are something like:
    datetimelist   # a list of datetime.datetime instances
    datalist       # a list of numbers
    startdate = '2010-04-24T00:00:00' # ISO-8601 date or datetime string
    enddate = '2010-04-25T12:00:00'   # ISO-8601 date or datetime (optional)
    yrange = [-33000, 33000]          # optional
    ylabel = 'Value [unknown unit]'   # optional
    label = 'Legend of the symbol'    # optional
    title = 'Title of the plot'       # optional
    col = matplotlib color            # optional
    index = 0 # only necessary to set this for vector quantities (to choose which channel)
    No return values.
    """
    # import pylab as pl

    if enddatetime == None:
        enddatetime = startdatetime

    trange = [get_datetime_from_isodatetime(startdatetime), \
        get_datetime_from_isodatetime(enddatetime)]

    # extend upper limit of datetime to 24:00 when needed
    if ((trange[1]-trange[0]) <= datetime.timedelta(0)) or \
        (len(enddatetime.split('T')) == 1):
        trange[1] += datetime.timedelta(1)

    # trim the time series in order to optimize y-axis range (and speed)
    datetimearray = pl.array(datetimelist)
    dataarray = pl.array(datalist)
    try:
        dataarray = dataarray[:,index]
    except:
        print "Invalid index (%d) for this monitor point. Using 0 instead."
        dataarray = dataarray[:,0]
    timeinrange = (((datetimearray-trange[0]) > datetime.timedelta(0)) & \
        ((datetimearray-trange[1]) < datetime.timedelta(0)))

    thedata = dataarray[timeinrange]
    # remove discontinuity and outliers
    if removediscontinuity:
        thedata = remove_discontinuity(thedata)
    if removeoutlier:
        thedata = remove_outlier(thedata)

    subpl.plot_date(datetimearray[timeinrange], thedata, \
        tickmark, ms=2.4, label=label, color=col)

    if title != None:
        subpl.set_title(title)

    subpl.set_xticklabels( \
        [litem.get_text() for litem in subpl.get_xticklabels()], \
        fontsize='small', rotation=30, ha='right')
    if ylabel == None:
        subpl.set_ylabel('Value')
    else:
        subpl.set_ylabel(ylabel)
    subpl.set_yticklabels( \
        [litem.get_text() for litem in subpl.get_yticklabels()], \
        fontsize='small')
    subpl.set_xlim(trange)
    if yrange != None:
        subpl.set_ylim(yrange)

    if (trange[1]-trange[0]) < datetime.timedelta(0, 300):
        subpl.xaxis.set_major_formatter( \
            pl.matplotlib.dates.DateFormatter('%m-%d %H:%M:%S'))
    else:
        subpl.xaxis.set_major_formatter( \
            pl.matplotlib.dates.DateFormatter('%m-%d %H:%M'))
    subpl.yaxis.set_major_formatter( \
        pl.matplotlib.ticker.ScalarFormatter(useOffset=False))
    #        pl.matplotlib.ticker.ScalarFormatter('%f'))
    
    if not showxticklabels:
        subpl.set_xticklabels('')

    if label != None:
        subpl.legend(prop=pl.matplotlib.font_manager.FontProperties(size=10))

    subpl.grid()

def show_monitor_data_in_subplot(subpl, antenna, device, monitorpoint, \
    startdatetime, enddatetime=None, yrange=None, ylabel=None, tickmark='.',label=None, \
    title=None, showxticklabels=True, col='b', removediscontinuity=False, \
    removeoutlier=False, index=0):

    """
    Plot a time series of TMC monitor data in a pylab.figure.subplot instance.

    The TMC monitor data are retrieved, read, and removed automatically.
    You need to create a subplot instance in advance by, e.g.:
        import pylab as pl
        plf = pl.figure()
        subpl = plf.add_subplot(1, 1, 1)
    and to show it:
        plf.show()
    The other parameters are something like:
    antenna = 'DV01'
    device = 'LLC'
    monitorpoint = 'CNTR_0'
    startdate = '2010-04-24T00:00:00' # ISO-8601 date or datetime string
    enddate = '2010-04-25T12:00:00'   # ISO-8601 date or datetime (optional)
    yrange = [-33000, 33000]          # optional
    ylabel = 'Value [unknown unit]'   # optional
    label = 'Legend of the symbol'    # optional
    title = 'Title of the plot'       # optional
    col = a matplotlib color ('r','b'...)  # optional
    index = 0 # only necessary to set this for vector quantities (to choose which channel)
    No return values.
    """

    if enddatetime == None:
        enddatetime = startdatetime

    trange = [get_datetime_from_isodatetime(startdatetime), \
        get_datetime_from_isodatetime(enddatetime)]

    # range of dates for which the TMC data are obtained
    startdate = trange[0].date().strftime('%Y-%m-%d')
    enddate = trange[1].date().strftime('%Y-%m-%d')

       
    tmcdata = get_tmc_data(antenna=antenna, device=device, \
        monitorpoint=monitorpoint, startdate=startdate, enddate=enddate, \
        removefile=True, verbose=False)

    if title == None:
        indices = len(tmcdata['value'][0])
        if (indices > 1):  # this is a vector of values, so show the index
            title = '%s %s %s:%d/%d %s' % (antenna, device, monitorpoint, index, indices-1, startdate)
        else:
            title = '%s %s %s %s' % (antenna, device, monitorpoint, startdate)
    if ylabel == None:
        ylabel = '%s' % (monitorpoint)
    show_time_series_in_subplot(subpl, \
        tmcdata['datetime'], tmcdata['value'], \
        startdatetime, enddatetime, yrange, ylabel, tickmark,label, title, \
        showxticklabels, col,removediscontinuity, removeoutlier, index)

def check_for_time_gaps(antenna='DV01', device='IFProc0', monitorpoint='GAINS',
                        startdate=None, enddate=None, sigma=5,
                        showFirstGap=False, outpath='./',removefile=True, debug=False):
    """
    Check for gaps in the time series data for a specific monitor point
    Parameters are something like:
    antenna = 'DV01'
    device = 'LLC'
    monitorpoint = 'CNTR_0'
    startdate = '2010-04-24T00:00:00' # ISO-8601 date or datetime string (default=yesterday)
    enddate = '2010-04-25T12:00:00'   # ISO-8601 date or datetime (optional)
    sigma: the factor by which an interval must me larger than the median to be declared a gap
    """
    
    if (startdate==None):
        startdate = datetime.date.today() + datetime.timedelta(-1)
        startdate = startdate.strftime('%Y-%m-%dT00:00:00')
    if (enddate==None):
        enddate= datetime.date.today()+ datetime.timedelta(-1)
        enddate=enddate.strftime('%Y-%m-%dT23:59:59')

    fsrTimeList = check_for_FSR(startdate, enddate, verbose = False)
    if debug: print fsrTimeList
    try:
        tmcdata = get_tmc_data(antenna=antenna, device=device, \
                               monitorpoint=monitorpoint, startdate=startdate, enddate=enddate, \
                               removefile=removefile, outpath=outpath, verbose=False)
        unixtime = []
        dateTimeList = tmcdata['datetime']
    except:
        return 1,[],100
    
    for d in dateTimeList:
        unixtime.append(int(d.strftime("%s")))
    unixtime = np.array(unixtime)
    successiveDifferences = unixtime[1:] - unixtime[:-1]
    medianInterval = np.median(successiveDifferences)
    madInterval = MAD(successiveDifferences)
    print "Median Interval = %f seconds" % (medianInterval)
    idelta =  datetime.timedelta(seconds = 600)
    
    gaps = []
    for i in range(len(successiveDifferences)):
        if (successiveDifferences[i] > medianInterval*sigma):                
            if debug: print "gap %d is at %s" % (len(gaps),str(dateTimeList[i]))
            if (len(gaps) == 0 and showFirstGap):
                print "First gap is at %s" % (str(dateTimeList[i]))
            gaps.append(successiveDifferences[i])
            # check if this belongs to an FSR
            for fsrt in fsrTimeList:
                if (dateTimeList[i] < fsrt+idelta) and (dateTimeList[i] > fsrt-idelta):
                    # this gap is within an FSR
                    if debug: print dateTimeList[i],  fsrt+idelta,  fsrt-idelta
                    gaps.pop(-1)
                    break
    if (len(gaps) > 0):
        print " %d gaps (not due to FSR) of median size = %f seconds" % (len(gaps), np.median(gaps))
        percentLost = sum(gaps)*100.0/(unixtime[-1]-unixtime[0]+medianInterval)
        print "Lost data = %f seconds = %f percent of total dataset" % (sum(gaps), percentLost)
    else:
        print "Found NO gaps in this time range"
        percentLost = 0
        
    return len(gaps), gaps, percentLost


def check_for_FSR(startdate, enddate, verbose = True):
    # this assumes we are on osf-red or scops
    
    print "### Querying SLT for list of FSR from %s to %s"%(startdate,enddate)
    tmpfile1 = 'tmp_query1.txt'
    owd = os.getcwd()
    cmd = 'shiftlog-query -it OTHER -ib %s -ie %s -l OSF-AOS -t CMDLN -s \"FullSystemRestart\" > %s/%s '%(startdate, enddate,owd, tmpfile1)
    if verbose: print cmd
    #raw_input()
    os.system(cmd)

    fsrTimes = []
    f = open(tmpfile1,'r')
    lines = f.readlines()
    f.close()
    for line in lines:
        if line.startswith('"CMDLN'):
            sline = line.split('"')
            fsr = sline[17]
            x = fsr.split()
            date = '%s %s %s %s %s'%(x[4],x[5],x[6],x[7],x[8])
            if verbose: print 'FSR # %d at %s'%(len(fsrTimes),date)
            fsrTimes.append(datetime.datetime.strptime(date,'%a %b %d %H:%M:%S %Y'))
        
    print 'Found %d FSRs in this time range'%(len(fsrTimes))
    os.system('rm -Rf tmp_query1.txt')
    
    return fsrTimes


def plot_monitor_data_to_png(antenna, device, monitorpoint, \
    startdatetime, enddatetime=None, yrange=None, showplot=False, \
    removediscontinuity=False, removeoutlier=False, index=0):
    """
    Plot a time series of TMC monitor data and save as a PNG image.
    If you want to plot all the monitor points in a given antenna/device,
    keep monitorpoint=''

    Parameters are something like:
    antenna = 'DV01'
    device = 'LLC'
    monitorpoint = 'CNTR_0'
    startdate = '2010-04-24T00:00:00' # ISO-8601 date or datetime string
    enddate = '2010-04-25T12:00:00'   # ISO-8601 date or datetime (optional)
    yrange = [-33000, 33000]          # optional
    showplot = True
    index = 0 # only necessary to set this for vector quantities (to choose which channel)

    Return the name of the PNG image.
    """
    sdate = get_datetime_from_isodatetime( \
        startdatetime).date().strftime('%Y-%m-%d')
    
    if monitorpoint == '' :
        monitorpointlist = \
            get_available_monitorpoints_in_device_on_date \
                (sdate, antenna, device)
    else :
        monitorpointlist = [monitorpoint]

    
    outfiles = []
    for mp in monitorpointlist:
        print mp
        plf = pl.figure()
        subpl = plf.add_subplot(1, 1, 1)

        show_monitor_data_in_subplot(subpl, antenna, device, mp, \
            startdatetime, enddatetime, yrange, \
            removediscontinuity=removediscontinuity, \
            removeoutlier=removeoutlier, index=index)
        
        # just determining output file name here ...
        if os.path.exists('plots') == False:
            os.system('mkdir plots')
        startdate = get_datetime_from_isodatetime( \
        startdatetime).date().strftime('%Y%m%d')
        if enddatetime == None:
            enddate = startdate
        else:
            enddate = get_datetime_from_isodatetime( \
            enddatetime).date().strftime('%Y%m%d')

        if (index == 0):
            outfile = '%s-%s_%s_%s_%s.png' % \
                      (startdate, enddate, antenna, device, mp)
        else:
            outfile = '%s-%s_%s_%s_%d_%s.png' % \
                      (startdate, enddate, antenna, device, index, mp)

        plf.savefig(outfile, form='png')
        os.system('mv %s plots/' % outfile)
        outfiles.append(outfile)

        if showplot:
            plf.show()
            dummy = raw_input('Hit return to proceed: ')
        pl.close()

    pl.close('all')
    return outfiles

################################
### for weather station data ###
################################

def retrieve_daily_weather_data_file(date, station):
    """
    Retrieve AOS temperature data via HTTP.
    
    date = '2010-04-24'  # ISO-8601 date or datetime string

    Return the name of the file if succeeded, otherwise '_CURL_FAILED_'.
    """
    
    isodate = get_datetime_from_isodatetime(date).date().strftime('%Y-%m-%d')

    rooturl = 'http://weather.aiv.alma.cl/data/data/files/'
    outfile = '%s_%s.dat'%(station,date)
    targeturl = '%s/%s/%s'%(date[0:4],date[5:7], outfile)
    completeurl = '"%s/%s"' % (rooturl, targeturl)
    print 'Retrieving %s weather: %s' % (isodate, completeurl)
    exitcode = os.system('curl -s -f %s -o %s' % (completeurl, outfile))

    if exitcode == 0:
        return outfile
    else:
        print 'Retrieval failed.'
        return '_CURL_FAILED_'


def retrieve_weather_data_files(startdate, enddate, station):
    """
    Retrieve weather data files for given period.

    Parameters are something like:
    startdate = '2010-04-24'  # ISO-8601 date or datetime string
    enddate = '2010-04-24'    # ISO-8601 date or datetime string
    
    Return a list of filenames.
    """    

    sdate = get_datetime_from_isodatetime(startdate).date()
    edate = get_datetime_from_isodatetime(enddate).date()
    y1900 = datetime.date(1900, 1, 1)

    if ((sdate-y1900).days < 0) or ((edate-y1900).days < 0):
        raise ValueError, "Startdate and/or enddate are invalid."

    if ((edate-sdate).days < 0):
        raise ValueError, "Startdate is later than enddate."

    ndays = (edate-sdate).days + 1
    files = []

    for i in range(ndays):
        targetdate = sdate + datetime.timedelta(i)
        filename = retrieve_daily_weather_data_file(date=targetdate.strftime('%Y-%m-%d'), station=station)
        if filename != '_CURL_FAILED_':
            files.append(filename)

    return files


def read_weather_data_file(filename, removefile=True):
    """
    Read given weather data file.

    Set removefile to True in order to remove files once it was read.
    Return is a dictionary of lists of datetime.datetime and float values:
    {'datetime': [datetime], 'value': [value]}
    """

    if not os.path.exists(filename):
        raise Exception, ("File %s not found." % filename)

    datetimelist = []
    H = [] # Humidity
    T = [] # Temp
    D = [] #Dewpoint
    WD = []  # Wind direction
    WS = [] # Wind Speed
    P = [] # Pressure
    for line in open(filename, 'r'):
        if line[0] == '#':
            continue
        (strdatetime, h, t, d, wd, ws, p) = line.split(';')
        datetimelist.append(get_datetime_from_isodatetime(strdatetime))
        H.append(float(h))
        T.append(float(t))
        D.append(float(d))
        WD.append(float(wd))
        WS.append(float(ws))
        P.append(float(p))

    if removefile:
        os.system('rm %s' % filename)

    return {'datetime': datetimelist, 'H': H, 'T':T,'D':D,'WD':WD,'WS':WS, 'P':P}


def get_weather_data(startdate, enddate, station = 'Meteo1',removefile=False):
    """
    Obtain monitor values recorded in weather monitoring for given set of parameters.

    Parameters are something like:
    startdate = '2010-04-24'  # ISO-8601 date or datetime string
    enddate = '2010-04-24'    # ISO-8601 date or datetime string
    station = 'Meteo1'  # can be Meteo1 or Meteo2 for AOS
    removefile = False        # optional: set True to delete files once read
    
    Files are automatically retrieved via HTTP, read, and deleted.
    Return is a dictionary of lists of datetime.datetime and float values:
    {'datetime': [datetime], 'value': [value]}
    """

    if (station is not 'Meteo1') and (station is not 'Meteo2'):
        raise Exception,  'Station must be Meteo1 or Meteo2. check again...'
    
    files = retrieve_weather_data_files(startdate, enddate, station)
    if len(files) == 0:
        raise Exception, "Failed to retrieve data files."
    
    datalist = {}
    for filename in files:
        weatherdata = read_weather_data_file(filename, removefile)
        for k in weatherdata.keys():
            try:
                datalist[k] += weatherdata[k]
            except:
                datalist[k]=[]
                datalist[k] += weatherdata[k]
                
    return datalist

def get_ASTE_weather_data(startdate, enddate):
    """
    retrieves the Aste weather data and  returns it in a dictionnary.
    startdate should be format 2014-10-02
    end date should be format 2014-10-02
    """
    import datetime
    
    sdate = get_datetime_from_isodatetime(startdate).date()
    edate = get_datetime_from_isodatetime(enddate).date()
        
    ndays = (edate-sdate).days + 1
    fileList = []

    for i in range(ndays):
        targetdate = sdate + datetime.timedelta(i)
        date=targetdate.strftime('%Y%m%d')
        print date
        filename = '/data/dbarkats/meteo_data/aste_meteo/weather_%s_0000.log'%(date)
        fileList.append(filename)

    datetimelist = []
    H = [] # Humidity
    T = [] # Temp
    WD = []  # Wind direction
    WS = [] # Wind Speed
    P = [] # Pressure

    for filename in fileList:
        for line in open(filename, 'r'):
            if line[0] == '#' or line[0] == '\n':
                continue
            l = line.split()
            
            y = l[0].split('/')[0]
            m = l[0].split('/')[1]
            d = l[0].split('/')[2]
            strdatetime = '%s-%s-%sT%s:00'%(y,m,d,l[1])
            datetimelist.append(get_datetime_from_isodatetime(strdatetime))
            if l[5] != '---':
                H.append(float(l[5]))
            else:
                H.append(np.nan)
            if l[4] != '---':
                T.append(float(l[4]))
            else:
                T.append(np.nan)
            if l[10] != '---':
                WD.append(float(l[10]))
            else:
                WD.append(np.nan)
            if l[9] != '---':
                WS.append(float(l[9]))
            else:
                WS.append(np.nan)
            if l[8] != '---':
                P.append(float(l[8]))
            else:
                P.append(np.nan)                     
        
    return {'datetime': datetimelist, 'H': H, 'T':T,'WD':WD,'WS':WS, 'P':P}

def get_Nanten_weather_data(startdate, enddate):
    """
    retrieves the Nanten weather data and  returns it in a dictionnary.
    """
    import datetime
    
    sdate = get_datetime_from_isodatetime(startdate).date()
    edate = get_datetime_from_isodatetime(enddate).date()
        
    ndays = (edate-sdate).days + 1
    fileList = []

    for i in range(ndays):
        targetdate = sdate + datetime.timedelta(i)
        date=targetdate.strftime('%Y%m%d')
        print date
        filename = '/users/dbarkats/nanten_meteo/%s%s/%s.nwd'%(date[0:4],date[4:6],date)
        fileList.append(filename)

    datetimelist = []
    H = [] # Humidity
    T = [] # Temp
    WD = []  # Wind direction
    WS = [] # Wind Speed
    P = [] # Pressure

    for filename in fileList:
        for line in open(filename, 'r'):
            if line[0] == '#' or line[0] == '\n':
                continue
            l = line.split(',')
            strdatetime = '%s-%s-%sT%s:%s:%s'%(l[0].strip(),l[1].strip(),l[2].strip(),l[3].strip(),l[4].strip(),l[5].strip())
            datetimelist.append(get_datetime_from_isodatetime(strdatetime))
            H.append(float(l[9]))
            T.append(float(l[7]))
            WD.append(float(l[10]))
            WS.append(float(l[11]))
            P.append(float(l[12]))
        
    return {'datetime': datetimelist, 'H': H, 'T':T,'WD':WD,'WS':WS, 'P':P}



#### 20141111 These next two function are not funtionnal after recent changes by dB to
# read_weather_data_files
#####

def show_weather_data_in_subplot(subpl, \
    startdatetime, enddatetime=None, yrange=None, ylabel=None, label=None, \
    title=None, showxticklabels=True, \
    removediscontinuity=False, removeoutlier=False):
    """
    Plot a time series of weather data in a pylab.figure.subplot instance.

    You need to create a subplot instance in advance by, e.g.:
        import pylab as pl
        plf = pl.figure()
        subpl = plf.add_subplot(1, 1, 1)
    and to show it:
        plf.show()
    The other parameters are something like:
    startdate = '2010-04-24T00:00:00' # ISO-8601 date or datetime string
    enddate = '2010-04-25T12:00:00'   # ISO-8601 date or datetime (optional)
    yrange = [-10, 10]                # optional
    ylabel = 'Value [unknown unit]'   # optional
    label = 'Legend of the symbol'    # optional
    title = 'Title of the plot'       # optional

    No return values.
    """
    # import datetime
    # import pylab as pl

    if enddatetime == None:
        enddatetime = startdatetime

    trange = [get_datetime_from_isodatetime(startdatetime), \
        get_datetime_from_isodatetime(enddatetime)]

    # range of dates for which the TMC data are obtained
    startdate = trange[0].date().strftime('%Y-%m-%d')
    enddate = trange[1].date().strftime('%Y-%m-%d')

    weatherdata = get_weather_data(startdate=startdate, enddate=enddate, \
        removefile=True)

    if title == None:
        title = '%s' % (startdate)
    if ylabel == None:
        ylabel = '%s' % ('Temperature [C]')

    show_time_series_in_subplot(subpl, \
        weatherdata['datetime'], weatherdata['value'], \
        startdatetime, enddatetime, yrange, ylabel, label, title, \
        showxticklabels, removediscontinuity, removeoutlier)


### data resampling ###

def resample_data_in_dict(datadict, targetkey, \
    samp_begin, samp_end, samp_step, nullvalue=0.):
    """
    Resample the data in a dictionary.

    The data should be given as a dictionary: something like
    {'key1': [list1], 'key2': [list2], ...}.
    The sampling of the original data is datadict[targetkey], which can be
    a list of int/float values or datetime.datetime instances.  It should be
    sorted in advance, otherwise the result will be incorrect.
    The new sampling is determined by the begin/end values (samp_begin and
    samp_end: int/float or datetime.datetime) and the step (samp_step: give it
    as seconds if samp_begin/samp_end are datetime).

    Caveat: this method linearly interpolates the data no matter how much
    the original sampling is sparse.
    """
    if not isinstance(datadict, dict):
        raise ValueError, 'datadict should be a dictionary.'

    keys = datadict.keys()
    if not (targetkey in keys):
        raise ValueError, "targetkey '%s' is not found in keys." % targetkey

    target = datadict[targetkey]
    ### if the original sampling is datetime, convert it to float (seconds)
    targetisdatetime = False
    if isinstance(target[0], datetime.datetime):
        targetisdatetime = True
        target = [datetime_to_seconds_from_origin(item) for item in target]
        samp_begin = datetime_to_seconds_from_origin(samp_begin)
        samp_end = datetime_to_seconds_from_origin(samp_end)

    ### check dimensions of the data
    nsample_org = len(target)
    for key in keys:
        if len(datadict[key]) != nsample_org:
            raise ValueError, "dimension of datadict does not match for " + \
                "keys '%s' and '%s'" % (key, targetkey)

    ### check if the data are already sorted
    sortedtarget = target[:]
    sortedtarget.sort()
    if not (target == sortedtarget):
        print 'WARNING: datadict[targetkey] is not sorted.'

    ### define new samplings and resultant dictionary
    nsample_new = int((samp_end-samp_begin)/samp_step)+1
    newtarget = [samp_begin+samp_step*i for i in range(nsample_new)]
    result = {}
    for key in keys:
        result[key] = [nullvalue] * nsample_new

    ### main loop
    index = -1
    for i in range(nsample_org):
        prev_index = index
        index = int(math.floor((target[i]-samp_begin)/samp_step))
        if i == 0:
            continue
        ### interpolation between two (original) samples
        if (0 <= prev_index+1 < nsample_new) and \
            (0 <= index < nsample_new) and \
            (prev_index != index):
            for j in range(index-prev_index):
                weight = (newtarget[prev_index+j+1]-target[i-1]) / \
                    (target[i]-target[i-1])
                for key in keys:
                    if key == targetkey:
                        continue
                    result[key][prev_index+j+1] = \
                        (1.-weight)*datadict[key][i-1]+weight*datadict[key][i]

    if targetisdatetime:
        newtarget = [seconds_from_origin_to_datetime(item) \
            for item in newtarget]
    result[targetkey] = newtarget

    return result


def datetime_to_seconds_from_origin(value, \
    origin=datetime.datetime(1970, 1, 1)):
    """
    Convert datetime.datetime to seconds since the origin.
    """
    delta = value - origin
    return 86400.*delta.days + delta.seconds + 1e-6*delta.microseconds


def seconds_from_origin_to_datetime(value, \
    origin=datetime.datetime(1970, 1, 1)):
    """
    Convert seconds since the origin to datetime.datetime.
    """
    return origin + datetime.timedelta(0, value)


### remove discontinuities (jumps) in a series of values

def remove_discontinuity(value, xgap=10, ygap=200):
    """
    Remove discontinuity (sudden jump) in a series of values.
    Written by Denis, developed for LLC Fringe Counts data.
    value : list or numpy.array
    xgap  : "width" of index of the list/array to adjust steps
    ygap  : threshold value to detect discontinuity
    """
    difflist = pl.diff(value)
    discont_index = pl.find(abs(difflist) > ygap)

    if len(discont_index) == 0:
        return value
    else:
        discont_index = pl.append(discont_index, len(difflist))

    # find indice at discontinuities
    discont = {'start': [], 'end': []}
    qstart = discont_index[0]
    for i in range(len(discont_index)-1):
        if discont_index[i+1]-discont_index[i] > xgap:
            qend = discont_index[i]
            discont['start'].append(qstart-xgap)
            discont['end'].append(qend+xgap)
            qstart = discont_index[i+1]

    # add offsets at discontinuities
    result = pl.array(value)
    for i in range(len(discont['end'])):
        result[0:discont['start'][i]] += \
            result[discont['end'][i]] - result[discont['start'][i]]

    #remove the median
    result=result-pl.median(result)
    return result


def remove_outlier(value, sigma_th=100., width=200, ntrim=20):
    """
    Replace outlying (> sigma_th times robust std deviation) values by NaN.

    Robust (trimmed) standard deviation and average are calculated for
    each subsample whose size is "width" and within which the smallest and
    the largest "ntrim" samples are trimmed.
    """
    ndata = len(value)

    if width <= 2*ntrim:
        raise ValueError, \
            "remove_outlier: width should be greater than 2*ntrim."

    if ndata < width:
        print "Warning: Not enough number of samples to remove outliers."
        return value

    niter = int(ndata/width)+1

    result = value[:]
    #pl.clf()
    for i in range(niter):
        #pl.clf()
        idx = width*i
        if (idx+width) >= ndata:
            idx = ndata-width-1

        subarr = pl.array(value[idx:idx+width])
        tsubarr = pl.sort(subarr)[ntrim:width-ntrim]
        #pl.plot(abs((subarr-tsubarr.mean())/tsubarr.std()),'.')
        #raw_input()

        subarr[abs(subarr-tsubarr.mean()) > sigma_th*tsubarr.std()] = pl.nan
        result[idx:idx+width] = subarr

    return result

def MAD(a, c=0.6745, axis=0):
    """
    Median Absolute Deviation along given axis of an array:

    median(abs(a - median(a))) / c

    c = 0.6745 is the constant to convert from MAD to std; it is used by
    default

    """
    a = np.array(a)
    good = (a==a)
    a = np.asarray(a, np.float64)
    if a.ndim == 1:
        d = np.median(a[good])
        m = np.median(np.fabs(a[good] - d) / c)
#        print  "mad = %f" % (m)
    else:
        d = np.median(a[good], axis=axis)
        # I don't want the array to change so I have to copy it?
        if axis > 0:
            aswp = swapaxes(a[good],0,axis)
        else:
            aswp = a[good]
        m = np.median(np.fabs(aswp - d) / c, axis=0)

    return m

def plotCabinTemperatures(startdate=None, vis=None):
    if (vis != None):
        if (os.path.exists(vis) == False):
            print "Cannot find measurement set."
            return
        mymsmd = createCasaTool(msmdtool)
        mymsmd.open(vis)
        antennas = range(mymsmd.nantennas())
        mymsmd.close()
    else:
        antennas = allAntennas
    if (startdate==None):
        startdate = datetime.date.today() + datetime.timedelta(-1)
        startdate = startdate.strftime('%Y-%m-%d')
    for antenna in antennas:
        plot_monitor_data_to_png(antenna, 'Mount', 'ANTENNA_TEMPS',
                                 startdate, enddatetime=None, yrange=None, showplot=False,
                                 removediscontinuity=False, removeoutlier=False, index=0)
