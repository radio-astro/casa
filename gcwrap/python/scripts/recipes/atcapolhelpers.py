#!/usr/bin/env python
#
# atcapolhelpers.py
#
# History:
#  v1.0 (gmoellen, 2012Oct24) == initial version (linpolhelpers)
#  V1.2 (mwiering, 2016Aug01) updated for ATCA use
#
# This script defines the qufromgain method currently needed for
# instrumental polarization calibration for the linear
# feed basis.
# To import this function, type (at the CASA prompt):
#
# from recipes.atcapolhelpers import *
#
#
import os
from math import pi,floor,atan2,sin,cos,sqrt
import taskinit
import pylab as pl

def qufromgain(caltable,badspw=[],badant=[],fieldids=[],paoffset=None):

    mytb=taskinit.tbtool()
    myme=taskinit.metool()

    pos=myme.observatory('atca')
    myme.doframe(pos)

    # _geodetic_ latitude
    latr=myme.measure(pos,'WGS84')['m1']['value']
    #print 'latitude: ',latr*180/pi

    mytb.open(caltable+'/FIELD')
    nfld=mytb.nrows()
    dirs=mytb.getcol('DELAY_DIR')[:,0,:]
    mytb.close()
    print 'Found '+str(nfld)+' fields.'

    mytb.open(caltable+'/SPECTRAL_WINDOW')
    freq=mytb.getcol('REF_FREQUENCY')
    nspw=mytb.nrows()
    mytb.close()
    print 'Found '+str(nspw)+' spws.'

    #sort out pa offset to apply
    paoff=pl.zeros(nspw)
    if paoffset==None:
        # use defaults for ATCA
        # (should get these from Feed subtable, but cal table doesn't have one)
        for ispw in range(nspw):
            if freq[ispw]<30e9 or freq[ispw]>50e9:
                paoff[ispw]=45.
            else:
                paoff[ispw]=135.
    else:
        paoff=paoffset
    
    R=pl.zeros((nspw,nfld))
    Q=pl.zeros((nspw,nfld))
    U=pl.zeros((nspw,nfld))
    mask=pl.ones((nspw,nfld),dtype=bool)

    if (len(badspw)>0):
        mask[badspw,:]=False

    if (len(fieldids)==0):
        fieldids=range(nfld)

    QU={}
    mytb.open(caltable)
    for ifld in fieldids:
        for ispw in range(nspw):
            if not mask[ispw,ifld]:
                continue
            st=mytb.query('FIELD_ID=='+str(ifld)+' && SPECTRAL_WINDOW_ID=='+str(ispw))
            nrows=st.nrows()
            if nrows > 0:
                rah=dirs[0,ifld]*12.0/pi
                decr=dirs[1,ifld]
                times=st.getcol('TIME')
                gains=st.getcol('CPARAM')
                ants=st.getcol('ANTENNA1')

                ntimes=len(pl.unique(times))
                nants=ants.max()+1

                #print nrows, ntimes, nants

                # times
                time0=86400.0*floor(times[0]/86400.0)
                rtimes=times-time0

                # amplitude ratio
                amps=pl.absolute(gains)
                amps[amps==0.0]=1.0
                ratio=amps[0,0,:]/amps[1,0,:]
                
                
                # parang
                parang=pl.zeros(len(times))
                
                for itim in range(len(times)):
                    tm=myme.epoch('UTC',str(times[itim])+'s')
                    last=myme.measure(tm,'LAST')['m0']['value']
                    last-=floor(last)  # days
                    last*=24.0  # hours
                    ha=last-rah  # hours
                    har=ha*2.0*pi/24.0
                    
                    parang[itim]=atan2( (cos(latr)*sin(har)),
                                        (sin(latr)*cos(decr)-cos(latr)*sin(decr)*cos(har)) )

                ratio.resize(nrows/nants,nants)
                parang.resize(nrows/nants,nants)
                parang+=paoff[ispw]*pi/180.
                parangd=parang*(180.0/pi)

                A=pl.ones((nrows/nants,3))
                A[:,1]=pl.cos(2*parang[:,0])
                A[:,2]=pl.sin(2*parang[:,0])

                fit=pl.lstsq(A,pl.square(ratio))
               
                amask=pl.ones(nants,dtype=bool)
                if len(badant)>0:
                    amask[badant]=False
                rsum=pl.sum(ratio[:,amask],1)
                rsum/=pl.sum(amask)
                
                fit=pl.lstsq(A,pl.square(rsum))

                R[ispw,ifld]=fit[0][0]
                Q[ispw,ifld]=fit[0][1]/R[ispw,ifld]/2.0
                U[ispw,ifld]=fit[0][2]/R[ispw,ifld]/2.0
                P=sqrt(Q[ispw,ifld]**2+U[ispw,ifld]**2)
                X=0.5*atan2(U[ispw,ifld],Q[ispw,ifld])*180/pi

                print 'Fld=%i, Spw=%i, PA Offset=%5.1f, Gx/Gy=%5.3f, Q=%5.3f, U=%5.3f, P=%5.3f, X=%5.1f' % (ifld,ispw,paoff[ispw],R[ispw,ifld],Q[ispw,ifld],U[ispw,ifld],P,X)

            else:
                mask[ispw,ifld]=False

            st.close()

        if (sum(mask[:,ifld]))>0:
            print 'For field id = ',ifld,' there are ',sum(mask[:,ifld]),'good spws.'

            Qm=pl.mean(Q[mask[:,ifld],ifld])
            Um=pl.mean(U[mask[:,ifld],ifld])
            QU[ifld]=(Qm,Um)
            Qe=pl.std(Q[mask[:,ifld],ifld])
            Ue=pl.std(U[mask[:,ifld],ifld])
            Pm=sqrt(Qm**2+Um**2)
            Xm=0.5*atan2(Um,Qm)*180/pi
            print 'Spw mean: Fld=%i Fractional: Q=%5.3f, U=%5.3f, (rms= %5.3f,%5.3f), P=%5.3f, X=%5.1f' % (ifld,Qm,Um,Qe,Ue,Pm,Xm)
    mytb.close()

    return QU
