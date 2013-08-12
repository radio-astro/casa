# This program ...
# Copyright (C) 2013
# Associated Universities, Inc. Washington DC, USA.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be addressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# $kgolap$
import dateutil
from datetime import datetime
import numpy as np
import pylab as pl
from casac import casac
qa=casac.quanta()
me=casac.measures()
def specframeconvert(val='344.84e9Hz',   inframe='TOPO', indoppler='RADIO', restfreq='1.410GHz', timebeg='2011/5/28/01h00m00', timeend='', direction='J2000 12h01m51 -18d52m00', observatory='VLA',  outframe='LSRK', npoints=100, doplot=False):
    """
    val= frequency or velocity quantity (e.g '113GHz' or '-25km/s')
    inframe=frame of input value (e.g 'TOPO', 'GEO', BARY')
    indoppler=doppler velocity definition if val is velocity ('RADIO', 'OPTICAL', 'Z' etc)
    restfrequency=line rest frequency if val is in velocity units
    timebeg=start of time range in UTC e.g '2012/12/25/00h00m00'
    timeend=end of time range in UTC or '' if only one value is needed
    direction=Source direction of interest (e.g 'J2000 12h01m51 -18d52m00')
    observatory=name of observatory to know where on earth this calculation is for
    outframe=frame of the output frequencies for the time range requested
    npoints= number of points to have values calculated in time range
    doplot=True or False (if Frequecy v/s Time plot is needed)
    """
    me.doframe(me.observatory(observatory))
    me.doframe(me.source(direction))
    me.doframe(me.epoch('utc', timebeg))
    epnow=me.epoch('utc', timebeg)
    interval=0
    xday=[]
    yday=[]
    if(timeend==''):
        npoints=1
    else:
        interval=qa.convert(qa.sub(qa.quantity(timeend), qa.quantity(timebeg)), 's')['value']
        interval=interval/npoints

    if ((qa.quantity(val)['unit'].find('Hz')) <0 and (qa.quantity(val)['unit'].find('m/s') >=0)):
        vel=me.doppler(indoppler, val)
        freq=me.tofrequency(inframe, vel, qa.quantity(restfreq))['m0']
    else:
        freq=qa.quantity(val)
    freqval=me.frequency(inframe, freq)
    for k in range(npoints):
        me.doframe(epnow)
        xday.append(pl.date2num(dateutil.parser.parse(qa.time(epnow['m0'], form='ymd')[0])))
        yday.append(me.measure(freqval, 'LSRK')['m0']['value'])
        epnow['m0']=qa.add(epnow['m0'], qa.quantity(interval, 's'))
    if(doplot):
        pl.figure(1)
        pl.clf()
        pl.plot_date(xday, np.array(yday), 'o')
        pl.xlabel('Time')
        pl.ylabel('Frequency in  Hz')
        pl.title('Variation of frequency in '+outframe)
    return np.array(yday)
