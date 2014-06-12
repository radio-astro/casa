import os
import numpy as np
import traceback
import string
import functools
import re
import abc
import datetime

from casac import casac
from taskinit import casalog, gentools, qatool

#
# The following functions are for timerange selection (CAS-5496)
#
def split_timerange(timerange, separator):
    return [s.strip() for s in timerange.split(separator)
            if not (s.isspace() or len(s) == 0)]

def split_date_string(date_string, full=True):
    split_by_slash = date_string.split('/')
    split_by_colon = split_by_slash[-1].split(':')
    if full:
        split_by_comma = split_by_colon[-1].split('.')
    else:
        split_by_comma = [split_by_colon[-1]]
    elements_list = [element for element in
                     split_by_slash[:-1] + split_by_colon[:-1] + split_by_comma
                     if len(element) > 0]
    if full and len(elements_list) > 1 and date_string.find('.') == -1:
        elements_list += ['0']
    return elements_list
        
def get_full_description(date_string, year='YYYY', month='MM', day='DD', hour='hh', minute='mm', second='ss', subsecond='ff', default=None):
    number_of_slashes = date_string.count('/')
    number_of_colons = date_string.count(':')

    elements_list = split_date_string(date_string)

    template = string.Template('$year/$month/$day/$hour:$min:$sec.$subsec')
    keys = ['year', 'month', 'day', 'hour', 'min', 'sec', 'subsec']

    if default is None:
        default_values = [year, month, day, hour, minute, second, subsecond]
    else:
        default_values = split_date_string(default)
        if len(default_values) < 7:
            default_values = defalut_values + ['00']
            
    values = default_values[:7-len(elements_list)] + elements_list

    return template.safe_substitute(**dict(zip(keys, values)))

def to_datetime(date):
    date_elements_list = split_date_string(date, full=False)
    date_list = map(int, date_elements_list[:-1]) + [float(date_elements_list[-1])]
    t = datetime.datetime(date_list[0], date_list[1], date_list[2],
                          date_list[3], date_list[4], int(date_list[5]),
                          int((date_list[5]-int(date_list[5]))*1e6))
    return t

def to_timedelta(delta):
    delta_elements_list = split_date_string(delta, full=False)
    delta_list = map(int, delta_elements_list[:-1]) + [float(delta_elements_list[-1])]
    while len(delta_list) < 3:
        delta_list.insert(0, 0)
    dummy1 = datetime.datetime(1999, 1, 1)
    dummy2 = datetime.datetime(1999, 1, 1,
                               delta_list[0], delta_list[1], int(delta_list[2]),
                               int((delta_list[2]-int(delta_list[2]))*1e6))
    dt = dummy2 - dummy1
    return dt

def add_time(date, delta):
    t = to_datetime(date)
    dt = to_timedelta(delta)
    return t+dt    

def sub_time(date, delta):
    t = to_datetime(date)
    dt = to_timedelta(delta)
    return t-dt

def select_by_timerange(data, timerange):
    tb = gentools(['tb'])[0]
    qa = qatool()

    # first get default time and interval
    if data is not None:
        tb.open(data)
        irow = 0
        while (irow < tb.nrows()):
            all_flagged_chan = True
            npol = tb.getcell('FLAG', irow).shape[0]
            for i in xrange(npol):
                all_flagged_chan = all_flagged_chan and all(tb.getcell('FLAG', irow)[i] != 0)
            all_flagged = (tb.getcell('FLAG_ROW', irow) != 0) or all_flagged_chan
            if all_flagged:
                irow = irow + 1
            else:
                break
        irow %= tb.nrows()
        default_mjd = tb.getcell('TIME', irow) / 86400.0
        default_interval = tb.getcell('EXPOSURE', irow)
        tb.close()
    else:
        default_mjd = 0.0
        defalut_interval = 0.0

    qdate = qa.quantity(default_mjd, 'd')
    date_dict = qa.splitdate(qdate)
    parameters = {'year': str(date_dict['year']),
                  'month': str(date_dict['month']),
                  'day': str(date_dict['monthday']),
                  'hour': str(date_dict['hour']),
                  'minute': str(date_dict['min']),
                  'second': str(date_dict['sec']),
                  'subsecond': str(date_dict['usec'])}    
    
    if re.match('.+~.+', timerange):
        # This is case 1: 'T0~T1'
        dates_list = split_timerange(timerange, '~')
        first_date = get_full_description(dates_list[0], **parameters)
        second_date = get_full_description(dates_list[1], default=first_date)
        taql = 'TIME >= MJD(DATETIME("%s")) && TIME <= MJD(DATETIME("%s"))'%(first_date, second_date)
    elif re.match('.+\+.+', timerange):
        # This is case 3: 'T0+dT'
        dates_list = split_timerange(timerange, '+')
        first_date = get_full_description(dates_list[0], **parameters)
        delta_time = dates_list[1]
        second_date_datetime = add_time(first_date, delta_time)
        second_date = second_date_datetime.strftime('%Y/%m/%d/%H:%M:%S')
        microsec = '%s'%(second_date_datetime.microsecond/1e6)
        second_date += microsec.lstrip('0')
        taql = 'TIME >= MJD(DATETIME("%s")) && TIME <= MJD(DATETIME("%s"))'%(first_date, second_date)
    elif re.match('^ *>.+', timerange):
        # This is case 4: '>T0'
        dates_list = split_timerange(timerange, '>')
        first_date = get_full_description(dates_list[0], **parameters)
        taql = 'TIME > MJD(DATETIME("%s"))'%(first_date)
    elif re.match('^ *<.+', timerange):
        # This is case 5: '<T0'
        dates_list = split_timerange(timerange, '<')
        first_date = get_full_description(dates_list[0], **parameters)
        taql = 'TIME < MJD(DATETIME("%s"))'%(first_date)
    elif re.match('^[0-9/:.]+$', timerange):
        # This is case 2: 'T0'
        middle_date = get_full_description(timerange, **parameters)
        hours = int(0.5 * default_interval / 3600.0)
        minutes = int((0.5 * default_interval - hours * 3600.0) / 60.0)
        seconds = (0.5 * default_interval) % 60.0
        delta_time = '%d:%d:%s'%(hours, minutes, seconds)
        first_date_datetime = sub_time(middle_date, delta_time)
        second_date_datetime = add_time(middle_date, delta_time)
        first_date = first_date_datetime.strftime('%Y/%m/%d/%H:%M:%S')
        microsec = '%s'%(first_date_datetime.microsecond/1e6)
        first_date += microsec.lstrip('0')
        second_date = second_date_datetime.strftime('%Y/%m/%d/%H:%M:%S')
        microsec = '%s'%(second_date_datetime.microsecond/1e6)
        second_date += microsec.lstrip('0')
        taql = 'TIME >= MJD(DATETIME("%s")) && TIME <= MJD(DATETIME("%s"))'%(first_date, second_date)
    else:
        # invalid format
        casalog.post('WARNING: timerange="%s" is invalid'%(timerange), priority='WARN')
        taql = ''

    casalog.post('taql for timerange: \'%s\''%(taql), priority='DEBUG')

    return taql
