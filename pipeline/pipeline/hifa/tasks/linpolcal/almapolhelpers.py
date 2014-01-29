#!/usr/bin/env python
#
# almapolhelpers.py
#
# History:
#  v1.0 (gmoellen; 2013Oct02) initial distributed version
#
# This script defines several functions useful for ALMA Polarization processing.
#
# To access these functions, type (at the CASA prompt):
#
# from recipes.almapolhelpers import *
#
# For more information about each function type

#
#

# jfl additions
import matplotlib.pylab as pl
import pipeline.infrastructure.casatools as casatools
from math import *
import os

# end of jfl additions

import glob as glob
from recipes.almahelpers import tsysspwmap


def bandname(spwname):
    # before first #, after last _
    return spwname.split("#")[0].split('_')[-1]

def bandpa(name):
    #  From rhills (CSV-481, 2012Sep04)
    #  B1: [ 135.0, 225.0]
    #  B2: [ 45.0, 135.0]
    #  B3: [-10.0, 80.0]
    #  B4: [-170.0, -80.0]
    #  B5: [-45.0, 45.0]
    #  B6: [-135.0, -45.0]
    #  B7: [-52.5, 37.5] ( +37.5 from radial)
    #  B8: [ 0.0, 90.0]
    #  B9: [-180.0, -90.0]
    #  B10: [ 90.0, 180.0]
    bparad=pl.array([0.0,135.,45.,-10.,-170.,-45.,-135.,-52.5,0.,-180.,90.])*pi/180.0 # in rad
    iband=0
    if (type(name)==type(1)):
        iband=name
    else:
        # process as a stringspecified string
        bn=bandname(str(name))
        if (bn.isdigit()):
            iband=int(bn)
        else:
            iband=0

    if (iband<11):
        return bparad[iband]
    else:
        raise Exception, "Can't discern bandname from: "+str(name)


