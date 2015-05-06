# Function to get Az, El for a particular source

#date= '2010/02/02'
from numpy import *
from casac import *

qa = casac.quanta()
me = casac.measures()


#obsMeas = me.observatory('ALMA')
#srcCoord = 'J2000 17h52m0s 9d39m0s'
def getazel(obs,srcname,srcCoord,date,tref):
    """
    Calculate azimuth and elevation of a given source coordinates
    Input:
    observatory  observatory name known to CASA (e.g. 'ALMA')
    srcCoord     source coordinates (RA,Dec) with epoch in a string (e.g. 'J2000 17h45m0.0s -29d00m00.0s' )
                 or known ephemeris source name 
    date         date in a string (YYYY/MM/DD or YYYY-MM-DD)
    tref         time reference (tref='UTC-3' or 'CDT' for Chilean daylight saving time,
                                      'UTC-4' or 'CLT' for Chilean standard time,
                                      'LST', or 'UTC')
    Output:
    vector of az values,el values,time,utctime 
    TT - 2012.04.19 
    """
# original 2010.02.02 TT
# modified 2012.04.19 TT
 
    t=qa.quantity(date)
    if t['unit'] != 'd':
        msg = 'Cannot decode date (format should a string with YYYY/MM/DD or YYYY-MM-DD)'
        raise Exception, msg
    if tref=='UTC-3' or tref=='CDT': # chile daylight saving time
        tshft=-3/24.0
    elif tref=='UTC-4' or tref=='CLT': # chile standard time
        tshft=-4/24.0
    else:
        tshft=0.0
    t0=t['value']
    t0 -= tshft
    print "TO = ",t0
    # coaser time (maybe better if called from plotting script for many sources
    #tl = arange(1.,1455.,15) # go over a bit longer than a day
    tl = arange(1.,1455.,1) # go over a bit longer than a day
    tl /=1440.
    tm = t0+tl
  
    obsMeas = me.observatory(obs)
    me.doframe(obsMeas)

    elar=zeros(len(tm))
    azar=zeros(len(tm))
    lastar=zeros(len(tm))
    retutc=zeros(len(tm))
    
    (ep,ra,dec) = srcCoord.split(" ")[0:3]

    #coord = me.direction(ep,ra,dec)
    for i in range(len(tm)):
        t = tm[i]
        tq = qa.quantity(t,'d')
        tim = me.epoch('utc',tq)
        me.doframe(tim)
        last = me.measure(tim,'last')
        if dec=='':
	  coord=me.direction(ra)
        else:
          coord=me.direction(ep,ra,dec)

        azel = me.measure(coord, 'azel')
        az = me.getvalue(azel)['m0']['value']
        el = me.getvalue(azel)['m1']['value']
        if i ==0:
          riseset=me.riseset(coord)
          if tref=='LST':
            timref='last'
          else:
            timref='utc' 
          tmeridian=(riseset['rise'][timref]['m0']['value']+riseset['set'][timref]['m0']['value'])/2.
          print "%s :Meridian passage: %s" % (srcname, qa.time(str(tmeridian)+'d')[0]+' ('+timref+')') 
          #print "Rise:",riseset['rise'] 
          #print "Set:",riseset['set'] 
        azar[i]=az
        elar[i]=el
        lastar[i]=last['m0']['value']
    tm = tm + tshft
    if tref=='LST':
        rettm = lastar
        retutc = tm
    else:
        rettm = tm

    return azar, elar, rettm, retutc
