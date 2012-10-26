#!/usr/bin/env python
#
# linfeedpolhelpers.py
#
# History:
#  v1.0 (gmoellen, 2012Oct24) == initial version
#
# This script defines several methods currently needed for
# instrumental polarization calibration for the linear
# feed basis.
# To import these functions, type (at the CASA prompt):
#
# from recipes.linfeedpolhelpers import *
#
#
import os

def qufromgain(gt,badspw=[]):

    me.doframe(me.observatory('alma'))

    tb.open(gt+'/ANTENNA')
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

    tb.open(gt+'/FIELD')
    nfld=tb.nrows()
    dirs=tb.getcol('DELAY_DIR')[:,0,:]
    tb.close()
    print 'Found as many as '+str(nfld)+' fields.'

    tb.open(gt+'/SPECTRAL_WINDOW')
    nspw=tb.nrows()
    tb.close()
    print 'Found as many as '+str(nspw)+' spws.'

    R=pl.zeros((nspw,nfld))
    Q=pl.zeros((nspw,nfld))
    U=pl.zeros((nspw,nfld))
    mask=pl.ones((nspw,nfld),dtype=bool)

    if (len(badspw)>0):
        mask[badspw,:]=False

    tb.open(gt)
    for ifld in range(nfld):
        for ispw in range(nspw):
            if not mask[ispw,ifld]:
                continue
            st=tb.query('FIELD_ID=='+str(ifld)+' && SPECTRAL_WINDOW_ID=='+str(ispw))
            nrows=st.nrows()
            if nrows > 0:
                rah=dirs[0,ifld]*12.0/pi
                decr=dirs[1,ifld]
                times=st.getcol('TIME')
                gains=st.getcol('CPARAM')
                ants=st.getcol('ANTENNA1')

                ntimes=len(pl.unique(times))
                nants=ants.max()+1

                print nrows, ntimes, nants

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
                    tm=me.epoch('UTC',str(times[itim])+'s')
                    last=me.measure(tm,'LAST')['m0']['value']
                    last-=floor(last)  # days
                    last*=24.0  # hours
                    ha=last-rah  # hours
                    har=ha*2.0*pi/24.0
                    
                    parang[itim]=atan2( (cos(latr)*sin(har)),
                                        (sin(latr)*cos(decr)-cos(latr)*sin(decr)*cos(har)) )

                ratio.resize(nrows/nants,nants)
                parang.resize(nrows/nants,nants)
                parangd=parang*(180.0/pi)

                A=pl.ones((nrows/nants,3))
                A[:,1]=pl.cos(2*parang[:,0])
                A[:,2]=pl.sin(2*parang[:,0])

                fit=pl.lstsq(A,pl.square(ratio))

                Rant=fit[0][0]
                Qant=fit[0][1]/fit[0][0]/2
                Uant=fit[0][2]/fit[0][0]/2
                Pant=pl.sqrt(Qant*Qant + Uant*Uant)
                Xant=0.5*pl.arctan2(Uant,Qant)*180/pi
                
                print 'By antenna:'
                print ' R = ', Rant,pl.mean(Rant)
                print ' Q = ', Qant,pl.mean(Qant)
                print ' U = ', Uant,pl.mean(Uant)
                print ' P = ', Pant,pl.mean(Pant)
                print ' X = ', Xant,pl.mean(Xant)

                pl.plot(Qant,Uant,',')

                
                ants0=range(nants)
                rsum=pl.sum(ratio[:,ants0],1)
                rsum/=len(ants0)
                
                fit=pl.lstsq(A,pl.square(rsum))

                R[ispw,ifld]=fit[0][0]
                Q[ispw,ifld]=fit[0][1]/R[ispw,ifld]/2.0
                U[ispw,ifld]=fit[0][2]/R[ispw,ifld]/2.0
                P=sqrt(Q[ispw,ifld]**2+U[ispw,ifld]**2)
                X=0.5*atan2(U[ispw,ifld],Q[ispw,ifld])*180/pi

                print 'Fld=',ifld,'Spw=',ispw,'Gx/Gy=',R[ispw,ifld],'Q=',Q[ispw,ifld],'U=',U[ispw,ifld],'P=',P,'X=',X

                pl.plot(Q[ispw,ifld],U[ispw,ifld],'o')


            else:
                mask[ispw,ifld]=False

            st.close()

        print 'For field id = ',ifld,' there are ',sum(mask[:,ifld]),'good spws.'

        Qm=pl.mean(Q[mask[:,ifld],ifld])
        Um=pl.mean(U[mask[:,ifld],ifld])
        Qe=pl.std(Q[mask[:,ifld],ifld])
        Ue=pl.std(U[mask[:,ifld],ifld])
        Pm=sqrt(Qm**2+Um**2)
        Xm=0.5*atan2(Um,Qm)*180/pi
        print 'Spw mean: Fld=', ifld,' Fractional: Q=',Qm,'U=',Um,'(rms=',Qe,Ue,')','P=',Pm,'X=',Xm
    tb.close()

    pl.plot(Qm,Um,'*')

    return pl.array([Qm,Um])