def qufromgain(caltable,badspw=[],paoffset=0.0):

    ctb = casatools.table
    cme = casatools.measures

    ctb.open(caltable+'/ANTENNA')
    pos=ctb.getcol('POSITION')
    meanpos=pl.mean(pos,1)
    frame=ctb.getcolkeyword('POSITION','MEASINFO')['Ref']
    units=ctb.getcolkeyword('POSITION','QuantumUnits')
    mpos=cme.position(frame,
                     str(meanpos[0])+units[0],
                     str(meanpos[1])+units[1],
                     str(meanpos[2])+units[2])
    cme.doframe(mpos)

    # _geodetic_ latitude
    latr=cme.measure(mpos,'WGS84')['m1']['value']

    print 'Latitude = ',latr*180/pi

    ctb.open(caltable+'/FIELD')
    nfld=ctb.nrows()
    dirs=ctb.getcol('DELAY_DIR')[:,0,:]
    ctb.close()
    print 'Found as many as '+str(nfld)+' fields.'

    ctb.open(caltable+'/SPECTRAL_WINDOW')
    nspw=ctb.nrows()
    bandnames=[x.split('#')[0].split('_')[-1] for x in ctb.getcol('NAME')]
    ctb.close()
    print 'Found as many as '+str(nspw)+' spws.'

    R=pl.zeros((nspw,nfld))
    Q=pl.zeros((nspw,nfld))
    U=pl.zeros((nspw,nfld))
    mask=pl.ones((nspw,nfld),dtype=bool)

    if (len(badspw)>0):
        mask[badspw,:]=False

    QU={}
    ctb.open(caltable)
    for ifld in range(nfld):
        for ispw in range(nspw):
            st=ctb.query('FIELD_ID=='+str(ifld)+' && SPECTRAL_WINDOW_ID=='+str(ispw))
            nrows=st.nrows()
            if nrows > 0:


                rah=dirs[0,ifld]*12.0/pi
                decr=dirs[1,ifld]
                times=st.getcol('TIME')
                gains=st.getcol('CPARAM')
                ants=st.getcol('ANTENNA1')

                nants=ants.max()+1
                    
                # times
                time0=86400.0*floor(times[0]/86400.0)
                rtimes=times-time0

                # amplitude ratio
                amps=pl.absolute(gains)
                amps[amps==0.0]=1.0
                ratio=amps[0,0,:]/amps[1,0,:]
                
                ratio.resize(nrows/nants,nants)
                
                # parang
                parang=pl.zeros(len(times))
                
                for itim in range(len(times)):
                    tm=cme.epoch('UTC',str(times[itim])+'s')
                    last=cme.measure(tm,'LAST')['m0']['value']
                    last-=floor(last)  # days
                    last*=24.0  # hours
                    ha=last-rah  # hours
                    har=ha*2.0*pi/24.0
                    
                    parang[itim]=atan2( (cos(latr)*sin(har)),
                                        (sin(latr)*cos(decr)-cos(latr)*sin(decr)*cos(har)) )

                parang.resize(nrows/nants,nants)
                parang+=bandpa(bandnames[ispw])  # feed pos ang offset
                parang+=(paoffset*pi/180.)       # manual feed pa offset
                parangd=parang*(180.0/pi)

                A=pl.ones((nrows/nants,3))
                A[:,1]=pl.cos(2*parang[:,0])
                A[:,2]=pl.sin(2*parang[:,0])

                fit=pl.lstsq(A,pl.square(ratio))

                ants0=range(nants)
                rsum=pl.sum(ratio[:,ants0],1)
                rsum/=len(ants0)
                
                fit=pl.lstsq(A,pl.square(rsum))
                R[ispw,ifld]=fit[0][0]
                Q[ispw,ifld]=fit[0][1]/R[ispw,ifld]/2.0
                U[ispw,ifld]=fit[0][2]/R[ispw,ifld]/2.0
                P=sqrt(Q[ispw,ifld]**2+U[ispw,ifld]**2)
                X=0.5*atan2(U[ispw,ifld],Q[ispw,ifld])*180/pi

                print 'Fld=',ifld,'Spw=',ispw,'(B='+str(bandnames[ispw])+', PA offset='+str(bandpa(bandnames[ispw])*180./pi)+'deg)','Gx/Gy=',R[ispw,ifld],'Q=',Q[ispw,ifld],'U=',U[ispw,ifld],'P=',P,'X=',X
                
            else:
                mask[ispw,ifld]=False

            st.close()

        if sum(mask[:,ifld])>0:
            print 'For field id = ',ifld,' there are ',sum(mask[:,ifld]),'good spws.'

            Qm=pl.mean(Q[mask[:,ifld],ifld])
            Um=pl.mean(U[mask[:,ifld],ifld])
            QU[ifld]=(Qm,Um)
            Qe=pl.std(Q[mask[:,ifld],ifld])
            Ue=pl.std(U[mask[:,ifld],ifld])
            Pm=sqrt(Qm**2+Um**2)
            Xm=0.5*atan2(Um,Qm)*180/pi
            print 'Spw mean: Fld=', ifld,'Q=',Qm,'U=',Um,'(rms=',Qe,Ue,')','P=',Pm,'X=',Xm

    ctb.close()

    return QU


def xyamb(xytab,qu,xyout=''):

    ctb = casatools.table
    cme = casatools.measures

    if not isinstance(qu,tuple):
        raise Exception,'qu must be a tuple: (Q,U)'

    if xyout=='':
        xyout=xytab
    if xyout!=xytab:
        os.system('cp -r '+xytab+' '+xyout)

    QUexp=complex(qu[0],qu[1])
    print 'Expected QU = ',qu   # , '  (',pl.angle(QUexp)*180/pi,')'

    ctb.open(xyout,nomodify=False)

    QU=ctb.getkeyword('QU')['QU']
    P=pl.sqrt(QU[0,:]**2+QU[1,:]**2)

    nspw=P.shape[0]
    for ispw in range(nspw):
        st=ctb.query('SPECTRAL_WINDOW_ID=='+str(ispw))
        if (st.nrows()>0):
            q=QU[0,ispw]
            u=QU[1,ispw]
            qufound=complex(q,u)
            c=st.getcol('CPARAM')
            fl=st.getcol('FLAG')
            xyph0=pl.angle(pl.mean(c[0,:,:][pl.logical_not(fl[0,:,:])]),True)
            print 'Spw = '+str(ispw)+': Found QU = '+str(QU[:,ispw])  # +'   ('+str(pl.angle(qufound)*180/pi)+')'
            #if ( (abs(q)>0.0 and abs(qu[0])>0.0 and (q/qu[0])<0.0) or
            #     (abs(u)>0.0 and abs(qu[1])>0.0 and (u/qu[1])<0.0) ):
            if ( pl.absolute(pl.angle(qufound/QUexp)*180/pi)>90.0 ):
                c[0,:,:]*=-1.0
                xyph1=pl.angle(pl.mean(c[0,:,:][pl.logical_not(fl[0,:,:])]),True)
                st.putcol('CPARAM',c)
                QU[:,ispw]*=-1
                print '   ...CONVERTING X-Y phase from '+str(xyph0)+' to '+str(xyph1)+' deg'
            else:
                print '      ...KEEPING X-Y phase '+str(xyph0)+' deg'
            st.close()
    QUr={}
    QUr['QU']=QU
    ctb.putkeyword('QU',QUr)
    ctb.close()
    QUm=pl.mean(QU[:,P>0],1)
    QUe=pl.std(QU[:,P>0],1)
    Pm=pl.sqrt(QUm[0]**2+QUm[1]**2)
    Xm=0.5*atan2(QUm[1],QUm[0])*180/pi

    print 'Ambiguity resolved (spw mean): Q=',QUm[0],'U=',QUm[1],'(rms=',QUe[0],QUe[1],')','P=',Pm,'X=',Xm

    stokes=[1.0,QUm[0],QUm[1],0.0]
    print 'Returning the following Stokes vector: '+str(stokes)
    
    return stokes



