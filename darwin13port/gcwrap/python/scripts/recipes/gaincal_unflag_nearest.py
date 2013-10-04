import numpy as np
import time
import pylab as pl
from casa import table as tb

def fill_flagged_soln(caltable='', doplot=False):
    """
    This is to replace the gaincal solution of flagged/failed solutions by the nearest valid 
    one.
    If you do not do that and applycal blindly with the table your data gets 
    flagged between  calibration runs that have a bad/flagged solution at one edge.
    Can be pretty bad when you calibrate every hour or more 
    (when you are betting on self-cal) of observation (e.g L-band of the EVLA)..one can 
    lose the whole hour of good data without realizing !
    """
    tb.open(caltable, nomodify=False)
    flg=tb.getcol('FLAG')
    sol=tb.getcol('SOLUTION_OK')
    ant=tb.getcol('ANTENNA1')
    gain=tb.getcol('GAIN')
    t=tb.getcol('TIME')
    dd=tb.getcol('CAL_DESC_ID')
    maxant=np.max(ant)
    maxdd=np.max(dd)
    npol=len(gain[:,0,0])
    nchan=len(gain[0,:,0])
    
    k=1
    if(doplot):
        pl.ion()
        pl.figure(1)
        pl.plot(t[(ant==k)], sol[0,0,(ant==k)], 'b+')
        pl.plot(t[(ant==k)], flg[0,0,(ant==k)], 'r+')
        pl.twinx()
        pl.plot(t[(ant==k)], abs(gain[0,0,(ant==k)]), 'go')
    print 'maxant', maxant
    numflag=0.0
    for k in range(maxant+1):
        for j in range (maxdd+1):
            subflg=flg[:,:,(ant==k) & (dd==j)]
            subt=t[(ant==k) & (dd==j)]
            subsol=sol[:,:,(ant==k) & (dd==j)]
            subgain=gain[:,:,(ant==k) & (dd==j)]
            #print 'subgain', subgain.shape
            for kk in range(1, len(subt)):
                for chan in range(nchan):
                    for pol in range(npol):
                        if(subflg[pol,chan,kk] and not subflg[pol,chan,kk-1]):
                            numflag += 1.0
                            subflg[pol,chan,kk]=False
                            subsol[pol, chan, kk]=True
                            subgain[pol,chan,kk]=subgain[pol,chan,kk-1]
                        if(subflg[pol,chan,kk-1] and not subflg[pol,chan,kk]):
                            numflag += 1.0
                            subflg[pol,chan,kk-1]=False
                            subsol[pol, chan, kk-1]=True
                            subgain[pol,chan,kk-1]=subgain[pol,chan,kk]
            flg[:,:,(ant==k) & (dd==j)]=subflg
            sol[:,:,(ant==k) & (dd==j)]=subsol
            gain[:,:,(ant==k) & (dd==j)]=subgain


    print 'numflag', numflag
    if(doplot):
        pl.figure(2)
        k=1
        #pl.clf()
        pl.plot(t[(ant==k)], sol[0,0,(ant==k)], 'b+')
        pl.plot(t[(ant==k)], flg[0,0,(ant==k)], 'r+')
        pl.twinx()
        pl.plot(t[(ant==k)], abs(gain[0,0,(ant==k)]), 'go')
        pl.title('antenna='+str(k))
     
    ###
    tb.putcol('FLAG', flg)
    tb.putcol('SOLUTION_OK', sol)
    tb.putcol('GAIN', gain)
    tb.done()