def xyamb(xy,qu,xyout=''):
    if xyout=='':
        xyout=xy
    if xyout!=xy:
        os.system('cp -r '+xy+' '+xyout)

    tb.open(xyout,nomodify=F)

    QU=tb.getkeyword('QU')['QU']
    P=pl.sqrt(QU[0,:]**2+QU[1,:]**2)

    nspw=P.shape[0]
    for ispw in range(nspw):
        q=QU[0,ispw]
        u=QU[1,ispw]
        if ( (abs(q)>0.0 and abs(qu[0])>0.0 and (q/qu[0])<0.0) or
             (abs(u)>0.0 and abs(qu[1])>0.0 and (u/qu[1])<0.0) ):
            print 'Fixing ambiguity in spw',ispw
            st=tb.query('SPECTRAL_WINDOW_ID=='+str(ispw))
            c=st.getcol('CPARAM')
            c[0,:,:]*=-1.0
            st.putcol('CPARAM',c)
            st.close()
            QU[:,ispw]*=-1
    QUr={}
    QUr['QU']=QU
    tb.putkeyword('QU',QUr)
    tb.close()
    QUm=pl.mean(QU[:,P>0],1)
    QUe=pl.std(QU[:,P>0],1)
    print 'mean ambiguity-resolved fractional QU:',QUm,' +/- ',QUe
    
    return pl.array([1.0,QUm[0],QUm[1],0.0])



def dxy(dtab,xtab,dout):

    os.system('cp -rf '+dtab+' '+dout)

    # How many spws
    tb.open(dtab+'/SPECTRAL_WINDOW')
    nspw=tb.nrows()
    tb.close()


    for ispw in range(nspw):
        print 'Spw = ',ispw
        tb.open(xtab)
        x=[]
        st=tb.query('SPECTRAL_WINDOW_ID=='+str(ispw))
        x=st.getcol('CPARAM')
        st.close()
        tb.close()
        #print ' x.shape = ',x.shape, len(x)
        
        if (len(x)>0):

            tb.open(dout,nomodify=F)
            st=tb.query('SPECTRAL_WINDOW_ID=='+str(ispw))
            d=st.getcol('CPARAM')
            #print ' d.shape = ',d.shape, len(d), d.shape==x.shape

        
            # the following assumes all antennas and chans same in both tables.

            if (len(d)>0):
                if (d.shape[1:3]==x.shape[1:3]):
                    # Xinv.D.X:
                    d[0,:,:]*=pl.conj(x[0,:,:])
                    d[1,:,:]*=x[0,:,:]
                    st.putcol('CPARAM',d)
                else:
                    print ' D and X shapes do not match!'
            else:
                print '  No D solutions for this spw'
            st.close()
            tb.close()
        else:
            print '  No X solutions for this spw'


 
def zeromeanD(dtab,dout):

    os.system('cp -rf '+dtab+' '+dout)

    tb.open(dout+'/SPECTRAL_WINDOW')
    # How many spws
    nspw=tb.nrows()
    tb.close()

    tb.open(dout,nomodify=F)
    for ispw in range(nspw):
        print 'Spw = ',ispw

        st=tb.query('SPECTRAL_WINDOW_ID=='+str(ispw))

        if (st.nrows()>0):
            d=st.getcol('CPARAM')
            fl=st.getcol('FLAG')
            wt=pl.ones(d.shape)
            wt[fl]=0.0
            drm=pl.average(d[0,:,:],1,wt[0,:,:])
            dlm=pl.average(d[1,:,:],1,wt[1,:,:])
            a=(drm-pl.conj(dlm))/2.0
            a=a.reshape((len(a),1))
            d[0,:,:]=d[0,:,:]-a
            d[1,:,:]=d[1,:,:]+pl.conj(a)
            st.putcol('CPARAM',d)
        else:
            print ' No D in this spw'
        st.close()
    tb.close()



def nonorthogD(dtab,dout):

    os.system('cp -rf '+dtab+' '+dout)

    tb.open(dout+'/SPECTRAL_WINDOW')
    # How many spws
    nspw=tb.nrows()
    tb.close()

    tb.open(dout,nomodify=F)
    for ispw in range(nspw):
        print 'Spw = ',ispw

        st=tb.query('SPECTRAL_WINDOW_ID=='+str(ispw))

        if (st.nrows()>0):
            d=st.getcol('CPARAM')
            dorth=(d[0,:,:]-pl.conj(d[1,:,:]))/2.0
            d[0,:,:]-=dorth
            d[1,:,:]+=pl.conj(dorth)
            st.putcol('CPARAM',d)
        else:
            print ' No D in this spw'
        st.close()
    tb.close()