def dxy(dtab,xytab,dout):

    os.system('cp -r '+dtab+' '+dout)

    # How many spws
    tb.open(dtab+'/SPECTRAL_WINDOW')
    nspw=tb.nrows()
    tb.close()


    for ispw in range(nspw):
        tb.open(xytab)
        st=tb.query('SPECTRAL_WINDOW_ID=='+str(ispw))
        x=st.getcol('CPARAM')
        st.close()
        tb.close()

        tb.open(dout,nomodify=False)
        st=tb.query('SPECTRAL_WINDOW_ID=='+str(ispw))
        d=st.getcol('CPARAM')

        # the following assumes all antennas and chans same in both tables.

        # Xinv.D.X:
        d[0,:,:]*=pl.conj(x[0,:,:])
        d[1,:,:]*=x[0,:,:]

        st.putcol('CPARAM',d)
        st.close()
        tb.close()


def Dgen(dtab,dout):

    ctb = casatools.table
    cme = casatools.measures

    os.system('cp -r '+dtab+' '+dout)

    ctb.open(dout,nomodify=False)

    irec=ctb.info()
    st=irec['subType']
    if st.count('Df')>0:
        irec['subType']='Dfgen Jones'
    elif st.count('D')>0:
        irec['subType']='Dgen Jones'
    else:
        ctb.close()
        raise Exception, 'Not a D?'

    ctb.putinfo(irec)
    ctb.putkeyword('VisCal',irec['subType'])
    ctb.close()


def fixfeedpa(vis,defband='',forceband=''):
    tb.open(vis+'/SPECTRAL_WINDOW')
    spwnames=tb.getcol('NAME')
    tb.close()
    if len(forceband)>0:
        print 'Forcing band = ',forceband
        spwnames[:]=forceband
        defband=forceband
    tb.open(vis+'/FEED',nomodify=F)
    spwids=tb.getcol('SPECTRAL_WINDOW_ID')
    ra=tb.getcol('RECEPTOR_ANGLE')
    ra[:,:]=0.0
    spwmask=(spwids>-1)
    ra[0,spwmask]=[bandpa(spwnames[ispw]) for ispw in spwids[spwmask]]
    spwmask=pl.logical_not(spwmask)
    if (sum(spwmask)>0):
        if (len(defband)>0):
            print 'NB: Setting spwid=-1 rows in FEED table to RECEPTOR_ANGLE for band='+str(defband)
            ra[0,spwmask]=bandpa(defband)
        else:
            print 'NB: Setting spwid=-1 rows in FEED table to RECEPTOR_ANGLE=(0,pi/2)'
    ra[1,:]=ra[0,:]+(pi/2.)
    tb.putcol('RECEPTOR_ANGLE',ra)
    tb.close()


