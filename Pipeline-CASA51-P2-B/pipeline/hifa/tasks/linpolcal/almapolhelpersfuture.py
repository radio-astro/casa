#!/usr/bin/env python
#
# almapolhelpersfuture.py
#
# History:
#  Something that George needs to write better version of. The
#  minparallelresponse method finds the scan among those in the given 
#  gain caltable where the observed parallactic angle and given QU
#  gives a minimum response. This will be the best scan to calculate the 
#  X-Y delay for this source.
#

import matplotlib.pylab as pl
from pipeline.infrastructure.casatools import table as tb
from pipeline.infrastructure.casatools import measures as me
from math import *
import os


def minparallelresponse(caltable, gqu):

    # get obs position
    tb.open(caltable+'/ANTENNA')
    pos=tb.getcol('POSITION')
    meanpos=pl.mean(pos,1)
    frame=tb.getcolkeyword('POSITION','MEASINFO')['Ref']
    units=tb.getcolkeyword('POSITION','QuantumUnits')
    mpos=me.position(frame,
                     str(meanpos[0])+units[0],
                     str(meanpos[1])+units[1],
                     str(meanpos[2])+units[2])
    me.doframe(mpos)

    # _geodetic_ latitude
    latr=me.measure(mpos,'WGS84')['m1']['value']

    print 'Latitude = ',latr*180/pi

    tb.open(caltable+'/FIELD')
    nfld=tb.nrows()
    dirs=tb.getcol('DELAY_DIR')[:,0,:]
    tb.close()
    print 'Found as many as '+str(nfld)+' fields.'

    tb.open(caltable+'/SPECTRAL_WINDOW')
    nspw=tb.nrows()
    tb.close()
    print 'Found as many as '+str(nspw)+' spws.'

    minscan = {}
    tb.open(caltable)
    for ifld in range(nfld):
        for ispw in range(nspw):
            st=tb.query('FIELD_ID=='+str(ifld)+' && SPECTRAL_WINDOW_ID=='+str(ispw))
            nrows=st.nrows()
            if nrows > 0:

                rah = dirs[0,ifld] * 12.0 / pi
                decr = dirs[1,ifld]
                times = st.getcol('TIME')
                scans = st.getcol('SCAN_NUMBER')

                unique_times = set(times)
                unique_times = list(unique_times)
                unique_times.sort()
                    
                minresponse = 1e6
                mintime = pl.nan
                for itim in range(len(unique_times)):
                    tm = me.epoch('UTC', str(unique_times[itim])+'s')
                    last = me.measure(tm,'LAST')['m0']['value']
                    last -= floor(last)  # days
                    last *= 24.0         # hours
                    ha = last - rah      # hours
                    har = ha * 2.0 * pi / 24.0
                    
                    parang = atan2((cos(latr) * sin(har)),
                                   (sin(latr) * cos(decr) - cos(latr) * sin(decr)*cos(har)))
                    parallelresponse = \
                      gqu[ifld][0] * pl.cos(2.0*parang) + \
                      gqu[ifld][1] * pl.sin(2.0*parang)

                    if abs(parallelresponse) < minresponse:
                        mintime = unique_times[itim]
                        minresponse = abs(parallelresponse)

                if mintime is not pl.nan:
                    minscan[ifld] = scans[times==mintime][0]

                # for now assume all spw the same
                break

            st.close()

    tb.close()

    return minscan