def fillsplitconcat(asdms,outvis,spw='',intent='',field='',dotsys=False,dowvr=False,wvrtie=[],cleanup=True):

    assert type(asdms)==type([]), "Please specify a list for asdms"
    #assert type(spw)==type(""), "Please specify a spw selection string"
    assert type(outvis)==type("") and len(outvis)>0, "Please specify a name for outvis"

    if len(glob.glob(outvis))>0:
        raise Exception, "Found "+outvis+" already generated."

    # a temporary space for the intermediate files
    tmpdir='./FILLSPLITCONCAT_TMP/'
    if len(glob.glob(tmpdir))<1:
        os.mkdir(tmpdir)

    splitlist=[]
    for sdmpath in asdms:
        # fill
        assert len(sdmpath)>0
        sdm=sdmpath.rstrip('/').split('/')[-1]
        assert len(sdm)>0
        fillms=tmpdir+sdm+'.ms'
        # only fill if not already present
        if (len(glob.glob(fillms))<1):
            print 'Filling '+sdmpath+' to '+fillms
            importasdm(asdm=sdmpath,vis=fillms)
        else:
            print 'Found '+fillms+' already filled.'

        if dotsys or dowvr:
            gaintable=[]
            gainfield=[]
            spwmap=[]
            interp=[]
            if dotsys:
                tsysname=fillms+'.tsys'
                if (len(glob.glob(tsysname))<1):
                    gencal(vis=fillms,caltable=tsysname,caltype='tsys')
                gaintable.append(tsysname)
                gainfield.append('nearest')
                spwmap.append(tsysspwmap(vis=fillms,tsystable=tsysname))
                interp.append('linear,linear')
            if dowvr:
                wvrname=fillms+'.wvr'
                if (len(glob.glob(wvrname))<1):
                    wvrgcal(vis=fillms,caltable=wvrname,tie=wvrtie)
                gaintable.append(wvrname)
                gainfield.append('nearest')
                spwmap.append([])
                interp.append('linear')

            applycal(vis=fillms,spw=spw,intent=intent,field=field,
                     gaintable=gaintable,gainfield=gainfield,
                     spwmap=spwmap,interp=interp,calwt=F)


        if (len(spw)>0 or len(intent)>0):
            # only split if spw and/or intent selection specified
            # and not already split
            splitms=tmpdir+sdm+'.split_'+spw.replace('*','')+'_'+intent.replace('*','').replace(',','-')+'.ms'
            if (len(glob.glob(splitms))<1):
                print '  Splitting spw=\''+spw+'\' from '+fillms+' into '+splitms
                dc='data'
                if dotsys or dowvr:
                    dc='corrected'
                if (split(vis=fillms,outputvis=splitms,
                          spw=spw,intent=intent,field=field,
                          datacolumn=dc)):
                    # only if split successful (spw might not select anything or import failed)
                    splitlist.append(splitms)
                else:
                    print 'split failed on '+str(fillms)+'; continuing without it'
            else:
                splitlist.append(splitms)
                print '  Found '+splitms+' already split.'
        else:
            # the filled ms is what we will concat
            splitlist.append(fillms)
    
    if len(splitlist)>1:
        print 'Concat-ing: ',splitlist,' to ',outvis
        concat(vis=splitlist,concatvis=outvis)

    else:
        if len(splitlist[0])>0:
            print 'Renaming: ',splitlist[0],' to ',outvis
            os.rename(splitlist[0],outvis)
        else:
            raise Exception, 'No data was generated by filling/splitting/concat-ing'

    # remove the temporary directory
    if cleanup:
        os.system('rm -Rf '+tmpdir)


def scanbystate(vis,undo=False):
    tb.open(vis,nomodify=False)
    scans=tb.getcol('SCAN_NUMBER')
    states=tb.getcol('STATE_ID')
    print 'Unique STATE_IDs = ',str(pl.unique(states))
    maxstate=states.max()

    if undo:
        d=10**int(floor(log10(scans.min())))
        if d<10:
            tb.close()
            raise Exception, 'Apparently, nothing to undo'
        scans-=states
        scans/=d
        print 'New SCAN_NUMBER = (SCAN_NUMBER - STATE_ID) / '+str(d)
    else:
        m=10**int(floor(log10(states.max())+1.0))
        scans*=m
        scans+=states
        print 'New SCAN_NUMBER = SCAN_NUMBER * '+str(m)+' + STATE_ID'

    tb.putcol('SCAN_NUMBER',scans)
    tb.close()
    
    


