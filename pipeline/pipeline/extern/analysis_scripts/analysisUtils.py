#!/usr/bin/env python
#
"""
This set of libraries, analysisUtils.py, is a collection of often useful
python and/or CASA related functions.  It is an open package for anyone
on the science team to add to and generalize (and commit).  A few practices
will allow us to keep this useful.

1) Write the routines as generally as you can.

2) Before fundamentally changing the default behavior of a function
or class, consider your coworkers.  Do not modify the default behavior
without extreme need and warning.  If you need to modify it quickly,
consider a separate version until the versions can be blended (but please
do try to do the blending!).

3) There is a comment structure within the routines.  Please keep this
for additions because the documentation is automatically generated from
these comments.
 
All examples assume you have imported the library to aU, as import
analysisUtils as aU. You can of course do whatever you like, but the
examples will thus have to be modified.

Thanks and good luck!  If you have any questions, bother Barkats or
Corder, then Robert.
 
S. Corder, 2010-11-07
"""

if 1 :
    import os
    import sys
    import re
    from types import NoneType
    import telnetlib    
    import math
    import numpy as np
    import binascii # used for debugging planet()
    from mpfit import mpfit
    from pylab import *
    import pylab as pb
    from numpy.fft import fft
    import fnmatch, pickle, traceback, copy as python_copy # needed for editIntents
    import scipy as sp
    import scipy.signal as spsig
    import string
    import simutil  # needed for obslist, has useful pad transformation functions
    import plotbandpass as plotbp
    import plotbandpass2 as plotbp2
    import readscans as rs
    import time as timeUtilities
    import datetime
    from scipy import polyfit
    from scipy import optimize # used by class linfit
    import random  # used by class linfit
    from taskinit import *
    import matplotlib.ticker # used by plotWeather
    from matplotlib import rc # used by class linfit
    from matplotlib.figure import SubplotParams
    from matplotlib.ticker import MultipleLocator # used by plotPointingResults
    import commands  # useful for capturing stdout from a system call
    #
    # This arrangement allows you to call casa commands by name: - T. Hunter
    from gencal_cli import gencal_cli as gencal
    from imview_cli import imview_cli as imview
    try:
        from predictcomp_cli import predictcomp_cli as predictcomp
        usePredictComp = True
    except:
        usePredictComp = False
    try:      # CASA 3.4
        from simobserve_cli import simobserve_cli as simobserve
        from simanalyze_cli import simanalyze_cli as simanalyze
    except:   # CASA 3.3
        from sim_observe_cli import sim_observe_cli as simobserve
        from sim_analyze_cli import sim_analyze_cli as simanalyze
    try:
        import solar_system_setjy as sss
        useSolarSystemSetjy = True
    except:
        useSolarSystemSetjy = False
    #
    from importasdm import importasdm
    import fileIOPython as fiop
    from scipy.interpolate import splev, splrep
    import types
    import operator
    import XmlObjectifier
    from plotcal import plotcal
    from split import split
    from setjy import setjy
    from fixvis import fixvis
    from fixplanets import fixplanets
    from xmlrpclib import ServerProxy
    import viewertool
    import casadef

"""
Constants that are sometimes useful.  Warning these are cgs, we might want to change them
to SI given the propensity in CASA to use SI.
"""
h=6.6260755e-27
k=1.380658e-16
c=2.99792458e10
c_mks=2.99792458e8
jy2SI=1.0e-26
jy2cgs=1.0e-23
pc2cgs=3.0857e18
au2cgs=1.4960e13
solmass2g=1.989e33
earthmass2g=5.974e27
solLum2cgs = 3.826e33
mH = 1.673534e-24
G  = 6.67259e-8
Tcmb = 2.725
defaultEphemeris = 'Butler-JPL-Horizons 2010'
JPL_HORIZONS_ID = {'ALMA': '-7',
                   'VLA': '-5',
                   'GBT': '-9',
                   'MAUNAKEA': '-80',
                   'OVRO': '-81'
}

bandDefinitions = {# 1  : [31.3e9, 45e9  ],
                   # 2  : [67e9  , 90e9  ],
                    3  : [84e9  , 116e9 ],
                    4  : [125e9 , 163e9 ],
                   # 5  : [163e9 , 211e9 ],
                    6  : [211e9 , 275e9 ],
                    7  : [275e9 , 370e9 ],
                    8  : [385e9 , 500e9 ],
                    9  : [602e9 , 720e9 ],
                   # 10 : [787e9 , 950e9]
                   }

def version():
    """
    Returns the CVS revision number.
    """
    myversion = "$Id: analysisUtils.py,v 1.1.2.7 2012/11/23 15:03:16 swilliam Exp $"
    return myversion

def getBand(freq) :
    for band in bandDefinitions.keys() :
        if ((freq <= bandDefinitions[band][1]) and (freq >= bandDefinitions[band][0])) :
            return band


# A useful sequence of 19 unique matplotlib colors to cycle through
overlayColors = [
      [0.00,  0.00,  1.00],
      [0.00,  0.50,  0.00],
      [1.00,  0.00,  0.00],
      [0.00,  0.75,  0.75],
      [0.75,  0.00,  0.75],
      [0.25,  0.25,  0.25],
      [0.75,  0.25,  0.25],
#      [0.95,  0.95,  0.00],  yellow
      [0.25,  0.25,  0.75],
      [0.75,  0.75,  0.75],
      [0.00,  1.00,  0.00],
      [0.76,  0.57,  0.17],
      [0.54,  0.63,  0.22],
      [0.34,  0.57,  0.92],
      [1.00,  0.10,  0.60],
      [0.88,  0.75,  0.73],
      [0.10,  0.49,  0.47],
      [0.66,  0.34,  0.65],
      [0.99,  0.41,  0.23]]
overlayColors += overlayColors + overlayColors

def makeList(input) :
    if list not in [type(input)] : return [input]
    else : return input

def locate(msfile):
    """
    Script used to locate an ASDM or an MS file in the RADIO data directory tree.
    """
    host=os.getenv('HOSTNAME')
    if 'gns' in host:
        datadir='/groups/science/data/RADIO'
    elif 'red' in  host:
        datadir="/data/RADIO/"
    else:
        datadir="/data/RADIO/"
        
    if not msfile.find(':') == []:  msfile=msfile.replace('/','_').replace(':','_')
    print msfile

    # first search via locate utility
    a=os.popen('locate %s' %(msfile)).read()
    m=re.search('/data/RADIO/[A-Z]*/.*/[0-9].*/%s/' %msfile,a)
  
    if  type(m) is not(NoneType):
        location=m.group(0)
        print 'using Unix locate'
        location=makeList(location)
    else:
        print 'using Unix find'
        location=os.popen('find  %s -name %s' %(datadir,  msfile)).read().split('\n')
        
    if location != ['']:
        for i in range(size(location)):
            if ('ASDMBinary' not in location[i]):
                location=location[i]
                break
            else:
                print 'could not find this file in local dir or in /data/RADIO'
                return
        dir=location.strip(msfile)
        return [location, dir]
    else:
        print 'could not find this file in local dir or in /data/RADIO'
        return

def psd(d, fsample):
    
    # function to take the psd of a timestream and return also the freq axis and the phase. 
    # input should be 1D
    # fsample in Hz
    
    d = double(d)
    if floor(size(d) / 2) * 2 != size(d):
        d=d[1:]
        
    n = size(d) / 2
    transform = fft(d)
    transform=transform[0:n+1]
    freq = (fsample / 2.0) * (arange(n+1)) / n
    factor = repeat([2.0],n+1)
    factor[0]=1.
    factor[-1]=1.
    spec = sqrt(factor / freq[1]) * transform/ (2*n);
    spec = abs(spec);
        
    return freq,spec


def avpsd(d, fsample, fres,  deg=1):
    # %function [freq,spec,nrep]=avpsd(input, fsample, fres, med, deg)
    # make an average psd by averaging the psd from small segments of resolution fres
  
    n = size(d);
    nout = floor(fsample / fres)
    nrep = floor(n / nout)
    x=arange(nout)
    psdarr = zeros([nrep, nout/2+1])
    print "%i %i" %(nout, nrep)
    
    if (deg !=-1):
        for i in arange(nrep): 
            y=d[i*nout : (i+1)* nout]
            p =polyfit(x,y, deg);
            baseline=polyval(p,x);
            y=y-baseline;
            [freq,ps]=psd(y, fsample);        
            psdarr[i, :] = ps
    else:
        for i in arange(nrep):
            y=d[i*nout : (i+1) * nout]
            [freq,ps]=psd(y, fsample);
            psdarr[i, :] = ps;

    print shape(psdarr)
    spec=sqrt(mean(psdarr*psdarr,0));
    
    return freq, spec

def smooth(x, window_len=10, window='hanning'):
    """
    smooth the data using a window with requested size.
    
    This method is based on the convolution of a scaled window with the signal.
    The signal is prepared by introducing reflected copies of the signal 
    (with the window size) in both ends so that transient parts are minimized
    in the begining and end part of the output signal.
    
    input:
        x: the input signal 
        window_len: the dimension of the smoothing window
        window: the type of window from 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'
            flat window will produce a moving average smoothing.

    output:
        the smoothed signal
        
    example:

    t = linspace(-2,2,0.1)
    x = sin(t)+random.randn(len(t))*0.1
    y = smooth(x)
    
    see also: 
    
    numpy.hanning, numpy.hamming, numpy.bartlett, numpy.blackman, numpy.convolve
    scipy.signal.lfilter
 
    TODO: the window parameter could be the window itself if an array instead of a string   
    """
    import numpy as np

    if x.ndim != 1:
        raise ValueError, "smooth only accepts 1 dimension arrays."

    if x.size < window_len:
        raise ValueError, "Input vector needs to be bigger than window size."

    if window_len < 3:
        return x

    if not window in ['flat', 'hanning', 'hamming', 'bartlett', 'blackman', 'gauss']:
        raise ValueError, "Window is on of 'flat', 'hanning', 'hamming', 'bartlett', 'blackman', 'gauss'"

    s=np.r_[2*x[0]-x[window_len:1:-1], x, 2*x[-1]-x[-1:-window_len:-1]]
    #print(len(s))
    
    if window == 'flat': #moving average
        w = np.ones(window_len,'d')
    elif window == 'gauss':
        w=gauss_kern(window_len)
    else:
        w = getattr(np, window)(window_len)
    y = np.convolve(w/w.sum(), s, mode='same')
    return y[window_len-1:-window_len+1]


# some quick code I added.
def movingStD(a, w):
   """
   Moving standard deviation of array a calculated with window size w
   """
   res=[a[i:i+w].std() for i in range(len(a)-w)]
   return res


def mov_rms(t,d, removeoutlier=True):
    trms=[]
    drms=[]

    if removeoutlier:
        d = remove_outlier(d, sigma_th=10)
    
    s=pl.size(d)
    w=200
    #print s, pl.ceil(s/w)
    
    for i in range(s/w):
        x=pl.arange(w-1)
        y=d[w*i:w*(i+1)-1]
        polycoeffs=pl.polyfit(x, y, 2)
        yfit=pl.polyval(polycoeffs, x)
        #pl.clf()
        #pl.plot(x,y)
        #pl.plot(x,yfit)
        y=y-yfit
        #pl.plot(x,y)
        #raw_input()
           
        trms.append(t[w*(i+1)])
        drms.append(pl.std(y))

    return {'trms': trms, 'rms': drms}


def gauss_kern(size):
    """ Returns a normalized 2D gauss kernel array for convolutions """
    size = int(size)
    x= np.mgrid[-size:size+1]
    g = np.exp(-(x**2/float(size)))
    return g / g.sum()

def onedgaussianplus(x, H,A,dx,w, r):
    #fit a 1D gaussian plus a linear term
    return H+A*np.exp(-(x-dx)**2/(2*w**2))+r*(x-dx)

def onedgaussian(x,H,A,dx,w):
    """
    Returns a 1-dimensional gaussian of form
    H+A*np.exp(-(x-dx)**2/(2*w**2))
    """
    return H+A*np.exp(-(x-dx)**2/(2*w**2))

def onedgaussfit(xax,data,err=None,params=[0,1,0,1,0],fixed=[False,False,False,False,False],limitedmin=[False,False,False,False,False],
        limitedmax=[False,False,False,False,False],minpars=[0,0,0,0,0],maxpars=[0,0,0,0,0],quiet=True,shh=True):
    """
    Inputs:
       xax - x axis
       data - y axis
       err - error corresponding to data

       params - Fit parameters: Height of background, Amplitude, Shift, Width
       fixed - Is parameter fixed?
       limitedmin/minpars - set lower limits on each parameter
       limitedmax/maxpars - set upper limits on each parameter
       quiet - should MPFIT output each iteration?
       shh - output final parameters?

    Returns:
       Fit parameters
       Model
       Fit errors
       chi2
    """

    def mpfitfun(x,y,err):
        if err == None:
            def f(p,fjac=None): return [0,(y-onedgaussianplus(x,*p))]
        else:
            def f(p,fjac=None): return [0,(y-onedgaussianplus(x,*p))/err]
        return f

    if xax == None:
        xax = np.arange(len(data))

    parinfo = [ {'n':0,'value':params[0],'limits':[minpars[0],maxpars[0]],'limited':[limitedmin[0],limitedmax[0]],'fixed':fixed[0],'parname':"HEIGHT",'error':0} ,
                {'n':1,'value':params[1],'limits':[minpars[1],maxpars[1]],'limited':[limitedmin[1],limitedmax[1]],'fixed':fixed[1],'parname':"AMPLITUDE",'error':0},
                {'n':2,'value':params[2],'limits':[minpars[2],maxpars[2]],'limited':[limitedmin[2],limitedmax[2]],'fixed':fixed[2],'parname':"SHIFT",'error':0},
                {'n':3,'value':params[3],'limits':[minpars[3],maxpars[3]],'limited':[limitedmin[3],limitedmax[3]],'fixed':fixed[3],'parname':"WIDTH",'error':0},
                {'n':4,'value':params[4],'limits':[minpars[4],maxpars[4]],'limited':[limitedmin[4],limitedmax[4]],'fixed':fixed[4],'parname':"LINEAR",'error':0}]
    
    mp = mpfit(mpfitfun(xax,data,err),parinfo=parinfo,quiet=quiet)
    mp.status
    mpp = mp.params
    mpperr = mp.perror
    chi2 = mp.fnorm

    if not shh:
        for i,p in enumerate(mpp):
            parinfo[i]['value'] = p
            print parinfo[i]['parname'],p," +/- ",mpperr[i]
        print "Chi2: ",mp.fnorm," Reduced Chi2: ",mp.fnorm/len(data)," DOF:",len(data)-len(mpp)

    return mpp,onedgaussianplus(xax,*mpp),mpperr,chi2

def getChannelAverageData(inputms):
    tb.open(inputms)
    chanAvertb = tb.query('ANTENNA1==0 && ANTENNA2==1 && DATA_DESC_ID==1')
    chanAverData = chanAvertb.getcol('DATA')
    chanAverTime = chanAvertb.getcol('TIME')
    # single pol and single channel:
    visi = chanAverData[0][0]
    chanAverPhase = angle(visi)
    chanAverAmp = absolute(visi)
    tb.close()
    return [chanAverTime, chanAverAmp, chanAverPhase]

def getDataColumnName(inputms):
    tb.open(inputms)
    colnames = tb.colnames()
    if 'FLOAT_DATA' in colnames:
        data_query= 'FLOAT_DATA'
    else:
        data_query = 'DATA'
    tb.close()
    return(data_query)

def getSpectralData(inputms, dd, scanum=[]):
    tb.open(inputms)
    if size(scanum) == 0:
        specTb = tb.query('ANTENNA1==0 && ANTENNA2==1 && DATA_DESC_ID==%d'%(dd))
    else:
        specTb = tb.query('ANTENNA1==0 && ANTENNA2==1 && DATA_DESC_ID==%d && SCAN_NUMBER == %d'%(dd,scanum))
    if 'FLOAT_DATA' in specTb.colnames():
        data_query= 'FLOAT_DATA'
    else:
        data_query='DATA'
    specData = specTb.getcol(data_query)
    specTime = specTb.getcol('TIME')
    date  = int(specTime[0]/ 86400)
    specTime = specTime - date*86400
    return [specTime, specData]

def getSpectralAutoData(inputms,iant, dd, scanum=[]):
    tb.open(inputms)
    if size(scanum) == 0:
        specTb = tb.query('ANTENNA1==%d && ANTENNA2==%d && DATA_DESC_ID==%d'%(iant, iant,dd))
    else:
        specTb = tb.query('ANTENNA1==%d && ANTENNA2==%d && DATA_DESC_ID==%d && SCAN_NUMBER == %d' % (iant, iant ,dd, scanum))
    if 'FLOAT_DATA' in specTb.colnames():
        data_query= 'FLOAT_DATA'
    else:
        data_query='DATA'
    specData = specTb.getcol(data_query)
    specTime = specTb.getcol('TIME')
    date  = int(specTime[0]/ 86400)
    specTime = specTime - date*86400
    return [specTime, specData]

def batchasdm2MS(filelist, ocorr='ca', overw=True):
    """
    Simple wrapper around imortasdm to run it in batch mode
    """
    if not isinstance(filelist, list): filelist=[filelist]
    for file in filelist:
        if not file.find(':') == []:  file=file.replace('/','_').replace(':','_')
        asdmfile=locate(file)[0]
        print asdmfile
        msfile=asdmfile+'.ms'
        if(not os.path.isdir(msfile) or not os.path.isdir(msfile+'/ASDM_FOCUS')):
            print "running ASDM2MS on this file"
            importasdm(asdm=asdmfile, verbose=False, asis='Focus',overwrite=T)
            os.system("rm -Rf %s.flagversions" %msfile)

    return

def getMeasFocus(msfile, aid, scanum):
    
    try:
        print "Focus values from actual measurements at start of Beam map."
        if os.path.isdir(msfile+'/ASDM_FOCUS'):
            tb.open(msfile+'/ASDM_FOCUS')
            antid=tb.getcol('antennaId')
            foc=tb.getcol('measuredFocusPosition')
            asdmtime=tb.getcol('timeInterval')

            q=find(antid== 'Antenna_%i' %aid)
            asdmtime=asdmtime[0,q]
            foc=foc[:,q]
            
            tb.open(msfile+'/ANTENNA')
            antennas=tb.getcol('NAME')
            tb.open(msfile)
            scan=tb.getcol('SCAN_NUMBER')
            t0=tb.getcol('TIME')
            tb.close() 
            f=find(scan==int(scanum))[0]
            scanbeg=t0[f]
           
            # cross reference asdmtime and standard time for start of beam map.
            asdmindex=find(asdmtime > scanbeg)[0]
            focus=zeros([3])
            um=1e6
            antenna=antennas[aid]
            focus[0]=foc[0,asdmindex]
            focus[1]=foc[1,asdmindex]
            focus[2]=foc[2,asdmindex]
            print "%s (X  Y  Z)= (%.0f %.0f %.0f) microns " \
                  % (antenna, focus[0]*um, focus[1]*um, focus[2]*um)
            return focus
        else:
            print "The ms file is missing /ASDM_FOCUS table. Regenerate the MS file with that table. Using batchasdm2MS for exemple"

    except:
        return (NaN, NaN, NaN)

plotOption = {'0-1' : 'b.', '0-2' : 'r.'}

def getAllanVariance(vis,antenna1=None,antenna2=None,spwID=None,param='phase',scan=None,state=None,doPlot=True) :
    if param not in ['phase','real','imag'] : return 'you are a dumb fuck.'
    if spwID == None : spwID = getChanAverSpwIDBaseBand0(vis)
    if antenna2 == None : antenna2 = getAntennaNames(vis)
    else : antenna2 = makeList(antenna2)
    if antenna1 == None : antenna1 = getAntennaNames(vis)
    else : antenna1 = makeList(antenna1)
    data = Visibility(vis,antenna1=antenna1[0],antenna2=antenna2[0],spwID=spwID,scan=scan,state=state)
    aV = {}
    for i in antenna1 :
        for j in antenna2 :
            if i < j :
                data.setAntennaPair(i,j)
                aV[("%s-%s" % (i,j))] = allanVariance(data.phase,data.specTime,data.specFreq.mean())
                if doPlot : pb.plot(aV[("%s-%s" % (i,j))][:,0],np.log10(aV[("%s-%s" % (i,j))][:,1]),'.')
    return aV

def allanVariance(phase,time,ref_freq) :
    Nallan=[]
    ave=[]
    Ndata = len(time)
    Nmax = int(floor(Ndata/2))
    dt = (time[1:]-time[:-1]).mean()
    for i in range(1,Nmax):
        n=0
        y = []
        for j in range(Ndata):
            k=j+2*i
            if k > Ndata-1:
                break
            z=phase[:,:,k]-2*phase[:,:,i+j]+phase[:,:,j]
            y.append(z**2)
            n+=1
        ave.append(mean(y))
        Nallan.append(n)
    Ntau=len(ave)
    Allan=array(Ntau*[2*[0.0]],dtype=float)
    for i in range(Ntau):
        y=[]
        Allan[i][0]=dt*(i+1)
        Allan[i][1]=math.sqrt(ave[i]/2.0/(2*math.pi*ref_freq*Allan[i][0])**2)
    return Allan

def phaseClosure(vis, antenna1, antenna2, antenna3, spw, field, scan):
     antennas=sorted([antenna1,antenna2,antenna3])
     antenna1=antennas[0]
     antenna2=antennas[1]
     antenna3=antennas[2]
     visVal=Visibility(vis,antenna1=antenna1,antenna2=antenna2,spwID=spw,field=field,scan=scan)
     phi01=visVal.phase
     visVal.setAntenna2(antenna3)
     phi02=visVal.phase
     visVal.setAntenna1(antenna2)
     phi12=visVal.phase
     close=phi01+phi02-phi12
     
     return close,visVal

class StuffToLieAbout:
    """
    """
    import os
    
    def __init__(self,calTable=None):
        if calTable <> None :
            self.calTable = calTable
            #self.calTableExplorer = CalTableExplorer(self.calTable,antenna=0,spwID=1)
            self.calTableExplorer = CalTableExplorer(self.calTable,antenna=0)
        else :
            self.calTable = None

    def setCalTable(self,calTable) :
        self.calTable = calTable
        self.calTableExplorer = CalTableExplorer(self.calTable,antenna=0,spwID=1)

    def getCalTable(self) : return self.calTable

    def tsysInSpectralWindow(self,badSpw,goodSpw=None,setValue=False) :
        inputMs  = self.calTableExplorer.inputMs
        badRaw   = Visibility(inputMs,spwID=badSpw, correctedData = False)
        if ((goodSpw == None) and (not setValue)) :
           goodSpw = list(self.calTableExplorer.getSpwID())
           goodSpw.remove(badSpw)[0]
           goodCorr = Visibility(inputMs,spwID=goodSpw,correctedData = True)
           goodRaw  = Visibility(inputMs,spwID=goodSpw,correctedData = False)
           tsysToApply = goodCorr.amp/goodRaw.amp
        elif setValue :
            tsysToApply = setValue*np.ones(tsysToApply.shape,'complex128')
        badCorr  = badRaw.amp*tsysToApply
        self.stuffVisInDataset(badCorr,badSpw)

    def TFBOffset(self,offsetFromOT,inputMs=None,spwIds=None) :
        if inputMs == None and self.calTable == none :
            return "Please identify a visibility dataset."
        elif inputMs == None :
            inputMs = self.calTableExplorer.intputMs
        if offsetFromOT < 1000.0 : return "Please input your offsetFromOT in MHz."        
        vm = ValueMapping(inputMs)
        if spwIds == None :
            spwIds = []
            print "I am assuming you want to fix all FDM modes by the same amount."
            for i in vm.spwInfo.keys() :
                if vm.spwInfo[i]["numChannels"] > 256 : spwIds.append(i)
        else :
            spwIds = makeList(spwIds)
        for i in spwIds :
            tb.open("%s/SPECTRAL_WINDOW" % inputMs,nomodify=False)
            delta = 3000.0-offsetFromOT
            correction = 2.0*delta*vm.spwInfo[i]["sideband"]*1e6
            newChanFreq = vm.spwInfo[i]["chanFreqs"]-correction
            print ("Subtracting %f to your channel frequencies in spectral window %i" % (correction,i) )
            print vm.spwInfo[i]["chanFreqs"].mean(),newChanFreq.mean()
            tb.putcell("CHAN_FREQ",i,newChanFreq)
        tb.close()
        vm.doSpwAndFrequency()

    def doTsysInfo(self) :
        return

    def stuffVisInDataset(self,badCorr) :
        inputMs = self.calTableExplorer.inputMs
        tb.open(inputMs,nomodify=False)
        for i in range(len(badCorr.subtable.rownumbers())) :
            tb.putcell('CORRECTED_DATA',badCorr.subtable.rownumbers()[i],badCorr.specData[...,i])
        tb.close()
    
    def gain(self,antennaToReplace,replacementAntenna=None) :
        if str(antennaToReplace).isdigit() :
            antennaToReplace = self.calTableExplorer.ValueMapping.antennaNamesForAntennaIds[int(antennaToReplace)]
        if replacementAntenna==None :
            antList = list(self.calTableExplorer.ValueMapping.antennaNamesForAntennaIds)
            antList.pop(antennaToReplace)
            replacementAntenna = antList[0]
        elif str(replacementAntenna).isdigit() :
            replacementAntenna = self.calTableExplorer.ValueMapping.antennaNamesForAntennaIds[int(replacementAntenna)]
        print antennaToReplace,replacementAntenna
        antennaToReplace = self.calTableExplorer.ValueMapping.getAntennaIdsForAntennaName(antennaToReplace)
        replacementAntenna = self.calTableExplorer.ValueMapping.getAntennaIdsForAntennaName(replacementAntenna)
        print antennaToReplace,replacementAntenna        
        tb.open(self.calTable,nomodify=False)
        retb  = tb.query('ANTENNA1 == %s' % replacementAntenna)
        rerow = retb.rownumbers()
        badtb = tb.query('ANTENNA1 == %s' % antennaToReplace)
        badrow = badtb.rownumbers()
        for i in range(len(badrow)) :
            _badrow = badrow[i]
            _rerow  = rerow[i]
            goodGain = tb.getcell('GAIN',_rerow)
            tb.putcell('GAIN',_badrow,goodGain)
        tb.close()


class ValueMapping:
    """
    Input: The name of an MS dataset as a string.
    Purpose: This class provides details on the mapping of various parameters to each other.  For example, if you would like to
             know which scans observed a given field source or over which time interval a scan was executed, this is the place to look.
             Included in that are functions which map antenna name to antenna id and field name to field id.  This is useful in building
             other routines that allow you to not require the user to input one or the other type of information.  It also gets unique
             lists of items, like antennas, field sources, scans, intents, etc.

    Responsible: S. Corder and other contributors
    Example: vm = aU.ValueMapping('myInputMS.ms')
    Suggested Improvements:
          (done, 06-04-2011, scorder)1) Change some of the get methods to do methods because they aren't really returning anything
          2) Add spectral window mapping information, spectral windows, spectral windows to fields, spectral windows to scans,
             spectral windows to frequency (central and channelized): Basically make a dictionary of all the spectral line info stuff,
             per spectral window.  Rework combination of SensitivityCalculator and VM....
          3) Add integration time calculator per source
          4) Do sensitivity calculator (maybe needs to be separate function/class that inhereits this)
    """
    def __init__(self,inputMs):
        """
        Instantiation of this class calls this, i.e., vm = aU.ValueMapping('myInputMS.ms').  The dataset name is the only allowed
        input.  It generates the mappings are part of this instantiation.
        """
        self.inputMs = inputMs
        self.setInputMs(self.inputMs)

    def getInputMs(self):
        """
        Input: None
        Output: The active measurement set in the class as a string
        Responsible: S. Corder
        Purpose: Return the name of the active dataset
        """
        return self.inputMs

    def setInputMs(self,inputMs):
        """
        Input: New measurement set that you wish to become the active one, as a string and activate that change to all other parameters
        Output: None
        Responsible: S. Corder
        Purpose: This changes the active dataset and remakes the relevant mappings.  The order of the functions is very
                 important.
        """
        self.inputMs = inputMs
        self.doScanTimeMapping()
        self.doFieldsAndSources()   ;  self.doStatesAndIntents()
        self.doAntennasAndNames()
        self.doScanStateMapping()   ;  self.doFieldTimeMapping() ;
        self.doAntennaTimeMapping() ;  self.doAntennaStateMapping()
        self.doDataDescId()         ; self.doSpwAndDataDescId()
        self.doPolarizations()
        self.doSpwAndFrequency()    
        self.doSpwScanMapping()     ;  self.doSpwFieldMapping()
        self.doSpwIntentMapping()

    def doSpwAndFrequency(self,ignoreWVR=True) :
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose: Creates a dictionary (spwInfo) of spectral windows, with keys of the spectral window number.
                 For each spectral window, another dictionary is formed that has keys of bandwidth, sideband,
                 chanFreqs, chanWidth, numChannels, and meanFreq.
        """
        self.spwInfo = {}
        tb.open("%s/SPECTRAL_WINDOW" % self.inputMs)
        specWinIds = range(tb.nrows())
        junk = []
        for i in specWinIds :
            self.spwInfo[i] = {}
            self.spwInfo[i]["bandwidth"] = tb.getcell("TOTAL_BANDWIDTH",i)
            self.spwInfo[i]["chanFreqs"] = tb.getcell("CHAN_FREQ",i)
            self.spwInfo[i]["chanWidth"] = tb.getcell("CHAN_WIDTH",i)[0]
            self.spwInfo[i]["edgeChannels"] = [min(self.spwInfo[i]["chanFreqs"]),max(self.spwInfo[i]["chanFreqs"])]
            netSideband  = tb.getcell("NET_SIDEBAND",i)
            if netSideband == 2 : self.spwInfo[i]["sideband"] = 1
            else : self.spwInfo[i]["sideband"] = -1
            self.spwInfo[i]["meanFreq"]  = self.spwInfo[i]["chanFreqs"].mean()
            self.spwInfo[i]["numChannels"] = self.spwInfo[i]["chanFreqs"].shape[0]
            if ((ignoreWVR) and (self.spwInfo[i]['numChannels'] == 4)) :
                junk.append(i)
                self.spwInfo.pop(i)
        tb.close()
        if ignoreWVR:
            if (len(junk) > 0):
                print "Ignoring spectral window %s because it is WVR related" % junk

    def doSpwAndDataDescId(self) :
        tb.open("%s/DATA_DESCRIPTION" % self.inputMs)
        self.spwForDataDescId = tb.getcol('SPECTRAL_WINDOW_ID')
        tb.close()

    def doSpwFieldMapping(self) :
        tb.open("%s" % self.inputMs)
        self.fieldsForSpw = {}
        for i in self.spwForDataDescId :
            spw = self.spwForDataDescId[i]
            indices = np.where(self.dataDescId == i)
            self.fieldsForSpw[spw] = np.unique(self.fields[indices])
        tb.close()
        return

    def doDataDescId(self) :
        tb.open("%s" % self.inputMs)
        self.dataDescId = tb.getcol('DATA_DESC_ID')
        tb.close()

    def doPolarizations(self) :
        # Determine the number of polarizations for the first OBSERVE_TARGET intent.
        # Used by plotbandpass for BPOLY plots since the number of pols cannot be inferred
        # correctly from the caltable alone.  You cannot not simply use the first row, because
        # it may be a pointing scan which may have different number of polarizations than what
        # the TARGET and BANDPASS calibrator will have.
        # -- T. Hunter
        myscan = -1
        starttime = timeUtilities.time()
        for s in self.uniqueScans:
            intents = self.getIntentsForScan(s)
            for i in intents:
                if (i.find('OBSERVE_TARGET')>=0):
                    myscan = s
#                    print "First OBSERVE_TARGET scan = ", myscan
                    break
            if (myscan >= 0):
                break
        if (myscan == -1):
            # if there is no OBSERVE_TARGET, then just use the first scan
            myscan = 0
        self.getDataColumnNames()
        tb.open("%s" % self.inputMs)
        if (myscan == 0):
            # assume the first row in the table is for the first scan, to save time
            self.nPolarizations = np.shape(tb.getcell(self.dataColumnName,0))[0]
        else:
            scans = tb.getcol('SCAN_NUMBER')
            self.nPolarizations = 0
            for s in range(len(scans)):
                if (scans[s]==myscan):
                    self.nPolarizations = np.shape(tb.getcell(self.dataColumnName,s))[0]
                    break
        tb.close()
        donetime = timeUtilities.time()
#        print "doPolarizations took %.1f sec" % (donetime-starttime)

    def getDataColumnNames(self):
        tb.open(self.inputMs)
        colnames = tb.colnames()
        self.correctedDataColumnName = ''
        self.modelDataColumnName = ''
        if 'FLOAT_DATA' in colnames:
            self.dataColumnName = 'FLOAT_DATA'
            self.correctedDataColumnName = 'FLOAT_DATA'
        elif 'DATA' in colnames:
            self.dataColumnName = 'DATA'
        if 'CORRECTED_DATA' in colnames:
            self.correctedDataColumnName = 'CORRECTED_DATA'
        if 'MODEL_DATA' in colnames:
            self.modelDataColumnName = 'MODEL_DATA'
        tb.close()
        return

    def doSpwScanMapping(self) :
        tb.open("%s" % self.inputMs)
        self.scansForSpw = {}
        for i in self.spwForDataDescId :
            spw = self.spwForDataDescId[i]
            indices = np.where(self.dataDescId == i)
            self.scansForSpw[spw] = np.unique(self.scans[indices])
        tb.close()
        return
    
    def doSpwIntentMapping(self) :
        tb.open("%s" % self.inputMs)
        self.intentsForSpw = {}
        for i in self.spwForDataDescId :
            spw = self.spwForDataDescId[i]
            indices = np.where(self.dataDescId == i)
            statesForSpw = np.unique(self.states[indices])
            _intent = []
            for i in statesForSpw :
                __intent = []
# The 'if' statement is needed to support telescopes w/o intents. -T. Hunter
                if (len(self.intentsForStates) > 0):
                  for j in self.intentsForStates[i] :
#                    __map = j.split('#')[0]
                    __map = j
                    __intent.append(__map)
                  _intent += __intent
            self.intentsForSpw[spw] = np.unique(np.array(_intent))
        tb.close()


    def getSpwsForIntent(self,intent) :
        spwsForIntent = []
        for i in self.intentsForSpw.keys() :
            if (intent in self.intentsForSpw[i]) : spwsForIntent.append(i)
        return spwsForIntent

    def getIntentsForSpw(self,spw) :
        return self.intentsForSpw[spw]

    def getSpwsForField(self,field) :
        if not str(field).isdigit() : field = self.getFieldIdsForFieldName(field)
        spwsForField = []
        for i in self.fieldsForSpw.keys() :
            if (field in self.fieldsForSpw[i]) : spwsForField.append(i)
        return spwsForField

    def getFieldsForSpw(self,spw,returnName = True) :
        if returnName :
            return self.getFieldNamesForFieldId(np.unique(np.array(self.fieldsForSpw[spw])))
        else :
            return np.unique(np.array(self.fieldsForSpw[spw]))

    def getSpwsForScan(self,scan):
        spwsForScan = []
        for i in self.scansForSpw.keys() :
            if (scan in self.scansForSpw[i]) : spwsForScan.append(i)
        return spwsForScan

    def getScansForSpw(self,spw) :
        return self.scansForSpw[spw]

    def getAntennaNamesForAntennaId(self,id):
        """
        Input: Antenna id as an integer or string
        Output: Antenna name as a string
        Responsible: S. Corder
        Purpose: Allows translation between antenna id and antenna name.
        """

        return self.antennaNamesForAntennaIds[int(id)]

    def getAntennaIdsForAntennaName(self,antennaName):
        """
        Input: Antenna names as a string
        Output: Antenna index as an integer.
        Responsible: S. Corder
        Purpose: This allows translation between antenna name and antenna id
        """

        return np.where(self.antennaNamesForAntennaIds == antennaName)[0][0]

    def doStatesAndIntents(self):
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose: This function defines two attributes, uniqueStates, which is a python list of the different intents present in the dataset,
                 and intentsForStates is another python list which give the intents for state id as a nested list.  The first index of
                 the intentsForStates is the state id.  If you choose a state id, then the result is a list of intents for that state.
        """
        tb.open("%s/STATE" % self.inputMs)
        intents = tb.getcol("OBS_MODE")
        tb.close()
        _intents = []
        for i in intents : _intents.append(i.split(','))
        self.intentsForStates = _intents
        self.uniqueIntents = []
        for i in self.intentsForStates : self.uniqueIntents.extend(i)
        self.uniqueIntents = np.unique(np.array(self.uniqueIntents))

    def doFieldsAndSources(self):
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose: This function defines two attributes, uniqueField and fieldNamesForFieldIds.  For the time being these are identical.
                 fieldNamesForFieldIds is simply a numpy array where the index is the field id and the value is the name of the field source.
        """
        tb.open("%s/FIELD" % self.inputMs)
        self.fieldNamesForFieldIds = tb.getcol('NAME')
#        print '%d field names = '%len(self.fieldNamesForFieldIds), self.fieldNamesForFieldIds
        self.uniqueFields = self.fieldNamesForFieldIds
        tb.close()

    def doAntennasAndNames(self) :
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose: This function defines two attributes, uniqueAntennas (which is a little excessive) and antennaNamesForAntennaIds.
                 antennaNamesForAntennaIds is a numpy array and has indices that are the antenna ids and values that are the antenna names.
        """
        tb.open("%s/ANTENNA" % self.inputMs)
        self.antennaNamesForAntennaIds = tb.getcol('NAME')
        self.uniqueAntennas = np.unique(self.antennaNamesForAntennaIds)
        self.numAntennas = len(self.uniqueAntennas)
        tb.close()

    def doScanStateMapping(self):
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose: This function defines an attribute, statesForScans, that is the mapping between states and scan numbers.  It is
                 a python dictionary that has keys of the scan number and values, in a list, of the states used in that scan.
        """
        tb.open("%s" % self.inputMs)
        self.states = tb.getcol("STATE_ID")
        tb.close()
        self.statesForScans = {}
        for i in self.uniqueScans :
            indices = np.where(self.scans == i)
            self.statesForScans[i] = np.unique(self.states[indices])

    def doScanTimeMapping(self):
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose: This function defines four attributes, scans, time, uniqueScans and scansForTiems.  scans and time are simply
                 the scan number and time table from the main data table as python arrays.  uniqueScans is a numpy array of the independent scans
                 in the table.  scansForTimes is a python dictionary with keys as scans and values as times in which data was taken
                 for that scan.
        """
        tb.open(self.inputMs)
        self.scans = tb.getcol('SCAN_NUMBER')
        self.time = tb.getcol('TIME')
        tb.close()
        self.uniqueScans = np.unique(self.scans)
        self.scansForTimes = {}
        for i in self.uniqueScans :
            indices = np.where(self.scans == i)
            self.scansForTimes[i] = self.time[indices]

    def doFieldTimeMapping(self):
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose: This function defines two attributes, fields, a numpy array, and fieldsForTimes, a dictionary with keys of field name.
                 The fields is just the field id from the data table.  The values of fieldsForTimes are the times during which data was
                 collected for that field source.
        """
        
        tb.open(self.inputMs)
        self.fields = tb.getcol('FIELD_ID')
        tb.close()
        self.fieldsForTimes = {}
        for i in range(len(self.fieldNamesForFieldIds)) :
            indices = np.where(self.fields == i)
            self.fieldsForTimes[self.fieldNamesForFieldIds[i]] = self.time[indices]

    def doAntennaTimeMapping(self):
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose: This function defines three attributes. antenna1 and antenna2 are numpy arrays containing the antenna1 and antenna2 columns
                 from the data table.  antennasForTimes defines the times over which data was collected for that antenna.  It is a python
                 dictionary with keys of baseline (using antenna names) and values of numpy array of times.
        """

        tb.open(self.inputMs)
        self.antennas1 = tb.getcol('ANTENNA1')
        self.antennas2 = tb.getcol('ANTENNA2')
        tb.close()
        self.antennasForTimes = {}
        for i in range(len(self.uniqueAntennas)) :
            for j in range(len(self.uniqueAntennas)) :
                if i <= j :
                    antennaKey = "%s-%s" % (str(self.uniqueAntennas[j]),str(self.uniqueAntennas[i]))
                    indices = np.where((self.antennas1 == self.uniqueAntennas[i] and self.antennas2 == self.uniqueAntennas[j]))
                    self.antennasForTimes[antennaKey] = self.time[indices]

    def doAntennaStateMapping(self):
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose: This function defines one attribute, antennasForStates, a python dictionary.  The keys are baselines (using
                 antenna names) and values are staes used for that baseline.  Usually the autocorrelations are most useful.
        """

        self.antennasForStates = {}
        for i in range(len(self.uniqueAntennas)) :
            for j in range(len(self.uniqueAntennas)) :
                if i <= j :
                    antennaKey = "%s-%s" % (str(self.uniqueAntennas[j]),str(self.uniqueAntennas[i]))
                    indices = np.where((self.antennas1 == self.uniqueAntennas[i] and self.antennas2 == self.uniqueAntennas[j]))
                    self.antennasForStates[antennaKey] = self.states[indices]

    
    def getScansForTime(self,time,fudge=0.0):
        """
        Input: Time stamp in CASA native units as a string or float
        Output: Scan number associated with that time stamp.
        Responsible: S. Corder
        Purpose: This function returns the scan number for a specific timestamp.  It allows tranlsation between time and scan.
        """
        for i in self.scansForTimes.keys() :
            if ((float(time) >= self.scansForTimes[i][0]-fudge) and (float(time) <= self.scansForTimes[i][-1]+fudge)) :
                return i

    def getTimesForScans(self,scans):
        """
        Input: Scan number as an integer or string
        Output: Time range over which data exists for that scan as a numpy array.
        Responsible: S. Corder, copied from getTimesForScan and modified by T. Hunter
        Purpose: Return the times associated with a given timestamp.  This allows translation between scan and time.
        """
        times = []
        if (type(scans) == int or type(scans) == np.int32):
            scans = [scans]
        for scan in scans:
            times.append(self.scansForTimes[int(scan)])
        return (times)

    def getTimesForScan(self,scan):
        """
        Input: Scan number as an integer or string
        Output: Time range over which data exists for that scan as a numpy array.
        Responsible: S. Corder
        Purpose: Return the times associated with a given timestamp.  This allows translation between scan and time.
        """
        return self.scansForTimes[int(scan)]

    def getScansForState(self,state):
        """
        Input: State id as an integer or string
        Output: The scan numbers, as a list, that use that specific state.
        Responsible: S. Corder
        Purpose: Return the scans that used a specific state.  This allos translation between state and scan.
        """

        scansForState = []
        for i in self.uniqueScans :
            if int(state) in self.statesForScans[i] : scansForState.append(i)
        return scansForState

    def getStatesForScan(self,scan):
        """
        Input: Scan number as a string or integer
        Output: States used during that scan.
        Responsible: S. Corder
        Purpose: Returns the states used during a given scan.  This allows translation between scan and state
        """

        return self.statesForScans[int(scan)]

    def getIntentsForScan(self,scan) :
        """
        Input: Scan as an integer or string.
        Output: Intent as a an array of strings with the names of the intents as values.
        Responsible: S. Corder
        Purpose: This returns the intents used in a specific scan allowing translation between scan and intent.
        """

        intentsForScan = []
        for i in range(len(self.intentsForStates)) :
            subIntents = self.intentsForStates[i]
            if int(scan) in self.getScansForState(i) : intentsForScan.extend(subIntents)
        return np.unique(intentsForScan)
            
    def getScansForIntent(self,intent) :
        """
        Input: Intent (as a string)
        Output: A numpy array of scans using the input intent.
        Responsible: S. Corder
        Purpose: This returns the scans using a specific intent.  This allows flagging based on intent and translation
                 between intent and scan.
        """

        scansForIntent = []
        for i in range(len(self.states)) :
            if intent in self.intentsForStates[self.states[i]] :
                scansForIntent.extend(self.getScansForState(self.states[i]))
        return np.unique(scansForIntent)
        
    def getScansForFieldID(self,field):
        """
        Input: Field, as an id.
        Output: Scans using that field
        Responsible: T. Hunter
        Purpose: This takes a field ID and tells you what scans it was used in.  It was
                 created to avoid a strange behavior of getScansForField for integer inputs.
        """
        indices = np.where(self.fields == field)
        return np.unique(self.scans[indices])
        
    def getScansForField(self,field):
        """
        Input: Field, as a name or id.
        Output: Scans using that field
        Responsible: S. Corder
        Purpose: This takes a field source and tells you what scans it was used in.
        """

        if not str(field).isdigit() : field = self.getFieldIdsForFieldName(field)
        indices = np.where(self.fields == field)
        return np.unique(self.scans[indices])

    def getFieldsForScans(self,scans,returnName=True):
        slist = []
        for scan in scans:
            slist.append(self.getFieldsForScan(scan))
        return([item for sublist in slist for item in sublist])
    
    def getFieldsForScan(self,scan,returnName=True):
        """
        Input: Scan as an integer or string
        Output: Field ids observed during that scan.
        Responsible: S. Corder
        Purpose: This takes a scan number and returns a field observed during that scan.  This allows translation between
                 scan and field.
        """

        indices = np.where(self.scans == int(scan))
        if returnName : return self.getFieldNamesForFieldId(np.unique(self.fields[indices]))
        else : return np.unique(self.fields[indices])

    def getFieldsForIntent(self,intent,returnName=True):
        """
        Input: intent as a string
        Output: field id as integer or array of names
        Responsible: S. Corder
        Purpose: This retrieves all of the fields that have been assigned a given intent during an observation.  
        """        
        _fields = []
        scans = self.getScansForIntent(intent)
        for i in scans :
            _field = self.getFieldsForScan(i)
            if _field not in _fields : _fields.append(_field)
        if returnName :
            return _fields
        else :
            return self.getFieldIdsForFieldName(_fields)
        
    def getFieldIdsForFieldName(self,sourceName):
        """
        Input: source name as string
        Output: field id as integer (actually it is returning the source id-Todd)
        Responsible: S. Corder
        Purpose: This translates between source/field name and field id.
        """
# The following fails because the case varies when .title() is applied: QSO vs. Qso, and TW Hya vs Tw Hya
#        return np.where(upper == sourceName.title())[0][0]
#        print "looking for %s in " % (sourceName), self.fieldNamesForFieldIds
        return np.where(self.fieldNamesForFieldIds == sourceName)[0]
        
    def getFieldNamesForFieldId(self,sourceId):
        """
        Input: field id (as string or integer)
        Output: field name
        Responsible: S. Corder
        Purpose: This translates between field id and field/source name.
        """
        if (type(sourceId) == int or type(sourceId) == np.int32 or type(sourceId) == np.int64 or type(sourceId) == str):
            if (len(self.fieldNamesForFieldIds) > int(sourceId)):
                # prevent "index out of bounds" error if field is not present 
                return self.fieldNamesForFieldIds[int(sourceId)]
            else:
                return (None)
        else:
            # Todd added this check which was necessary for the Antennae Band 7 mosaic
            return [self.fieldNamesForFieldIds[s] for s in sourceId]

    def getFieldsForTime(self,time,returnName=True):
        """
        Input: Time in casa native units, returnName (boolean).  If returnName is true, the name is returned, else the id is returned.
               Default is returnName=True
        Output: Field name or id (depending on value of returnName).
        Responsible: S. Corder
        Purpose: Allows the field id/name to be returned for a specific observation time.
        """

        for i in self.fieldsForTimes.keys() :
            if (time in self.fieldsForTimes[i]) :
                if returnName : return i
                else : return self.getFieldNamesForFieldId(i)

    def getTimesForField(self,field):
        """
        Input: Field name or id (as a string or integer)
        Output: Times as a numpy array over which that field was observed in casa native units.
        Responsible: S. Corder
        Purpose: This allows you to determine the data time stamps for observations of a specific field source.
        """

        if str(field).isdigit() : field = self.fieldNamesForFieldIds(int(field))
        return self.fieldsForTimes[field]


class TsysExplorer:
    """ Put something in about updating the Tsys on TDM observations in real time.  """
    def __init__(self,inputMs,antenna=None,spwID=None,autoSubtableQuery=True,queryString='',cross_auto_all='all'):
        if autoSubtableQuery==False and queryString=='' : return 'Must either automatically generate the (autoSubtableQuery=True) or provide a subtable query string (queryString)'
        self.inputMs = inputMs
        self.valueMapping = ValueMapping(inputMs)
        if antenna == None :
            antenna = self.valueMapping.getAntennaNamesForAntennaId(0)
        self.antenna = antenna
        self.checkAntenna()
        if spwID == None :
            spwID = getChanAverSpwIDBaseBand0(inputMs)
        self.spwID = spwID
        self.elevation = None
        self.time      = None
 #       self.tcal      = None
        self.trx       = None
        self.tsky      = None
        self.tsys      = None
        self.freq      = None
        self.elevTime  = None
        self.scan      = []
        self.field     = []
        self.autoSubtableQuery = autoSubtableQuery
        self.queryString = queryString
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def getScanAndField(self):
        for i in self.time : 
            self.scan.append(self.valueMapping.getScansForTime(i))
            self.field.append(self.valueMapping.getFieldsForTime(i))

    def restrictTimeField(self):
        return

    def getElevation(self) :
        tb.open("%s/POINTING" % self.inputMs)
        subtable = tb.query("ANTENNA_ID == %s" % self.antenna)
        tb.close()        
        self.elevation = subtable.getcol("DIRECTION")
        self.elevTime  = subtable.getcol("TIME")
        elev = []
        for i in self.time :
            diffTime = abs(self.elevTime - i)
            indy = np.where(diffTime == diffTime.min())
            elev.append(self.elevation[...,indy[0][0]]*180.0/math.pi)
        elev = np.array(elev)
#        print elev.shape,elev
        self.elevation = elev[:,1]

    def getFreq(self) :
        tb.open("%s/SPECTRAL_WINDOW" % self.inputMs)
        rows = tb.selectrows(self.spwID)
        freqs = rows.getcol("CHAN_FREQ")
        tb.close()
        self.freq = freqs
        
    def getTsysData(self) :
        self.time = self.subtable.getcol("TIME")-self.subtable.getcol("INTERVAL")/2.0
#        self.tcal = self.subtable.getcol("TCAL_SPECTRUM")
        self.trx  = self.subtable.getcol("TRX_SPECTRUM")
        self.tsky = self.subtable.getcol("TSKY_SPECTRUM")
        self.tsys = self.subtable.getcol("TSYS_SPECTRUM")

    def getAntenna(self) : return self.antenna

    def setAntenna(self,antenna) :
        self.antenna = antenna
        self.checkAntenna()
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def getInputMs(self) : return self.inputMs

    def setInputMs(self,inputMs) :
        self.inputMs = inputMs
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()
        self.ValueMapping.setInputMs(inputMs)

    def checkAntenna(self) :
        if self.antenna <> None : 
            self.antenna = str(self.antenna)
            if not self.antenna.isdigit() : self.antenna = getAntennaIndex(self.inputMs,self.antenna)

    def getSpwID(self) : return self.spwID

    def setSpwID(self,spwID) :
        self.spwID = spwID
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def makeSubtableQuery(self) :
        self.parameterList = []
        queryString = ''
        if self.antenna <> None : self.parameterList.append('ANTENNA_ID == %s' % self.antenna)
#        if self.field <> None    : self.parameterList.append('FIELD_ID == %s' % self.field)
        if self.spwID <> None    : self.parameterList.append('SPECTRAL_WINDOW_ID == %s' % self.spwID)
#        if self.state <> None    : self.parameterList.append('STATE_ID == %s' % self.state)
#        if self.scan <> None     : self.parameterList.append('SCAN_NUMBER == %s' % self.scan)
        for i in self.parameterList : queryString = self.appendQuery(queryString,i)
        self.queryString = queryString

    def appendQuery(self,queryString,additive) :
        if queryString == '' :
            if additive == '' : return queryString
            else : return additive
        else :
            if additive == '' : return queryString
            else : 
                queryString = queryString + ' && ' + additive
                return queryString

    def makeAutoSubtable(self) :
        self.checkAntenna()
        self.makeSubtableQuery()
        tb.open("%s/SYSCAL" % self.inputMs)
        self.subtable = tb.query(self.queryString)
        tb.close()
        self.getTsysData()
        self.getFreq()
        #self.getElevation()
        self.getScanAndField()

    def makeSubtable(self) :
        tb.open("%s/SYSCAL" % self.inputMs)
        self.subtable = tb.query(self.queryString)
        tb.close()
        self.getTsysData()
        self.getFreq()
        #self.getElevation()
        self.getScanAndField()

    def setAutoSubtableQuery(self,autoSubtableQuery) :
        self.autoSubtableQuery = autoSubtableQuery

    def getAutoSubtableQuery(self) : return self.autoSubtableQuery
        
class SysCal:
    def __init__(self,inputMs,antenna=None,spwID=None,scan=None):
        if spwID == None :
            spwID = getChanAverSpwIDBaseBand0(inputMs)
        self.inputMs = inputMs
        self.spwID = spwID
        tb.open(inputMs)
        self.antenna = antenna
        if self.antenna <> None : self.antenna    = str(antenna)
        self.checkAntenna()
        if cross_auto_all.lower() in ['cross','auto','all'] :
            self.cross_auto_all = cross_auto_all
        else :
            return "Improper value for cross_auto_all, please select, cross, auto or all."
        self.field       = field
        self.correctedData = correctedData
#        if not self.field.isdigit() : self.field = self.fieldNameToID(self.field)
        self.scan        = scan
        self.autoSubtableQuery = autoSubtableQuery
        self.queryString = queryString
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()
        self.getSpectralData()
        self.getAmpAndPhase()
#        self.time
        tb.close()

    def checkAntenna(self) :
        if self.antenna <> None : 
            self.antenna = str(self.antenna)
            if not self.antenna.isdigit() : self.antenna1 = getAntennaIndex(self.inputMs,self.antenna)

    def getSpwID(self) : return self.spwID

    def setSpwID(self,spwID) :
        self.swpID = swpID
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def makeSubtableQuery(self) :
        self.parameterList = []
        queryString = ''
        if self.antenna <> None : self.parameterList.append('ANTENNA_ID == %s' % self.antenna)
#        if self.field <> None    : self.parameterList.append('FIELD_ID == %s' % self.field)
        if self.spwID <> None    : self.parameterList.append('SPECTRAL_WINDOW_ID == %s' % spwID)
#        if self.state <> None    : self.parameterList.append('STATE_ID == %s' % self.state)
#        if self.scan <> None     : self.parameterList.append('SCAN_NUMBER == %s' % self.scan)
        for i in self.parameterList : queryString = self.appendQuery(queryString,i)
        self.queryString = queryString

    def appendQuery(self,queryString,additive) :
        if queryString == '' :
            if additive == '' : return queryString
            else : return additive
        else :
            if additive == '' : return queryString
            else : 
                queryString = queryString + ' && ' + additive
                return queryString

    def makeAutoSubtable(self) :
        self.checkAntenna()
        self.makeSubtableQuery()
        tb.open("%s/SYSCAL" % self.inputMs)
        self.subtable = tb.query(self.queryString)
        tb.close()
        self.getTsysData()
        self.getFreq()
        self.getElevation()
        self.getScanAndField()

    def makeSubtable(self) :
        tb.open("%s/SYSCAL" % self.inputMs)
        self.subtable = tb.query(self.queryString)
        tb.close()
        self.getTsysData()
        self.getFreq()
        self.getElevation()
        self.getScanAndField()

    def setAutoSubtableQuery(self,autoSubtableQuery) :
        self.autoSubtableQuery = autoSubtableQuery

    def getAutoSubtableQuery(self) : return self.autoSubtableQuery
        
    def checkAntenna(self) :
        if self.antenna <> None : 
            self.antenna = str(self.antenna)
            if not self.antenna.isdigit() : self.antenna = getAntennaIndex(self.inputMs,self.antenna)

def plotWeeklyTrx(uid,showPlots=False) :
    msFile=uid+'.ms'
    vm = ValueMapping(msFile)
    tsys = TsysExplorer(msFile,antenna=0,spwID=1)
    tsysSpw = vm.getSpwsForIntent('CALIBRATE_ATMOSPHERE#OFF_SOURCE')[1:]
    figureIndex = 0
    band = getBand(tsys.freq.mean())
    ObsDate=qa.time({'value' : vm.time[0],'unit': 's'}, form=['ymd'])
    fnDate=(''.join([str(l) for l in (ObsDate.split(':')[0].split('/'))]))
    fnDate2=(''.join([str(l) for l in ((''.join([str(l2) for l2 in (ObsDate.split('/'))])).split(':'))]))
    fDir=('/data/RADIO/TRX/AOS/%.8s' %(fnDate))
    if not os.path.exists(fDir): os.makedirs(fDir)
    F = open(fDir+'/TrxRB%s_%s.txt' %(band,fnDate2),'w')
    print >> F, ('%s %s ALMARB_%s' % (msFile, ObsDate, band))
    print >> F, ('Mean Freq. Mean Trx  Std. Dev.')
    print >> F, ('GHz.       K         K')
    badAnts = []
    for antenna in vm.uniqueAntennas :
        figureIndex +=1 
        if showPlots: pb.figure(figureIndex)
        meanVals = []
        meanFreq = []
        stdVals  = []
        for spw in tsysSpw :
            if vm.spwInfo[spw]['numChannels'] == 128 :
                tsys.setSpwID(int(spw))
                tsys.setAntenna(antenna)
                meanFreq.append(tsys.freq.mean())
                meanVals+=(list(tsys.trx[:,3:125].mean(1).transpose()[0]))
                stdVals+=(list(tsys.trx[:,3:125].std(1).transpose()[0]))
                if showPlots:
                    pb.plot(tsys.freq[3:125]/1e9,tsys.trx[0,3:125],'g.')
                    pb.plot(tsys.freq[3:125]/1e9,tsys.trx[1,3:125],'b.')
        print >> F, antenna
        print >> F,('\n'.join(['%6.2f'%round(float(l1/1e9),2)+"     "+'%6.2f'%round(float(l2),2)+"     "+'%6.2f'%round(float(l3),2) for (l1,l2,l3) in zip(meanFreq,meanVals,stdVals)]))
#        specVal100 = {3: 60, 6: 136, 7: 219, 9: 261} #This is specification over 100% of the bands
        specVal80 = {3: 45, 6: 83, 7: 147, 9: 175} #This is specification over 80% of the bands

        if band == 3 : spec = specVal80[3]
        elif band == 6 : spec = specVal80[6]
        elif band == 7 : spec = specVal80[7]
        elif band == 9 : spec = specVal80[9]
        else: return 'Not a valid weekly Trx testing Frequency'
        for i in meanVals :
            if i >= spec :
                badAnts.append(antenna)
        if showPlots:
            pb.plot((100*np.arange(len(meanFreq))),(spec*np.ones(len(meanFreq))),'r-')
            pb.xlim(meanFreq[0]/1e9-10,meanFreq[len(meanFreq)-1]/1e9+10)
            pb.xlabel('Frequency (GHz)')
            pb.ylabel('Trx (K)')
            pb.suptitle('%s %s %s' % (antenna,qa.time({'value' : vm.time[0],'unit': 's'},form=['ymd']),msFile), size='14')
            pb.title('Green--Pol0   Blue--Pol1   Red--Specification(80%)', size='12')
            pb.savefig(fDir+'/TrxRB%s%s_%s.png' % (band,antenna,fnDate2))
    print >> F, "Problematic Antennas: %s" % str(badAnts)
    F.close()    
    print "Antennas: %s seem to have problems." % str(badAnts)
    raw_input("Hit Return to quit: ")
#    pb.close('all')
#Keep on having problem with close all
    fig_numbers = [x.num
               for x in matplotlib._pylab_helpers.Gcf.get_all_fig_managers()]
    for n in fig_numbers: pb.close(n)

class CalTableExplorer:
    """Stuff: Only works for antenna based solutions"""
    """Stuff to add: start and end channels (started, but need to make it consaistent, good metrics for differences"""
    """plotting routines, interpolation? In freq and/or time?  Basics are done"""
    """I need to make the residual functions make sense, I think the names are screwy. """
    
    def __init__(self,inputTable,antenna=None,spwID=None,feed=None,scan=None,state=None,field=None,autoSubtableQuery=True,queryString='',startChan=None,endChan=None):
        self.inputTable   = inputTable
        self.inputMs      = self.getMS_NAME(self.inputTable)
        self.ValueMapping = ValueMapping(self.inputMs)
        self.antenna     = antenna
        self.checkAntenna()
        self.startChan   = startChan
        self.endChan     = endChan
        self.spwID        = spwID
        self.feed         = feed
        self.scan         = scan
        self.state        = state
        self.field        = field
        self.checkField()
        self.getCalDescSpwIDMapping()
        if spwID == None : self.spwID = self.calDescSpwIDMapping[0]
        self.autoSubtableQuery = autoSubtableQuery
        self.queryString  = queryString
        if self.autoSubtableQuery == True :
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def getFreq(self) :
        tb.open("%s/SPECTRAL_WINDOW" % self.inputMs)
        rows = tb.selectrows(self.spwID)
        freqs = rows.getcol("CHAN_FREQ")
        tb.close()
        self.freq = freqs

    def getTimeAndInterval(self) :
        self.time     = self.subtable.getcol("TIME")
        self.mjd      = mjdSecondsVectorToMJD(self.time)
        self.ut       = mjdVectorToUTHours(self.mjd)
        self.interval = self.subtable.getcol("INTERVAL")

    def getFit(self) :
        self.gain     = self.subtable.getcol("GAIN")
        self.solOkay  = self.subtable.getcol("SOLUTION_OK")
        self.flags    = self.subtable.getcol("FLAG")
        self.fit      = self.subtable.getcol("FIT")
        self.snr      = self.subtable.getcol("SNR")
        self.real     = np.real(self.gain)
        self.imag     = np.imag(self.gain)
        self.phase = np.arctan2(self.imag,self.real)
        self.amp   = abs(self.gain)

    def plotFit(self,pol,xtype=''):
        date = (mjdSecondsToMJDandUT(self.time[0])[1]).split()[0]
        if (xtype=='ut'):
            pb.plot(self.ut,self.gain[pol,:,:][0],'b.')
            pb.xlabel('UT (hours)')
        elif (xtype=='mjd'):
            pb.plot(self.mjd,self.gain[pol,:,:][0],'b.')
            pb.xlabel('MJD days')
        else:
            pb.plot(self.time,self.gain[pol,:,:][0],'b.')
            pb.xlabel('MJD seconds')
        pb.ylabel('Gain')
        pb.title(date)
        
    def timeAverageSolutions(self) :
        self.tavgGain  = self.gain.mean(-1)
        self.tavgReal  = self.tavgGain.real
        self.tavgImag  = self.tavgGain.imag
        self.tavgPhase = np.arctan2(self.tavgImag,self.tavgReal)
        self.tavgAmp   = abs(self.tavgGain)
        
    def freqAverageSolutions(self) :
        self.favgGain  = self.gain[...,self.startChan:self.endChan,...].mean(1)
        self.favgReal  = self.favgGain.real
        self.favgImag  = self.favgGain.imag
        self.favgPhase = np.arctan2(self.favgImag,self.favgReal)
        self.favgAmp   = abs(self.favgGain)

    def generateSpectralResiduals(self) :
        self.fresidGain = self.gain
        for i in range(self.gain.shape[-1]) :
            self.fresidGain[:,:,i] = self.fresidGain[:,:,i]-self.tavgGain
        self.fresidGain = self.fresidGain
        self.fresidReal = self.fresidGain.real
        self.fresidImag = self.fresidGain.imag
        self.fresidPhase = np.arctan2(self.fresidImag,self.fresidReal)
        self.fresidAmp   = abs(self.fresidGain)

    def unwrapPhase(self,simple=True) :
        from math import pi
        phaseShape = self.phase.shape
        for i in range(phaseShape[2]-1) :
            diff = self.phase[:,:,i]-self.phase[:,:,i+1]
            _diffg = (diff > 1.*pi)*2*pi
            _diffl = (diff < -1.*pi)*2*pi
            self.phase[:,:,i+1] = self.phase[:,:,i+1]+_diffg-_diffl

    def generateTimeResiduals(self) :
        self.tresidGain = self.gain
        for i in range(self.gain.shape[1]) :
            self.tresidGain[:,i,:] = self.tresidGain[:,i,:]-self.favgGain
        self.tresidGain = self.tresidGain[...,self.startChan:self.endChan,...]
        self.tresidFreq = self.freq[self.startChan:self.endChan]
        self.tresidReal = self.tresidGain.real
        self.tresidImag = self.tresidGain.imag
        self.tresidPhase = np.arctan2(self.tresidImag,self.tresidReal)
        self.tresidAmp   = abs(self.tresidGain)

    def getMS_NAME(self,inputTable) :
        tb.open("%s/CAL_DESC" % inputTable)
        msFiles = tb.getcol("MS_NAME")
        tb.close()
        return np.unique(msFiles)[0]

    def getCalDescForSpwID(self,calDesc) :
        try:
            return np.where(self.calDescSpwIDMapping[0] == self.spwID)[0][0]
        except:
            print 'The identified spwID does not have a solution in this table.'
            sys.exit()
        
    def getCalDescSpwIDMapping(self) :
        tb.open("%s/CAL_DESC" % self.inputTable)
        self.calDescSpwIDMapping = tb.getcol("SPECTRAL_WINDOW_ID")
        tb.close()

    def setField(self,field) :
        self.field = field
        self.checkField()
        if self.autoSubtableQuery : self.makeAutoSubtable()

    def getField(self) : return self.field

    def checkField(self) :
        if self.field <> None : 
            self.field = str(self.field)
            if not self.field.isdigit() : self.field = self.ValueMapping.getFieldIdsForFieldName(self.field)[0]

    def checkAntenna(self) :
        if self.antenna <> None : 
            self.antenna = str(self.antenna)
            if not self.antenna.isdigit() : self.antenna = getAntennaIndex(self.inputMs,self.antenna)

    def getAntenna(self) : return self.antenna

    def setAntenna(self,antenna) :
        self.antenna = antenna
        self.checkAntenna()
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def getInputTable(self) : return self.inputTable

    def setInputTable(self,inputMs) :
        self.inputTable = inputTable
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()
        self.ValueMapping.setInputMs(inputMs)

    def getSpwID(self) : return self.spwID

    def setSpwID(self,spwID) :
        self.spwID = spwID
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def setScan(self,scan) :
        self.scan = scan
        if self.autoSubtableQueyry == True :
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def setFeed(self,feed) :
        self.feed = feed
        if self.autoSubtableQueyry == True :
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def setState(self,state) : 
        self.state = state
        if self.autoSubtableQueyry == True :
            self.makeAutoSubtable()
        else : self.makeSubtable()

    def getScan(self) : return self.scan

    def getFeed(self) : return self.feed

    def getState(self) : return self.state

    def makeSubtableQuery(self) :
        self.parameterList = []
        queryString = ''
        if self.antenna <> None : self.parameterList.append('ANTENNA1 == %s' % self.antenna)
        if self.field <> None    : self.parameterList.append('FIELD_ID == %s' % self.field)
        if self.spwID <> None    : self.parameterList.append('CAL_DESC_ID == %s' % self.getCalDescForSpwID(self.spwID))
        if self.state <> None    : self.parameterList.append('STATE_ID == %s' % self.state)
        if self.scan <> None     : self.parameterList.append('SCAN_NUMBER == %s' % self.scan)
        if self.feed <> None     : self.parameterList.append('FEED_ID == %s' % self.feed)
        for i in self.parameterList : queryString = self.appendQuery(queryString,i)
        self.queryString = queryString

    def appendQuery(self,queryString,additive) :
        if queryString == '' :
            if additive == '' : return queryString
            else : return additive
        else :
            if additive == '' : return queryString
            else : 
                queryString = queryString + ' && ' + additive
                return queryString

    def makeAutoSubtable(self) :
        self.checkAntenna()
        self.makeSubtableQuery()
        tb.open("%s" % self.inputTable)
        self.subtable = tb.query(self.queryString)
        tb.close()
        self.getFreq()
        self.getTimeAndInterval()
        self.getFit()
#        self.freqAverageSolutions()
#        self.timeAverageSolutions()
#        self.generateSpectralResiduals()
#        self.generateTimeResiduals()

    def makeSubtable(self) :
        tb.open("%s" % self.inputTable)
        self.subtable = tb.query(self.queryString)
        tb.close()
        self.getFreq()
        self.getTimeAndInterval()
        self.getFit()
        self.freqAverageSolutions()
        self.timeAverageSolutions()
        self.generateFrequencyResiduals()
        self.generateTimeResiduals()

    def setAutoSubtableQuery(self,autoSubtableQuery) :
        self.autoSubtableQuery = autoSubtableQuery

    def getAutoSubtableQuery(self) : return self.autoSubtableQuery

class ScaleGainsClass(CalTableExplorer):
    
    def __init__(self,calTable) :
        self.calTable = calTable
        CalTableExplorer.__init__(self,self.calTable)
        self.vm      = ValueMapping(self.inputMs)

    def calculateGainsScaling(self,calfieldL,calfieldH,caltableL,caltableH):

        tb.open("%s" % caltableL,nomodify=False)
        tableRow = tb.selectrows(0,'table_junk')
        polgainshape=len(tableRow.getcol('GAIN'))
        tb.close()	
        
        ####### Setting up dictionaries for low frequency table ######

        # Initialising cal to spw mapping dictionary
        self.calSpwMapL = {}

        #open descriptor file
        tb.open("%s/CAL_DESC" % caltableL ,nomodify=False)

        for k in range(tb.nrows()):
            tableRow = tb.selectrows(k,'table_junk')
            self.calSpwMapL[k] = tableRow.getcol('SPECTRAL_WINDOW_ID')[...,0]
	       
        tb.close()
	
        #Delete file containing each table row    
        os.system('rm -rf table_junk')

        ####### Setting up dictionaries for high frequency table ######

        # Initialising cal to spw mapping dictionary
        self.calSpwMapH = {}

        #open descriptor file
        tb.open("%s/CAL_DESC" % caltableH ,nomodify=False)

        for k in range(tb.nrows()):
            tableRow = tb.selectrows(k,'table_junk')
            self.calSpwMapH[k] = tableRow.getcol('SPECTRAL_WINDOW_ID')[...,0]
	       
        tb.close()
	
        #Delete file containing each table row    
        os.system('rm -rf table_junk')

        phasediff = np.zeros([self.vm.uniqueAntennas.shape[0],len(self.calSpwMapL),polgainshape]) 

        for pol in range(polgainshape):
             for calid in self.calSpwMapL:
                for antname1 in self.vm.uniqueAntennas:
           
                    ant = self.vm.getAntennaIdsForAntennaName(antname1)
                    spw = self.calSpwMapL[calid]
                     
                    print "Antenna, spw, Corr", antname1, spw, pol

                    ###### Caltable low: read in and unwrap phases ######

                    ct = CalTableExplorer("%s" % caltableL,spwID=spw,field=calfieldL)
                    
                    ct.setAntenna(antname1)
                    
                    phaseLpol=ct.phase[pol,0,:]
                    sizeL=ct.phase[pol,0,:].shape
           
                    ct.unwrapPhase()
                    
                    timeL=ct.time

                    pLinterp=interp1d(timeL,phaseLpol,kind=1,bounds_error=False,fill_value=np.nan)
                    
                    ###### Caltable high: read in and unwrap phases ######

                    if spw+4 in self.calSpwMapH.values():

                        ct = CalTableExplorer("%s" % caltableH,spwID=spw+4,field=calfieldH)
                    
                        ct.setAntenna(antname1)

                        phaseHpol=ct.phase[pol,0,:]

                        ct.unwrapPhase()
                        
                        timeH =ct.time

                        # Find interpolated values of low frequency at high data times
                        phaseLtoH= pLinterp(timeH)

                    else:
                        phaseHpol=np.zeros(sizeL)
                        phaseLtoH=np.zeros(sizeL)

                    keep = ~np.isnan(phaseHpol) & ~np.isnan(phaseLtoH)
                    
                    phasediff[ant,spw,pol] = np.mean(phaseHpol[keep]-phaseLtoH[keep])
                    
        return phasediff


    def scaleGains(self,phasediff,newTable=None) :
        
        # Define table name
        if newTable == None :
            self.newTable = '%s.scaled' % self.calTable
        else :
            self.newTable = newTable
            
        # Create copy of table, deep=T copies all tables not just data
        tb.open(self.calTable,nomodify=False)
        tb.copy(self.newTable,deep=True,valuecopy=True)
        tb.close()
        
        # Initialising cal to spw mapping dictionary, and row holders
	self.calSpwMap = {}

	#open descriptor file
        tb.open("%s/CAL_DESC" % self.calTable,nomodify=False)

        #Initialise row holder
	descrowvals  = {}

        for k in range(tb.nrows()):
            tableRow = tb.selectrows(k,'table_junk')
	    #Convert row to a dictionary
	    for i in tableRow.colnames():
                descrowvals[i] = tableRow.getcol(i)[...,0]
            self.calSpwMap[k] = descrowvals['SPECTRAL_WINDOW_ID'][0]
	       
	    # Write out to new table
	    #self.reconstructRow(descrowvalsp,"%s" % self.newTable)

	tb.close()
	
	#Delete file containing each table row    
	os.system('rm -rf table_junk')

	#Open caltable, find number of rows in table
        tb.open("%s" % self.calTable,nomodify=False)
        numsoln=tb.nrows()
        tb.close()

	#Initialise row holder
	rowvals  = {}

        # Loop over rows/solutions in caltable
        for k in range(numsoln) :

            tb.open("%s" % self.calTable,nomodify=False)

            #Read each row and write it into table_junk
            tableRow = tb.selectrows(k,'table_junk')
            
	    #Convert row to a dictionary
            for i in tableRow.colnames():
	        if 'REF_' in i : continue
	        else:
                    rowvals[i] = tableRow.getcol(i)[...,0]

            tb.close()

            #Set atribute of this class = rowvals
            self.rowvals = rowvals
                            
	    #Copy parameters for a given solution
	    self.rowvalsp = rowvals.copy()
                              
            #initialise scaled phase and amp arrays         
	    sphase = np.zeros(self.rowvals['GAIN'].shape)
	    samp = np.zeros(self.rowvals['GAIN'].shape)

	    ant = self.rowvalsp['ANTENNA1']
	    spw = self.calSpwMap[int(self.rowvalsp['CAL_DESC_ID'])]

            sphase[:,:]=np.angle(self.rowvals['GAIN']) + phasediff[ant,spw][:,np.newaxis]
            samp[:,:]=np.abs(self.rowvals['GAIN'])
   
            self.rowvalsp['GAIN'] = samp * np.exp(sp.sqrt(-1.)*sphase)
       
	    # Write out result to new table
	    self.reconstructRow(self.rowvalsp,"%s" % self.newTable)
             
        tb.open("%s" % self.newTable,nomodify=False)
        tb.removerows(range(numsoln))
        tb.close()

        #Delete table_junk file            
        os.system("rm -rf table_junk")
        
        #Remove table locks
        os.system("rm -rf %s/table.lock" % self.calTable)
        os.system("rm -rf %s/CAL_DESC/table.lock" % self.calTable)

    def reconstructRow(self,rowVals,tableName,tableRow=None) :
        tb.open(tableName,nomodify=False)
        if tableRow == None :
            tb.addrows()
            rownum = tb.nrows()-1L
        else :
            rownum = tableRow
        for i in rowVals.keys() :
            if rowVals[i].shape == () : isRealArray = False
            else : isRealArray = True
            rownum,i,rowVals[i]
            try:
                dt = tb.coldatatype(i)
                if dt == 'boolean' :
                    if not isRealArray :
                        if not rowVals[i] :
                            tb.putcell(i,rownum,0)
                        else :
                            tb.putcell(i,rownum,1)
                    else :
                        tb.putcell(i,rownum,rowVals[i])
                elif dt == 'double' :
                    if not isRealArray :
                        tb.putcell(i,rownum,float(rowVals[i]))
                    else :
                        tb.putcell(i,rownum,rowVals[i])
                elif dt == 'float' :
                    if not isRealArray :
                        tb.putcell(i,rownum,float(rowVals[i]))
                    else :
                        junk = np.array(rowVals[i],dtype='float32')
                        tb.putcell(i,rownum,junk)                    
                elif dt == 'integer' :
                    if not isRealArray :
                        tb.putcell(i,rownum,int(rowVals[i]))
                    else :
                        tb.putcell(i,rownum,rowVals[i])
                elif dt == 'string' :
                    if not isRealArray :
                        tb.putcell(i,rownum,str(rowVals[i]))
                    else :
                        tb.putcell(i,rownum,rowVals[i])
                else :
                    tb.putcell(i,rownum,rowVals[i])
            except:
                print 'skipping'
        tb.close()
#                tb.putcell(i,rownum,rowVals[i])


class InterpolateTsys(CalTableExplorer):
    """The Flag "iKnowWhatIAmDoing" should be used with care.  It implies that the user knows very well that the
       ordering of the basebands vs frequency was proper and consistent between the tdm and fdm mode and that the
       tdm mode was listed in the OT ranging from BB1, BB2, BB3 and BB4, not some other order.  Failure to use this
       options will in some cases result in slightly poorer performance and slightly better in other cases...thus
       you should really know what you are doing before you use it.
    """
       
    def __init__(self,calTable) :
        self.calTable = calTable
        self.inputMs = self.getMS_NAME(calTable)
        #CalTableExplorer.__init__(self,self.calTable)
        self.vm      = ValueMapping(self.inputMs)

    def getMS_NAME(self,inputTable) :
        tb.open("%s/CAL_DESC" % inputTable)
        msFiles = tb.getcol("MS_NAME")
        tb.close()
        return np.unique(msFiles)[0]

    def setCalTable(self,calTable) :
        self.calTable = calTable
        CalTableExplorer.__init__(self.calTable)
        self.vm.setInputMs(self.inputMs)

    def correctBadTimes(self, force=False):
        tb.open(self.calTable,nomodify=False)
        time=tb.getcol('TIME')
        interval = tb.getcol('INTERVAL')
        if max(time) < 7.0e9 and force == False:
            return "This process appears to have been done already."
        else :
            corr_time = time-interval/2.0
            tb.putcol("TIME",corr_time)
        tb.close()
        os.system("rm -rf %s/table.lock" % self.calTable)

    def assignFieldAndScanToSolution(self, iKnowWhatImDoing=False) :
        tb.open(self.calTable,nomodify=False)
        fieldId = tb.getcol("FIELD_ID")
        scans   = tb.getcol("SCAN_NUMBER")
        times   = tb.getcol("TIME")
        linesToRemove = []
        for i in range(tb.nrows()):
            _scan = self.vm.getScansForTime(times[i],5e-6)
            if _scan != None:
                  fieldId[i] = self.vm.getFieldsForScan(_scan,False)
                  scans[i] = _scan
            else:
                  linesToRemove.append(i)
        if iKnowWhatImDoing:
              for i in sorted(linesToRemove, reverse=True):
                  fieldId = np.delete(fieldId, i)
                  scans = np.delete(scans, i)
                  tb.removerows(i)
        tb.putcol('FIELD_ID',fieldId)
        tb.putcol("SCAN_NUMBER",scans)
        tb.close()
        os.system("rm -rf %s/table.lock" % self.calTable)
        

    def getTdmFdmSpw(self,iKnowWhatIAmDoing=True):
        """
        Input: None
        Output: None
        Responsible: S. Corder
        Purpose:
        """
        tb.open("%s/SYSCAL" % self.inputMs)
        tsysTdmSpw = sorted(dict.fromkeys(tb.getcol('SPECTRAL_WINDOW_ID')).keys())
        tb.close()
        tb.open("%s/SPECTRAL_WINDOW" % self.inputMs)
        #self.tdm = tb.query("(NUM_CHAN == 128) or (NUM_CHAN == 256) or (NUM_CHAN == 64)")
        self.tdm = tb.selectrows([int(i) for i in tsysTdmSpw])
        self.tdmSpw = self.tdm.rownumbers()
        if self.tdm.nrows() == 1: self.tdmSpw = [self.tdmSpw]
        self.tdmBBC = self.tdm.getcol('BBC_NO')
        self.fdm = tb.query("(NUM_CHAN == 7680) or (NUM_CHAN == 3840) or (NUM_CHAN == 1920) or (NUM_CHAN == 4096)")
        self.fdmSpw = self.fdm.rownumbers()
        if self.fdm.nrows() == 1: self.fdmSpw = [self.fdmSpw]
        self.fdmBBC = self.fdm.getcol('BBC_NO')
        if iKnowWhatIAmDoing : self.fdmBBC.sort()
        fdmFreqs = self.fdm.getcol('CHAN_FREQ')
        tdmFreqs = self.tdm.getcol('CHAN_FREQ')
        self.fdmFreqs = fdmFreqs
        self.tdmFreqs = tdmFreqs
        tb.close()
        self.tdmFdmMap = {}
        self.fdmTdmMap = {}
        for i in range(len(fdmFreqs[0])) :
            for j in range(len(tdmFreqs[0])) :
                delT = abs(tdmFreqs[1,j]-tdmFreqs[0,j])   # Added by S. Corder 2012/5/18
                minF = np.min(fdmFreqs[:,i])
                maxF = np.max(fdmFreqs[:,i])
                minT = np.min(tdmFreqs[:,j])
                maxT = np.max(tdmFreqs[:,j])
#                if ((minF >= minT) and (maxF <= maxT)) :  # Change requested by S. Corder 2012/5/18
                if ((minF >= (minT-0.5*delT)) and (maxF <= (maxT+0.5*delT))) :
                    if self.fdmTdmMap.has_key(int(self.fdmSpw[i])) :
                        if self.fdmBBC[i] == self.tdmBBC[j] :
                           self.fdmTdmMap[self.fdmSpw[i]] = self.tdmSpw[j]
                    else :
                        self.fdmTdmMap[self.fdmSpw[i]] = self.tdmSpw[j]
        for k,v in self.fdmTdmMap.iteritems() :
            self.tdmFdmMap[v] = self.tdmFdmMap.get(v,[])
            self.tdmFdmMap[v].append(k)
        print '# Mapping of Tsys and science spws.'
        print '# Please check that you have only one science spw per Tsys spw.'
        print '# ' + str(self.tdmFdmMap)
            
    def interpolateTsys(self,newTable=None,interpType='linear') :
        self.badRows = []
        if newTable == None :
            self.newTable = '%s.fdm' % self.calTable
        else :
            self.newTable = newTable
        tb.open(self.calTable,nomodify=False)
        tb.copy(self.newTable,deep=True,valuecopy=True)
        tb.close()
        tb.open("%s/CAL_DESC" % self.calTable,nomodify=False)
        self.numCalSol = tb.nrows()
        self.calSpwMap = {}
        tb.close()
        noSpw = []
        for k in range(self.numCalSol) :
            tb.open("%s/CAL_DESC" % self.calTable,nomodify=False)            
            tableRow = tb.selectrows(k,'table_junk')
            rowvals  = self.extractRow(tableRow)
            x = self.fdmFreqs.shape[0]
            y = rowvals['CHAN_WIDTH'].shape[0]
            y1 = rowvals['CHAN_RANGE'].shape[0]
            self.calSpwMap[k] = rowvals['SPECTRAL_WINDOW_ID'][0]
            self.calSpwMap[tb.nrows()+k]  = self.fdmSpw[k]
            rowvals['MS_NAME'] = np.array(rowvals['MS_NAME'],'str')
            rowvals['JONES_TYPE'] = np.array(rowvals['JONES_TYPE'],'str')
            rowvals['NUM_CHAN'] = np.array([x],'int')
            rowvals['SPECTRAL_WINDOW_ID'] = np.array([self.fdmSpw[k]],'int')
            rowvals['CHAN_FREQ'] = np.zeros((1,x))
            rowvals['CHAN_WIDTH'] = np.zeros((1,x))
            rowvals['CHAN_RANGE'] = np.zeros((y1,1,x),'int')
            rowvals['POLARIZATION_TYPE'] = np.zeros((1,x),'str')
            if self.fdmSpw[k] in self.fdmTdmMap.keys() : self.reconstructRow(rowvals,"%s/CAL_DESC" % self.newTable)
            tb.close()
        os.system('rm -rf table_junk')
        tb.open("%s/CAL_DESC" % self.calTable)
        calIds = tb.getcol("SPECTRAL_WINDOW_ID")
        tb.close()
        tb.open("%s" % self.calTable,nomodify=False)
        self.calrows = tb.nrows()
        self.spwCalMap = {}
        tb.close()
        noData = []
        for k,v in self.calSpwMap.iteritems() :
            self.spwCalMap[v] = self.spwCalMap.get(v,[])
            self.spwCalMap[v].append(k)
        counter = 0
        countMe = 0
        for k in range(self.calrows) :
#            if self.calSpwMap.has_key(int(rowvals['CAL_DESC_ID'])):
            tb.open("%s" % self.calTable,nomodify=False)
            tableRow = tb.selectrows(k,'table_junk')
            tb.close()
            rowvals = self.extractRow(tableRow)
            self.rowvals = rowvals
            if 1:
                tdmSpwID = self.calSpwMap[int(rowvals['CAL_DESC_ID'])]
                tdmRow = self.tdmSpw.index(tdmSpwID)
                tdmFreq = self.tdmFreqs[:,tdmRow]
                self.tdmFreq = tdmFreq
                if self.tdmFdmMap.has_key(tdmSpwID):
                    for i in self.tdmFdmMap[tdmSpwID] :
                        fdmSpwID = i
                        fdmRow = self.fdmSpw.index(i)
                        fdmFreq = self.fdmFreqs[:,fdmRow]
                        self.fdmFreq = fdmFreq
                        rowvalsp = rowvals.copy()
                        self.rowvalsp = rowvalsp
                        self.rowvals  = rowvals
                        val = int(self.spwCalMap[i][0])-self.numCalSol
                        rowvalsp['CAL_DESC_ID'] = np.array(int(self.spwCalMap[i][0])-self.numCalSol,'int')
                        if interpType == 'cubicspline' :
                            _real = (self.interpSpline(tdmFreq,fdmFreq,np.real(rowvals['GAIN'])))
                            _imag = (self.interpSpline(tdmFreq,fdmFreq,np.imag(rowvals['GAIN'])))
                            rowvalsp['GAIN'] = np.zeros(_real.shape,'complex64')
                            for i in range(rowvalsp['GAIN'].shape[0]) :
                                for j in range(rowvalsp['GAIN'].shape[1]) :
                                    rowvalsp['GAIN'][i,j] = np.complex(_real[i,j],_imag[i,j])
#                            rowvalsp['GAIN'] = (self.interpSpline(tdmFreq,fdmFreq,np.real(rowvals['GAIN'])))
                            rowvalsp['SOLUTION_OK'] = np.ones((rowvals['SOLUTION_OK'].shape[0],x),'bool') #self.interpSpline(tdmFreq,fdmFreq,rowvals['SOLUTION_OK'])
                            rowvalsp['FIT'] = np.ones((1,x),'float32') #self.interpSpline(tdmFreq,fdmFreq,rowvals['FIT'])
                            rowvalsp['FIT_WEIGHT'] = np.ones((1,x),'float32') #self.interpSpline(tdmFreq,fdmFreq,rowvals['FIT_WEIGHT'])
                            rowvalsp['FLAG'] = np.zeros((rowvals['FLAG'].shape[0],x),'bool') #self.interpSpline(tdmFreq,fdmFreq,rowvals['FLAG'])
                            rowvalsp['SNR'] = np.ones((rowvals['SNR'].shape[0],x),'float32') #self.interpSpline(tdmFreq,fdmFreq,rowvals['SNR'])
                        elif interpType == 'linear' :
                            _real = (self.interpLinear(tdmFreq,fdmFreq,np.real(rowvals['GAIN'])))
                            _imag = (self.interpLinear(tdmFreq,fdmFreq,np.imag(rowvals['GAIN'])))
                            rowvalsp['GAIN'] = np.zeros(_real.shape,'complex64')
                            for i in range(rowvalsp['GAIN'].shape[0]) :
                                for j in range(rowvalsp['GAIN'].shape[1]) :
                                    rowvalsp['GAIN'][i,j] = np.complex(_real[i,j],_imag[i,j])
#                            rowvalsp['GAIN'] = (self.interpLinear(tdmFreq,fdmFreq,np.real(rowvals['GAIN'])))
                            rowvalsp['SOLUTION_OK'] = np.ones((rowvals['SOLUTION_OK'].shape[0],x),'bool') #self.interpLinear(tdmFreq,fdmFreq,rowvals['SOLUTION_OK'])
                            rowvalsp['FIT'] = np.ones((1,x),'float32') #self.interpLinear(tdmFreq,fdmFreq,rowvals['FIT'])
                            rowvalsp['FIT_WEIGHT'] = np.ones((1,x),'float32') #self.interpLinear(tdmFreq,fdmFreq,rowvals['FIT_WEIGHT'])
                            rowvalsp['FLAG'] = np.zeros((rowvals['FLAG'].shape[0],x),'bool') #self.interpLinear(tdmFreq,fdmFreq,rowvals['FLAG'])
                            rowvalsp['SNR'] = np.ones((rowvals['SNR'].shape[0],x),'float32') #self.interpLinear(tdmFreq,fdmFreq,rowvals['SNR'])
                        else :
                            return "Invalid interpType, please pick linear or cubicspline."
                        self.reconstructRow(rowvalsp,"%s" % self.newTable,counter)
                        counter+=1
#                        sys.stdin.readline()
                    countMe+=1
                else :
                    #self.nullRow("%s" % self.newTable,k)
                    noData.append(counter)
        os.system("rm -rf table_junk")
        print counter
        print noData
        tb.close()
        if noData <> [] : 
            if min(makeList(noData)) < self.calrows :
                while max(makeList(noData)) >= self.calrows :
                    noData.remove(max(makeList(noData)))
        os.system("rm -rf %s/table.lock" % self.newTable)    
        tb.open(self.newTable,nomodify=False)
        if noData <> [] : tb.removerows(noData)
        tb.close()
        os.system("rm -rf %s/CAL_DESC/table.lock" % self.newTable)
        tb.open("%s/CAL_DESC" % self.newTable,nomodify=False)
        tb.removerows(range(self.numCalSol))
        tb.close()
        os.system("rm -rf %s/table.lock" % self.calTable)
        os.system("rm -rf %s/CAL_DESC/table.lock" % self.calTable)
        os.system("rm -rf table_junk")

       
    def interpLinear(self,tmpFreq,newFreq,tmpData) :
        tmpFreq,tmpData,checker = self.checkOrder(tmpFreq,tmpData)
        if newFreq[1]-newFreq[0] < 0 : newFreq = newFreq[::-1]
        newData = np.zeros((tmpData.shape[0],newFreq.shape[0]))
        for i in range(tmpData.shape[0]) :
            newData[i,:] = np.interp(newFreq,tmpFreq,tmpData[i,:])
        if checker :
            return newData.transpose()[::-1].transpose()
        else :
            return newData

    def interpSpline(self,tmpFreq,newFreq,tmpData) :
        tmpFreq,tmpData,checker = self.checkOrder(tmpFreq,tmpData)        
        newData = np.zeros((tmpData.shape[0],newFreq.shape[0]))
        for i in range(tmpData.shape[0]) :
            tck = splrep(tmpFreq,tmpData[i,:],s=0)
            newData[i,:] = splev(newFreq,tck,der=0)
        if checker :
            return newData.transpose()[::-1].transpose()
        else :
            return newData

    def checkOrder(self,inpFreq,inpData) :
        if ((inpFreq[1]-inpFreq[0]) > 0) :
            return inpFreq,inpData,0
        else :
            return inpFreq[::-1],inpData.transpose()[::-1].transpose(),1
        
    def extractRow(self,row) :
        rowvals = {}
        for i in row.colnames() :
            try:
                rowvals[i] = row.getcol(i)[...,0]
            except:
                print "Unable to extract data for %s" % i
        return rowvals

    def reconstructRow1(self,tableName,tableRow=None) :
        tb.open(tableName,nomodify=False)
        if tableRow == None : tb.addrows()
        else :
            self.badRows.append(tableRow)
        tb.close()

    def nullRow(self,tableName,tableRow) :
        tb.open(tableName,nomodify=False)
        row = tb.selectrows(tableRow,'table_null')
        rowVals = self.extractRow(row)
        for i in rowVals.keys() :
            if rowVals[i].shape == () : isRealArray = False
            else : isRealArray = True
            row,i,rowVals[i]
            try:
                dt = tb.coldatatype(i)
                if dt == 'boolean' :
                    if not isRealArray :
                        if not rowVals[i] :
                            tb.putcell(i,rownum,0)
                        else :
                            tb.putcell(i,rownum,1)
                    else :
                        tb.putcell(i,rownum,rowVals[i])
                elif dt == 'double' :
                    if not isRealArray :
                        tb.putcell(i,rownum,float(rowVals[i])*0)
                    else :
                        tb.putcell(i,rownum,rowVals[i]*0)
                elif dt == 'float' :
                    if not isRealArray :
                        tb.putcell(i,rownum,float(rowVals[i])*0)
                    else :
                        junk = np.array(rowVals[i]*0,dtype='float32')
                        tb.putcell(i,rownum,junk)                    
                elif dt == 'integer' :
                    if not isRealArray :
                        tb.putcell(i,rownum,int(rowVals[i]*0))
                    else :
                        tb.putcell(i,rownum,rowVals[i]*0)
                elif dt == 'string' :
                    if not isRealArray :
                        tb.putcell(i,rownum,str(rowVals[i]))
                    else :
                        tb.putcell(i,rownum,rowVals[i]*0)
                else :
                    tb.putcell(i,rownum,rowVals[i])
            except:
                print 'skipping'
        tb.close()
        os.system("rm -rf table_null")

    def reconstructRow(self,rowVals,tableName,tableRow=None) :
        tb.open(tableName,nomodify=False)
        if tableRow == None :
            tb.addrows()
            rownum = tb.nrows()-1L
        else :
            rownum = tableRow
        for i in rowVals.keys() :
            if rowVals[i].shape == () : isRealArray = False
            else : isRealArray = True
            rownum,i,rowVals[i]
            try:
                dt = tb.coldatatype(i)
                if dt == 'boolean' :
                    if not isRealArray :
                        if not rowVals[i] :
                            tb.putcell(i,rownum,0)
                        else :
                            tb.putcell(i,rownum,1)
                    else :
                        tb.putcell(i,rownum,rowVals[i])
                elif dt == 'double' :
                    if not isRealArray :
                        tb.putcell(i,rownum,float(rowVals[i]))
                    else :
                        tb.putcell(i,rownum,rowVals[i])
                elif dt == 'float' :
                    if not isRealArray :
                        tb.putcell(i,rownum,float(rowVals[i]))
                    else :
                        junk = np.array(rowVals[i],dtype='float32')
                        tb.putcell(i,rownum,junk)                    
                elif dt == 'integer' :
                    if not isRealArray :
                        tb.putcell(i,rownum,int(rowVals[i]))
                    else :
                        tb.putcell(i,rownum,rowVals[i])
                elif dt == 'string' :
                    if not isRealArray :
                        tb.putcell(i,rownum,str(rowVals[i]))
                    else :
                        tb.putcell(i,rownum,rowVals[i])
                else :
                    tb.putcell(i,rownum,rowVals[i])
            except:
                print 'skipping'
        tb.close()
#                tb.putcell(i,rownum,rowVals[i])

class Visibility:
    """Instantiation requires the input MS file.  Also, if spwID is not set and there is more than one, beware as a
    failure will occur if the spw have different shapes.  If you create this instance, what you get is a structure with
    various attributes.  If you use the data.setX methods the table selection is redone, i.e. data.antenna1='DV01' (or 0, it interprets both,
    I am working on the same thing for field) will not make you a new table but data.setAntenna1('DV01') will make a new table with
    antenna1 as DV01 instead of whatever it was before.  You can also make the table not automatically create the subtable by setting
    autoSubtableQuery==False and then you can put in your own queryString.  cross_auto_all is set to 'all' by default but if you put
    in 'cross' or 'auto' it will select the relevant items.  There are a few functions that return the amplitude and phase (or recalculate them)
    and there is an unwrap and wrap phase option however, use this with caution as it depends on having alot of signal to noise in each measurement,
    i.e. it is not smart.  Let me know if you have questions, additions, or whatever...additions can just be made and a warning ;)  Try to make
    changes backwards compatible...that'll make it ugly but it'll make it work!
    """
    
    def __init__(self,inputMs,antenna1=0,antenna2=0,spwID=None,field=None,state=None,scan=None,autoSubtableQuery=True,queryString='',cross_auto_all='all',correctedData=False):
        if autoSubtableQuery==False and queryString=='' : return 'Must either automatically generate the (autoSubtableQuery=True) or provide a subtable query string (queryString)'
        if spwID == None :
            spwID = getChanAverSpwIDBaseBand0(inputMs)
        self.inputMs = inputMs
        self.valueMap = ValueMapping(self.inputMs)
        self.antenna1 = antenna1
        self.antenna2 = antenna2
        if self.antenna1 <> None : self.antenna1    = str(antenna1)
        if self.antenna2 <> None : self.antenna2    = str(antenna2)
        self.checkAntenna()
        if cross_auto_all.lower() in ['cross','auto','all'] :
            self.cross_auto_all = cross_auto_all
        else :
            return "Improper value for cross_auto_all, please select, cross, auto or all."
        self.spwID       = spwID
        self.field       = field
        self.checkField()
        self.correctedData = correctedData
        self.state       = state
        self.scan        = scan
        self.autoSubtableQuery = autoSubtableQuery
        self.queryString = queryString
        if self.autoSubtableQuery == True : 
            self.makeAutoSubtable()
        else : self.makeSubtable()
        self.getSpectralData()
        self.getAmpAndPhase()

    def checkAntenna(self) :
        if self.antenna1 <> None : 
            self.antenna1 = str(self.antenna1)
            if not self.antenna1.isdigit() : self.antenna1 = getAntennaIndex(self.inputMs,self.antenna1)
        if self.antenna2 <> None :
            self.antenna2 = str(self.antenna2)
            if not self.antenna2.isdigit() : self.antenna2 = getAntennaIndex(self.inputMs,self.antenna2)

    def makeAutoSubtable(self) :
        self.checkAntenna()
        self.makeSubtableQuery()
        tb.open(self.inputMs)
        self.subtable = tb.query(self.queryString)
        tb.close()
        self.getSpectralData()
        self.getAmpAndPhase()

    def makeSubtable(self) :
        tb.open(self.inputMs)
        self.subtable = tb.query(self.queryString)
        tb.close()

    def setAutoSubtableQuery(self,autoSubtableQuery) :
        self.autoSubtableQuery = autoSubtableQuery

    def getAutoSubtableQuery(self) : return self.autoSubtableQuery

    def setAntennaPair(self,antenna1,antenna2) :
        self.antenna1 = str(antenna1)
        self.antenna2 = str(antenna2)
        self.checkAntenna()
        if self.autoSubtableQuery : self.makeAutoSubtable()

    def setAntenna1(self,antenna1) :
        self.antenna1 = str(antenna1)
        self.checkAntenna()
        if self.autoSubtableQuery : self.makeAutoSubtable()

    def setAntenna2(self,antenna2) :
        self.antenna2 = str(antenna2)
        self.checkAntenna()
        if self.autoSubtableQuery : self.makeAutoSubtable()

    def getAntenna1(self) : return self.antenna1

    def getAntenna2(self) : return self.antenna2

    def getAntennaPair(self) :
        return [self.antenna1,self.antenna2]

    def setSpwID(self,spwID) :
        self.spwID = spwID
        if self.autoSubtableQuery : self.makeAutoSubtable()

    def getSpwID(self) : return self.spwID

    def setField(self,field) :
        self.field = field
        self.checkField()
        if self.autoSubtableQuery : self.makeAutoSubtable()

    def getField(self) : return self.field

    def checkField(self) :
        if self.field <> None : 
            self.field = str(self.field)
            if not self.field.isdigit() : self.field = self.valueMap.getFieldIdsForFieldName(self.field)[0]

    def setState(self,state) :
        self.state = state
        if self.autoSubtableQuery : self.makeAutoSubtable()
    
    def getState(self) : return self.state
    
    def setScan(self,scan) :
        self.scan = scan
        if self.autoSubtableQuery : self.makeAutoSubtable()
    
    def getScan(self) : return self.scan
    
    def makeSubtableQuery(self) :
        self.parameterList = []
        queryString = ''
        if self.antenna1 <> None : self.parameterList.append('ANTENNA1 == %s' % self.antenna1)
        if self.antenna2 <> None : self.parameterList.append('ANTENNA2 == %s' % self.antenna2)
        if self.field <> None    : self.parameterList.append('FIELD_ID == %s' % self.field)
        if self.spwID <> None    : self.parameterList.append('DATA_DESC_ID == %s' % getDataDescriptionId(self.inputMs,self.spwID))
        if self.state <> None    : self.parameterList.append('STATE_ID == %s' % self.state)
        if self.scan <> None     : self.parameterList.append('SCAN_NUMBER == %s' % self.scan)
        if self.cross_auto_all == 'cross' : self.parameterList.append('ANTENNA1 <> ANTENNA2')
        elif self.cross_auto_all == 'auto' : self.parameterList.append('ANTENNA1 == ANTENNA2')
        for i in self.parameterList : queryString = self.appendQuery(queryString,i)
        self.queryString = queryString

    def getSpectralData(self) :
        if 'FLOAT_DATA' in self.subtable.colnames() :
            if self.correctedData :
                self.specData = self.subtable.getcol('FLOAT_DATA')
            else :
                self.specData = self.subtable.getcol('FLOAT_DATA')
        else :
            if self.correctedData:
                self.specData = self.subtable.getcol('CORRECTED_DATA')
            else :
                self.specData = self.subtable.getcol('DATA')
        self.specTime = self.subtable.getcol('TIME')
        self.specTime = self.specTime
        self.specFreq = getFrequencies(self.inputMs,self.spwID)
        self.tavgSpecData = self.specData.mean(-1)
        self.favgSpecData = self.specData.mean(-2)

    def getAmpAndPhase(self) :
        rData = self.specData.real
        rTavgData = self.tavgSpecData.real
        rFavgData = self.favgSpecData.real
        iData = self.specData.imag
        iTavgData = self.tavgSpecData.imag
        iFavgData = self.favgSpecData.imag
        self.phase = np.arctan2(iData,rData)
        self.amp   = (rData**2.0+iData**2.0)**0.5
        self.tavgPhase = np.arctan2(iTavgData,rTavgData)
        self.tavgAmp   = (rTavgData**2.0+iTavgData**2.0)**0.5
        self.favgPhase = np.arctan2(iFavgData,rFavgData)
        self.favgAmp   = (rFavgData**2.0+iFavgData**2.0)**0.5

    def wrapPhase(self,simple=True) :
        from math import pi
        phaseShape = self.phase.shape
        for i in range(phaseShape[2]) :
            if self.phase[:,:,i] <  -pi : self.phase[:,:,i]=self.phase[:,:,i]+2*pi
            elif self.phase[:,:,i] > pi : self.phase[:,:,i]=self.phase[:,:,i]-2*pi
        
    def unwrapPhase(self,simple=True) :
        from math import pi
        phaseShape = self.phase.shape
        for i in range(phaseShape[2]-1) :
            diff = self.phase[:,:,i]-self.phase[:,:,i+1]
            _diffg = (diff > 1.*pi)*2*pi
            _diffl = (diff < -1.*pi)*2*pi
            self.phase[:,:,i+1] = self.phase[:,:,i+1]+_diffg-_diffl

    def appendQuery(self,queryString,additive) :
        if queryString == '' :
            if additive == '' : return queryString
            else : return additive
        else :
            if additive == '' : return queryString
            else : 
                queryString = queryString + ' && ' + additive
                return queryString


def getAntennaIndex(msFile,antennaName) :
    """
    Returns the index number of the specified antenna in the specified ms.
    The antennaName must be a string, e.g. DV01.
    """
    if str(antennaName).isdigit() : 
        return antennaName
    else :
        ids = getAntennaNames(msFile)
        return np.where(ids == antennaName)[0][0]

def getAntennaName(msFile,antennaID) :
    """
    Returns the antenna name of the specified antenna in the specified ms.
    The antennaID must be an integer.
    Todd Hunter
    """
    names = getAntennaNames(msFile)
    if (antennaID >= 0 and antennaID < len(names)):
        return (names[antennaID])
    else:
        print "This antenna ID (%d) is not in the ms." % (antennaID)
        return ('')

def getAntennaNames(msFile) :
    """
    Returns the list of antenna names in the specified ms.
    """
    tb.open(msFile+'/ANTENNA')
    names = tb.getcol('NAME')
    tb.close()
    return names

def getNumberOfAntennas(msFile) :
    """
    Returns the number of antennas in the specified ms.
    """
    return len(np.unique(getAntennaNames(msFile)))

def convertTimeStamps(timesIn) :
    timesIn = makeList(timesIn)
    timesOut = []
    for i in timesIn :
        timesOut.append(convertTimeStamp(i))
    return timesOut

def convertTimeStamp(timeIn) :
    conv   = timeIn.split('T')
    date   = conv[0].split('-')
    time   = conv[1].split(':')
    year   = float(date[0])
    month  = float(date[1])
    day    = float(date[2])
    hour   = float(time[0])
    minute = float(time[1])
    second = float(time[2])
    ut=hour+minute/60+second/3600
    if (100*year+month-190002.5)>0:
        sig=1
    else:
        sig=-1
    return 367*year - int(7*(year+int((month+9)/12))/4) + int(275*month/9) + day + 1721013.5 + ut/24 - 0.5*sig +0.5

def parseTrx(antennaSel,polSel=0,bandNumberSel=3,filename='/data/checkTrx.txt') :
    data = fiop.readcolPy(filename,'s,f,s,f,i,s,f,f,f,f,f')
    recTime = np.array(data[0])
    elev    = np.array(data[1])
    jdTime  = recTime
    for i in range(len(recTime)) : jdTime[i] = convertTimeStamp(recTime[i])-2455198
    antenna = np.array(data[2])
    freq    = np.array(data[3])*1e9
    pol     = np.array(data[4])
    chan    = np.array(data[5])
    trx     = np.array(data[6])
    errtrx  = np.array(data[7])
    gain    = np.array(data[8])
    errgain = np.array(data[9])
    tsys    = np.array(data[10])
    indexAnt  = np.where(antenna == antennaSel)[0]
    indexPol  = np.where(pol == polSel)[0]
    indexFreqLow = np.where(freq <= bandDefinitions[bandNumberSel][1])[0]
    indexFreqHigh = np.where(freq >= bandDefinitions[bandNumberSel][0])[0]
    indexVal = list((set(indexAnt) & set(indexPol) & set(indexFreqLow) & set(indexFreqHigh)))
    return jdTime[indexVal],elev[indexVal],recTime[indexVal],chan[indexVal],trx[indexVal],errtrx[indexVal],gain[indexVal],errgain[indexVal],freq[indexVal],tsys[indexVal]

def parseTrxInfo(antennaList=['DV01','DV02','DV03','DV04','DV05','PM02','PM03'],polList=[0,1],bandList=[3,6],filename=None) :
    if filename == None : filename='/data/checkTrx.txt'
    info = {}
    for i in antennaList :
        info[i] = {}
        for j in bandList :
            info[i][j] = {}
            for k in polList :
                info[i][j][k] = {'jdTime' : [], 'recTime' : [], 'chan' : [], 'trx' : [], 'errtrx' : [], 'gain' : [], 'errgain' : [], 'freq' : [],'elev' : [],'tsys':[]}
                info[i][j][k]['jdTime'],info[i][j][k]['elev'],info[i][j][k]['recTime'],info[i][j][k]['chan'],info[i][j][k]['trx'],info[i][j][k]['errtrx'],info[i][j][k]['gain'],info[i][j][k]['errgain'],info[i][j][k]['freq'],info[i][j][k]['tsys'] = parseTrx(i,k,j,filename)
    return info


def plotTrxInfo(antennaList=['DV01','DV02','DV03','DV04','DV05','PM02','PM03'],polList=[0,1],bandList=[3,6],filename=None) :
    if filename == None : filename='/data/checkTrx.txt'
    antennaList = makeList(antennaList)
    polList = makeList(polList)
    bandList = makeList(bandList)
    colorList = ['b','g','r','c','m','y','k']
    pointList = ['x','.','o','^','<','>','s','+',',','D','1','2','3','4','h','H','o','|','_']
    info = parseTrxInfo(antennaList,polList,bandList,filename)
    clf()
    limits = {3 : 100, 6: 100, 7:300, 9:500}
    spec = {3 : 30, 6: 70, 7:137, 9: 500}
    subplot(len(bandList),1,1)
    hold(True)
    rcParams['font.size'] = 9.0
    for i in range(len(antennaList)) :
        for j in range(len(bandList)) :
            subplot(len(bandList),1,j+1)
            for k in range(len(polList)) :
                legendInfo = 'Antenna %s' % (antennaList[i])
                time = info[antennaList[i]][bandList[j]][polList[k]]['jdTime']
                trx  = info[antennaList[i]][bandList[j]][polList[k]]['trx']
                err  = info[antennaList[i]][bandList[j]][polList[k]]['errtrx']
                psym = colorList[i]+pointList[k]
#                errorbar(time,trx,yerr=err,fmt=None)
                try:
                   if k==0 : plot(time,trx,psym,label=legendInfo)
                   else : plot(time,trx,psym)
                except:
                    print len(info[antennaList[i]][bandList[j]][polList[k]]['trx'])
                    print 'invalid data for antenna %s, polarization %i, band %i' % (antennaList[i],polList[k],bandList[j])
                print len(time),spec[bandList[j]]
                try: plot(time,[spec[bandList[j]]]*len(time),'k-')
                except: continue
            legend(loc=0)
            title('Trx vs time')
            xlabel('Julian Date')
            ylabel('Receiver Temperature (K)')
            ylim(0,limits[bandList[j]])
#    legend(loc=1)
    show()

def makeSpecTrx(freq,band) :
    newSpec = []
    for i in freq :
        low = (bandDefinitions[band][1]-bandDefinitions[band][0])*0.1/1e9+bandDefinitions[band][0]/1e9+6
        high = -(bandDefinitions[band][1]-bandDefinitions[band][0])*0.1/1e9+bandDefinitions[band][1]/1e9-6
        print low,high
        if band == 3 :
            if (i < low) or (i > high) : alpha = 10
            else : alpha = 6
        if band == 6 :
            if (i < low) or (i > high) : alpha = 10
            else : alpha = 6
            alpha = 6
        if band == 7 :
            if (i < low) or (i > high) : alpha = 12
            else : alpha = 8
        if band == 9 :
            if (i < low) or (i > high) : alpha = 15
            else : alpha = 10
        newSpec.append(0.048*alpha*i+4)
    return newSpec
                     
def plotTrxFreq(antennaList=['DV01','DV02','DV03','DV04','DV05','PM02','PM03'],polList=[0,1],bandList=[3,6],filename=None) :
    if filename == None : filename='/data/checkTrx.txt'
    
    antennaList = makeList(antennaList)
    polList = makeList(polList)
    bandList = makeList(bandList)
    colorList = ['b','g','r','c','m','y','k']
    pointList = ['x','o','.','^','<','>','s','+',',','D','1','2','3','4','h','H','o','|','_']
    info = parseTrxInfo(antennaList,polList,bandList,filename)
    clf()
    subplot(len(bandList),1,1)
    limits = {3 : 100, 6: 150, 7:300, 9:500}
    rcParams['font.size'] = 9.0
    hold(True)
    for i in range(len(antennaList)) :
        for j in range(len(bandList)) :
            subplot(len(bandList),1,j+1)
            legendInfo = 'Antenna %s, Band %s' % (antennaList[i],bandList[j])
            for k in range(len(polList)) :
                freq = info[antennaList[i]][bandList[j]][polList[k]]['freq']/1e9
                trx  = info[antennaList[i]][bandList[j]][polList[k]]['trx']
                err  = info[antennaList[i]][bandList[j]][polList[k]]['errtrx']
                psym = colorList[j]+pointList[i] ; print psym,len(trx),len(freq)
                plot(freq,trx,psym)
                newSpec = makeSpecTrx(freq,bandList[j])
                newSpec = np.array(newSpec)
                freq = np.array(freq)
                print newSpec.shape,freq.shape
                freq.sort()
                newSpec.sort()
                plot(freq,newSpec,'k-')
                ylim(0,limits[bandList[j]])
        legend(loc=0)
        title('Trx vs Frequency')
        xlabel('Frequency (GHz)')
        ylabel('Receiver Temperature (K)')
#    ylim(20,200)
    subplot(len(bandList),1,1)
    title('Trx vs Frequency')
    show()
    return newSpec

class MakeTable: 
    def __init__(self,inputMs,queryString='') :
        self.inputMs = inputMs
        self.queryString = queryString
        self.makeSubtable()
        self.data = {}
        for i in self.subtable.colnames() : self.data[i] = self.subtable.getcol(i)
        tb.close()

    def makeSubtable(self) :
        tb.open(self.inputMs)
        self.subtable = tb.query(self.queryString)
        tb.close()

#class SkyDip(MakeTable):
#    def __init__(self,inputMs,pressureCut=700,location='AOS'):
#        return


class Weather(MakeTable):
    def __init__(self,inputMs,pressureCut=700,location='AOS'):
        queryString = ("PRESSURE < %s" % pressureCut)
        MakeTable.__init__(self,"%s/WEATHER" % inputMs,queryString)
        self.location      = location
        self.getAtmProfile()
        self.data['ATM_TEMP'] = self.getAtmProfile()
        
    def getAtmProfile(self) :
        if self.location == 'AOS'   : alt = casac.Quantity(5000.0,'m')
        elif self.location == 'OSF' : alt = casac.Quantity(3000.0,'m')
        tatm = []
        tProfile = []
        for i in range(len(self.data["REL_HUMIDITY"])) : 
            tmp = casac.Quantity(self.data['TEMPERATURE'][i],'K')
            pre = casac.Quantity(self.data['PRESSURE'][i],'mbar')
            maxA = casac.Quantity(48.0,'km')
            hum = self.data["REL_HUMIDITY"][i]
            myatm   = at.initAtmProfile(alt,tmp,pre,maxA,hum)
            tempPro = at.getProfile()['temperature'] 
            tProfile.append(np.array(tempPro.value))
            tatm.append(sum(tempPro.value)/len(tempPro.value))
        return np.array(tatm)
        
class InterpolateTableTime: #I think this also would work for Frequency without trying at all, just have to fix verbage?  I also think the call in Tsys is wrong..and probably giving me all sorts of errors.
    def __init__(self,table,timeSeries=None,nonRealInterp='nearest',realInterp='linear',ifRepeat='average',tableQuery=None) :
        print """Warning: Interpolation of many variables may be inaccurate, be careful of using anything interpolated."""
        if nonRealInterp <> 'nearest' and nonRealInterp <> 'change' :
            print """You must enter nearetst or change for the nonRealInterp value.  Nearest is nearest neighbor, change
                     is the same value is repeated until the boolean changes values."""
            return
        else :
            self.nonrealInterpType = nonRealInterp
        if realInterp <> 'linear' and realInterp <> 'cubicspline' :
            print """You must enter linear or cubicspline for the nonRealInterp value.  Nearest is nearest neighbor, change
                     is the same value is repeated until the boolean changes values."""
            return
        else :
            self.realInterpType = realInterp
        self.table = table
        print self.table
        self.colNames = table.data.keys()
        if "TIME" not in self.colNames :
            print """CRITICAL: Time must be a component of the table to use this function."""
            return
        self.time = self.table.data["TIME"]
        self.oldData = self.table.data.copy()
        self.oldTime = self.oldData.pop("TIME")
        self.colNames.remove('TIME')
        self.timeSeries = timeSeries
        
    def interpolateData(self,timeSeries,quiet=False) :
        self.newTime = timeSeries
        self.newData = {}
        self.newTime.sort()
        if self.newTime.shape <> np.unique(self.newTime).shape :
            if not quiet : print "Removing repeated times."
            self.newTime = np.unique(self.newTime)

        for i in self.colNames :
            if not quiet : print "Doing parameter %s" % i
            if 'float' in str(self.oldData[i].dtype) or 'complex' in str(self.oldData[i].dtype) :
                if self.oldTime.shape <> np.unique(self.oldTime).shape :
                    tmpTime,tmpData = self.handleTimeRepeats(i)
                else :
                    tmpTime = self.oldTime ;  tmpData = self.oldData[i]
                tuppleMax  = self.oldData[i].shape[:-1]
                indexList  = np.ndindex(tuppleMax)
                _newTmp = []
                for j in indexList :
                    _tmpData = np.transpose(tmpData)[j][:]
                    if self.realInterpType == 'cubicspline' : _new = self.interpSpline(tmpTime,_tmpData)
                    elif self.realInterpType == 'linear' : _new = self.interpLinear(tmpTime,_tmpData)
                    _newTmp.append(np.transpose(_new))
                self.newData[i] = np.array(_newTmp)
                for j in range(len(self.newTime)) :
                    if self.oldTime[0]-self.newTime[j] > 0 :
                        self.newData[i][...,j] = self.oldData[i][...,0]
            else :
                if self.nonrealInterpType == 'change' : self.interpChange(i)
                elif self.nonrealInterpType == 'nearest' : self.interpNearest(i)
            self.newData[i] = np.squeeze(self.newData[i])

    def handleTimeRepeats(self,colname) : 
        tmpTime = []
        tmpData = []
        for j in np.unique(self.oldTime) :
            locos = np.where(self.oldTime == j)
            tmpTime.append(np.mean(self.oldTime[locos]))
            tmpData.append(np.mean(self.oldData[colname][...,locos],-1))
        tmpTime = np.array(tmpTime)
        tmpData = (np.squeeze(np.array(tmpData)))
        return tmpTime,tmpData

    def interpLinear(self,tmpTime,tmpData) :
        return np.interp(self.newTime,tmpTime,tmpData)

    def interpSpline(self,tmpTime,tmpData) :
        tck = splrep(tmpTime,tmpData,s=0)
        return splev(self.newTime,tck,der=0)

    def interpChange(self,colname) :
        self.newData[colname] = []
        for j in self.newTime :
            timeDiff = self.oldTime-j
            goodIndex = max(np.where(timeDiff < 0)[0])
            self.newData[colname].append(self.oldData[colname][goodIndex])
            self.newData[colname] = np.array(self.newData[colname])

    def interpNearest(self,colname) :
        self.newData[colname] = []
        for j in self.newTime :
            timeDiff = (self.oldTime-j)
            indexCount = abs(timeDiff).min()
            locos      = np.where(abs(timeDiff) == indexCount)
            self.newData[colname].append(np.mean(self.oldData[colname][locos]))
        self.newData[colname] = np.array(self.newData[colname])        

def getSourceFieldMapping(inputMs) :
    tb.open("%s/FIELD" % (inputMs) )
    sourceNames = tb.getcol('NAME')
    sourceIds = {}
    for i in range(len(sourceNames)) :
        sourceIds[sourceNames[i]] = i
    tb.close()
    return sourceIds,sourceNames

def getSourceScans(inputMs,sourceName) :
    if str(sourceName).isdigit() : fieldId = sourceName
    else:
        sourceIds,sourceNames = getSourceFieldMapping(inputMs)
        try:
            fieldId = sourceIds[sourceName]
        except:
            return 'What you talking about Willis?'
    tb.open(inputMs)
    scans = tb.getcol('SCAN_NUMBER')
    fields = tb.getcol('FIELD_ID')
    list1 = where(fields == fieldId)
    fieldscans = scans[list1]
    return np.unique(fieldscans)

def getBasebandNumber(inputMs,spwId) :
    """
    Returns the number of the baseband for the specified spw in the specified ms.
    """
    tb.open("%s/SPECTRAL_WINDOW" % inputMs)
    bbNums = tb.getcol("BBC_NO")
    tb.close()
    return bbNums[spwId]

def getBasebandNumbers(inputMs) :
    """
    Returns the baseband numbers associated with each spw in the specified ms.
    Todd Hunter
    """
    tb.open("%s/SPECTRAL_WINDOW" % inputMs)
    bbNums = tb.getcol("BBC_NO")
    tb.close()
    return bbNums

def getFrequencies(inputMs,spwId) :
    """
    Returns the list of channel frequencies in the specified spw in the
    specified ms.
    """
    tb.open("%s/SPECTRAL_WINDOW" % inputMs)
    chanFreq = tb.getcol("CHAN_FREQ",startrow=spwId,nrow=1)
    tb.close()
    return chanFreq
    
def getChanAverSpwIDBaseBand0(inputMs) :
    tb.open("%s/SPECTRAL_WINDOW" % inputMs)
    bbc_no = tb.getcol('BBC_NO')
    ind1 = np.where(bbc_no == 1)[0]
    num_chan = tb.getcol('NUM_CHAN')
    ind2 = np.where(num_chan == 1)[0]
    return np.intersect1d(ind1,ind2)[0]

def getDataDescriptionId(inputMs,spwId) :
    tb.open("%s/DATA_DESCRIPTION" % inputMs)
    spectralWindows = tb.getcol("SPECTRAL_WINDOW_ID")
    tb.close()
    ids = np.where(spectralWindows == spwId)[0]
    return int(ids)

def getSpectralWindowId(inputMs,dataDesId) :
    tb.open("%s/DATA_DESCRIPTION" % inputMs)
    spectralWindows = tb.getcol("SPECTRAL_WINDOW_ID")
    tb.close()
    return spectralWindows[dataDesId]
    
def getFlux(inputMs,spwID=None,jyPerK=33,badAntennas=[],useCorrected=False) :
    if spwID == None :
        spwID = getChanAverSpwIDBaseBand0(inputMs)
    sourceIds,sourceNames = getSourceFieldMapping(inputMs)
    antennas = getAntennaNames(inputMs)
    tsys = Tsys(inputMs,spwID=spwID)
    sourceFlux = {}
    averageFlux = {}
    for i in range(len(badAntennas)) :
        badAntennas[i] = getAntennaIndex(inputMs,badAntennas[i])
    for i in range(len(sourceIds)) :
        fieldId = sourceIds[sourceNames[i]]
        sourceName = sourceNames[i]
        sourceFlux[sourceName] = {}
        averageFlux[sourceName] = {}
        sourceScans = getSourceScans(inputMs,fieldId)
        for k in range(len(sourceScans)/2) :
            tsysScan = sourceScans[2*k]
            sourceScan = sourceScans[2*k+1]
            tsys_ = {}
            sourceFlux[sourceName][sourceScan] = {}
            for j in range(len(antennas)) :
#                print antennas[j]
                tsys_[antennas[j]] = tsys.sysInfo[antennas[j]][tsysScan]['Tsys']['value']
            averageFlux[sourceName][sourceScan] = {'Flux' : np.zeros(tsys_[tsys.sysInfo.keys()[0]].shape)}
            for j in range(len(antennas)) :
                for m in range(len(antennas)) :
                    if j < m :
                        if j in badAntennas or m in badAntennas :
                            continue
                        else :
                            sourceFlux_ = Visibility(inputMs,antenna1=j,antenna2=m,spwID=spwID,field=fieldId,scan=sourceScan,correctedData=useCorrected)
                            flux_  = ((tsys_[antennas[j]]*tsys_[antennas[m]])**0.5)*sourceFlux_.amp.mean(-1)*jyPerK
                            dflux_ = ((tsys_[antennas[j]]*tsys_[antennas[m]])**0.5)*sourceFlux_.amp.std(-1)*jyPerK/sourceFlux_.amp.shape[-1]**0.5
                            baseline = ('%i-%i' % (j,m))
                            sourceFlux[sourceName][sourceScan][baseline] = {'Flux' : flux_, 'Error' : dflux_}
            for j in sourceFlux[sourceName][sourceScan].keys() :
                averageFlux[sourceName][sourceScan]['Flux'] = averageFlux[sourceName][sourceScan]['Flux']+sourceFlux[sourceName][sourceScan][j]['Flux']/len(sourceFlux[sourceName][sourceScan].keys())
    return sourceFlux,tsys,averageFlux


class Tsys(Weather):
    def __init__(self,inputMs,spwID=None,tau=0.05,etaF=0.99,doRefSub=False):
        if spwID == None :
            spwID = getChanAverSpwIDBaseBand0(inputMs)
        Weather.__init__(self,inputMs)
        self.inputMs = inputMs
        self.atm = AtmStates(inputMs)
        self.loads = self.atm.antennaInfo[self.atm.antennaNames[0]].keys()
        self.spwID = spwID
        self.tau = tau
        self.etaF = etaF
        interTab = InterpolateTableTime(self,self.spwID)
        tb.open("%s/SPECTRAL_WINDOW" % inputMs)
        self.freq = tb.getcol("REF_FREQUENCY")[self.spwID]
        tb.close()
        self.specFreq = getFrequencies(inputMs,spwID)
        self.atmRes = {}
        for i in self.atm.antennaNames :
#            print i
            _visVal = Visibility(inputMs,spwID=spwID,antenna1=i,antenna2=i)
            scanNums = np.unique(_visVal.subtable.getcol('SCAN_NUMBER'))
            self.atmRes[i] = {}
            noScan = []
            for m in scanNums :
                self.atmRes[i][m] = {}
                for j in self.loads :
                    stateVal = self.atm.antennaInfo[i][j]['state']
                    self.atmRes[i][m][j] = {}
                    for k in stateVal :
                        try:
                            visVal = Visibility(inputMs,spwID=spwID,antenna1=i,antenna2=i,scan=m,state=k)
                            self.atmRes[i][m][j]['power'] = np.mean(visVal.amp,len(visVal.amp.shape)-1)
                            self.atmRes[i][m][j]['error'] = np.std(visVal.amp,len(visVal.amp.shape)-1)
                            self.atmRes[i][m][j]['time']  = np.mean(visVal.subtable.getcol("TIME"))
                            if j in ["HOT","AMB"] :
                                states = self.atm.antennaInfo[i][j]['state']
                                loadTemps = self.atm.antennaInfo[i][j]['loadTemp']
                                checker = states.index(k)
                                self.atmRes[i][m][j]['loadTemp'] = loadTemps[checker]
                        except:
                            continue
                if self.atmRes[i][m]["AMB"].keys() == [] : noScan.append(m)
            noScan = np.unique(np.array(noScan))
            for m in noScan :
                self.atmRes[i].pop(m)
            counter = 0
            for m in self.atmRes[i].keys() :
                try:
                    if self.atmRes[i][m]['REF'].keys() == [] : self.atmRes[i][m]['REF'] = self.atmRes[i][scanNums[counter-1]]['REF']
                    if self.atmRes[i][m]['HOT'].keys() == [] : self.atmRes[i][m]['HOT'] = self.atmRes[i][scanNums[counter-1]]['HOT']
                    counter+=1
                except:
                    continue
        self.sysInfo = {}
        for i in self.atmRes.keys() :
            self.sysInfo[i] = {}
            for m in self.atmRes[i].keys() :
                print m,i
                self.sysInfo[i][m] = {}
                pHot = self.atmRes[i][m]['HOT']['power']
                eHot = self.atmRes[i][m]['HOT']['error']
                timetHot = self.atmRes[i][m]['HOT']['time']
                tHot = jVal(self.freq,self.atmRes[i][m]['HOT']['loadTemp'])
                pAmb = self.atmRes[i][m]['AMB']['power']
                eAmb = self.atmRes[i][m]['AMB']['error']
                timeAmb = self.atmRes[i][m]['AMB']['time']
                tAmb = jVal(self.freq,self.atmRes[i][m]['AMB']['loadTemp'])
#                print self.atmRes[i][m]['AMB']
                try:
                    pRef = self.atmRes[i][m]['REF']['power']
                    eRef = self.atmRes[i][m]['REF']['error']
                    timeRef = self.atmRes[i][m]['REF']['time']
                except:
                    pRef = np.zeros(self.atmRes[i][m]['AMB']['power'].shape)
                    eRef = pRef
                    timeRf = self.atmRes[i][m]['AMB']['time']
                pSky = self.atmRes[i][m]['SKY']['power']
                eSky = self.atmRes[i][m]['SKY']['error']
                timeSky = self.atmRes[i][m]['SKY']['time']
                tCmb = jVal(self.freq,Tcmb)
                Gain,dGain,Trx,dTrx,Tsky,dTsky,y,dy = calcTrxGain(pHot,pAmb,pSky,tHot,tAmb,pRef,eHot,eAmb,eRef,doRefSub=doRefSub)
                meanTime = (timeAmb+timeSky)/2.0
                interTab.interpolateData(np.array(meanTime),quiet=True)
                tOut = interTab.newData['TEMPERATURE'].mean()
                tAtm = interTab.newData['ATM_TEMP'].mean()
                alph = solveAlpha(tHot,tAmb,tAtm,tOut,etaF)
                tCal,tSys = solveTsys(tAtm,pHot,pAmb,pSky,tCmb,alph,pRef,doRefSub=doRefSub)
                self.sysInfo[i][m]['gain'] = {'value' : Gain, 'error' : dGain}
                self.sysInfo[i][m]['Trx']  = {'value' : Trx, 'error' : dTrx}
                self.sysInfo[i][m]['Tsky'] = {'value' : Tsky, 'error' : dTsky}
                self.sysInfo[i][m]['y']    = {'value' : y, 'error' : dy}
                self.sysInfo[i][m]['Tcal'] = {'value' : tCal, 'error' : 0}
                self.sysInfo[i][m]['Tsys'] = {'value' : tSys, 'error' : 0}
                self.sysInfo[i][m]['Time'] = {'value' : meanTime, 'error' : 0}
#                self.sysInfo[i][m]['Freq'] = {'value' : , 'error' : 0}

def solveTsys(tAtm,pHot,pAmb,pSky,tCmb,alpha,pRef,doRefSub=False) :
    if not doRefSub : pRef = pRef-pRef
    tCal  = tAtm-tCmb
    pLoad = alpha*pHot+(1-alpha)*pAmb
    tSys  = tCal*(pSky-pRef)/(pLoad-pSky)
    return tCal,tSys

def calcTrxGain(pHot,pAmb,pSky,tHot,tAmb,pRef=0,eHot=0,eAmb=0,eSky=0,eRef=0,etHot=0,etAmb=0,Gain=None,dGain=None,Trx=None,dTrx=None,doRefSub=False) :
    if not doRefSub : pRef = pRef-pRef
    if Gain == None  : Gain  = (pHot-pAmb)/(tHot-tAmb)
    if dGain == None : dGain = (((eHot**2.0+eAmb**2.0)/(tHot-tAmb)**2.0)+((pHot-pAmb)**2.0/(tHot-tAmb)**4.0)*(etHot**2.0+etAmb**2.0))**0.5
    if Trx == None   : Trx   = ((pHot-pRef)/Gain)-tHot
    if dTrx == None  : dTrx  = ((eHot/Gain)**2.0+(eRef/Gain)**2.0+((pHot-pRef)*dGain/Gain**2.0)**2.0 + etHot**2.0)**0.5
    Tsky  = tAmb-(pAmb-pSky)/Gain
    dTsky = ((eAmb/Gain)**2.0+(eSky/Gain)**2.0+((pAmb-pSky)*dGain/Gain**2.0)**2.0+etAmb**2.0)**0.5
    y     = (pHot-pRef)/(pAmb-pRef)
    dy    = ((eHot/(pAmb-pRef))**2.0+((pHot-pRef)*eAmb/(pAmb-pRef)**2.0)**2.0+(eRef/(pAmb-pRef)+(pAmb*eRef)/(pAmb-pRef)**2.0)**2.0)**0.5
    return Gain,dGain,Trx,dTrx,Tsky,dTsky,y,dy

def solveAlpha(tHot,tAmb,tAtm,tOut,etaF) :
    return (etaF*tAtm-tAmb+(1-etaF)*tOut)/(tHot-tAmb)

def jVal(freq,temp) :
    import math as m
    x = h*freq/k
    return x*(m.exp(x/temp)-1)**(-1)

def djVal(freq,temp,detemp) :
    x = h*freq*1e9/k
    return abs(x*(m.exp(x/temp)-1)**(-2)*(x*dtemp/temp**2.0)*m.exp(x/temp))

        
class AtmStates:
    def __init__(self,inputMs):
        self.inputMs = inputMs
        tb.open('%s/STATE' % self.inputMs)
        self.loadTemps    = tb.getcol("LOAD")+273.15
        self.stateIntents = tb.getcol("OBS_MODE")
        tb.close()
        self.atmSky = []
        self.atmRef = []
        self.atmHot = []
        self.atmAmb = []
        self.atmScans = []
        self.antennaNames = getAntennaNames(self.inputMs)
        self.antennaInfo  = {}
        self.numAtmCals = 0
        for i in self.antennaNames : self.antennaInfo[i] = {'AMB' : {'state' : [], 'loadTemp' : []},
                                                            'HOT' : {'state' : [], 'loadTemp' : []},
                                                            'REF' : {'state' : []},
                                                            'SKY' : {'state' : []}
                                                           }
        self.getAtmCalTargetStates()
        self.associateStateWithAntenna()

    def getAtmCalTargetStates(self) :
        for i in range(len(self.stateIntents)) :
            if "CALIBRATE_ATMOSPHERE.OFF_SOURCE" in self.stateIntents[i] : self.atmSky.append(i)
            if "CALIBRATE_ATMOSPHERE.REFERENCE"  in self.stateIntents[i] : self.atmRef.append(i)
            if "CALIBRATE_ATMOSPHERE.ON_SOURCE"  in self.stateIntents[i] and self.loadTemps[i] > 330 : self.atmHot.append(i)
            if "CALIBRATE_ATMOSPHERE.ON_SOURCE"  in self.stateIntents[i] and self.loadTemps[i] < 330 : self.atmAmb.append(i)

    def associateStateWithAntenna(self) :
        self.antennaInfo  = {}
        for i in self.antennaNames : self.antennaInfo[i] = {'AMB' : {'state' : [], 'loadTemp' : []},
                                                            'HOT' : {'state' : [], 'loadTemp' : []},
                                                            'REF' : {'state' : []},
                                                            'SKY' : {'state' : []}
                                                           }
        visTemp = Visibility(self.inputMs,antenna1=None,antenna2=None,spwID=None,state=None)

        for i in self.atmAmb :
            try:
                visTemp.setState(i)
                ant1 = visTemp.subtable.getcol('ANTENNA1')
                ant2 = visTemp.subtable.getcol('ANTENNA2')
                goodIndex = np.where(ant1 == ant2)[0]
                goodIndex = list(goodIndex)
                antennaIds = np.unique(ant1[goodIndex])
                for j in antennaIds :
                    antName = self.antennaNames[j]
                    antName = getAntennaNames(self.inputMs)[j]
                    self.antennaInfo[antName]['AMB']['state'].append(i)
                    self.antennaInfo[antName]['AMB']['loadTemp'].append(self.loadTemps[i])
            except:
                continue
        for i in self.atmHot :
            try:
                visTemp.setState(i)
                ant1 = visTemp.subtable.getcol('ANTENNA1')
                ant2 = visTemp.subtable.getcol('ANTENNA2')
                goodIndex = np.where(ant1 == ant2)[0]
                goodIndex = list(goodIndex)
                antennaIds = np.unique(ant1[goodIndex])
                for j in antennaIds :
                    antName = self.antennaNames[j]
                    self.antennaInfo[antName]['HOT']['state'].append(i)
                    self.antennaInfo[antName]['HOT']['loadTemp'].append(self.loadTemps[i])
            except:
                continue
        for i in self.atmRef :
            try:
                visTemp.setState(i)
                ant1 = visTemp.subtable.getcol('ANTENNA1')
                ant2 = visTemp.subtable.getcol('ANTENNA2')
                goodIndex = np.where(ant1 == ant2)[0]
                goodIndex = list(goodIndex)
                antennaIds = np.unique(ant1[goodIndex])
                for j in antennaIds :
                    antName = self.antennaNames[j]
                    self.antennaInfo[antName]['REF']['state'].append(i)
            except:
                continue
        for i in self.atmSky :
            try:
               visTemp.setState(i)
               ant1 = visTemp.subtable.getcol('ANTENNA1')
               ant2 = visTemp.subtable.getcol('ANTENNA2')
               goodIndex = np.where(ant1 == ant2)[0]
               goodIndex = list(goodIndex)
               antennaIds = np.unique(ant1[goodIndex])
               for j in antennaIds :
                   antName = self.antennaNames[j]
                   self.antennaInfo[antName]['SKY']['state'].append(i)
            except:
                continue

class processDVTiltMeter:
    def __init__(self,dvTiltmeterFile,outFile=None):
        self.dvTiltmeterFile = dvTiltmeterFile
        self.fulltable = fiop.fileToTable(dvTiltmeterFile,keepType=True)
        self.columns   = fiop.getInvertTable(self.fulltable)
        self.oldtime   = self.columns[0]
        self.time = np.array(convertTimeStamps(self.columns[0]))
        self.time = self.time-self.time[0]
        self.antenna = self.columns[1]
        self.an0    = np.array(self.columns[2])
        self.aw0    = np.array(self.columns[3])
        self.x     = np.array(self.columns[4])
        self.y     = np.array(self.columns[5])
        self.t1    = np.array(self.columns[6])
        self.t2    = np.array(self.columns[7])
        if outFile <> None :
            self.outFile = outFile
            self.fitT2Trend()
            self.removeTrend()
            self.writeResiduals()

    def fitT1Trend(self) :
        self.px = polyfit(self.t1,self.x,1)
        self.py = polyfit(self.t1,self.y,1)
        self.newX=polyval(self.px,self.t1)
        self.newY=polyval(self.py,self.t1)

    def fitT2Trend(self) :
        self.px = polyfit(self.t2,self.x,1)
        self.py = polyfit(self.t2,self.y,1)
        self.newX=polyval(self.px,self.t2)
        self.newY=polyval(self.py,self.t2)

    def removeTrend(self) :
        self.x = self.newX-self.x
        self.y = self.newY-self.y

    def restoreTrend(self) :
        self.x = self.x+self.newX
        self.y = self.y+self.newY

    def plotTime(self) :
        return
    def plotT1(self) :
        return
    def plotT2(self) : 
        return

    def writeResiduals(self) :
        f = open(self.outFile,'w')
        for i in range(len(self.antenna)) :
            f.write("%s %s %f %f %f %f %f %f\n" % (self.oldtime[i],self.antenna[i],self.an0[i],self.aw0[i],self.newX[i],self.newY[i],self.t1[i],self.t2[i]) )
        f.close()
        
def fixMyDelays(asdm,caltableName=None,vis=None,doImport=True,sign=1) :
    """
    This function will extract the TelCal solutions for delay and generate a
    calibration table, which is useful if the solutions were not applied during
    observations.
    This version will handle single or multiple receiver bands in the ASDM.
    There are two main use cases for this command:
      1) If you have already run importasdm with asis='*', then you don't 
         need to specify the asdm, only the ms:
         fixMyDelays('','my.delaycal',vis='uid.ms',False)
     2) If you want to use this function as a wrapper for importasdm, then 
        you can say:  fixMyDelays('uid__blah_blah',None,None,True)
     The optional 'sign' parameter can be used to flip the sign if and when 
     someone changes the sign convention in TelCal.
    - Todd Hunter
    """
    asis = '*'
    if (len(asdm) > 0):
        [asdm,dir] = locate(asdm)
        print "asdm = %s" % (asdm)
    if (vis == None):
        vis = "%s.ms" % asdm.split('/')[-1]
        print "vis = %s" % (vis)
    elif (len(vis) < 1):
        vis = "%s.ms" % asdm.split('/')[-1]
        print "vis = %s" % (vis)
    if (caltableName == None) or (len(caltableName) < 1):
        if (len(asdm) == 0):
            caltableName = "%s.delaycal" % (vis)
        else:
            caltableName = "%s.delaycal" % asdm.split('/')[-1]
    if doImport : importasdm(asdm=asdm,asis=asis,vis=vis,overwrite=True)
    antennaIds = getAntennaNames(vis)
    bbands     = getBasebandAndBandNumbers(vis)
#    print bbands
    tb.open("%s/ASDM_CALDELAY" % vis)
    antennaNames = tb.getcol("antennaName")
    if (len(antennaNames) < 1):
        print "The ASDM_CALDELAY table has no data.  Delay correction cannot be done."
        return
    delayOffsets = tb.getcol("delayOffset")
    basebands    = tb.getcol("basebandName") # format is 'BB_1'
#    print "basebands = ", basebands
    rxbands    = tb.getcol("receiverBand")    # format is 'ALMA_RB_%02d'
#    print "rxbands = ", rxbands
    polList = tb.getcol("polarizationTypes")  # e.g. X or Y or ...
    tb.close()
    outList = []
    spwNames = []
    rxBands = []
    for j in bbands:
        rxName = "ALMA_RB_%02d" % (j[0])
        rxBands.append(j[0])
        bbName = "BB_%i" % (j[1])
        spwNames.append("%d"%j[2])
#        print "bbName=%s, rxName=%s" % (bbName, rxName)
        for i in antennaIds :
#            print "ith antenna = ", i
            ant = np.where(antennaNames == i)
            bb  = np.where(basebands == bbName)
            rx  = np.where(rxbands == rxName)
#            ind = np.intersect1d_nu(ant,bb)
            newlist = np.intersect1d(ant[0],bb[0])
            newlist = np.intersect1d(newlist,rx[0])
#            print "newlist = ", newlist
            ind = np.unique(newlist)
#            print "ind = ", ind
            if (sign < 0):
                print "Applying the reverse sign of the delays to %s, ant %s" %(bbName,i)
            p  = sign*delayOffsets[:,ind].mean(1)*1e9
            for k in p :
                outList.append(k)
    parameter = outList
    pol = ''
    print "polList[:,0] = ", polList[:,0]
    print "spws = ", spwNames
    print "rx bands = ", rxBands
    for k in range(len(polList[:,0])):
        pol += "%s," % (polList[:,0][k])
    pol = pol[:-1]
    
    antenna   = stringListJoin(np.unique(antennaNames))
    spw       = stringListJoin(spwNames)
    print "Removing any old caltable = %s." % (caltableName)
    os.system('rm -rf %s'%(caltableName))
    print "Calling gencal('%s','%s','sbd','%s','%s','%s')" % (vis,caltableName,spw,antenna,pol)
    gencal(vis=vis,caltable=caltableName,caltype='sbd',spw=spw,antenna=antenna,pol=pol,parameter=parameter)
    # end of fixMyDelays
            
def getUniqueBasebandNumbers(inputMs) :
    """
    Returns the list of baseband numbers in the specified ms.
    """
    tb.open("%s/SPECTRAL_WINDOW" % inputMs)
    bb = tb.getcol("BBC_NO")
    tb.close()
    return np.unique(bb)

def freqToBand(freq):
    """
    Returns the ALMA band that can observe the specified frequency.
    It will accept either a single frequency or a list.
    It is kind of a kludge until something better is devised.
    """
    band = []
    for f in freq:
        if (f > 750e9):
            band.append(10)
        elif (f > 550e9):
            band.append(9)
        elif (f > 379e9):
            band.append(8)
        elif (f > 275e9):
            band.append(7)
        elif (f > 211e9):
            band.append(6)
        elif (f > 163e9):
            band.append(5)
        elif (f > 120e9):
            band.append(4)
        elif (f > 84e9):
            band.append(3)
        elif (f > 60e9):
            band.append(2)
        elif (f > 30e9):
            band.append(1)
    return(band)
        
def getBasebandAndBandNumbers(inputMs) :
# new experimental version to try to deal with band 6/9 phase transfer data
# which uses a bit of a kludge to convert freq to receiverBand
# return orderedlist:  rxBand, baseBand, spw
    tb.open("%s/SPECTRAL_WINDOW" % inputMs)
    bb = tb.getcol("BBC_NO")
    freq = tb.getcol("REF_FREQUENCY")
    numchan = tb.getcol("NUM_CHAN")
    tbw = tb.getcol("TOTAL_BANDWIDTH")
    band = freqToBand(freq)
    pair = []
    for i in range(len(band)):
        # remove the channel-average spws and the WVR spws
        if (numchan[i] > 1 and tbw[i] < 7e9):
            pair.append((band[i],bb[i],i))
    tb.close()
    return np.unique(pair)

def getBasebandAndBandNumbersTest(inputMs) :
# New experimental version to try to deal with band 6/9 phase transfer data
# which tries to determine receiverBand properly, but seems to be impossible.
# return orderedlist:  rxBand, baseBand, spw
# Todd Hunter
    tb.open("%s/SPECTRAL_WINDOW" % inputMs)
    bb = tb.getcol("BBC_NO")
    numchan = tb.getcol("NUM_CHAN")
    tbw = tb.getcol("TOTAL_BANDWIDTH")
    tb.close()
    tb.open("%s/ASDM_RECEIVER" % inputMs)
    spws = tb.getcol('spectralWindowId')
    bands = tb.getcol('frequencyBand')
    tb.close()
    pair = []
    band = []
    for i in range(len(numchan)):
      window = ('SpectralWindow_%d'%(i))
      print "window = ", window
      findspw = np.where(spws==window)
      if (len(findspw) > 0):
        print "findspw = ", findspw
        band.append(bands[findspw[0][0]])
        # remove the channel-average spws and the WVR spws
        if (numchan[i] > 1 and tbw[i] < 7e9):
            pair.append((band[i],bb[i],i))
    return np.unique(pair)

def stringListJoin(inp,combo=',') :
    out = ''
    for i in inp : out+=("%s%s" % (str(i),combo))
    return out[:-1]

def printLOs(inputMs, sort='spw', showWVR=False,
             showCentralFreq=False, verbose=False):
    """
    Print the LO settings for an MS.
    Options:
    sort: 'spw', otherwise sort by row number in ASDM_RECEIVER table
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/PrintLOs
    Todd Hunter
    """
    try:
        [LOs,bands,spws,names,sidebands,receiverIds] = getLOs(inputMs)
    except:
        return
    bbc = getBasebandNumbers(inputMs)
    # find gap in spectralWindowId numbering in ASDM_RECEIVER
    gap = 0
    for i in range(len(spws)):
        if (spws[i] > 0):
            gap = spws[i] - 1
            break
    for i in range(len(spws)):
        if (spws[i] > 0):
            if (verbose):
                print "Renaming spw%d to spw%d" % (spws[i],spws[i]-gap)
            spws[i] -= gap
    index = range(len(spws))
    print 'Row refers to the row number in the ASDM_RECEIVER table.'
    if (showCentralFreq):
        print 'Row spw BB RxBand CenFreq Nchan LO1(GHz) LO2(GHz) Sampler YIG(GHz) TFBoffset(MHz)'
    else:
        print 'Row spw BB RxBand Ch1Freq Nchan LO1(GHz) LO2(GHz) Sampler YIG(GHz) TFBoffset(MHz)'
    for i in range(len(LOs)):
        freqs = getFrequencies(inputMs,spws[index[i]]) * (1e-9)
        meanFreqGHz = np.mean(freqs)
        if (index[i] >= len(LOs)):
            row = 'n/a'
        else:
            row = str(index[i])
        myspw = spws[index[i]]
        if (bands[index[i]].split('_')[-1].isdigit()):
            rxband = bands[index[i]].split('_')[-1]
        else:
            rxband = 'WVR'
        line = "%2s  %2d  %d %3s " % (row,myspw,bbc[spws[index[i]]],rxband)
        if (showCentralFreq):
            line += "%10.6f %4d " % (meanFreqGHz,len(freqs))
        else:
            line += "%10.6f %4d " % (freqs[0],len(freqs))
        if (receiverIds[index[i]] != 0):
            # receiverIds > 0 seem to be bogus repeats of spws
            continue
        if (LOs[index[i]][0] < 0):
            print line
            continue
        for j in range(len(LOs[index[i]])):
            if (bbc[spws[index[i]]]>0):
                if (j != 2):
                    line = line + '%10.6f'%(LOs[index[i]][j]*1e-9)
                else:
                    line = line + '%5.2f'%(LOs[index[i]][j]*1e-9)
        yig = LOs[index[i]][0] / yigHarmonic(bands[index[i]])
        if (yig > 0):
            line = line + ' %.6f' % (yig*1e-9)
        if (len(freqs) > 256):
            # work out what LO4 must have been
            LO1 = LOs[index[i]][0]
            LO2 = LOs[index[i]][1]
            LO3 = LOs[index[i]][2]
            if (sidebands[index[i]][0] == 'USB'):
                IFlocation = LO3 - (LO2 - (meanFreqGHz*1e9 - LO1))
            else:
                IFlocation = LO3 - (LO2 - (LO1 -  meanFreqGHz*1e9))
            LO4 = 2e9 + IFlocation
            TFBLOoffset = LO4-3e9
            line += '%9.3f %+8.3f' % (LO4 * 1e-6,  TFBLOoffset * 1e-6)
            
        if (bands[index[i]] == 'ALMA_RB_06'):
            if (len(LOs[index[i]]) > 1):
                if (LOs[index[i]][1] < 11.3e9 and LOs[index[i]][1] > 10.5e9):
                    line = line + ' YIG may leak into this spw'
                    yigLeakage = LOs[index[i]][0] + (LOs[index[i]][1] - LOs[index[i]][2]) + (yig - LOs[index[i]][1])
                    if (yigLeakage > 0):
                        line = line + ' at %.6f' % (yigLeakage*1e-9)
        print line

def yigHarmonic(bandString):
    """
    Returns the YIG harmonic for the specified ALMA band, given as a string used in casa tables.
    For example:  yigHarmonic('ALMA_RB_03')  returns the integer 6.
    Todd Hunter
    """
    # remove any leading spaces
    #bandString = bandString[bandString.find('ALMA_RB'):]
    harmonics = {'ALMA_RB_03':6, 'ALMA_RB_04':6, 'ALMA_RB_06': 18, 'ALMA_RB_07': 18, 'ALMA_RB_08':18, 'ALMA_RB_09':27}
    try:
        harmonic = harmonics[bandString]
    except:
        harmonic = -1
    return(harmonic)

def printLOsFromASDM(sdmfile, spw='', showCentralFreq=False, showYIG=True):
    """
    Prints the values of LO1, LO2 and the TFB LO offset (if applicable).  If no
    spw is specified, then it prints the values for all spws in the ASDM.
    For further help and examples, see
    https://safe.nrao.edu/wiki/bin/view/ALMA/PrintLOsFromASDM
    Todd Hunter
    """
    if (os.path.exists(sdmfile)==False):
        print "Could not find this ASDM file."
        return
    if (spw != ''):
      if (type(spw) == list):
        spw = spw
      elif (type(spw) == str):
        a = spw.split(',')
        spw = [int(x) for x in a]
      else:
        spw = [int(spw)]
    scandict = getLOsFromASDM(sdmfile)
    scandictspw = getSpwsFromASDM(sdmfile)
    if (showCentralFreq):
        line = "spw BB# Chan RxId CenFrq(GHz) LO1(GHz)  LO2(GHz) Sampler"
    else:
        line = "spw BB# Chan RxId Ch1Frq(GHz) LO1(GHz)  LO2(GHz) Sampler"
    if (showYIG):
        line += '  YIG(GHz)'
    line += ' TFBoffset(MHz)'
    
    print line

    # find gap in spectral window numbering in Receiver.xml
    gap = 0
    for i in range(len(scandict)):
        if (scandict[i]['spectralWindowId'] > 0):
            gap = scandict[i]['spectralWindowId']-1
            break
    for j in range(len(scandictspw)):
      # find spw
      myspw = -1
      for i in range(len(scandict)):
          if (scandictspw[j]['spectralWindowId'] == scandict[i]['spectralWindowId']):
              myspw = i
              break
      if (myspw < 0):
            continue
      if (spw == '' or j in spw):
        if (j >= gap+1):  
            myline =  " %2d %2d " % (scandict[myspw]['spectralWindowId']-gap,
                                     scandictspw[j]['basebandNumber'])
        else:
            myline =  " %2d %2d " % (scandict[myspw]['spectralWindowId'],
                                     scandictspw[j]['basebandNumber'])
        if (scandict[myspw]['frequencyBand'].split('_')[-1].isdigit()):
            rxband = scandict[myspw]['frequencyBand'].split('_')[-1]
        else:
            rxband = 'WVR'
        myline += '%4d  %3s ' % (scandictspw[j]['numChan'],
#                                  scandict[myspw]['receiverId'])
                                  rxband)
        if (showCentralFreq):
            myline += '%10.6f  ' % (1e-9*scandictspw[j]['centerFreq'])
        else:
            myline += '%10.6f  ' % (1e-9*scandictspw[j]['chanFreqStart'])
        if (scandictspw[j]['basebandNumber'] == 0):
            # don't show (irrelevant) LO for WVR
            print myline
            continue
        for i in range(scandict[myspw]['numLO']):
            if (i<2):
                myline += '%10.6f' % (scandict[myspw]['freqLO'][i]*1e-9)
            else:
                myline += '%5.2f' % (scandict[myspw]['freqLO'][i]*1e-9)
        if (showYIG):
            yigHarm = yigHarmonic(scandict[myspw]['frequencyBand'])
            yig = 1e-9 * scandict[myspw]['freqLO'][0] / (yigHarm*1.0)
            myline += '%10.6f' % yig
        if (scandictspw[j]['numChan'] > 256):
            # work out what LO4 must have been
            LO1 = scandict[myspw]['freqLO'][0]
            LO2 = scandict[myspw]['freqLO'][1]
            LO3 = scandict[myspw]['freqLO'][2]
            if (scandictspw[j]['sideband'] > 0):
                IFlocation = LO3 - (LO2 - (scandictspw[j]['centerFreq'] - LO1))
            else:
                IFlocation = LO3 - (LO2 - (LO1 - scandictspw[j]['centerFreq']))
            LO4 = 2e9 + IFlocation
            TFBLOoffset = LO4-3e9
            myline += '%9.3f %+8.3f' % (LO4 * 1e-6,  TFBLOoffset * 1e-6)
        print myline
    
def getLOsFromASDM(sdmfile):
    """
    Returns a dictionary of the LO values for every spw in the specified ASDM.
    Dictionary contents: numLO, freqLO, spectralWindowId.  freqLO is itself a
    list of floating point values.
    Todd Hunter
    """
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)

    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    xmlscans = minidom.parse(sdmfile+'/Receiver.xml')
    scandict = {}
    rowlist = xmlscans.getElementsByTagName("row")
    fid = 0
    for rownode in rowlist:
        scandict[fid] = {}
        rownumLO = rownode.getElementsByTagName("numLO")
        numLO = int(rownumLO[0].childNodes[0].nodeValue)
        rowfreqLO = rownode.getElementsByTagName("freqLO")
        rowreceiverId = rownode.getElementsByTagName("receiverId")
        receiverId = int(rowreceiverId[0].childNodes[0].nodeValue)
        rowfrequencyBand = rownode.getElementsByTagName("frequencyBand")
        frequencyBand = rowfrequencyBand[0].childNodes[0].nodeValue
        freqLO = []
        r = filter(None,(rowfreqLO[0].childNodes[0].nodeValue).split(' '))
        for i in range(2,len(r)):
            freqLO.append(float(r[i]))
        
        rowspwid = rownode.getElementsByTagName("spectralWindowId")
        spwid = int(str(rowspwid[0].childNodes[0].nodeValue).split('_')[1])
        scandict[fid]['spectralWindowId'] = spwid
        scandict[fid]['freqLO'] = freqLO
        scandict[fid]['numLO'] = numLO
        scandict[fid]['receiverId'] = receiverId
        scandict[fid]['frequencyBand'] = frequencyBand
        fid +=1
    return(scandict)


def getLOs(inputMs, verbose=True):
    """
    Reads the LO information from an ms's ASDM_RECEIVER table.  It returns
    a list of four lists: [freqLO,band,spws,names]
    Todd Hunter
    """
    if (os.path.exists(inputMs)):
        if (os.path.exists("%s/ASDM_RECEIVER" % inputMs)):
            try:
                tb.open("%s/ASDM_RECEIVER" % inputMs)
            except:
                print "Could not open the existing ASDM_RECEIVER table"
                return([])
        else:
            if (verbose):
                print "The ASDM_RECEIVER table for this ms does not exist."
            return([])
    else:
        print "This ms does not exist."
        return([])
        
    numLO = tb.getcol('numLO')
    freqLO = []
    band = []
    spws = []
    names = []
    sidebands = []
    receiverIds = []
    for i in range(len(numLO)):
        freqLO.append(tb.getcell('freqLO',i))
        band.append(tb.getcell('frequencyBand',i))
        spws.append(int((tb.getcell('spectralWindowId',i).split('_')[1])))
        names.append(tb.getcell('name',i))
        sidebands.append(tb.getcell('sidebandLO',i))
        receiverIds.append(int(tb.getcell('receiverId',i)))
    tb.close()
    return([freqLO,band,spws,names,sidebands,receiverIds])
    

def getSpwOfBB(inputMs,bbNum,kind='spectral') :
    if kind <> 'spectral' and kind <> 'chanaver' : return "you must specify kind to be spectral or chanaver"
    tb.open("%s/SPECTRAL_WINDOW" % inputMs)
    bbNums = tb.getcol("BBC_NO")
    numChan = tb.getcol("NUM_CHAN")
    tb.close()
    id1 = np.where(bbNums == bbNum)
    if kind == 'spectral' :
        idx = np.where(numChan <> 1)
        idy = np.where(numChan <> 4)
        id2 = np.intersect1d_nu(idx,idy)
    if kind == 'chanaver' :
        id2 = np.where(numChan == 1)
    ind = np.intersect1d_nu(id1,id2)
    return ind[0]

def fixMyDelaysOneReceiver(asdm,caltableName=None,vis=None,doImport=True) :
    """
    This function will extract the TelCal solutions for delay and generate a
    calibration table, which is useful if the solutions were not applied during
    observations.
    **** This version only works for single receiver band data. ****
    The regular fixMyDelays() works for single or multiple receiver data.
    There are two main use cases for this command:
    1) If you have already run importasdm with asis='*', then you don't need to specify
       the asdm, only the ms:
         fixMyDelays('','my.delaycal',vis='uid.ms',False)
    2) If you want to use this function as a wrapper for importasdm, then you can say:
         fixMyDelays('uid__blah_blah',None,None,True)
    """
    asis = '*'
    if (len(asdm) > 0):
        [asdm,dir] = locate(asdm)
        print "asdm = %s" % (asdm)
    if vis == None :
        vis = "%s.ms" % asdm.split('/')[-1]
        print "vis = %s" % (vis)
    if caltableName == None:
        caltableName = "%s.delaycal" % asdm.split('/')[-1]
    if doImport : importasdm(asdm=asdm,asis=asis,vis=vis,overwrite=True)
    antennaIds = getAntennaNames(vis)
    bbands     = getUniqueBasebandNumbers(vis)
    tb.open("%s/ASDM_CALDELAY" % vis)
    antennaNames = tb.getcol("antennaName")
#    referenceAntenna = tb.getcol("refAntennaName")[0]
    delayOffsets = tb.getcol("delayOffset")
    basebands    = tb.getcol("basebandName")
    polList = tb.getcol("polarizationTypes")  # e.g. X or Y or ...
    tb.close()
    outList = []
    spwNames = []
    for j in bbands[1:] :
        bbName = "BB_%i" % (j)
        spwNames.append(getSpwOfBB(vis,j))
        for i in antennaIds :
            ant = np.where(antennaNames == i)
            bb  = np.where(basebands == bbName)
#            ind = np.intersect1d_nu(ant,bb)
            newlist = np.intersect1d(ant[0],bb[0])
            ind = np.unique(newlist)
            p  = delayOffsets[:,ind].mean(1)*1e9
            for k in p :
                outList.append(k)
    parameter = outList
    pol = ''
# This was an old workaround (before we found the polarizationTypes column):
#    polList = ['XX','YY']
    print "polList[:,0] = ", polList[:,0]
    for k in range(len(polList[:,0])):
        pol += "%s," % (polList[:,0][k])
    pol = pol[:-1]
    
    antenna   = stringListJoin(np.unique(antennaNames))
    spw       = stringListJoin(spwNames)
#    print "Calling gencal('%s','%s','sbd','%s','%s','%s')" % (vis,caltableName,spw,antenna,pol)
    gencal(vis=vis,caltable=caltableName,caltype='sbd',spw=spw,antenna=antenna,pol=pol,parameter=parameter)

def mjdVectorToUTHours(mjd):
    return(24*(np.modf(mjd)[0]))

def mjdSecondsVectorToMJD(mjdsec):
    return(mjdsec / 86400.0)

def call_qa_time(arg, form='', prec=0):
    """
    This is a wrapper for qa.time(), which in casa 3.5 returns a list of strings instead
    of just a scalar string.  
    - Todd Hunter
    """
    result = qa.time(arg, form=form, prec=prec)
    if (type(result) == list or type(result) == np.ndarray):
        return(result[0])
    else:
        return(result)

def mjdSecondsListToDateTime(mjdsecList):
    """
    Takes a list of mjd seconds and converts it to a list of datetime structures.
    """
    dt = []
    typelist = type(mjdsecList)
    if not (typelist == list or typelist == np.ndarray):
        mjdsecList = [mjdsecList]
    for mjdsec in mjdsecList:
        today = me.epoch('utc','today')
        mjd = mjdsec / 86400.
        today['m0']['value'] =  mjd
        hhmmss = call_qa_time(today['m0'])
        date = qa.splitdate(today['m0'])  # date is now a dict
        mydate = datetime.datetime.strptime('%d-%d-%d %d:%d:%d'%(date['monthday'],date['month'],date['year'],date['hour'],date['min'],date['sec']),'%d-%m-%Y %H:%M:%S')
        dt.append(mydate)
    return(dt)

def mjdToJD(MJD):
    """
    Converts an MJD value to JD
    """
    JD = MJD + 2400000.5
    return(JD)

def mjdToUT(mjd):
    """
    Converts an MJD value to a UT date and time string
    """
    utstring = mjdSecondsToMJDandUT(mjd*86400)[1]
    return(utstring)
        
def mjdSecondsToMJDandUT(mjdsec):
    """
    Converts a value of MJD seconds into MJD, and into a UT date/time string.
    For example:  2011-01-04 13:10:04 UT
    Caveat: only works for a scalar input value
    Todd Hunter
    """
    today = me.epoch('utc','today')
    mjd = mjdsec / 86400.
    today['m0']['value'] =  mjd
    hhmmss = call_qa_time(today['m0'])
    date = qa.splitdate(today['m0'])
    utstring = "%s-%02d-%02d %s UT" % (date['year'],date['month'],date['monthday'],hhmmss)
    return(mjd, utstring)

def dateStringToMJD(datestring):
    """
    Convert a date/time string to floating point MJD
    Input date format: 2011/10/15 05:00:00  or   2011/10/15-05:00:00
                    or 2011-10-15 05:00:00
    The time portion is optional.
    -- Todd Hunter
    """
    if (datestring.find('/') < 0):
        if (datestring.count('-') != 2):
            print "Date format: 2011/10/15 05:00:00  or   2011/10/15-05:00:00"
            print"           or 2011-10-15 05:00:00"
            print "The time portion is optional."
            return(None)
        else:
            d = datestring.split('-')
            datestring = d[0] + '/' + d[1] + '/' + d[2]
    return(dateStringToMJDSec(datestring)/86400.)

def dateStringToMJDSec(datestring='2011/10/15 05:00:00',datestring2='',help=False):
    """
    Converts a date string into MJD seconds.  This is useful for passing
    time ranges to plotms, because they must be specified in mjd seconds.
    Either of these formats is valid: 2011/10/15 05:00:00
                                      2011/10/15-05:00:00
    The time portion is optional.
    If a second string is given, both values will be converted and a
    string will be created that can be used as a plotrange in plotms.
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/DateStringToMJDSec
    Todd Hunter
    """
    if (help):
        print "dateStringToMJDSec(datestring='2011/10/15 05:10:54', datestring2='')"
        print "   the other valid format is: 2011/10/15-05:10:54"
        print "   The time portion is optional."
        print "   The result can be passed to plotms as the time-axis plotrange."
        print "   If a second string is given, both values will be converted and a"
        print "   plotrange string will be created. "
        return
    mydate = datestring.split()
    if (len(mydate) < 2):
        mydate = datestring.split('-')
    hours = 0
    if (len(mydate) > 1):
        mytime = (mydate[1]).split(':')
        for i in range(len(mytime)):
            hours += float(mytime[i])/(60.0**i)
#        print "hours = %f" % (hours)
#    print "mydate[0] = ", mydate[0]
    mjd = me.epoch('utc',mydate[0])['m0']['value'] + hours/24.
    mjdsec = 86400*mjd
    print "MJD= %.5f, MJDseconds= %.1f, JD= %.5f" % (mjd, mjdsec, mjdToJD(mjd))
    if (len(datestring2) > 0):
        mydate2 = datestring2.split()
        if (len(mydate2) < 1):
            return(mjdsec)
        if (len(mydate2) < 2):
            mydate2 = datestring2.split('-')
        hours = 0
        if (len(mydate2) > 1):
            mytime2 = (mydate2[1]).split(':')
            if (len(mytime2) > 1):
              for i in range(len(mytime2)):
                hours += float(mytime2[i])/(60.0**i)
#            print "hours = %f" % (hours)
        mjd = me.epoch('utc',mydate2[0])['m0']['value'] + hours/24.
        mjdsec2 = mjd*86400
        print "plotrange = [%.1f, %.1f, 0, 0]" % (mjdsec, mjdsec2)
        return([mjdsec,mjdsec2])
    else:
        return(mjdsec)
        
        
    
def plotPointingResults(vis='', figfile=False, source='',buildpdf=False,
                        gs='gs', convert='convert',
                        help=False, verbose=False, labels=False):
    """
    This function will plot the pointing results for an ms, assuming that the
    ASDM_CALPOINTING table was filled, e.g. by importasdm(asis='*').  See also
    plotPointingResultsFromASDM(). The default behavior is to plot all sources
    that were pointed on.  In order to plot just one, then give the source
    name.  Setting labels=True will draw tiny antenna names at the points.
    If buildpdf does not work because gs or convert are not in the standard
    location, then you can specify the full paths with gs='' and convert=''.
    For further help and examples, see
    https://safe.nrao.edu/wiki/bin/view/ALMA/PlotPointingResults
    Todd Hunter
    """
    if (help):
        print "Usage: plotPointingResults(vis, source='', figfile=False, buildpdf=False, gs='gs', convert='convert',labels=False)"
        print "  Default is to plot all sources.  To plot just one, give the source name."
        print "  Setting labels=True will draw tiny antenna names at the points."
        return
    fname = '%s/ASDM_CALPOINTING' % vis
    if (os.path.exists(fname)):
#        print "Trying to open table = %s" % fname
        tb.open(fname)
    else:
        fname = './ASDM_CALPOINTING'
        if (os.path.exists(fname)):
            print "Looking for table in current directory"
            tb.open(fname)
        else:
            print "No ASDM_CALPOINTING table found."
            return
    colOffsetRelative = 206264.8*tb.getcol('collOffsetRelative')
    calDataId = tb.getcol('calDataId')
    antennaName = tb.getcol('antennaName')
    colError = 206264.8*tb.getcol('collError')
    pols = tb.getcol('polarizationTypes')
    startValidTime = tb.getcol('startValidTime')
    uniqueAntennaNames = np.unique(antennaName)
    if (len(startValidTime) == 0):
        print "ASDM_CALPOINTING table is empty."
        return
    (mjd, utstring) = mjdSecondsToMJDandUT(startValidTime[0])
    print 'time = %f = %f = %s' % (startValidTime[0], mjd, utstring)
    tb.close()
    #
    fname = '%s/ASDM_CALDATA'%vis
    if (os.path.exists(fname)):
        tb.open(fname)
    else:
        fname = './ASDM_CALDATA'
        if (os.path.exists(fname)):
            tb.open('./ASDM_CALDATA')
        else:
            print "Could not open ASDM_CALDATA table."
            return
    calDataList = tb.getcol('calDataId')
    calType = tb.getcol('calType')
    startTimeObserved = tb.getcol('startTimeObserved')
    endTimeObserved = tb.getcol('endTimeObserved')
    #        matches = np.where(calDataList == calDataId[0])
    matches = np.where(calType == 'CAL_POINTING')[0]
    print "Found %d pointing scans" % len(matches)
    nscans = len(matches)
    tb.close()
    plotfiles = []
    filelist = ''
    if (type(figfile)==str):
     if (figfile.find('/')):
      directories = figfile.split('/')
      directory = ''
      for d in range(len(directories)):
          directory += directories[d] + '/'
      if (os.path.exists(directory)==False):
          print "Making directory = ", directory
          os.system("mkdir -p %s" % directory)

    vm = ValueMapping(vis)
    previousSeconds = 0
    antenna = 0
    for i in range(nscans):
        index = matches[i]
        # determine sourcename
        mytime = 0.5*(startTimeObserved[index]+endTimeObserved[index])
        scan = vm.getScansForTime(mytime)
#        print "Scan = ", scan
        [avgazim,avgelev] = listazel(vis,scan,antenna,vm)
        pointingSource = vm.getFieldsForScan(scan)[0]
        print "Field = ", pointingSource
        if (source != ''):
            if (source != pointingSource):
                print "Source does not match the request, skipping"
                continue
        pb.clf()
        adesc = pb.subplot(111)
        c = ['blue','red']
        seconds = []
        for p in range(len(pols)):
            seconds.append(np.max(np.max(np.abs(colOffsetRelative[p][0]))))
            seconds.append(np.max(np.max(np.abs(colOffsetRelative[p][1]))))
        seconds = np.max(seconds)*1.20
        if (seconds <= previousSeconds):
            seconds = previousSeconds
        previousSeconds = seconds
        pb.xlim([-seconds,seconds])
        pb.ylim([-seconds,seconds])
        thisscan0 = np.where(startTimeObserved[index] < startValidTime)[0]
        thisscan1 = np.where(endTimeObserved[index] > startValidTime)[0]
        thisscan = np.intersect1d(thisscan0,thisscan1)
        pb.hold(True)
        for a in thisscan: # range(len(antennaName[thisscan])):
            for p in range(len(pols)):
                if (verbose):
                    print "%s: %.2f, %.2f = %.2e, %.2e" % (uniqueAntennaNames[a],
                       colOffsetRelative[p][0][a], colOffsetRelative[p][1][a], 
                       colOffsetRelative[p][0][a]/206264.8, colOffsetRelative[p][1][a]/206264.8)
                pb.plot(colOffsetRelative[p][0][a], colOffsetRelative[p][1][a], 'o', markerfacecolor=overlayColors[a], markersize=6, color=overlayColors[a])
                if (labels):
                    pb.text(colOffsetRelative[p][0][a], colOffsetRelative[p][1][a], uniqueAntennaNames[a], color='k', size=7)
        # Draw spec
        cir = pb.Circle((0, 0), radius=2, facecolor='none', edgecolor='k', linestyle='dotted')
        pb.gca().add_patch(cir)
        pb.hold(False)
        pb.title('Relative collimation offsets at %s - %s scan %d' % (utstring,pointingSource,scan),fontsize=12)
        pb.axvline(0,-seconds,seconds,color='k',linestyle='--')
        pb.axhline(0,-seconds,seconds,color='k',linestyle='--')
        yFormatter = ScalarFormatter(useOffset=False)
        adesc.yaxis.set_major_formatter(yFormatter)
        adesc.xaxis.set_major_formatter(yFormatter)
        minorTickSpacing = 1.0
        xminorLocator = MultipleLocator(minorTickSpacing)
        yminorLocator = MultipleLocator(minorTickSpacing)
        adesc.xaxis.set_minor_locator(xminorLocator)
        adesc.yaxis.set_minor_locator(yminorLocator)
        majorTickSpacing = 5.0
        majorLocator = MultipleLocator(majorTickSpacing)
        adesc.xaxis.set_major_locator(majorLocator)
        adesc.yaxis.set_major_locator(majorLocator)
        adesc.xaxis.grid(True,which='major')
        adesc.yaxis.grid(True,which='major')
        pb.xlabel('Cross-elevation offset (arcsec)')
        pb.ylabel('Elevation offset (arcsec)')
        myxlim = [-seconds,seconds]
        myylim = [-seconds,seconds]
        xrange = myxlim[1]-myxlim[0]
        yrange = myylim[1]-myylim[0]
        y0 = myylim[1]-np.abs(yrange*0.05)
        xreturn = 0
        for a in range(len(antennaName[thisscan])):
            if ((a % 5) == 0):
                y0 -= yrange*0.04
                xreturn = a
            x0 = myxlim[0] + 0.14*xrange*(a-xreturn) + 0.05*xrange
            pb.text(x0, y0, uniqueAntennaNames[a], color=overlayColors[a],fontsize=14)
        x0 = myxlim[0] + 0.05*xrange
        y0 = myylim[1]-np.abs(yrange*0.05)
        pb.text(x0,y0,'azim=%+.0f, elev=%.0f'%(avgazim,avgelev),color='k', fontsize=14)
        pb.text(myxlim[0]+0.02*xrange, myylim[1]+0.05*yrange,vis,fontsize=12,color='k')
        pb.axis('scaled')
        pb.axis([-seconds,seconds,-seconds,seconds])
        if (figfile==True):
            myfigfile = vis+'.pointing.%s.png'%(pointingSource)
            pb.savefig(myfigfile,density=144)
            plotfiles.append(myfigfile)
            print "Figure left in %s" % myfigfile
#            print "i, len(plotfiles) = ", i, len(plotfiles)
        elif (figfile != False):
            myfigfile=figfile
            pb.savefig(figfile,density=144)
            plotfiles.append(myfigfile)
            print "Figure left in %s" % myfigfile
        else:
            print "To make a hardcopy, re-run with figfile=True or figfile='my.png'"
        if (buildpdf == True):
             cmd = '%s -density 144 %s %s.pdf'%(convert,myfigfile,myfigfile)
             print "Running command = ", cmd
             mystatus = os.system(cmd)
             if (mystatus == 0):
                print "plotfiles[i] = ", plotfiles[i]
                filelist += plotfiles[i] + '.pdf '
             else:
                print "ImageMagick's convert command is missing, no PDF created."
                buildpdf = False
        pb.draw()
        if (i < nscans-1):
            mystring = raw_input("Press return for next scan (or 'q' to quit): ")
            if (mystring.lower().find('q') >= 0):
                return
    if (buildpdf):
      pdfname = vis+'.pointing.pdf'
      if (nscans > 1):
          cmd = '%s -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=%s %s' % (gs,pdfname,filelist)
      else:
          cmd = 'cp %s %s' % (filelist,pdfname)
      print "Running command = %s" % (cmd)
      mystatus = os.system(cmd)
      if (mystatus == 0):
          print "PDF left in %s" % (pdfname)
          os.system("rm -f %s" % filelist)
      else:
          print "ghostscript is missing, no PDF created"
    
def readscans(asdm):
    """
    This function was ported from a version originally written by Steve Myers
    for EVLA.  It works for both ALMA and EVLA data.  It returns a dictionary
    containing: startTime, endTime, timerange, sourcename, intent, nsubscans, duration
    Todd Hunter
    """
    # This function lives in a separate .py file.
    return(rs.readscans(asdm))

def listscans(asdm):
    """
    This function was ported from a version originally written by Steve Myers
    for EVLA.  It works for both ALMA and EVLA data.  It prints the summary
    of each scan in the ASDM and the total time on each source. For further
    help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/Listscans
    Todd Hunter
    """
    # This function resides in an external .py file.
    return(rs.listscans(rs.readscans(asdm)))

def plotPointingResultsFromASDM(asdm='', figfile=False, buildpdf=False, gs='gs', convert='convert',
                                help=False,labels=False,xrange=[0,0],yrange=[0,0]):
    """
    This function will plot the pointing results for an ASDM. To use an ms instead,
    see plotPointingResults(). The default behavior is to plot all sources
    that were pointed on.  In order to plot just one, then give the source name.
    Setting labels=True will draw tiny antenna names at the points.
    If buildpdf does not work because gs or convert are not in the standard
    location, then you can specify the full paths with gs='' and convert=''.
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/PlotPointingResultsFromASDM
    Todd Hunter
    """
    if (help):
        print "Usage: plotPointingResultsFromASDM(asdm, figfile=False, buildpdf=False, gs='gs', "
        print "          convert='convert', labels=False, xrange=[0,0], yrange=[0,0])"
        print "  Default is to plot all sources.  To plot just one, give the source name."
        print "  Setting labels=True will draw tiny antenna names at the points."
        return
    source = ''
    [colOffsetRelative,antennaName,startValidTime,pols,calDataID,azim,elev,colError] = readCalPointing(asdm)
    # Need to match calDataId to scan numbers
        
    scandict = rs.readscans(asdm)[0]
    caldict = readCalData(asdm)  # this contains translation from calDataId to scan number
    uniqueScans = 1+np.array(range(len(scandict)))  # in this list, scans appear only once
    scans = []  # in this list, each scan will appear N times, where N=number of antennas
    for i in calDataID:
      for c in range(len(caldict)):
        if (int(caldict[c]['calDataId'].split('_')[1]) == i):
            scans.append(caldict[c]['scan'])

    pointingScans = []
    for i in range(len(caldict)):
        if (int(caldict[i]['calDataId'].split('_')[1]) in calDataID):
            pointingScans.append(caldict[i]['scan'])
    uniquePointingScans = np.unique(pointingScans)
    
    print "pointing scans = ", pointingScans
    nPointingScans = len(uniquePointingScans)
    if (len(startValidTime) == 0):
        print "CalPointing.xml table is empty."
        return
    
    plotfiles = []
    filelist = ''
    if (type(figfile)==str):
     if (figfile.find('/')):
      directories = figfile.split('/')
      directory = ''
      for d in range(len(directories)):
          directory += directories[d] + '/'
      if (os.path.exists(directory)==False):
          print "Making directory = ", directory
          os.system("mkdir -p %s" % directory)

    previousSeconds = 0
    antenna = 0
    uniqueAntennaNames = np.unique(antennaName)
    for i in range(nPointingScans):
        pb.clf()
        adesc = pb.subplot(111)
        c = ['blue','red']
        seconds = []
        for p in range(len(pols)):
            seconds.append(np.max(np.max(np.abs(colOffsetRelative[p][0]))))
            seconds.append(np.max(np.max(np.abs(colOffsetRelative[p][1]))))
        seconds = np.max(seconds)*1.20
        if (seconds <= previousSeconds):
            seconds = previousSeconds
        previousSeconds = seconds
        pb.xlim([-seconds,seconds])
        pb.ylim([-seconds,seconds])
        thisscan = np.where(scans == uniquePointingScans[i])[0]
        pb.hold(True)
        for a in thisscan:
            antenna = np.where(uniqueAntennaNames==antennaName[a])[0][0]
            for p in range(len(pols[a])):
#                pb.plot(colOffsetRelative[a][p][0], colOffsetRelative[a][p][1], 'o',
#                        markerfacecolor=overlayColors[antenna], markersize=6,
#                         color=overlayColors[antenna])
                pb.errorbar(colOffsetRelative[a][p][0], colOffsetRelative[a][p][1], fmt='o',
                            yerr=colError[a][p][1], xerr=colError[a][p][0],
                            color=overlayColors[antenna], markersize=5,
                            markerfacecolor=overlayColors[antenna])
                if (labels):
                    pb.text(colOffsetRelative[a][p][0], colOffsetRelative[a][p][1], antennaName[a], color='k', size=7)
        # Draw spec
        cir = pb.Circle((0, 0), radius=2, facecolor='none', edgecolor='k', linestyle='dotted')
        pb.gca().add_patch(cir)
#        print "scandict = ", scandict
#        print "len(scandict) = ", len(scandict)
#        print "\n scandict[%d].keys() = " % (uniquePointingScans[i]), scandict[uniquePointingScans[i]].keys()
        pointingSource = scandict[uniquePointingScans[i]]['source']
        (mjd, utstring) = mjdSecondsToMJDandUT(startValidTime[thisscan[0]])
        pb.title('Relative collimation offsets at %s - %s scan %d' % (utstring,pointingSource,uniquePointingScans[i]),fontsize=12)
        pb.axvline(0,-seconds,seconds,color='k',linestyle='--')
        pb.axhline(0,-seconds,seconds,color='k',linestyle='--')
        pb.xlabel('Cross-elevation offset (arcsec)')
        pb.ylabel('Elevation offset (arcsec)')
        myxlim = [-seconds,seconds]
        myylim = [-seconds,seconds]
        myxrange = myxlim[1]-myxlim[0]
        myyrange = myylim[1]-myylim[0]
        y0 = myylim[1]-np.abs(myyrange*0.05)
        xreturn = 0
        for a in range(len(uniqueAntennaNames)):
            if ((a % 5) == 0):
                y0 -= myyrange*0.04
                xreturn = a
            x0 = myxlim[0] + 0.14*myxrange*(a-xreturn) + 0.05*myxrange
            pb.text(x0, y0, uniqueAntennaNames[a], color=overlayColors[a],fontsize=14)
        x0 = myxlim[0] + 0.05*myxrange
        y0 = myylim[1]-np.abs(myyrange*0.05)
        pb.text(x0,y0,'azim=%+.0f, elev=%.0f'%(azim[thisscan[0]],elev[thisscan[0]]),color='k', fontsize=14)
        pb.text(myxlim[0]+0.02*myxrange, myylim[1]+0.05*myyrange, asdm, fontsize=12,color='k')
        pb.axis('scaled')
        if (xrange[0] != 0 or xrange[1] != 0):
            if (yrange[0] != 0 or yrange[1] != 0):
                pb.axis([xrange[0],xrange[1],yrange[0],yrange[1]])
            else:
                pb.axis([xrange[0],xrange[1],-seconds,seconds])
        elif (yrange[0] != 0 or yrange[1] != 0):
            pb.axis([-seconds,seconds, yrange[0], yrange[1]])
        else:
            pb.axis([-seconds,seconds,-seconds,seconds])
        yFormatter = ScalarFormatter(useOffset=False)
        adesc.yaxis.set_major_formatter(yFormatter)
        adesc.xaxis.set_major_formatter(yFormatter)
        adesc.xaxis.grid(True,which='major')
        adesc.yaxis.grid(True,which='major')
        if (figfile==True):
            myfigfile = asdm+'.pointing.%s.png' % (pointingSource)
            pb.savefig(myfigfile,density=144)
            plotfiles.append(myfigfile)
            print "Figure left in %s" % myfigfile
        elif (figfile != False):
            myfigfile=figfile
            pb.savefig(figfile,density=144)
            plotfiles.append(myfigfile)
            print "Figure left in %s" % myfigfile
        else:
            print "To make a hardcopy, re-run with figfile=True or figfile='my.png'"
        if (buildpdf == True):
             cmd = '%s -density 144 %s %s.pdf'%(convert,myfigfile,myfigfile)
             print "Running command = ", cmd
             mystatus = os.system(cmd)
             if (mystatus == 0):
                print "plotfiles[i] = ", plotfiles[i]
                filelist += plotfiles[i] + '.pdf '
             else:
                print "ImageMagick's convert command is missing, no PDF created."
                buildpdf = False
        pb.draw()
        if (i < nPointingScans-1):
            mystring = raw_input("Press return for next scan (or 'q' to quit): ")
            if (mystring.lower().find('q') >= 0):
                return
    if (buildpdf):
      pdfname = asdm+'.asdmpointing.pdf'
      if (nPointingScans > 1):
          cmd = '%s -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=%s %s' % (gs,pdfname,filelist)
      else:
          cmd = 'cp %s %s' % (filelist,pdfname)
      print "Running command = %s" % (cmd)
      mystatus = os.system(cmd)
      if (mystatus == 0):
          print "PDF left in %s" % (pdfname)
          os.system("rm -f %s" % filelist)
      else:
          print "ghostscript is missing, no PDF created"
            
def offlineTcAtmosphere(asdm,scanlist,mode='AH',origin='specauto') :
    for scan in scanlist :
        print scan
        tc_atmosphere(asdm=asdm,dataorigin=origin,trecmode=mode,scans=str(scan),
                      antennas='',calresult=asdm,showplot=False,verbose=False)

def readwvr(sdmfile):
    """
    This function reads the CalWVR.xml table from the ASDM and returns a
    dictionary containing: 'start', 'end', 'startmjd', 'endmjd',
    'startmjdsec', 'endmjdsec',
    'timerange', 'antenna', 'water', 'duration'.
    'water' is the zenith PWV in meters.
    This function is called by readpwv(). -- Todd Hunter
    """

    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    xmlscans = minidom.parse(sdmfile+'/CalWVR.xml')
    scandict = {}
    rowlist = xmlscans.getElementsByTagName("row")
    fid = 0
    for rownode in rowlist:
        rowpwv = rownode.getElementsByTagName("water")
        pwv = float(rowpwv[0].childNodes[0].nodeValue)
        water = pwv
        scandict[fid] = {}

        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName("startValidTime")
        start = int(rowstart[0].childNodes[0].nodeValue)
        startmjd = float(start)*1.0E-9/86400.0
        t = qa.quantity(startmjd,'d')
        starttime = call_qa_time(t,form="ymd",prec=8)
        rowend = rownode.getElementsByTagName("endValidTime")
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjd = float(end)*1.0E-9/86400.0
        t = qa.quantity(endmjd,'d')
        endtime = call_qa_time(t,form="ymd",prec=8)
        # antenna
        rowantenna = rownode.getElementsByTagName("antennaName")
        antenna = str(rowantenna[0].childNodes[0].nodeValue)

        scandict[fid]['start'] = starttime
        scandict[fid]['end'] = endtime
        scandict[fid]['startmjd'] = startmjd
        scandict[fid]['endmjd'] = endmjd
        scandict[fid]['startmjdsec'] = startmjd*86400
        scandict[fid]['endmjdsec'] = endmjd*86400
        timestr = starttime+'~'+endtime
        scandict[fid]['timerange'] = timestr
        scandict[fid]['antenna'] = antenna
        scandict[fid]['water'] = water
        scandict[fid]['duration'] = (endmjd-startmjd)*86400
        fid += 1

    print '  Found ',rowlist.length,' rows in CalWVR.xml'

    # return the dictionary for later use
    return scandict
# Done

def readpwv(asdm):
  """
  This function assembles the dictionary returned by readwvr() into arrays
  containing the PWV measurements written by TelCal into the ASDM.
  -- Todd Hunter
  """
#  print "Entered readpwv"
  dict = readwvr(asdm)
#  print "Finished readwvr"
  bigantlist = []
  for entry in dict:
      bigantlist.append(dict[entry]['antenna'])
  antlist = np.unique(bigantlist)
  watertime = []
  water = []
  antenna = []
  for entry in dict:
      measurements = 1
      for i in range(measurements):
          watertime.append(dict[entry]['startmjdsec']+(i*1.0/measurements)*dict[entry]['duration'])
          water.append(dict[entry]['water'])
          antenna.append(dict[entry]['antenna'])
  return([watertime,water,antenna])   

def getScienceFrequencies(vis):
  # Return a list of the mean frequencies of each spw with more than 4 channels
  # i.e. not the WVR data nor the channel-averaged data.  -- Todd Hunter
  #
    freqs = []
    try:
        tb.open("%s/SPECTRAL_WINDOW" % vis)
    except:
        print "Could not open ms table = %s" % (vis)
        return(freqs)
    numChan = tb.getcol("NUM_CHAN")
    for i in range(len(numChan)):
        if (numChan[i] > 4):
            chanFreq = tb.getcell("CHAN_FREQ",i)
            freqs.append(np.mean(chanFreq))
    tb.close()
    return freqs
    
def listConditionsFromASDM(asdm, station=16, verbose=True):
    """
    This function extracts the weather conditions for the specified ASDM,
    and computes and returns a dictionary containing the median values.
    The default weather station to use is 16.
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/ListConditionsFromASDM
    Todd Hunter
    """
    [conditions, medianConditions] = getWeatherFromASDM(asdm,station=station)
    if (verbose):
        print "Median weather values for %s to %s" % (plotbp.utstring(conditions[0][0]),plotbp.utstring(conditions[0][-1]))
        print "  Pressure = %.2f mb" % (medianConditions['pressure'])
        print "  Temperature = %.2f C" % (medianConditions['temperature'])
#        print "  Dew point = %.2f C" % (medianConditions['dewpoint'])
        print "  Relative Humidity = %.2f %%" % (medianConditions['humidity'])
        print "  Wind speed = %.2f m/s" % (medianConditions['windSpeed'])
        print "  Wind max = %.2f m/s" % (np.max(conditions[6]))
        print "  Wind direction = %.2f deg" % (medianConditions['windDirection'])
    return(medianConditions)

def listConditions(vis='', scan='', antenna='0',verbose=True,asdm='',reffreq=0,
                   byscan=False, help=False):
  """
  Compiles the mean weather, pwv and opacity values for the given scan
  number or scan list for the specified ms.  If a scan number
  is not provided it returns the average over the whole ms.
  Setting byscan=True will return a dictionary with conditions per scan.
  Scan can be a single list: [1,2,3] or '1,2,3' or a single range: '1~4'.
  For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/Listconditions
  --- Todd Hunter
  """
  return(listconditions(vis,scan,antenna,verbose,asdm,reffreq,byscan,help))

def listconditions(vis='', scan='', antenna='0',verbose=True,asdm='',reffreq=0,
                   byscan=False, help=False):
  """
  Compiles the mean weather, pwv and opacity values for the given scan
  number or scan list for the specified ms.  If a scan number
  is not provided it returns the average over the whole ms.
  Setting byscan=True will return a dictionary with conditions per scan.
  Scan can be a single list: [1,2,3] or '1,2,3' or a single range: '1~4'.
  For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/Listconditions
  Todd Hunter
  """
  if (help):
      print "Usage: listconditions(vis='', scan='', antenna='0', verbose=True, asdm='', byscan=False)"
      print "  Leaving scan = '': will average the conditions over the whole track."
      print "  Scan can be a single list: [1,2,3] or '1,2,3' or a single range: '1~4'"
      print "  Setting byscan=True: will return a dictionary with conditions per scan"
      return
  if (os.path.exists(vis)==False):
      print "Could not find the ms = %s." % (vis)
      return
  vm = 0
  if (reffreq==0):
      freqs = 1e-9*np.array(getScienceFrequencies(vis))
  else:
      freqs = [reffreq]
  if (type(scan) == str):
      if (scan.find(',')>0):
          scan = [int(k) for k in scan.split(',')]
      elif (scan.find('~')>0):
          scan = range(int(scan.split('~')[0]),int(scan.split('~')[1])+1)
  if (byscan and (type(scan) == list or scan=='')):
      conditions = {}
      if (scan == ''):
          vm = ValueMapping(vis)
          scan = np.unique(vm.scans)
          print "Scans = ", scan
      for i in scan:
          [cond,myTimes,vm] = getWeather(vis,i,antenna,verbose,vm)
          if (len(myTimes) > 0):
              [pwv,pwvstd] = getMedianPWV(vis,myTimes,asdm,verbose)
              if (pwvstd > 0):
                  if (pwv > 0):
                      tau = []
                      zenithtau = []
                      for myfreq in range(len(freqs)):
                          reffreq = freqs[myfreq]
                          [z,t] = estimateOpacity(pwv, reffreq, cond,verbose)
                          zenithtau.append(z)
                          tau.append(t)
                      d2 = {}
                      d2['tauzenith'] = np.mean(zenithtau)
                      d2['tau'] = np.mean(tau)
                      d2['transmissionzenith'] = np.exp(-np.mean(zenithtau))
                      d2['transmission'] = np.exp(-np.mean(tau))
                      d2['pwv'] = pwv
                      d2['pwvstd'] = pwvstd
                      cond = dict(cond.items() + d2.items())
          conditions[i] = cond
  else:
    [conditions,myTimes,vm] = getWeather(vis,scan,antenna,verbose,vm)
    if (len(myTimes) < 1):
      return(conditions)
    [pwv,pwvstd] = getMedianPWV(vis,myTimes,asdm,verbose)
    if (pwvstd < 0):
      return(conditions)
    if (pwv > 0):
      tau = []
      zenithtau = []
      for i in range(len(freqs)):
        reffreq = freqs[i]
        [z,t] = estimateOpacity(pwv, reffreq, conditions,verbose)
        zenithtau.append(z)
        tau.append(t)
      d2 = {}
      d2['tauzenith'] = np.mean(zenithtau)
      d2['tau'] = np.mean(tau)
      d2['transmissionzenith'] = np.exp(-np.mean(zenithtau))
      d2['transmission'] = np.exp(-np.mean(tau))
      d2['pwv'] = pwv
      d2['pwvstd'] = pwvstd
      conditions = dict(conditions.items()+d2.items())
  return(conditions)


def readCalData(sdmfile):
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    xmlscans = minidom.parse(sdmfile+'/CalData.xml')
    scandict = {}
    rowlist = xmlscans.getElementsByTagName("row")
    fid = 0
    for rownode in rowlist:
        scandict[fid] = {}
        rowscan = rownode.getElementsByTagName("scanSet")
        tokens = rowscan[0].childNodes[0].nodeValue.split()
        scan = int(tokens[2])

        rowcaldataid = rownode.getElementsByTagName("calDataId")
        caldataid = str(rowcaldataid[0].childNodes[0].nodeValue)
        scandict[fid]['calDataId'] = caldataid
        scandict[fid]['scan'] = scan
        fid +=1 
    return(scandict)

def readCalPointingTable(sdmfile):
    """
    Reads the CalPointing.xml table for the specified ASDM
    and returns a dictionary of values.
    Todd Hunter
    """
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    xmlscans = minidom.parse(sdmfile+'/CalPointing.xml')
    scandict = {}
    rowlist = xmlscans.getElementsByTagName("row")
    fid = 0
    for rownode in rowlist:
        scandict[fid] = {}
        rowAntennaName = rownode.getElementsByTagName("antennaName")
        antenna = str(rowAntennaName[0].childNodes[0].nodeValue)

        rowDirection = rownode.getElementsByTagName("direction")
#        for r in range(len(rowDirection)):
#            for c in range(len(rowDirection[r].childNodes)):
#                print "%d %d = " % (r,c), rowDirection[r].childNodes[c].nodeValue
        tokens = rowDirection[0].childNodes[0].nodeValue.split()
        azimuth = float(tokens[2])
        elevation = float(tokens[3])

        rowFrequency = rownode.getElementsByTagName("frequencyRange")
        tokens = rowFrequency[0].childNodes[0].nodeValue.split()
        frequency1 = float(tokens[2])
        frequency2 = float(tokens[3])
        frequency = 0.5*(frequency1+frequency2)

        rowRelative = rownode.getElementsByTagName("collOffsetRelative")
        tokens = rowRelative[0].childNodes[0].nodeValue.split()
        azOffset = float(tokens[3])
        elOffset = float(tokens[4])
        azOffset2 = float(tokens[5])
        elOffset2 = float(tokens[6])
        
        rowRelative = rownode.getElementsByTagName("collError")
        tokens = rowRelative[0].childNodes[0].nodeValue.split()
        azError = float(tokens[3])
        elError = float(tokens[4])
        azError2 = float(tokens[5])
        elError2 = float(tokens[6])
        
        rowCalDataId = rownode.getElementsByTagName("calDataId")
        calDataId = str(rowCalDataId[0].childNodes[0].nodeValue)
        scan = int(calDataId.split('_')[1])

        rowpol = rownode.getElementsByTagName("polarizationTypes")
        tokens = rowpol[0].childNodes[0].nodeValue.split()
        poltypes = []
        poltypes.append(str(tokens[2]))
        poltypes.append(str(tokens[3]))

        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName("startValidTime")
        start = int(rowstart[0].childNodes[0].nodeValue)/1000000000
        startmjd = start/86400.0
        t = qa.quantity(startmjd,'d')
        starttime = call_qa_time(t,form="ymd",prec=8)
        rowend = rownode.getElementsByTagName("endValidTime")
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjd = float(end)*1.0E-9/86400.0
        t = qa.quantity(endmjd,'d')
        endtime = call_qa_time(t,form="ymd",prec=8)

        scandict[fid]['startValidTime'] = start
        scandict[fid]['endValidTime'] = end
        scandict[fid]['start'] = starttime
        scandict[fid]['end'] = endtime
        scandict[fid]['startmjd'] = startmjd
        scandict[fid]['endmjd'] = endmjd
        scandict[fid]['startmjdsec'] = startmjd*86400
        scandict[fid]['endmjdsec'] = endmjd*86400
        timestr = starttime+'~'+endtime
        scandict[fid]['azimuth'] = azimuth
        scandict[fid]['elevation'] = elevation
        scandict[fid]['antenna'] = antenna
        scandict[fid]['frequency'] = frequency
        scandict[fid]['azOffset'] = azOffset
        scandict[fid]['elOffset'] = elOffset
        scandict[fid]['azOffset2'] = azOffset2
        scandict[fid]['elOffset2'] = elOffset2
        scandict[fid]['azError'] = azError
        scandict[fid]['elError'] = elError
        scandict[fid]['azError2'] = azError2
        scandict[fid]['elError2'] = elError2
        scandict[fid]['scan'] = scan
        scandict[fid]['duration'] = (endmjd-startmjd)*86400
        scandict[fid]['polarizationTypes'] = poltypes
        fid += 1

    print '  Found ',rowlist.length,' rows in CalPointing.xml'

    # return the dictionary for later use
    return scandict
# end of readCalPointingTable(sdmfile):

def readCalPointing(asdm):
  """
  Calls readCalPointingTable() and converts the returned dictionary to a list
  of lists that is subsequently used by plotPointingResultsFromASDM().
  Todd Hunter
  """
  dict = readCalPointingTable(asdm)
  colOffsetRelative = []
  colError = []
  antennaName = []
  pols = []
  scans = []
  startValidTime = []
  azim = []
  elev = []
  for entry in dict:
      colOffsetRelative.append([[dict[entry]['azOffset'],dict[entry]['elOffset']],[dict[entry]['azOffset2'],dict[entry]['elOffset2']]])
      antennaName.append(dict[entry]['antenna'])
      startValidTime.append(dict[entry]['startValidTime'])
      pols.append(dict[entry]['polarizationTypes'])
      scans.append(dict[entry]['scan'])
      azim.append(dict[entry]['azimuth'])
      elev.append(dict[entry]['elevation'])
      colError.append([[dict[entry]['azError'], dict[entry]['elError']], [dict[entry]['azError2'], dict[entry]['elError2']]])
  return([206264.8*np.array(colOffsetRelative), antennaName, startValidTime, pols, scans,
          np.array(azim)*180/math.pi,np.array(elev)*180/math.pi, 206264.8*np.array(colError)])   


def getMedianPWV(vis='.', myTimes=[0,999999999999], asdm='', verbose=False, help=False):
    """
    Extracts the PWV measurements from the WVR on all antennas for the
    specified time range.  The time range is input as a two-element list of
    MJD seconds.  First, it tries to find ASDM_CALWVR in the ms.  If that fails, 
    it then tries to find CalWVR.xml in an ASDM of the same name.  If neither of 
    these exist, then it tries to find CalWVR.xml in the present working directory.
    If it still fails, it looks for CalWVR.xml in the .ms directory.  Thus,
    you only need to copy this xml file from the ASDM into your ms, rather
    than the entire ASDM.
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/GetMedianPWV
    -- Todd Hunter
    """
    pwvmean = 0
    success = False
    if (help):
        print "getMedianPWV(vis='.', mytimes=[0,999999999999], asdm='', verbose=False)"
        print "  Compute the median and standard deviation of the PWV over the specified"
        print "  time range in the ms (default = all times).  If it does not find the"
        print "  ASDM_CALWVR table, it will look for the CalWVR.xml file in the asdm (if"
        print "  specified), or in the present working directory.  If this fails, it looks"
        print "  CalWVR.xml in the .ms directory."
        return
    if (verbose):
        print "in getMedianPWV with myTimes = ", myTimes
    try:
      if (os.path.exists("%s/ASDM_CALWVR"%vis)):
        tb.open("%s/ASDM_CALWVR" % vis)
        pwvtime = tb.getcol('startValidTime')  # mjdsec
        antenna = tb.getcol('antennaName')
        pwv = tb.getcol('water')
        tb.close()
        success = True
        if (verbose):
            print "Opened ASDM_CALWVR table, len(pwvtime)=", len(pwvtime)
      elif (verbose):
        print "Did not find ASDM_CALWVR table in the ms"
    except:
      if (verbose):
        print "Could not open ASDM_CALWVR table in the ms"
    finally:
     # try to find the ASDM table
     if (success == False):
      if (len(asdm) > 0):
        try:
            [pwvtime,pwv,antenna] = readpwv(asdm)
        except:
            if (verbose):
               print "Could not open ASDM = %s" % (asdm)
            return(pwvmean)
      else:
        try:
            tryasdm = vis.split('.ms')[0]
            if (verbose):
               print "No ASDM name provided, so I will try this name = %s" % (tryasdm)
            [pwvtime,pwv,antenna] = readpwv(tryasdm)
        except:
            try:
                if (verbose):
                    print "Still did not find it.  Will look for CalWVR.xml in current directory."
                [pwvtime, pwv, antenna] = readpwv('.')
            except:
                try:
                    if (verbose):
                        print "Still did not find it.  Will look for CalWVR.xml in the .ms directory."
                    [pwvtime, pwv, antenna] = readpwv('%s/'%vis)
                except:
                    if (verbose):
                        print "No CalWVR.xml file found, so no PWV retrieved. Copy it to this directory and try again."
                    return(pwvmean,-1)
    try:
        matches = np.where(np.array(pwvtime)>myTimes[0])[0]
    except:
        print "Found no times > %d" % (myTimes[0])
        return(0)
    ptime = np.array(pwvtime)[matches]
    matchedpwv = np.array(pwv)[matches]
    matches2 = np.where(ptime<myTimes[-1])[0]
    if (len(matches2) < 1):
        # look for the value with the closest start time
        mindiff = 1e12
        for i in range(len(pwvtime)):
            if (abs(myTimes[0]-pwvtime[i]) < mindiff):
                mindiff = abs(myTimes[0]-pwvtime[i])
                pwvmean = pwv[i]*1000
        matchedpwv = []
        for i in range(len(pwvtime)):
            if (abs(abs(myTimes[0]-pwvtime[i]) - mindiff) < 1.0):
                matchedpwv.append(pwv[i])
        pwvmean = 1000*np.median(matchedpwv)
        if (verbose):
            print "Taking the median of %d pwv measurements from all antennas = %.3f mm" % (len(matchedpwv),pwvmean)
        pwvstd = np.std(matchedpwv)
    else:
        pwvmean = 1000*np.median(matchedpwv[matches2])
        pwvstd = np.std(matchedpwv[matches2])
        if (verbose):
            print "Taking the median of %d pwv measurements from all antennas = %.3f mm" % (len(matches2),pwvmean)
    return(pwvmean,pwvstd)
# end of getMedianPWV

def ReadWeatherStation(scandict, station):
    timeInterval = []
    pressure = []
    relHumidity = []
    temperature = []
    windDirection = []
    windSpeed = []
    windMax = []
    for entry in range(len(scandict)):
        if (scandict[entry]['stationId'] == station):
            timeInterval.append(scandict[entry]['timeInterval'])
            pressure.append(scandict[entry]['pressure'])
            relHumidity.append(scandict[entry]['relHumidity'])
            temperature.append(scandict[entry]['temperature'])
            windDirection.append(scandict[entry]['windDirection'])
            windSpeed.append(scandict[entry]['windSpeed'])
            windMax.append(scandict[entry]['windMax'])
#        else:
#            print "scandict=%d != %d" % (scandict[entry]['stationId'],station)
    d1 = [timeInterval, pressure, relHumidity, temperature, windDirection,
          windSpeed, windMax, station]
    return(d1)
    
def getWeatherFromASDM(sdmfile='',verbose=False, help=False, station=16):
    """
    Reads the weather table in the ASDM, and returns an array of values,
    and a dictionary of the median values.  Default station is 16, but it
    will search 9, 10, 11, 12 if 16 is not present.
    - T. Hunter
    """
    scandict = readWeatherFromASDM(sdmfile)
    if (verbose):
        print "len(scandict) = ", len(scandict)
    if (station == 9):
        stationOrder = [9,10,11,12,16,19,20]
    elif (station == 10):
        stationOrder = [10,9,11,12,16,19,20]
    elif (station == 11):
        stationOrder = [11,9,10,12,16,19,20]
    elif (station == 12):
        stationOrder = [12,9,10,11,16,19,20]
    elif (station == 16):
        stationOrder = [16,9,10,11,12,19,20]
    elif (station == 19):
        stationOrder = [19,9,10,11,12,16,20]
    elif (station == 29):
        stationOrder = [20,9,10,11,12,16,19]
    for s in stationOrder:
        d1 = ReadWeatherStation(scandict, station=s)
        if (len(d1[0]) > 0):
            break
    if (len(d1[0]) < 1):
        print "No data found from stations: ", stationOrder

    timeInterval, pressure, relHumidity, temperature, windDirection, windSpeed, windMax, station = d1
    d2 = {}
    d2['pressure'] = np.median(pressure)
    d2['humidity'] = np.median(relHumidity)
    d2['temperature'] = np.median(temperature)
    sinWindDirection = np.sin(np.array(windDirection)*np.pi/180)
    cosWindDirection = np.cos(np.array(windDirection)*np.pi/180)
    d2['windDirection'] = (180./np.pi)*np.arctan2(np.mean(sinWindDirection),
                                                  np.mean(cosWindDirection))
    if (d2['windDirection'] < 0):
        d2['windDirection'] += 360
    d2['windSpeed'] = np.median(windSpeed)
    d2['windMax'] = np.median(windMax)
    return(d1,d2)

def readWeatherFromASDM(sdmfile, station=16):
    """
    Reads the weather table in the ASDM, and returns a dictionary containing:
    timeInterval, pressure, relHumidity, temperature, windDirection, windSpeed,
    windMax and stationId.
    - T. Hunter
    """
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    xmlscans = minidom.parse(sdmfile+'/Weather.xml')
    scandict = {}
    rowlist = xmlscans.getElementsByTagName("row")
    fid = 0
    for rownode in rowlist:
        scandict[fid] = {}
        row = rownode.getElementsByTagName("timeInterval")
        tokens = row[0].childNodes[0].nodeValue.split()
        scandict[fid]['timeInterval'] = float(tokens[0])*1e-9  # MJD seconds
        row = rownode.getElementsByTagName("pressure")
        scandict[fid]['pressure'] = float(row[0].childNodes[0].nodeValue)*0.01 # mbar
        row = rownode.getElementsByTagName("relHumidity")
        scandict[fid]['relHumidity'] = float(row[0].childNodes[0].nodeValue)
        row = rownode.getElementsByTagName("temperature")
        scandict[fid]['temperature'] = float(row[0].childNodes[0].nodeValue)
        row = rownode.getElementsByTagName("windDirection")
        scandict[fid]['windDirection'] = float(row[0].childNodes[0].nodeValue)*180/math.pi  # degrees
        row = rownode.getElementsByTagName("windSpeed")
        scandict[fid]['windSpeed'] = float(row[0].childNodes[0].nodeValue)
        row = rownode.getElementsByTagName("windMax")
        scandict[fid]['windMax'] = float(row[0].childNodes[0].nodeValue)
        row = rownode.getElementsByTagName("stationId")
        scandict[fid]['stationId'] = int((str(row[0].childNodes[0].nodeValue)).split('_')[1])
        fid += 1
    return scandict

def readAntennasFromASDM(sdmfile):
    """
    Reads the list of antennas from the ASDM..
    - D Barkats
    """
    
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    xmlscans = minidom.parse(sdmfile+'/Antenna.xml')
    antList = []
    rowlist = xmlscans.getElementsByTagName("row")
    for rownode in rowlist:
        row = rownode.getElementsByTagName("name")
        tokens = row[0].childNodes[0].nodeValue.split()
        antList.append(str(tokens[0]))
    return antList


def getWeather(vis='', scan='', antenna='0',verbose=False, vm=0, help=False):
    """
    Queries the WEATHER and ANTENNA tables of an .ms by scan number or
    list of scAan numbers in order to return mean values of: angleToSun,
      pressure, temperature, humidity, dew point, wind speed, wind direction,
      azimuth, elevation, solarangle, solarelev, solarazim.
    If the sun is below the horizon, the solarangle returns is negated.
    This function needs to run ValueMapping, unless a ValueMapping object
    is passed via the vm argument.
    -- Todd Hunter
    """
    if (help):
        print "Usage: getWeather(vis, scan='', antenna='0', verbose=False, vm=0, help=False)"
        return
    if (verbose):
        print "Entered getWeather with vis,scan,antenna = ", vis, ",", scan, ",", antenna
    try:
      if str(antenna).isdigit():
        antennaName = getAntennaNames(vis)[int(str(antenna))]
      else:
        antennaName = antenna
        try:
          antenna = getAntennaIndex(vis,antennaName)
        except:
          antennaName = string.upper(antenna)
          antenna = getAntennaIndex(vis,antennaName)
    except:
      print "Either the ANTENNA table does not exist or antenna %s does not exist" % (antenna)
      return([0,[],vm])
    try:
      tb.open("%s/POINTING" % vis)
    except:
      print "POINTING table does not exist"
      return([0,0,vm])
    subtable = tb.query("ANTENNA_ID == %s" % antenna)
    tb.close()
    if (vm == 0):
        print "Running ValueMapping... (this may take a minute)"
        vm = ValueMapping(vis)
    else:
        if (verbose):
            print "getWeather: Using current ValueMapping result"
    try:
        tb.open("%s/OBSERVATION" % vis)
        observatory = tb.getcell("TELESCOPE_NAME",0)
        tb.close()
    except:
        print "OBSERVATION table does not exist, assuming observatory == ALMA"
        observatory = "ALMA"
    if (scan == ''):
        scan = vm.uniqueScans
    conditions = {}
    conditions['pressure']=conditions['temperature']=conditions['humidity']=conditions['dewpoint']=conditions['windspeed']=conditions['winddirection'] = 0
    conditions['scan'] = scan
    if (type(scan) == str):
        if (scan.find('~')>0):
            tokens = scan.split('~')
            scan = [int(k) for k in range(int(tokens[0]),int(tokens[1])+1)]
        else:
            scan = [int(k) for k in scan.split(',')]
    if (type(scan) == np.ndarray):
      scan = list(scan)
    if (type(scan) == list):
      myTimes = np.array([])
      for sc in scan:
        try:
            newTimes = vm.getTimesForScan(sc)
        except:
            print "Error reading scan %d, is it in the data?" % (sc)
            return([conditions,[],vm])
        myTimes = np.concatenate((myTimes,newTimes))
    else:
      try:
          myTimes = vm.getTimesForScan(scan)
      except:
          print "Error reading scan %d, is it in the data?" % (scan)
          return([conditions,[],vm])
    if (type(scan) == str):
        scan = [int(k) for k in scan.split(',')]
    if (type(scan) == list):
        listscan = ""
        listfield = []
        for sc in scan:
#            print "Processing scan ", sc
            listfield.append(vm.getFieldsForScan(sc))
            listscan += "%d" % sc
            if (sc != scan[-1]):
                listscan += ","
#        print "listfield = ", listfield
        listfields = np.unique(listfield[0])
        listfield = ""
        for field in listfields:
            listfield += "%s" % field
            if (field != listfields[-1]):
                listfield += ","
    else:
        listscan = str(scan)
        listfield = vm.getFieldsForScan(scan)
    [az,el] = ComputeSolarAzElForObservatory(myTimes[0], observatory)
    [az2,el2] = ComputeSolarAzElForObservatory(myTimes[-1], observatory)
    azsun = np.mean([az,az2])
    elsun = np.mean([el,el2])
    direction = subtable.getcol("DIRECTION")
    azeltime = subtable.getcol("TIME")
    telescopeName = getObservatoryName(vis)
    if (len(direction) > 0 and telescopeName.find('VLA') < 0):
      azimuth = direction[0][0]*180.0/math.pi
      elevation = direction[1][0]*180.0/math.pi
      npat = np.array(azeltime)
      matches = np.where(npat>myTimes[0])[0]
      matches2 = np.where(npat<myTimes[-1])[0]
      conditions['azimuth'] = np.mean(azimuth[matches[0]:matches2[-1]+1])
      conditions['elevation'] = np.mean(elevation[matches[0]:matches2[-1]+1])
      conditions['solarangle'] = angularSeparation(azsun,elsun,conditions['azimuth'],conditions['elevation'])
      conditions['solarelev'] = elsun
      conditions['solarazim'] = azsun
      if (verbose):
          print "Using antenna = %s to retrieve mean azimuth and elevation" % (antennaName)
          print "Separation from sun = %f deg" % (abs(conditions['solarangle']))
      if (elsun<0):
        conditions['solarangle'] = -conditions['solarangle']
        if (verbose):
            print "Sun is below horizon (elev=%.1f deg)" % (elsun)
      else:
        if (verbose):
            print "Sun is above horizon (elev=%.1f deg)" % (elsun)
      if (verbose):
          print "Average azimuth = %.2f, elevation = %.2f degrees" % (conditions['azimuth'],conditions['elevation'])
    else:
      if (verbose):
          print "The POINTING table is blank."
      if (type(scan) == int or type(scan)==np.int32):
          # compute Az/El for this scan
        fieldName = vm.getFieldsForScan(scan)
        if (type(fieldName) == list):
            fieldName = fieldName[0]
        myfieldId = vm.getFieldIdsForFieldName(fieldName)
        if (type(myfieldId) == list or type(myfieldId)==np.ndarray):
            # If the same field name has two IDs (this happens in EVLA data)
            myfieldId = myfieldId[0]
        else:
            print "type(myfieldId) = ", type(myfieldId)
        myscantime = np.mean(vm.getTimesForScans(scan))
        mydirection = getRADecForField(vis, myfieldId)
        if (len(telescopeName) < 1):
            telescopeName = 'ALMA'
        myazel = computeAzElFromRADecMJD(mydirection, myscantime/86400., telescopeName)
        conditions['elevation'] = myazel[1] * 180/math.pi
        conditions['azimuth'] = myazel[0] * 180/math.pi
        conditions['solarangle'] = angularSeparation(azsun,elsun,conditions['azimuth'],conditions['elevation'])
        conditions['solarelev'] = elsun
        conditions['solarazim'] = azsun
        if (verbose):
            print "Separation from sun = %f deg" % (abs(conditions['solarangle']))
        if (elsun<0):
            conditions['solarangle'] = -conditions['solarangle']
            if (verbose):
                print "Sun is below horizon (elev=%.1f deg)" % (elsun)
        else:
            if (verbose):
                print "Sun is above horizon (elev=%.1f deg)" % (elsun)
        if (verbose):
            print "Average azimuth = %.2f, elevation = %.2f degrees" % (conditions['azimuth'],conditions['elevation'])
      elif (type(scan) == list):
          myaz = []
          myel = []
          if (verbose):
              print "Scans to loop over = ", scan
          for s in scan:
              fieldName = vm.getFieldsForScan(s)
              if (type(fieldName) == list):
                  # take only the first pointing in the mosaic
                  fieldName = fieldName[0]
              myfieldId = vm.getFieldIdsForFieldName(fieldName)
              if (type(myfieldId) == list or type(myfieldId)==np.ndarray):
                  # If the same field name has two IDs (this happens in EVLA data)
                  myfieldId = myfieldId[0]
              myscantime = np.mean(vm.getTimesForScans(s))
              mydirection = getRADecForField(vis, myfieldId)
              telescopeName = getObservatoryName(vis)
              if (len(telescopeName) < 1):
                  telescopeName = 'ALMA'
              myazel = computeAzElFromRADecMJD(mydirection, myscantime/86400., telescopeName)
              myaz.append(myazel[0]*180/math.pi)
              myel.append(myazel[1]*180/math.pi)
          conditions['azimuth'] = np.mean(myaz)
          conditions['elevation'] = np.mean(myel)
          conditions['solarangle'] = angularSeparation(azsun,elsun,conditions['azimuth'],conditions['elevation'])
          conditions['solarelev'] = elsun
          conditions['solarazim'] = azsun
          if (verbose):
              print "Using antenna = %s to retrieve mean azimuth and elevation" % (antennaName)
              print "Separation from sun = %f deg" % (abs(conditions['solarangle']))
          if (elsun<0):
              conditions['solarangle'] = -conditions['solarangle']
              if (verbose):
                  print "Sun is below horizon (elev=%.1f deg)" % (elsun)
          else:
              if (verbose):
                  print "Sun is above horizon (elev=%.1f deg)" % (elsun)
          if (verbose):
              print "Average azimuth = %.2f, elevation = %.2f degrees" % (conditions['azimuth'],conditions['elevation'])
          
              
    # now, get the weather
    try:
        tb.open("%s/WEATHER" % vis)
    except:
        print "Could not open the WEATHER table for this ms."
        return([conditions,myTimes,vm])
    if (1==1):
        mjdsec = tb.getcol('TIME')
        indices = np.argsort(mjdsec)
        mjd = mjdsec/86400.
        pressure = tb.getcol('PRESSURE')
        relativeHumidity = tb.getcol('REL_HUMIDITY')
        temperature = tb.getcol('TEMPERATURE')
        if (np.mean(temperature) > 100):
            # must be in units of Kelvin, so convert to C
            temperature -= 273.15        
        dewPoint = tb.getcol('DEW_POINT')
        if (np.mean(dewPoint) > 100):
            # must be in units of Kelvin, so convert to C
            dewPoint -= 273.15        
        if (np.mean(dewPoint) == 0):
            # assume it is not measured and use NOAA formula to compute from humidity:
            dewPoint = ComputeDewPointCFromRHAndTempC(relativeHumidity, temperature)
        sinWindDirection = np.sin(tb.getcol('WIND_DIRECTION'))
        cosWindDirection = np.cos(tb.getcol('WIND_DIRECTION'))
        windSpeed = tb.getcol('WIND_SPEED')
        tb.close()

        # put values into time order (they mostly are, but there can be small differences)
        mjdsec = np.array(mjdsec)[indices]
        pressure = np.array(pressure)[indices]
        relativeHumidity = np.array(relativeHumidity)[indices]
        temperature = np.array(temperature)[indices]
        dewPoint = np.array(dewPoint)[indices]
        windSpeed = np.array(windSpeed)[indices]
        sinWindDirection = np.array(sinWindDirection)[indices]
        cosWindDirection = np.array(cosWindDirection)[indices]

        # find the overlap of weather measurement times and scan times
        matches = np.where(mjdsec>=np.min(myTimes))[0]
        matches2 = np.where(mjdsec<=np.max(myTimes))[0]
#            print "len(matches)=%d, len(matches2)=%d" % (len(matches), len(matches2))
        if (len(matches)>0 and len(matches2) > 0):
            # average the weather points enclosed by the scan time range
            selectedValues = range(matches[0], matches2[-1]+1)
            if (selectedValues == []):
                # there was a either gap in the weather data, or an incredibly short scan duration
                if (verbose):
                    print "----  Finding the nearest weather value --------------------------- "
                selectedValues = findClosestTime(mjdsec, myTimes[0])
        elif (len(matches)>0):
            # all points are greater than myTime, so take the first one
            selectedValues = matches[0]
        elif (len(matches2)>0):
            # all points are less than myTime, so take the last one
            selectedValues = matches2[-1]
        if (type(selectedValues) == np.int64 or type(selectedValues) == np.int32 or  
            type(selectedValues) == np.int):
            conditions['readings'] = 1
            if (verbose):
                print "selectedValues=%d, myTimes[0]=%.0f, myTimes[1]=%.0f, len(matches)=%d, len(matches2)=%d" % (selectedValues,
                   myTimes[0],myTimes[1], len(matches), len(matches2))
                if (len(matches) > 0):
                    print "matches[0]=%f, matches[-1]=%f" % (matches[0], matches[-1])
                if (len(matches2) > 0):
                    print "matches2[0]=%f, matches2[-1]=%d" % (matches2[0], matches2[-1])
        else:
            conditions['readings'] = len(selectedValues)
        conditions['pressure'] = np.mean(pressure[selectedValues])
        if (conditions['pressure'] != conditions['pressure']):
            # A nan value got through, due to no selected values (should be impossible)"
            if (verbose):
                print ">>>>>>>>>>>>>>>>>>>>>>>>  selectedValues = ", selectedValues
                print "len(matches)=%d, len(matches2)=%d" % (len(matches), len(matches2))
                print "matches[0]=%f, matches[-1]=%f, matches2[0]=%f, matches2[-1]=%d" % (matches[0], matches[-1], matches2[0], matches2[-1])
        conditions['temperature'] = np.mean(temperature[selectedValues])
        conditions['humidity'] = np.mean(relativeHumidity[selectedValues])
        conditions['dewpoint'] = np.mean(dewPoint[selectedValues])
        conditions['windspeed'] = np.mean(windSpeed[selectedValues])
        conditions['winddirection'] = (180./math.pi)*np.arctan2(np.mean(sinWindDirection[selectedValues]),np.mean(cosWindDirection[selectedValues]))
        if (conditions['winddirection'] < 0):
            conditions['winddirection'] += 360
        if (verbose):
            print "Mean weather values for scan %s (field %s)" % (listscan,listfield)
            print "  Pressure = %.2f mb" % (conditions['pressure'])
            print "  Temperature = %.2f C" % (conditions['temperature'])
            print "  Dew point = %.2f C" % (conditions['dewpoint'])
            print "  Relative Humidity = %.2f %%" % (conditions['humidity'])
            print "  Wind speed = %.2f m/s" % (conditions['windspeed'])
            print "  Wind direction = %.2f deg" % (conditions['winddirection'])

    return([conditions,myTimes,vm])
    # end of getWeather

def listazel(vis, scan, antenna='0', vm=0):
    """
    Extracts the mean azimuth and elevation for the specified 'scan' in the
    specified ms (whose name is passed in by the vis argument).
    The scan can be a single scan or a list, as in [1,2,3], or in '1,2,3'
    The antenna can be either the antenna number or its name.
    This function needs to run ValueMapping, unless a ValueMapping object
    is passed via the vm argument.
    -- Todd Hunter
    """
    try:
      if str(antenna).isdigit():
        antennaName = getAntennaNames(vis)[int(str(antenna))]
      else:
        try:
          antenna = getAntennaIndex(vis,antennaName)
        except:
          antennaName = string.upper(antenna)
          antenna = getAntennaIndex(vis,antennaName)
    except:
      print "Either the ANTENNA table, or antenna %s does not exist" % str(antenna)
      return([0,0])
    try:
      tb.open("%s/POINTING" % vis)
    except:
      print "POINTING table does not exist"
      return([0,0])
    subtable = tb.query("ANTENNA_ID == %s" % antenna)
    tb.close()
    if (vm==0):
        vm = ValueMapping(vis)
    if (type(scan) == str):
      scan = [int(k) for k in scan.split(',')]
    if (type(scan) == list):
      myTimes = np.array([])
      for sc in scan:
        try:
            newTimes = vm.getTimesForScan(sc)
        except:
            print "Error reading scan %d, is it in the data?" % (sc)
            return
        myTimes = np.concatenate((myTimes,newTimes))
    else:
      try:
          myTimes = vm.getTimesForScan(scan)
      except:
          print "Error reading scan %d, is it in the data?" % (sc)
          return
    direction = subtable.getcol("DIRECTION")
    time = subtable.getcol("TIME")
    azimuth = direction[0][0]*180.0/math.pi
    elevation = direction[1][0]*180.0/math.pi
    npat = np.array(time)
    matches = np.where(npat>myTimes[0])[0]
    matches2 = np.where(npat<myTimes[-1])[0]
    azimuth = np.mean(azimuth[matches[0]:matches2[-1]+1])
    elevation = np.mean(elevation[matches[0]:matches2[-1]+1])
    if (type(scan) == list):
        listscan = ""
        listfield = []
        for sc in scan:
            listfield.append(vm.getFieldsForScan(sc))
            listscan += "%d" % sc
            if (sc != scan[-1]):
                listscan += ","
        listfields = np.unique(listfield)
        listfield = ""
        for field in listfields:
            listfield += "%s" % field
            if (field != listfields[-1]):
                listfield += ","
    else:
        listscan = str(scan)
        listfield = vm.getFieldsForScan(listscan)
    print "Scan %s (field=%s): azim = %.2f,  elev = %.2f  (degrees)" % (listscan, listfield, azimuth, elevation)
    return([azimuth, elevation])

def plotElevation(vis, antenna='0', xrange=None, yrange=None):
    """
    Plots the elevation vs. time for a specified antenna in an ms.
    The antenna parameter can be either the number, the number string,
    or the name string of the antenna.  xrange and yrange can be
    used to set the plot range.
    Produces a plot called  <vis>.elev.png.
    -- Todd Hunter
    """
    plotPosition(vis,1,antenna,xrange,yrange)

def plotAzimuth(vis, antenna='0',xrange=None,yrange=None):
    """
    Plots the azimuth vs. time for a specified antenna in an ms.
    The antenna parameter can be either the number, the number string,
    or the name string of the antenna.  xrange and yrange can be
    used to set the plot range.
    Produces a plot called <vis>.azim.png.
     -- Todd Hunter
    """
    plotPosition(vis,0,antenna,xrange,yrange)

def plotPosition(vis, azel, antenna='0', xrange=None, yrange=None):
    """
    Plots the azimuth or elevation vs. time for a specified antenna in an ms.
    azel=0 means azimuth,  1 means elevation
    This function is useful for showing the az/el of planetary bodies which
    might appear with RA,dec = 0,0 in plotms (in older ALMA data).
    Produces a plot called  <vis>.elev.png or <vis>.azim.png.
    -- Todd Hunter
    """
    clf()
    try:
      if str(antenna).isdigit():
        antennaName = getAntennaNames(vis)[int(str(antenna))]
      else:
        antennaName = antenna
        try:
          antenna = getAntennaIndex(vis,antennaName)
        except:
          antennaName = string.upper(antenna)
          antenna = getAntennaIndex(vis,antennaName)
    except:
      print "Either the ANTENNA file or antenna %s does not exist" % (antennaName)
      return
    tb.open("%s/POINTING" % vis)
    subtable = tb.query("ANTENNA_ID == %s" % antenna)
    tb.close()        
    direction = subtable.getcol("DIRECTION")
    elevation = direction[azel][0]*180.0/math.pi
    mjdsec = (subtable.getcol("TIME"))
    mjd  = mjdsec/86400.
    elevTime = (mjd-math.floor(mjd[0]))*24.
    plot(elevTime,elevation,'r.')
    if (azel == 1):
        ylabel('Elevation (deg)')
        ylim([0,90])
    else:
        ylabel('Azimuth (deg)')
    if (yrange != None):
        ylim(yrange)
    if (xrange != None):
        xlim(xrange)
    (mjd, datestring) = mjdSecondsToMJDandUT(mjdsec[0])
    xlabel('UT hour on %s'%(datestring[0:10]))
    title('%s'%(antennaName))
    if (azel==1):
        savefig('%s.elev.png'%(vis))
    else:
        savefig('%s.azim.png'%(vis))

def ComputeLSTDay(mjdsec=-1, date='', longitude=-67.7549):
    """
    Computes the LST day (useful for running NRAO dopset)
    and LST (in hours) for a specified time/date and longitude.
    Default longitude is for ALMA.
    The input longitude is in degrees, where east of Greenwich is positive.
    If mjdsec is a string, it will be assumed to be a date string.
    For date, either of these formats is valid: 2011/10/15
                                                2011/10/15 05:00:00
                                                2011/10/15-05:00:00
    -- Todd Hunter
    """
    if (date == ''):
        if (type(mjdsec) == str):
            mjdsec = dateStringToMJDSec(mjdsec)
        else:
            if (mjdsec<0):
                mjdsec = me.epoch('mjd','today')['m0']['value'] * 86400
                print "Using current date/time"
    else:
        mjdsec = dateStringToMJDSec(date)
    LST = ComputeLST(mjdsec, longitude)
    MJD = mjdsec/86400.
    JD = MJD + 2400000.5

# See http://gge.unb.ca/Pubs/TR171.pdf
    siderealDay = int(np.floor(JD*(0.002737909350795) + MJD) )

    print "LST day = %d,  LST = %.4f hours (at longitude=%f)" % (siderealDay,
                                                                 LST,longitude)
    print "Julian day = %f, MJD = %f, MJD seconds = %f" % (JD,MJD,mjdsec)
    return(siderealDay)
      
def ComputeLST(mjdsec, longitude):
  """
  Computes the LST (in hours) for a specified time and longitude. 
  The input longitude is in degrees, where east of Greenwich is positive.
  -- Todd Hunter
  """
  JD = mjdsec/86400. + 2400000.5
  T = (JD - 2451545.0) / 36525.0
  sidereal = 280.46061837 + 360.98564736629*(JD - 2451545.0) + 0.000387933*T*T - T*T*T/38710000.
# now we have LST in Greenwich, need to scale back to site
  sidereal += longitude
  sidereal /= 360.
  sidereal -= np.floor(sidereal)
  sidereal *= 24.0
  if (sidereal < 0):
      sidereal += 24
  if (sidereal >= 24):
      sidereal -= 24
  return(sidereal)

def angularSeparationOfPlanets(planet1='Saturn', planet2='Titan',date='',
                               observatory=JPL_HORIZONS_ID['ALMA'], useJPL=True, target=None):
    """
    Computes the current angular separation of two planets at 0 UT on the current date,
    or the exact specified date string.  Default viewing location is ALMA.
    If 'target' is specified as a valid RA/Dec, then the separation between it and planet1
    will be computed. Required format for target:  HH[:MM:SS.S]  [+]DD[:MM:SS.S]
      date: one possible format of the date string is: '2011-10-31 11:59:59'
            or simply '2011-10-31' for 0:00 UT. A list of allowed formats for date
            is at:   http://ssd.jpl.nasa.gov/?horizons_doc#time
    Todd Hunter
    """
    data1 = planet(planet1, date=date, observatory=observatory, useJPL=useJPL)
    if (target == None):
        data2 = planet(planet2, date=date, observatory=observatory, useJPL=useJPL)
    else:
        results = target.split()
        if (len(results) != 2):
            print "Required format for target:  HH[:MM:SS.S]  [+]DD[:MM:SS.S]"
            return
        ra,dec = results
        if (ra.find(':') > 0):
            ratokens = ra.split(':')
            ra = float(ratokens[0])
            if (ratokens > 1):
                ra += float(ratokens[1])/60.
            if (ratokens > 2):
                ra += float(ratokens[2])/3600.
        else:
            print "Required format for target:  HH[:MM:SS.S]  [+]DD[:MM:SS.S]"
            return
        
        if (dec.find(':') > 0):
            dectokens = dec.split(':')
            dec = abs(float(dectokens[0]))
            if (dectokens > 1):
                dec += float(dectokens[1])/60.
            if (dectokens > 2):
                dec += float(dectokens[2])/3600.
            if (dectokens[0].find('-') >= 0):
                dec = -dec
        else:
            print "Required format for target:  HH[:MM:SS.S]  [+]DD[:MM:SS.S]"
            return
        data2 = {'directionRadians': [float(ra)*pi/12.0, float(dec)*pi/180]}
    print data1['directionRadians'][0], data1['directionRadians'][1], data2['directionRadians'][0], data2['directionRadians'][1]
    rad = angularSeparationRadians(data1['directionRadians'][0], data1['directionRadians'][1], 
                                   data2['directionRadians'][0], data2['directionRadians'][1])
    print "Separation = %g rad = %g deg = %g arcsec" % (rad, rad*180/math.pi, rad*3600*180/math.pi)
    print "RA separation = %g hours" % (12*(data1['directionRadians'][0] - data2['directionRadians'][0])/pi)
    
def angularSeparationOfDirections(dir1,dir2):
    """
    Accepts two direction dictionaries and returns the separation in radians
    --Todd Hunter
    """
    print dir1, dir2
    rad = angularSeparationRadians(dir1['m0']['value'], dir1['m1']['value'], dir2['m0']['value'], dir2['m1']['value'])
    return(rad)

def angularSeparationOfDirectionsArcsec(dir1,dir2):
    """
    Accepts two direction dictionaries and returns the separation in arcsec.
    Todd Hunter
    """
    arcsec = angularSeparationOfDirections(dir1, dir2) * 180*3600 / np.pi
    return(arcsec)

def angularSeparationRadians(ra0,dec0,ra1,dec1):
  """
  Computes the great circle angle between two celestial coordinates.
  using the Vincenty formula (from wikipedia) which is correct for all
  angles, as long as you use atan2() to handle a zero denominator.  
     See  http://en.wikipedia.org/wiki/Great_circle_distance
  Input and output are in radians.  It also works for the az,el coordinate system.
  See also angularSeparation()
  -- Todd Hunter
  """
  return(angularSeparation(ra0*180/math.pi, dec0*180/math.pi, ra1*180/math.pi, dec1*180/math.pi)*math.pi/180.)

def angularSeparation(ra0,dec0,ra1,dec1):
  """
  Computes the great circle angle between two celestial coordinates.
  using the Vincenty formula (from wikipedia) which is correct for all
  angles, as long as you use atan2() to handle a zero denominator.  
     See  http://en.wikipedia.org/wiki/Great_circle_distance
  ra,dec must be given in degrees, as is the output.
  It also works for the az,el coordinate system.
  See also angularSeparationRadians()
  -- Todd Hunter
  """
  ra0 *= math.pi/180.
  dec0 *= math.pi/180.
  ra1 *= math.pi/180.
  dec1 *= math.pi/180.
  deltaLong = ra0-ra1
  argument1 = (((math.cos(dec1)*math.sin(deltaLong))**2) +
               ((math.cos(dec0)*math.sin(dec1)-math.sin(dec0)*math.cos(dec1)*math.cos(deltaLong))**2))**0.5
  argument2 = math.sin(dec0)*math.sin(dec1) + math.cos(dec0)*math.cos(dec1)*math.cos(deltaLong)
  angle = math.atan2(argument1, argument2) / (math.pi/180.)
  return(angle)

def sun(observatory='' ,mjdsec='', mjd='', help=False):
    """
    Determines the az/el of the Sun for the specified observatory and specified
    time in MJD seconds (or MJD).  Defaults are ALMA and 'now'.
    Other observatories available:
      ARECIBO  ATCA  BIMA  CLRO  DRAO  DWL  GB  GBT  GMRT  IRAM PDB  IRAM_PDB
      JCMT  MOPRA  MOST  NRAO12M  NRAO_GBT  PKS  SAO SMA  SMA  VLA  VLBA  WSRT
      ATF  ATA  CARMA  ACA  OSF  OVRO_MMA  EVLA  ASKAP  APEX  SMT  NRO  ASTE
      LOFAR  MeerKAT  KAT-7  EVN  LWA1  PAPER_SA  PAPER_GB  e-MERLIN  MERLIN2

    For further help and examples, run sun(help=True)
    or see https://safe.nrao.edu/wiki/bin/view/ALMA/Sun
    -- Todd Hunter
    """
    if (help):
        print "Usage: sun(observatory='ALMA', mjdsec='', mjd='')"
        print "  The observatory defaults to ALMA, and mjdsec to now. Other observatories available:"
        listAvailableObservatories()
        return
    if (mjdsec=='' and mjd==''):
        mjdsec = me.epoch('mjd','today')['m0']['value'] * 86400
    elif (mjdsec==''):
        mjdsec = double(mjd)*86400
    else:
        mjdsec = double(mjdsec)

    mjd = mjdsec/86400.
    if (observatory==''):
        observatory = 'ALMA'
    try:
        [latitude,longitude,observatory] = getObservatoryLatLong(observatory)
        print "Found location for %s: %f, %f degrees" % (observatory,longitude,latitude)
        print "MJD = %f = %f seconds" % (mjd, mjdsec)
    except:
        print "Did not find this observatory=%s, using ALMA instead." % (observatory)
        
    (az,el) = ComputeSolarAzElLatLong(mjdsec,latitude,longitude)
    print "At %s, the Sun is currently at azim=%.3f, elev=%.3f" % (observatory,az,el)

def listAvailableObservatories():
        repotable=os.getenv("CASAPATH").split()[0]+"/data/geodetic/Observatories"
        try:
            tb.open(repotable)
            Name = tb.getcol('Name')
            ns = ''
            for N in Name:
                ns += N + "  "
                if (len(ns) > 70):
                    print ns
                    ns = ''
            tb.close()
        except:
            print "Could not open table = %s" % (repotable)


def getObservatoryLatLong(observatory='',help=False):
     """
     Opens the casa table of known observatories and returns the latitude and longitude
     in degrees for the specified observatory name string.
     -- Todd Hunter
     """
     if (help):
        print "Opens the casa table of known observatories and returns the latitude"
        print "and longitude in degrees for the specified observatory (default=ALMA)."
        print "Usage: getObservatoryLatLong(observatory='ALMA')"
        print "Usage: getObservatoryLatLong(observatory=-7)"
        listAvailableObservatories()
        return
     repotable=os.getenv("CASAPATH").split()[0]+"/data/geodetic/Observatories"
     try:
        tb.open(repotable)
     except:
        print "Could not open table = %s, returning ALMA coordinates instead" % (repotable)
        longitude = -67.7549  # ALMA
        latitude = -23.0229   # ALMA
        observatory = 'ALMA'
        return([latitude,longitude,observatory])
     if (type(observatory) == 'int' or observatory in JPL_HORIZONS_ID.values()):
         if (str(observatory) in JPL_HORIZONS_ID.values()):
             observatory = JPL_HORIZONS_ID.keys()[JPL_HORIZONS_ID.values().index(str(observatory))]
         else:
            print "Did not recognize observatory='%s', using ALMA instead." % (observatory)
            observatory = 'ALMA'
             
     Name = tb.getcol('Name')
     matches = np.where(np.array(Name)==observatory)
     if (len(matches) < 1 and str(observatory).find('500') < 0):
            print "Names = ", Name
            print "Did not find observatory='%s', using ALMA instead." % (observatory)
            for n in Name:
                if (n.find(observatory) >= 0):
                    print "Partial match: ", n
            observatory = 'ALMA'
            longitude = -67.7549  # ALMA
            latitude = -23.0229   # ALMA
     elif (str(observatory).find('500') >= 0 or
           str(observatory).lower().find('geocentric') >= 0):
         observatory = 'Geocentric'
         longitude = 0
         latitude = 0
     else:
         longitude = tb.getcol('Long')[matches[0]]
         latitude = tb.getcol('Lat')[matches[0]]
     tb.close()
         
     return([latitude,longitude,observatory])

def ComputeSolarAzElForObservatory(mjdsec, observatory):
  [latitude,longitude,obs] = getObservatoryLatLong(observatory) 
  return(ComputeSolarAzElLatLong(mjdsec,latitude,longitude))
    
def ComputeSolarAzEl(mjdsec):
  """
  Return the az and el of the Sun in degrees for the specified
  time.  Assumes ALMA. See also ComputeSolarAzElLatLong().
  Todd Hunter
  """
  [latitude,longitude,obs] = getObservatoryLatLong('ALMA') 
  return(ComputeSolarAzElLatLong(mjdsec,latitude,longitude))

def ComputeSolarAzElLatLong(mjdsec,latitude,longitude):
  """
  Computes the apparent Az,El of the Sun for a specified time and location
  on Earth.  Latitude and longitude must arrive in degrees, with positive
  longitude meaning east of Greenwich.
  -- Todd Hunter
  """
  DEG_TO_RAD = math.pi/180.
  RAD_TO_DEG = 180/math.pi
  HRS_TO_RAD = math.pi/12.
  [RA,Dec] = ComputeSolarRADec(mjdsec)
  LST = ComputeLST(mjdsec, longitude)

  phi = latitude*DEG_TO_RAD
  hourAngle = HRS_TO_RAD*(LST - RA)
  azimuth = RAD_TO_DEG*math.atan2(math.sin(hourAngle), (math.cos(hourAngle)*math.sin(phi) - math.tan(Dec*DEG_TO_RAD)*math.cos(phi)))

# the following is to convert from South=0 (which the French formula uses)
# to North=0, which is what the rest of the world uses */
  azimuth += 180.0;

  if (azimuth > 360.0):
    azimuth -= 360.0
  if (azimuth < 0.0):
    azimuth += 360.0

  argument = math.sin(phi)*math.sin(Dec*DEG_TO_RAD) + math.cos(phi)*math.cos(Dec*DEG_TO_RAD) * math.cos(hourAngle);
  elevation = RAD_TO_DEG*math.asin(argument);

  return([azimuth,elevation])
    
def ComputeSolarRADec(mjdsec):
  """
  Computes the RA,Dec of the Sun for a specified time. -- Todd Hunter
  """   
  jd = mjdsec/86400. + 2400000.5
  RAD_TO_DEG = 180/math.pi
  RAD_TO_HRS = (1.0/0.2617993877991509)
  DEG_TO_RAD = math.pi/180.
  T = (jd - 2451545.0) / 36525.0
  Lo = 280.46646 + 36000.76983*T + 0.0003032*T*T
  M = 357.52911 + 35999.05029*T - 0.0001537*T*T
  Mrad = M * DEG_TO_RAD
  e = 0.016708634 - 0.000042037*T - 0.0000001267*T*T
  C = (1.914602 - 0.004817*T - 0.000014*T*T) * math.sin(Mrad) +  (0.019993 - 0.000101*T) * math.sin(2*Mrad) + 0.000289*math.sin(3*Mrad)
  L = Lo + C
  nu = DEG_TO_RAD*(M + C)
  R = 1.000001018 * (1-e*e) / (1 + e*math.cos(nu))
  Omega = DEG_TO_RAD*(125.04 - 1934.136*T)
  mylambda = DEG_TO_RAD*(L - 0.00569 - 0.00478 * math.sin(Omega))  
  epsilon0 = (84381.448 - 46.8150*T - 0.00059*T*T + 0.001813*T*T*T) / 3600.
  epsilon = (epsilon0 + 0.00256 * math.cos(Omega)) * DEG_TO_RAD
  rightAscension = RAD_TO_HRS*math.atan2(math.cos(epsilon)*math.sin(mylambda), math.cos(mylambda))
  if (rightAscension < 0):
      rightAscension += 24.0
  argument = math.sin(epsilon) * math.sin(mylambda)
  declination = RAD_TO_DEG*math.asin(argument)
  return([rightAscension, declination])
    
def parseMonitorPowerLevels(filename,antenna,polarization=0,plot_type='sb_power') :
    if ((plot_type <> 'sb_power') and (plot_type <> 'bb_power') and (plot_type <> 'atten')) :
        return "You must select plot_type sb_power, bb_power or atten"
    if ((polarization <> 0) and (polarization <> 1)) :
        return 'Polarization needs to be 0 or 1.'
    polarization = str(polarization)
    table = fiop.fileToTable(filename)
    [rest,rows]  = fiop.getRestrictTable(table,1,antenna)
    [rest2,rows] = fiop.getRestrictTable(rest,2,polarization)
    rest2 = fiop.getInvertTable(rest2)
    times = rest2[0]
    times_ = convertTimeStamps(times)
    for i in range(len(times_[:-2000])) : print times_[i],i
    clf()
    if plot_type == 'sb_power' :
        plot(times_,rest2[3])
        plot(times_,rest2[4])
    elif plot_type == 'bb_power' :
        plot(times_,rest2[5])
        plot(times_,rest2[6])
        plot(times_,rest2[7])
        plot(times_,rest2[8])        
    elif plot_type == 'atten' :
        plot(times_,rest2[9],'.')
        plot(times_,rest2[10],'.')
        plot(times_,rest2[11],'.')
        plot(times_,rest2[12],'.')
        plot(times_,rest2[13],'.')
        plot(times_,rest2[14],'.')
    show()
        
class FixPosition:
  def __init__(self,inputMs=''):
    """
    Instantiation of this class calls this, i.e.,
        fp = aU.FixPosition('myInputMS.ms').  
    The dataset name is the only allowed input.
    For further help, see https://safe.nrao.edu/wiki/bin/view/ALMA/FixPosition
    Todd Hunter (May 2011)
    """
    self.inputMs = inputMs

  def getDirectionFromProperMotion(self,field,years=0):
      """
      Find the first occurrence of the source name or source ID, 
      and then use its position and proper motion to compute a
      new direction, and return it.
      Letting years=0 means to start at J2000 and come forward.
      -Todd Hunter (May 2011)
      """
      from math import pi
      tb.open('%s/SOURCE'%(self.inputMs),nomodify=True)
      if (isinstance(field,str)):
          source = tb.getcol('NAME')
      else:
          source = tb.getcol('SOURCE_ID')
      direction = tb.getcol('DIRECTION')
      properMotion = tb.getcol('PROPER_MOTION')
      tb.close()
#      print "len(direction[0]) = %d" % (len(direction[0]))
      if (years == 0):
          tb.open('%s/FIELD'%(self.inputMs),nomodify=True)
          if (isinstance(field,str)):
             mySource = tb.getcol('NAME')
          else:
             mySource = tb.getcol('SOURCE_ID')
          for i in range(len(mySource)):
              if (mySource[i] == field):
                  timestamp = tb.getcell('TIME',i)
                  j2000 = 4453401600
                  years = (timestamp - j2000)/31556925.0
                  seconds = (timestamp - j2000)
                  print "Applying %f years (%.1f seconds) of proper motion since J2000." % (years,seconds)
                  break
          tb.close()
      else:
          seconds = years*31556925.
      foundSource = 0
      for i in range(len(direction[0])):
          if (source[i] == field):
              new_dec = direction[1][i] + properMotion[1][i]*seconds
              new_ra = direction[0][i] + properMotion[0][i]*seconds/math.cos(new_dec)
              new_direction = np.array([[new_ra], [new_dec]])
              hr = (new_ra*180/pi)/15.
              h = int(floor(hr))
              m = int((hr-h)*60)
              s = 3600*(hr-h-m/60.)
              if (new_dec < 0):
                  mysign = '-'
              else:
                  mysign = '+'
              dec = abs(new_dec*180/pi)
              d = int(floor(dec))
              dm = int(60*(dec-d))
              ds = 3600*(dec-d-dm/60.)
              print "New direction = %.12f, %.12f = %02d:%02d:%07.4f, %c%02d:%02d:%07.4f" % (new_ra, new_dec, h,m,s,mysign,d,dm,ds)
              foundSource = 1
              break
      if (foundSource == 0):
          print "Did not find source = ", field
          return(0)
      else:
          return(new_direction)
      
  def setDirectionFromRADec(self,field,raString,decString):
      """
      Sets the direction of a field from a specified ra and dec
      string. The strings
      should be in the form:  hh:mm:ss.sssss   +dd:mm:ss.ssss
      Todd Hunter
      """
      from math import pi
      (hour,minute,second) = raString.split(':')
      (deg,dmin,dsec) = decString.split(':')
      rahour = (float(hour)+float(minute)/60.+float(second)/3600.) 
      ra = 15*rahour*pi/180.
      if (deg.find('-')>=0):
          sign = -1
      else:
          sign = +1
      deg = abs(float(deg))
      dec = sign*(deg+float(dmin)/60.+float(dsec)/3600.)*pi/180
      new_direction = np.array([[ra],[dec]])
      # now double check it by showing it to the user
      hr = (ra*180/pi)/15.
      h = int(floor(hr))
      m = int(floor((hr-h)*60))
      s = 3600*(hr-h-m/60.)
      if (dec < 0):
          mysign = -1
          mychar = '-'
      else:
          mysign = +1
          mychar = '+'
      decdeg = abs(dec*180/pi)
      d = int(floor(decdeg))
      dm = int(floor(60*(decdeg-d)))
      ds = 3600*(decdeg-d-dm/60.)
      print "New direction: %.11f,%.11fdeg = %02d:%02d:%06.3f,%c%02d:%02d:%07.4f" % (hr, mysign*decdeg, h,m,s,mychar,d,dm,ds)
      print "             = %.13f,%.13f radian" % (ra,dec)
      self.setDirectionFromValue(field,new_direction)
      return
      
  def setDirectionFromProperMotion(self,field,years=0):
      """
      Will set the PHASE, DELAY and REFERENCE directions of the field to the modified
      position as computed from the proper motion in the SOURCE table.
      Todd Hunter
      """
      new_direction = self.getDirectionFromProperMotion(field,years)
      if (isinstance(new_direction,int)):
          print "Did not find source"
          return
      self.setDirectionFromValue(field,new_direction)
      return
      
  def setDirectionFromValue(self,field,new_direction):
      tb.open('%s/FIELD'%(self.inputMs),nomodify=False)
      sourceName = tb.getcol('NAME')
      if (isinstance(field,str)):
          source = tb.getcol('NAME')
      else:
          source = tb.getcol('SOURCE_ID')
      changed = 0
      for j in range(len(source)):
          if (source[j] == field):
              direction = tb.getcell('PHASE_DIR',j)   
#              print "len(direction), direction = ", len(direction), direction
#              print "len(new_direction), new_direction = ", len(new_direction), new_direction
              tb.putcell('PHASE_DIR',j,new_direction)   
              tb.putcell('DELAY_DIR',j,new_direction)   
              tb.putcell('REFERENCE_DIR',j,new_direction)   
              changed += 1
      tb.close()
      print "Changed %d positions in FIELD table." % (changed)
      return

  def getRADecJ2000FromAntennas(self,field,timestamp=0):
#      loop over antennas,  this needs improvement
    ant = range(0,9)
    self.doRADecJ2000FromAntenna(field,0,ant,timestamp=timestamp)

  def doRADecJ2000FromAntenna(self, field, observatory=JPL_HORIZONS_ID['ALMA'], set=0, ant=0, timestamp=0):
    """
    Computes the mean RA/Dec in J2000 coordinates for the specified 
    field string by using the azim/elev pointing of the specified 
    antenna and stuffs this into the FIELD tables. This is the 
    equivalent of something Dirk wrote independently and should be 
    superceded by the method: getRADecJ2000FromJPL().

    ant = antenna number index (0..n)
    timestamp = 0 means compute the mean position
      (otherwise, find the antenna position at the nearest time to this)
    Todd Hunter
    """ 
    vm = ValueMapping(self.inputMs)
    try:
        times = vm.getTimesForField(field)
    except:
        print "did not find %s in the ms" %(field)
        return
    tb.open('%s/POINTING'%(self.inputMs))
    tim=tb.getcol('TIME')
    dir=tb.getcol('DIRECTION')
    antenna=tb.getcol('ANTENNA_ID')
    tb.close()
    tb.open('%s/ANTENNA'%(self.inputMs))
    antpos=tb.getcol('POSITION')
    tb.close()

    if (isinstance(ant,int)):
        ants = [ant]
    else:
        ants = ant
    antCounter = -1
    for ant in ants:
      iant = ant
      me.doframe(me.position('ITRF','%.8fm'%(antpos[0][iant]), '%.8fm'%(antpos[1][iant]), '%.8fm'%(antpos[2][iant])))
      raRadian = []
      decRadian = []
      for k in range(len(tim)):
        if ((tim[k] > times[0]) and (tim[k] < times[-1])):
          if (antenna[k] == ant):
            me.doframe(me.epoch('UTC', qa.quantity(tim[k],'s')))
            dirJ2000=me.measure(me.direction('AZELGEO',
                                             qa.quantity(dir[0,0,k], 'rad'), 
                                             qa.quantity(dir[1,0,k],'rad')),
                               'J2000')
            raRadian.append(dirJ2000['m0']['value'])
            decRadian.append(dirJ2000['m1']['value'])
      if (timestamp == 0):
        meanRA = mean(raRadian)
        meanDEC = mean(decRadian)
      else:
        index = self.find_index_of_nearest(times, timestamp)
        if (ant == ants[0]):
          print "The nearest time to %.3f is %.3f, a difference of %.3f seconds" % (timestamp,times[index],timestamp-times[index])
        meanRA = raRadian[index]
        meanDEC = decRadian[index]
      if (meanRA < 0):
        meanRA = meanRA+2*math.pi
      hours = meanRA*12/math.pi
      deg = meanDEC*180/math.pi
      absdeg = abs(deg)
      if (len(ants) < 2):
        antstring = ''
        if (timestamp == 0):
          print 'mean RA, Dec:'
        else:
          print 'RA, Dec at closest time:'
      else:
        antstring = '%d'%(ant)

      xyz = "antpos = %.4fm, %.4fm, %.4fm" % (antpos[0][ant],antpos[1][ant],antpos[2][ant])
      print 'Antenna %s  %s  %02d:%02d:%07.4f, %c%02d:%02d:%06.3f  %s' % (antstring,
        vm.getAntennaNamesForAntennaId(ant),
        int(hours), 
        int(60*(hours-int(hours))), 3600*(hours-int(hours)-int(60*(hours-int(hours)))/60.),
        ('%+f'%(deg))[0], int(absdeg), int(60*(absdeg-int(absdeg))), 3600*(absdeg-int(absdeg)-int(60*(absdeg-int(absdeg)))/60.), 
        xyz)
    tb.open('%s/FIELD'%(self.inputMs),nomodify=False)
    name = tb.getcol('NAME')
    j = 0
    for i in range(len(name)):
        if (name[i] == field):
            reference_dir = tb.getcell('REFERENCE_DIR',j)   
            reference_dir = np.array([[meanRA],[meanDEC]])
            if (set):
                tb.putcell('REFERENCE_DIR',j,reference_dir)   
                tb.putcell('PHASE_DIR',j,reference_dir)   
                tb.putcell('DELAY_DIR',j,reference_dir)   
                print "Values updated"
#            else:
#                print "Values NOT updated"
        j = j+1
    tb.close()

  def setRADecJ2000FromAntenna(self,field,ant=0,timestamp=0):
      self.doRADecJ2000FromAntenna(field,set=1,ant=ant,timestamp=timestamp)

  def getRADecJ2000FromAntenna(self,field,ant=0,timestamp=0):
      self.doRADecJ2000FromAntenna(field,set=0,ant=ant,timestamp=timestamp)

  def find_nearest(self,array,value):
    idx = (np.abs(array-value)).argmin()
    return array[idx]

  def find_index_of_nearest(self,array,value):
    idx = (np.abs(array-value)).argmin()
    return idx

  def doRADecJ2000FromJPL(self, field, observatory=JPL_HORIZONS_ID['ALMA'], set=0, verbose=0):
    """
    Contacts JPL Horizons via the telnet interface, gets the position and derivative
    for the mean time of observation, and stuffs them into the FIELD table of the ms.
    At present, this function can only accept a single field, but could be modified to
    do more than one at once.  The apparent coordinates are for ALMA.
    It returns the polynomial for use in subsequent .ms.
    Todd Hunter
    """
    nPolyTerms = 2
    vm = ValueMapping(self.inputMs)
    try:
        times = vm.getTimesForField(field)
    except:
        print "did not find %s in the ms" %(field)
        return
    meanTime = mean(times)
    print "Will query JPL Horizons for position & derivative at mean time = %.1f " % (meanTime)
    s = mjdSecondsToMJDandUT(meanTime)
    print " = %.5f = %s" % (s[0],s[1])
    tb.open('%s/FIELD'%(self.inputMs),nomodify=False)
    sourceName = tb.getcol('NAME')
    if (isinstance(field,str)):
        source = tb.getcol('NAME')
    else:
        source = tb.getcol('SOURCE_ID')
    j = 0
    for i in range(len(source)):
        if (source[i] == field):
            [directionRadians,rateRadiansPerSecond,angularDiameter] = self.contactJPLHorizons(sourceName[i], meanTime, observatory, verbose)
            reference_dir = tb.getcell('REFERENCE_DIR',j)   
            print "original PHASE_DIR = ", reference_dir
# definitely works:
            reference_dir = np.array([[directionRadians[0], rateRadiansPerSecond[0]],
                                      [directionRadians[1], rateRadiansPerSecond[1]]])
# this does not
#            reference_dir = np.array([directionRadians[0], directionRadians[1], 
#                                      rateRadiansPerSecond[0], rateRadiansPerSecond[1]])

            print "new PHASE_DIR = ", reference_dir
            if (set):
                tb.putcell('REFERENCE_DIR',j,reference_dir)   
                tb.putcell('PHASE_DIR',j,reference_dir)   
                tb.putcell('DELAY_DIR',j,reference_dir)   
                tb.putcell('TIME',j,meanTime)
                tb.putcell('NUM_POLY',j,nPolyTerms)   
                print "Values updated"
            else:
                print "Values NOT updated"
        j = j+1
    if (set):
        tb.putcolkeyword('PHASE_DIR','MEASINFO', {'Ref':'J2000', 'type':'direction'})
        tb.putcolkeyword('DELAY_DIR','MEASINFO', {'Ref':'J2000', 'type':'direction'})
        tb.putcolkeyword('REFERENCE_DIR','MEASINFO', {'Ref':'J2000', 'type':'direction'})
    tb.close()
    return([reference_dir,meanTime])

  def setRADecJ2000FromPolynomial(self,field,polynomial,timestamp,set=True):
    """
    Sets the RA and Dec for the specified field using the specified
    polynomial and timestamp.
    Todd Hunter
    """
    nPolyTerms = len(polynomial) - 1 # Bryan Butler told me to add the "- 1" on March, 12 2012. - TRH
    tb.open('%s/FIELD'%(self.inputMs),nomodify=False)
    sourceName = tb.getcol('NAME')
    if (isinstance(field,str)):
        source = tb.getcol('NAME')
    else:
        source = tb.getcol('SOURCE_ID')
    j = 0
    for i in range(len(source)):
        if (source[i] == field):
            reference_dir = polynomial
            print "new PHASE_DIR = ", reference_dir
            if (set):
                tb.putcell('REFERENCE_DIR',j,reference_dir)   
                tb.putcell('PHASE_DIR',j,reference_dir)   
                tb.putcell('DELAY_DIR',j,reference_dir)   
                tb.putcell('TIME',j,timestamp)
                tb.putcell('NUM_POLY',j,nPolyTerms)   
                print "Values updated"
            else:
                print "Values NOT updated"
        j = j+1
    if (set):
        tb.putcolkeyword('PHASE_DIR','MEASINFO', {'Ref':'J2000', 'type':'direction'})
        tb.putcolkeyword('DELAY_DIR','MEASINFO', {'Ref':'J2000', 'type':'direction'})
        tb.putcolkeyword('REFERENCE_DIR','MEASINFO', {'Ref':'J2000', 'type':'direction'})
    tb.close()
    return

  def setRADecJ2000FromJPL(self,field,observatory=JPL_HORIZONS_ID['ALMA']):
    """
    Contacts JPL Horizons via the telnet interface, gets the position
    of a planetary body along with its derivative for the mean time of
    observation, and stuffs them into the FIELD table of the ms.
    Todd Hunter
    """
    polynomialData = self.doRADecJ2000FromJPL(field, observatory, set=1)
    return(polynomialData)

  def getRADecJ2000FromJPL(self,field, observatory=JPL_HORIZONS_ID['ALMA'], verbose=0):
    """
    Contacts JPL Horizons via the telnet interface, gets the position
    of a planetary body along with its derivative for the mean time of
    observation and returns it.
    Todd
    """
    polynomialData = self.doRADecJ2000FromJPL(field, observatory, set=0, verbose=verbose)
    return(polynomialData)

  def getRaDecSize(self, body, datestring, observatory=JPL_HORIZONS_ID['ALMA'], 
                   verbose=False, apparent=False):
    if (len(datestring.split()) < 2):
        # append a UT time if not given
        datestring += ' 00:00'
        print "Assuming 0 hours UT"
    mjdsec = dateStringToMJDSec(datestring)
    (directionRadians, rateRadiansPerSecond,angularDiameter,rangeRate,rangeAU) = self.contactJPLHorizons(body, mjdsec, observatory, verbose, apparent)
    data = {}
    data['directionRadians'] = directionRadians
    data['rateRadiansPerSecond'] = rateRadiansPerSecond
    data['angularDiameter'] = angularDiameter
    data['rangeRateKms'] = rangeRate
    data['rangeAU'] = rangeAU
    return(data)      
  
  def contactJPLHorizons(self, body, mjdsec, observatory=JPL_HORIZONS_ID['ALMA'], verbose=False, apparent=False):
    """
    example interactive session:
    telnet://horizons.jpl.nasa.gov:6775
    606 # = Titan
    e  # for ephemeris
    o  # for observables
    -7 # for ALMA
    y  # confirm
    2011-Apr-23 00:00  #  UT
    2011-Apr-23 01:00  #  UT
    1h #  interval
    y  # default output
    1,3,13,20 # RA/DEC and rates (Rarcsec/hour), angular diameter, and range+rate
    space  # to get to next prompt
    q   # quit
    """

    OBSERVATORY_ID = observatory
    tstart = mjdSecondsToMJDandUT(mjdsec)[1][0:-3]
    if (verbose):
        print "tstart = ", tstart
    tstop = mjdSecondsToMJDandUT(mjdsec+3600)[1][0:-3]
    timeout = 4  #seconds
    t = telnetlib.Telnet('horizons.jpl.nasa.gov',6775)
    t.set_option_negotiation_callback(self.optcallback)
    data = t.read_until('Horizons> ')
    if (verbose):
        print "data = ", data
        print "hex string = %s\n\n" % binascii.hexlify(data)
    while (data.find('Horizons>') < 0):
        t.write('\n')
        data = t.read_until('Horizons> ')
        if (verbose):
            print "data = ", data
    t.write(body+'\n')
    data = t.read_until('Select ... [E]phemeris, [F]tp, [M]ail, [R]edisplay, ?, <cr>: ',timeout)
    if (verbose):
        print "data = ", data
    if (data.find('phemeris') < 0):
      if (data.find('EXACT')>=0):
        t.write('\n')
        data = t.read_until('Select ... [E]phemeris, [F]tp, [M]ail, [R]edisplay, ?, <cr>: ', timeout)
        if (verbose):
            print data
        useID = ''
      else:
        # then we have a conflict in the name. 
        # e.g. Titan vs. Titania, or Mars vs. Mars Barycenter
        # Try to resolve by forcing an exact match.
        lines = data.split('\n')
        if (verbose):
            print "Multiple entries found, using exact match"
            print "nlines = %d" % (len(lines))
        firstline = -1
        lastvalidline = -1
        l = 0
        useID = -1
        for line in lines:
            if (verbose):
                print line
            if (line.find('-----') >= 0):
                if (firstline == -1):
                    firstline = l+1
            else:
              tokens = line.split()
              if (firstline>=0 and lastvalidline == -1):
                if (len(tokens) < 2):
                  lastvalidline = l-1
                elif (tokens[1] == body and len(tokens) < 3):
                  # The <3 is necessary to filter out entries for a planet's barycenter
                  useID = int(tokens[0])
                  useBody = tokens[1]
                  if (verbose):
                      print "Use instead the id = %s = %d" % (tokens[0],useID)
            l = l+1
        if (useID == -1):
          # Try again with only the first letter capitalized, Probably not necessary
          body = string.upper(body[0]) + string.lower(body[1:])
#          print "Try the exact match search again with body = ", body
          firstline = -1
          lastvalidline = -1
          l = 0
          for line in lines:
            if (verbose):
                print line
            if (line.find('-----') >= 0):
                if (firstline == -1):
                    firstline = l+1
            elif (firstline > 0):
              if (verbose):
                  print "Splitting this line = %s" % (line)
              tokens = line.split()
              if (verbose):
                  print "length=%d,  %d tokens found" % (len(line),len(tokens))
              if (firstline>=0 and lastvalidline == -1):
                if (len(tokens) < 2):
                  # this is the final (i.e. blank) line in the list
                  lastvalidline = l-1
                elif (tokens[1] == body):
#                  print "%s %s is equal to %s." % (tokens[0],tokens[1],body)
                  useID = int(tokens[0])
                  useBody = tokens[1]
                  if (len(tokens) < 3):
                    if (verbose):
                      print "Use instead the id = %s = %d" % (tokens[0],useID)
                  elif (len(tokens[2].split()) < 1):
                    if (verbose):
                      print "Use instead the id = ",tokens[0]
                else:
                    if (verbose):
                        print "%s %s is not equal to %s." % (tokens[0],tokens[1],body)
            l = l+1
        if (verbose):
            print "line with first possible source = ", firstline
            print "line with last possible source = ", lastvalidline
            print "first possible source = ", (lines[firstline].split())[1]
            print "last possible source = ", (lines[lastvalidline].split())[1]
            print "Writing ", useID
        t.write(str(useID)+'\n')
        data = t.read_until('Select ... [E]phemeris, [F]tp, [M]ail, [R]edisplay, ?, <cr>: ')
        if (verbose):
            print data
    else:
        useID = ''
    t.write('e\n')
    data = t.read_until('Observe, Elements, Vectors  [o,e,v,?] : ')
    if (verbose):
        print data
    t.write('o\n')
    data = t.read_until('Coordinate center [ <id>,coord,geo  ] : ')
    if (verbose):
        print data
    t.write('%s\n' % OBSERVATORY_ID)
    data = t.read_until('[ y/n ] --> ')
    pointer = data.find('----------------')
    ending = data[pointer:]
    lines = ending.split('\n')
    try:
        if (verbose):
            print "Parsing line = %s" % (lines)
        tokens = lines[1].split()
    except:
        print "Telescope code unrecognized by JPL."
        return([],[],[])
        
    if (verbose):
        print data
    obsname = ''
    for i in range(4,len(tokens)):
        obsname += tokens[i]
        if (i < len(tokens)+1): obsname += ' '
    # display LST date/time and Julian date/time
    [latitude,longitude,obs] = getObservatoryLatLong(OBSERVATORY_ID)
    ComputeLSTDay(mjdsec, longitude=longitude)
    print "Confirmed Observatory name = ", obsname
    if (useID != ''):
        print "Confirmed Target ID = %d = %s" % (useID, useBody)
    t.write('y\n')
    data = t.read_until('] : ',1)
    if (verbose):
        print data
    t.write(tstart+'\n')
    data = t.read_until('] : ',1)
    if (verbose):
        print data
    t.write(tstop+'\n')
    data = t.read_until(' ? ] : ',timeout)
    if (verbose):
        print data
    t.write('1h\n')
    data = t.read_until(', ?] : ',timeout)
    if (verbose):
        print data
    if (1==1):
        # Set the first '1' to '2' for apparent coordinates
        if (apparent):
            print "Querying for apparent coordinates......"
            t.write('n\n2,3,13,20,\nJ2000\n\n\n\nDEG\nYES\n\n\n\n\n\n\n\n\n\n')
        else:
            t.write('n\n1,3,13,20,\nJ2000\n\n\n\nDEG\nYES\n\n\n\n\n\n\n\n\n\n')
    else:
        t.write('y\n') # accept default output?
        data = t.read_until(', ?] : ') #,timeout)
        if (verbose):
            print data
        t.write('1,3\n')
    t.read_until('$$SOE',timeout)
    data = t.read_until('$$EOE',timeout)
    if (verbose):
        print data
    t.close()
    lines = data.split('\n')
    if (verbose):
        print "lines = ", lines
    if (len(lines[1].split()) == 9):
        (date, time, raDegrees, decDegrees, raRate, decRate, angularDiameter, rangeAU, rangeRate) = lines[1].split()
    else:
        (date, time, flag, raDegrees, decDegrees, raRate, decRate, angularDiameter, rangeAU, rangeRate) = lines[1].split()
    raRadian = float(raDegrees)*math.pi/180.0
    decRadian = float(decDegrees)*math.pi/180.0
    directionRadians = [raRadian,decRadian]
    directionDegrees = [float(raDegrees), float(decDegrees)]
    rate = [float(raRate), float(decRate)]
    rateRadiansPerSecond = [float(raRate)*math.pi/(180*3600.*3600.),float(decRate)*math.pi/(180*3600.*3600.)]
    rangeAU = float(rangeAU)
    rangeRate = float(rangeRate)
    print "Range rate = %+f km/sec" % (rangeRate)
    if (verbose):
        print "degrees: position = ", directionDegrees, "  rates (arcsec/hr) = ",rate
    if (apparent):
        coords = 'Apparent'
    else:
        coords = 'J2000'
    print '%s Position: %s, %s' % (coords,qa.formxxx('%.12fdeg'%directionDegrees[0],format='hms',prec=5),
                                   qa.formxxx('%.12fdeg'%directionDegrees[1],format='dms',prec=4).replace('.',':',2))
    pa = np.arctan2(rate[0],rate[1]) * 180/np.pi
    print "%s Rate: %+.4f, %+.4f arcsec/hour (position angle = %+.1fdeg)" % (coords, rate[0], rate[1], pa)    
    print '%s Rate: %+.6f, %+.6f arcsec/second  (position angle = %+.1fdeg)' % (coords, rate[0]/3600., rate[1]/3600., pa)
    if (verbose):
        print "radians: position = ", directionRadians, "  rates = ",rateRadiansPerSecond
    if (angularDiameter.find('n.a.') >=0):
        return(directionRadians, rateRadiansPerSecond, [], rangeRate, rangeAU)
    else:
        return(directionRadians, rateRadiansPerSecond, float(angularDiameter), rangeRate, rangeAU)

  # Reject all telnet requests.  Vanilla all the way.
  def optcallback(self, socket, command, option):
        cnum = ord(command)
        onum = ord(option)
        if cnum == telnetlib.WILL: # and onum == ECHO:
                socket.write(telnetlib.IAC + telnetlib.DONT + onum)
        if cnum == telnetlib.DO and onum == telnetlib.TTYPE:
                socket.write(telnetlib.IAC + telnetlib.WONT + telnetlib.TTYPE)

# It is often useful to look up the shortest or longest baseline.  Or see how long a
# specific baseline is.  The following functions compute this. -- Todd Hunter
def computeBaselineLength(pos1,pos2):
    length = ((pos1[0]-pos2[0])**2+(pos1[1]-pos2[1])**2+(pos1[2]-pos2[2])**2)**0.5
    return(length)

def getBaselineExtrema(msFile):
    """
    Will compute the shortest and longest baseline for the specified ms.
    Projection toward the sources in the ms is not accounted for.
    Todd Hunter
    """
    tb.open(msFile+'/ANTENNA')
    names = tb.getcol('NAME')
    positions = np.transpose(tb.getcol('POSITION'))
    maxlength = 0
    minlength = 1e20
    for i in range(len(positions)):
        for j in range(i+1,len(positions)):
            length = computeBaselineLength(positions[i],positions[j])
            if (length > maxlength):
                maxlength = length
                maxbaseline = '%s-%s' % (names[i],names[j])
            if (length < minlength):
                minlength = length
                minbaseline = '%s-%s' % (names[i],names[j])
    tb.close()
    print "Longest baseline = %.3f m = %s" % (maxlength,maxbaseline)
    print "Shortest baseline = %.3f m = %s" % (minlength,minbaseline)
#    return([maxlength,minlength,maxbaseline,minbaseline])
    return
   
def getBaselineLengths(msFile='', sort=True, help=False):
    """
    Determines the baseline lengths for the specified ms
    Returns a dictionary with the key being the baseline string and the
    value being the baseline length in meters.
    sort=True sorts by baseline length, sort=False sorts by antenna name
    For further help and examples, see http://casaguides.nrao.edu/index.php?title=GetBaselineLengths
    Todd Hunter
    """
    if (help):
        print "getBaselineLengths(msFile, sort=True)"
        print "  'sort=True' sorts by baseline length, 'sort=False' sorts by antenna name"
        return
    try:
        tb.open(msFile+'/ANTENNA')
    except:
        print "Could not open table = ", msFile+'/ANTENNA'
        return
    names = tb.getcol('NAME')
    positions = np.transpose(tb.getcol('POSITION'))
    tb.close()
    # Now figure out where it ranks in the array
    l = {}
    for i in range(len(names)):
      for j in range(i,len(names)):
          if (i != j):
              l['%s-%s'%(names[i],names[j])] = ((positions[i][0]-positions[j][0])**2 +
                               (positions[i][1]-positions[j][1])**2 +
                               (positions[i][2]-positions[j][2])**2)**0.5
    if (sort):
        sortedBaselines = sorted(l.items(), key=operator.itemgetter(1))
        return(sortedBaselines)
    else:
        return(l)

def getBaselineLength(msFile='', ant1='', ant2='', help=False, verbose=True) :
    """
    Computes the specified baseline's length in meters.  The antennas
    can be specified by name string or ID number.
    Also compute its percentile length for the configuration.
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/GetBaselineLength
    Todd Hunter (June 2011)
    """
    if (help):
        print "Usage example: getBaselineLength(ms, 'DV01', 'DV02')"
        return
    try:
        tb.open(msFile+'/ANTENNA')
    except:
        print "Could not open table = ", msFile+'/ANTENNA'
        return
    if (type(ant1) == str):
        if (len(ant1) < 1):
            print "You must specify two separate antenna names or IDs"
            return
    if (type(ant2) == str):
        if (len(ant2) < 1):
            print "You must specify two separate antenna names or IDs"
            return
    names = tb.getcol('NAME')
    positions = np.transpose(tb.getcol('POSITION'))
    tb.close()
    position = []
    j = 0
    for i in range(len(names)):
        if (isinstance(ant1,str)):
            if (names[i] == ant1):
                position.append(positions[i])
                ant1 = i
                j += 1
        else:
            if (i == ant1):
                position.append(positions[i])
                j += 1
        if (isinstance(ant2,str)):
            if (names[i] == ant2):
                position.append(positions[i])
                ant2 = i
                j += 1
        else:
            if (i == ant2):
                position.append(positions[i])
                j += 1
        if (j==2):
            break
    if (j<2):
        print "Did not find both antennas: %s, %s" % (ant1,ant2)
        print "Found: ", names
        length = 0
        return length
    else:
        length = ((position[0][0]-position[1][0])**2+
                  (position[0][1]-position[1][1])**2+
                  (position[0][2]-position[1][2])**2
                  )**0.5
        if (verbose):
            print "Length of %s-%s (%d-%d) is %.3f m." % (names[ant1],names[ant2],ant1,ant2,length)
    # Now figure out where it ranks in the array
    l = []
    for i in range(len(names)):
      for j in range(i,len(names)):
          if (i != j):
             l.append(((positions[i][0]-positions[j][0])**2 +  (positions[i][1]-positions[j][1])**2 +
                  (positions[i][2]-positions[j][2])**2)**0.5)
    sortedBaselines = np.sort(l)
    nBaselines = len(sortedBaselines)
    for i in range(nBaselines):
#        print sortedBaselines[i]
        if (length <= sortedBaselines[i]):
            if (verbose):
                print "Of %d baselines, this baseline length is percentile %.0f, where 100=longest." % (nBaselines,i*100.0/(nBaselines-1))
            break
    return length,names[ant1],names[ant2]

def freqif(vis, freq) :
    """
    A Castro Carrizo (August 2011)
    Computes the frequency values corresponding to the different freq planes, which is particularly
    useful for the investigation of spurious signals generated in the system
    Sky (GHz)
    IF1 (4-8) or (5-10) depending on band (GHz)
    IF2 (2-4) (GHz)
    IF3 (0-2) (GHz) + channel numbering 
    entries: vis = ms file - if empty, it would read some defaults from script;
                           possible improvement could be that flo1 and flo2 would be given as inputs
             freq = input freq, which could be in sky, if1 or if2 units (always GHz). If empty, it 
                    will give all the details about LOs and spectral configuration.
    If LO4s are introduced in future ASDM datasets to define the position of the spectral unit within
    the IF3 window, this is to be read and included in the script to properly calculate the channel
    numbering
    """
    viss="no"+vis+"ne" 
    freqq="no"+freq+"ne"
    out_of_range=True
    if freqq == 'none' and viss != 'none':  
        tb.open("%s/SPECTRAL_WINDOW" % vis)
        bb = tb.getcol("BBC_NO")           # num unidad
        freq = tb.getcol("REF_FREQUENCY")  # freq cada unidad
        numchan = tb.getcol("NUM_CHAN")    # num channels
        tbw = tb.getcol("TOTAL_BANDWIDTH") # total bandwidth
        tb.close()
        try:
            tb.open("%s/ASDM_RECEIVER" % vis)
        except:
            print "Could not open ASDM_RECEIVER table:"
            print " ' asdm2MS --asis='*' ' may be needed "
            return([])
        numLO = tb.getcol('numLO')  # num of used spectral units
        freqLO = []
        band = []
        spws = []
        names = []
        for i in range(len(numLO)):
            freqLO.append(tb.getcell('freqLO',i))
            band.append(tb.getcell('frequencyBand',i))
            spws.append(int((tb.getcell('spectralWindowId',i).split('_')[1])))
            names.append(tb.getcell('name',i))
        tb.close() 
        print "SPECTRAL_WINDOW ...."
        print "basebands", bb
        print "Sky freqs (ghz)? ", freq/1e9
        print "num chan", numchan
        print "total bw (mhz)", tbw/1e6
        print ""
        print "ASDM_RECEIVER ...."
        print "numLO ", numLO
        for i in range(len(numLO)):
            print "freqLO (ghz)", freqLO[i]/1e9
        print "band", band
        print "spws", spws
        print "names", names
        return([])
 
    paras=float(freq)

    if paras > 12.:
        skyfreq=True
    else:
        skyfreq=False
    
      
    if viss != 'none':              # read spect config from ms file
        tb.open("%s/SPECTRAL_WINDOW" % vis)
        bb = tb.getcol("BBC_NO")           # num unidad
        freq = tb.getcol("REF_FREQUENCY")  # freq cada unidad
        numchan = tb.getcol("NUM_CHAN")    # num channels
        tbw = tb.getcol("TOTAL_BANDWIDTH") # total bandwidth
        tb.close()

        try:
            tb.open("%s/ASDM_RECEIVER" % vis)
        except:
            print " "
            print "Could not open ASDM_RECEIVER table:"
            print " ' asdm2MS --asis='*' ' may be needed "
            return([])
        numLO = tb.getcol('numLO')  # num of used spectral units
        freqLO = []
        band = []
        spws = []
        names = []
        for i in range(len(numLO)):
            freqLO.append(tb.getcell('freqLO',i))
            band.append(tb.getcell('frequencyBand',i))
            spws.append(int((tb.getcell('spectralWindowId',i).split('_')[1])))
            names.append(tb.getcell('name',i))
        tb.close() 
        numbb=(len(numLO)-1)/2
        flo1=freqLO[0]/1e9
        flo2=pb.zeros(numbb)
        flo3=pb.zeros(numbb)
        nchan=pb.zeros(numbb,int)
        c=0
        for i in range(1,len(numLO),2):  #BB_1,3,5,7...
            flo2[c]=freqLO[i][1]/1e9
            flo3[c]=freqLO[i][2]/1e9
            nchan[c]=numchan[i]
            c=c+1

        if1min = 4000.0/1e3          
        kk=band[1]   # a unique band is observed -- TO BE ADAPTED 
        if kk[9]=='6':
            if1min = 5000.0/1e3
        if1max = 8000.0/1e3 
        if kk[9]=='1' or kk[9]=='2' or kk[9]=='9' or kk[9]=='10':
            if1max = 12000.0/1e3
        elif kk[9]=='6':
            if1max = 10000.0/1e3
    else:                          # no ms file to read spect config 
        numbb = 4  # for general use it's worth to adapt this part to be read as input parameters(freq,lo1,lo2s)
        # 
        freq = [1.8342e11, 8.71848362499E10, 8.71848362499E10, 8.73020237499E10, 8.73020237499E10, 9.71848362501E10, 9.71848362501E10, 9.73020237501E10,  9.73020237501E10]
        flo1 = 92.21228               #
        flo1 = 104.26885              #
        flo2 = [9.0274437501,8.9102562501,8.9725562501,9.0897437501] #
        flo2 = [9.02744375,9.052256250,8.97255625,8.94774375]        #
        flo3 = [4.0,4.0,4.0,4.0]      #          
        nchan= [3840,3840,3840,3840]  #
        if1min = 4000.0/1e3           #
        if1max = 8000.0/1e3           #
        tbw = [1.875*1e9,1.875*1e9,1.875*1e9,1.875*1e9,1.875*1e9,1.875*1e9,1.875*1e9,1.875*1e9,1.875*1e9,1.875*1e9]  #
        tbw = [1.171875E8,1.171875E8,1.171875E8,1.171875E8,1.171875E8,1.171875E8,1.171875E8,1.171875E8,1.171875E8]   #

        
    if2min = (2000.+2000.*1/16/2)/1e3     # fix: start 2ghz band
    if2max = (4000.-2000.*1/16/2)/1e3     # fix: end 2ghz band
    if2cen = 3000./1e3
    bandif2 = 2.*15/16

    if3min = (2000.*1/16/2)/1e3           # fix: start 2ghz band  !DG output
    if3max = (2000.-2000.*1/16/2)/1e3     # fix: end 2ghz band    !
    if3cen = 1000./1e3                    #                       !
    
    if2par=pb.zeros(numbb)                #  if2 units (2-4)
    if3par=pb.zeros(numbb)                #  if3 units (0-2)
    # if3par_sbb=pb.zeros(numbb)          #  subbaseband filter number in if3 (?)
    chanpar = pb.zeros(numbb)             #  channel number

    
    for i in range(0,numbb):
        chanpar[i] = -1

    if skyfreq:
        parif1 = [False,False]              # lsb,usb 
        parif2 = [False,False,False,False,False,False,False,False] # should be limited to nunbb
        parif3 = [False,False,False,False,False,False,False,False] # should be limited to nunbb
        if1par=pb.zeros(2)                  # lsb,usb 
        if1par[0] = flo1 - paras            # if parasite in lsb
        if1par[1] = paras - flo1            # if parasite in usb
        # print if1par[0],if1par[1]

        for i in range(0,2):                # lsb,usb  
            if if1par[i] < if1max and if1par[i] > if1min:
                parif1[i] = True 
            # print " if1 = ",if1par[i],parif1[i]
                
        for i in range(0,numbb):
            if freq[(i+1)*2]/1e9 < flo1:    
                if2par[i] = flo2[i] - if1par[0]     # lsb
                #print i,' lsb',(freq[(i+1)*2]/1e9),if2par[i]
            if freq[(i+1)*2]/1e9 > flo1: 
                if2par[i] = flo2[i] - if1par[1]     # usb
                #print i,' usb',(freq[(i+1)*2]/1e9),if2par[i]
            if if2par[i] < if2max and if2par[i] > if2min:
                parif2[i] = True
                
            if3par[i] = flo3[i] - if2par[i]
            if if3par[i] < if3max and if3par[i] > if3min:
                parif3[i] = True
                # if3par_sbb[i] = ((if3par[i]-if3cen)*32./1.875)+(16.)  
                chanpar[i] = ((if3par[i]-if3cen)*nchan[i]/(tbw[(i+1)*2]/1e9)+(nchan[i]+1.)/2)  # valid if units are centered,
                                                                   # here LO4 is to be read once they're not centered anymore 
                # chanpar[i] = ((if3par[i]-if3cen)*nchan[i]/(tbw[(i+1)*2]/1e9)+(nchan[i])/2)   # valid if units are centered
                # print i,if3par[i],if3cen,(if3par[i]-if3cen),nchan[i],tbw[(i+1)*2]/1e9
            # print " if2 = ",i,if2par[i],flo2[i]
            # print " if3 = ",i,if3par[i],flo3[i]
            
    else:
        parif1 = False   
        parif2 = False  
        parif3 = False

        if (paras < 4.0) and (paras > 2.0):
            parif2 = True
        elif (paras > 4.0):
            parif1 = True

        if1par=pb.zeros(numbb)
        fsky_par=pb.zeros(numbb)
    
        if parif2:              # parasite in 2-4ghz bandwidth
            for i in range(0,numbb):
                if2par[i] = paras
                if1par[i] = flo2[i] - if2par[i]
                if3par[i] = flo3[i] - if2par[i]
                # print " parif2",if1par[i],if2par[i],if3par[i]

        if parif1:              # parasite in 4-12ghz bandwidth
            for i in range(0,numbb):
                if1par[i] = paras
                if2par[i] = flo2[i] - if1par[i]
                if3par[i] = flo3[i] - if2par[i]
                # print " parif1",if1par[i],if2par[i],if3par[i]
        
        for i in range(0,numbb):     # channel with parasite
            if if3par[i] > if3min and if3par[i] < if3max:
                # if3par_sbb[i] = ((if3par[i]-if3cen)*32./1.875)+(16.)  
                chanpar[i] = ((if3par[i]-if3cen)*nchan[i]/(tbw[(i+1)*2]/1e9)+(nchan[i]+1.)/2)  # valid if units are centered
                                                                   # here LO4 is to be read once they're not centered anymore 
                # chanpar[i] = ((if3par[i]-if3cen)*nchan[i]/(tbw[(i+1)*2]/1e9)+(nchan[i])/2)   # valid if units are centered
                # print if3par[i],if3cen,(if3par[i]-if3cen),nchan[i],tbw[(i+1)*2]/1e9
                
        for i in range(0,numbb):
            if freq[(i+1)*2]/1e9 < flo1: 
                fsky_par[i] = flo1 - 1*if1par[i]  #lsb
            if freq[(i+1)*2]/1e9 > flo1: 
                fsky_par[i] = flo1 + 1*if1par[i]  #usb
            
            
    # Present results
    # print " "
    # print " Input must be in GHz, either for Sky, LO1 or LO2 frequencies "
    print " "
    print " INPUT frequency at ...", paras, "GHz !!"
    print " "
    print " FLo1 (GHz)   =  %7.3f " %flo1
    print "        LSB=(","%5.3f" %(flo1-if1max),"-","%5.3f" %(flo1-if1min),")GHz  and  USB=(","%5.3f" %(flo1+if1min),"-","%5.3f" %(flo1+if1max),")GHz "
    print " IFLo1    LSB=(","%5.3f" %(if1max),"-","%5.3f" %(if1min),")GHz  and  USB=(","%5.3f" %(if1min),"-","%5.3f" %(if1max),")GHz "
    print " "
    # print "                   BB_1(L)    BB_2(L)    BB_3(U)    BB_4(U)" 
    print " FLo2 (GHz)   =  ", ["%7.3f" %flo2[i] for i in range(0,numbb)]
    print " IFLo2 BB_LSB=(","%5.3f" %(if2min),"-","%5.3f" %(if2max),")GHz  and BB_USB=(","%5.3f" %(if2max),"-","%5.3f" %(if2min),")GHz "
    print " "
    print " FLo3 (GHz)   =  ", ["%7.3f" %flo3[i] for i in range(0,numbb)]
    print " IFLo3 BB_LSB=(","%5.3f" %(if3max),"-","%5.3f" %(if3min),")GHz  and BB_USB=(","%5.3f" %(if3min),"-","%5.3f" %(if3max),")GHz "
    print " "
    print " N_channels   =  ", ["%i" %nchan[i] for i in range(0,numbb)]
    print "   channel width (KHz) = ", ["%7.5f" %((tbw[(i+1)*2]/1e3)/nchan[i]) for i in range(0,numbb)]
    print "     total width (MHz) = ", ["%7.3f" %((tbw[(i+1)*2]/1e6)) for i in range(0,numbb)]
    print " " 
    print " RESULTS:"

    if skyfreq:

        sb=["LSB","USB"]
        for i in range(0,2):
            if parif1[i]:
                print " Equivalent IF1 freq: %10.4f " %(if1par[i]*1000), "MHz in the IF_LO1",sb[i],",   of range (","%5.3f" %if1min,"-","%5.3f" %if1max,") GHz "
            #else:
                #print "         (deduced IF1 freq =%7.3f"  %if1par[i], sb[i],")"
                #  print parif1[i]
                out_of_range=False

        for i in range(0,numbb):
            if parif2[i]:
                print ""
                print " Equivalent IF2 freq: %10.4f " %(if2par[i]*1000), "MHz in the IF_LO2, BB_",(i+1)," of range (","%5.3f" %if2min,"-","%5.3f" %if2max,") GHz "
                out_of_range=False
                if parif3[i]:
                    print "         which is %10.6f " %(if3par[i]*1000), "MHz in the IF_LO3, BB_",(i+1)," of range (","%5.3f" %if3min,"-","%5.3f" %if3max,") GHz "
                    # print " IF_LO3 channel    = ", ["%i" %chanpar[i] for i in range(0,numbb)]
                    print " IF_LO3 channel    = ", ["%7.3f" %chanpar[i] for i in range(0,numbb)]
                    if chanpar[i] > nchan[i] or chanpar[i] < 0:
                        print "     which is out of range for the selected setup... "
                    # print " IF_LO3 SBB filter = ", ["%7.3f" %if3par_sbb[i] for i in range(0,numbb)]

        if out_of_range:
            print " The proposed frequency is out of range..."
            
    else:
        print " Sky freq (GHz) = ", ["%10.6f" %fsky_par[i] for i in range(0,numbb)]," from LSB=(","%5.3f" %(flo1-if1max),"-","%5.3f" %(flo1-if1min),") and USB=(","%5.3f" %(flo1+if1min),"-","%5.3f" %(flo1+if1max),")"
        print " IF_LO1 freq (MHz) = ", ["%10.4f" %(if1par[i]*1000) for i in range(0,numbb)]," of range (","%5.3f" %if1min,"-","%5.3f" %if1max,") GHz "
        print " IF_LO2 freq (MHz) = ", ["%10.4f" %(if2par[i]*1000) for i in range(0,numbb)]," of range (","%5.3f" %if2min,"-","%5.3f" %if2max,") GHz "
        print " IF_LO3 freq (MHz) = ", ["%10.7f" %(if3par[i]*1000) for i in range(0,numbb)]," of range (","%5.3f" %if3min,"-","%5.3f" %if3max,") GHz "

        # print " IF_LO3 channel    = ", ["%i" %chanpar[i] for i in range(0,numbb)]
        print " IF_LO3 channel    = ", ["%7.3f" %chanpar[i] for i in range(0,numbb)] # no integer, have more precision
        if chanpar[i] > nchan[i] or chanpar[i] < 0:
            print "     which is out of range for the selected setup... "
        # print " IF_LO3 SBB filter = ", ["%7.3f" %if3par_sbb[i] for i in range(0,numbb)]

    return

def ComputeDewPointCFromRHAndTempC(relativeHumidity, temperature):
    """
    inputs:  relativeHumidity in percentage, temperature in C
    output: in degrees C
    Uses formula from http://en.wikipedia.org/wiki/Dew_point#Calculating_the_dew_point
    Todd Hunter
    """
    es = 6.112*np.exp(17.67*temperature/(temperature+243.5))
    e = relativeHumidity*0.01*es
    dewPoint = 243.5*np.log(e/6.112)/(17.67-np.log(e/6.112))
    return(dewPoint)

def computeWVP(d):
    """
    This simply converts the specified temperature (in Celsius) to water vapor
    pressure, which can be used to estimate the relative humidity from the
    measured dew point. -- Todd Hunter
    """
    # d is in Celsius
    t = d+273.15
    w = np.exp(-6096.9385/t +21.2409642-(2.711193e-2)*t +(1.673952e-5)*t**2 +2.433502*log(t))
    return(w)


def resizeFonts(adesc,fontsize):
    """
    Todd Hunter
    """
    yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    adesc.xaxis.set_major_formatter(yFormat)
    pb.setp(adesc.get_xticklabels(), fontsize=fontsize)
    pb.setp(adesc.get_yticklabels(), fontsize=fontsize)

def estimateALMAOpacity(pwv,reffreq,airmass=1.0,h0=2.0,verbose=True):
   """
   Estimate the zenith opacity at ALMA using J. Pardo's model in casa.
   This function is useful if the weather conditions for your specific
   observation are not available and the average weather conditions must
   be used. -- Todd Hunter
   For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/EstimateOpacity
   """
   P = 563.0
   H = 20.0
   T = 273.0
   altitude = 5059
   midLatitudeWinter = 3
   at.initAtmProfile(humidity=H,
                     temperature=casac.Quantity(T,"K"),
                     altitude=casac.Quantity(altitude,"m"),
                     pressure=casac.Quantity(P,'mbar'),
                     atmType = midLatitudeWinter,
                     h0 =casac.Quantity(h0,'km'))

   chansep=1
   numchan=1
   nb = 1
   fC = casac.Quantity(reffreq,'GHz')
   fR = casac.Quantity(chansep,'GHz')
   fW = casac.Quantity(numchan*chansep,'GHz')
   at.initSpectralWindow(nb,fC,fW,fR)
   at.setUserWH2O(casac.Quantity(pwv,'mm'))
   dry = at.getDryOpacitySpec(0)['dryOpacity']
   wet = at.getWetOpacitySpec(0)['wetOpacity'].value
   if (verbose):
       print "%.1f GHz tau at zenith = %.3f (dry=%.3f, wet=%.3f)" % (reffreq,dry+wet, dry, wet)
   TebbSkyZenith = at.getTebbSky(nc=0, spwid=0).value
   TebbSky = TebbSkyZenith*(1-np.exp(-airmass*(wet+dry)))/(1-np.exp(-wet-dry))
   hvkT = h*reffreq*1e9/(k*TebbSky)
   J = hvkT / (np.exp(hvkT) - 1)
   TebbSky_Planck = TebbSky*J
   return(airmass*(dry+wet), TebbSky_Planck, TebbSky)
    
def estimateOpacity(pwvmean=1.0,reffreq=230,conditions=None,verbose=True,
                    elevation=90, altitude=5059, P=563, H=20, T=273, Trx=0,
                    etaTelescope=0.75,
                    telescope=None,help=False, airmass=0):
   """
   Estimate the opacity at a specified frequency and weather condition at ALMA
   using J. Pardo's ATM in casa. For further help, run estimateOpacity(help=True).
   Return values:
     If Trx is not specified, then it returns:  [tauZenith, tau]
     If Trx is specified, then it returns:
       [[tauZenith, tau], [transZenith, trans], [TskyZenith, Tsky], [TsysZenith, Tsys]]
   Equation for Tsys:
     (Trx + etaTelescope*T*(1-etaSky) + T*(1-etaTelescope))/(etaTelescope*etaSky)
     where etaSky = atmospheric transmission (as a fraction)
       and etaTelescope = telescope efficiency (as a fraction)
   -- Todd Hunter
   """
   if (help):
       print "Usage: estimateOpacity(pwvmean=1, reffreq=230, conditions=None, verbose=True,"
       print "       elevation=90, altitude=5059., P=563., H=20., T=273., Trx=0,"
       print "       etaTelescope=0.75, telescope=None, help=False, airmass=0)"
       print " units: pwv(mm), reffreq(GHz), elevation(deg), altitude(m), pressure(mb), temp(K)"
       print " The default values are nominal conditions at ALMA.  To change them, you may"
       print " either specify the weather and location variables, or use the shortcuts:"
       print "  telescope: 'ALMA', 'SMA', 'EVLA'"
       print "  conditions: a dictionary containing:"
       print "              pressure,solarangle,temperature,humidity,elevation"
       return
   # default P,H,T are set to ALMA typical, but should get this from observatory
   if (airmass >= 1.0):
       elevation = math.asin(1./airmass)*180/math.pi
   if (conditions != None):
     if (conditions['pressure'] > 1e-10):
       angle = conditions['solarangle']
       P = conditions['pressure']
       T = conditions['temperature'] + 273.15
       H = conditions['humidity']
       elevation = conditions['elevation']
   elif (telescope != None):
       if (telescope == 'SMA'):
           P = 629.5
           altitude = 4072
           print "Using pressure=%.1fmb, temperature=%.1fK and humidity=%.0f%% at SMA." % (P,T,H)
       elif (telescope.find('VLA')>=0):
           P = 785
           altitude = 2124
           print "Using pressure=%.1fmb, temperature=%.1fK and humidity=%.0f%% at %s." % (P,T,H,telescope)
       elif (telescope != 'ALMA'):
           print "Unrecognized telescope.  Available choices: SMA, ALMA, (E)VLA"
           return
   else:
       if (verbose):
           print "Using pressure=%.1fmb, temperature=%.1fK, humidity=%.0f%%, etaTelescope=%.2f at ALMA." % (P,T,H,etaTelescope)
   midLatitudeWinter = 3
   at.initAtmProfile(humidity=H, temperature=casac.Quantity(T,"K"),
                     altitude=casac.Quantity(altitude,"m"),
                     pressure=casac.Quantity(P,'mbar'),
                     atmType=midLatitudeWinter)
   chansep=1
   numchan=1
   nb = 1
   reffreq = double(reffreq)
   chansep = double(chansep)
   numchan = int(numchan)
   chansep = double(chansep)
   fC = casac.Quantity(reffreq,'GHz')
   fR = casac.Quantity(chansep,'GHz')
   fW = casac.Quantity(numchan*chansep,'GHz')
   at.initSpectralWindow(nb,fC,fW,fR)
   at.setUserWH2O(casac.Quantity(pwvmean,'mm'))
   spwid = 0
   dry = at.getDryOpacitySpec(spwid)['dryOpacity']
   wet = at.getWetOpacitySpec(spwid)['wetOpacity'].value
   TebbSkyZenith = at.getTebbSky(nc=0, spwid=spwid).value
   transZenith = math.exp(-dry-wet)*100
   if (verbose):
       print "%.1f GHz tau at zenith= %.3f (dry=%.3f,wet=%.3f), trans=%.1f%%, Tsky=%.2f" % (reffreq,dry+wet, dry, wet,math.exp(-dry-wet)*100, TebbSkyZenith)
   airmass = 1/math.sin(elevation*math.pi/180.)
   TebbSky = TebbSkyZenith * (1-np.exp(-airmass*(wet+dry)))/(1-np.exp(-wet-dry))
   trans = math.exp((-dry-wet)*airmass)*100
   if (verbose and elevation<90):
       print "%.1f GHz tau toward source (elev=%.1f,airm=%.2f)=%.3f, trans=%.1f%%, Tsky=%.2f" % (reffreq,elevation,airmass,(dry+wet)*airmass,math.exp((-dry-wet)*airmass)*100, TebbSky)
   if (Trx > 0):
       # compute expected Tsys
       etaSkyZenith = math.exp(-(dry+wet))
       TsysZenith = (T*(1/etaSkyZenith-1) + Trx/etaSkyZenith) / etaTelescope
       etaSky = math.exp(-airmass*(dry+wet))
       Tsys = (T*(1/etaSky-1) + Trx/etaSky) / etaTelescope
       if (verbose):
           print "Expected Tsys at zenith = %.1fK,   toward source = %.1fK" % (TsysZenith,Tsys)
       return([[dry+wet, (dry+wet)*airmass], [transZenith,trans], [TebbSkyZenith,TebbSky],
               [TsysZenith,Tsys]])
   else:
       return([dry+wet, (dry+wet)*airmass])

def computeTskyForSPW(vis='', spw=0, pwv=1.0):
   """
   Returns arrays of frequency and Tsky computed for the observation
   """
   spw = int(spw)
   try:
       tb.open(vis+'/SPECTRAL_WINDOW')
   except:
       print "Could not open ms = %s." % (vis)
       return
   midLatitudeWinter = 3
   T = 270
   altitude = 5056
   P = 563
   H = 20
   at.initAtmProfile(humidity=H, temperature=casac.Quantity(T,"K"),
                     altitude=casac.Quantity(altitude,"m"),
                     pressure=casac.Quantity(P,'mbar'),
                     atmType=midLatitudeWinter)

   numchan = tb.getcol("NUM_CHAN")[spw]    # num channels
   chanfreq = tb.getcol("CHAN_FREQ")[spw]    # num channels
   if (numchan == 1):
       chansep = 1
   else:
       chansep = (chanfreq[-1] - chanfreq[0]) / (numchan-1)
   numberWindows = 1
   reffreq = chanfreq[0]
   chansep = double(chansep)
   fC = casac.Quantity(reffreq,'Hz')
   fR = casac.Quantity(chansep,'Hz')
   fW = casac.Quantity(numchan*chansep,'Hz')
   at.initSpectralWindow(numberWindows, fC, fW, fR)
   at.setUserWH2O(casac.Quantity(pwv, 'mm'))
   spwid = 0
   try:
       TebbSky = at.getTebbSkySpec(spwid).value
   except:
       startTime = timeUtilities.time()
       TebbSky = []
#       print "Computing Tsky for all %d channels." % (numchan)
       for chan in range(numchan):
           TebbSky.append(at.getTebbSky(nc=chan, spwid=spwid).value)
       tdiff = timeUtilities.time()-startTime
#       print "Took %.1f sec to read Tsky for all channels." % (tdiff)
           
   chanfreqGHz = 1e-9 * chanfreq
   return(chanfreqGHz, TebbSky)
    

def readPWVFromMS(vis):
    tb.open("%s/ASDM_CALWVR" % vis)
    time = tb.getcol('startValidTime')  # mjdsec
    antenna = tb.getcol('antennaName')
    pwv = tb.getcol('water')
    tb.close()
    return[time,pwv,antenna]
    
def plotpwv(ms,figfile='',plotrange=[0,0,0,0],clip=True):
  """
  Read and plot the PWV values from the ms via the ASDM_CALWVR table.
  Different antennas are shown in different colored points.
  ms:  the measurement set
  plotrange: the ranges for the X and Y axes (default=[0,0,0,0] which is autorange)
  clip:  default=True,  True=do not plot outliers beyond 5*MAD from the median.
  figfile: True, False, or a string
           If figfile is not a string, the file created will be <ms>.pwv.png.
  For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/PlotPWV
  -- Todd Hunter
  """
  return(plotPWV(ms,figfile,plotrange,clip))

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

def plotPWV(ms, figfile='', plotrange=[0,0,0,0], clip=True):
  """
  Read and plot the PWV values from the ms via the ASDM_CALWVR table.
  Different antennas are shown in different colored points.
  Arguments:
  ms:  the measurement set
  plotrange: the ranges for the X and Y axes (default=[0,0,0,0] which is autorange)
  clip:  default=True,  True=do not plot outliers beyond 5*MAD from the median.
  figfile: True, False, or a string
           If figfile is not a string, the file created will be <ms>.pwv.png.
  For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/PlotPWV
  -- Todd Hunter
  """
  if (os.path.exists(ms) == False):
      print "Could not find the ms = %s" % (ms)
      return
  if (os.path.exists(ms+'/ASDM_CALWVR') == False):
      # Confirm that it is ALMA data
      observatory = getObservatoryName(ms)
      if (observatory.find('ALMA') < 0 and observatory.find('ACA')<0):
          print "This is not ALMA data, and this telescope (%s) does not appear to have WVR data." %(observatory)
      else:
          print "Could not find %s/ASDM_CALWVR.  Did you importasdm(asis='*')?" % (ms)
          print "You could also try: plotPWVFromASDM(your_asdm)"
      return
  try:
      [watertime,water,antennaName] = readPWVFromMS(ms)
  except:
      print "Could not open %s/ASDM_CALWVR.  Did you importasdm(asis='*')?" % (ms)
      print "You could also try: plotPWVFromASDM(your_asdm)"
      return
  pb.clf()
  adesc = subplot(111)
  ms = ms.split('/')[-1]
#  watertime = np.array(watertime)/86400.   
#  offset = int(floor(watertime[0]))
#  watertime -= offset
  water = np.array(water)*1000
  if (clip):
      mad = MAD(water)
      median = np.median(water)
      matches = np.where(abs(water - median) < 5*mad)[0]
      print "Dropped %d points" % (len(water)-len(matches))
      water = water[matches]
      watertime = watertime[matches]
      antennaName = antennaName[matches]
  uniqueAntennas = np.unique(antennaName)
  pb.hold(True)
  list_of_date_times = mjdSecondsListToDateTime(watertime)
  timeplot = pb.date2num(list_of_date_times)
  for a in range(len(uniqueAntennas)):
      matches = np.where(uniqueAntennas[a] == np.array(antennaName))[0]
      pb.plot_date(timeplot[matches], water[matches], '.', color=overlayColors[a])
#      pb.plot(watertime[matches], water[matches], '.', color=overlayColors[a])

  # now sort to average duplicate timestamps to one value, then fit spline
  indices = np.argsort(watertime)
  watertime = watertime[indices]
  water = water[indices]
  newwater = []
  newtime = []
  for w in range(len(water)):
      if (watertime[w] not in newtime):
          matches = np.where(watertime[w] == watertime)[0]
          newwater.append(np.median(water[matches]))
          newtime.append(watertime[w])
  watertime = newtime
  water = newwater
  regularTime = np.linspace(watertime[0], watertime[-1], len(watertime))
  ius = splrep(watertime, water,s=len(watertime)-sqrt(2*len(watertime)))
  water = splev(regularTime, ius, der=0)
  list_of_date_times = mjdSecondsListToDateTime(regularTime)
  timeplot = pb.date2num(list_of_date_times)
#  pb.plot(regularTime,water,'k-')
  pb.plot_date(timeplot,water,'k-')
  print "Median value at zenith = %.3f mm" % (np.median(water))
  if (plotrange[0] != 0 or plotrange[1] != 0):
      pb.xlim([plotrange[0],plotrange[1]])
  if (plotrange[2] != 0 or plotrange[3] != 0):
      pb.ylim([plotrange[2],plotrange[3]])
  xlim = pb.xlim()
  ylim = pb.ylim()
  xrange = xlim[1]-xlim[0]
  yrange = ylim[1]-ylim[0]
  for a in range(len(uniqueAntennas)):
      pb.text(xlim[1]+0.01*xrange, ylim[1]-0.04*yrange*a, antennaName[a], color=overlayColors[a])
  pb.xlabel('Universal Time (%s)' % (plotbp.utdatestring(watertime[0])))
  pb.ylabel('PWV (mm)')
  pb.title(ms)
  adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
  adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
  adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
  adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
  RescaleXAxisTimeTicks(pb.xlim(), adesc)
  autoFigureName = "%s.pwv.png" % (ms)
  pb.draw()
  if (figfile==True):
      pb.savefig(autoFigureName)
      print "Wrote file = %s" % (autoFigureName)
  elif (len(figfile)>0):
      pb.savefig(figfile)
      print "Wrote file = %s" % (autoFigureName)
  else:
      print "To create a png file, rerun with either:"
      print "  plotPWV(%s,figfile=True) to produce the automatic name=%s" % (ms,autoFigureName)
      print "  plotPWV('%s',figfile='myname.png')" % (ms)
   
def getSBGainFromASDM(asdm=''):
    """
    This function reads the CalAtmosphere.xml table from the ASDM and returns a dictionary
    """
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)
    sdmfile = asdm
    xmlscans = minidom.parse(sdmfile+'/CalAtmosphere.xml')
    scandict = {}
    rowlist = xmlscans.getElementsByTagName("row")
    fid = 0
    for rownode in rowlist:
      myrow = rownode.getElementsByTagName("syscalType")
      mytype = myrow[0].childNodes[0].nodeValue
      if (mytype == 'SIDEBAND_RATIO'):
        rowSBGain = rownode.getElementsByTagName("sbGain")
        sbGain = []
        tokens = rowSBGain[0].childNodes[0].nodeValue.split()
        gains = []
        for i in range(int(tokens[1])):
            gains.append(float(tokens[i+2]))
        sbGain = gains
        scandict[fid] = {}

        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName("startValidTime")
        start = int(rowstart[0].childNodes[0].nodeValue)
        startmjd = float(start)*1.0E-9/86400.0
        t = qa.quantity(startmjd,'d')
        starttime = call_qa_time(t,form="ymd",prec=8)
        rowend = rownode.getElementsByTagName("endValidTime")
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjd = float(end)*1.0E-9/86400.0
        t = qa.quantity(endmjd,'d')
        endtime = call_qa_time(t,form="ymd",prec=8)
        # antenna
        rowantenna = rownode.getElementsByTagName("antennaName")
        antenna = str(rowantenna[0].childNodes[0].nodeValue)
        # baseband
        rowbaseband = rownode.getElementsByTagName("basebandName")
        baseband = int(rowbaseband[0].childNodes[0].nodeValue.split('_')[1])
        # scan
        rowCalDataId = rownode.getElementsByTagName("calDataId")
        calDataId = str(rowCalDataId[0].childNodes[0].nodeValue)
        scan = int(calDataId.split('_')[1])
        rowCalDataId = rownode.getElementsByTagName("receiverBand")
        receiverBand = int(rowCalDataId[0].childNodes[0].nodeValue.split('_')[2])

        scandict[fid]['start'] = starttime
        scandict[fid]['end'] = endtime
        scandict[fid]['startmjd'] = startmjd
        scandict[fid]['endmjd'] = endmjd
        scandict[fid]['startmjdsec'] = startmjd*86400
        scandict[fid]['endmjdsec'] = endmjd*86400
        timestr = starttime+'~'+endtime
        scandict[fid]['timerange'] = timestr
        scandict[fid]['antenna'] = antenna
        scandict[fid]['baseband'] = baseband
        scandict[fid]['scan'] = scan
        scandict[fid]['sbGain'] = sbGain
        scandict[fid]['receiverBand'] = receiverBand
        scandict[fid]['duration'] = (endmjd-startmjd)*86400
        fid += 1

    print '  Found ',rowlist.length,' rows in CalAtmosphere.xml'

    # return the dictionary for later use
    return scandict
    
    
def plotSBGainFromASDM(asdm='', pol='', figfile='', verbose=False,antenna='',baseband=''):
    """
    Plots the sideband gain ratio values from the SIDEBAND_RATIO entries in
    the CalAtmosphere.xml table for the specified ASDM.
    pol: 'X', 'Y', or ''=both,  only plot this polarization
    figfile: name of png file to save, default = asdm + '.sbgain.png'
    verbose: print all values of sbGain
       antenna: restrict the verbose printing to one specified antenna name
       baseband: restrict the verbose printing to one specified baseband number (1..4)
    -Todd Hunter
    """
    scandict = getSBGainFromASDM(asdm)
    sbGains = []
    sbGainsX = []
    sbGainsY = []
    scans = []
    antennas = []
    receiverBands = []
    basebands = []
    mjdsec = []
    for sd in scandict:
        s = scandict[sd]
        sbGainsX.append(s['sbGain'][0])
        if (len(s['sbGain']) > 1):
            sbGainsY.append(s['sbGain'][1])
        scans.append(s['scan'])
        antennas.append(s['antenna'])
        receiverBands.append(s['receiverBand'])
        basebands.append(s['baseband'])
#        mjdsec.append((s['startmjdsec']+s['endmjdsec'])*0.5)
#   The end time is often set to be way in the future (30 days).
        mjdsec.append(s['startmjdsec'])
        if (verbose):
            if (antenna == '' or antenna == s['antenna']):
              if (baseband == '' or baseband == s['baseband']):
                print "ant=%s, scan=%d, rx=%d, baseband=%d, s['sbGain'] = " % (s['antenna'],
                    s['scan'], s['receiverBand'], s['baseband']), s['sbGain']
    sbGainsX = np.array(sbGainsX)
    sbGainsY = np.array(sbGainsY)
    sbGainsXdB = 10*np.log10(1-sbGainsX)
    sbGainsYdB = 10*np.log10(1-sbGainsY)
    uniqueAntennaNames = list(np.unique(antennas))
    uniqueAntennaNumbers = range(len(uniqueAntennaNames))
    antennaNumbers = []
    for antennaName in antennas:
        antennaNumbers.append(uniqueAntennaNames.index(antennaName))
    matches = np.where(sbGainsX > 0.01)  # avoid apparently bogus scans
    if (pol == '' or 'X' in pol):
        print "Median  for Pol X = %f/%f = %f dB" % (np.median(sbGainsX[matches]),
                                                     1-np.median(sbGainsX[matches]),
                                        10*np.log10(1-np.median(sbGainsX[matches])))
        print "Minimum for Pol X = %f/%f = %f dB" % (np.min(sbGainsX[matches]),
                                                     1-np.min(sbGainsX[matches]),
                                             10*np.log10(1-np.min(sbGainsX[matches])))
        print "Maximum for Pol X = %f/%f = %f dB" % (np.max(sbGainsX[matches]),
                                                     1-np.max(sbGainsX[matches]),
                                                 10*np.log10(1-np.max(sbGainsX[matches])))
    if (pol == '' or 'Y' in pol):
        print "Median for Pol Y = %f/%f = %f dB" % (np.median(sbGainsY[matches]),
                                                    1-np.median(sbGainsY[matches]),
                                             10*np.log10(1-np.median(sbGainsY[matches])))
        print "Minimum for Pol Y = %f/%f = %f dB" % (np.min(sbGainsY[matches]),
                                                     1-np.min(sbGainsY[matches]),
                                             10*np.log10(1-np.min(sbGainsY[matches])))
        print "Maximum for Pol Y = %f/%f = %f dB" % (np.max(sbGainsY[matches]),
                                                     1-np.max(sbGainsY[matches]),
                                            10*np.log10(1-np.max(sbGainsY[matches])))
    matches = np.where(sbGainsX < 0.1)[0]
    for i in matches:
        print "Suspect value=%f for pol X, ant=%s, calDataId=%d, rx=%d, baseband=%d" % (sbGainsX[i], antennas[i],
                    scans[i], receiverBands[i], basebands[i])
    matches = np.where(sbGainsY < 0.1)[0]
    for i in matches:
        print "Suspect value=%f for pol Y, ant=%s, calDataId=%d, rx=%d, baseband=%d" % (sbGainsY[i], antennas[i],
                    scans[i], receiverBands[i], basebands[i]) 

    pb.clf()
    list_of_date_times = mjdSecondsListToDateTime(mjdsec)
    timeplot = pb.date2num(list_of_date_times)
    if (1==0):
        adesc = pb.subplot(111)
        if (pol == '' or 'X' in pol):
            adesc.plot_date(timeplot, sbGainsX, 'bo')
            pb.hold(True)
        if (pol == '' or 'Y' in pol):
            adesc.plot_date(timeplot, sbGainsY, 'go')
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
        adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
        RescaleXAxisTimeTicks(pb.xlim(), adesc)
        pb.xlabel("Time (UT on %s)" % (plotbp.utdatestring(mjdsec[0])))
        pb.ylabel("Sideband gain ratio")
        adesc.xaxis.grid(True,which='major')
        adesc.yaxis.grid(True,which='major')
    else:
        adesc = pb.subplot(211)
        pb.hold(True)
        c = ['k','r','g','c']
        for a in range(len(antennaNumbers)):
            pb.plot(antennaNumbers[a], sbGainsX[a], 'o', color=c[basebands[a]-1])
            pb.plot(antennaNumbers[a], sbGainsY[a], '+', color=c[basebands[a]-1], linewidth=2)
        x0,x1 = pb.xlim()
        y0,y1 = pb.ylim()
        yrange = y1-y0
        pb.xlim([-0.9,x1+0.9])
        pb.ylim([y0-0.2*yrange,y1+yrange*0.05])
        y0,y1 = pb.ylim()
        yrange = y1-y0
        for a in range(len(uniqueAntennaNames)):
            pb.text(a-0.5, y0+0.01*yrange, uniqueAntennaNames[a], size=8)
        bbs = np.unique(basebands)
        for a in range(len(bbs)):
            pb.text(0.1+a*0.1, 0.08, 'BB%d'%bbs[a], color=c[bbs[a]-1], size=10, transform=adesc.transAxes)
        pb.text(0.1+0.1*len(bbs), 0.08, 'XX=dots', size=10, transform=adesc.transAxes)
        pb.text(0.25+0.1*len(bbs), 0.08, 'YY=crosses', size=10, transform=adesc.transAxes)
        pb.ylabel("Sideband gain ratio (sbGain)")
        pb.xlabel('Antenna')
        pb.title('Band %d (%s) ASDM = %s' % (receiverBands[0],plotbp.utstring(mjdsec[0]),asdm.split('/')[-1]), size=12)
        adesc.xaxis.grid(True,which='major')
        adesc.yaxis.grid(True,which='major')

        adesc = pb.subplot(212)
        for a in range(len(antennaNumbers)):
            pb.plot(antennaNumbers[a], sbGainsXdB[a], 'o', color=c[basebands[a]-1])
            pb.plot(antennaNumbers[a], sbGainsYdB[a], '+', color=c[basebands[a]-1], linewidth=2)
        pb.xlabel('Antenna')
        x0,x1 = pb.xlim()
        y0,y1 = pb.ylim()
        yrange = y1-y0
        pb.xlim([-0.9,x1+0.9])
        pb.ylim([y0-0.2*yrange,y1+1])
        y0,y1 = pb.ylim()
        yrange = y1-y0
        for a in range(len(uniqueAntennaNames)):
            pb.text(a-0.5, y0+0.01*yrange, uniqueAntennaNames[a], size=8)
        for a in range(len(bbs)):
            pb.text(0.8+a*2, y0+0.08*yrange, 'BB%d'%bbs[a], color=c[bbs[a]-1], size=10)
        pb.text(0.5+2*len(bbs), y0+0.08*yrange, 'XX=dots', size=10)
        pb.text(2.8+2*len(bbs), y0+0.08*yrange, 'YY=crosses', size=10)
        pb.ylabel("10*log10(1-sbGain) (dB)")
        adesc.xaxis.grid(True,which='major')
        adesc.yaxis.grid(True,which='major')
    if (figfile == ''):
        figfile = asdm + '.sbgain.png'
    pb.savefig(figfile)

def RescaleXAxisTimeTicks(xlim, adesc):
    if (xlim[1] - xlim[0] < 10/1440.):
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,1)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,30)))
    elif (xlim[1] - xlim[0] < 0.5/24.):
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,5)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,1)))
    elif (xlim[1] - xlim[0] < 1/24.):
        adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
        adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,2)))
                


    
def plotPWVfromASDM(asdm='',figfile=False,showMedianOnly=True,avgtime=30.0,
                    meanelev=-1, freq=-1,
                    help=False, station=16, linetype='.-', markersize=7):
  """
  Read and plot the PWV values from the ASDM via the CalWVR.xml table.
  avgtime is the time in secs over which to compute the median value from all"
  antennas.  If meanelev is specified, it will compute the PWV toward that"
  elevation. If freq is further specified (GHz), it will compute the"
  transmission toward that elevation using the measured weather conditions."
  For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/PlotPWVfromASDM
   -- Todd Hunter
  """
  plotPWVFromASDM(asdm, figfile, showMedianOnly, avgtime, meanelev, freq,
                  help, station, linetype, markersize)
  return
    
def plotPWVFromASDM(asdm='',figfile=False,showMedianOnly=True,avgtime=30.0,
                    meanelev=-1, freq=-1,
                    help=False, station=16, linetype='.-', markersize=7):
  """
  Read and plot the PWV values from the ASDM via the CalWVR.xml table.
  avgtime is the time in secs over which to compute the median value from all
  antennas.  If meanelev is specified, it will compute the PWV toward that
  elevation. If freq is further specified (GHz), it will compute the
  transmission toward that elevation using the measured weather conditions.
  For further help and examples, see
  https://safe.nrao.edu/wiki/bin/view/ALMA/PlotPWVfromASDM
   -- Todd Hunter
  """
  if (help):
      print "Usage: plotPWVFromASDM(asdm='', figfile=False, showMedianOnly=True, avgtime=30.0,"
      print "    meanelev=-1, freq=-1, markersize=7, help=False)"
      print "  avgtime is the time in seconds over which to compute the median value from all"
      print "  antennas.  If meanelev is specified, it will compute the PWV toward that"
      print "  elevation. If freq is further specified (GHz), it will compute the"
      print "  transmission toward that elevation using the measured weather conditions."
      return
  if (os.path.exists(asdm)):
    try:
      [watertime,water,antenna] = readpwv(asdm)
    except:
      print "Could not open %s/CalWVR.xml" % (asdm)
      return
  else:
      print "Could not find file %s/CalWVR.xml" % (asdm)
      return
  pb.clf()
  adesc = subplot(111)
  asdm = asdm.split('/')[-1]
  print "Found WVR data from %d antennas" % (len(np.unique(antenna)))
  roundedData = avgtime*(np.round(np.array(watertime)/avgtime))
  uniqueTimes = np.unique(roundedData)
  watertime = np.array(watertime)
  mjdsec = watertime
#  watertimeHours = np.array(watertime)/3600.
#  watertimeDays = np.array(watertime)/86400.
#  offset = int(floor(watertimeDays[0]))
#  watertimeDays -= offset
  water = np.array(water)*1000
  print "Found %d times" % (len(uniqueTimes))
  timeMedian = []
  pwvMedian = []
  if (showMedianOnly==False):
      list_of_date_times = mjdSecondsListToDateTime(watertime)
      timeplot = pb.date2num(list_of_date_times)
      pb.plot(timeplot, water, 'b.',markersize=markersize)
      pb.hold(True)
      for t in uniqueTimes:
          matches = np.where(roundedData == t)[0]
          timeMedian.append(np.median(watertime[matches]))
          pwvMedian.append(np.median(water[matches]))
#      pb.plot(timeMedian, pwvMedian, 'r%s'%(linetype),markersize=markersize+2)
      list_of_date_times = mjdSecondsListToDateTime(timeMedian)
      timeplot = pb.date2num(list_of_date_times)
      pb.plot_date(timeplot, pwvMedian, 'r%s'%(linetype),markersize=markersize+2)
  else:
      pb.hold(True)
      for t in uniqueTimes:
          matches = np.where(roundedData == t)[0]
          timeMedian.append(np.median(watertime[matches]))
          pwvMedian.append(np.median(water[matches]))
#      pb.plot(timeMedian, pwvMedian, 'b%s'%(linetype), markersize=markersize+2)
      list_of_date_times = mjdSecondsListToDateTime(timeMedian)
      timeplot = pb.date2num(list_of_date_times)
      pb.plot_date(timeplot, pwvMedian, 'r%s'%(linetype),markersize=markersize+2)
  pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(watertime[0]))
  if (showMedianOnly):
      pb.ylabel('PWV (mm) (median of all antennas)')
  else:
      pb.ylabel('PWV (mm) (red = median of all antennas)')
      
  adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
  adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
  adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
  adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
  RescaleXAxisTimeTicks(pb.xlim(), adesc)
  adesc.xaxis.grid(True,which='major')
  adesc.yaxis.grid(True,which='major')
  (mjd, utstring) = mjdSecondsToMJDandUT(mjdsec[0])
  if (asdm == "." or asdm == "./"):
      pb.title(os.getenv('PWD').split('/')[-1]+'   start time='+utstring)
  else:
      pb.title(asdm+'   start time='+utstring)
  deltaTime = (np.max(watertime) - np.min(watertime))/3600.
  print " median PWV at zenith = %.3f mm over %.3f hours" % (np.median(water),deltaTime)
  if (meanelev > 0):
      airmass = 1.0/math.cos((90-meanelev)*math.pi/180.)
      print " median PWV at elev=%.0f = %.3f mm over %.3f hours" % (meanelev,airmass*np.median(water),deltaTime)
      if (freq > 0):
          [conditions,medianConditions] = getWeatherFromASDM(asdm,station=station)
          P = medianConditions['pressure']
          H = medianConditions['humidity']
          T = medianConditions['temperature']
          print "Median conditions: P,H,T = ", P, H, T
          midLatitudeWinter = 3
          at.initAtmProfile(humidity=H,temperature=casac.Quantity(T,"K"),altitude=casac.Quantity(5059,"m"),pressure=casac.Quantity(P,'mbar'),atmType=midLatitudeWinter)
          fCenter = casac.Quantity(freq,'GHz')
          fWidth = casac.Quantity(1.0,'GHz')
          fResolution = fWidth
          at.initSpectralWindow(1, fCenter, fWidth, fResolution)
          at.setUserWH2O(casac.Quantity(airmass*np.median(water), 'mm'))
          dry = np.array(at.getDryOpacitySpec(0)['dryOpacity'])
          wet = np.array(at.getWetOpacitySpec(0)['wetOpacity'].value)
          transmission = np.exp((-wet-dry)*airmass)* 100
          print " transmission at median PWV at %.1f GHz = %.1f percent" % (freq, transmission)
  pb.draw()
  if (figfile==True):
      myfig = asdm + '.pwv.png'
      pb.savefig(myfig, density=144)
      print "Figure saved to %s" % (myfig)
  elif (figfile != False):
      myfig = figfile
      pb.savefig(myfig, density=144)
      print "Figure saved to %s" % (myfig)
  else:
      print "To write a png file, re-run with figfile=T"
                                                        
def plotweather(vis='', figfile='', station=[], help=False):
    """
    Compiles and plots the major weather parameters for the specified ms. 
    Station can be a single integer or integer string, or a list of two integers.
    The default empty list means to plot all data from up to 2 of the stations
    present in the data.  The default plot file name will be 'vis'.weather.png.
    For further help and examples, see http://casaguides.nrao.edu/index.php?title=Plotweather
    -- Todd Hunter
    """
    return(plotWeather(vis,figfile,station,help))

def plotWeather(vis='', figfile='', station=[], help=False):
    """
    Compiles and plots the major weather parameters for the specified ms.
    Station can be a single integer or integer string, or a list of two integers.
    The default empty list means to plot all data from up to 2 of the stations
    present in the data.  The default plot file name will be 'vis'.weather.png.
    For further help and examples, see http://casaguides.nrao.edu/index.php?title=Plotweather
    -- Todd Hunter
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
        tb.open("%s/WEATHER" % vis)
    except:
        print "Could not open WEATHER table.  Did you importasdm with asis='*'?"
        return
    mjdsec = tb.getcol('TIME')
    mjdsec1 = mjdsec
    vis = vis.split('/')[-1]
    pressure = tb.getcol('PRESSURE')
    relativeHumidity = tb.getcol('REL_HUMIDITY')
    temperature = tb.getcol('TEMPERATURE')
    dewPoint = tb.getcol('DEW_POINT')
    stations = tb.getcol('NS_WX_STATION_ID')
    windDirection = (180/math.pi)*tb.getcol('WIND_DIRECTION')
    windSpeed = tb.getcol('WIND_SPEED')
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
        pressure = pressure[firstStationRows]
        relativeHumidity = relativeHumidity[firstStationRows]
        temperature = temperature[firstStationRows]
        dewPoint = dewPoint[firstStationRows]
        windDirection = windDirection[firstStationRows]
        windSpeed = windSpeed[firstStationRows]
        mjdsec1 = mjdsec[firstStationRows]

        pressure2 = tb.getcol('PRESSURE')[secondStationRows]
        relativeHumidity2 = tb.getcol('REL_HUMIDITY')[secondStationRows]
        temperature2 = tb.getcol('TEMPERATURE')[secondStationRows]
        dewPoint2 = tb.getcol('DEW_POINT')[secondStationRows]
        windDirection2 = (180/math.pi)*tb.getcol('WIND_DIRECTION')[secondStationRows]
        windSpeed2 = tb.getcol('WIND_SPEED')[secondStationRows]
        mjdsec2 = mjdsec[secondStationRows]
        if (np.mean(temperature2) > 100):
            # convert to Celsius
            temperature2 -= 273.15        
        if (np.mean(dewPoint2) > 100):
            dewPoint2 -= 273.15        
        
    if (np.mean(temperature) > 100):
        # convert to Celsius
        temperature -= 273.15        
    if (np.mean(dewPoint) > 100):
        dewPoint -= 273.15        
    if (np.mean(dewPoint) == 0):
        # assume it is not measured and use NOAA formula to compute from humidity:
        dewPoint = ComputeDewPointCFromRHAndTempC(relativeHumidity, temperature)
    if (np.mean(relativeHumidity) < 0.001):
        print "Replacing zeros in relative humidity with value computed from dew point and temperature."
#        dewPointWVP = np.exp(17.271*dewPoint/(237.7+dewPoint))
#        ambientWVP = np.exp(17.271*temperature/(237.7+temperature))
        dewPointWVP = computeWVP(dewPoint)
        ambientWVP = computeWVP(temperature)
        print "dWVP=%f, aWVP=%f" % (dewPointWVP[0],ambientWVP[0])
        relativeHumidity = 100*(dewPointWVP/ambientWVP)

    tb.close()
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
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
#    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Pressure (mb)',size=mysize)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

    adesc = pb.subplot(322)
    pb.plot_date(timeplot,temperature, markersize=markersize)
    if (len(uniqueStations) > 1):
        pb.hold(True)
        list_of_date_times = mjdSecondsListToDateTime(mjdsec2)
        timeplot2 = pb.date2num(list_of_date_times)
        pb.plot_date(timeplot2, temperature2, markersize=markersize, color='r')
    resizeFonts(adesc,myfontsize)
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
#    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
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
    else:
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
#    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
    pb.ylabel('Relative Humidity (%)',size=mysize)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

    adesc = pb.subplot(324)
    pb.plot_date(timeplot,dewPoint, markersize=markersize)
    if (len(uniqueStations) > 1):
        pb.hold(True)
        list_of_date_times = mjdSecondsListToDateTime(mjdsec2)
        timeplot2 = pb.date2num(list_of_date_times)
        pb.plot_date(timeplot2, dewPoint2, markersize=markersize, color='r')
    resizeFonts(adesc,myfontsize)
#    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Dew point (C)',size=mysize)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

    adesc = pb.subplot(325)
    pb.plot_date(timeplot, windSpeed, markersize=markersize)
    if (len(uniqueStations) > 1):
        pb.hold(True)
        list_of_date_times = mjdSecondsListToDateTime(mjdsec2)
        timeplot2 = pb.date2num(list_of_date_times)
        pb.plot_date(timeplot2, windSpeed2, markersize=markersize, color='r')
    resizeFonts(adesc,myfontsize)
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Wind speed (m/s)',size=mysize)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

    adesc= pb.subplot(326)
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
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

def plotWeatherFromASDM(asdm,station=16):
    """
    Calls getWeatherFromASDM() for the specified ASDM, then plots the results.
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/PlotWeatherFromASDM
    -- Todd Hunter
    """
    myfontsize = 8
    [conditions, medianConditions] = getWeatherFromASDM(asdm,station=station,verbose=False)
#    print "len(conditions) = ", len(conditions)
#    print "shape(conditions) = ", np.shape(conditions)
    mjdsec = np.array(conditions[0])
#    print "len(mjdsec) = ", len(mjdsec)
    pressure = conditions[1]
    temperature = conditions[3]
    station = conditions[-1]
    pb.clf()
    adesc = pb.subplot(321)
    pb.title(asdm+' (station %d)'%station,size=10)
#    pb.plot(mjdFraction,pressure)
    list_of_date_times = mjdSecondsListToDateTime(mjdsec)
    timeplot = pb.date2num(list_of_date_times)
    pb.plot_date(timeplot,pressure)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')
    yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    mysize = 'small'
#    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Pressure (mb)',size=mysize)

    adesc = pb.subplot(322)
#    pb.plot(mjdFraction,np.array(temperature)-273.15)
    pb.plot_date(timeplot,np.array(temperature)-273.15)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
#    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Temperature (C)',size=mysize)
    pb.title('%s - %s' % (plotbp.utstring(conditions[0][0]),plotbp.utstring(conditions[0][-1])), size=10)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

    relativeHumidity = conditions[2]
    adesc = pb.subplot(323)
#    pb.plot(mjdFraction,relativeHumidity)
    pb.plot_date(timeplot,relativeHumidity)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
#    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
    pb.ylabel('Relative Humidity (%)',size=mysize)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

#    adesc = pb.subplot(324)
#    pb.plot(mjdFraction,dewPoint)
#    pb.xlabel('MJD - %d'%mjdOffset,size=mysize)
#    pb.ylabel('Dew point (C)',size=mysize)

    windSpeed = conditions[5]
    adesc = pb.subplot(325)
#    pb.plot(mjdFraction,windSpeed)
    pb.plot_date(timeplot,windSpeed)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Wind speed (m/s)',size=mysize)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')

    windDirection = conditions[4]
    adesc= pb.subplot(326)
    pb.xlabel('Universal Time (%s)'%plotbp.utdatestring(mjdsec[0]),size=mysize)
    pb.ylabel('Wind direction (deg)',size=mysize)
#    pb.plot(mjdFraction,windDirection)
    pb.plot_date(timeplot,windDirection)
    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
    yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    RescaleXAxisTimeTicks(pb.xlim(), adesc)
    adesc.xaxis.grid(True,which='major')
    adesc.yaxis.grid(True,which='major')
    weatherFile = asdm+'.weather.png'
    pb.subplots_adjust(hspace=0.25, wspace=0.30)
    pb.savefig(weatherFile)
    pb.draw()
    print "Wrote file = %s" % (weatherFile)

def find_key(dic, val):
    """return the key of dictionary dic given the value"""
    return [k for k, v in dic.iteritems() if v == val][0]

def rad2radec(ra=0,dec=0,imfitdict=None, prec=5):
    """
    Convert a position in RA/Dec from radians to sexagesimal.
    The position can either be entered via the 'ra' and 'dec' parameters, 
    or an imfit dictionary can be passed via the 'imfitdict' argument, and the
    position of component 0 will be displayed in RA/Dec sexagesimal.
    Todd Hunter
    """
    if (type(imfitdict) == dict):
        ra  = imfitdict['results']['component0']['shape']['direction']['m0']['value']
        dec = imfitdict['results']['component0']['shape']['direction']['m1']['value']
    mystring = '%s, %s' % (qa.formxxx('%.12frad'%ra,format='hms',prec=prec),
                          qa.formxxx('%.12frad'%dec,format='dms',prec=prec).replace('.',':',2))
    print mystring
    return(mystring)
    
def direction2radec(direction=None, prec=5):
    """
    Convert a direction dictionary to a sexagesimal string.
    Todd Hunter
    """
    ra  = direction['m0']['value']
    dec = direction['m1']['value']
    mystring = '%s, %s' % (qa.formxxx('%.12frad'%ra,format='hms',prec=prec),
                           qa.formxxx('%.12frad'%dec,format='dms',prec=prec).replace('.',':',2))
    return(mystring)
    
def parseSSSerror(s):
    if (s == 0):
        return('success')
    elif (s==1):
        return ('Error: unsupported body')
    elif (s==2):
        return ('Error: unsupported frequency or time for body')
    elif (s==3):
        return ('Error: Tb model file not found')
    elif (s==4):
        return ('Error: ephemeris table not found, or time out of range (note - the case where the MJD times span two ephemeris files is not supported')
    else:
        return('Error: unknown error code')
    

def planetFlux(body='', date=None, mjd=55600, frequency=345e9, bandwidth=1e6, 
               dayIncrement=1.0, plotfile=None, verbose=False):
    """
    A wrapper for testing Bryan Butler's solar_system_fd() function.
    Makes a plot of flux density vs. time,  or vs. frequency if a range of dates
    or frequencies is requested.

    Parameters:
    body: To see the list of supported planets, type help(au.sss.solar_system_fd)
    mjd: a single value or a list of 1 or 2 MJDs (2 = a range)
    date: a string of format:  2011/10/15, 2011/10/15 05:00:00, or 2011/10/15-05:00:00"
    frequency: a single value or a list of 1 or 2 frequencies in Hz (a list of 2 is interpreted as a range)
    bandwidth: define the bandwidth at a single frequency or the increment for the range (in Hz)
    dayIncrement: define the MJD increment for the plot
    plotfile: specify the output png name, None=use default name

    Return value:
        if single value computed:  direction dictionary
        if plot computed: nothing

    Example: plot Neptune's flux density in ALMA Band 7:
      au.planetFlux('Neptune',mjd=55600, frequency=[275e9,373e9], bandwidth=1e9)

    - Todd Hunter
    """
    if (useSolarSystemSetjy == False):
        print "This version of CASA does not contain the solar_system_setjy module."
        return
    if (type(frequency) != list):
        frequency = [[frequency-0.5*bandwidth, frequency+0.5*bandwidth]]
    elif (type(frequency[0]) != list):
        if (len(frequency) == 1):
            frequency = [[frequency[0]-0.5*bandwidth, frequency[0]+0.5*bandwidth]]
        else:
            frequency = [frequency]
    elif (len(frequency[0]) == 1):
        frequency = [[frequency[0][0]-0.5*bandwidth,frequency[0][0]+0.5*bandwidth]]
    if (verbose):
        print "frequency = ", frequency
    if (date != None):
        if (type(date) == list):
            mjd1 = dateStringToMJD(date[0])
            if (mjd1 == None):
                return
            if (len(date) > 1):
                mjd2 = dateStringToMJD(date[1])
                if (mjd2 == None):
                    return
                mjd = [mjd1,mjd2]
            else:
                mjd = [mjd1]
        else:
            mjd = dateStringToMJD(date)
            if (mjd == None):
                return
    if (type(mjd) != list):
        mjd = [mjd]
    if ((frequency[0][1]-frequency[0][0]) <= bandwidth):
        if (len(mjd) == 1):
            (status,flux,uncertainty,size,direction) = sss.solar_system_fd(body, MJDs=mjd, frequencies = frequency)
            if (status[0][0] == 0):
                # Just print the flux density
                print "Flux density at %.3f GHz averaged over a %.3f GHz bandwidth is %f Jy." % (np.mean(frequency[0])*1e-9, (frequency[0][1]-frequency[0][0])*1e-9, flux[0][0])
                print "J2000 Position = %s" % (direction2radec(direction[0]))
                return(direction[0])
            else:
                print parseSSSerror(status[0][0])

        else:
            # Make a plot of flux and major axis vs. time
            mjd = np.arange(mjd[0], mjd[1], dayIncrement)
#            print "mjds = ", mjd
#            print "frequencies = ", frequency
            (status,flux,uncertainty,size,direction) = sss.solar_system_fd(body, MJDs=mjd, frequencies = frequency)
            if (status[0][0] != 0):
                print parseSSSerror(status[0][0])
                return
            pb.clf()
            adesc = pb.subplot(211)
            flux = [item for sublist in flux for item in sublist]
            pb.plot(mjd, flux, 'b-')
            pb.xlabel('Date (MJD)')
            pb.ylabel('Flux density (Jy)')
            pb.title(body+' at %.3f GHz (BW=%.3fGHz)'%(np.mean(frequency[0])*1e-9, bandwidth*1e-9))
            adesc.xaxis.grid(True,which='major')
            adesc.yaxis.grid(True,which='major')

            adesc = pb.subplot(212)
            majoraxis = np.transpose(size)[0]
            pb.plot(mjd, majoraxis, 'b-')
            pb.xlabel('Date (MJD)')
            pb.ylabel('Major axis (arcsec)')
            adesc.xaxis.grid(True,which='major')
            adesc.yaxis.grid(True,which='major')
            if (plotfile == None):
                plotfile = body+'.fluxvstime.%d-%d.png'%(mjd[0],mjd[-1])
            pb.savefig(plotfile)
            print "Plot left in ", plotfile
            pb.draw()
    elif (len(mjd) == 1):
        # Make a plot of flux vs. frequency
        fluxdensity = []
        freq = []
        for f in np.arange(frequency[0][0], frequency[0][1], bandwidth):
            freqs = [[f,f+bandwidth]]
            (status,flux,uncertainty,size,direction) = sss.solar_system_fd(body, MJDs=mjd, frequencies = freqs)
            if (status[0][0] != 0):
                print parseSSSerror(status[0][0])
                return
            fluxdensity.append(flux[0][0])
            freq.append(f+0.5*bandwidth)
        pb.clf()
        adesc = pb.subplot(111)
        freqGHz = np.array(freq)*1e-9
        pb.plot(freqGHz, fluxdensity, 'b-')
        pb.xlabel('Frequency (GHz)')
        pb.ylabel('Flux density (Jy)')
        adesc.xaxis.grid(True,which='major')
        adesc.yaxis.grid(True,which='major')
        pb.title(body+' at MJD=%.1f (%s)'%(mjd[0], mjdToUT(mjd[0])))
        if (plotfile == None):
            plotfile = body+'.fluxvsfreq.%d-%dGHz.png'%(int(freq[0]*1e-9),int(freq[-1]*1e-9))
        pb.savefig(plotfile)
        print "Plot left in ", plotfile
        pb.draw()
    else:
        print "Multiple MJDs and multiple frequencies are not supported."
    return

def planet(body='',date='',observatory=JPL_HORIZONS_ID['ALMA'],
           verbose=False,help=False, mjd=None,
           beam='',useJPL=True, standard=defaultEphemeris, subroutine=False,
           apparent=False):
    """
    This function returns the position and angular size of any Solar System
    body. If useJPL=True (default), it queries the JPL Horizons telnet server
    for exact positions as viewed from the specified observatory.
    If useJPL=False, it will first try to use the casa ephemerides
    (default='Butler-JPL-Horizons 2010').  Note that the casa tables are 
    quantized to one day. If the casa tables fail
    for any reason, it will automatically revert to using JPL's server.
    The dictionory returned is:
      data['directionRadians']
      data['angularDiameter']
      data['rateRadiansPerSecond'] (only present if useJPL=True)
      data['rangeRateKms'] (only present if useJPL=True)
      data['rangeAU'] (only present if useJPL=True)
    body: string name of Solar system body
    date: one possible format of the date string is: '2011-10-31 11:59:59'
    mjd: floating point MJD (alternative to specifying a date string)
    observatory: string name or JPL integer ID (run planet(help=True) for a list)
    beam: FWHM of the observing beam in arcsecs
    standard: the name of the ephemeris table to use (if useJPL=False)
    apparent: get the apparent coordinates rather than J2000
    
    For further help and examples, run au.planet(help=True) or
    see https://safe.nrao.edu/wiki/bin/view/ALMA/Planet
    -- Todd Hunter
    """
    if (help):
        print "This function returns the position and angular size of any "
        print "Solar System body for the specified date from the specified observatory."
        print "Usage: planet(solarSystemBody, date, observatory='%s', verbose=False," % (JPL_HORIZONS_ID['ALMA'])
        print "              beam='', useJPL=True, standard='%s', apparent=False)"%(defaultEphemeris)
        print "  body: string name of Solar system body"
        print "  date: one possible format of the date string is: '2011-10-31 11:59:59'"
        print "    or simply '2011-10-31' for 0:00 UT. A list of allowed formats for date"
        print "    is at:   http://ssd.jpl.nasa.gov/?horizons_doc#time"
        print "  mjd: floating point MJD (alternative to specifying a date string)"
        print "  useJPL: if False, first try the casa ephemerides.  Note that the casa tables"
        print "    are quantized to one day, and can be in error by many arcminutes. If the "
        print "    casa tables fail for any reason, it will automatically revert to using "
        print "    JPL's server.  It may require a few seconds to reach the JPL server."
        print "  observatory: Observatories can be specifed by JPL ID string, or by the "
        print "    following names:"
        for n in JPL_HORIZONS_ID:
            print "     '%s' (which will be converted to = '%s')" % (n, JPL_HORIZONS_ID[n])
        print "     '500' (which will be converted to = 'Geocentric')"
        print "  beam:  If a beam size is included (in arcsec), the expected FWHM will be"
        print "    computed using scipy.signal.convolve() of a Gaussian with a uniform disk."
        print "  apparent: False = the return values are J2000 values as seen from the specified observatory"
        print "  apparent: True = the return values are apparent values as seen from the specified observatory"
        print "The dictionary returned is:"
        print "  data['directionRadians']"
        print "  data['angularDiameter']"
        print "  data['rateRadiansPerSecond'] (only present if useJPL=True)"
        print "  data['rangeRateKms'] (only present if useJPL=True)"
        print "  data['rangeAU'] (only present if useJPL=True)"
        return
    if (apparent and useJPL==False):
        print "You cannot request apparent=True with useJPL=False. CASA contains only one coordinate set."
        return
    foundObservatory  = False
    if (type(observatory) == int):
        observatory = str(observatory)
    elif (type(observatory) == str):
        if (len(observatory) < 1):
            observatory = 'ALMA'
    for n in JPL_HORIZONS_ID:
        if (n.find(observatory) >= 0 or (len(observatory)>2 and observatory.find(n) >= 0)):
            observatory = JPL_HORIZONS_ID[n]
            print "Using observatory: %s = %s" % (n, JPL_HORIZONS_ID[n])
            foundObservatory  = True
            break
    if (foundObservatory == False):
        if (observatory.lower().find('geocentric') >= 0):
            observatory = 500
        try:
            o = int(observatory)
            key = []
            try:
                key = find_key(JPL_HORIZONS_ID,observatory)
            except:
                if (key == []):
                    print "Using observatory: %s" % (observatory)
                else:
                    print "Using observatory: %s = %s" % (observatory,key)
                
        except:
            print "Unrecognized observatory = %s" % (observatory)
            print "For a list of codes, see http://ssd.jpl.nasa.gov/horizons.cgi#top"
            return
    if (len(date) < 1):
        if (mjd == None):
            date = datetime.date.today().strftime('%Y-%m-%d')
            print "No date/time specified, assuming today=%s at UT 0:00" % (date)
        else:
            date = mjdToUT(float(mjd))
            print "Converted MJD=%f to %s" % (mjd,date)
    fp = FixPosition()
    while (body[-1] == ' ' and len(body) > 0):
        body = body[0:-1]
    try:
        if (useJPL==False and (observatory.find('-7')>=0 or observatory.find('ALMA')>=0) and usePredictComp):
            print "Using casa's ephemerides for %s. Note these are in apparent coordinates in casa <=3.4." % (body)
            if (len(date.split()) == 1):
                epoch = date + ' 00:00:00'
            else:
                epoch = date
            print "Using epoch = ", epoch
            data = {}

            myim = imtool.create()
            myme = metool.create()
            mepoch = myme.epoch('UTC', epoch)  # This ignores the hours portion.
            result =  epoch.split()
            if (len(result) > 1):
                result = result[1].split(':')
                hours = 0
                if (len(result) > 0):
                    hours += int(result[0])
                if (len(result) > 1):
                    hours += int(result[1]) / 60.
                if (len(result) > 2):
                    hours += int(result[2]) / 3600.
#                print "Adding %f days" % (hours/24.)
                mepoch['m0']['value'] = mepoch['m0']['value'] + hours/24.
            clist = myim.predictcomp(body, standard, mepoch, [345e9], '/tmp/')
            # clist is the name of the disk file component list created
            mycl = cltool.create()
            mycl.open(clist)
            d = mycl.getcomponent(0)
            mycl.close(False)   # False prevents the stupid warning on the terminal.
            os.system('rm -r %s'%clist)

            data['angularDiameter'] = d['shape']['majoraxis']['value']
            if (d['shape']['majoraxis']['unit'] == 'arcmin'):
                data['angularDiameter'] *= 60
            data['directionRadians'] = [d['shape']['direction']['m0']['value'],
                                        d['shape']['direction']['m1']['value']]
            directionDegrees = [data['directionRadians'][0]*180/math.pi,
                                data['directionRadians'][1]*180/math.pi
                                ]
            if (apparent):
                coords = 'Apparent Position'
            else:
                coords = 'J2000 Position'

            print '%s: %s, %s' % (coords,qa.formxxx('%.12fdeg'%directionDegrees[0],format='hms',prec=5),
                                  qa.formxxx('%.12fdeg'%directionDegrees[1],format='dms',prec=4).replace('.',':',2))
        else:
            if (useJPL==False and (observatory.find('-7')>=0 or observatory.find('ALMA')>=0) and usePredictComp==False):
                print "Will not use casa because predictcomp module is missing."
#            print "Will not use casa, observatory=", observatory
            raise
    except:
        print "Contacting JPL Horizons for %s" % (body)
        data = fp.getRaDecSize(body, date, observatory, verbose, apparent)
    if (data['angularDiameter'] != []):
        print "Angular diameter = %f arcsec" % (data['angularDiameter'])
        if (beam != ''):
            expectedFWHM = computeExpectedFWHM(data['angularDiameter'],float(beam))
            print 'The expected FWHM with a %.2f" beam is %.2f".' % (float(beam),expectedFWHM)
    else:
        print "Angular diameter is not available."
    return(data)
    
def computeExpectedFWHM(disk_diameter_arcsec, beam_fwhm):
    """
    Performs a convolution of a top-hat function with a Gaussian, and
    reports the FWHM of the result.
    """
    p = 1
    M = 1000  # number of points in the profile
    image_size_arcsec = np.max([3*disk_diameter_arcsec,3*beam_fwhm])
    scale_size = image_size_arcsec/np.float(M)
    disk_radius_arcsec = disk_diameter_arcsec*0.5
    sig = beam_fwhm*0.5 / ((2*math.log(2))**0.5) / scale_size
    gaussian = spsig.general_gaussian(M,p,sig)
    r = scale_size*(np.arange(0, M) - (M - 1.0) / 2.0)
    disk = np.zeros(len(r))
    for i in range(len(r)):
        if (np.abs(r[i]) < disk_radius_arcsec):
            disk[i] = 1

    beam = spsig.convolve(gaussian,disk,mode='same')
    beam = beam/np.max(beam)
    for i in range(len(beam)):
        if (beam[i] > 0.5):
            fwhm = np.abs(2*r[i])
            break
    return(fwhm)

def buildConfigurationFile(vis='', simmospath=None):
    """
    Reads the list of antenna stations in an .ms and creates a .cfg file based
    on the antenna locations in the files in /usr/lib64/casapy/data/alma/simmos.
    Useful for running simulations on previously-obtained datasets.
    vis: an ms
    simmospath: alternative place to search for observatory .cfg files
    - Todd Hunter
    """
    if (vis == ''):
        print "Usage: buildConfigurationFile(vis=myvis)"
        return
    if (simmospath == None or simmospath == ''):
        repotable = os.getenv("CASAPATH").split()[0]+"/data/alma/simmos/"
    else:
        repotable = simmospath
        if (repotable[-1] != '/'): 
            repotable += '/'
    observatory = getObservatoryName(vis)
    tb.open(vis+'/ANTENNA')
    stations = tb.getcol('STATION')
    output = vis+'.cfg'
    cfg = open(output, 'w')
    if (observatory.find('VLA')>=0):
        coordsys = '# coordsys=XYZ\n'
        observatory = 'VLA'
    elif (observatory.find('ALMA')>=0):
        coordsys = '# coordsys=LOC (local tangent plane)\n'
    else:
        coordsys = '# coordsys=LOC (local tangent plane)\n'
    cfg.write(coordsys)
    cfg.write('# observatory=%s\n' % observatory)
    cfg.write('# x y z diam pad#\n')
    cfg.write('# %s\n' % vis)
    found = np.zeros(len(stations))
    configs = 6  # number of Cycle 1 configs
    wrote = 0
    dirlist = os.listdir(repotable)
    configs = []
    for c in dirlist:
        if ((c.find('alma')>=0 and c.find('cycle')>=0) or (c.find('aca')==0)):
            configs.append(c)
    configs.append('vla.a.cfg')
    configs.append('vla.b.cfg')
    configs.append('vla.c.cfg')
    configs.append('vla.d.cfg')
    configs.append('vla.bna.cfg')
    configs.append('vla.cnb.cfg')
    configs.append('vla.dnc.cfg')
    for c in range(len(configs)):
      cstation = 0
      cfilename = '%s%s' % (repotable,configs[c])
      if (os.path.exists(cfilename)):
        cfile = open(cfilename, 'r')
        lines = cfile.readlines()
        for s in range(len(stations)):
            if (found[s] == 0):
                for line in lines:
                    if (line.find('#') < 0):
                        cstation += 1
                        if (line.find(stations[s]) > 0 or
                            # The following line can be removed once ACA
                            # files have pad names.
                            (c==configs and cstation==int(stations[s][-1]) and stations[s].find('T70')>=0)):
                            found[s] = 1
                            print "Found %s in file: %s" % (stations[s],cfilename)
                            cfg.write(line)
                            wrote += 1
                            break
        cfile.close()
    cfg.close()
    print "Wrote %d of %d antenna stations to: %s" % (wrote,len(stations),output)
    for s in range(len(stations)):
        if (found[s] == 0):
            print "Did not find %s." % (stations[s])
    
def buildMinorPlanetPlot(bodies='Venus,Mars,Ceres,Vesta,Pallas,Juno,Jupiter,Saturn,Uranus,Neptune',date='',help=False,cals=[],observatory='ALMA',useJPL=False,standard=defaultEphemeris):
    """
    Takes a list of solar system bodies and a date, and creates a Gildas astro
    script that will plot their elevation vs. time at ALMA.  If useJPL=False
    (default), it will first try to use the casa ephemerides to get the positions
    of the non-major planets in the body list.  If this fails, or if useJPL=True,
    it queries the JPL Horizons telnet server to get the positions.
    Usage: au.buildMinorPlanetPlot(bodies='Venus,Mars,Ceres,Vesta,Pallas,Juno,
        Jupiter,Saturn,Uranus,Neptune',date='',help=False,cals=[],
        observatory='ALMA',useJPL=False,standard='Butler-JPL-Horizons 2010')
    The date format required by Astro is: DD-MMM-YYYY, i.e. 01-apr-2012
    Additional fixed sources can be added as strings, e.g.:
        cals=['3C279 12:56:11.16 -05:47:21.5', '3C273 12:29:06.7 +02:03:08.6']
    For further help and examples, see
    https://safe.nrao.edu/wiki/bin/view/ALMA/BuildMinorPlanetPlot
    -- Todd Hunter
    """
    if (help):
        print "Takes a list of solar system bodies and a date, and creates a Gildas astro"
        print "script that will plot their elev. vs. time at ALMA.  If useJPL=False (default),"
        print "it will first try to use the casa ephemerides to get the positions of the"
        print "non-major planets in the body list.  If this fails, or if useJPL=True,"
        print "then it queries the JPL Horizons telnet server to get the positions."
        print ""
        print "Usage: buildMinorPlanetPlot(bodies='Venus,Mars,Ceres,Vesta,Pallas,Juno,Jupiter,Saturn,Uranus,Neptune',date='',help=False,cals=[],observatory='ALMA',useJPL=False,standard='%s')"%(defaultEphemeris)
        print "  The date format required by Astro is: DD-MMM-YYYY, i.e. 01-apr-2012"
        print "  Additional fixed sources can be added as strings, e.g.:"
        print "   cals=['3C279 12:56:11.166 -05:47:21.52', '3C273 12:29:06.699 +02:03:08.598']"
        print "Observatories can be specifed by JPL ID string, or by the following names:"
        for n in JPL_HORIZONS_ID:
            print "   '%s' (which will be converted to = '%s')" % (n, JPL_HORIZONS_ID[n])
        return
    if (len(date) < 1):
        mydate = datetime.date.today().strftime('%d-%b-%Y')
        print "No date/time specified, assuming today=%s at UT 0:00" % (date)
    else:
        try:
            mydate = datetime.datetime.strptime(date,'%d-%b-%Y')
            # it will bomb here if the format is not correct
            mydate = date
        except:
            print "You need to specify a date like so:  date='01-apr-2012'"
            return

    # The following objects are already known about by Astro, but need to be in order.
    majorPlanets = ['SUN','MERCURY','VENUS','MOON','MARS','JUPITER','SATURN','URANUS','NEPTUNE','PLUTO']
    fname = 'minorplanets.%s'%(mydate)
    f = open(fname, 'w')
    sname = "%s.astro" % mydate
    scriptname = open(sname,'w')
    if (type(cals) != list):
        cals = [cals]
    calnames = []
    for cal in cals:
        tokens = cal.split()
        if (len(tokens)<3):
            print "1)Invalid calibrator string format: ", cal
            return
#        print "tokens = ", tokens
        # remove any spaces in the source name
        myname = ''
        for t in range(len(tokens)-2):
            myname += tokens[t]
        calnames.append(myname.upper())
        if ((tokens[-2].find(':') < 1) or (tokens[-2].find(':') < 1)):
            print "2)Invalid calibrator string format: ", cal
            return
        f.write('%s %s %s LS 0\n'%(calnames[-1],tokens[-2],tokens[-1]))
    bodyList = bodies.split(',')
    for body in bodyList:
        if (body.upper() not in majorPlanets):
            planetData = planet(body,date=mydate,observatory=observatory,
                                verbose=False,useJPL=useJPL,
                                standard=standard,subroutine=True)
            dRad = planetData['directionRadians']
            f.write("%s %s %s LS 0\n"%(str(body).upper(), qa.formxxx('%frad'%(dRad[0]),format='hms',prec=3),
                                      qa.formxxx('%frad'%(dRad[1]),format='dms',prec=2).replace('.',':',2)))
    f.close()
    f = open(sname,"w")
    f.write("set plot landscape\n")
    f.write("pen /weight 2\n")
    f.write("catalog %s\n"%(fname))
    if (observatory == 'OVRO'):
        f.write("observatory -118:16:54.00 +37:13:54.12\n")
    elif (observatory == 'GBT'):
        f.write("observatory -79:50:24.00 +38:25:58.58 0.822 50\n")
    else:
        f.write("observatory %s\n"%(observatory))

    f.write("time 00:00 %s\n"%(mydate))
    f.write("frame horizontal\n")
    f.write("horizon 20 30 40 50 60 70 /source ")
    for body in bodyList:
        if (body.upper() not in majorPlanets):
            f.write(body.upper() + ' ')
    for calname in calnames:
        f.write('%s '%calname)
    f.write('/planet SUN ')
    planetsToWrite = []
    for body in bodyList:
        if (body.upper() in majorPlanets):
            planetsToWrite.append(body.upper())
    for p in majorPlanets:
        if (p in planetsToWrite):
            f.write(p + ' ')
    f.write('/night\n')
    f.write("$rm -f %s.eps\n"%(fname))
    f.write("hard %s.eps /dev eps f\n"%(fname))
    f.write("$convert -rotate 90 -density 108 %s.eps %s.png\n"%(fname,fname))
    f.write("say wrote plot files: %s.ps and %s.png"%(fname,fname))
    print "Wrote %s and %s to be used by gildas astro to create plots." % (fname,sname)
    try:
        shell = os.getenv('SHELL')
    except:
        shell = '/bin/tcsh'
    if (shell.find('csh')>=0):
        print "In Charlottesville, do the following (1-3 can be put in your .cshrc):"
        print "1) setenv GAG_EXEC_SYSTEM x86_64-redhatClient-g95"
        print "2) setenv GAG_ROOT_DIR /opt/local/stow/gildas-exe-dec10c/"
    else:
        print "In Charlottesville, do the following (1-3 can be put in your .bashrc):"
        print '1) export GAG_EXEC_SYSTEM="x86_64-redhatClient-g95"'
        print '2) export GAG_ROOT_DIR="/opt/local/stow/gildas-exe-dec10c/"'
    print "3) source $GAG_ROOT_DIR/etc/login"
    print "4) astro (which runs $GAG_ROOT_DIR/$GAG_EXEC_SYSTEM/bin/astro)"
    print "5) @%s" % (sname)
    return
# end of buildMinorPlanetPlot()

def plotbandpass(caltable='', antenna='', field='', spw='', yaxis='amp',
                 xaxis='chan', figfile='', plotrange=[0,0,0,0], help=False,
                 caltable2='',overlay='', showflagged=False, timeranges='',
                 buildpdf=False, caltable3='',markersize=3, density=108,
                 interactive=True, showpoints='auto', showlines='auto',
                 subplot='22', zoom='', poln='', showatm=False, pwv='auto',
                 gs='gs', convert='convert', chanrange='',
                 solutionTimeThresholdSeconds=60.0, debug=False, vm='',
                 phase='', ms='', showtsky=False, showfdm=False,showatmfield='',
                 lo1=None, showimage=False, showatmPoints=False):
  """
  This is a tool to plot bandpass solutions faster than plotcal.  It is designed
  to work on the old cal table format of casa 3.3 and prior.  The source code is
  in plotbandpass.py.  If it detects a new cal table, it will automatically call
  plotbandpass2, which was developed for casa 3.4 cal tables.
  For more detailed help, run au.plotbandpass(help=True) or see examples at:
  http://casaguides.nrao.edu/index.php?title=Plotbandpass
  -- Todd Hunter
  """
  retval = plotbp.plotbandpass(caltable, antenna, field, spw, yaxis,
                             xaxis, figfile, plotrange, help,
                             caltable2, overlay, showflagged, timeranges,
                             buildpdf, caltable3, markersize, density,
                             interactive, showpoints, showlines,
                             subplot, zoom, poln, showatm, pwv,
                             gs, convert, chanrange,
                             solutionTimeThresholdSeconds, debug, vm,
                             phase, ms, showtsky, showfdm, showatmfield,
                             lo1, showimage, showatmPoints)
  if (retval == 3.4):
      retval = plotbandpass2(caltable, antenna, field, spw, yaxis,
                             xaxis, figfile, plotrange, help,
                             caltable2, overlay, showflagged, timeranges,
                             buildpdf, caltable3, markersize, density,
                             interactive, showpoints, showlines,
                             subplot, zoom, poln, showatm, pwv,
                             gs, convert, chanrange,
                             solutionTimeThresholdSeconds, debug, vm,
                             phase, ms, showtsky, showfdm,showatmfield,
                             lo1, showimage, showatmPoints)
  return(retval)

def plotbandpass2(caltable='', antenna='', field='', spw='', yaxis='amp',
                 xaxis='chan', figfile='', plotrange=[0,0,0,0], help=False,
                 caltable2='',overlay='', showflagged=False, timeranges='',
                 buildpdf=False, caltable3='',markersize=3, density=108,
                 interactive=True, showpoints='auto', showlines='auto',
                 subplot='22', zoom='', poln='', showatm=False, pwv='auto',
                 gs='gs', convert='convert', chanrange='',
                 solutionTimeThresholdSeconds=60.0, debug=False, vm='',
                 phase='', ms='', showtsky=False, showfdm=False,showatmfield='',
                 lo1=None, showimage=False, showatmPoints=False):
  """
  This is a tool to plot bandpass solutions faster than plotcal.  It is designed
  to work for the new cal table format introduced in casa 3.4.  The source code
  is in plotbandpass2.py. If it detects a new cal table, it will automatically
  call plotbandpass, which was developed for casa 3.3 cal tables. For more
  detailed help, run au.plotbandpass2(help=True) or see examples at:
  http://casaguides.nrao.edu/index.php?title=Plotbandpass
  -- Todd Hunter
  """
  retval = plotbp2.plotbandpass2(caltable, antenna, field, spw, yaxis,
                             xaxis, figfile, plotrange, help,
                             caltable2, overlay, showflagged, timeranges,
                             buildpdf, caltable3, markersize, density,
                             interactive, showpoints, showlines,
                             subplot, zoom, poln, showatm, pwv,
                             gs, convert, chanrange,
                             solutionTimeThresholdSeconds, debug, vm,
                             phase, ms, showtsky, showfdm,showatmfield,
                             lo1,showimage, showatmPoints)
  if (retval == 3.3):
      retval = plotbandpass(caltable, antenna, field, spw, yaxis,
                             xaxis, figfile, plotrange, help,
                             caltable2, overlay, showflagged, timeranges,
                             buildpdf, caltable3, markersize, density,
                             interactive, showpoints, showlines,
                             subplot, zoom, poln, showatm, pwv,
                             gs, convert, chanrange,
                             solutionTimeThresholdSeconds, debug, vm,
                             phase, ms, showtsky, showfdm,showatmfield,
                             lo1,showimage, showatmPoints)      
  return(retval)

def getCOFA(ms):
    """
    Return the ITRF coordinates, Longitude and Latitude of the center of the array of the
    specified ms.  - Todd Hunter
    """
    u = simutil.simutil()
    try:
        antTable = ms+'/ANTENNA'
        tb.open(antTable)
    except:
        print "Could not open ANTENNA table: %s" % (antTable)
        return([],[])
    position = tb.getcol('POSITION')
    station = tb.getcol('STATION')
    name = tb.getcol('NAME')
    tb.close()
    try:
        antTable = ms+'/OBSERVATION'
        tb.open(antTable)
        myName = tb.getcell('TELESCOPE_NAME')
        tb.close()
    except:
        print "Could not open OBSERVATION table to get the telescope name: %s" % (antTable)
        myName = ''
        return
    repotable=os.getenv("CASAPATH").split()[0]+"/data/geodetic/Observatories"
    tb.open(repotable)
    Name = tb.getcol('Name')
    cx = position[0][0]
    cy = position[1][0]
    cz = position[2][0]
    myType = ''
    for i in range(len(Name)):
        if (Name[i] == myName):
            Long = tb.getcell('Long',i)
            Lat = tb.getcell('Lat',i)
            Height = tb.getcell('Height',i)
            myType = tb.getcell('Type',i)
            if (myType == 'ITRF'):
                cx = tb.getcell('X',i)
                cy = tb.getcell('Y',i)
                cz = tb.getcell('Z',i)
            else:
                # WGS84
                output = u.long2xyz(Long*math.pi/180.,Lat*math.pi/180.,Height,myType)
                (cx,cy,cz) = output
            break
    return(cx,cy,cz,Long,Lat)
    
def obslist(ms,cofa=''):
    """
    Parses the telescope name from the OBSERVATION table, then finds the
    coordinates of the center of the array (COFA), then converts each station
    coordinates into local offsets in meters from the COFA and prints the
    information. You can optionally define the COFA as an antenna ID or name
    via the cofa parameter.  For further help and examples, see:
    http://casaguides.nrao.edu/index.php?title=Obslist
    Todd Hunter
    """
    try:
        antTable = ms+'/ANTENNA'
        tb.open(antTable)
    except:
        print "Could not open ANTENNA table: %s" % (antTable)
        return([],[])
    position = tb.getcol('POSITION')
    station = tb.getcol('STATION')
    name = tb.getcol('NAME')
    tb.close()
    try:
        antTable = ms+'/OBSERVATION'
        tb.open(antTable)
        myName = tb.getcell('TELESCOPE_NAME')
        tb.close()
    except:
        print "Could not open OBSERVATION table to get the telescope name: %s" % (antTable)
        myName = ''
        
    u = simutil.simutil()
    repotable=os.getenv("CASAPATH").split()[0]+"/data/geodetic/Observatories"
    tb.open(repotable)
    Name = tb.getcol('Name')
    cx = position[0][0]
    cy = position[1][0]
    cz = position[2][0]
    myType = ''
    if (cofa == ''):
      for i in range(len(Name)):
        if (Name[i] == myName):
            Long = tb.getcell('Long',i)
            Lat = tb.getcell('Lat',i)
            Height = tb.getcell('Height',i)
            myType = tb.getcell('Type',i)
            if (myType == 'ITRF'):
                cx = tb.getcell('X',i)
                cy = tb.getcell('Y',i)
                cz = tb.getcell('Z',i)
            else:
                # WGS84
                output = u.long2xyz(Long*math.pi/180.,Lat*math.pi/180.,Height,myType)
                (cx,cy,cz) = output
            break
    else:
        if (str(cofa).isdigit()):
            if (int(cofa) < 0 or int(cofa) >= len(name)):
                print "Invalid antenna ID. (Valid range = 0..%d)" % (len(name)-1)
                return
        else:
            matches = np.where(name == cofa)[0]
            if (len(matches) < 1):
                print "cofa must be either an integer, a blank string, or a valid antenna name"
                return
            else:
                print "Antenna %s has ID %d" % (cofa,matches[0])
                cofa = matches[0]
            cx = position[0][int(cofa)]
            cy = position[1][int(cofa)]
            cz = position[2][int(cofa)]
        myType = 'user'
    if (len(myType) < 1):
        print "Did not find telescope data, using first station as center of array."
    tb.close()
    try:
        (x,y) = output = u.irtf2loc(position[0,:],position[1,:],position[2,:],cx,cy,cz)
    except:
        (x,y) = output = u.itrf2loc(position[0,:],position[1,:],position[2,:],cx,cy,cz)
    for i in range(len(x)):
        print "Antenna %2d = %s on pad %s, lat/long offset (m) = %+10.4f/%+10.4f" % (i,name[i],station[i],x[i],y[i])
    getBaselineExtrema(ms)
    return

def antennaPosition(vis, vis2=None, ant=''):
    """
    This utility prints the XYZ positions from the ANTENNA table for 1 or 2 ms's.
    If vis2 is specified, the difference in positions is also computed.
    This is meant to help understan antenna position correction algorithms.
    Todd Hunter  (May 2012)
    """
    try:
        antTable = vis+'/ANTENNA'
        tb.open(antTable)
    except:
        print "Could not open ANTENNA table: %s" % (antTable)
        return([],[])
    position = tb.getcol('POSITION')
    station = tb.getcol('STATION')
    names = tb.getcol('NAME')
    tb.close()

    if (vis2 != None):
      try:
          antTable2 = vis2+'/ANTENNA'
          tb.open(antTable2)
      except:
          print "Could not open ANTENNA table: %s" % (antTable2)
          return([],[])
      position2 = tb.getcol('POSITION')
      station2 = tb.getcol('STATION')
      names2 = tb.getcol('NAME')
      tb.close()

    antennas = len(station)
    axis = ['X','Y','Z']
    if (vis2 == None):
        print "           vis1"
    else:
        print "           vis1              vis2        Difference (m)"
    for antenna in range(len(names)):
      if (ant == '' or names[antenna] == ant):
        print "Antenna %02d = %4s on %s: " % (antenna,names[antenna],station[antenna])
        for component in range(3):
            comp = position[component][antenna]
            if (vis2 == None):
                print "  %s: %+.6f" % (axis[component],comp)
            elif (names[antenna] in names2):
                index2 = list(names2).index(names[antenna])
                comp2 = position2[component][index2]
                if (component == 0):
                    print "  %s: %+.6f   %+.6f    %.6f  (%s on %s)" % (axis[component],comp,comp2,comp-comp2,names2[index2],station2[index2])
                else:
                    print "  %s: %+.6f   %+.6f    %.6f" % (axis[component],comp,comp2,comp-comp2)
            else:
                print "  %s: %+.6f" % (axis[component],comp)
    if (vis2 != None):
        for antenna in range(len(names2)):
          if (ant == '' or names2[antenna] == ant):
            if (names2[antenna] not in names):
                print     "                        Antenna %02d = %4s on %s: " % (antenna,names2[antenna],station2[antenna])
                for component in range(3):
                    comp2 = position2[component][antenna]
                    print "  %s: ----------------  %+.6f" % (axis[component],comp2)
            
def antennaPositionASDM(vis, vis2=None, ant='', itrf=False, verbose=False):
    """
    This utility prints the antenna positions from the ASDM_ANTENNA table for 1 or 2 ms's.
    If vis2 is specified, the difference in positions is also computed.
    This is meant to help understand antenna position correction algorithms.
    Specifying ant will limit the display to one antenna.
    If itrf=True, then convert positions from relative ENU to absolute ECEF.
    Todd Hunter  (May 2012)
    """
    try:
        antTable = vis+'/ASDM_ANTENNA'
        tb.open(antTable)
    except:
        print "Could not open ASDM_ANTENNA table: %s" % (antTable)
        return([],[])
    position = tb.getcol('position')
    stationId = tb.getcol('stationId')
    names = tb.getcol('name')
    tb.close()
    try:
        antTable = vis+'/ASDM_STATION'
        tb.open(antTable)
    except:
        print "Could not open ASDM_STATION table: %s" % (antTable)
        return([],[])
    station = tb.getcol('name')
    padPosition = tb.getcol('position')
    padStationId = tb.getcol('stationId')
    tb.close()

    if (vis2 != None):
      try:
          antTable2 = vis2+'/ASDM_ANTENNA'
          tb.open(antTable2)
      except:
          print "Could not open ASDM_ANTENNA table: %s" % (antTable2)
          return([],[])
      position2 = tb.getcol('position')
      stationId2 = tb.getcol('stationId')
      names2 = tb.getcol('name')
      tb.close()
      try:
          antTable2 = vis2+'/ASDM_STATION'
          tb.open(antTable2)
      except:
          print "Could not open ASDM_STATION table: %s" % (antTable2)
          return([],[])
      station2 = tb.getcol('name')
      padPosition2 = tb.getcol('position')
      padStationId2 = tb.getcol('stationId')

    antennas = len(station)
    if (itrf):
        axis = ['    X','    Y','    Z']
        cx,cy,cz,long,lat = getCOFA(vis)
        if (vis2 == None):
            print "           vis1"
        else:
            print "           vis1            vis2            Difference (m)  (vis1-vis2)"
    else:
        axis = ['East ','North','Up   ']
        if (vis2 == None):
            print "       vis1"
        else:
            print "       vis1        vis2        Difference (m)  (vis1-vis2)"
    for antenna in range(len(names)):
      antindex = list(padStationId).index(stationId[antenna])
      if (itrf):
          # compute antenna XYZ from pad XYZ and ENU correction
          # http://en.wikipedia.org/wiki/Geodetic_system   (ENU to ECEF)
          if (verbose):
              print "antenna%d=%s in vis1 is on %s=%s" % (antenna,names[antenna],
                                                          stationId[antenna],station[antindex])
          phi = lat*math.pi/180.
          lam = math.atan2(padPosition[1][antindex], padPosition[0][antindex])
          itrf_correction = []
          itrf_correction.append(-np.sin(lam)*position[0][antenna] \
                                 -np.sin(phi)*np.cos(lam)*position[1][antenna] + \
                                  np.cos(phi)*np.cos(lam)*position[2][antenna] + \
                                 padPosition[0][antindex])
          itrf_correction.append(+np.cos(lam)*position[0][antenna] \
                                 -np.sin(phi)*np.sin(lam)*position[1][antenna] + \
                                  np.cos(phi)*np.sin(lam)*position[2][antenna] + \
                                 padPosition[1][antindex])
          itrf_correction.append(+np.cos(phi)*position[1][antenna] + \
                                  np.sin(phi)*position[2][antenna] + padPosition[2][antindex])
          if (names[antenna] in names2):
              antenna2 = list(names2).index(names[antenna])
              vis2stationId = stationId2[antenna2]
              ant2index = list(padStationId2).index(vis2stationId)
              if (verbose):
                  print "antenna%d=%s in vis2 is on %s=%s (ant2index=%d)" % (antenna2,
                         names2[antenna2],stationId2[antenna2],station2[ant2index], ant2index)
              itrf_correction2 = []
              phi = lat*math.pi/180.
              lam = math.atan2(padPosition2[1][ant2index], padPosition2[0][ant2index])
              itrf_correction2.append(-np.sin(lam)*position2[0][antenna2] \
                                     -np.sin(phi)*np.cos(lam)*position2[1][antenna2] + \
                                     np.cos(phi)*np.cos(lam)*position2[2][antenna2] + \
                                     padPosition2[0][ant2index])
              itrf_correction2.append(+np.cos(lam)*position2[0][antenna2] \
                                     -np.sin(phi)*np.sin(lam)*position2[1][antenna2] + \
                                     np.cos(phi)*np.sin(lam)*position2[2][antenna2] + \
                                     padPosition2[1][ant2index])
              itrf_correction2.append(+np.cos(phi)*position2[1][antenna2] + \
                                   np.sin(phi)*position2[2][antenna2]+ padPosition2[2][ant2index])
          
      if (ant == '' or names[antenna] == ant):
        print "Antenna %02d = %4s on %s: " % (antenna,names[antenna],station[antindex])
        for component in range(3):
            if (itrf):
                comp = itrf_correction[component]
            else:
                comp = position[component][antenna]
            if (vis2 == None):
                print "%s: %+.6f" % (axis[component], comp)
            elif (names[antenna] in names2):
                if (itrf):
                    comp2 = itrf_correction2[component]
                else:
                    index2 = list(names2).index(names[antenna])
                    antenna2 = list(names2).index(names[antenna])
                    vis2stationId = stationId2[antenna2]
                    ant2index = list(padStationId2).index(vis2stationId)
                    comp2 = position2[component][index2] 
                if (component == 0):
                    print "%s: %+.6f   %+.6f    %+.6f  (%s on %s)" % (axis[component],comp,comp2,comp-comp2,names2[antenna2],station2[ant2index])
                else:
                    print "%s: %+.6f   %+.6f    %+.6f" % (axis[component],comp,comp2,comp-comp2)
            else:
                print "%s: %+.6f" % (axis[component],comp)
    if (vis2 != None):
        for antenna in range(len(names2)):
          if (itrf):
              # compute antenna XYZ from pad XYZ and ENU correction
              # http://en.wikipedia.org/wiki/Geodetic_system   (ENU to ECEF)
              antindex = list(padStationId2).index(stationId2[antenna])
              itrf_correction2 = []
              phi = lat*math.pi/180.
              lam = math.atan2(padPosition2[1][antindex],padPosition2[0][antindex])
              itrf_correction2.append(-np.sin(lam)*position2[0][antenna] \
                                     -np.sin(phi)*np.cos(lam)*position2[1][antenna] + \
                                     np.cos(phi)*np.cos(lam)*position2[2][antenna] + \
                                     padPosition2[0][antindex])
              itrf_correction2.append(+np.cos(lam)*position2[0][antenna] \
                                     -np.sin(phi)*np.sin(lam)*position2[1][antenna] + \
                                     np.cos(phi)*np.sin(lam)*position2[2][antenna] + \
                                     padPosition2[1][antindex])
              itrf_correction2.append(+np.cos(phi)*position2[1][antenna] + \
                                     np.sin(phi)*position2[2][antenna]+ padPosition2[2][antindex])
          
          if (ant == '' or names2[antenna] == ant):
            if (names2[antenna] not in names):
                print     "                  Antenna %02d = %4s on %s: " % (antenna,names2[antenna],station2[antenna])
                for component in range(3):
                    if (itrf):
                        comp2 = itrf_correction2[component]
                        print "%s: ----------------  %+.6f" % (axis[component],comp2)
                    else:
                        comp2 = position2[component][antenna]
                        print "%s: ----------  %+.6f" % (axis[component],comp2)
            

def padPositionASDM(vis, vis2=None, ant=''):
    """
    This utility prints the pad positions from the ASDM_STATION table for 1 or 2 ms's.
    If vis2 is specified, the difference in positions is also computed.
    This is meant to help understand antenna position correction algorithms.
    Specifying ant will limit the display to one antenna.
    Todd Hunter  (May 2012)
    """
    try:
        antTable = vis+'/ASDM_ANTENNA'
        tb.open(antTable)
    except:
        print "Could not open ASDM_ANTENNA table: %s" % (antTable)
        return([],[])
#    position = tb.getcol('position')
    names = tb.getcol('name')
    tb.close()
    try:
        antTable = vis+'/ASDM_STATION'
        tb.open(antTable)
    except:
        print "Could not open ASDM_STATION table: %s" % (antTable)
        return([],[])
    station = tb.getcol('name')
    position = tb.getcol('position')
    tb.close()

    if (vis2 != None):
      try:
          antTable2 = vis2+'/ASDM_ANTENNA'
          tb.open(antTable2)
      except:
          print "Could not open ASDM_ANTENNA table: %s" % (antTable2)
          return([],[])
#      position2 = tb.getcol('position')
      names2 = tb.getcol('name')
      tb.close()
      try:
          antTable2 = vis2+'/ASDM_STATION'
          tb.open(antTable2)
      except:
          print "Could not open ASDM_STATION table: %s" % (antTable2)
          return([],[])
      station2 = tb.getcol('name')
      position2 = tb.getcol('position')

    antennas = len(station)
    axis = ['X','Y','Z']
    if (vis2 == None):
        print "       vis1"
    else:
        print "       vis1        vis2        Difference (m)  (vis1-vis2)"
    for antenna in range(len(names)):
      if (ant == '' or names[antenna] == ant):
        print "Antenna %02d = %4s on %s: " % (antenna,names[antenna],station[antenna])
        for component in range(3):
            comp = position[component][antenna] 
            if (vis2 == None):
                print "  %s: %+.6f" % (axis[component],comp)
            elif (names[antenna] in names2):
                index2 = list(names2).index(names[antenna])
                comp2 = position2[component][index2] 
                if (component == 0):
                    print "  %s: %+.6f   %+.6f    %+.6f  (%s on %s)" % (axis[component],comp,comp2,comp-comp2,names2[index2],station2[index2])
                else:
                    print "  %s: %+.6f   %+.6f    %+.6f" % (axis[component],comp,comp2,comp-comp2)
            else:
                print "  %s: %+.6f" % (axis[component],comp)
    if (vis2 != None):
        for antenna in range(len(names2)):
          if (ant == '' or names2[antenna] == ant):
            if (names2[antenna] not in names):
                print     "                  Antenna %02d = %4s on %s: " % (antenna,names2[antenna],station2[antenna])
                for component in range(3):
                    comp2 = position2[component][antenna] 
                    print "  %s: ----------  %+.6f" % (axis[component],comp2)
            

    

def smoothbandpass(caltable='',window_len=20, window='flat', method='ri' ,
                   avoidflags=True, verbose=False, fullVerbose=False,
                   help=False, log='', outputname=None):
  """
  Unless the outputname is specified, the output table will have
  '_smoothXXwindow_method' appended to the name,
  where 'XX'=window_len and 'window' is the window type.  The window 
  type options are: 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'.
  Method types are 'ri' = real & imaginary or 'ap' = amp & phase.
  The input bandpass file is in caltable. 
  For further help and examples, see http://casaguides.nrao.edu/index.php?title=Smoothbandpass
  -- Ed Fomalont & Todd Hunter, Sep 2011
  """
  if (help):
      print "Usage: smoothbandpass(caltable, window_len=20, window='flat', method='ri',"
      print "           avoidflags=True, verbose=False, fullVerbose=False, help=False, log='')"
      print "Unless the outputname is specified, the output table will have"
      print "  '_smoothXXwindow_method' appended to the name,"
      print "where 'XX'=window_len and 'window' is the window type.  The window "
      print "type options are: 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'."
      print "Method types are 'ri' = real & imaginary or 'ap' = amp & phase."
      return
#  Set some defaults
  ggr = 0
  ggr = 0

  tb.open(caltable)
  if (outputname == None):
      smooth_table = caltable+'_smooth%d%s_%s' % (window_len,window,method)
  else:
      smooth_table = outputname
  if ('GAIN' not in tb.colnames()):
      print "This is a new-style caltable.  You should not use smoothbandpass, but instead use"
      print "the new on-the-fly channel averaging of the casa 3.4 command bandpass()."
      return
  g=tb.getcol('GAIN')
  flags=tb.getcol('FLAG')
  antennas=tb.getcol('ANTENNA1')
  cal_desc_id=tb.getcol('CAL_DESC_ID')
  gsm=g
  p=g.shape
  l1 = p[0]  # polarizations
  l2 = p[2]  # spw+antenna combinations   (size=nspw * nant), i.e. nrows in table
  ld = p[1]  # channels
  chan = range(0,ld)
  if (log != ''):
      f = open(log,'w')
  for i in range (0,l1):
    #   essentially smooth each pol/spw
    for j in range (0,l2):
        dcompl = g[i,:,j]
        flag = flags[i,:,j]
        dreal = np.real(dcompl)
        dimag = np.imag(dcompl)
        if (avoidflags):
          # Now replace each flagged channel's value with nearest unflagged value in this spw
          channelsToReplace = np.where(flag == 1)[0]
          goodChannels = np.where(flag == 0)[0]
          channelsReplaced = 0
          if (len(goodChannels) > 0 and len(channelsToReplace)>0):
            # only examine spws that are not completely flagged and not completely unflagged
            for mychan in range(len(channelsToReplace)):
               # find the closest unflagged channel
               closestDistance = 1000000
               for good in range(len(goodChannels)):
                   distance = abs(channelsToReplace[mychan]-goodChannels[good])
                   if (distance < closestDistance):
                       closestGoodChannel = good
                       closestDistance = distance
               dreal[channelsToReplace[mychan]] = dreal[goodChannels[closestGoodChannel]]
               dimag[channelsToReplace[mychan]] = dimag[goodChannels[closestGoodChannel]]
               if (fullVerbose):
                   print "row=%d, pol=%d, spw=%d, Ant%d: avoiding flagged channel %d with good channel %d" % (j,i,
                          cal_desc_id[j],antennas[j],channelsToReplace[mychan],goodChannels[closestGoodChannel])
               if (log != ''):
                   f.write("row=%d, pol=%d, spw=%d, Ant%d: avoiding flagged channel %d with good channel %d\n" % (j,i,
                            cal_desc_id[j],antennas[j],channelsToReplace[mychan],goodChannels[closestGoodChannel]))
               channelsReplaced += 1
          if (channelsReplaced > 0 and (verbose or fullVerbose)):
              print "Avoided %d flagged channels on pol=%d, spw=%d, antenna=%d" % (channelsReplaced,i,cal_desc_id[j],antennas[j])
        #   window type and window_len will be options
        if (method == 'ri'):
            ggr = smooth(dreal, window_len, window)
            ggi = smooth(dimag, window_len, window)
            for k in range (0,ld):
                gsm[i,k,j] = np.complex(ggr[k],ggi[k])
        elif (method == 'ap'):
            phase = np.arctan2(dimag, dreal)
            amp = np.abs(dcompl)
            ampSmooth = smooth(amp, window_len, window)
            phaseSmooth = smooth(phase, window_len, window)
            dreal = ampSmooth * np.cos(phaseSmooth)
            dimag = ampSmooth * np.sin(phaseSmooth)
            for k in range (0,ld):
                gsm[i,k,j] = np.complex(dreal[k],dimag[k])
        else:
            print "Unrecognized method = ", method
            return

#        print 'finished ' , i, j, ' stream'
    #
  #
  if (log != ''):
      f.close()
  tb.close()
#   Overwrite smoothed bandpass into already defined output.
  os.system('cp -r %s %s'%(caltable,smooth_table))
  tb.open(smooth_table, nomodify=False)
  tb.putcol('GAIN', gsm)
  tb.close()
  print "Smooth solution written to: %s" % (smooth_table)
  return

def getScansForIntentFast(vm,uniqueScans,intent):
  """
  This is a faster version of the one in ValueMapping.
  - Todd Hunter
  """
  onsourceScans = []
  for s in uniqueScans:
    scanIntents = vm.getIntentsForScan(s)
    if (intent in scanIntents):
        onsourceScans.append(s)
  return(onsourceScans)

def lstRange(vis, verbose=True, vm=''):
  """
  Compute the LST of start and end of the specified ms.
  For further help and examples, see http://casaguides.nrao.edu/index.php?title=Lstrange
  -- Todd Hunter
  """
  return(lstrange(vis,verbose=verbose,vm=vm))
    
def lstrange(vis, verbose=True, vm=''):
  """
  Compute the LST of start and end of the specified ms.
  For further help and examples, see http://casaguides.nrao.edu/index.php?title=Lstrange
  -- Todd Hunter
  """
  if (os.path.exists('%s/table.dat'%vis)==False):
      print "Could not find %s/table.dat, are you sure this is an ms?" % (vis)
      return
  try:
        antTable = vis+'/OBSERVATION'
        tb.open(antTable)
        observatory = tb.getcell('TELESCOPE_NAME')
#        print "Found observatory = %s" % observatory
        tb.close()
  except:
        observatory = 'ALMA'
        print "Could not open OBSERVATION table to get the telescope name: %s" % (antTable)
        print "Assuming ", observatory

  if (vm == ''):
      print "Running ValueMapping... (this may take a minute)"
      vm = ValueMapping(vis)
  uniqueScans = np.unique(vm.scans)

# This way is really slow:
#  onsourceScans = np.unique(vm.getScansForIntent('OBSERVE_TARGET#ON_SOURCE'))
  if (verbose):
      print "Checking intents for each scan..."
  onsourceScans = getScansForIntentFast(vm,uniqueScans,'OBSERVE_TARGET#ON_SOURCE')
  if (len(onsourceScans) < 1):
      # "old school" format
      onsourceScans = getScansForIntentFast(vm,uniqueScans,'OBSERVE_TARGET.ON_SOURCE')
  if (verbose):
        print "OBSERVE_TARGET scans = ", onsourceScans
  i = 0
  wikiline = ''
  wikiline2 = ''
  wikiline3 = 'no onsource time | '
  # First, examine the list of all scans (where i will be 0), then 
  # the list of the on-target scans (where i will be 1).
  for scans in [uniqueScans,onsourceScans]:
   if (len(scans) > 0):
    times = vm.getTimesForScans(scans)
    mjdsecmin = 1e12
    mjdsecmax = 0
    for t in times:
#  This is too slow:
#        mjdsecmin = np.amin([np.amin(t),mjdsecmin])
#        mjdsecmax = np.amax([np.amax(t),mjdsecmax])
#  Assume the times are in ascending order:
        mjdsecmin = np.amin([t[0],mjdsecmin])
        mjdsecmax = np.amax([t[-1],mjdsecmax])
    LST = np.zeros(2)
    [latitude, longitude, obs] = getObservatoryLatLong(observatory)
    LST[0] = ComputeLST(mjdsecmin, longitude)
    LST[1] = ComputeLST(mjdsecmax, longitude)
    if (i == 1):
        style = "on source"
    else:
        style = "of whole SB"
    duration = LST[1]-LST[0]
    duration2 = LST[1]+24-LST[0]
    if (duration2 < duration):
        duration = duration2
    if (verbose):
        print "LST range %s = %.2f to %.2f = %02d:%02d to %02d:%02d (%.1f minutes)" % (style,LST[0],LST[1],
       np.floor(LST[0]),np.floor(60*(LST[0]-np.floor(LST[0]))), np.floor(LST[1]),
       np.floor(60*(LST[1]-np.floor(LST[1]))), duration*60)
    [mjdmin,utmin] = mjdSecondsToMJDandUT(mjdsecmin)
    [mjdmax,utmax] = mjdSecondsToMJDandUT(mjdsecmax)
    if (i==0):
        clockTimeMinutes = (mjdmax-mjdmin)*1440.
    if (verbose):
        print "MJD range %s = %.4f to %.4f" % (style, mjdmin, mjdmax)
        print " UT range %s = %s to %s" % (style, utmin, utmax)
    tb.open(vis+'/OBSERVATION')
    sched = tb.getcol('SCHEDULE')
    sbname = '%s' % (sched[0][0].split()[1])  # This is the SB UID.
    exec_uid = '%s' % (sched[1][0].split()[1])
    tb.close()
    if (i==0):
       wikiline2 += "| %s | %s | %s | %s-%s | %02d:%02d-%02d:%02d | %.1f | " % (utmin[0:10],sbname,exec_uid,utmin[10:-6],utmax[11:-6],np.floor(LST[0]),np.floor(60*(LST[0]-np.floor(LST[0]))), np.floor(LST[1]), np.floor(60*(LST[1]-np.floor(LST[1]))), clockTimeMinutes)
       csvline = "%s,%s,%.2f" % (sbname, exec_uid, clockTimeMinutes)
       wikiline += "%s-%s | %02d:%02d-%02d:%02d | %.1f |" % (utmin[0:-6],utmax[11:-6],np.floor(LST[0]),np.floor(60*(LST[0]-np.floor(LST[0]))), np.floor(LST[1]), np.floor(60*(LST[1]-np.floor(LST[1]))),clockTimeMinutes)
    else:
      # print out elevation range for mjdsecmin to mjdsecmax
      # could use TsysExplorer(vis) but it fails at line 1144
      tb.open("%s/POINTING" % vis)
      azel = 1
      try:
        elevation = np.transpose(tb.getcol("DIRECTION")[azel])
        elevTime  = tb.getcol("TIME")
        tb.close()
        t = vm.getTimesForScans(scans[0])[0]
        matches1 = np.where(elevTime > np.min(t[0]))[0]
        matches2 = np.where(elevTime < np.max(t[-1]))[0]
        matches = np.intersect1d(matches1,matches2)
        startElev = elevation[matches[0]]*180/math.pi
        t = vm.getTimesForScans(scans[-1])[0]
        matches1 = np.where(elevTime > np.min(t[0]))[0]
        matches2 = np.where(elevTime < np.max(t[-1]))[0]
        matches = np.intersect1d(matches1,matches2)
        stopElev = elevation[matches[-1]]*180/math.pi
        if (verbose):
            print "Elevation range on OBSERVE_TARGET scans = %.1f-%.1f" % (startElev,stopElev)
        wikiline += "%02d:%02d-%02d:%02d | %.0f-%.0f | " % (np.floor(LST[0]),np.floor(60*(LST[0]-np.floor(LST[0]))), np.floor(LST[1]), np.floor(60*(LST[1]-np.floor(LST[1]))), startElev, stopElev)
        wikiline3 = "%.0f-%.0f | " % (startElev,stopElev)
      except:
        wikiline3 = "pointing table empty | "
        if (verbose):
            print "The pointing table appears to be empty.  Was it deleted because this is a mosaic?"
        wikiline += "%02d:%02d-%02d:%02d | " % (np.floor(LST[0]),np.floor(60*(LST[0]-np.floor(LST[0]))), np.floor(LST[1]), np.floor(60*(LST[1]-np.floor(LST[1]))))
    i += 1
  if (verbose):
      print "wikiline = %s" % (wikiline)
  csvline = 'csvline = %s' % (csvline)
  return (wikiline2, wikiline3,clockTimeMinutes,csvline)
  # end of lstrange()

def findNearestField(ra,dec,raAverageDegrees, decAverageDegrees):
    """
    Finds the field in a list that is nearest to the specified average position.
    ra and dec must be equal-sized lists.
    raAvergeDegrees and decAverageDegrees must be scalar values.
    -- Todd Hunter
    """
    nearestField = -1
    smallestSeparation = 1e15
    for i in range(len(ra)):
        separation = angularSeparation(ra[i],dec[i],raAverageDegrees,decAverageDegrees)
        if (separation < smallestSeparation):
            smallestSeparation = separation
            nearestField = i
    return([nearestField,smallestSeparation])

def plotMosaic(vis='',sourceid='',figfile='', coord='relative', skipsource=-1,
               doplot=True, help=False):
  """
  Produce a plot of the pointings with the primary beam FWHM and field names.
  For further help and examples, see http://casaguides.nrao.edu/index.php?title=Plotmosaic
  -- Todd Hunter
  """
  return plotmosaic(vis,sourceid,figfile,coord,skipsource,doplot,help)

def plotmosaic(vis='',sourceid='',figfile='', coord='relative', skipsource=-1,
               doplot=True, help=False):
  """
  Produce a plot of the pointings with the primary beam FWHM and field names.
  For further help and examples, see http://casaguides.nrao.edu/index.php?title=Plotmosaic
  -- Todd Hunter
  """
  if (help):
      print "Usage: plotmosaic(vis, sourceid='', figfile='', coord='relative',"
      print "                  skipsource=-1, doplot=True, help=False)"
      print "  The sourceid may be either an integer, integer string, or the string name"
      print "     of the source (but it cannot be a list)."
      print "  If doplot=False, then the central field ID is returned as an integer. "
      print "     Otherwise, a list is returned: "
      print "         [central field,  maxRA, minRA, minDec, maxDec]"
      print "     where the angles are in units of arcsec relative to the center."
      print "  If coord='absolute', then nothing is returned."
      return

  # open the ms table
  if (coord.find('abs')<0 and coord.find('rel')<0):
      print "Invalid option for coord, must be either 'rel'ative or 'abs'olute."
      return
  try:
      fieldTable = vis+'/FIELD'
      tb.open(fieldTable)
  except:
      print "Could not open table = %s" % fieldTable 
      return
  delayDir = tb.getcol('DELAY_DIR')
  sourceID = tb.getcol('SOURCE_ID')
  name = tb.getcol('NAME')
  if (type(sourceid) == str):
      try:
          sourceid = int(sourceid)
      except:
          # need to convert name to id
          matches=np.where(name==sourceid)[0]
          srcs=np.unique(sourceID[matches])
          nsrcs=len(srcs)
          if (nsrcs>1):
              print "More than one source ID matches this name: ",sourceID
              print "Try again using one of these."
              return
          elif (nsrcs==0):
              if (sourceid != ''):
                  print "No sources match this name = %s" % sourceid
                  print "Available sources = ",np.unique(name)
                  return
              else:
                  print "No source specified in the second argument, so plotting all sources."
          else:
              sourceid = srcs[0]
  sourcename = name[sourceid]
  fields = np.array(range(len(sourceID)))
  if (sourceid != ''):
      matches = np.where(sourceID == int(sourceid))[0]
      fields = fields[matches]
      matches = np.where(fields != skipsource)[0]
      fields = fields[matches]
      if (len(fields) < 1):
          print "No fields contain source ID = ", sourceid
          return
      print "Field IDs with matching source ID = ", fields
  name = tb.getcol('NAME')
  tb.close()
  try:
      antennaTable = vis+'/ANTENNA'
      tb.open(antennaTable)
  except:
      print "Could not open table = %s" % antennaTable 
      return
  dishDiameter = np.unique(tb.getcol('DISH_DIAMETER'))
  tb.close()
  try:
      spwTable = vis+'/SPECTRAL_WINDOW'
      tb.open(spwTable)
      num_chan = tb.getcol('NUM_CHAN')
      refFreqs = tb.getcol('REF_FREQUENCY')
      tb.close()
  except:
      print "Could not open table = %s" % antennaTable 
      print "Will not print primary beam circles"
      titleString = vis.split('/')[-1]
      dishDiameter =  [0]
  [latitude,longitude,obs] = getObservatoryLatLong('ALMA') 
  print "Got longitude = %.3f deg" % (longitude)

# once Stuartt adds this feature, I can use it
#  spws = vm.getSpwsForIntent('OBSERVE_TARGET#ON_SOURCE')
#  matches = np.intersect1d(spws,matches)
  if (3840 in num_chan):
      matches = np.where(num_chan == 3840)[0]
  else:
      matches = np.where(num_chan > 4)[0]  # this kills the WVR and channel averaged data
  meanRefFreq = np.mean(refFreqs[matches])
  print "Mean frequency = %f GHz" % (meanRefFreq*1e-9)
  lambdaMeters = c_mks / meanRefFreq
  ra = delayDir[0,:][0]*12/math.pi
  dec = delayDir[1,:][0]*180/math.pi
  ra *= 15
  raAverageDegrees = np.mean(ra[fields])
  decAverageDegrees = np.mean(dec[fields])
  cosdec = 1.0/cos(decAverageDegrees*np.pi/180)

  # Here we scale by cos(dec) to make then pointing pattern in angle on sky
  raRelativeArcsec = 3600*(ra - raAverageDegrees)*cos(decAverageDegrees*math.pi/180.)
  decRelativeArcsec = 3600*(dec - decAverageDegrees)

  markersize = 4
  print "Found %d pointings" % (shape(ra)[0])
  [centralField,smallestSeparation] = findNearestField(ra[fields],dec[fields],
                                          raAverageDegrees, decAverageDegrees)
  # This next step is crucial, as it converts from the field number 
  # determined from a subset list back to the full list.
  centralField = fields[centralField]
  
  print "Field %d is closest to the center of the area covered (%.1f arcsec away)." % (centralField,smallestSeparation*3600)
  if (doplot==False):
      return(centralField)
  pb.clf()
  desc = pb.subplot(111)
  if (coord.find('abs')>=0):
    raunit = 'deg'  # nothing else is supported (yet)
    desc = pb.subplot(111,aspect=cosdec)
    # SHOW ABSOLUTE COORDINATES
    pb.plot(ra[fields],dec[fields],"k+",markersize=markersize)
    for j in dishDiameter:
      for i in range(len(ra)):
          if (i in fields):
              if (j > 0):
                  arcsec = 0.5*primaryBeamArcsec(wavelength=lambdaMeters*1000,diameter=j)
                  radius = arcsec/3600.0
                  cir = matplotlib.patches.Ellipse((ra[i], dec[i]), width=2*radius*cosdec,
                                                   height=2*radius, facecolor='none', edgecolor='b',
                                                   linestyle='dotted')
                  pb.gca().add_patch(cir)
#                  cir = pb.Circle((ra[i], dec[i]), radius=radius, facecolor='none', edgecolor='b', linestyle='dotted')
      titleString = vis.split('/')[-1]+', %s, average freq. = %.1f GHz, beam = %.1f"'%(sourcename,meanRefFreq*1e-9,2*arcsec)
    resizeFonts(desc, 10)
    if (raunit.find('deg') >= 0):
        pb.xlabel('Right Ascension (deg)')
    else:
        pb.xlabel('Right Ascension (hour)')
    pb.ylabel('Declination (deg)')
    raRange = np.max(ra[fields])-np.min(ra[fields])
    decRange = np.max(dec[fields])-np.min(dec[fields])
    x0 = np.max(ra[fields]) + 1.2*radius*cosdec
    x1 = np.min(ra[fields]) - 1.2*radius*cosdec
    y1 = np.max(dec[fields]) + 1.2*radius
    y0 = np.min(dec[fields]) - 1.2*radius
    pb.xlim([x0,x1])
    pb.ylim([y0,y1])
    pb.title(titleString,size=10)
    for i in range(len(ra)):
      if (i in fields):
        pb.text(ra[i]-0.02*raRange, dec[i]+0.02*decRange, str(i),fontsize=12, color='k')
  elif (coord.find('rel')>=0):
    # SHOW RELATIVE COORDINATES
    pb.plot(raRelativeArcsec[fields], decRelativeArcsec[fields], 'k+', markersize=markersize)
    for j in dishDiameter:
        for i in range(len(ra)):
            if (i in fields):
                if (j > 0):
                    arcsec = 0.5*1.18*lambdaMeters*3600*180/j/math.pi
                    radius = arcsec
                    cir = pb.Circle((raRelativeArcsec[i], decRelativeArcsec[i]),
                                    radius=radius, facecolor='none', edgecolor='b', linestyle='dotted')
                    pb.gca().add_patch(cir)
    titleString = vis.split('/')[-1]+', %s, average freq. = %.1f GHz, beam = %.1f"'%(sourcename,meanRefFreq*1e-9,2*arcsec)
    resizeFonts(desc, 10)
    pb.xlabel('Right ascension offset (arcsec)')
    pb.ylabel('Declination offset (arcsec)')
    pb.title(titleString,size=10)
    raRange = np.max(raRelativeArcsec[fields])-np.min(raRelativeArcsec[fields])
    decRange = np.max(decRelativeArcsec[fields])-np.min(decRelativeArcsec[fields])
    for i in range(len(ra)):
      if (i in fields):
        pb.text(raRelativeArcsec[i]-0.02*raRange, decRelativeArcsec[i]+0.02*decRange, str(i),fontsize=12, color='k')
    x0 = np.max(raRelativeArcsec[fields]) + 1.2*radius # 0.25*raRange
    x1 = np.min(raRelativeArcsec[fields]) - 1.2*radius # - 0.25*raRange
    y1 = np.max(decRelativeArcsec[fields]) + 1.2*radius # 0.25*decRange
    y0 = np.min(decRelativeArcsec[fields]) - 1.2*radius # 0.25*decRange
    mosaicInfo = []
    mosaicInfo.append(centralField)
    mosaicInfo.append(np.max(raRelativeArcsec[fields]) + 2*radius)
    mosaicInfo.append(np.min(raRelativeArcsec[fields]) - 2*radius)
    mosaicInfo.append(np.max(decRelativeArcsec[fields]) + 2*radius)
    mosaicInfo.append(np.min(decRelativeArcsec[fields]) - 2*radius)
    pb.xlim(x0,x1)
    pb.ylim(y0,y1)
    pb.axis('equal')
  else:
    print "Invalid option for coord, must be either 'rel'ative or 'abs'olute."
    return

  yFormatter = ScalarFormatter(useOffset=False)
  desc.yaxis.set_major_formatter(yFormatter)
  desc.xaxis.set_major_formatter(yFormatter)
  desc.xaxis.grid(True,which='major')
  desc.yaxis.grid(True,which='major')
  pb.draw()
  autoFigureName = "%s.pointings.%s.png" % (vis,coord)
  if (figfile==True):
      try:
        pb.savefig(autoFigureName)
        print "Wrote file = %s" % (autoFigureName)
      except:
        print "WARNING:  Could not save plot file.  Do you have write permission here?"
  elif (len(figfile) > 0):
      try:
        pb.savefig(figfile)
        print "Wrote file = %s" % (figfile)
      except:
        print "WARNING:  Could not save plot file.  Do you have write permission here?"
  else:
        print "To save a plot, re-run with either:"
        print "  plotmosaic('%s',figfile=True) to produce the automatic name=%s" % (vis,autoFigureName)
        print "  plotmosaic('%s',figfile='myname.png')" % (vis)
  if (coord.find('rel')>=0):
      return mosaicInfo
  else:
      return

def plotconfig(telescope='',config='',figfile='',list=False):
    """
    Plots pad layout for specified telescope and configuration name. It also returns an
    array containing the baseline lengths, sorted from shortest to longest.
    For further help and examples, see http://casaguides.nrao.edu/index.php?title=Plotconfig
    -- Todd Hunter
    Telescopes and configurations available to plot (this list may be outdated):"
    aca: i, ns, tp
    alma: cycle0.compact, cycle0.extended, 1..28
    carma: a, b, c, d, e
    meerkat
    pdbi: a, b, c, d
    sma: subcompact, compact, compact.n, extended, vextended
    vla: a, bna, b, cnb, c, dnc, d
    WSRT
    telescope='file' option: specify any filename as config='myfilename'
    """
    z = None
    if (telescope == 'wsrt'):
        telescope = telescope.upper()
    else:
        telescope = telescope.lower()
        
    if (len(str(config)) < 1 and telescope.find('meerkat')<0 and telescope.find('WSRT')<0):
        print "Telescope and configuration available to plot:"
        print "aca: i, ns, tp"
        print "alma: cycle0.compact, cycle0.extended, 1..28"
        print "carma: a, b, c, d, e"
        print "meerkat"
        print "pdbi: a, b, c, d"
        print "sma: subcompact, compact, compact.n, extended, vextended"
        print "vla: a, bna, b, cnb, c, dnc, d"
        print "WSRT"
        print "file: specify any filename as config='myfilename'"
        return
    configString = str(config)
    if (len(str(config)) > 0):
        if (telescope.find('alma')<0 or telescope.find('cycle')):
            if (telescope.find('pdbi')<0):
                configString = '.'+str(config)
            else:
                configString = '-'+str(config)

    repotable=os.getenv("CASAPATH").split()[0]+"/data/alma/simmos/"
    if (telescope == 'alma' and str.isdigit(str(config))):
        myfile = repotable+telescope+'.out%02d'%int(config)+".cfg"
    else:
        myfile = repotable+telescope+configString+".cfg"
    if (telescope == 'file'):
        myfile = config
    try:
        fd = open(myfile,'r')
    except:
        print "Could not open file = ", myfile
        return
    columns = []
    line = fd.readline()
    while (len(line) > 0):
        if (line.find('#')>=0):
            mystr = line.split('=')
            if (len(mystr) > 1):
                mystr = mystr[1].split('\n')[0]
                if (line.find('observatory')>=0):
                    telescope = mystr
                elif (line.find('coordsys')>=0):
                    coordsys = mystr
                elif (line.find('datum')>=0):
                    datum = mystr
                elif (line.find('zone')>=0):
                    zone = int(mystr)
                elif (line.find('hemisphere')>=0):
                    nors = mystr
        else:
            newcolumns = line.split()
            if (len(newcolumns) > 2):
                if (str.isdigit(newcolumns[-1].replace(".","",1))):
                    columns.append(newcolumns)
                else:
                    columns.append(newcolumns[0:len(newcolumns)-1])
        line=fd.readline()
    print "Read %d pads for observatory=%s in coordsys=%s" % (len(columns), telescope,coordsys)
    if (coordsys.find('LOC')>=0):
        x = np.array(columns).transpose()[0].astype('float')
        y = np.array(columns).transpose()[1].astype('float')
        z = np.array(columns).transpose()[2].astype('float')
        diameter = np.array(columns).transpose()[3].astype('float')
    else:
      position = np.array(columns).transpose().astype('float')
      diameter = np.array(columns).transpose()[3].astype('float')
      u = simutil.simutil()
      repotable=os.getenv("CASAPATH").split()[0]+"/data/geodetic/Observatories"
      tb.open(repotable)
      Name = tb.getcol('Name')
      cx = position[0][0]
      cy = position[1][0]
      cz = position[2][0]
      myType = ''
      for i in range(len(Name)):
          if (Name[i] == telescope):
              Long = tb.getcell('Long',i)
              Lat = tb.getcell('Lat',i)
              Height = tb.getcell('Height',i)
              myType = tb.getcell('Type',i)
              if (myType == 'ITRF'):
                  cx = tb.getcell('X',i)
                  cy = tb.getcell('Y',i)
                  cz = tb.getcell('Z',i)
              else:
                  # WGS84
                  print "COFA in long/lat/height = ", Long, Lat, Height
                  output = u.long2xyz(Long*math.pi/180.,Lat*math.pi/180.,Height,myType)
                  (cx,cy,cz) = output
                  if (coordsys.find('UTM')>=0):
                      (position[0],position[1],position[2]) = u.utm2xyz(position[0,:], position[1,:],position[2,:],zone,datum,nors)
              print "COFA in x/y/z = ", cx,cy,cz
              break
      if (len(myType) < 1):
        print "Did not find telescope data, using first station as center of array."
      tb.close()
      try:
         (x,y) = output = u.irtf2loc(position[0,:],position[1,:],position[2,:],cx,cy,cz)
      except:
         (x,y) = output = u.itrf2loc(position[0,:],position[1,:],position[2,:],cx,cy,cz)
    pb.clf()
    uniqueDiameters = np.unique(diameter)
    adesc = pb.subplot(111)
    pb.hold(True)
    for d in uniqueDiameters:
        matches = np.where(d == diameter)[0]
        pb.plot(x[matches],y[matches],'o')
    pb.hold(False)
    pb.title('%s %s' % (telescope,config))
    pb.xlabel('X (m)')
    pb.ylabel('Y (m)')
    pb.axis('equal')
    lengths = []
    for i in range(len(x)):
        for j in range(i,len(x)):
            if (i != j):
                if (z==None):
                  lengths.append(((x[i]-x[j])**2+(y[i]-y[j])**2)**0.5)
                else:
                  lengths.append(((x[i]-x[j])**2+(y[i]-y[j])**2+(z[i]-z[j])**2)**0.5)
    lengths = np.sort(lengths)
    if (list):
        for i in range(len(lengths)):
            print "%.1f m" % (lengths[i])
    if (1==1):
        yFormatter = matplotlib.ticker.ScalarFormatter(useOffset=False)
        adesc.yaxis.set_major_formatter(yFormatter)
        adesc.yaxis.grid(True,which='major')
        xFormatter = matplotlib.ticker.ScalarFormatter(useOffset=False)
        adesc.xaxis.set_major_formatter(xFormatter)
        adesc.xaxis.grid(True,which='major')
    pb.draw()
    autoFigureName = "%s.%s.png" % (telescope,config)
    if (figfile == True):
        pb.savefig(autoFigureName)
        print "Wrote file = %s" % (autoFigureName)
    elif (len(figfile) > 0):
        pb.savefig(figfile)
        print "Wrote file = %s" % (figfile)
    else:
        print "To save a plot, re-run with either:"
        print "  plotconfig('%s','%s',figfile=True) to produce the automatic name=%s" % (telescope,config,autoFigureName)
        print "  plotconfig('%s','%s',figfile='myname.png')" % (telescope,config)
    print "min/median/rms/max = %.2f / %.2f / %.2f / %.2f m" % (np.min(lengths),np.median(lengths),np.sqrt(np.mean(lengths**2)),np.max(lengths))
    return(lengths)

class stuffForScienceDataReduction():
    def locatePath(self, pathEnding):
        # This method will locate any file in the active analysisUtils
        # "science" subdirectory tree. You need to call it with the
        # trailing part of the path beyond the ~/AIV/science/ directory.
        # e.g. 'PadData/almaAntPos.txt'
        #
#        print "Looking for %s in the analysisUtils area" % (pathEnding)
        tokens = __file__.split('/')
        mypath = ''
        for i in range(len(tokens)-len(pathEnding.split('/'))):
            mypath += tokens[i] + '/'
        mypath += pathEnding
        return(mypath)
        
    def correctMyAntennaPositions(self, msName, obsTime='', verbose=True):

        #casaCmd = 'print "# Correcting for antenna position errors."\n\n'
        casaCmd = ''

        tb.open(msName)
        if 'ASDM_ANTENNA' not in tb.keywordnames() or 'ASDM_STATION' not in tb.keywordnames():
            sys.exit('ERROR: The ANTENNA and/or STATION tables from the ASDM were not transferred to the MS.')
        tb.close()
        try:
            f=open(os.path.expanduser('~/AIV/science/PadData/almaAntPos.txt'), 'r')
        except:
            try:
                mypath = self.locatePath('PadData/almaAntPos.txt')
                f=open(mypath)
            except:
                print "Failed to find path=%s" % (mypath)
            
        almaAntPos=XmlObjectifier.XmlObject(f.read())
        f.close()

        try:
            g=open(os.path.expanduser('~/AIV/science/PadData/antennaMoves.txt'), 'r')
        except:
            try:
                mypath = self.locatePath('PadData/antennaMoves.txt')
                g=open(mypath)
            except:
                print "Failed to find path=%s" % (mypath)
            
        gc=g.read().splitlines()
        g.close()

        gc=sorted(gc)

        ij=-1
        antMoves={}
        for line in gc:
            line=line.strip()
            if (len(line) < 1): continue  # added by T. Hunter
            if line[0] == '#': continue
            ele=line.split()
            ij=ij+1
            antMoves[ij]={}
            antMoves[ij]['Date']=datetime.datetime.strptime(ele[0], '%Y-%m-%dT%H:%M')
            antMoves[ij]['Antenna']=ele[1]
            antMoves[ij]['From']=ele[2]
            antMoves[ij]['To']=ele[3]

        baseRunObj = almaAntPos.AntennaPositions.BaselineRun
        if not isinstance(baseRunObj, types.ListType): baseRunObj = [baseRunObj]

        measTime=[]
        for i in range(len(baseRunObj)):
           baseRunObj1 = baseRunObj[i]
           measTime.append(datetime.datetime.strptime(baseRunObj1.getAttribute('measTime'), '%Y-%m-%dT%H:%M:%S'))

        measTime = sorted(enumerate(measTime), key=operator.itemgetter(1), reverse=True)
        sort1 = []
        for i in range(len(measTime)):
           sort1.append(measTime[i][0])

        if obsTime == '':
            tb.open(msName+'/OBSERVATION')
            obsTimeRange = tb.getcol('TIME_RANGE')
            obsTime = (obsTimeRange[0]+obsTimeRange[1])/2.0
            obsTime = ((obsTime/86400.0)+2400000.5-2440587.5)*86400.0
            obsTime = timeUtilities.strftime('%Y-%m-%dT%H:%M:%S', timeUtilities.gmtime(obsTime))
            obsTime = datetime.datetime.strptime(obsTime, '%Y-%m-%dT%H:%M:%S')
            tb.close()
        else:
            obsTime = datetime.datetime.strptime(obsTime, '%Y-%m-%dT%H:%M:%S')

        tb.open(msName+'/ANTENNA')
        antNames = tb.getcol('NAME')
        padNames = tb.getcol('STATION')
        tb.close()

        msAntPos = {}
        for i in range(len(antNames)):
           msAntPos[antNames[i]] = {}
           msAntPos[antNames[i]]['pad'] = padNames[i]

        tb.open(msName+'/ASDM_ANTENNA')
        asdmAntNames = tb.getcol('name')
        asdmAntPositions = tb.getcol('position')
        tb.close()

        for i in range(len(antNames)):
           if antNames[i] not in asdmAntNames: sys.exit('ERROR: Antenna not found.')
           for j in range(len(asdmAntNames)):
              if asdmAntNames[j] == antNames[i]:
                 msAntPos[antNames[i]]['antPos'] = [asdmAntPositions[0][j], asdmAntPositions[1][j], asdmAntPositions[2][j]]
                 break

        tb.open(msName+'/ASDM_STATION')
        asdmPadNames = tb.getcol('name')
        asdmPadPositions = tb.getcol('position')
        tb.close()

        for i in range(len(antNames)):
           if padNames[i] not in asdmPadNames: sys.exit('ERROR: Pad not found.')
           for j in range(len(asdmPadNames)):
              if asdmPadNames[j] == padNames[i]:
                 msAntPos[antNames[i]]['padPos'] = [asdmPadPositions[0][j], asdmPadPositions[1][j], asdmPadPositions[2][j]]
                 break

        for i in range(len(antNames)):

           ij = -1
           antMoves1 = {}
           for j in range(len(antMoves)):
              if antMoves[j]['Antenna'] == antNames[i]:
                 ij = ij+1
                 antMoves1[ij] = antMoves[j]

           if len(antMoves1) == 0:
              sys.exit("ERROR: No antenna move information available for "+str(antNames[i]))
           if obsTime < antMoves1[0]['Date']:
              sys.exit("ERROR: No antenna move information available for "+str(antNames[i]))

           for j in range(len(antMoves1)-1, -1, -1):
              if antMoves1[j]['Date'] <= obsTime: break

           if antMoves1[j]['To'] != padNames[i]:
              sys.exit("ERROR: Current data file has antenna "+str(antNames[i])+" on pad "+str(padNames[i])+" while latest move recorded is on pad "+str(antMoves1[j]['To']))

           msAntPos[antNames[i]]['putInTime'] = antMoves1[j]['Date']

        msAntCorr = {}

        for i in msAntPos:

           found = 0

           for j in range(len(baseRunObj)):
              baseRunObj1 = baseRunObj[sort1[j]]
              measTime1 = datetime.datetime.strptime(baseRunObj1.getAttribute('measTime'), '%Y-%m-%dT%H:%M:%S')
              if measTime1 > msAntPos[i]['putInTime'] and measTime1 <= obsTime: # modified
                 antObj = baseRunObj1.Antenna
                 for k in range(len(antObj)):
                    antObj1 = antObj[k]
                    if antObj1.getAttribute('name') == i and antObj1.getAttribute('pad') == msAntPos[i]['pad']:
                       found = 1
                       padPosObj = antObj1.PadPosition
                       brPadPos = [padPosObj.getAttribute('X'), padPosObj.getAttribute('Y'), padPosObj.getAttribute('Z')]
                       antVecObj = antObj1.AntennaVector
                       brAntVec = [antVecObj.getAttribute('X'), antVecObj.getAttribute('Y'), antVecObj.getAttribute('Z')]
                       posErrObj = antObj1.PositionError
                       brPosErr = [posErrObj.getAttribute('X'), posErrObj.getAttribute('Y'), posErrObj.getAttribute('Z')]
                       break
              if found == 1: break # modified

           if found != 1:

              casaCmd = casaCmd + '# Note: no baseline run found for antenna '+i+'.\n\n'

           else:

              if brPadPos[0] == msAntPos[i]['padPos'][0] and brPadPos[1] == msAntPos[i]['padPos'][1] and brPadPos[2] == msAntPos[i]['padPos'][2]:

                 if brAntVec[0] != msAntPos[i]['antPos'][0] or brAntVec[1] != msAntPos[i]['antPos'][1] or brAntVec[2] != msAntPos[i]['antPos'][2]:

                    posDiff = []
                    for j in range(3):
                       posDiff.append(brAntVec[j] - msAntPos[i]['antPos'][j])

                    if sqrt((posDiff[0]/brPosErr[0])**2 + (posDiff[1]/brPosErr[1])**2 + (posDiff[2]/brPosErr[2])**2) > 2:

                       brLat = math.asin(brPadPos[2]/sqrt(brPadPos[0]**2+brPadPos[1]**2+brPadPos[2]**2))
                       brLon = math.atan2(brPadPos[1], brPadPos[0])

                       posDiff1 = []
                       posDiff1.append(-math.sin(brLon)*posDiff[0]-math.cos(brLon)*math.sin(brLat)*posDiff[1]+math.cos(brLon)*math.cos(brLat)*posDiff[2])
                       posDiff1.append(math.cos(brLon)*posDiff[0]-math.sin(brLon)*math.sin(brLat)*posDiff[1]+math.sin(brLon)*math.cos(brLat)*posDiff[2])
                       posDiff1.append(math.cos(brLat)*posDiff[1]+math.sin(brLat)*posDiff[2])

                       if np.linalg.norm(posDiff1) > 2e-3: casaCmd = casaCmd + '# Note: the correction for antenna '+i+' is larger than 2mm.\n\n'

                       msAntCorr[i] = {}
                       msAntCorr[i]['posDiff'] = posDiff1
                       if verbose == True: casaCmd = casaCmd + '# Position for antenna '+i+' is derived from baseline run made on '+str(measTime1)+'.\n\n' # modified

              else:

                 brLat = math.asin(brPadPos[2]/sqrt(brPadPos[0]**2+brPadPos[1]**2+brPadPos[2]**2))
                 brLon = math.atan2(brPadPos[1], brPadPos[0])

                 brPosTot = []
                 brPosTot.append(brAntVec[0] + -math.sin(-brLon)*brPadPos[0]-math.cos(-brLon)*math.sin(-brLat)*brPadPos[1]+math.cos(-brLon)*math.cos(-brLat)*brPadPos[2])
                 brPosTot.append(brAntVec[1] + math.cos(-brLon)*brPadPos[0]-math.sin(-brLon)*math.sin(-brLat)*brPadPos[1]+math.sin(-brLon)*math.cos(-brLat)*brPadPos[2])
                 brPosTot.append(brAntVec[2] + cos(-brLat)*brPadPos[1]+sin(-brLat)*brPadPos[2])

                 msLat = math.asin(msAntPos[i]['padPos'][2]/sqrt(msAntPos[i]['padPos'][0]**2+msAntPos[i]['padPos'][1]**2+msAntPos[i]['padPos'][2]**2))
                 msLon = math.atan2(msAntPos[i]['padPos'][1], msAntPos[i]['padPos'][0])

                 msPosTot = []
                 msPosTot.append(msAntPos[i]['antPos'][0] + -math.sin(-msLon)*msAntPos[i]['padPos'][0]-math.cos(-msLon)*math.sin(-msLat)*msAntPos[i]['padPos'][1]+math.cos(-msLon)*math.cos(-msLat)*msAntPos[i]['padPos'][2])
                 msPosTot.append(msAntPos[i]['antPos'][1] + math.cos(-msLon)*msAntPos[i]['padPos'][0]-math.sin(-msLon)*math.sin(-msLat)*msAntPos[i]['padPos'][1]+math.sin(-msLon)*math.cos(-msLat)*msAntPos[i]['padPos'][2])
                 msPosTot.append(msAntPos[i]['antPos'][2] + math.cos(-msLat)*msAntPos[i]['padPos'][1]+math.sin(-msLat)*msAntPos[i]['padPos'][2])

                 posDiff = []
                 for j in range(3):
                    posDiff.append(brPosTot[j] - msPosTot[j])

                 if sqrt((posDiff[0]/brPosErr[0])**2 + (posDiff[1]/brPosErr[1])**2 + (posDiff[2]/brPosErr[2])**2) > 2:

                    brPosTot1 = []
                    brPosTot1.append(brPadPos[0] + -math.sin(brLon)*brAntVec[0]-math.cos(brLon)*math.sin(brLat)*brAntVec[1]+math.cos(brLon)*math.cos(brLat)*brAntVec[2])
                    brPosTot1.append(brPadPos[1] + math.cos(brLon)*brAntVec[0]-math.sin(brLon)*math.sin(brLat)*brAntVec[1]+math.sin(brLon)*math.cos(brLat)*brAntVec[2])
                    brPosTot1.append(brPadPos[2] + math.cos(brLat)*brAntVec[1]+math.sin(brLat)*brAntVec[2])

                    msPosTot1 = []
                    msPosTot1.append(msAntPos[i]['padPos'][0] + -math.sin(msLon)*msAntPos[i]['antPos'][0]-math.cos(msLon)*math.sin(msLat)*msAntPos[i]['antPos'][1]+math.cos(msLon)*math.cos(msLat)*msAntPos[i]['antPos'][2])
                    msPosTot1.append(msAntPos[i]['padPos'][1] + math.cos(msLon)*msAntPos[i]['antPos'][0]-math.sin(msLon)*math.sin(msLat)*msAntPos[i]['antPos'][1]+math.sin(msLon)*math.cos(msLat)*msAntPos[i]['antPos'][2])
                    msPosTot1.append(msAntPos[i]['padPos'][2] + math.cos(msLat)*msAntPos[i]['antPos'][1]+math.sin(msLat)*msAntPos[i]['antPos'][2])

                    posDiff1 = []
                    for j in range(3):
                       posDiff1.append(brPosTot1[j] - msPosTot1[j])

                    if np.linalg.norm(posDiff1) > 2e-3: casaCmd = casaCmd + '# Note: the correction for antenna '+i+' is larger than 2mm.\n\n'

                    msAntCorr[i] = {}
                    msAntCorr[i]['posDiff'] = posDiff1
                    if verbose == True: casaCmd = casaCmd + '# Position for antenna '+i+' is derived from baseline run made on '+str(measTime1)+'.\n\n' # modified

        gcAntList = []
        for i in msAntCorr: gcAntList.append(i)
        gcAntList = ','.join(gcAntList)

        gcAntParam = []
        gcAntParam0 = []
        for i in msAntCorr:
           for j in range(3):
              gcAntParam.append(str(msAntCorr[i]['posDiff'][j]))
              gcAntParam0.append('0')
        gcAntParam = ','.join(gcAntParam)
        gcAntParam0 = ','.join(gcAntParam0)

        casaCmd = casaCmd + "os.system('rm -rf %s.antpos') \n"%(msName)  # Added by CLB
        casaCmd = casaCmd + "gencal(vis = '"+msName+"',\n"
        casaCmd = casaCmd + "  caltable = '"+msName+".antpos',\n"
        casaCmd = casaCmd + "  caltype = 'antpos',\n"
        casaCmd = casaCmd + "  antenna = '"+gcAntList+"',\n"
        casaCmd = casaCmd + "  parameter = ["+gcAntParam0+"])\n"
        casaCmd = casaCmd + "#  parameter = ["+gcAntParam+"])\n"

        return casaCmd

    def getSpwInfo(self, msName, intent='OBSERVE_TARGET'):

        tb.open(msName+'/STATE')
        obsModes = tb.getcol('OBS_MODE')
        tb.close()

        obsModes1 = []
        for i in range(len(obsModes)):
            if re.search(intent, obsModes[i]) != None:
                obsModes1.append(i)

        if len(obsModes1) == 0: sys.exit('ERROR: Intent not found.')

        tb.open(msName+'/PROCESSOR')
        tb1 = tb.query("SUB_TYPE == 'ALMA_CORRELATOR_MODE'")
        procIds = tb1.rownumbers()

        if (type(procIds) == int):
            # Added by T. Hunter to prevent crash at 'for' loop when only one value is returned.
            procIds = [procIds]
        tb.close()

        tb.open(msName+'/DATA_DESCRIPTION')
        spwIds = tb.getcol('SPECTRAL_WINDOW_ID')
        tb.close()

        tb.open(msName+'/SPECTRAL_WINDOW')
        numChans = tb.getcol('NUM_CHAN')
        tb.close()

        tb.open(msName)

        dataDescIds = []
        for i in procIds:
            for j in obsModes1:
                tb1 = tb.query('PROCESSOR_ID == '+str(i)+' AND STATE_ID == '+str(j))
                dataDescIds1 = sorted(dict.fromkeys(tb1.getcol('DATA_DESC_ID')).keys())
                dataDescIds.extend(dataDescIds1)

        tb.close()

        dataDescIds = sorted(dict.fromkeys(dataDescIds).keys())

        if intent == 'OBSERVE_TARGET':

              tb.open(msName)

              integTime = []
              for i in dataDescIds:
                  tb1 = tb.query('DATA_DESC_ID == '+str(i))
                  integTime1 = dict.fromkeys(tb1.getcol('EXPOSURE')).keys()
                  if len(integTime1) != 1:
                      print "WARNING: DATA ASSOCIATED TO DATA_DESC_ID="+str(i)+" IN "+msName+" HAVE MORE THAN ONE INTEGRATION TIME."
                  integTime.append(integTime1[0])

              tb.close()

        spwInfo = {}
        for i in range(len(dataDescIds)):
            if numChans[spwIds[dataDescIds[i]]] != 1:
                spwInfo[spwIds[dataDescIds[i]]] = {}
                spwInfo[spwIds[dataDescIds[i]]]['numChans'] = numChans[spwIds[dataDescIds[i]]]
                if intent == 'OBSERVE_TARGET': spwInfo[spwIds[dataDescIds[i]]]['integTime'] = integTime[i]

        return spwInfo

    def generateTsysCalTable(self, msName, calTableName=[]):

        #casaCmd = 'print "# Generation of the Tsys cal table."\n\n'
        casaCmd = ''

	if re.search('^3.3', casadef.casa_version) != None:
            es = stuffForScienceDataReduction()
            sciSpwInfo = es.getSpwInfo(msName)
            tsysSpwInfo = es.getSpwInfo(msName, intent='CALIBRATE_ATMOSPHERE')

            sciNumChans = []
            for i in sciSpwInfo: sciNumChans.append(sciSpwInfo[i]['numChans'])
            sciNumChans = sorted(dict.fromkeys(sciNumChans).keys())
            if len(sciNumChans) != 1: sys.exit('ERROR: Configuration not supported.')

            tsysNumChans = []
            for i in tsysSpwInfo: tsysNumChans.append(tsysSpwInfo[i]['numChans'])
            tsysNumChans = sorted(dict.fromkeys(tsysNumChans).keys())
            if len(tsysNumChans) != 1: sys.exit('ERROR: Configuration not supported.')

        casaCmd = casaCmd + "os.system('rm -rf %s.tsys') \n"%(msName)  # Added by CLB
        if re.search('^3.3', casadef.casa_version) != None:
	    casaCmd = casaCmd + "os.system('rm -rf %s.tsys.fdm') \n\n"%(msName)  # Added by CLB
        casaCmd = casaCmd + "gencal(vis = '"+msName+"',\n"
        casaCmd = casaCmd + "  caltable = '"+msName+".tsys',\n"
        casaCmd = casaCmd + "  caltype = 'tsys')\n\n"

	if re.search('^3.3', casadef.casa_version) != None:
            casaCmd = casaCmd + "interTsys = aU.InterpolateTsys('"+msName+".tsys')\n"
            casaCmd = casaCmd + "interTsys.correctBadTimes(force=True)\n"
            casaCmd = casaCmd + "interTsys.assignFieldAndScanToSolution()\n"

	calTableName1 = msName+'.tsys'

	if re.search('^3.3', casadef.casa_version) != None:
            if tsysNumChans < sciNumChans:
                casaCmd = casaCmd + "interTsys.getTdmFdmSpw()\n"
                casaCmd = casaCmd + "interTsys.interpolateTsys()\n"
                calTableName1 = msName+'.tsys' # CLB removed .fdm temporarily for Tsys plotting below

            casaCmd = casaCmd + "clearstat()\n\n"

# CLB Added additional Tsys plot:  TDM with overlay='time', atmospheric transmission,
# a slightly restricted chanrange (to exclude wild edge values), and showing the
# location of the FDM spws (if present)
        casaCmd = casaCmd + "aU.plotbandpass(caltable='%s', overlay='time', \n" %(calTableName1)
        casaCmd = casaCmd + "  xaxis='freq', yaxis='amp', subplot=22, buildpdf=False, interactive=False,\n" 
        casaCmd = casaCmd + "  showatm=True,pwv='auto',chanrange='5~122',showfdm=True, \n"
        casaCmd = casaCmd + "  field='', figfile='%s') \n\n" %(calTableName1+'.plots.overlayTime/'+calTableName1.split('/')[-1])

	if re.search('^3.3', casadef.casa_version) != None:
            if tsysNumChans < sciNumChans:   # CLB added
                calTableName1 += '.fdm'      # CLB added .fdm for following plot
        calTableName.append(calTableName1)
        casaCmd = casaCmd + "\nes.checkCalTable('"+calTableName1+"', msName='"+msName+"', interactive=False) \n"

        return casaCmd

    def getCalWeightStats(self, msName):

        sciSpwInfo = self.getSpwInfo(msName)

        tb.open(msName+'/DATA_DESCRIPTION')
        spwIds = tb.getcol('SPECTRAL_WINDOW_ID')
        tb.close()

        output = "# Spw Pol Min Mean Max\n"

        tb.open(msName)

        for i in sciSpwInfo:
            dataDescId = np.where(np.array(spwIds) == i)[0]
            if len(dataDescId) != 1: sys.exit('ERROR: Too many data desc ids')
            dataDescId = dataDescId[0]
            tb1 = tb.query('DATA_DESC_ID == '+str(dataDescId)+' AND FLAG_ROW != True')
            calWeightStats = tb1.statistics(column='WEIGHT')
            for j in calWeightStats.keys():
                output = output + '%d %s %.2f %.2f %.2f' %(i, j.split('_')[1], calWeightStats[j]['min'], calWeightStats[j]['mean'], calWeightStats[j]['max'])
                if calWeightStats[j]['max'] < 10:
                    output = output + ' -> OK\n'
                else:
                    output = output + ' -> NOT OK\n'

        tb.close()

        print output

        f = open(msName+'.calweights', 'w')
        print >> f, output
        f.close()

    def getFlagStats(self, msName):

        import flag

        flagStats = flag.flag_calc(msName)

        print 'Overall -> %.2f' %(100.0*flagStats['flagged']/flagStats['total'])
        print ''
        print 'Per spw (over total of dataset):'
        for i in flagStats['spw']:
              print i+' -> %.2f' %(100.0*flagStats['spw'][i]['flagged']/flagStats['total'])
        print ''
        print 'Per antenna (over total of dataset):'
        for i in flagStats['antenna']:
              print i+' -> %.2f' %(100.0*flagStats['antenna'][i]['flagged']/flagStats['total'])

    def getImageStats(self, imgName):

        import image

        imgStats = image.image_calc(imgName)

        nChans = len(imgStats['frequency'][0])

        print '# frequency max_in rms_out dynrange'
        for i in range(nChans):
              print str(i) + " %.2f" %(imgStats['frequency'][0][i]/1.0e9) + " %.2g" %(imgStats['max_in'][0][i]) + " %.2g" %(imgStats['rms_out'][0][i]) + " %.2f" %(imgStats['dynrange'][0][i])

    def getPsfStats(self, psfName):

        import psf

        psfStats = psf.psf_calc(psfName)

        nChans = len(psfStats['all_fits']['keyword']['frequency'])

        print '# frequency major_axis error unit minor_axis error unit position_angle error unit'
        for i in range(nChans):
              if psfStats['all_fits']['converged'][0][i] == True:
                    print str(i) + " %.2f %.4g %.4g %s %.4g %.4g %s %.4g %.4g %s" %((psfStats['all_fits']['keyword']['frequency'][i]/1.0e9), \
                        psfStats['all_fits']['major_axis'][0][i],psfStats['all_fits']['major_axis_err'][0][i],psfStats['all_fits']['keyword']['major_axis_unit'], \
                        psfStats['all_fits']['minor_axis'][0][i],psfStats['all_fits']['minor_axis_err'][0][i],psfStats['all_fits']['keyword']['minor_axis_unit'], \
                        psfStats['all_fits']['position_angle'][0][i],psfStats['all_fits']['position_angle_err'][0][i],psfStats['all_fits']['keyword']['position_angle_unit'])
              else:
                    print str(i) + " %.2f PSF fit did not converge." %(psfStats['all_fits']['keyword']['frequency'][i]/1.0e9)

    def plotAntennas(self, ms, figfile=None, title=None):
        """
        Plot antenna/pad positions in ANTENNA table.

        """
        if figfile is None:
            figfile = ms.basename + '.plotAntennas.png'
        if title is None:
            title = 'Antenna Station Positions'

        import pipeline.infrastructure.casatools as casatools
        mt = casatools.measures
        qt = casatools.quanta
        site = mt.observatory(ms.antenna_array.name)
        sitelon = qt.getvalue(qt.convert(site['m0'], 'rad'))
        sitelat = qt.getvalue(qt.convert(site['m1'], 'rad'))
#        (sitelon, sitelat) = (pb.radians(-67.75), pb.radians(-23.02))
        tb.open(ms.name+'/ANTENNA')
        ant_pos = tb.getcol('POSITION')
        ant_name = tb.getcol('NAME')
        ant_station = tb.getcol('STATION')
        tb.close()

        config_antennas = {}
        for (i, ant) in enumerate(ant_name):
            config_antennas[ant] = ant_station[i]

        pads = {}
        for (i, pad) in enumerate(ant_station):
#            if ms.antenna_array.name == 'ALMA':
#                pads[pad] = pb.array(self.shiftAlmaCoord(self.geoToAlma(sitelon, sitelat, \
#                                                                        (ant_pos[0][i], ant_pos[1][i], ant_pos[2][i]))))
#            else:
                pads[pad] = pb.array(self.geoToAlma(sitelon, sitelat, (ant_pos[0][i], ant_pos[1][i], ant_pos[2][i])))                

        # map: with pad names
        plf1 = pb.figure(1)
        #plf1.clf()
        subpl1 = plf1.add_subplot(1, 1, 1, aspect='equal')
        self.draw_pad_map_in_subplot(subpl1, pads, config_antennas)
        pb.title(title)
        pb.savefig(figfile,format='png',density=108)
        #plf1.show()

        ## map: without pad names
        #plf2 = pb.figure(2)
        ##plf2.clf()
        #subpl2 = plf2.add_subplot(1, 1, 1, aspect='equal')
        #self.draw_pad_map_in_subplot(subpl2, pads, config_antennas, showemptypads=False)
        #plf2.show()

    def shiftAlmaCoord(self, pos):
        """
        Arbitrarily shift ALMA coord so that central cluster comes around (0, 0).
        """
        return (pos[0]+480., pos[1]-14380., pos[2])

    def geoToAlma(self, lon, lat, geo):
        """
        Convert the geocentric coordinates into the local (horizontal) ones.
        """
        alma = [0, 0, 0]
        alma[0] = - geo[0]*pb.sin(lon) + \
                    geo[1]*pb.cos(lon)
        alma[1] = - geo[0]*pb.cos(lon)*pb.sin(lat) - \
                    geo[1]*pb.sin(lon)*pb.sin(lat) + \
                    geo[2]*pb.cos(lat)
        alma[2] =   geo[0]*pb.cos(lon)*pb.cos(lat) + \
                    geo[1]*pb.sin(lon)*pb.cos(lat) + \
                    geo[2]*pb.sin(lat)
        return alma

    def draw_pad_map_in_subplot(self, subpl, pads, antennas, xlimit=None, ylimit=None,
        showemptypads=True):
        """
        Draw a map of pads and antennas on them.

        subpl: a pylab.subplot instance
        pads: a dictionary of antennas {"Name": (X, Y, Z), ...}
        antennas: a dictionary of antennas {"AntennaName": "PadName", ...}
        xlimit, ylimit: lists (or tuples, arrays) for the x and y axis limits.
                        if not given, automatically adjusted.
        showemptypads: set False not to draw pads and their names
        showbaselinelength: set True to display baseline length
        """
        subpl.clear()
        if showemptypads:
            for pad in pads.keys():
                padpos = pads[pad]
                if pad[:1] in ['J', 'N']:
                    radius = 3.5
                else:
                    radius = 6.0
                circ = pb.Circle(padpos[:2], radius)
                subpl.add_artist(circ)
                circ.set_alpha(0.5)
                circ.set_facecolor([1.0, 1.0, 1.0])
                tt = subpl.text(padpos[0]+8., padpos[1]-5., pad)
                pb.setp(tt, size='small', alpha=0.5)

        (xmin, xmax, ymin, ymax) = (9e9, -9e9, 9e9, -9e9)
        for ant in antennas.keys():
            if ant[:2] == 'CM':
                radius = 3.5
            else:
                radius = 6.
            padpos = pads[antennas[ant]]
            circ = pb.Circle(padpos[:2], radius=radius)
            subpl.add_artist(circ)
            circ.set_alpha(1.0)
            circ.set_facecolor([0.8, 0.8, 0.8])
            subpl.text(padpos[0], padpos[1]+2, ant)
            if padpos[0] < xmin: xmin = padpos[0]
            if padpos[0] > xmax: xmax = padpos[0]
            if padpos[1] < ymin: ymin = padpos[1]
            if padpos[1] > ymax: ymax = padpos[1]

        subpl.set_xlabel('X [m]')
        subpl.set_ylabel('Y [m]')
        plotwidth = max(xmax-xmin, ymax-ymin) * 6./10. # extra 1/10 is the margin
        (xcenter, ycenter) = ((xmin+xmax)/2., (ymin+ymax)/2.)
	#print 'center ', xcenter, ycenter, 'plotwidth ', plotwidth
        if xlimit == None:
            #subpl.set_xlim(xcenter-plotwidth, xcenter+plotwidth)
            subpl.set_xlim(xcenter[0]-plotwidth[0], xcenter[0]+plotwidth[0])
        else:
            subpl.set_xlim(xlimit[0], xlimit[1])
        if ylimit == None:
            #subpl.set_ylim(ycenter-plotwidth, ycenter+plotwidth)
            subpl.set_ylim(ycenter[0]-plotwidth[0], ycenter[0]+plotwidth[0])
        else:
            subpl.set_ylim(ylimit[0], ylimit[1])

        ants = antennas.keys()
        ants.sort()

        return None

    def getAntennasForFluxscale2(self, msName, fluxCalId='', refant='', thresh=0.15):

        spwInfo = self.getSpwInfo(msName)

        msSplit = 0

        tb.open(msName)
        colNames = tb.colnames()
        tb.close()

        if 'MODEL_DATA' not in colNames:

            sciSpwIds = sorted(spwInfo.keys())
            sciSpwIds1 = ','.join(['%s' %i for i in sciSpwIds])

            split(vis = msName, outputvis = msName+'.temp', datacolumn = 'data', field = fluxCalId, spw = sciSpwIds1)

            msName = msName+'.temp'

            fixplanets(vis = msName, field = '0', fixuvw = True)
	    if re.search('^3.3', casadef.casa_version) == None:
                setjy(vis = msName, field = '0', standard = 'Butler-JPL-Horizons 2010', usescratch = True)
	    else:
	        setjy(vis = msName, field = '0', standard = 'Butler-JPL-Horizons 2010')

            for i in sciSpwIds:
                spwInfo[str(sciSpwIds.index(i))] = spwInfo[i]
                spwInfo.pop(i)

            fluxCalId = '0'

            msSplit = 1

        tb.open(msName+'/DATA_DESCRIPTION')
        spwIds = tb.getcol('SPECTRAL_WINDOW_ID').tolist()
        tb.close()

        uvDist1 = []

        tb.open(msName)

        for i in spwInfo:

            dataDescId = spwIds.index(int(i))
            tb1 = tb.query('FIELD_ID == '+fluxCalId+' AND DATA_DESC_ID == '+str(dataDescId))
            uvw = tb1.getcol('UVW')
            modelData = tb1.getcol('MODEL_DATA')

            uvDist = []
            modelData1 = []
            for j in range(len(uvw[0])):
                uvDist.append(sqrt(uvw[0][j]**2+uvw[1][j]**2))
                modelData1.append(abs(modelData[0][int(spwInfo[i]['numChans'])/2][j]))

            uvDist = np.array(uvDist)
            modelData1 = np.array(modelData1)
            ij = np.where(modelData1 < thresh*max(modelData1))
            if len(ij[0]) != 0: uvDist1.append(min(uvDist[ij]))

        tb.close()

        tb.open(msName+'/ANTENNA')
        antList = tb.getcol('NAME')
        tb.close()

        if len(uvDist1) == 0:

            if msSplit == 1: os.system('rm -Rf '+msName)
            return antList.tolist()

        else:

            uvDist1 = min(np.array(uvDist1))

            baselineLen = getBaselineLengths(msName)

            antList1 = []
            for i in antList:
                for j in baselineLen:
                    if i in j[0] and refant in j[0] and j[1] <= uvDist1:
                        antList1.append(i)
                        break

            while len(antList1) > 1:

                maxBaselineLen = 0.0
                for i in baselineLen:
                    ii = i[0].split('-')
                    if ii[0] in antList1 and ii[1] in antList1 and i[1] > maxBaselineLen: maxBaselineLen = i[1]
                if maxBaselineLen <= uvDist1: break

                avgBaselineLen = []
                for i in antList1:
                    count1 = 0
                    sum1 = 0.0
                    for j in baselineLen:
                        if i in j[0] and re.sub('-?'+i+'-?', '', j[0]) in antList1:
                            count1 = count1+1
                            sum1 = sum1+j[1]
                    avgBaselineLen.append(sum1 / count1)

                j = avgBaselineLen.index(max(avgBaselineLen))
                antList1.pop(j)

            if msSplit == 1: os.system('rm -Rf '+msName)
            return antList1

    def checkCalTable(self, calTableName, msName='', interactive=True, forceremove=False):

        if os.path.isdir(calTableName+'.plots') == True:
            if (forceremove == False):
                raw_input("Directory for plots already exists. It will be removed. Press Enter to continue...")
            os.system('rm -Rf '+calTableName+'.plots')

        supportedCalTypes = ['B Jones', 'G Jones', 'B TSYS', 'T Jones']

        if msName == '':
            tb.open(calTableName+'/CAL_DESC')
            msName = dict.fromkeys(tb.getcol('MS_NAME')).keys()
            if len(msName) != 1: sys.exit('ERROR: Too many names.')
            msName = msName[0]
            tb.close()

        tb.open(calTableName)
        calType = (tb.info())['subType']
        if calType not in supportedCalTypes: sys.exit('ERROR: Cal type not supported.')

        if calType == 'B TSYS':
            fieldIds = sorted(dict.fromkeys(tb.getcol('FIELD_ID')).keys())
            tb.close()
            vm = ValueMapping(msName)
            for i in fieldIds:
                plotbandpass(caltable=calTableName, overlay='antenna', xaxis='freq', yaxis='amp', subplot=22, buildpdf=False, interactive=interactive, field=str(i), vm=vm, figfile=calTableName+'.plots/'+calTableName+'.field'+str(i))
            casaCmd  = "# This is what checkCalTable executed:\n"  # Added by CLB
            casaCmd += "#for i in %s:\n" % list(fieldIds)   # Added by CLB
            casaCmd += "#   aU.plotbandpass(caltable='%s', overlay='antenna', xaxis='freq', yaxis='amp', subplot=22, buildpdf=False, interactive=False, field=str(i), figfile='%s.plots/%s.field'+str(i))\n\n" % (calTableName,calTableName,calTableName.split('/')[-1])   # Added by CLB
            print casaCmd  # Added by CLB

        if calType == 'B Jones':
            tb.close()
            vm = ValueMapping(msName)
            casaCmd  = "# This is what checkCalTable executed:\n"  # Added by CLB
            if os.path.isdir(calTableName+'_smooth20flat_ri') == True:
                plotbandpass(caltable=calTableName, caltable2=calTableName+'_smooth20flat_ri', xaxis='freq', yaxis='both', showatm=True, pwv='auto', subplot=22, buildpdf=False, interactive=interactive, vm=vm, figfile=calTableName+'.plots/'+calTableName)
                casaCmd += "#aU.plotbandpass(caltable='%s', caltable2='%s_smooth20flat_ri', xaxis='freq', yaxis='both', showatm=True, pwv='auto', subplot=22, buildpdf=False, interactive=False, vm=vm, figfile='%s.plots/%s)\n" % (calTableName,calTableName,calTableName,calTableName.split('/')[-1])   # Added by CLB
            else:
                plotbandpass(caltable=calTableName, xaxis='freq', yaxis='both', showatm=True, pwv='auto', subplot=22, buildpdf=False, interactive=interactive, vm=vm, figfile=calTableName+'.plots/'+calTableName)
                casaCmd += "#aU.plotbandpass(caltable='%s', xaxis='freq', yaxis='both', showatm=True, pwv='auto', subplot=22, buildpdf=False, interactive=False, vm=vm, figfile='%s.plots/%s')\n" % (calTableName,calTableName,calTableName.split('/')[-1])  # Added by CLB
            print casaCmd  # Added by CLB

        if calType in ['G Jones', 'T Jones']:

	    if re.search('^3.3', casadef.casa_version) != None:
                calStats = {}
                calStats['GAIN_amp'] = (tb.statistics(column='GAIN', complex_value='amp'))['GAIN']
                calStats['GAIN_phase'] = (tb.statistics(column='GAIN', complex_value='phase'))['GAIN']
                tb.close()
	    else:
                calStats = {}
                calStats['GAIN_amp'] = (tb.statistics(column='CPARAM', complex_value='amp'))['CPARAM']
                calStats['GAIN_phase'] = (tb.statistics(column='CPARAM', complex_value='phase'))['CPARAM']
                tb.close()

            tb.open(msName+'/ANTENNA')
            antList = tb.getcol('NAME')
            tb.close()

            os.system('mkdir '+calTableName+'.plots')

            #if calStats['GAIN_amp']['medabsdevmed'] != 0:
            if abs(calStats['GAIN_amp']['min']-1) > 0.001 or abs(calStats['GAIN_amp']['max']-1) > 0.001:
                minAmp = calStats['GAIN_amp']['min']
                maxAmp = calStats['GAIN_amp']['max']
                for i in antList:
                    plotcal(caltable=calTableName, xaxis='time', yaxis='amp', antenna=i, iteration='antenna,spw', subplot=411, plotrange=[0, 0, minAmp, maxAmp], figfile=calTableName+'.plots/'+calTableName+'.amp.'+i+'.png')  # subplot changed to 411 by CLB so that antenna name can be seen
                    if interactive == True:
                        userRawInput = raw_input("Press Enter to continue, q to quit.")
                        if userRawInput.lower() == 'q': break
                casaCmd  = "# This is what checkCalTable executed:\n"  # Added by CLB
                casaCmd += "#for i in %s:\n" % list(antList)   # Added by CLB
                casaCmd += "#   plotcal(caltable='%s', xaxis='time', yaxis='amp', antenna=i, iteration='antenna,spw', subplot=411, plotrange=[0, 0, %f, %f], figfile='%s.plots/%s.amp.'+str(i)+'.png')\n" % (calTableName,minAmp, maxAmp,calTableName,calTableName.split('/')[-1])  # Added by CLB
                print casaCmd  # Added by CLB
            #if calStats['GAIN_phase']['medabsdevmed'] != 0:
            if abs(calStats['GAIN_phase']['min']) > 0.001 or abs(calStats['GAIN_phase']['max']) > 0.001:
                minPhase = degrees(calStats['GAIN_phase']['min'])
                maxPhase = degrees(calStats['GAIN_phase']['max'])
                for i in antList:
                    plotcal(caltable=calTableName, xaxis='time', yaxis='phase', antenna=i, iteration='antenna,spw', subplot=411, plotrange=[0, 0, minPhase, maxPhase], figfile=calTableName+'.plots/'+calTableName+'.phase.'+i+'.png')  # subplot changed to 411 by CLB so that antenna name can be seen
                    if interactive == True:
                        userRawInput = raw_input("Press Enter to continue, q to quit.")
                        if userRawInput.lower() == 'q': break
                casaCmd  = "# This is what checkCalTable executed:\n"  # Added by CLB
                casaCmd += "#for i in %s:\n" % list(antList)   # Added by CLB
                casaCmd += "#    plotcal(caltable='%s', xaxis='time', yaxis='phase', antenna=i, iteration='antenna,spw', subplot=411, plotrange=[0, 0,%f,%f], figfile='%s.plots/%s.phase.'+str(i)+'.png')\n" % (calTableName, minPhase, maxPhase,calTableName,calTableName.split('/')[-1])  # Added by CLB
                print casaCmd  # Added by CLB

    def checkCalTable2(self, calTableName, interactive=True):

        casaCmd = ''

        supportedCalTypes = ['B Jones', 'G Jones', 'B TSYS', 'T Jones']

        tb.open(calTableName+'/CAL_DESC')
        msName = dict.fromkeys(tb.getcol('MS_NAME')).keys()
        if len(msName) != 1: sys.exit('ERROR: Too many names.')
        msName = msName[0]
        tb.close()

        tb.open(calTableName)
        calType = (tb.info())['subType']
        if calType not in supportedCalTypes: sys.exit('ERROR: Cal type not supported.')

        if calType == 'B TSYS':
            fieldIds = sorted(dict.fromkeys(tb.getcol('FIELD_ID')).keys())
            tb.close()
            casaCmd = casaCmd + "vm = aU.ValueMapping("+msName+")\n\n"
            casaCmd = casaCmd + "for i in "+str(fieldIds)+":\n"
            casaCmd = casaCmd + "  aU.plotbandpass(caltable = '"+calTableName+"',\n"
            casaCmd = casaCmd + "    overlay = 'antenna',\n"
            casaCmd = casaCmd + "    xaxis = 'freq',\n"
            casaCmd = casaCmd + "    yaxis = 'amp',\n"
            casaCmd = casaCmd + "    subplot = 22,\n"
            casaCmd = casaCmd + "    buildpdf = False,\n"
            casaCmd = casaCmd + "    interactive = "+str(interactive)+",\n"
            casaCmd = casaCmd + "    field = str(i),\n"
            casaCmd = casaCmd + "    vm = vm,\n"
            casaCmd = casaCmd + "    figfile = '"+calTableName+".plots/"+calTableName+".field'+str(i))\n"

        if calType == 'B Jones':
            tb.close()
            casaCmd = casaCmd + "vm = aU.ValueMapping("+msName+")\n\n"
            casaCmd = casaCmd + "aU.plotbandpass(caltable = '"+calTableName+"',\n"
            if os.path.isdir(calTableName+'_smooth20flat_ri') == True:
                casaCmd = casaCmd + "  caltable2 = '"+calTableName+"_smooth20flat_ri',\n"
            else:
                casaCmd = casaCmd + "  #caltable2 = '"+calTableName+"_smooth20flat_ri', # please uncomment this line if you have run smoothbandpass\n"
            casaCmd = casaCmd + "  xaxis = 'freq',\n"
            casaCmd = casaCmd + "  yaxis = 'both',\n"
            casaCmd = casaCmd + "  showatm = True,\n"
            casaCmd = casaCmd + "  pwv = 'auto',\n"
            casaCmd = casaCmd + "  subplot = 22,\n"
            casaCmd = casaCmd + "  buildpdf = False,\n"
            casaCmd = casaCmd + "  interactive = "+str(interactive)+",\n"
            casaCmd = casaCmd + "  vm = vm,\n"
            casaCmd = casaCmd + "  figfile = '"+calTableName+".plots/"+calTableName+"')\n"

        if calType in ['G Jones', 'T Jones']:

            calStats = {}
            calStats['GAIN_amp'] = (tb.statistics(column='GAIN', complex_value='amp'))['GAIN']
            calStats['GAIN_phase'] = (tb.statistics(column='GAIN', complex_value='phase'))['GAIN']
            tb.close()

            tb.open(msName+'/ANTENNA')
            antList = tb.getcol('NAME')
            tb.close()

            os.system('mkdir '+calTableName+'.plots')

            if calStats['GAIN_amp']['medabsdevmed'] != 0:
                minAmp = calStats['GAIN_amp']['min']
                maxAmp = calStats['GAIN_amp']['max']
                for i in antList:
                    plotcal(caltable=calTableName, xaxis='time', yaxis='amp', antenna=i, iteration='antenna,spw', subplot=221, plotrange=[0, 0, minAmp, maxAmp], figfile=calTableName+'.plots/'+calTableName+'.amp.'+i+'.png')
                    if interactive == True:
                        userRawInput = raw_input("Press Enter to continue, q to quit.")
                        if userRawInput.lower() == 'q': break
            if calStats['GAIN_phase']['medabsdevmed'] != 0:
                minPhase = degrees(calStats['GAIN_phase']['min'])
                maxPhase = degrees(calStats['GAIN_phase']['max'])
                for i in antList:
                    plotcal(caltable=calTableName, xaxis='time', yaxis='phase', antenna=i, iteration='antenna,spw', subplot=221, plotrange=[0, 0, minPhase, maxPhase], figfile=calTableName+'.plots/'+calTableName+'.phase.'+i+'.png')
                    if interactive == True:
                        userRawInput = raw_input("Press Enter to continue, q to quit.")
                        if userRawInput.lower() == 'q': break

        return casaCmd

    def getFieldsForFixPlanets(self, msName):

        tb.open(msName+'/FIELD')
        phaseDir = tb.getcol('PHASE_DIR')
        tb.close()

        fieldIds = []
        for i in range(len(phaseDir[0][0])):
            if phaseDir[0][0][i] == 0 and phaseDir[1][0][i] == 0: fieldIds.append(i)

        return fieldIds

    def runFixPlanets(self, msName):

        fieldIds = self.getFieldsForFixPlanets(msName)

        if len(fieldIds) != 0:

              #casaCmd = 'print "# Running fixplanets on fields with 0,0 coordinates."\n\n'
              casaCmd = ''

              tb.open(msName+'/FIELD')
              fieldNames = tb.getcol('NAME')
              tb.close()

              fieldNames = ['%s' %fieldNames[i] for i in fieldIds]
              fieldNames = ','.join(fieldNames)
              fieldIds = ['%s' %i for i in fieldIds]
              fieldIds = ','.join(fieldIds)

              casaCmd = casaCmd + "fixplanets(vis = '"+msName+"',\n"
              casaCmd = casaCmd + "  field = '"+fieldIds+"', # "+fieldNames+"\n"
              casaCmd = casaCmd + "  fixuvw = T)\n"

              return casaCmd

    def applyAprioriCalTables_old(self, msName, tsys='', wvr='', antpos=''):

        casaCmd = 'print "# Application of the WVR, Tsys and antpos cal tables."'

        if tsys=='' and wvr=='' and antpos=='': sys.exit('ERROR: No cal table specified.')

        gainTable = []
        gainTable.append(tsys)
        gainTable.append(wvr)
        gainTable.append(antpos)
        gainTable = ['%s' %i for i in gainTable if i != '']

        spwInfo = self.getSpwInfo(msName)
        spwIds = spwInfo.keys()
        spwIds1 = ','.join(['%s' %i for i in spwIds])

        if tsys=='':

            casaCmd = casaCmd + "\n\napplycal(vis = '"+msName+"',\n"
            casaCmd = casaCmd + "  spw = '"+spwIds1+"',\n"
            casaCmd = casaCmd + "  gaintable = "+str(gainTable)+",\n"
            casaCmd = casaCmd + "  interp = 'linear',\n"
            casaCmd = casaCmd + "  calwt = F,\n"
            casaCmd = casaCmd + "  flagbackup = F)\n"

        else:

            tb.open(msName+'/FIELD')
            sourceIds = tb.getcol('SOURCE_ID')
            tb.close()

            if os.path.exists(tsys) == True:
                  tb.open(tsys)
                  fieldIds = sorted(dict.fromkeys(tb.getcol('FIELD_ID')).keys())
                  tb.close()
            else:
                  intentSources = self.getIntentsAndSourceNames(msName)
                  fieldIds = intentSources['CALIBRATE_ATMOSPHERE']['id']

            for i in fieldIds:
                fieldIds1 = (np.where(sourceIds == sourceIds[i]))[0]
                if len(fieldIds1) > 1:
                    j0 = 0
                    fieldIds2 = str(fieldIds1[j0])
                    for j in range(len(fieldIds1)-1):
                        if fieldIds1[j+1] == fieldIds1[j]+1: continue
                        fieldIds2 = fieldIds2 + '~' + str(fieldIds1[j])
                        j0 = j+1
                        fieldIds2 = fieldIds2 + ',' + str(fieldIds1[j0])
                    fieldIds2 = fieldIds2 + '~' + str(fieldIds1[j+1])
                else:
                    fieldIds2 = str(fieldIds1[0])

                gainField = []
                for j in range(len(gainTable)): gainField.append('')
                gainField[0] = str(i)

                casaCmd = casaCmd + "\n\napplycal(vis = '"+msName+"',\n"
                casaCmd = casaCmd + "  field = '"+fieldIds2+"',\n"
                casaCmd = casaCmd + "  spw = '"+spwIds1+"',\n"
                casaCmd = casaCmd + "  gaintable = "+str(gainTable)+",\n"
                casaCmd = casaCmd + "  gainfield = "+str(gainField)+",\n"
                casaCmd = casaCmd + "  interp = 'linear',\n"
                casaCmd = casaCmd + "  calwt = T,\n"
                casaCmd = casaCmd + "  flagbackup = F)\n\n"

            casaCmd = casaCmd + "es.getCalWeightStats('"+msName+"') \n"

            #casaCmd = casaCmd + "plotms(vis = '"+msName+"',\n"
            #casaCmd = casaCmd + "    xaxis = 'time',\n"
            #casaCmd = casaCmd + "    yaxis = 'wt',\n"
            #casaCmd = casaCmd + "    spw = '"+spwIds1+"',\n"
            #casaCmd = casaCmd + "    antenna = '*&*',\n"
            #casaCmd = casaCmd + "    coloraxis = 'field',\n"
            #casaCmd = casaCmd + "    plotfile = '"+msName+".weights.png')\n"

        return casaCmd

    def applyAprioriCalTables(self, msName, tsys='', wvr='', antpos=''):

        #casaCmd = 'print "# Application of the WVR, Tsys and antpos cal tables."'
        casaCmd = ''

        if tsys=='' and wvr=='' and antpos=='': sys.exit('ERROR: No cal table specified.')

        gainTable = []
        gainTable.append(tsys)
        gainTable.append(wvr)
        gainTable.append(antpos)
        gainTable = ['%s' %i for i in gainTable if i != '']

        spwInfo = self.getSpwInfo(msName)
        spwIds = spwInfo.keys()
        spwIds1 = ','.join(['%s' %i for i in spwIds])

        if tsys=='':

            casaCmd = casaCmd + "\n\napplycal(vis = '"+msName+"',\n"
            casaCmd = casaCmd + "  spw = '"+spwIds1+"',\n"
            casaCmd = casaCmd + "  gaintable = "+str(gainTable)+",\n"
            if re.search('^3.3', casadef.casa_version) == None:
	        casaCmd = casaCmd + "  interp = 'linear,linear',\n"
	    else:
	        casaCmd = casaCmd + "  interp = 'linear',\n"
            casaCmd = casaCmd + "  calwt = F,\n"
            casaCmd = casaCmd + "  flagbackup = F)\n"

        else:

	    if re.search('^3.3', casadef.casa_version) == None:
	        casaCmd = casaCmd + "\n\nfrom recipes.almahelpers import tsysspwmap\n"
	        casaCmd = casaCmd + "tsysmap = tsysspwmap(vis = '"+msName+"', tsystable = '"+tsys+"')\n\n"

            tb.open(msName+'/FIELD')
            sourceIds = tb.getcol('SOURCE_ID')
            sourceNames = tb.getcol('NAME')
            tb.close()

            sourceIds1 = sorted(dict.fromkeys(sourceIds).keys())

            phaseCal = self.getPhaseCal(msName)

#            if os.path.exists(tsys) == True:
#                  tb.open(tsys)
#                  fieldIds = sorted(dict.fromkeys(tb.getcol('FIELD_ID')).keys())
#                  tb.close()
#            else:
#                  intentSources = self.getIntentsAndSourceNames(msName)
#                  fieldIds = intentSources['CALIBRATE_ATMOSPHERE']['id']

            intentSources = self.getIntentsAndSourceNames(msName)
            fieldIds = intentSources['CALIBRATE_ATMOSPHERE']['id']

            for i in sourceIds1:

                fieldIds1 = (np.where(sourceIds == i))[0]
                sourceName = sourceNames[fieldIds1[0]]

                if len(fieldIds1) > 1:
                    j0 = 0
                    fieldIds2 = str(fieldIds1[j0])
                    for j in range(len(fieldIds1)-1):
                        if fieldIds1[j+1] == fieldIds1[j]+1: continue
                        fieldIds2 = fieldIds2 + '~' + str(fieldIds1[j])
                        j0 = j+1
                        fieldIds2 = fieldIds2 + ',' + str(fieldIds1[j0])
                    fieldIds2 = fieldIds2 + '~' + str(fieldIds1[j+1])
                else:
                    fieldIds2 = str(fieldIds1[0])

                fieldIds3 = [j for j in fieldIds1 if j in fieldIds]

                if len(fieldIds3) > 1: sys.exit('ERROR: Too many Tsys fields per source.')

                if len(fieldIds3) == 0:
                    if i in intentSources['OBSERVE_TARGET']['sourceid']:
                        if phaseCal[sourceName]['phaseCalId'] in fieldIds:
                            fieldIds3 = [phaseCal[sourceName]['phaseCalId']]
                            casaCmd = casaCmd + "\n\n# Note: "+sourceName+" didn't have any Tsys measurement, so I used the one made on "+phaseCal[sourceName]['phaseCalName']+". This is probably Ok."
                        else:
                            casaCmd = casaCmd + "\n\n# Warning: "+sourceName+" didn't have any Tsys measurement, and I couldn't find any close measurement. This is a science target, so this is probably not Ok."
                            continue
                    else:
                        casaCmd = casaCmd + "\n\n# Note: "+sourceName+" didn't have any Tsys measurement, and I couldn't find any close measurement. But this is not a science target, so this is probably Ok."
                        continue

                gainField = []
                for j in range(len(gainTable)): gainField.append('')
                gainField[0] = str(fieldIds3[0])

	        gainSpwMap = []
	        for j in range(len(gainTable)): gainSpwMap.append("[]")
	        gainSpwMap[0] = 'tsysmap'
	        gainSpwMap = ','.join(gainSpwMap)

                casaCmd = casaCmd + "\n\napplycal(vis = '"+msName+"',\n"
                casaCmd = casaCmd + "  field = '"+fieldIds2+"',\n"
                casaCmd = casaCmd + "  spw = '"+spwIds1+"',\n"
                casaCmd = casaCmd + "  gaintable = "+str(gainTable)+",\n"
                casaCmd = casaCmd + "  gainfield = "+str(gainField)+",\n"
                if re.search('^3.3', casadef.casa_version) == None:
	            casaCmd = casaCmd + "  interp = 'linear,linear',\n"
	        else:
	            casaCmd = casaCmd + "  interp = 'linear',\n"
	        if re.search('^3.3', casadef.casa_version) == None: casaCmd = casaCmd + "  spwmap = ["+gainSpwMap+"],\n"
                casaCmd = casaCmd + "  calwt = T,\n"
                casaCmd = casaCmd + "  flagbackup = F)\n\n"

            casaCmd = casaCmd + "es.getCalWeightStats('"+msName+"') \n"

            #casaCmd = casaCmd + "plotms(vis = '"+msName+"',\n"
            #casaCmd = casaCmd + "    xaxis = 'time',\n"
            #casaCmd = casaCmd + "    yaxis = 'wt',\n"
            #casaCmd = casaCmd + "    spw = '"+spwIds1+"',\n"
            #casaCmd = casaCmd + "    antenna = '*&*',\n"
            #casaCmd = casaCmd + "    coloraxis = 'field',\n"
            #casaCmd = casaCmd + "    plotfile = '"+msName+".weights.png')\n"

        return casaCmd

    def getIntentsAndSourceNames(self, msName):

        intentsToSearch = ['CALIBRATE_POINTING', 'CALIBRATE_FOCUS', 'CALIBRATE_BANDPASS', 'CALIBRATE_AMPLI', 'CALIBRATE_PHASE', 'OBSERVE_TARGET', 'CALIBRATE_ATMOSPHERE']

        tb.open(msName+'/STATE')
        obsModes = tb.getcol('OBS_MODE')
        tb.close()

        tb.open(msName+'/FIELD')
        fieldNames = tb.getcol('NAME')
        sourceIds = tb.getcol('SOURCE_ID')
        tb.close()

        tb.open(msName+'/DATA_DESCRIPTION')
        spwIds = tb.getcol('SPECTRAL_WINDOW_ID').tolist()
        tb.close()

        intentSources = {}

        for i in range(len(intentsToSearch)):

            obsModes1 = []
            for j in range(len(obsModes)):
                if re.search(intentsToSearch[i], obsModes[j]) != None:
                    obsModes1.append(j)

            obsModes1 = sorted(dict.fromkeys(obsModes1).keys())

            intentSources[intentsToSearch[i]] = {}

            if len(obsModes1) == 0:

                intentSources[intentsToSearch[i]]['id'] = ['']
                intentSources[intentsToSearch[i]]['idstring'] = ['']
                intentSources[intentsToSearch[i]]['name'] = ['']
                intentSources[intentsToSearch[i]]['spw'] = ['']
                intentSources[intentsToSearch[i]]['sourceid'] = ['']

            else:

                tb.open(msName)

                fieldIds = []
                dataDescIds = []
                for j in obsModes1:
                    tb1 = tb.query('STATE_ID == '+str(j))
                    fieldIds1 = sorted(dict.fromkeys(tb1.getcol('FIELD_ID')).keys())
                    fieldIds.extend(fieldIds1)
                    dataDescIds1 = sorted(dict.fromkeys(tb1.getcol('DATA_DESC_ID')).keys())
                    dataDescIds.extend(dataDescIds1)

                tb.close()

                fieldIds = sorted(dict.fromkeys(fieldIds).keys())
                fieldNames1 = ['%s' %fieldNames[j] for j in fieldIds]
                #fieldNames1 = sorted(dict.fromkeys(fieldNames1).keys())
                dataDescIds = sorted(dict.fromkeys(dataDescIds).keys())
                sourceIds1 = [sourceIds[j] for j in fieldIds]

                intentSources[intentsToSearch[i]]['id'] = fieldIds
                fieldIdStrings = []
                for f in fieldIds:
                    fieldIdStrings.append(str(f))
                intentSources[intentsToSearch[i]]['idstring'] = fieldIdStrings
                intentSources[intentsToSearch[i]]['name'] = fieldNames1
                intentSources[intentsToSearch[i]]['spw'] = [spwIds[k] for k in dataDescIds]
                intentSources[intentsToSearch[i]]['sourceid'] = sourceIds1

        return intentSources

    def listOfIntentsWithSources(self, msName):

        intentSources = self.getIntentsAndSourceNames(msName)

        for i in sorted(intentSources.keys()):
            print i+': '+','.join(sorted(dict.fromkeys(intentSources[i]['name']).keys()))

    def doBandpassCalibration(self, msName, msName1='', chanAvg=0.2, refant='', iHaveSplitMyScienceSpw=False, calTableName=[]):

        #casaCmd = 'print "# Bandpass calibration."\n\n'
        casaCmd = ''

        if msName1 == '': msName1 = msName
        if refant == '': sys.exit('ERROR: No reference antenna specified.')
        if chanAvg > 1: sys.exit('ERROR: The channel averaging bandwidth must be specified as a fraction of the total bandwidth.')

        tb.open(msName+'/FIELD')
        fieldNames = tb.getcol('NAME')
        tb.close()

        intentSources = self.getIntentsAndSourceNames(msName)
        bpassCalId = intentSources['CALIBRATE_BANDPASS']['id']

        if bpassCalId[0] != '':

            if len(bpassCalId) != 1: casaCmd = casaCmd + "# Note: there are more than one bandpass calibrator, I'm picking the first one: "+fieldNames[bpassCalId[0]]+".\n"
            bpassCalId = bpassCalId[0]

        else:

            casaCmd = casaCmd + "# Note: there are no bandpass calibrator, I'm picking a phase calibrator.\n"
            phaseCalId = intentSources['CALIBRATE_PHASE']['id']
            ampCalId = intentSources['CALIBRATE_AMPLI']['id']
            phaseOnlyCalId = [i for i in phaseCalId if i not in ampCalId]
            if len(phaseOnlyCalId) != 1: casaCmd = casaCmd + "# Note: there are more than one phase calibrator, I'm picking the first one: "+fieldNames[phaseOnlyCalId[0]]+".\n"
            bpassCalId = phaseOnlyCalId[0]

        spwInfo = self.getSpwInfo(msName)
        spwIds = spwInfo.keys()

        sciNumChans = []
        for i in spwInfo: sciNumChans.append(spwInfo[i]['numChans'])
        sciNumChans = sorted(dict.fromkeys(sciNumChans).keys())
        if len(sciNumChans) != 1: sys.exit('ERROR: Configuration not supported.')
        sciNumChans = sciNumChans[0]

        spwSpec = ''
        for i in range(len(spwIds)):
            if spwSpec != '': spwSpec = spwSpec+','
            startChan = int((spwInfo[spwIds[i]]['numChans'] / 2.) * (1-chanAvg))
            endChan = int((spwInfo[spwIds[i]]['numChans'] / 2.) * (1+chanAvg))
            if iHaveSplitMyScienceSpw == True:
                  spwSpec = spwSpec+str(i)
            else:
                  spwSpec = spwSpec+str(spwIds[i])
            spwSpec = spwSpec+':'+str(startChan)+'~'+str(endChan)

        calTableName1 = msName1+'.bandpass'
        casaCmd = casaCmd + "os.system('rm -rf %s.ap_pre_bandpass') \n"%(msName1) # Added by CLB
        casaCmd = casaCmd + "\ngaincal(vis = '"+msName1+"',\n"
        casaCmd = casaCmd + "  caltable = '"+msName1+".ap_pre_bandpass',\n"
        casaCmd = casaCmd + "  field = '"+str(bpassCalId)+"', # "+fieldNames[bpassCalId]+"\n"
        casaCmd = casaCmd + "  spw = '"+spwSpec+"',\n"
        casaCmd = casaCmd + "  solint = 'int',\n"
        casaCmd = casaCmd + "  refant = '"+refant+"',\n"
        casaCmd = casaCmd + "  calmode = 'ap')\n"
        casaCmd = casaCmd + "\nes.checkCalTable('"+msName1+".ap_pre_bandpass', msName='"+msName1+"', interactive=False) \n\n"
        casaCmd = casaCmd + "os.system('rm -rf %s.bandpass') \n"%(msName1) # Added by CLB
        casaCmd = casaCmd + "bandpass(vis = '"+msName1+"',\n"
        casaCmd = casaCmd + "  caltable = '"+calTableName1+"',\n"
        casaCmd = casaCmd + "  field = '"+str(bpassCalId)+"', # "+fieldNames[bpassCalId]+"\n"
        casaCmd = casaCmd + "  solint = 'inf',\n"
        casaCmd = casaCmd + "  combine = 'scan',\n"
        casaCmd = casaCmd + "  refant = '"+refant+"',\n"
        casaCmd = casaCmd + "  solnorm = T,\n"
        casaCmd = casaCmd + "  bandtype = 'B',\n"
        casaCmd = casaCmd + "  gaintable = '"+msName1+".ap_pre_bandpass')\n"
        if sciNumChans > 256:
            casaCmd = casaCmd + "os.system('rm -rf %s.bandpass_smooth20flat_ri') \n"%(msName1) # Added by CLB
            casaCmd = casaCmd + "\naU.smoothbandpass('"+calTableName1+"') \n"
        casaCmd = casaCmd + "\nes.checkCalTable('"+calTableName1+"', msName='"+msName1+"', interactive=False) \n"

        if sciNumChans > 256: calTableName1 = calTableName1+'_smooth20flat_ri'
        calTableName.append(calTableName1)

        return casaCmd

    def doAprioriFlagging_old(self, msName):

        casaCmd = 'print "# A priori flagging."\n\n'

        intentsToFlag = ['POINTING', 'FOCUS', 'SIDEBAND_RATIO', 'ATMOSPHERE']

        vm = ValueMapping(msName)
        fullIntentList = vm.uniqueIntents

        scanIntentList = []
        for i in intentsToFlag:
            for j in fullIntentList:
                if re.search(i, j) != None:
                    scanIntentList.append('*'+i+'*')
                    break

        scanIntentList = ','.join(scanIntentList)

        casaCmd = casaCmd + "flagdata(vis = '"+msName+"',\n"
        casaCmd = casaCmd + "  mode = 'manualflag',\n"
        casaCmd = casaCmd + "  autocorr = T,\n"
        casaCmd = casaCmd + "  flagbackup = F)\n\n"
        casaCmd = casaCmd + "flagdata(vis = '"+msName+"',\n"
        casaCmd = casaCmd + "  mode = 'manualflag',\n"
        casaCmd = casaCmd + "  intent = '"+scanIntentList+"',\n"
        casaCmd = casaCmd + "  flagbackup = F)\n"

        return casaCmd

    def doAprioriFlagging(self, msName):

        #casaCmd = 'print "# A priori flagging."\n\n'
        casaCmd = ''

        intentsToFlag = ['POINTING', 'FOCUS', 'SIDEBAND_RATIO', 'ATMOSPHERE']

        vm = ValueMapping(msName)
        fullIntentList = vm.uniqueIntents

        scanIntentList = []
        for i in intentsToFlag:
            for j in fullIntentList:
                if re.search(i, j) != None:
                    scanIntentList.append('*'+i+'*')
                    break

        scanIntentList = ','.join(scanIntentList)

	if re.search('^3.3', casadef.casa_version) == None:
            casaCmd = casaCmd + "tflagdata(vis = '"+msName+"',\n"
            casaCmd = casaCmd + "  mode = 'manual',\n"
            casaCmd = casaCmd + "  autocorr = T,\n"
            casaCmd = casaCmd + "  flagbackup = F)\n\n"
            casaCmd = casaCmd + "tflagdata(vis = '"+msName+"',\n"
            casaCmd = casaCmd + "  mode = 'manual',\n"
            casaCmd = casaCmd + "  intent = '"+scanIntentList+"',\n"
            casaCmd = casaCmd + "  flagbackup = F)\n"
	else:
            casaCmd = casaCmd + "flagdata(vis = '"+msName+"',\n"
            casaCmd = casaCmd + "  mode = 'manualflag',\n"
            casaCmd = casaCmd + "  autocorr = T,\n"
            casaCmd = casaCmd + "  flagbackup = F)\n\n"
            casaCmd = casaCmd + "flagdata(vis = '"+msName+"',\n"
            casaCmd = casaCmd + "  mode = 'manualflag',\n"
            casaCmd = casaCmd + "  intent = '"+scanIntentList+"',\n"
            casaCmd = casaCmd + "  flagbackup = F)\n"

        tb.open(msName)
        tableNames = tb.keywordnames()
        tb.close()

        if 'FLAG_CMD' in tableNames:

            tb.open(msName+'/FLAG_CMD')
            nFlagRows = tb.nrows()
            tb.close()

            if nFlagRows != 0:

	        if re.search('^3.3', casadef.casa_version) == None:
                    casaCmd = casaCmd + "\nflagcmd(vis = '"+msName+"',\n"
                    casaCmd = casaCmd + "  inpmode = 'table',\n"
                    casaCmd = casaCmd + "  action = 'plot')\n\n"
                    casaCmd = casaCmd + "flagcmd(vis = '"+msName+"',\n"
                    casaCmd = casaCmd + "  inpmode = 'table',\n"
                    casaCmd = casaCmd + "  action = 'apply')\n"
		else:
                    casaCmd = casaCmd + "\nflagcmd(vis = '"+msName+"',\n"
                    casaCmd = casaCmd + "  flagmode = 'table',\n"
                    casaCmd = casaCmd + "  optype = 'plot')\n\n"
                    casaCmd = casaCmd + "flagcmd(vis = '"+msName+"',\n"
                    casaCmd = casaCmd + "  flagmode = 'table',\n"
                    casaCmd = casaCmd + "  optype = 'apply')\n"

        return casaCmd

    def doInitialFlagging(self, msName, msName1='', chanEdge=0.0625, thresh=0.2, iHaveSplitMyScienceSpw=False):

        #specLines = {'Neptune': [[340.0, 352.0]], 'Titan': [[229, 232]]}
        specLines = {'Neptune': [[340.0, 352.0]]}

        if msName1 == '': msName1 = msName

        #casaCmd = 'print "# Initial flagging."\n\n'
        casaCmd = ''

	if re.search('^3.3', casadef.casa_version) != None:
            casaCmd = casaCmd + "# Flagging shadowed data\n\n"
            casaCmd = casaCmd + "flagdata(vis = '"+msName1+"',\n"
            casaCmd = casaCmd + "  mode = 'shadow',\n"
            casaCmd = casaCmd + "  flagbackup = F)\n\n"
	else:
            casaCmd = casaCmd + "# Flagging shadowed data\n\n"
            casaCmd = casaCmd + "tflagdata(vis = '"+msName1+"',\n"
            casaCmd = casaCmd + "  mode = 'shadow',\n"
            casaCmd = casaCmd + "  flagbackup = F)\n\n"

        spwInfo = self.getSpwInfo(msName)
        spwIds = spwInfo.keys()
        if iHaveSplitMyScienceSpw == True: spwIds = range(len(spwIds))

        sciNumChans = []
        for i in spwInfo: sciNumChans.append(spwInfo[i]['numChans'])
        sciNumChans = sorted(dict.fromkeys(sciNumChans).keys())
        if len(sciNumChans) != 1: sys.exit('ERROR: Configuration not supported.')
        sciNumChans = sciNumChans[0]

        if sciNumChans <= 256:

            spwSpec = ''
            for i in spwIds:
                if spwSpec != '': spwSpec = spwSpec+','
                spwSpec = spwSpec+str(i)+':0~'+str(long(sciNumChans*chanEdge-1))+';'+str(long(sciNumChans-sciNumChans*chanEdge))+'~'+str(sciNumChans-1)

	    if re.search('^3.3', casadef.casa_version) != None:
                casaCmd = casaCmd + "# Flagging edge channels\n\n"
                casaCmd = casaCmd + "flagdata(vis = '"+msName1+"',\n"
                casaCmd = casaCmd + "  mode = 'manualflag',\n"
                casaCmd = casaCmd + "  spw = '"+spwSpec+"',\n"
                casaCmd = casaCmd + "  flagbackup = F)\n\n"
	    else:
                casaCmd = casaCmd + "# Flagging edge channels\n\n"
                casaCmd = casaCmd + "tflagdata(vis = '"+msName1+"',\n"
                casaCmd = casaCmd + "  mode = 'manual',\n"
                casaCmd = casaCmd + "  spw = '"+spwSpec+"',\n"
                casaCmd = casaCmd + "  flagbackup = F)\n\n"

        intentSources = self.getIntentsAndSourceNames(msName)
        calFieldIds = intentSources['CALIBRATE_AMPLI']['id']
        calFieldNames = intentSources['CALIBRATE_AMPLI']['name']

        for i in range(len(calFieldIds)):
            if calFieldNames[i] in specLines.keys():
                for j in specLines[calFieldNames[i]]:
                    for k in range(len(spwIds)):
                        spwId1 = int(spwInfo.keys()[k])
                        chanRange = getChanRangeFromFreqRange(vis = msName, spwid = spwId1, minf = j[0]*1.e9, maxf = j[1]*1.e9)
                        if chanRange == [-1, -1]: continue
                        if (chanRange[1]-chanRange[0]) / (spwInfo[spwId1]['numChans']*1.) > thresh: print '# Warning: more than '+str(thresh*100)+'% of spw '+str(spwIds[k])+' on '+calFieldNames[i]+' will be flagged due to atmospheric line.'
                        spwSpec = str(spwIds[k])+':'+str(chanRange[0])+'~'+str(chanRange[1])

			if re.search('^3.3', casadef.casa_version) != None:
                            casaCmd = casaCmd + "# Flagging atmospheric line(s)\n\n"
                            casaCmd = casaCmd + "flagdata(vis = '"+msName1+"',\n"
                            casaCmd = casaCmd + "  mode = 'manualflag',\n"
                            casaCmd = casaCmd + "  spw = '"+spwSpec+"',\n"
                            casaCmd = casaCmd + "  field = '"+str(calFieldIds[i])+"',\n"
                            casaCmd = casaCmd + "  flagbackup = F)\n\n"
			else:
                            casaCmd = casaCmd + "# Flagging atmospheric line(s)\n\n"
                            casaCmd = casaCmd + "tflagdata(vis = '"+msName1+"',\n"
                            casaCmd = casaCmd + "  mode = 'manual',\n"
                            casaCmd = casaCmd + "  spw = '"+spwSpec+"',\n"
                            casaCmd = casaCmd + "  field = '"+str(calFieldIds[i])+"',\n"
                            casaCmd = casaCmd + "  flagbackup = F)\n\n"

        return casaCmd

    def generateWVRCalTable_old(self, msName, calTableName=[]):

        casaCmd = 'print "# Generation and time averaging of the WVR calibration table."\n\n'

        intentSources = self.getIntentsAndSourceNames(msName)

        sciSourceId = intentSources['OBSERVE_TARGET']['sourceid']
        sciSourceId1 = dict.fromkeys(sciSourceId).keys()
        if len(sciSourceId1) != 1: casaCmd = casaCmd + "# Warning: there are more than one science target, I'm picking the lowest id. Please check this is right.\n\n"
        sciSourceName = intentSources['OBSERVE_TARGET']['name'][sciSourceId.index(min(sciSourceId1))]
        sciSourceId = min(sciSourceId1)

        phaSourceId = intentSources['CALIBRATE_PHASE']['sourceid']
        ampSourceId = intentSources['CALIBRATE_AMPLI']['sourceid']
        phaOnlySourceId = [i for i in phaSourceId if i not in ampSourceId]
        phaOnlySourceId1 = dict.fromkeys(phaOnlySourceId).keys()
        if len(phaOnlySourceId1) != 1: casaCmd = casaCmd + "# Warning: there are more than one phase calibrator, I'm picking the one with the highest id. Please check this is right.\n\n"
        phaOnlySourceId = max(phaOnlySourceId1)

        tb.open(msName+'/ANTENNA')
        antNames = tb.getcol('NAME')
        tb.close()
        cmAntList = []
        for i in antNames:
              if re.search('CM[0-9]{2}', i) != None: cmAntList.append(i)

        casaCmd = casaCmd + "os.system(" + '"' + "wvrgcal --ms "+msName+" --output "+msName+".wvr --toffset -1 --statsource '"+sciSourceName+"' --segsource --tie '"+str(phaOnlySourceId)+","+str(sciSourceId)+"'"
        if len(cmAntList) != 0:
              for i in cmAntList: casaCmd = casaCmd + " --wvrflag '"+i+"'"
        casaCmd = casaCmd + ' | tee '+msName+'.wvrgcal")\n'

        calTableName1 = msName+'.wvr'

        spwInfo = self.getSpwInfo(msName)
        integTime = []
        for i in spwInfo: integTime.append(spwInfo[i]['integTime'])
        integTime = dict.fromkeys(integTime).keys()
        if len(integTime) != 1: casaCmd = casaCmd + "# Warning: more than one integration time found on science data, I'm picking the lowest value. Please check this is right.\n\n"
        integTime = min(integTime)

        if integTime > 1.152:
              casaCmd = casaCmd + "\nsmoothcal(vis = '"+msName+"',\n"
              casaCmd = casaCmd + "  tablein = '"+msName+".wvr',\n"
              casaCmd = casaCmd + "  caltable = '"+msName+".wvr.smooth',\n"
              casaCmd = casaCmd + "  smoothtype = 'mean',\n"
              casaCmd = casaCmd + "  smoothtime = "+str(integTime)+")\n"
              calTableName1 = msName+'.wvr.smooth'

        calTableName.append(calTableName1)

        return casaCmd

    def generateWVRCalTable(self, msName, calTableName=[]):

        #casaCmd = 'print "# Generation and time averaging of the WVR calibration table."\n\n'
        casaCmd = ''

        intentSources = self.getIntentsAndSourceNames(msName)

        sciSourceId = intentSources['OBSERVE_TARGET']['sourceid']
        sciSourceId1 = dict.fromkeys(sciSourceId).keys()
        #if len(sciSourceId1) != 1: casaCmd = casaCmd + "# Warning: there are more than one science target, I'm picking the lowest id. Please check this is right.\n\n"
        sciSourceName = intentSources['OBSERVE_TARGET']['name'][sciSourceId.index(min(sciSourceId1))]
        sciSourceId = min(sciSourceId1)

        phaseCal = self.getPhaseCal(msName)

        casaCmd = casaCmd + "os.system('rm -rf %s.wvr') \n\n"%(msName)

	if re.search('^3.3', casadef.casa_version) != None:
            casaCmd = casaCmd + "os.system(" + '"' + "wvrgcal --ms "+msName+" --output "+msName+".wvr --toffset -1 --statsource '"+sciSourceName+"' --segsource"
            if len(phaseCal) == 1:
                casaCmd = casaCmd + " --tie '"+str(phaseCal[sciSourceName]['phaseCalId'])+","+str(sciSourceId)+"'"
            casaCmd = casaCmd + ' | tee '+msName+'.wvrgcal")\n\n'
	else:
	    casaCmd = casaCmd + "casalog.setlogfile('"+msName+".wvrgcal')\n\n"
	    casaCmd = casaCmd + "wvrgcal(vis = '"+msName+"',\n"
	    casaCmd = casaCmd + "  caltable = '"+msName+".wvr',\n"
	    casaCmd = casaCmd + "  tie = "+str([','.join([i, phaseCal[i]['phaseCalName']]) for i in phaseCal])+",\n"
	    casaCmd = casaCmd + "  statsource = '"+sciSourceName+"')\n\n"
	    casaCmd = casaCmd + "casalog.setlogfile('')\n\n"

        calTableName1 = msName+'.wvr'

        spwInfo = self.getSpwInfo(msName)
        integTime = []
        for i in spwInfo: integTime.append(spwInfo[i]['integTime'])
        integTime = dict.fromkeys(integTime).keys()
        if len(integTime) != 1: casaCmd = casaCmd + "# Warning: more than one integration time found on science data, I'm picking the lowest value. Please check this is right.\n\n"
        integTime = min(integTime)

        if integTime > 1.152:
              casaCmd = casaCmd + "os.system('rm -rf %s.wvr.smooth') \n\n"%(msName)
              casaCmd = casaCmd + "smoothcal(vis = '"+msName+"',\n"
              casaCmd = casaCmd + "  tablein = '"+msName+".wvr',\n"
              casaCmd = casaCmd + "  caltable = '"+msName+".wvr.smooth',\n"
              casaCmd = casaCmd + "  smoothtype = 'mean',\n"
              casaCmd = casaCmd + "  smoothtime = "+str(integTime)+")\n\n\n"
              calTableName1 = msName+'.wvr.smooth'

        calTableName.append(calTableName1)

# CLB added plots of WVR cross-correlations
        casaCmd = casaCmd + "aU.plotWVRSolutions(caltable='%s', spw='1',\n" %(calTableName1)
        casaCmd = casaCmd + "  yrange=[-180,180],subplot=22, interactive=False,\n" 
        casaCmd = casaCmd + "  figfile='%s') \n\n" %(calTableName1+'.plots/'+calTableName1.split('/')[-1])
        casaCmd = casaCmd + "#Note: If you see wraps in these plots, try changing yrange or unwrap=True \n"

        return casaCmd

    def clearPointingTable(self, msName):

        #casaCmd = 'print "# Clearing the pointing table."\n\n'
        casaCmd = ''

        casaCmd = casaCmd + "tb.open('"+msName+"/POINTING', nomodify = False)\n"
        casaCmd = casaCmd + "a = tb.rownumbers()\n"
        casaCmd = casaCmd + "tb.removerows(a)\n"
        casaCmd = casaCmd + "tb.close()\n"

        return casaCmd

    def getFieldsForSetjy(self, msName):

        setjyModels = ['Mars', 'Jupiter', 'Uranus', 'Neptune', 'Pluto', 'Io', 'Europa', 'Ganymede', 'Callisto', 'Titan', 'Triton', 'Ceres', 'Pallas', 'Vesta', 'Juno', 'Victoria', 'Davida']

        #tb.open(msName+'/FIELD')
        #fieldNames = tb.getcol('NAME')
        #tb.close()

        intentSources = self.getIntentsAndSourceNames(msName)
        ampSourceIds = intentSources['CALIBRATE_AMPLI']['id']
        ampSourceNames = intentSources['CALIBRATE_AMPLI']['name']

        fieldIds = []
        for i in range(len(ampSourceNames)):
              for j in setjyModels:
                    if re.search(j, ampSourceNames[i], re.IGNORECASE) != None: fieldIds.append(ampSourceIds[i])

        return fieldIds

    def runSetjy_old(self, msName, msName1=''):

        casaCmd = 'print "# Putting a model for the spatially-resolved calibrators."\n\n'

        if msName1 == '': msName1 = msName

        fieldIds = self.getFieldsForSetjy(msName)

        if fieldIds == []:
              casaCmd = casaCmd + "# Note: this dataset does not contain Solar system objects, no need to run setjy.\n"
        else:

              tb.open(msName+'/FIELD')
              fieldNames = tb.getcol('NAME')
              tb.close()

              fieldNames = ['%s' %fieldNames[i] for i in fieldIds]
              fieldNames = ','.join(fieldNames)
              fieldIds = ['%s' %i for i in fieldIds]
              fieldIds = ','.join(fieldIds)

              casaCmd = casaCmd + "setjy(vis = '"+msName1+"',\n"
              casaCmd = casaCmd + "  field = '"+fieldIds+"', # "+fieldNames+"\n"
              casaCmd = casaCmd + "  standard = 'Butler-JPL-Horizons 2010')\n"

        return casaCmd

    def runSetjy(self, msName, msName1='', iHaveSplitMyScienceSpw=False):

        if msName1 == '': msName1 = msName

        casaCmd = ''

        fieldIds = self.getFieldsForSetjy(msName)

        if fieldIds != []:

              #casaCmd = 'print "# Putting a model for the flux calibrator(s)."\n\n'

              tb.open(msName+'/FIELD')
              fieldNames = tb.getcol('NAME')
              tb.close()

              fieldNames = ['%s' %fieldNames[i] for i in fieldIds]
              fieldNames = ','.join(fieldNames)
              fieldIds = ['%s' %i for i in fieldIds]
              fieldIds1 = ','.join(fieldIds)

              spwInfo = self.getSpwInfo(msName)
              spwIds = spwInfo.keys()
              if iHaveSplitMyScienceSpw == True: spwIds = range(len(spwIds))
              spwIds = ['%s' %i for i in spwIds]
              spwIds = ','.join(spwIds)

              casaCmd = casaCmd + "setjy(vis = '"+msName1+"',\n"
              casaCmd = casaCmd + "  field = '"+fieldIds1+"', # "+fieldNames+"\n"
              casaCmd = casaCmd + "  spw = '"+spwIds+"',\n"
              casaCmd = casaCmd + "  standard = 'Butler-JPL-Horizons 2010')\n\n"
 
              casaCmd = casaCmd + "os.system('rm -rf %s.setjy.field*.png') \n"%(msName1)
              casaCmd = casaCmd + "for i in "+str(fieldIds)+":\n"
              casaCmd = casaCmd + "  plotms(vis = '"+msName1+"',\n"
              casaCmd = casaCmd + "    xaxis = 'uvdist',\n"
              casaCmd = casaCmd + "    yaxis = 'amp',\n"
              casaCmd = casaCmd + "    ydatacolumn = 'model',\n"
              casaCmd = casaCmd + "    field = i,\n"
              casaCmd = casaCmd + "    spw = '"+spwIds+"',\n"
              casaCmd = casaCmd + "    avgchannel = '9999',\n"
              casaCmd = casaCmd + "    coloraxis = 'spw',\n"
              casaCmd = casaCmd + "    plotfile = '"+msName1+".setjy.field'+i+'.png')\n"

        return casaCmd

    def doGainCalibration_old(self, msName, msName1='', refant='', bandpass='', iHaveSplitMyScienceSpw=False):

        casaCmd = 'print "# Gain calibration."\n\n'

        if refant == '': casaCmd = casaCmd + "# Important note: remember to specify a reference antenna.\n"
        if msName1 == '': msName1 = msName
        if bandpass == '': bandpass = msName1+'.bandpass'

        tb.open(msName+'/FIELD')
        fieldNames = tb.getcol('NAME')
        tb.close()

        fieldIds = range(len(fieldNames))

        intentSources = self.getIntentsAndSourceNames(msName)
        sciFieldIds = intentSources['OBSERVE_TARGET']['id']
        if sciFieldIds[0] == '': sys.exit('ERROR: There seems to be no science field.')

        calFieldIds = [i for i in fieldIds if i not in sciFieldIds]
        if len(calFieldIds) == '': sys.exit('ERROR: There seems to be no calibrator field.')

        calFieldNames = [fieldNames[i] for i in calFieldIds]
        calFieldNames = ','.join(calFieldNames)

        if len(calFieldIds) > 1:
            j0 = 0
            calFieldIds1 = str(calFieldIds[j0])
            for j in range(len(calFieldIds)-1):
                if calFieldIds[j+1] == calFieldIds[j]+1: continue
                calFieldIds1 = calFieldIds1 + '~' + str(calFieldIds[j])
                j0 = j+1
                calFieldIds1 = calFieldIds1 + ',' + str(calFieldIds[j0])
            calFieldIds1 = calFieldIds1 + '~' + str(calFieldIds[j+1])
        else:
            calFieldIds1 = str(calFieldIds[0])

        spwInfo = self.getSpwInfo(msName)
        spwIds = spwInfo.keys()
        if iHaveSplitMyScienceSpw == True: spwIds = range(len(spwIds))

        casaCmd = casaCmd + "\ngaincal(vis = '"+msName1+"',\n"
        casaCmd = casaCmd + "  caltable = '"+msName1+".phase_int',\n"
        casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
        casaCmd = casaCmd + "  solint = 'int',\n"
        casaCmd = casaCmd + "  refant = '"+refant+"',\n"
        casaCmd = casaCmd + "  gaintype = 'G',\n"
        casaCmd = casaCmd + "  calmode = 'p',\n"
        casaCmd = casaCmd + "  gaintable = '"+bandpass+"')\n"
        casaCmd = casaCmd + "\nes.checkCalTable('"+msName1+".phase_int', msName='"+msName1+"', interactive=False) \n\n"
        casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
        casaCmd = casaCmd + "  caltable = '"+msName1+".ampli_inf',\n"
        casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
        casaCmd = casaCmd + "  solint = 'inf',\n"
        casaCmd = casaCmd + "  refant = '"+refant+"',\n"
        casaCmd = casaCmd + "  gaintype = 'T',\n"
        casaCmd = casaCmd + "  calmode = 'a',\n"
        casaCmd = casaCmd + "  gaintable = ['"+bandpass+"', '"+msName1+".phase_int'])\n"
        casaCmd = casaCmd + "\nes.checkCalTable('"+msName1+".ampli_inf', msName='"+msName1+"', interactive=False) \n\n"

        fluxCalId = self.getFieldsForSetjy(msName)
        if len(fluxCalId) == 0:
              print '# Warning: found no Solar system flux calibrator, you will have to run setjy on a quasar. Please check the script before running.'
              casaCmd = casaCmd + "# Warning: found no Solar system flux calibrator, you will have to run setjy on a quasar.\n\n"
              casaCmd = casaCmd + "setjy(vis = '"+msName1+"',\n"
              casaCmd = casaCmd + "  field = '', # put here the field id of the quasar you want to use\n"
              casaCmd = casaCmd + "  fluxdensity = []) # put here the flux of the quasar you want to use, as [I,Q,U,V]\n\n"
              casaCmd = casaCmd + "os.system('rm -rf %s.fluxscale')\n\n" % (msName1) # Added by CLB
              casaCmd = casaCmd + "os.system('rm -rf %s.flux_inf')\n\n" % (msName1) # Added by CLB
              casaCmd = casaCmd + "casalog.setlogfile('"+msName1+".fluxscale')\n\n"
              casaCmd = casaCmd + "fluxscale(vis = '"+msName1+"',\n"
              casaCmd = casaCmd + "  caltable = '"+msName1+".ampli_inf',\n"
              casaCmd = casaCmd + "  fluxtable = '"+msName1+".flux_inf',\n"
              casaCmd = casaCmd + "  reference = '') # put here the field id of the quasar you want to use\n\n"
              casaCmd = casaCmd + "casalog.setlogfile('')\n\n"
        else:
              if len(fluxCalId) > 1: casaCmd = casaCmd + "# Warning: found multiple Solar system flux calibrators, I'm picking the first one.\n\n"
              casaCmd = casaCmd + "os.system('rm -rf %s.fluxscale')\n\n" % (msName1) # Added by CLB
              casaCmd = casaCmd + "os.system('rm -rf %s.flux_inf') \n" % (msName1)  # Added by CLB
              casaCmd = casaCmd + "casalog.setlogfile('"+msName1+".fluxscale')\n\n"
              casaCmd = casaCmd + "fluxscale(vis = '"+msName1+"',\n"
              casaCmd = casaCmd + "  caltable = '"+msName1+".ampli_inf',\n"
              casaCmd = casaCmd + "  fluxtable = '"+msName1+".flux_inf',\n"
              casaCmd = casaCmd + "  reference = '"+str(fluxCalId[0])+"') # "+fieldNames[fluxCalId[0]]+"\n\n"
              casaCmd = casaCmd + "casalog.setlogfile('')\n\n"

        casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
        casaCmd = casaCmd + "  caltable = '"+msName1+".phase_inf',\n"
        casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
        casaCmd = casaCmd + "  solint = 'inf',\n"
        casaCmd = casaCmd + "  refant = '"+refant+"',\n"
        casaCmd = casaCmd + "  gaintype = 'G',\n"
        casaCmd = casaCmd + "  calmode = 'p',\n"
        casaCmd = casaCmd + "  gaintable = '"+bandpass+"')\n"
        casaCmd = casaCmd + "\nes.checkCalTable('"+msName1+".phase_inf', msName='"+msName1+"', interactive=False) \n"

        return casaCmd

    def doGainCalibration(self, msName, msName1='', refant='', bandpass='', iHaveSplitMyScienceSpw=False):

        if msName1 == '': msName1 = msName
        if refant == '': sys.exit('ERROR: No reference antenna specified.')
        if bandpass == '': sys.exit('ERROR: No bandpass cal table specified.')

        tb.open(msName+'/FIELD')
        fieldNames = tb.getcol('NAME')
        tb.close()

        fieldIds = range(len(fieldNames))

        intentSources = self.getIntentsAndSourceNames(msName)
        sciFieldIds = intentSources['OBSERVE_TARGET']['id']
        if sciFieldIds[0] == '': sys.exit('ERROR: There seems to be no science field.')

        calFieldIds = [i for i in fieldIds if i not in sciFieldIds]
        if len(calFieldIds) == '': sys.exit('ERROR: There seems to be no calibrator field.')

        calFieldNames = [fieldNames[i] for i in calFieldIds]
        calFieldNames = ','.join(calFieldNames)

        if len(calFieldIds) > 1:
            j0 = 0
            calFieldIds1 = str(calFieldIds[j0])
            for j in range(len(calFieldIds)-1):
                if calFieldIds[j+1] == calFieldIds[j]+1: continue
                calFieldIds1 = calFieldIds1 + '~' + str(calFieldIds[j])
                j0 = j+1
                calFieldIds1 = calFieldIds1 + ',' + str(calFieldIds[j0])
            calFieldIds1 = calFieldIds1 + '~' + str(calFieldIds[j+1])
        else:
            calFieldIds1 = str(calFieldIds[0])

        spwInfo = self.getSpwInfo(msName)
        spwIds = spwInfo.keys()
        if iHaveSplitMyScienceSpw == True: spwIds = range(len(spwIds))

        fluxCalId = self.getFieldsForSetjy(msName)

        #casaCmd = 'print "# Gain calibration."\n\n'
        casaCmd = ''

        if len(fluxCalId) == 0:
              casaCmd = casaCmd + "os.system('rm -rf %s.phase_int') \n"%(msName1)
              casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
              casaCmd = casaCmd + "  caltable = '"+msName1+".phase_int',\n"
              casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
              casaCmd = casaCmd + "  solint = 'int',\n"
              casaCmd = casaCmd + "  refant = '"+refant+"',\n"
              casaCmd = casaCmd + "  gaintype = 'G',\n"
              casaCmd = casaCmd + "  calmode = 'p',\n"
              casaCmd = casaCmd + "  gaintable = '"+bandpass+"')\n\n"

              casaCmd = casaCmd + "es.checkCalTable('"+msName1+".phase_int', msName='"+msName1+"', interactive=False) \n\n"

              casaCmd = casaCmd + "os.system('rm -rf %s.flux_inf') \n"%(msName1)
              casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
              casaCmd = casaCmd + "  caltable = '"+msName1+".flux_inf',\n"
              casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
              casaCmd = casaCmd + "  solint = 'inf',\n"
              casaCmd = casaCmd + "  refant = '"+refant+"',\n"
              casaCmd = casaCmd + "  gaintype = 'T',\n"
              casaCmd = casaCmd + "  calmode = 'a',\n"
              casaCmd = casaCmd + "  gaintable = ['"+bandpass+"', '"+msName1+".phase_int'])\n\n"

              casaCmd = casaCmd + "es.checkCalTable('"+msName1+".flux_inf', msName='"+msName1+"', interactive=False) \n\n"

              #casaCmd = casaCmd + "casalog.setlogfile('"+msName1+".fluxscale')\n\n"
              #casaCmd = casaCmd + "fluxscale(vis = '"+msName1+"',\n"
              #casaCmd = casaCmd + "  caltable = '"+msName1+".ampli_inf',\n"
              #casaCmd = casaCmd + "  fluxtable = '"+msName1+".flux_inf',\n"
              #casaCmd = casaCmd + "  reference = fluxCalId)\n\n"
              #casaCmd = casaCmd + "casalog.setlogfile('')\n\n"

        else:

              if len(fluxCalId) > 1: casaCmd = casaCmd + "# Note: There are more than one Solar system object in this dataset, I'm using the first one as flux calibrator.\n\n"
              fluxCalId = fluxCalId[0]

              tb.open(msName+'/ANTENNA')
              antList = tb.getcol('NAME')
              tb.close()

              antList1 = self.getAntennasForFluxscale2(msName, fluxCalId=str(fluxCalId), refant=refant)

              if len(antList) == len(antList1):

                  casaCmd = casaCmd + "os.system('rm -rf %s.phase_int') \n"%(msName1)
                  casaCmd = casaCmd + "\ngaincal(vis = '"+msName1+"',\n"
                  casaCmd = casaCmd + "  caltable = '"+msName1+".phase_int',\n"
                  casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
                  casaCmd = casaCmd + "  solint = 'int',\n"
                  casaCmd = casaCmd + "  refant = '"+refant+"',\n"
                  casaCmd = casaCmd + "  gaintype = 'G',\n"
                  casaCmd = casaCmd + "  calmode = 'p',\n"
                  casaCmd = casaCmd + "  gaintable = '"+bandpass+"')\n\n"

                  casaCmd = casaCmd + "es.checkCalTable('"+msName1+".phase_int', msName='"+msName1+"', interactive=False) \n\n"

                  casaCmd = casaCmd + "os.system('rm -rf %s.ampli_inf') \n"%(msName1)
                  casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
                  casaCmd = casaCmd + "  caltable = '"+msName1+".ampli_inf',\n"
                  casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
                  casaCmd = casaCmd + "  solint = 'inf',\n"
                  casaCmd = casaCmd + "  refant = '"+refant+"',\n"
                  casaCmd = casaCmd + "  gaintype = 'T',\n"
                  casaCmd = casaCmd + "  calmode = 'ap',\n"
                  casaCmd = casaCmd + "  gaintable = ['"+bandpass+"', '"+msName1+".phase_int'])\n\n"

                  casaCmd = casaCmd + "es.checkCalTable('"+msName1+".ampli_inf', msName='"+msName1+"', interactive=False) \n\n"

                  casaCmd = casaCmd + "os.system('rm -rf %s.flux_inf') \n"%(msName1) 
                  casaCmd = casaCmd + "os.system('rm -rf %s.fluxscale') \n"%(msName1) 
                  casaCmd = casaCmd + "casalog.setlogfile('"+msName1+".fluxscale')\n\n"
                  casaCmd = casaCmd + "fluxscale(vis = '"+msName1+"',\n"
                  casaCmd = casaCmd + "  caltable = '"+msName1+".ampli_inf',\n"
                  casaCmd = casaCmd + "  fluxtable = '"+msName1+".flux_inf',\n"
                  casaCmd = casaCmd + "  reference = '"+str(fluxCalId)+"') # "+fieldNames[fluxCalId]+"\n\n"
                  casaCmd = casaCmd + "casalog.setlogfile('')\n\n"

              else:

                  phaseCal = self.getPhaseCal(msName)  # Added by CLB
                  phaseCalNames = []  # Added by CLB
                  for i in phaseCal:  # Added by CLB
                      phaseCalNames.append(phaseCal[i]['phaseCalName']) # Added by CLB
                      
                  if len(antList1) < 2:
                      print 'WARNING: THE SOLAR SYSTEM OBJECT SEEMS TO BE EXTREMELY RESOLVED'
                      print 'WARNING: I COULD NOT FIND A SUBSET OF ANTENNAS ON WHICH TO RUN GAINCAL'
                      print 'WARNING: YOU SHOULD LOOK AT THE DATA, AND THEN UPDATE THE SCRIPT'

                  casaCmd += "# Note: the Solar system object used for flux calibration is highly resolved on some baselines.\n"
                  casaCmd += "# Note: we will first determine the flux of the phase calibrator(s) on a subset of antennas.\n\n"

                  casaCmd += "clearcal('%s',field='%s')\n\n" % (msName1, ",".join(map(str,list(np.unique(phaseCalNames))))) # Added by CLB

                  numAntList1 = len(antList1)
                  antList1 = ','.join(antList1)

                  casaCmd = casaCmd + "os.system('rm -rf %s.phase_short_int') \n"%(msName1) 
                  casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
                  casaCmd = casaCmd + "  caltable = '"+msName1+".phase_short_int',\n"
                  casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
                  casaCmd = casaCmd + "  selectdata = T,\n"
                  casaCmd = casaCmd + "  antenna = '"+str(antList1)+"&',\n"
                  casaCmd = casaCmd + "  solint = 'int',\n"
                  casaCmd = casaCmd + "  refant = '"+refant+"',\n"

                  if numAntList1 < 5:
                      casaCmd = casaCmd + "  minblperant = "+str(numAntList1-1)+",\n"
                      casaCmd = casaCmd + "  minsnr = 2.0,\n"

                  casaCmd = casaCmd + "  gaintype = 'G',\n"
                  casaCmd = casaCmd + "  calmode = 'p',\n"
                  casaCmd = casaCmd + "  gaintable = '"+bandpass+"')\n\n"

                  casaCmd = casaCmd + "es.checkCalTable('"+msName1+".phase_short_int', msName='"+msName1+"', interactive=False) \n\n"

                  casaCmd = casaCmd + "os.system('rm -rf %s.ampli_short_inf') \n"%(msName1) 
                  casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
                  casaCmd = casaCmd + "  caltable = '"+msName1+".ampli_short_inf',\n"
                  casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
                  casaCmd = casaCmd + "  selectdata = T,\n"
                  casaCmd = casaCmd + "  antenna = '"+str(antList1)+"&',\n"
                  casaCmd = casaCmd + "  solint = 'inf',\n"
                  casaCmd = casaCmd + "  refant = '"+refant+"',\n"

                  if numAntList1 < 5:
                      casaCmd = casaCmd + "  minblperant = "+str(numAntList1-1)+",\n"
                      casaCmd = casaCmd + "  minsnr = 2.0,\n"

                  casaCmd = casaCmd + "  gaintype = 'T',\n"
                  casaCmd = casaCmd + "  calmode = 'ap',\n"
                  casaCmd = casaCmd + "  gaintable = ['"+bandpass+"', '"+msName1+".phase_short_int'])\n\n"

                  casaCmd = casaCmd + "es.checkCalTable('"+msName1+".ampli_short_inf', msName='"+msName1+"', interactive=False) \n\n"

                  casaCmd = casaCmd + "os.system('rm -rf %s.flux_short_inf') \n"%(msName1) 
                  casaCmd = casaCmd + "os.system('rm -rf %s.fluxscale') \n"%(msName1) 
                  casaCmd = casaCmd + "casalog.setlogfile('"+msName1+".fluxscale')\n\n"
                  casaCmd = casaCmd + "fluxscale(vis = '"+msName1+"',\n"
                  casaCmd = casaCmd + "  caltable = '"+msName1+".ampli_short_inf',\n"
                  casaCmd = casaCmd + "  fluxtable = '"+msName1+".flux_short_inf',\n"
                  casaCmd = casaCmd + "  reference = '"+str(fluxCalId)+"') # "+fieldNames[fluxCalId]+"\n\n"
                  casaCmd = casaCmd + "casalog.setlogfile('')\n\n"

                  casaCmd = casaCmd + "f = open('"+msName1+".fluxscale')\n"
                  casaCmd = casaCmd + "fc = f.readlines()\n"
                  casaCmd = casaCmd + "f.close()\n\n"

                  #phaSourceId = intentSources['CALIBRATE_PHASE']['sourceid']
                  #ampSourceId = intentSources['CALIBRATE_AMPLI']['sourceid']
                  #phaOnlySourceId = [i for i in phaSourceId if i not in ampSourceId]
                  #phaOnlySourceId1 = dict.fromkeys(phaOnlySourceId).keys()
                  #if len(phaOnlySourceId1) != 1: casaCmd = casaCmd + "# Warning: there are more than one phase calibrator, I'm picking the one with the highest id. Please check this is right.\n\n"
                  #phaOnlySourceId = max(phaOnlySourceId1)

                  phaseCal = self.getPhaseCal(msName)
                  phaseCalNames = []
                  for i in phaseCal:
                      phaseCalNames.append(phaseCal[i]['phaseCalName'])

                  casaCmd = casaCmd + "for phaseCalName in "+str(list(set(phaseCalNames)))+":\n"
                  casaCmd = casaCmd + "  for i in range(len(fc)):\n"
                  #casaCmd = casaCmd + "    if re.search('Flux density for '+phaseCalName+' in SpW=[0-9]+ is: [0-9]+\.[0-9]+', fc[i]) != None:\n"
                  casaCmd = casaCmd + "    if fc[i].find('Flux density for '+phaseCalName) != -1 and re.search('in SpW=[0-9]+( \(ref SpW=[0-9]+\))? is: [0-9]+\.[0-9]+', fc[i]) != None:\n"
                  #casaCmd = casaCmd + "      line = (re.findall('in SpW=[0-9]+ is: [0-9]+\.[0-9]+', fc[i]))[0]\n"
                  casaCmd = casaCmd + "      line = (re.search('in SpW=[0-9]+( \(ref SpW=[0-9]+\))? is: [0-9]+\.[0-9]+', fc[i])).group(0)\n"
                  casaCmd = casaCmd + "      spwId = (line.split('='))[1].split()[0]\n"
                  casaCmd = casaCmd + "      flux = float((line.split(':'))[1].split()[0])\n"
                  casaCmd = casaCmd + "      setjy(vis = '"+msName1+"',\n"
                  casaCmd = casaCmd + "        field = phaseCalName.replace(';','*;').split(';')[0],\n"
                  casaCmd = casaCmd + "        spw = spwId,\n"
                  casaCmd = casaCmd + "        fluxdensity = [flux,0,0,0])\n\n"


                  casaCmd = casaCmd + "os.system('rm -rf %s.phase_int') \n"%(msName1) 
                  casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
                  casaCmd = casaCmd + "  caltable = '"+msName1+".phase_int',\n"
                  casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
                  casaCmd = casaCmd + "  solint = 'int',\n"
                  casaCmd = casaCmd + "  refant = '"+refant+"',\n"
                  casaCmd = casaCmd + "  gaintype = 'G',\n"
                  casaCmd = casaCmd + "  calmode = 'p',\n"
                  casaCmd = casaCmd + "  gaintable = '"+bandpass+"')\n\n"

                  casaCmd = casaCmd + "es.checkCalTable('"+msName1+".phase_int', msName='"+msName1+"', interactive=False) \n\n"

                  casaCmd = casaCmd + "os.system('rm -rf %s.flux_inf') \n"%(msName1) 
                  casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
                  casaCmd = casaCmd + "  caltable = '"+msName1+".flux_inf',\n"
                  casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
                  casaCmd = casaCmd + "  solint = 'inf',\n"
                  casaCmd = casaCmd + "  refant = '"+refant+"',\n"
                  casaCmd = casaCmd + "  gaintype = 'T',\n"
                  casaCmd = casaCmd + "  calmode = 'ap',\n"
                  casaCmd = casaCmd + "  gaintable = ['"+bandpass+"', '"+msName1+".phase_int'])\n\n"

                  casaCmd = casaCmd + "es.checkCalTable('"+msName1+".flux_inf', msName='"+msName1+"', interactive=False) \n\n"

                  #casaCmd = casaCmd + "casalog.setlogfile('"+msName1+".fluxscale')\n\n"
                  #casaCmd = casaCmd + "fluxscale(vis = '"+msName1+"',\n"
                  #casaCmd = casaCmd + "  caltable = '"+msName1+".ampli_inf',\n"
                  #casaCmd = casaCmd + "  fluxtable = '"+msName1+".flux_inf',\n"
                  #casaCmd = casaCmd + "  reference = '"+str(phaOnlySourceId)+"') # "+fieldNames[phaOnlySourceId]+"\n\n"
                  #casaCmd = casaCmd + "casalog.setlogfile('')\n\n"

        casaCmd = casaCmd + "os.system('rm -rf %s.phase_inf') \n"%(msName1) 
        casaCmd = casaCmd + "gaincal(vis = '"+msName1+"',\n"
        casaCmd = casaCmd + "  caltable = '"+msName1+".phase_inf',\n"
        casaCmd = casaCmd + "  field = '"+calFieldIds1+"', # "+calFieldNames+"\n"
        casaCmd = casaCmd + "  solint = 'inf',\n"
        casaCmd = casaCmd + "  refant = '"+refant+"',\n"
        casaCmd = casaCmd + "  gaintype = 'G',\n"
        casaCmd = casaCmd + "  calmode = 'p',\n"
        casaCmd = casaCmd + "  gaintable = '"+bandpass+"')\n\n"

        casaCmd = casaCmd + "es.checkCalTable('"+msName1+".phase_inf', msName='"+msName1+"', interactive=False) \n"

        return casaCmd

    def getRefAntenna_old(self, msName, minEl=30):

        tb.open(msName+'/ANTENNA')
        antList = tb.getcol('NAME')
        tb.close()

        baselineLen = getBaselineLengths(msName)

        antInfo = {}

        for i in antList:
            antInfo[i] = {}
            minLen = 100000.
            maxLen = 0.
            for j in baselineLen:
                if i in j[0]:
                    if j[1] < minLen: minLen = j[1]
                    if j[1] > maxLen: maxLen = j[1]
            antInfo[i]['minLen'] = minLen
            antInfo[i]['maxLen'] = maxLen

        refAnt = ''
        maxLen = 100000.

        for i in antInfo:
            if re.search('CM[0-9]{2}', i) != None: continue
            if antInfo[i]['minLen'] < 12. / sin(radians(minEl)): continue
            if antInfo[i]['maxLen'] < maxLen:
                refAnt = i
                maxLen = antInfo[i]['maxLen']

        return refAnt

    def getRefAntenna(self, msName, minEl=30):

        tb.open(msName+'/ANTENNA')
        antList = tb.getcol('NAME').tolist()
        antDiam = tb.getcol('DISH_DIAMETER').tolist()
        tb.close()

        baselineLen = getBaselineLengths(msName)

        antInfo = {}

        for i in antList:
            antInfo[i] = {}
            minLen = 100000.
            maxLen = 0.
            otherAntOnMinLen = ''
            for j in baselineLen:
                if i in j[0]:
                    if j[1] < minLen:
                        minLen = j[1]
                        antList1 = j[0].split('-')
                        antList1.pop(antList1.index(i))
                        otherAntOnMinLen = antList1[0]
                    if j[1] > maxLen: maxLen = j[1]
            antInfo[i]['otherAntOnMinLen'] = otherAntOnMinLen
            antInfo[i]['minLen'] = minLen
            antInfo[i]['maxLen'] = maxLen

        refAnt = ''
        maxLen = 100000.

        for i in antInfo:
            #if re.search('CM[0-9]{2}', i) != None: continue
            diam1 = []
            diam1.append(antDiam[antList.index(i)])
            diam1.append(antDiam[antList.index(antInfo[i]['otherAntOnMinLen'])])
            if antInfo[i]['minLen'] < max(diam1) / sin(radians(minEl)): continue
            if antInfo[i]['maxLen'] < maxLen:
                refAnt = i
                maxLen = antInfo[i]['maxLen']

        return refAnt

    def applyBandpassAndGainCalTables(self, msName, msName1='', bandpass='', phaseForCal='', phaseForSci='', flux='', useForLoop=True):

        #casaCmd = 'print "# Application of the bandpass and gain cal tables."\n\n'
        casaCmd = ''

        if bandpass == '' or phaseForCal == '' or phaseForSci == '' or flux == '': sys.exit('ERROR: Missing table(s).')
        if msName1 == '': msName1 = msName

        tb.open(msName+'/FIELD')
        fieldNames = tb.getcol('NAME')
        tb.close()

        fieldIds = range(len(fieldNames))

        intentSources = self.getIntentsAndSourceNames(msName)
        sciFieldIds = intentSources['OBSERVE_TARGET']['id']
        if sciFieldIds[0] == '': sys.exit('ERROR: There seems to be no science field.')

        calFieldIds = [i for i in fieldIds if i not in sciFieldIds]
        if len(calFieldIds) == '': sys.exit('ERROR: There seems to be no calibrator field.')

        phaseCal = self.getPhaseCal(msName)
        phaseCalFieldIds = []
        for j in phaseCal:
            phaseCalFieldIds.append(phaseCal[j]['phaseCalId'])
        phaseCalFieldIds = sorted(dict.fromkeys(phaseCalFieldIds).keys())
        calFieldIds = [i for i in calFieldIds if i not in phaseCalFieldIds]
        calFieldIds1 = [str(i) for i in calFieldIds]

        calFieldNames = [fieldNames[i] for i in calFieldIds]
        calFieldNames = ','.join(calFieldNames)

        gainTable = []
        gainTable.append(bandpass)
        gainTable.append(phaseForCal)
        gainTable.append(flux)

        if useForLoop == True:
            casaCmd = casaCmd + "for i in "+str(calFieldIds1)+": # "+calFieldNames+"\n"
            casaCmd = casaCmd + "  applycal(vis = '"+msName1+"',\n"
            casaCmd = casaCmd + "    field = i,\n"
            casaCmd = casaCmd + "    gaintable = "+str(gainTable)+",\n"
            casaCmd = casaCmd + "    gainfield = ['', i, i],\n"
            if re.search('^3.3', casadef.casa_version) == None:
                casaCmd = casaCmd + "    interp = 'linear,linear',\n"
            else:
                casaCmd = casaCmd + "    interp = 'linear',\n"
            casaCmd = casaCmd + "    calwt = F,\n"
            casaCmd = casaCmd + "    flagbackup = F)\n"
        else:
            for i in calFieldIds:
                casaCmd = casaCmd + "applycal(vis = '"+msName1+"',\n"
                casaCmd = casaCmd + "  field = '"+str(i)+"', # "+fieldNames[i]+"\n"
                casaCmd = casaCmd + "  gaintable = "+str(gainTable)+",\n"
                casaCmd = casaCmd + "  gainfield = ['', '"+str(i)+"', '"+str(i)+"'],\n"
                if re.search('^3.3', casadef.casa_version) == None:
                    casaCmd = casaCmd + "  interp = 'linear,linear',\n"
                else:
                    casaCmd = casaCmd + "  interp = 'linear',\n"
                casaCmd = casaCmd + "  calwt = F,\n"
                casaCmd = casaCmd + "  flagbackup = F)\n\n"

        gainTable = []
        gainTable.append(bandpass)
        gainTable.append(phaseForSci)
        gainTable.append(flux)

        for i in phaseCalFieldIds:

            sciFieldIds = []
            sciFieldNames = []
            for j in phaseCal:
                if phaseCal[j]['phaseCalId'] == i:
                    sciFieldIds1 = phaseCal[j]['sciFieldIds']
                    for k in sciFieldIds1: sciFieldIds.append(k)
                    sciFieldNames.append(j)
            sciFieldIds = sorted(sciFieldIds)
            sciFieldNames = ','.join(sciFieldNames)

            if len(sciFieldIds) > 1:
                j0 = 0
                sciFieldIds1 = str(sciFieldIds[j0])
                for j in range(len(sciFieldIds)-1):
                    if sciFieldIds[j+1] == sciFieldIds[j]+1: continue
                    sciFieldIds1 = sciFieldIds1 + '~' + str(sciFieldIds[j])
                    j0 = j+1
                    sciFieldIds1 = sciFieldIds1 + ',' + str(sciFieldIds[j0])
                sciFieldIds1 = sciFieldIds1 + '~' + str(sciFieldIds[j+1])
            else:
                sciFieldIds1 = str(sciFieldIds[0])

            casaCmd = casaCmd + "\napplycal(vis = '"+msName1+"',\n"
            casaCmd = casaCmd + "  field = '"+str(i)+","+sciFieldIds1+"', # "+sciFieldNames+"\n"
            casaCmd = casaCmd + "  gaintable = "+str(gainTable)+",\n"
            casaCmd = casaCmd + "  gainfield = ['', '"+str(i)+"', '"+str(i)+"'], # "+fieldNames[i]+"\n"
            if re.search('^3.3', casadef.casa_version) == None:
                casaCmd = casaCmd + "  interp = 'linear,linear',\n"
            else:
                casaCmd = casaCmd + "  interp = 'linear',\n"
            casaCmd = casaCmd + "  calwt = F,\n"
            casaCmd = casaCmd + "  flagbackup = F)\n"

        return casaCmd

    def getPhaseCal(self, msName):

        vm = ValueMapping(msName)
        scanList = vm.uniqueScans

        phaseOnlyScanList = []
        for i in scanList:
            scanIntents = vm.getIntentsForScan(i)
            isPhaseOnlyScan = 1
            for j in scanIntents:
                scanIntent = (j.split('#'))[0]
                if re.search('^CALIBRATE_((PHASE)|(WVR))$', scanIntent) == None:
                    isPhaseOnlyScan = 0
                    break
            if isPhaseOnlyScan == 1: phaseOnlyScanList.append(i)

        if len(phaseOnlyScanList) != 0:

            phaseCal = {}

            for i in phaseOnlyScanList:
                scanField = vm.getFieldsForScan(i)
                if len(scanField) != 1: sys.exit('ERROR: Unexpected number of fields.')
                scanField = scanField[0]
                if scanField not in phaseCal:
                    phaseCal[scanField] = {}
                    phaseCal[scanField]['scans'] = []
                phaseCal[scanField]['scans'].append(i)

            for i in phaseCal:
                scanList = range(min(phaseCal[i]['scans']), max(phaseCal[i]['scans'])+1)
                scanList = [j for j in scanList if j not in phaseCal[i]['scans']]
                fieldList = []
                for j in scanList:
                    fieldList1 = vm.getFieldsForScan(j)
                    for k in fieldList1: fieldList.append(k)
                fieldList = sorted(dict.fromkeys(fieldList).keys())
                phaseCal[i]['fieldNames'] = fieldList

            tb.open(msName+'/FIELD')
            fieldNames = tb.getcol('NAME')
            tb.close()

            intentSources = self.getIntentsAndSourceNames(msName)
            sciFieldIds = intentSources['OBSERVE_TARGET']['id']
            if sciFieldIds[0] == '': sys.exit('ERROR: There seems to be no science field.')
            sciFieldNames = intentSources['OBSERVE_TARGET']['name']

            phaseCal1 = {}

            for i in sciFieldNames:
                phaseCal1[i] = {}
                sciFieldIds1 = np.where(fieldNames == i)[0].tolist()
                sciFieldIds1 = [j for j in sciFieldIds1 if j in sciFieldIds]
                phaseCal1[i]['sciFieldIds'] = sciFieldIds1
                phaseCalNames = []
                phaseCalFirstScan = []
                for j in phaseCal:
                    if i in phaseCal[j]['fieldNames']:
                        phaseCalNames.append(j)
                        phaseCalFirstScan.append(min(phaseCal[j]['scans']))
                phaseCal1[i]['phaseCalName'] = phaseCalNames[phaseCalFirstScan.index(min(phaseCalFirstScan))]
                #phaseCalId = np.where(fieldNames == phaseCal1[i]['phaseCalName'])[0].tolist()
                #phaseCalId = [j for j in phaseCalId if j in intentSources['CALIBRATE_PHASE']['id']]
                phaseCalId = []
                for j in vm.getFieldIdsForFieldName(phaseCal1[i]['phaseCalName']):
                    if min(phaseCalFirstScan) in vm.getScansForFieldID(j): phaseCalId.append(j)
                phaseCalId = sorted(dict.fromkeys(phaseCalId).keys())
                if len(phaseCalId) != 1: sys.exit('ERROR: Possible confusion between field ids.')
                phaseCal1[i]['phaseCalId'] = phaseCalId[0]
                phaseCal1[i]['allPhaseCalNames'] = phaseCalNames

            return phaseCal1

    def doFluxCalibration(self, msNames, fluxFile='allFluxes.txt', refant=''):

        if type(msNames).__name__ == 'str': msNames = [msNames]

        if os.path.exists(fluxFile) == False: sys.exit('ERROR: Flux file '+fluxFile+' does not seem to exist in the current directory.')

        casaCmd = 'print "# Flux calibration of the data."\n\n'

        for i in range(len(msNames)): casaCmd = casaCmd + self.split2(msNames[i], outMsName=msNames[i]+'.cal')

        f = open(fluxFile, 'r')
        fc = f.readlines()
        f.close()

        msName = []
        fieldName = []
        spwId = []
        fluxVal = []

        for line in fc:

            if len(line) == 0 or line.isspace() == True or line.lstrip()[0] == '#': continue

            casaCmd = casaCmd + '# ' + line

            line = line.split('"')
            fieldName.append(line[1])
            line = line[2].split()
            spwId.append(line[0])
            fluxVal.append(line[3])
            msName.append(line[5])

        msName = np.array(msName)
        fieldName = np.array(fieldName)

        casaCmd = casaCmd + '\n'

        for i in range(len(msName)):

            if msName[i] not in msNames: sys.exit('ERROR: Missing dataset.')

            casaCmd = casaCmd + "setjy(vis = '"+msName[i]+".cal',\n"
            casaCmd = casaCmd + "  field = '"+fieldName[i]+"',\n"
            casaCmd = casaCmd + "  spw = '"+spwId[i]+"',\n"
            casaCmd = casaCmd + "  fluxdensity = ["+fluxVal[i]+", 0, 0, 0])\n\n"

        for i in range(len(msNames)):

            calFieldNames = np.unique(fieldName[np.where(msName == msNames[i])])
            calFieldNames = ','.join(calFieldNames)

            #myRefAnt = self.getRefAntenna(msNames[i])
            myRefAnt = refant
            if myRefAnt == '': myRefAnt = self.getRefAntenna(msNames[i])
            casaCmd = casaCmd + "os.system('rm -rf %s.cal.ampli_inf') \n"%(msNames[i])
            casaCmd = casaCmd + "gaincal(vis = '"+msNames[i]+".cal',\n"
            casaCmd = casaCmd + "  caltable = '"+msNames[i]+".cal.ampli_inf',\n"
            casaCmd = casaCmd + "  field = '"+calFieldNames+"',\n"
            casaCmd = casaCmd + "  solint = 'inf',\n"
            casaCmd = casaCmd + "  combine = 'scan',\n"
            casaCmd = casaCmd + "  refant = '"+myRefAnt+"',\n"
            casaCmd = casaCmd + "  gaintype = 'T',\n"
            casaCmd = casaCmd + "  calmode = 'ap')\n\n"

            phaseCal = self.getPhaseCal(msNames[i])

            for sciFieldName in phaseCal.keys():

                sciFieldIds = phaseCal[sciFieldName]['sciFieldIds']

                if len(sciFieldIds) > 1:
                    j0 = 0
                    sciFieldIds1 = str(sciFieldIds[j0])
                    for j in range(len(sciFieldIds)-1):
                        if sciFieldIds[j+1] == sciFieldIds[j]+1: continue
                        sciFieldIds1 = sciFieldIds1 + '~' + str(sciFieldIds[j])
                        j0 = j+1
                        sciFieldIds1 = sciFieldIds1 + ',' + str(sciFieldIds[j0])
                    sciFieldIds1 = sciFieldIds1 + '~' + str(sciFieldIds[j+1])
                else:
                    sciFieldIds1 = str(sciFieldIds[0])

                casaCmd = casaCmd + "applycal(vis = '"+msNames[i]+".cal',\n"
                casaCmd = casaCmd + "  field = '"+str(phaseCal[sciFieldName]['phaseCalId'])+","+sciFieldIds1+"', # "+phaseCal[sciFieldName]['phaseCalName']+","+sciFieldName+"\n"
                casaCmd = casaCmd + "  gaintable = '"+msNames[i]+".cal.ampli_inf',\n"
                casaCmd = casaCmd + "  gainfield = '"+str(phaseCal[sciFieldName]['phaseCalId'])+"', # "+phaseCal[sciFieldName]['phaseCalName']+"\n"
                casaCmd = casaCmd + "  calwt = F,\n"
                casaCmd = casaCmd + "  flagbackup = F)\n\n"

        if len(msNames) > 1:
            casaCmd = casaCmd + 'print "# Concatenating the data."\n\n'
            casaCmd = casaCmd + "concat(vis = "+str([i+'.cal' for i in msNames])+",\n"
            casaCmd = casaCmd + "  concatvis = 'calibrated.ms')\n\n"

        return casaCmd

    def generateFluxFile(self, msNames, outfile='allFluxes.txt'):

        if type(msNames).__name__ == 'str': msNames = [msNames]

        msNames = sorted(msNames)

        spwInfo = self.getSpwInfo(msNames[0])
        spwIds = spwInfo.keys()

        vm = ValueMapping(msNames[0])
        spwMeanFreq = []
        for i in spwIds: spwMeanFreq.append(vm.spwInfo[i]['meanFreq'] / 1.e9)

        fluxVal = {}

        for i in range(len(msNames)):

            spwInfo = self.getSpwInfo(msNames[i])
            if spwInfo.keys() != spwIds: sys.exit('ERROR: The number of spw ids is not the same in all datasets.')

            vm = ValueMapping(msNames[i])
            for j in range(len(spwIds)):
                if vm.spwInfo[spwIds[j]]['meanFreq'] / 1.e9 != spwMeanFreq[j]: print 'WARNING: The mean frequency of spw '+str(spwIds[j])+' of dataset '+msNames[i]+' differs from that of the first dataset.'

            if os.path.exists(msNames[i]+'.fluxscale') != True: continue

            f = open(msNames[i]+'.fluxscale', 'r')
            fc = f.readlines()
            f.close()

            for j in range(len(fc)):
                if re.search('Flux density for .+ in SpW=[0-9]+ is: [0-9]+\.[0-9]+ \+/- [0-9]+\.[0-9]+', fc[j], re.IGNORECASE) != None:
                    line = (re.findall('Flux density for .+ in SpW=[0-9]+ is: [0-9]+\.[0-9]+ \+/- [0-9]+\.[0-9]+', fc[j], re.IGNORECASE))[0]
                    fieldName = (re.findall('for .+ in', line, re.IGNORECASE))[0]
                    fieldName = fieldName[4:len(fieldName)-3]
                    spwId = (re.findall('SpW=[0-9]+', line, re.IGNORECASE))[0]
                    spwId = int((spwId.split('='))[1])
                    flux = (re.findall('is: [0-9]+\.[0-9]+ \+/- [0-9]+\.[0-9]+', line, re.IGNORECASE))[0]
                    flux1 = float(flux.split()[1])
                    error1 = float(flux.split()[3])
                    if fieldName not in fluxVal: fluxVal[fieldName] = {}
                    if spwId not in fluxVal[fieldName]:
                        fluxVal[fieldName][spwId] = {}
                        fluxVal[fieldName][spwId]['flux'] = []
                        fluxVal[fieldName][spwId]['error'] = []
                        fluxVal[fieldName][spwId]['msName'] = []
                    fluxVal[fieldName][spwId]['flux'].append(flux1)
                    fluxVal[fieldName][spwId]['error'].append(error1)
                    fluxVal[fieldName][spwId]['msName'].append(msNames[i])

        #if len(fluxVal) == 0: sys.exit('ERROR: No fluxscale file found.')

        if len(fluxVal) != 0:
            for fieldName in fluxVal:
                for spwId in fluxVal[fieldName]:
                    sum1 = 0.
                    sum2 = 0.
                    for i in range(len(fluxVal[fieldName][spwId]['flux'])):
                        sum1 = sum1 + fluxVal[fieldName][spwId]['flux'][i] / fluxVal[fieldName][spwId]['error'][i]**2
                        sum2 = sum2 + 1. / fluxVal[fieldName][spwId]['error'][i]**2
                    meanFlux = sum1 / sum2
                    #meanFlux = np.mean(fluxVal[fieldName][spwId]['flux'])
                    fluxVal[fieldName][spwId]['meanFlux'] = meanFlux

        obsTime = []
        calFieldNames = []

        for msName in msNames:

            tb.open(msName+'/OBSERVATION')
            obsTimeRange = tb.getcol('TIME_RANGE')
            tb.close()

            obsTime1 = (obsTimeRange[0]+obsTimeRange[1])/2.0
            obsTime1 = ((obsTime1/86400.0)+2400000.5-2440587.5)*86400.0
            obsTime1 = timeUtilities.strftime('%Y-%m-%dT%H:%M:%S', timeUtilities.gmtime(obsTime1))
            obsTime.append(obsTime1)

#            intentSources = self.getIntentsAndSourceNames(msName)
#            bpassCalFieldName = intentSources['CALIBRATE_BANDPASS']['name']

            phaseCal = self.getPhaseCal(msName)
            phaseCalFieldNames = []
            for i in phaseCal.keys(): phaseCalFieldNames.append(phaseCal[i]['allPhaseCalNames'])

            calFieldNames1 = []
#            for i in bpassCalFieldName: calFieldNames1.append(i)
            for i in phaseCalFieldNames:
                for j in i: calFieldNames1.append(j)
            calFieldNames1 = sorted(dict.fromkeys(calFieldNames1).keys())
            calFieldNames.append(calFieldNames1)

        calFieldNames2 = []
        for i in calFieldNames:
            for j in i: calFieldNames2.append(j)
        calFieldNames2 = sorted(dict.fromkeys(calFieldNames2).keys())

        f1 = open(outfile, 'w')

        for fieldName in calFieldNames2:

            print >> f1, ''

            for spwId in spwIds:
                for msName in msNames:

                    fieldName1 = '"'+fieldName+'"'
                    line = '%50s' %fieldName1 + '%5d' %spwId + '%10.2f' %spwMeanFreq[spwIds.index(spwId)]

                    found = 0
                    if fieldName in fluxVal:
                        if spwId in fluxVal[fieldName]:
                            if msName in fluxVal[fieldName][spwId]['msName']:
                                k = fluxVal[fieldName][spwId]['msName'].index(msName)
                                line = line + '%12.5f' %fluxVal[fieldName][spwId]['flux'][k]
                                found = 1
                    if found == 0: line = line + '           -'

                    found = 0
                    if fieldName in fluxVal:
                        if spwId in fluxVal[fieldName]:
                            line = line + '%12.5f' %fluxVal[fieldName][spwId]['meanFlux']
                            found = 1
                    if found == 0: line = line + '           -'

                    k = msNames.index(msName)
                    line = line + '%25s' %obsTime[k] + ' %40s' %msName

                    print >> f1, line

        f1.close()

    def split2(self, msName, msName1='', outMsName='', splitMyScienceSpw=False, timebin=0., iHaveSplitMyScienceSpw=False):

        #casaCmd = 'print "# Splitting the data."\n\n'
        casaCmd = ''

        if msName1 == '': msName1 = msName
        if outMsName == '': outMsName = msName1+'.split'

        spwInfo = self.getSpwInfo(msName)

        spwIds = spwInfo.keys()
        if iHaveSplitMyScienceSpw == True: spwIds = range(len(spwIds))
        spwIds = ['%d' %i for i in spwIds]
        spwIds = ','.join(spwIds)

        numChans = []
        for i in spwInfo.keys(): numChans.append(spwInfo[i]['numChans'])
        if max(numChans) <= 256 and timebin != 0: casaCmd = casaCmd + "# Important note: the correlator mode for this dataset was TDM, you may want not to do any time averaging.\n\n"

        casaCmd = casaCmd + "os.system('rm -rf %s') \n"%(outMsName)
        casaCmd = casaCmd + "split(vis = '"+msName1+"',\n"
        casaCmd = casaCmd + "  outputvis = '"+outMsName+"',\n"
        casaCmd = casaCmd + "  datacolumn = 'corrected',\n"
        if splitMyScienceSpw == True: casaCmd = casaCmd + "  spw = '"+spwIds+"',\n"
        if timebin != 0.: casaCmd = casaCmd + "  timebin = '"+str(timebin)+"s',\n"
        casaCmd = casaCmd + "  keepflags = T)\n\n"

        return casaCmd

    def runCleanOnSource(self, msName, sourceId='', searchForLines=True, chanWid=1, angScale=0, iHaveSplitMyScienceSpw=False):

        import bisect
        import itertools
        import matplotlib.pyplot as plt

        optimumImsize = [216, 256, 360, 432, 640, 800, 1000, 1296, 1600, 2048]

        casaCmd = 'print "# Running clean."\n\n'

        tb.open(msName+'/FIELD')
        sourceIds = tb.getcol('SOURCE_ID')
        fieldNames = tb.getcol('NAME')
        tb.close()

        if sourceId == '':
            casaCmd = casaCmd + '# You have not specified a source Id, I will assume you want to clean the science target(s).\n\n'
            intentSources = self.getIntentsAndSourceNames(msName)
            sourceId = sorted(dict.fromkeys(intentSources['OBSERVE_TARGET']['sourceid']).keys())
            sourceId = ['%s' %i for i in sourceId]

        im.open(msName)
        imInfo = im.advise()
        im.close()

        if imInfo['cell']['unit'] != 'arcsec': sys.exit('ERROR: Cell unit not supported.')

        cellsizeInArcsec = imInfo['cell']['value'] * 0.5
        if cellsizeInArcsec > 0.2:
            cellsizeInArcsec = np.int(cellsizeInArcsec*20)/20.0
        else:
            cellsizeInArcsec = np.int(cellsizeInArcsec*100)/100.0

        if searchForLines == True: specLines = self.searchForLines(msName, chanWid=chanWid, angScale=angScale)

        save_stdout = sys.stdout
        sys.stdout = open('plotmosaic.output', 'w')
        plt.ioff()

        for i in sourceId:

            fieldIds1 = (np.where(sourceIds == int(i)))[0]
            if len(fieldIds1) > 1:
                j0 = 0
                fieldIds2 = str(fieldIds1[j0])
                for j in range(len(fieldIds1)-1):
                    if fieldIds1[j+1] == fieldIds1[j]+1: continue
                    fieldIds2 = fieldIds2 + '~' + str(fieldIds1[j])
                    j0 = j+1
                    fieldIds2 = fieldIds2 + ',' + str(fieldIds1[j0])
                fieldIds2 = fieldIds2 + '~' + str(fieldIds1[j+1])
            else:
                fieldIds2 = str(fieldIds1[0])

            spwInfo = self.getSpwInfo(msName)
            spwIds = spwInfo.keys()
            if iHaveSplitMyScienceSpw == True: spwIds = range(len(spwIds))

            mosaicInfo = plotmosaic(msName, sourceid=i, coord='relative')

            if len(fieldIds1) > 1:
                fieldAtCenter = str(mosaicInfo[0])
            else:
                fieldAtCenter = fieldIds2

            xImsizeInArcsec = abs(mosaicInfo[2] - mosaicInfo[1])
            yImsizeInArcsec = abs(mosaicInfo[4] - mosaicInfo[3])

            xImsizeInCells = xImsizeInArcsec/cellsizeInArcsec
            yImsizeInCells = yImsizeInArcsec/cellsizeInArcsec
            j = bisect.bisect_right(optimumImsize, xImsizeInCells)
            if j != len(optimumImsize): xImsizeInCells = optimumImsize[j]
            j = bisect.bisect_right(optimumImsize, yImsizeInCells)
            if j != len(optimumImsize): yImsizeInCells = optimumImsize[j]

            if searchForLines == True and int(i) in specLines:
                spwIds1 = []
                for j in spwIds:
                    if j not in specLines[int(i)]:
                        spwIds1.append(str(j))
                    else:
                        spwIds2 = []
                        for k in specLines[int(i)][j]:
                            for ij in range(k[0], k[1]+1):
                                spwIds2.append(ij)
                        spwIds3 = [ij for ij in range(spwInfo[j]['numChans']) if ij not in spwIds2]
                        spwIds4 = []
                        for ij, k in itertools.groupby(enumerate(spwIds3), lambda (x, y): y - x):
                            k = list(k)
                            spwIds4.append(str(k[0][1]) + '~' + str(k[-1][1]))
                        spwIds1.append(str(j) + ':' + ';'.join(spwIds4))
                        #spwIds2 = str(j) + ':0~'
                        #for k in specLines[int(i)][j]:
                        #    spwIds2 = spwIds2 + str(k[0]-1) + ';' + str(k[1]+1) + '~'
                        #spwIds2 = spwIds2 + str(spwInfo[j]['numChans'])
                        #spwIds1.append(spwIds2)
            else:
                spwIds1 = ['%s' %j for j in spwIds]

            spwIds1 = ','.join(spwIds1)

            casaCmd = casaCmd + "clean(vis = '"+msName+"',\n"
            casaCmd = casaCmd + "  imagename = '"+msName+".image.continuum.source"+i+"',\n"
            casaCmd = casaCmd + "  field = '"+fieldIds2+"', # "+fieldNames[fieldIds1[0]]+"\n"
            casaCmd = casaCmd + "  spw = '"+spwIds1+"',\n"
            casaCmd = casaCmd + "  mode = 'mfs',\n"
            if len(fieldIds1) > 1: casaCmd = casaCmd + "  imagermode = 'mosaic',\n"
            casaCmd = casaCmd + "  interactive = T,\n"
            casaCmd = casaCmd + "  imsize = ["+str(xImsizeInCells)+", "+str(yImsizeInCells)+"],\n"
            casaCmd = casaCmd + "  cell = '"+str(cellsizeInArcsec)+"arcsec',\n"
            casaCmd = casaCmd + "  phasecenter = "+fieldAtCenter+",\n"
            casaCmd = casaCmd + "  weighting = 'briggs',\n"
            casaCmd = casaCmd + "  robust = 0.5)\n\n"

            if searchForLines == True:
                if int(i) in specLines:
                    for j in specLines[int(i)]:
                        for k in specLines[int(i)][j]:
                            casaCmd = casaCmd + "clean(vis = '"+msName+"',\n"
                            casaCmd = casaCmd + "  imagename = '"+msName+".image.line.source"+i+".spw"+str(j)+".chans"+str(k[0])+"-"+str(k[1])+"',\n"
                            casaCmd = casaCmd + "  field = '"+fieldIds2+"', # "+fieldNames[fieldIds1[0]]+"\n"
                            casaCmd = casaCmd + "  spw = '"+str(j)+"',\n"
                            casaCmd = casaCmd + "  mode = 'channel',\n"
                            casaCmd = casaCmd + "  nchan = "+str((k[1]+1-k[0])/chanWid)+",\n"
                            casaCmd = casaCmd + "  start = "+str(k[0])+",\n"
                            casaCmd = casaCmd + "  width = "+str(chanWid)+",\n"
                            casaCmd = casaCmd + "  interpolation = 'nearest',\n"
                            if len(fieldIds1) > 1: casaCmd = casaCmd + "  imagermode = 'mosaic',\n"
                            casaCmd = casaCmd + "  interactive = T,\n"
                            casaCmd = casaCmd + "  imsize = ["+str(xImsizeInCells)+", "+str(yImsizeInCells)+"],\n"
                            casaCmd = casaCmd + "  cell = '"+str(cellsizeInArcsec)+"arcsec',\n"
                            casaCmd = casaCmd + "  phasecenter = "+fieldAtCenter+",\n"
                            casaCmd = casaCmd + "  weighting = 'briggs',\n"
                            casaCmd = casaCmd + "  robust = 0.5)\n\n"
        
        plt.ion()
        sys.stdout = save_stdout
        os.system('rm plotmosaic.output')

        return casaCmd

    def getRADecStringForField(self, msName, fieldId=''):

        if fieldId == '': sys.exit('ERROR: No field Id specified.')

        fieldRADec = getRADecForField(msName, int(fieldId))
        fieldRA = math.degrees(fieldRADec[0][0])
        fieldDec = math.degrees(fieldRADec[1][0])

        fieldRA = fieldRA/15.
        fieldRAh = int(fieldRA)
        fieldRAm = int(fieldRA%1*60)
        fieldRAs = (fieldRA%1*60)%1*60

        sign = int(abs(fieldDec)/fieldDec)

        fieldDec = abs(fieldDec)
        fieldDecd= int(fieldDec)
        fieldDecm= int(fieldDec%1*60)
        fieldDecs= (fieldDec%1*60)%1*60

        if(sign==+1):
          return "J2000 %02dh%02dm%05.2fs +%02dd%02dm%05.2fs" %(fieldRAh,fieldRAm,fieldRAs,fieldDecd,fieldDecm,fieldDecs)
        elif(sign==-1):
          return "J2000 %02dh%02dm%05.2fs -%02dd%02dm%05.2fs" %(fieldRAh,fieldRAm,fieldRAs,fieldDecd,fieldDecm,fieldDecs)

    def searchForLines_old(self, msName, dataCol='', chanWid=1, angScale=0, cutOff=0.2, minLineWid=3, chanPadding=1):
        vm = ValueMapping(msName)

        tb.open(msName)
        dataColNames = tb.colnames()
        tb.close()

        dataCol = dataCol.lower()

        if dataCol != '':
            if dataCol not in ['data', 'corrected']: sys.exit('ERROR: Data column not supported.')
            if dataCol not in dataColNames: sys.exit('ERROR: Data column not present in ms.')
        else:
            if 'CORRECTED' in dataColNames:
                dataCol = 'corrected'
            else:
                dataCol = 'data'

        tb.open(msName+'/FIELD')
        sourceIds = tb.getcol('SOURCE_ID')
        tb.close()

        intentSources = self.getIntentsAndSourceNames(msName)
        sciSourceIds = intentSources['OBSERVE_TARGET']['sourceid']
        sciSourceIds = sorted(dict.fromkeys(sciSourceIds).keys())

        spwInfo = self.getSpwInfo(msName)
        spwIds = spwInfo.keys()

        specLines = {}

        for i in sciSourceIds:

            fieldIds1 = (np.where(sourceIds == i))[0]
            if len(fieldIds1) > 1:
                j0 = 0
                fieldIds2 = str(fieldIds1[j0])
                for j in range(len(fieldIds1)-1):
                    if fieldIds1[j+1] == fieldIds1[j]+1: continue
                    fieldIds2 = fieldIds2 + '~' + str(fieldIds1[j])
                    j0 = j+1
                    fieldIds2 = fieldIds2 + ',' + str(fieldIds1[j0])
                fieldIds2 = fieldIds2 + '~' + str(fieldIds1[j+1])
                fieldNearCenter = plotmosaic(msName, sourceid=str(i), coord='relative', doplot=False)
                fieldNearCenterRADec = self.getRADecStringForField(msName, fieldId=str(fieldNearCenter))
                msName1 = 'fixvis.ms'
                fixvis(vis = msName, outputvis = msName1, field = fieldIds2, phasecenter = fieldNearCenterRADec)
            else:
                fieldIds2 = str(fieldIds1[0])
                msName1 = msName

            for j in spwIds:

                if angScale != 0:
                    maxFreq = max(vm.spwInfo[j]['chanFreqs']) / 1e9
                    uvRange = '0~'+str((72000. / maxFreq)/angScale)+'m'
                else:
                    uvRange = ''

                sum1 = {}
                count1 = {}
                for k in ['real', 'imag']:
                    sum1[k] = {}
                    count1[k] = {}
                    plotxy(vis = msName1, xaxis = 'chan', yaxis = k, datacolumn = dataCol, selectdata = True, antenna = '*&*', uvrange = uvRange, spw = str(j), field = fieldIds2,
                           averagemode = 'vector', timebin = 'all', crossscans = True, crossbls = True, width = str(chanWid), interactive = False, figfile = 'plotxy.'+k+'.txt')
                    f = open('plotxy.'+k+'.txt', 'r')
                    fc = f.readlines()
                    f.close()
                    for line in fc:
                        line = line.strip()
                        if line == '': continue
                        ele1 = line.split()
                        chan1 = int(ele1[0])
                        comp1 = float(ele1[1])
                        if chan1 not in sum1[k].keys():
                            sum1[k][chan1] = 0.
                            count1[k][chan1] = 0
                        sum1[k][chan1] = sum1[k][chan1] + comp1
                        count1[k][chan1] = count1[k][chan1] + 1
                    os.system('rm -f plotxy.'+k+'.txt')

                chanMax = max([max(sum1['real']), max(sum1['imag'])])

                avg1 = []
                for chan1 in range(chanMax+1):
                    if chan1 in sum1['real'] and chan1 in sum1['imag']:
                        avg1.append(sqrt( (sum1['real'][chan1]/count1['real'][chan1])**2 + (sum1['imag'][chan1]/count1['imag'][chan1])**2))
                    else:
                        avg1.append(0)

                hist, bin_edges = np.histogram(avg1, bins=int(ceil(sqrt(len(avg1)))))
                ij = np.where(hist >= cutOff * max(hist))[0]
                max1 = bin_edges[max(ij)+1]

                k = 0
                while k <= len(avg1)-minLineWid:
                    if avg1[k] > max1:
                        for kl in range(k, len(avg1)):
                            if avg1[kl] <= max1: break
                        if kl-k >= minLineWid:
                            if i not in specLines: specLines[i] = {}
                            if j not in specLines[i]: specLines[i][j] = []
                            specLines[i][j].append([(k-chanPadding)*chanWid, (kl+chanPadding)*chanWid-1])
                        k = kl-1
                    k = k+1

            if os.path.exists('fixvis.ms') == True: os.system('rm -Rf fixvis.ms')

        return specLines

    def searchForLines(self, msName, fieldId='', chanWid=1, angScale=0, cutOff=0.2, minLineWid=3, chanPadding=1):

        import itertools
        import matplotlib.pyplot as plt

        vm = ValueMapping(msName)

        tb.open(msName)
        dataColNames = tb.colnames()
        tb.close()

        if 'CORRECTED_DATA' in dataColNames:
            dataCol = 'corrected'
        else:
            dataCol = 'data'

        tb.open(msName+'/FIELD')
        sourceIds = tb.getcol('SOURCE_ID')
        tb.close()

        intentSources = self.getIntentsAndSourceNames(msName)
        sciSourceIds = intentSources['OBSERVE_TARGET']['sourceid']
        sciSourceIds = sorted(dict.fromkeys(sciSourceIds).keys())

        spwInfo = self.getSpwInfo(msName)
        spwIds = spwInfo.keys()

        specLines = {}

        for i in sciSourceIds:

            fieldIds1 = (np.where(sourceIds == i))[0]
            fieldIds1 = [j for j in fieldIds1 if j in intentSources['OBSERVE_TARGET']['id']]

            for fid in fieldIds1:

                if fieldId != '' and fid != int(fieldId): continue

                for j in spwIds:

                    spwChanFreqs = vm.spwInfo[j]['chanFreqs']

                    if angScale != 0:
                        maxFreq = max(spwChanFreqs) / 1e9
                        uvRange = '0~'+str((72000. / maxFreq)/angScale)+'m'
                    else:
                        uvRange = ''

                    sum1 = {}
                    count1 = {}
                    for k in ['real', 'imag']:
                        sum1[k] = {}
                        count1[k] = {}
                        plotxy(vis = msName, xaxis = 'chan', yaxis = k, datacolumn = dataCol, selectdata = True, antenna = '*&*', uvrange = uvRange, spw = str(j), field = str(fid),
                               averagemode = 'vector', timebin = 'all', crossscans = True, crossbls = True, width = str(chanWid), interactive = False, figfile = 'plotxy.'+k+'.txt')
                        f = open('plotxy.'+k+'.txt', 'r')
                        fc = f.readlines()
                        f.close()
                        for line in fc:
                            line = line.strip()
                            if line == '': continue
                            ele1 = line.split()
                            chan1 = int(ele1[0])
                            comp1 = float(ele1[1])
                            if chan1 not in sum1[k].keys():
                                sum1[k][chan1] = 0.
                                count1[k][chan1] = 0
                            sum1[k][chan1] = sum1[k][chan1] + comp1
                            count1[k][chan1] = count1[k][chan1] + 1
                        os.system('rm -f plotxy.'+k+'.txt')

                    chanMax = max([max(sum1['real']), max(sum1['imag'])])

                    avg1 = []
                    for chan1 in range(chanMax+1):
                        if chan1 in sum1['real'] and chan1 in sum1['imag']:
                            avg1.append(sqrt( (sum1['real'][chan1]/count1['real'][chan1])**2 + (sum1['imag'][chan1]/count1['imag'][chan1])**2))
                        else:
                            avg1.append(0)

                    plt.clf()
                    #plt.plot(range(0, len(avg1)*chanWid, chanWid), avg1)
                    plt.plot([spwChanFreqs[ij]/1.e9 for ij in range(0, len(avg1)*chanWid, chanWid)], avg1)
                    plt.savefig(msName+'.source'+str(i)+'.field'+str(fid)+'.spw'+str(j)+'.png')

                    hist, bin_edges = np.histogram(avg1, bins=int(ceil(sqrt(len(avg1)))))
                    ij = np.where(hist >= cutOff * max(hist))[0]
                    max1 = bin_edges[max(ij)+1]

                    k = 0
                    while k <= len(avg1)-minLineWid:
                        if avg1[k] > max1:
                            for kl in range(k, len(avg1)):
                                if avg1[kl] <= max1: break
                            if kl-k >= minLineWid:
                                specLine = [(k-chanPadding)*chanWid, (kl+chanPadding)*chanWid-1]
                                if i not in specLines: specLines[i] = {}
                                if j not in specLines[i]: specLines[i][j] = []
                                specLines[i][j].append(specLine)
                            k = kl-1
                        k = k+1

                    if i in specLines:
                        if j in specLines[i]:
                            specLines1 = []
                            for k in specLines[i][j]:
                                for kl in range(k[0], k[1]+1): specLines1.append(kl)
                            specLines1 = sorted(dict.fromkeys(specLines1).keys())
                            specLines2 = []
                            for kl, k in itertools.groupby(enumerate(specLines1), lambda (x, y): y - x):
                                k = list(k)
                                specLines2.append([k[0][1], k[-1][1]])
                            specLines[i][j] = specLines2

        return specLines

    def saveFlags(self, msName, name=''):

        if name == '': sys.exit('ERROR: Missing version name.')

        #casaCmd = 'print "# Saving flag column to version '+name+'"\n\n'
        casaCmd = ''

        casaCmd = casaCmd + "flagmanager(vis = '"+msName+"',\n"
        casaCmd = casaCmd + "  mode = 'save',\n"
        casaCmd = casaCmd + "  versionname = '"+name+"')\n\n"

        return casaCmd

    def listobs3(self, msName, figfile = ''):

        import matplotlib.pyplot as plt

        vm = ValueMapping(msName)

        fig = plt.figure(figsize=(14,9))
        ax = fig.add_subplot(111)

        scanList = vm.uniqueScans

        scanTimes = vm.getTimesForScan(1)
        obsStart = scanTimes.min()
        scanTimes = vm.getTimesForScan(scanList[len(scanList)-1])
        obsEnd = scanTimes.max()
        obsDuration = (obsEnd-obsStart) / 60.

        for i in scanList:
            scanTimes = vm.getTimesForScan(i)
            scanStartTime = (scanTimes.min() - obsStart) / 60.
            scanDuration = (scanTimes.max() - scanTimes.min()) / 60.
            scanIntent = vm.getIntentsForScan(i)
            if 'CALIBRATE_SIDEBAND_RATIO#ON_SOURCE' in scanIntent:
                scanColor = 'orange'
                scanY = 30
                ax.broken_barh([(scanStartTime, scanDuration)] , (scanY, 5), facecolors=scanColor)
                ax.annotate(str(i), (scanStartTime, scanY+6))
            if 'CALIBRATE_POINTING#ON_SOURCE' in scanIntent:
                scanColor = 'yellow'
                scanY = 25
                ax.broken_barh([(scanStartTime, scanDuration)] , (scanY, 5), facecolors=scanColor)
                ax.annotate(str(i), (scanStartTime, scanY+6))
            if 'CALIBRATE_ATMOSPHERE#ON_SOURCE' in scanIntent:
                scanColor = 'magenta'
                scanY = 20
                ax.broken_barh([(scanStartTime, scanDuration)] , (scanY, 5), facecolors=scanColor)
                ax.annotate(str(i), (scanStartTime, scanY+6))
            if 'CALIBRATE_AMPLI#ON_SOURCE' in scanIntent:
                scanColor = 'green'
                scanY = 15
                ax.broken_barh([(scanStartTime, scanDuration)] , (scanY, 5), facecolors=scanColor)
                ax.annotate(str(i), (scanStartTime, scanY+6))
            if 'CALIBRATE_BANDPASS#ON_SOURCE' in scanIntent:
                scanColor = 'red'
                scanY = 10
                ax.broken_barh([(scanStartTime, scanDuration)] , (scanY, 5), facecolors=scanColor)
                ax.annotate(str(i), (scanStartTime, scanY+6))
            if 'CALIBRATE_PHASE#ON_SOURCE' in scanIntent:
                scanColor = 'cyan'
                scanY = 5
                ax.broken_barh([(scanStartTime, scanDuration)] , (scanY, 5), facecolors=scanColor)
                ax.annotate(str(i), (scanStartTime, scanY+6))
            if 'OBSERVE_TARGET#ON_SOURCE' in scanIntent:
                scanColor = 'blue'
                scanY = 0
                ax.broken_barh([(scanStartTime, scanDuration)] , (scanY, 5), facecolors=scanColor)
                ax.annotate(str(i), (scanStartTime, scanY+6))

        ax.set_ylim(0, 37.5)
        ax.set_xlim(-1, obsDuration+1)
        ax.set_xlabel('Minutes since start of observation')
        ax.set_yticks([2.5,7.5,12.5,17.5,22.5,27.5, 32.5])
        ax.set_yticklabels(['SCIENCE', 'CAL_PHASE', 'CAL_BANDPASS', 'CAL_AMPLITUDE', 'CAL_ATMOSPHERE', 'CAL_POINTING', 'CAL_SIDEBAND'])
        ax.grid(True)

        obsStart=((obsStart/86400.0)+2400000.5-2440587.5)*86400.0
        obsEnd=((obsEnd/86400.0)+2400000.5-2440587.5)*86400.0
        plt.title('Measurement set = '+msName+' - Start time = '+timeUtilities.strftime('%Y-%m-%dT%H:%M:%S', timeUtilities.gmtime(obsStart))+' - End time = '+timeUtilities.strftime('%Y-%m-%dT%H:%M:%S', timeUtilities.gmtime(obsEnd)), fontsize = 12)

        if figfile != '': fig.savefig(figfile)

    def generateReducScript_old1(self, msNames, minElForRefAnt=30):

        if type(msNames).__name__ == 'str': msNames = [msNames]

        f = open('reducScript.py', 'w')

        print >> f, "es = aU.stuffForScienceDataReduction() \n\n"

        for msName in msNames:
              print >> f, "# Starting reduction of "+msName+"\n"
              myRefAnt = self.getRefAntenna(msName, minEl=minElForRefAnt)
              print >> f, "# Using reference antenna = "+myRefAnt+"\n"
              print >> f, self.runFixPlanets(msName)
              print >> f, self.doAprioriFlagging(msName)
              wvrCalTableName = []
              print >> f, self.generateWVRCalTable(msName, wvrCalTableName)
              tsysCalTableName = []
              print >> f, self.generateTsysCalTable(msName, tsysCalTableName)
              print >> f, self.correctMyAntennaPositions(msName)
              print >> f, self.applyAprioriCalTables(msName, tsys=tsysCalTableName[0], wvr=wvrCalTableName[0], antpos=msName+'.antpos')
              print >> f, self.split2(msName, splitMyScienceSpw=True, timebin=6.048)
              print >> f, self.clearPointingTable(msName+'.split')
              print >> f, "# Add here a flagdata command to flag the shadowed data.\n"
              print >> f, self.runSetjy(msName, msName1=msName+'.split')
              print >> f, self.doBandpassCalibration(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True, refant=myRefAnt)
              print >> f, self.doGainCalibration(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True, refant=myRefAnt)
              print >> f, self.applyBandpassAndGainCalTables(msName, msName1=msName+'.split', bandpass=msName+'.split.bandpass', phaseForCal=msName+'.split.phase_int', phaseForSci=msName+'.split.phase_inf', flux=msName+'.split.flux_inf')

        if len(msNames) > 1:
              print >> f, self.concat2(msNames, msNames1=['%s.split' %i for i in msNames], outMsName='concat.ms', iHaveSplitMyScienceSpw=True, refant=myRefAnt)
              print >> f, self.split2(msNames[0], msName1='concat.ms', outMsName='calibrated.ms', iHaveSplitMyScienceSpw=True)
        else:
              print >> f, self.split2(msNames[0], msName1=msNames[0]+'.split', outMsName='calibrated.ms', iHaveSplitMyScienceSpw=True)

        f.close()

    def generateReducScript_old2(self, msNames, timeBinForFinalData=6.048, minElForRefAnt=30):

        if type(msNames).__name__ == 'str': msNames = [msNames]

        for i in range(len(msNames)):
            msName = re.search('uid___[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+\.ms', msNames[i])
            if msName == None: sys.exit('ERROR: '+msNames[i]+' does not seem to be a standard ms.')
            msName = msName.group(0)
            if os.path.exists(msName) == False: sys.exit('ERROR: '+msNames[i]+' does not seem to exist in the current directory.')
            msNames[i] = msName

        f1 = open('scriptForAprioriCalibration.py', 'w')
        f2 = open('scriptForCalibration.py', 'w')

        print >> f1, "es = aU.stuffForScienceDataReduction() \n\n"
        print >> f2, "es = aU.stuffForScienceDataReduction() \n\n"

        print >> f1, 'print "# A priori calibration"\n'
        for msName in msNames:
              print >> f1, 'print "# Measurement set = '+msName+'"\n'
              print >> f1, self.runFixPlanets(msName)
              print >> f1, "listobs(vis = '"+msName+"',\n  listfile = '"+msName+".listobs')\n\n"
              print >> f1, self.doAprioriFlagging(msName)
              wvrCalTableName = []
              print >> f1, self.generateWVRCalTable(msName, wvrCalTableName)
              tsysCalTableName = []
              print >> f1, self.generateTsysCalTable(msName, tsysCalTableName)
              print >> f1, self.correctMyAntennaPositions(msName)
              print >> f1, self.applyAprioriCalTables(msName, tsys=tsysCalTableName[0], wvr=wvrCalTableName[0], antpos=msName+'.antpos')
              print >> f1, self.split2(msName, splitMyScienceSpw=True, timebin=timeBinForFinalData)

        print >> f2, 'print "# Calibration"\n'
        for msName in msNames:
              print >> f2, 'print "# Measurement set = '+msName+'"\n'
              myRefAnt = self.getRefAntenna(msName, minEl=minElForRefAnt)
              print >> f2, "# Using reference antenna = "+myRefAnt+"\n"
              print >> f2, "listobs(vis = '"+msName+".split',\n  listfile = '"+msName+".split.listobs')\n\n"
              print >> f2, self.clearPointingTable(msName+'.split')
              print >> f2, self.saveFlags(msName+'.split', name='Original')
              print >> f2, "# Flagging shadowed data\n"
              print >> f2, "flagdata(vis = '"+msName+".split',\n  mode = 'shadow',\n  flagbackup = F)\n\n"
              fluxCalId = self.getFieldsForSetjy(msName)
              if len(fluxCalId) == 0: print '# Warning: found no Solar system flux calibrator, you will have to run setjy on a quasar. Please check the script before running.'
              print >> f2, self.runSetjy(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True)
              print >> f2, self.saveFlags(msName+'.split', name='BeforeBandpassCalibration')
              bpassCalTableName = []
              print >> f2, self.doBandpassCalibration(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True, refant=myRefAnt, calTableName=bpassCalTableName)
              print >> f2, self.saveFlags(msName+'.split', name='BeforeGainCalibration')
              print >> f2, self.doGainCalibration(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True, refant=myRefAnt, bandpass=bpassCalTableName[0])
              print >> f2, self.saveFlags(msName+'.split', name='BeforeApplycal')
              print >> f2, self.applyBandpassAndGainCalTables(msName, msName1=msName+'.split', bandpass=bpassCalTableName[0], phaseForCal=msName+'.split.phase_int', phaseForSci=msName+'.split.phase_inf', flux=msName+'.split.flux_inf')

        if len(msNames) > 1:
              print >> f2, self.concat2(msNames, msNames1=['%s.split' %i for i in msNames], outMsName='concat.ms', iHaveSplitMyScienceSpw=True, refant=myRefAnt)
              print >> f2, self.split2(msNames[0], msName1='concat.ms', outMsName='calibrated.ms', iHaveSplitMyScienceSpw=True)
        else:
              print >> f2, self.split2(msNames[0], msName1=msNames[0]+'.split', outMsName='calibrated.ms', iHaveSplitMyScienceSpw=True)

        f1.close()
        f2.close()

    def addReducScriptStep(self, fx=None, stepTitles=None, thisStepTitle=None, stepText=None, indent='  '):
        # prints new step to file fx,
        # returns modified version of stepTitles

        if not (type(fx)==file):
            raise Exception("fx must be a file open for writing")

        if not (type(stepTitles)==dict):
            raise Exception("stepTitles must be a dictionary")
        currentNum = len(stepTitles)

        if not (type(thisStepTitle)==str):
            raise Exception("thisStepTitle must be a string")

        if not (type(stepText)==str):
            raise Exception("stepText must be a string")

        if not (type(indent)==str and len(indent)>1):
            raise Exception("indent must be a string of spaces with a length >= 2")

        stepText = indent + stepText.replace('\n','\n'+indent)

        print >> fx, "# "+thisStepTitle
        print >> fx, "mystep = "+str(currentNum)
        print >> fx, "if(mystep in thesteps):"
        print >> fx, indent+"casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')"
        print >> fx, indent+"print 'Step ', mystep, step_title[mystep]\n"
        print >> fx, stepText+"\n"

        stepTitles[currentNum] = thisStepTitle

        return stepTitles

    def prependReducScriptHeader(self, fx=None, stepTitles=None, scriptTitle='', indent='  '):

        if not (type(fx)==file):
            raise Exception("fx must be a file open for writing")

        if not (type(stepTitles)==dict):
            raise Exception("stepTitles must be a dictionary")

        if not (type(indent)==str and len(indent)>1):
            raise Exception("indent must be a string of spaces with a length >= 2")
        
        
        myheader = "# ALMA Data Reduction Script\n\n" \
                   + "# " + scriptTitle + "\n\n" \
                   + "thesteps = []\n" \
                   + "step_title = "+str(stepTitles).replace("'," , "',\n             " )+'\n\n' \
                   + "try:\n" \
                   + indent + "print 'List of steps to be executed ...', mysteps\n" \
                   + indent + "thesteps = mysteps\n" \
                   + "except:\n" \
                   + indent + "print 'global variable mysteps not set.'\n" \
                   + "if (thesteps==[]):\n" \
                   + indent + "thesteps = range(0,len(step_title))\n" \
                   + indent + "print 'Executing all steps: ', thesteps\n\n" \
                   + "# The Python variable 'mysteps' will control which steps\n" \
                   + "# are executed when you start the script using\n" \
                   + "#   execfile('scriptForCalibration.py')\n" \
                   + "# e.g. setting\n" \
                   + "#   mysteps = [2,3,4]" \
                   + "# before starting the script will make the script execute\n" \
                   + "# only steps 2, 3, and 4\n" \
                   + "# Setting mysteps = [] will make it execute all steps.\n"
                
        sname = fx.name

        fx.close()
        fx = open(sname, 'r+')
        old = fx.read() # read everything in the file
        fx.seek(0) # rewind
        fx.write(myheader +"\n"+ old) # prepend the header

        return True

    def generateReducScript_old3(self, msNames, step='calib', timeBinForFinalData=6.048, chanWid=1, angScale=0):

        if type(msNames).__name__ == 'str': msNames = [msNames]

        for i in range(len(msNames)):

            if os.path.exists(msNames[i]) == False: sys.exit('ERROR: '+msNames[i]+' does not seem to exist in the current directory.')

            spwInfo = self.getSpwInfo(msNames[i])
            spwIds = spwInfo.keys()
            vm = ValueMapping(msNames[i])
            spwScans = vm.getScansForSpw(spwIds[0]).tolist()
            for j in spwIds:
                if vm.getScansForSpw(j).tolist() != spwScans: sys.exit('ERROR: The scans are not the same for all science spws.')

        if step == 'calib':

            for msName in msNames:

                #msName = re.search('uid___[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+\.ms', msNames[i])
                #if msName == None: sys.exit('ERROR: '+msNames[i]+' does not seem to be a standard ms.')
                #msName = msName.group(0)
                #if os.path.exists(msName) == False: sys.exit('ERROR: '+msNames[i]+' does not seem to exist in the current directory.')

                f1 = open(msName+'.scriptForCalibration.py', 'w')
                print >> f1, "import re\n"
                print >> f1, "es = aU.stuffForScienceDataReduction() \n\n"

                myRefAnt = self.getRefAntenna(msName)
                print >> f1, "# Using reference antenna = "+myRefAnt+"\n"

                print >> f1, 'print "# A priori calibration"\n'
                print >> f1, self.runFixPlanets(msName)
                print >> f1, "listobs(vis = '"+msName+"',\n  listfile = '"+msName+".listobs')\n\n"
                print >> f1, self.doAprioriFlagging(msName)
                wvrCalTableName = []
                print >> f1, self.generateWVRCalTable(msName, wvrCalTableName)
                tsysCalTableName = []
                print >> f1, self.generateTsysCalTable(msName, tsysCalTableName)
                print >> f1, self.correctMyAntennaPositions(msName)
                print >> f1, self.applyAprioriCalTables(msName, tsys=tsysCalTableName[0], wvr=wvrCalTableName[0], antpos=msName+'.antpos')
                print >> f1, self.split2(msName, splitMyScienceSpw=True, timebin=timeBinForFinalData)

                print >> f1, 'print "# Calibration"\n'
                print >> f1, "listobs(vis = '"+msName+".split',\n  listfile = '"+msName+".split.listobs')\n\n"
                print >> f1, self.clearPointingTable(msName+'.split')
                print >> f1, self.saveFlags(msName+'.split', name='Original')
                print >> f1, self.doInitialFlagging(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True)
                #print >> f1, "# Flagging shadowed data\n"
                #print >> f1, "flagdata(vis = '"+msName+".split',\n  mode = 'shadow',\n  flagbackup = F)\n\n"
                print >> f1, self.runSetjy(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True)
                print >> f1, self.saveFlags(msName+'.split', name='BeforeBandpassCalibration')
                bpassCalTableName = []
                print >> f1, self.doBandpassCalibration(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True, refant=myRefAnt, calTableName=bpassCalTableName)
                print >> f1, self.saveFlags(msName+'.split', name='BeforeGainCalibration')
                print >> f1, self.doGainCalibration(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True, refant=myRefAnt, bandpass=bpassCalTableName[0])
                print >> f1, self.saveFlags(msName+'.split', name='BeforeApplycal')
                print >> f1, self.applyBandpassAndGainCalTables(msName, msName1=msName+'.split', bandpass=bpassCalTableName[0], phaseForCal=msName+'.split.phase_int', phaseForSci=msName+'.split.phase_inf', flux=msName+'.split.flux_inf')

                f1.close()

        if step == 'fluxcal':

            if os.path.exists('allFluxes.txt') == False:
                self.generateFluxFile(msNames)
            else:
                print 'File allFluxes.txt already exists, it will be loaded.'

            f1 = open('scriptForFluxCalibration.py', 'w')
            print >> f1, self.doFluxCalibration(msNames)
            if len(msNames) > 1:
                print >> f1, "# Concatenating datasets\n"
                print >> f1, "concat(vis = "+str(msNames)+",\n  concatvis = 'concat.ms')\n\n"
            f1.close()

        if step == 'imaging':

            if len(msNames) != 1: sys.exit('ERROR: Imaging of multiple datasets is not supported.')
            msNames = msNames[0]

            f1 = open('scriptForImaging.py', 'w')
            print >> f1, self.runCleanOnSource(msNames, chanWid=chanWid, angScale=angScale)
            f1.close()

    def generateReducScript(self, msNames, step='calib', corrAntPos=True, timeBinForFinalData=6.048, refant='', chanWid=1, angScale=0):

	if re.search('^3.3', casadef.casa_version) == None:
	    print 'WARNING: You are currently running another version of CASA than 3.3. Probably 3.4.'
	    print 'WARNING: The scripts have been ported, but for a bit of time, please be careful with the output.'
	    print 'WARNING: If you observe any issue or strange behavior, please send an email to Eric V.'
            raw_input('Hit a key to proceed.')

        if type(msNames).__name__ == 'str': msNames = [msNames]

        for i in range(len(msNames)):

            if os.path.exists(msNames[i]) == False: sys.exit('ERROR: '+msNames[i]+' does not seem to exist in the current directory.')

            spwInfo = self.getSpwInfo(msNames[i])
            spwIds = spwInfo.keys()
            vm = ValueMapping(msNames[i])
            spwScans = vm.getScansForSpw(spwIds[0]).tolist()
            for j in spwIds:
                if vm.getScansForSpw(j).tolist() != spwScans:
                    print 'WARNING: The scans are not the same for all science spws.'
                    print 'WARNING: The script generator is not compatible with this, it will very likely fail.'
                    print 'WARNING: If it does not fail, do not expect the reduction script to be good. Please check it carefully.'
                    raw_input('Hit a key to proceed.')

        if step == 'wvr':

            for msName in msNames:

                f1 = open(msName+'.scriptForWVRCalibration.py', 'w')
                print >> f1, "es = aU.stuffForScienceDataReduction() \n\n"

                myRefAnt = refant
                if myRefAnt == '': myRefAnt = self.getRefAntenna(msName)
                print >> f1, "# Using reference antenna = "+myRefAnt+"\n"

                print >> f1, self.doAprioriFlagging(msName)
                print >> f1, self.generateWVRCalTable(msName)
                print >> f1, "es.wvr_stat(ms1='"+msName+"', refAnt='"+myRefAnt+"', qa2_output_dir='./')\n"

                f1.close()

        if step == 'calib':

            mystepdict = {}
            mystepindent = "  "

            for msName in msNames:

                #msName = re.search('uid___[a-zA-Z0-9]+_[a-zA-Z0-9]+_[a-zA-Z0-9]+\.ms', msNames[i])
                #if msName == None: sys.exit('ERROR: '+msNames[i]+' does not seem to be a standard ms.')
                #msName = msName.group(0)
                #if os.path.exists(msName) == False: sys.exit('ERROR: '+msNames[i]+' does not seem to exist in the current directory.')

                f1 = open(msName+'.scriptForCalibration.py', 'w')
                print >> f1, "import re\n"
                print >> f1, "es = aU.stuffForScienceDataReduction() \n\n"

                myRefAnt = refant
                if myRefAnt == '': myRefAnt = self.getRefAntenna(msName)
                print >> f1, "# Using reference antenna = "+myRefAnt+"\n"

                print >> f1, 'print "# A priori calibration"\n'
                stext = self.runFixPlanets(msName)
                if stext != None: self.addReducScriptStep(f1, mystepdict, "Running fixplanets on fields with 0,0 coordinates", stext, mystepindent)

                stext = "os.system('rm -rf %s.listobs')\n" %(msName) # Added by CLB
                stext += "listobs(vis = '"+msName+"',\n  listfile = '"+msName+".listobs')\n\n" # Modified by CLB
                self.addReducScriptStep(f1, mystepdict, "listobs", stext, mystepindent)
                stext = self.doAprioriFlagging(msName)
                self.addReducScriptStep(f1, mystepdict, "A priori flagging", stext, mystepindent)
                wvrCalTableName = []
                stext = self.generateWVRCalTable(msName, wvrCalTableName)
                self.addReducScriptStep(f1, mystepdict, "Generation and time averaging of the WVR cal table", stext, mystepindent)
                tsysCalTableName = []
                stext = self.generateTsysCalTable(msName, tsysCalTableName)
                self.addReducScriptStep(f1, mystepdict, "Generation of the Tsys cal table", stext, mystepindent)

                if corrAntPos == True:
                    stext = self.correctMyAntennaPositions(msName)
                    self.addReducScriptStep(f1, mystepdict, "Generation of the antenna position cal table", stext, mystepindent)
                    stext = self.applyAprioriCalTables(msName, tsys=tsysCalTableName[0], wvr=wvrCalTableName[0], antpos=msName+'.antpos')
                    self.addReducScriptStep(f1, mystepdict, "Application of the WVR, Tsys and antpos cal tables", stext, mystepindent)
                else:
                    stext = self.applyAprioriCalTables(msName, tsys=tsysCalTableName[0], wvr=wvrCalTableName[0])
                    self.addReducScriptStep(f1, mystepdict, "Application of the WVR and Tsys cal tables", stext, mystepindent)

                stext = self.split2(msName, splitMyScienceSpw=True, timebin=timeBinForFinalData)
                self.addReducScriptStep(f1, mystepdict, "Split out science SPWs and time average", stext, mystepindent)

                print >> f1, 'print "# Calibration"\n'
                stext = "os.system('rm -rf %s.split.listobs')\n" % (msName) # Added by CLB
                # following line changed to += by CLB
                stext += "listobs(vis = '"+msName+".split',\n  listfile = '"+msName+".split.listobs')\n\n" \
                    + self.clearPointingTable(msName+'.split') \
                    + self.saveFlags(msName+'.split', name='Original')
                self.addReducScriptStep(f1, mystepdict, "Listobs, clear pointing table, and save original flags", stext, mystepindent)
                stext = self.doInitialFlagging(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True)
                self.addReducScriptStep(f1, mystepdict, "Initial flagging", stext, mystepindent)
                stext = self.runSetjy(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True)
                self.addReducScriptStep(f1, mystepdict, "Putting a model for the flux calibrator(s)", stext, mystepindent)
                stext = self.saveFlags(msName+'.split', name='BeforeBandpassCalibration')
                self.addReducScriptStep(f1, mystepdict, "Save flags before bandpass cal", stext, mystepindent)
                bpassCalTableName = []
                stext = self.doBandpassCalibration(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True, refant=myRefAnt, calTableName=bpassCalTableName)
                self.addReducScriptStep(f1, mystepdict, "Bandpass calibration", stext, mystepindent)
                stext = self.saveFlags(msName+'.split', name='BeforeGainCalibration')
                self.addReducScriptStep(f1, mystepdict, "Save flags before gain cal", stext, mystepindent)
                stext = self.doGainCalibration(msName, msName1=msName+'.split', iHaveSplitMyScienceSpw=True, refant=myRefAnt, bandpass=bpassCalTableName[0])
                self.addReducScriptStep(f1, mystepdict, "Gain calibration", stext, mystepindent)
                stext = self.saveFlags(msName+'.split', name='BeforeApplycal')
                self.addReducScriptStep(f1, mystepdict, "Save flags before applycal cal", stext, mystepindent)
                stext = self.applyBandpassAndGainCalTables(msName, msName1=msName+'.split', bandpass=bpassCalTableName[0], phaseForCal=msName+'.split.phase_int', phaseForSci=msName+'.split.phase_inf', flux=msName+'.split.flux_inf')
                self.addReducScriptStep(f1, mystepdict, "Application of the bandpass and gain cal tables", stext, mystepindent)

                self.prependReducScriptHeader(f1, mystepdict, "Calibration", mystepindent)

                f1.close()

        if step == 'fluxcal':

            if os.path.exists('allFluxes.txt') == False:
                self.generateFluxFile(msNames)
            else:
                print 'File allFluxes.txt already exists, it will be loaded.'

            myRefAnt = refant
            f1 = open('scriptForFluxCalibration.py', 'w')
            print >> f1, self.doFluxCalibration(msNames, refant=myRefAnt)
            f1.close()

        if step == 'imaging':

            if len(msNames) > 1: sys.exit('ERROR: the script generator does not support (yet) imaging of multiple datasets.')
            msNames = msNames[0]

            f1 = open('scriptForImaging.py', 'w')
            print >> f1, self.runCleanOnSource(msNames, chanWid=chanWid, angScale=angScale)
            f1.close()

    def ampcal_uvdist(self, ms2='', qa2_output_dir=''):

        """
        qa2_scripts/ampcal_uvdist.py

        This program plots the calibrated and the model visibility
        data for the selected source (usually a solar system object)


        INPUTS NEEDED:

           Assumes data base is ms2
           But need ss_object as name of source


        OUTPUTS:

        The calibrated and model amplitudes for the selected field
        number is placed in

           qa2_output_dir+'+ss_object+'_.png'


        USEAGE: assumes ms2 as the visibility data set.  Program looks for
        the relevant files.

        ss_object = 'Callisto'
        execfile ('ampcal_uvdist.py')

        """
        ss_object = self.getIntentsAndSourceNames(ms2)['CALIBRATE_AMPLI']['name'][0]

        plotxy(vis=ms2,
               xaxis = 'uvdist', yaxis = 'amp',
               field = ss_object,
               overplot = False, datacolumn = 'model',
               averagemode = 'vector',
               timebin = '30', width = '4000',
               interactive = False)

        plotxy(vis=ms2,
               xaxis = 'uvdist', yaxis = 'amp',
               field = ss_object,
               overplot = True, datacolumn = 'corrected',
               averagemode = 'vector',
               timebin = '30', width = '4000', plotsymbol = ',',
               interactive = False,
               figfile = qa2_output_dir+'ampcal_uvdist.png')

    def ant_amp_temporal(self, ms2='', phase_cal='', caltable='', qa2_output_dir=''):

        """
        ant_amp_temporal.py

        This program plots the antenna-based gains obtained from the
        phase calibrators---used to calibrate the target


        INPUTS NEEDED:

           Assumes original table is in ms2
           caltable must be specified
           phase_cal should be specified


        OUTPUTS:

        The phase plots for each antenna are placed in

           ant_amp_temporal.png'


        USEAGE: assumes ms2 as the visibility data set.  Program looks for
        the relevant files.

        phase_cal = '2'
        caltable = ms2+'.flux_inf'
        execfile ('ant_amp_temporal.py')

        """

        import numpy
        import pylab as pl


        #  Plot calibrator gains

        #  Get antenna paramters

        tb.open(ms2+'/ANTENNA')
        ant_names=tb.getcol('NAME')
        nant = len(ant_names)
        tb.close()

        #  Get gaintable

        tb.open(caltable)
        tb1 = tb.query('FIELD_ID == '+phase_cal)
        time=tb1.getcol('TIME')
        time = time-time[0]+5.0
        ant = tb1.getcol('ANTENNA1')
        gain = tb1.getcol('GAIN')
        spw = tb1.getcol('CAL_DESC_ID')
        field = tb1.getcol('FIELD_ID')
        g = numpy.abs(gain)
        gg = g[0][0]
        np=len(ant)
        tb.close()
        spw_un = numpy.unique(spw)
        ant_un = numpy.unique(ant)
        nspw = len(spw_un)
        nant = len(ant_un)
        ggf = []
        for i in range (0,np):
            if gg[i] != 1.0: ggf.append(gg[i])


        gmin = numpy.min(ggf)
        gmax = numpy.max(ggf)
        gmed = numpy.median(ggf)
        gmin = 0.8*gmin
        if gmax > 5.0*gmed:
            gmax = 5.0*gmed

        gmin = 0.0
        gmax = 1.1*gmax
        tmin = numpy.min(time)
        tmax = numpy.max(time)
        tdiff = numpy.int((0.8*tmax-tmin)/1000.0) * 1000.0
        tmin = tmin - (tmax-tmin)*0.2
        tmax = tmax * 1.2
        #print 'range ', gmin, gmax

        #  Set up plots

        #  Formatting of plots
        nsub3 = 0
        if nant <31: nsub1 = 6; nsub2 = 5

        if nant <26: nsub1 = 5; nsub2 = 5

        if nant <21: nsub1 = 4; nsub2 = 5

        if nant <17: nsub1 = 4; nsub2 = 4

        if nant <13: nsub1 = 3; nsub2 = 4

        #  Loop over antenna
        for iant in range(0,nant):
            nsub3 = nsub3 + 1
            for ispw in range(0,nspw):
                gamp = []
                tamp = []
                for pt in range(0,np):
                    if (ant[pt]==iant) and (spw[pt]==ispw) and (gg[pt]!= 1.0):
                        gamp.append(gg[pt])
                        tamp.append(time[pt])
                    #
                #
                pl.subplot(nsub1,nsub2,nsub3)
                if ispw == 0: pl.plot(tamp,gamp,'bo')

                if ispw == 1: pl.plot(tamp,gamp,'ro')

                if ispw == 2: pl.plot(tamp,gamp,'go')

                if ispw == 3: pl.plot(tamp,gamp,'yo')

                pl.ylim (gmin,gmax)
                pl.xlim (tmin,tmax)
                if nsub3 != 1: pl.yticks([])

                if nsub3 != nant:
                    pl.xticks([])
                else:
                    pl.xticks([0.0,tdiff])

                pl.title('AMP '+ant_names[iant])


        pl.savefig(qa2_output_dir+'ant_amp_temporal.png')

    def ant_gain_check(self, ms2='', qa2_output_dir=''):

        """
        ant_gain_check.py

        This is an experimental python script that determines the statistics
        of the pre-bandpass gain levels calibration.  It is meant to determine
        any bad antennas/pol/spws.

        INPUTS NEEDED:

           None.  assumes table is in ms2+'.ap_pre_bandpass'

        OUTPUTS:

        The ascii output file lists the relative gains of all data streams

        qa2_output_dir+'ant_gain_check.txt'

        USEAGE: assumes ms2 as the visibility data set

        execfile ('ant_gain_check.py')

        """

        import numpy

        #  Get antenna paramters

        tb.open(ms2+'/ANTENNA')
        ant_names=tb.getcol('NAME')
        ant_pos=tb.getcol('POSITION')
        nant = len(ant_names)
        tb.close()

        #  Get gaintable

        caltable = ms2+'.ap_pre_bandpass'
        tb.open(caltable)
        time=tb.getcol('TIME')
        time = time-time[0]+5.0
        ant = tb.getcol('ANTENNA1')
        gain = tb.getcol('GAIN')
        spw = tb.getcol('CAL_DESC_ID')
        g = numpy.abs(gain)
        np=len(ant)
        tb.close()
        spw_un = numpy.unique(spw)
        ant_un = numpy.unique(ant)
        nspw = len(spw_un)
        nant = len(ant_un)
        #  Get average g
        npol = g.shape[0]
        npts = g.shape[2]
        gg = []
        for i in range(0,npol):
            for j in range(0,npts):
                if g[i][0][j] != 1.0: gg.append(g[i][0][j])


        gavg = numpy.median(gg)

        #   Open file for writing
        zfileRes = qa2_output_dir+'ant_gain_check.txt'
        os.system('rm '+zfileRes)
        f = open (zfileRes, 'w')
        print 'opening file '+zfileRes
        f.write('\n\n')
        f.write('************************************************************************** \n')
        f.write('\n')
        f.write('                    MEDIAN GAIN VALUE = %8.3F \n\n' % (gavg))
        f.write('                RELATIVE ANTENNA GAIN FOR BANDPASS OBSERVATION \n\n')
        f.write('     Antenna          SPW0                SPW1                SPW2                SPW3\n')
        f.write('                   X        Y          X        Y          X        Y          X        Y\n')

        #  get a gain,phase for each ant, spw stream

        g_xall = []
        g_yall = []
        low_gain = 0.90
        high_gain = 1.10

        #  Get gain and phase average and rms for each stream (both pols)

        for iant in range(0,nant):
            gant_xavg = []
            gant_yavg = []
            gant_xrms = []
            gant_yrms = []
            g_xstar = []
            g_ystar = []
            for ispw in range(0,nspw):
                g_x = []
                g_y = []
                for j in range(0,np):
                    if (iant == ant[j]) & (ispw == spw[j]):
                        if g[0][0][j] != 1.0:
                            g_x.append(g[0][0][j])

                        if g[1][0][j] != 1.0:
                            g_y.append(g[1][0][j])

                    g_xavg = numpy.average(g_x)/gavg
                    g_xvar = numpy.var(g_x)/gavg
                    g_yavg = numpy.average(g_y)/gavg
                    g_yvar = numpy.var(g_y)/gavg

                gant_xavg.append(g_xavg)
                if (g_xavg > high_gain) or (g_xavg < low_gain):
                    g_xstar.append('**')
                else:
                    g_xstar.append('  ')

                gant_xrms.append(numpy.sqrt(g_xvar))
                gant_yavg.append(g_yavg)
                gant_yrms.append(numpy.sqrt(g_yvar))
                if (g_yavg > high_gain) or (g_yavg < low_gain):
                    g_ystar.append('**')
                else:
                    g_ystar.append('  ')



            f.write('    %3d-%4s  %6.2f%2s  %6.2f%2s ' % (iant, ant_names[iant], gant_xavg[0], g_xstar[0], gant_yavg[0], g_ystar[0]))
            for ispw in range (1,nspw-1):
                    f.write (' %6.2f%2s  %6.2f%2s ' % (gant_xavg[ispw], g_xstar[ispw], gant_yavg[ispw], g_ystar[ispw]))

            f.write (' %6.2f%2s  %6.2f%2s \n' % (gant_xavg[nspw-1], g_xstar[nspw-1], gant_yavg[nspw-1], g_ystar[nspw-1]))

        f.write ('\n ** means outside of normalized range: %6.2f to %6.2f \n ' % (low_gain, high_gain))

        f.close()
        os.system('cat '+zfileRes)

    def ant_phase_temporal(self, ms2='', caltable='', phase_cal='', qa2_output_dir=''):

        """
        ant_phase_temporal.py

        This program plots the antenna-based phases obtained from the
        phase calibrators---used to calibrate the target


        INPUTS NEEDED:

           Assumes original table is in ms2
           caltable must be specified
           phase_cal


        OUTPUTS:

        The phase plots for each antenna are placed in

             ant_phase_temporal.png'


        USEAGE: assumes ms2 as the visibility data set.  Program looks for
        the relevant files.

        caltable = ms2+'.gain_inf'
        execfile ('qa2_plot_bandpass.py')

        """

        import numpy
        import pylab as pl
        
        #  Plot calibrator phases


        #  Get antenna paramters

        tb.open(ms2+'/ANTENNA')
        ant_names=tb.getcol('NAME')
        nant = len(ant_names)
        tb.close()

        #  Get gaintable

        tb.open(caltable)
        tb1 = tb.query('FIELD_ID == '+phase_cal)
        time=tb1.getcol('TIME')
        time = time-time[0]+5.0
        ant = tb1.getcol('ANTENNA1')
        gain = tb1.getcol('GAIN')
        spw = tb1.getcol('CAL_DESC_ID')
        field = tb1.getcol('FIELD_ID')
        g = numpy.abs(gain)
        p = numpy.arctan2(numpy.imag(gain),numpy.real(gain))*180.0 / 3.14159
        gg = g[0][0]
        px = p[0][0]
        py = p[1][0]
        np=len(ant)
        tb.close()
        spw_un = numpy.unique(spw)
        ant_un = numpy.unique(ant)
        nspw = len(spw_un)
        nant = len(ant_un)


        #  Set up plots

        tmin = numpy.min(time)
        tmax = numpy.max(time)
        tdiff = numpy.int((0.8*tmax-tmin)/1000.0) * 1000.0
        tmin = tmin - (tmax-tmin)*0.2
        tmax = tmax * 1.2

        #  Formatting of plots
        if nant <31: nsub1 = 6; nsub2 = 5

        if nant <26: nsub1 = 5; nsub2 = 5

        if nant <21: nsub1 = 4; nsub2 = 5

        if nant <17: nsub1 = 4; nsub2 = 4

        if nant <13: nsub1 = 3; nsub2 = 4

        #  Loop over antenna
        pl.close()
        for ispw in spw_un:
          nsub3 = 0
          for iant in range(0,nant):
            if ant_names[iant] != 'junk':
                nsub3 = nsub3 + 1
                pxphase = []
                pyphase = []
                tamp = []
                for pt in range(0,np):
                    if (ant[pt]==iant) and (spw[pt]==ispw):
                        tamp.append(time[pt])
                        pxphase.append(px[pt])
                        pyphase.append(py[pt])


                nn = len(pxphase)
                if nn >0:
                  for i in range (1,nn):
                    pdiff = pxphase[i]-pxphase[i-1]
                    pdiff = numpy.mod(pdiff+900.0,360.0)-180.0
                    pxphase[i] = pxphase[i-1] + pdiff
                    pdiff = pyphase[i]-pyphase[i-1]
                    pdiff = numpy.mod(pdiff+900.0,360.0)-180.0
                    pyphase[i] = pyphase[i-1] + pdiff

                  pxavg = numpy.median(pxphase)
                  pyavg = numpy.median(pyphase)
                  pxphase = pxphase - pxavg
                  pyphase = pyphase - pyavg
                  pl.subplot(nsub1,nsub2,nsub3)
                  pl.plot(tamp,pxphase,'bo')
                  pl.plot(tamp,pyphase,'go')
                  pl.ylim (-190.0, 190.0)
                  if nsub3 != 1: pl.yticks([])

                  if nsub3 != nant:
                      pl.xticks([])
                  else:
                      pl.xticks([0.0,tdiff])

                  pl.title('PHS '+ant_names[iant])




        pl.savefig(qa2_output_dir+'ant_phase_temporal.png')

    def bandpass_plot(self, ms2='', qa2_output_dir=''):

        """
        band_pass.py

        This program plots the bandpass.  It will plot both the original and/or
        the smoothed or BPOLY solution for comparison.

        INPUTS NEEDED:

           None.  assumes original table is in ms2+'.bandpass',
                  smooth table is in           ms2+'.smooth_20flat_ri'
                  or bpoly table is in         ms2+'.bpoly'

        OUTPUTS:

        The plots are put in several files with the name form:


        USEAGE: assumes ms2 as the visibility data set.  Program looks for
        the relevant files.

        execfile ('bandpass_plot.py')
        """
        import numpy
        import pylab as pl

        caltable = ms2+'.bandpass'                 
        caltable2 = 'N'
        overlay_table = False
        if os.path.exists(caltable+'_smooth20flat_ri'):
            caltable2 = caltable+'_smooth20flat_ri'

        if os.path.exists(caltable+'_bpoly'):
            caltable2 = caltable+'_bpoly'

        if caltable2 != 'N': overlay_table = True

        #  Plot bandpass amp

        pl.close()
        tb.open (caltable)
        spw = tb.getcol('CAL_DESC_ID')
        spw_un = numpy.unique(spw)
        nsub1 = len(spw_un)
        nsub2 = 1
        nsub3 = 0
        tb.close()
        for ispw in spw_un:
          tb.open(caltable)
          nsub3 = nsub3 + 1
          tb1 = tb.query('CAL_DESC_ID=='+str(ispw))
          ant = tb1.getcol('ANTENNA1')
          g = tb1.getcol('GAIN')
          gbp = numpy.abs(g)
          tb.close()
          gch = []
          n0 = gbp.shape[0]
          n1 = gbp.shape[1]
          n2 = gbp.shape[2]
          chan = range(0,n1)
          for ich in range(0,n1):
            gsum = 0.0
            for pol in range(0,n0):
                for iant in range(0,n2):
                    gsum = gsum + gbp[pol][ich][iant]
            gch.append(gsum/n0/n2)
          pl.subplot(nsub1, nsub2, nsub3)
          pl.plot(chan,gch,'b,')
          pl.title('Avg Bandpass for spw '+str(ispw))
          if nsub3 < nsub1: pl.xticks([])


        if caltable2 != 'N':
            nsub3 = 0
            for ispw in spw_un:
                tb.open(caltable2)
                nsub3 = nsub3 + 1
                tb1 = tb.query('CAL_DESC_ID=='+str(ispw))
                ant = tb1.getcol('ANTENNA1')
                g = tb1.getcol('GAIN')
                gbp = numpy.abs(g)
                tb.close()
                gch = []
                n0 = gbp.shape[0]
                n1 = gbp.shape[1]
                n2 = gbp.shape[2]
                chan = range(0,n1)
                for ich in range(0,n1):
                    gsum = 0.0
                    for pol in range(0,n0):
                        for iant in range(0,n2):
                            gsum = gsum + gbp[pol][ich][iant]
                    gch.append(gsum/n0/n2)
                pl.subplot(nsub1, nsub2, nsub3)
                pl.plot(chan,gch,'r,')


        pl.savefig(qa2_output_dir+'bandpass_avg.png')

    def bandpass_rms(self, ms2='', refAnt='', qa2_output_dir=''):

        """
        bandpass_rms.py

        This is an experimental python script that determines the rms channel
        to channel scatter in the bandpass.  It can be used to determine if
        smoothing or BPOLY is needed.

        INPUTS NEEDED:

           None.  assumes table is in ms2+'.bandpass'

        OUTPUTS:

        The ascii output file lists the relative gains of all data streams

        bandpass_rms.txt'

        USEAGE: assumes ms2 as the visibility data set

        execfile ('bandoass_rms.py')

        """

        import numpy
        import pylab as pl

        caltable = ms2+'.bandpass'

        #  Get antenna parameters

        tb.open(ms2+'/ANTENNA')
        ant_names=tb.getcol('NAME')
        nant = len(ant_names)
        print 'number of antenna ', nant,'  refant = ', refAnt
        tb.close()

        #  Get bandpass table

        tb.open(caltable)
        time=tb.getcol('TIME')
        time = time-time[0]+5.0
        ant = tb.getcol('ANTENNA1')
        gain = tb.getcol('GAIN')
        spw = tb.getcol('CAL_DESC_ID')
        fg = tb.getcol('FLAG')
        g = numpy.abs(gain)
        clen = len(g[0])
        p = numpy.arctan2(numpy.imag(gain),numpy.real(gain))*180.0 / 3.14159
        tb.close()
        ns=len(ant)
        ant_un = numpy.unique(ant)
        spw_un = numpy.unique(spw)
        nspw = len(spw_un)
        nch = len(g[1])

        #  Get avg gain and rms for each stream (both pols)

        zfileRes = qa2_output_dir+'bandpass_rms.txt'
        os.system('rm '+zfileRes)
        f = open (zfileRes, 'w')
        print 'opening file '+zfileRes



        f.write('\n\n')
        f.write('************************************************************************* \n')
        f.write('\n')
        f.write('      NORMALIZED BANDPASS AMPLITUDE RMS OVER '+str(nch)+' CHANNELS \n')
        f.write('          using table %s  \n\n' % (caltable))
        f.write('  Antenna      SPW0           SPW1           SPW2           SPW3\n')
        f.write('             X      Y        X      Y      X      Y       X      Y\n')
        gall_rms_x = []
        gall_rms_y = []
        rms_med = []
        for iant in ant_un:
            for ispw in spw_un:
                gspw_x = []
                gspw_y = []
                for j in range(0,ns):
                    if (iant == ant[j]) and (ispw == spw[j]):
                        for ic in range (0.05*clen,0.95*clen):
                            gspw_x.append(g[0,ic,j])
                            gspw_y.append(g[1,ic,j])
                            #
                        #
                    #
                #
                grms_x = numpy.sqrt(numpy.var(gspw_x))
                grms_y = numpy.sqrt(numpy.var(gspw_y))
                rms_med.append(grms_x)
                rms_med.append(grms_y)
                if ispw == 0:
                    f1string = ('%3d-%4s  %6.3f %6.3f  '% (iant,ant_names[iant], grms_x,grms_y))
                    gall_rms_x.append(grms_x)
                    gall_rms_y.append(grms_y)

        #        if (ispw > 0) and (ispw < nspw-1):
        #            f2string = ('%s %6.3f %6.3f '% (f1string,grms_x,grms_y))
        #            gall_rms_x.append(grms_x)
        #            gall_rms_y.append(grms_y)
        #            f1string = f2string

        #        if ispw == (nspw-1):
        #            f3string = ('%s %6.3f %6.3f \n'% (f2string,grms_x,grms_y))
        #            gall_rms_x.append(grms_x)
        #            gall_rms_y.append(grms_y)

        #        if ispw == (nspw-1):
        #            f3string = ('%s %6.3f %6.3f \n'% (f2string,grms_x,grms_y))

                if ispw > 0:
                    f2string = ('%s %6.3f %6.3f '% (f1string,grms_x,grms_y))
                    gall_rms_x.append(grms_x)
                    gall_rms_y.append(grms_y)
                    f1string = f2string

                if ispw == (nspw-1):
                    f3string = ('%s \n'% (f1string))

            f.write(f3string)


        rms_med = numpy.median(rms_med)
        f.write('\n\n    BANDPASS RMS MEDIAN VALUE = %6.3f \n' % (rms_med))
        if rms_med > 0.05:
            f.write ('      Use BPOLY rather than snsmooth\n')
        else:
            f.write ('    USE SIMPLE FREQUENCY SMOOTHING\n')


        #  Get outliers in each spw
        gx = gall_rms_x
        gy = gall_rms_y
        gx = numpy.reshape(gx,(nant,nspw))
        gy = numpy.reshape(gy,(nant,nspw))
        gx = numpy.transpose(gx)
        gy = numpy.transpose(gy)
        tsigma = 3.0
        pbad = []
        antbad = []
        spwbad = []
        f.write('\n\n       RMS OUTLIERS >%3.1f SIGMA\n\n' % (tsigma))
        f.write(' SPW     Xpol           Ypol\n')
        f.write('      mean    rms    mean   rms     OUTLIERS \n\n')
        for ispw in spw_un:
            nout = 0; noutstr = 'No Outliers'
            xavg = numpy.median(gx[ispw])
            xeavg = numpy.sqrt(numpy.var(gx[ispw]))
            yavg = numpy.median(gy[ispw])
            yeavg = numpy.sqrt(numpy.var(gy[ispw]))
            f.write('%3d  %6.4f %6.4f  %6.4f %6.4f\n'% (ispw, xavg, xeavg, yavg, yeavg))
            for iant in ant_un:
                xoff = (gx[ispw][iant]-xavg)/xeavg
                cpol = 'X'
                if (numpy.abs(xoff) > tsigma):
                    f.write('%37s  ant=%2d  sigma=%4.1f \n'% (cpol, iant, xoff)); nout=nout+1
                    pbad.append(cpol)
                    antbad.append(iant)
                    spwbad.append(ispw)

                yoff = (gy[ispw][iant]-yavg)/yeavg
                cpol = 'Y'
                if (numpy.abs(yoff) > tsigma):
                    f.write('%37s  ant=%2d  sigma=%4.1f \n'% (cpol, iant, yoff)); nout=nout+1
                    pbad.append(cpol)
                    antbad.append(iant)
                    spwbad.append(ispw)
            if nout == 0: f.write('%47s \n'% (noutstr))


        f.close()
        os.system('cat '+zfileRes)

        #  Plot band bandpasses

        nobad = len(pbad)
        if nobad > 0:
            print '******* see plot in bandpass_bad.png ******'
            pl.close()
            nsub1 = nobad/2 + 1
            nsub2 = 2; nsub3 = 0
            tb.open (caltable)
            for i in range(0,len(pbad)):
                nsub3 = nsub3 + 1
                tb1 = tb.query('CAL_DESC_ID=='+str(spwbad[i])+' and ANTENNA1=='+str(antbad[i]))
                if pbad[i] == 'X': g = tb1.getcol('GAIN')[0]

                if pbad[i] == 'Y': g = tb1.getcol('GAIN')[1]

                gg = numpy.transpose(g)[0]
                chan = range(0,len(gg))
                pl.subplot(nsub1, nsub2, nsub3)
                pl.plot(chan,gg,'b,')
                pl.title('Bandpass: spw '+str(spwbad[i])+'; ant '+str(antbad[i])+'; pol '+pbad[i])
                pl.xticks([])
            pl.savefig(qa2_output_dir+'bandpass_bad.png')
            tb.close()

    def flag_calc(self, in_ms):

        import casa

        # Create the local instance of the flag tool and open it

        fgLoc = casa.__flaggerhome__.create()

        fgLoc.open( in_ms )


        # Get the flagging statistics

        fgLoc.setdata()
        fgLoc.setflagsummary()

        flag_stats_dict = fgLoc.run()


        # Close and delete the local flag tool

        fgLoc.done()

        del fgLoc


        # Return the dictionary containing the flagging statistics

        return flag_stats_dict

    def flag_stats(self, ms2='', qa2_output_dir=''):

            """
            flag_stats.py

            This is an experimental python script that determines the distribution
            of flags in the calibrated data base.  It only includes flags from
            shadowing and anomalous bad data.

            INPUTS NEEDED:

               None.  assumes visibility data set is ms2

            OUTPUTS:

            The ascii output file lists the spw and antenna percentage of
            flagged data.

            qa2/flag_stat.txt'

            USEAGE: assumes ms2 as the visibility data set

            execfile (flag_stats.py)

            """

            #import flag_calc

            flagStats = self.flag_calc(ms2)

            zfileRes = qa2_output_dir+'flag_stat.txt'
            os.system('rm '+zfileRes)
            f = open (zfileRes, 'w')
            print 'opening file '+zfileRes

            f.write('\n\n\n    FLAGGING STATISTICS \n\n')
            f.write( 'Overall -> %.2f' %(100.0*flagStats['flagged']/flagStats['total'])+'\n')
            f.write('\n')
            f.write('Per spw (over total of dataset):\n')
            for i in flagStats['spw']:
                  f.write(i+' -> %.2f' %(100.0*flagStats['spw'][i]['flagged']/flagStats['total'])+'\n')

            f.write('')
            f.write('Per antenna (over total of dataset):\n')
            for i in flagStats['antenna']:
                  f.write( i+' -> %.2f' %(100.0*flagStats['antenna'][i]['flagged']/flagStats['total'])+'\n')

            f.close()

    def flux_values(self, ms2='', qa2_output_dir=''):

        """
        flux_values.py

        This program reads the log file obtained with flux scale, and puts
        the output in a better format.

        execfile ('flux_values.py')

        INPUTS NEEDED:

           None.  assumes table is in ms2+'.fluxscale'

        OUTPUTS:

        The ascii output  is put in 

        ms2+'.flux.txt'

        USEAGE: 

        execfile ('qa2_flux_values.txt')

        """

        import os.path

        caltable = ms2+'.fluxscale'
        if os.path.isfile(caltable) == False: return 0
        caltable2 = qa2_output_dir+'flux.txt'
        f1 = open(caltable, 'r')
        f2 = open(caltable2, 'w')

        f2.write('**********************************************************\n\n')
        f2.write('                    Flux Density Determinations \n\n')

        lines = f1.readlines()
        for line in lines:
            x = line.find('Found reference')
            if x!=-1:
                line_out = line[x+26:x+42]
                f2.write('   Reference source %s\n' %(line_out))

            x = line.find('Flux density')
            if x!=-1:
                line_out = line[x+17:]
                f2.write('     Flux densities %s' %(line_out))


        f1.close()
        f2.close()
        os.system('cat '+caltable2)

    def listobs2(self, ms2='', makeplot=True, qa2_output_dir=''):

        """
        New_listobs.py

        This is an experimental qa2 python script that produces a more
        readable listobs and plotants than the casapy version.


        INPUTS NEEDED:

           ms2 = visibility data set.  inputset to desired ms.
           makeplot  T = make antenna array plot, F = do not ...

        OUTPUTS:

        The ascii listobs output file is placed in

        qa2_output_dir+'NewListobs.txt'

        The antenna plot file is placed in

        qa2_output_dir+'Antenna_config.png'

        USEAGE:

        ms2 = <data set ms>
        qa2_output_dir = <output directory path>
        makeplot = T
        execfile (listobs.py)

        """

        #==================================================#
        #  Basic Parameters

        import sys
        import time
        import os.path
        import datetime
        import string
        import numpy
        import math
        import pylab as pl



        ms.open(ms2)
        vm = ValueMapping(ms2)
        scanInfo = ms.getscansummary()
        conditions = listconditions(vis=ms2,byscan=True,scan='',antenna='0',verbose=False)
        scan_list = scanInfo['summary'].keys()   # are the scan numbers
        #  Sort scans
        scan_num = []
        for sc in scan_list:
            scan_num.append(int(sc))

        scan_srt = numpy.sort(scan_num)

        msname = vm.inputMs
        spw_info=vm.spwInfo
        nscans = len(scan_srt)

        #   Get experiment time range
        ttemp = vm.getTimesForScan(scan_srt[0])
        int_time = scanInfo['summary'][str(scan_srt[0])]['0']['IntegrationTime']
        ttemp = ((ttemp/86400.0)+2400000.5-2440587.5)*86400.0 - int_time/2.0
        scan_begin = numpy.min(ttemp)
        exp_start=time.strftime('%Y/%m/%d/%H:%M:%S', time.gmtime(scan_begin))
        ttemp = vm.getTimesForScan(scan_srt[nscans-1])
        ttemp = ((ttemp/86400.0)+2400000.5-2440587.5)*86400.0 + int_time/2.0
        scan_end = numpy.max(ttemp)
        exp_end=time.strftime('%Y/%m/%d/%H:%M:%S', time.gmtime(scan_end))

        scan_list = []
        field_list = []
        time_on_field = []

        #   Open file for writing
        #   Split data set?

        zfileRes = qa2_output_dir+'NewListObs.txt'
        os.system('rm '+zfileRes)
        f = open (zfileRes, 'w')
        print 'opening file '+zfileRes
        #
        f.write( '\n')
        f.write( '* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n')
        f.write( '\n')
        f.write( '          SUMMARY INFORMATION FOR  %s '% (msname)+'\n')
        f.write( '\n')
        f.write( '   Experiment Duration:  %19s to'%  (exp_start)+'\n')
        f.write( '                         %19s'%  (exp_end)+'\n')
        f.write( '\n')
        f.write( '   Processed from ms: '+ms2+'\n')
        f.write( '   Written to file:   NewListobs.txt\n')
        f.write( '\n')
        f.write( '                                   SCAN LISTING \n')
        f.write( '\n')
        f.write( '  Scan FdId srcId FieldName         StartTime    StopTime     Int(s) Elev  ScanIntent\n')
        for isc in scan_srt:
        #    print 'scan ', isc
            iscstr = str(isc)
            subscan = []
            nk = scanInfo['summary'][iscstr].keys()
            for sc in nk:
                subscan.append(int(sc))

            sub_scan = numpy.sort(subscan)
            sfid = []
            sstart = []
            sstop = []
            nrows = []
            for isubsc in sub_scan:
                j = str(isubsc)
                fid = scanInfo['summary'][iscstr][j]['FieldId']
                start_time = scanInfo['summary'][iscstr][j]['BeginTime']
                stop_time = scanInfo['summary'][iscstr][j]['EndTime']
                integration_time = scanInfo['summary'][iscstr][j]['IntegrationTime']
                sfid.append(fid)
                sstart.append(start_time)
                sstop.append(stop_time)

        #   Integration time addition
            int_time = integration_time / 2.0 / 3600.0 / 24.0
        #   Combine consecutive fieldids
            nslots = len(sfid)
            ib = []
            ie = []
            ib.append(0)
            for i in range(1,nslots):
                if sfid[i] != sfid[i-1]:
                   ie.append(i-1)
                   ib.append(i)

            ie.append(nslots-1)
            nsources = len(ie)
            for i in range (0,nsources):
                ibeg = ib[i]
                iend = ie[i]
                field = sfid[ibeg]
                field_name = vm.getFieldNamesForFieldId(field)
                source_id = vm.getFieldIdsForFieldName(field_name)[0]
                if len(field_name) > 12: field_name = field_name[0:12]+'*'

                mjd_start = sstart[ibeg] - int_time
                mjd_stop = sstop[iend] + int_time
                q_start = qa.quantity(mjd_start, unitname='d')
                q_stop  = qa.quantity(mjd_stop, unitname='d')
                scan_list.append(isc)
                field_list.append(field)
                time_on_field.append((mjd_stop-mjd_start)* 1440.0)
                elev = conditions[isc]['elevation']
        #        pwv = conditions[isc]['pwv']
                intent = vm.getIntentsForScan(isc)
                jintent = string.join(intent, '')
                scan_intent = []
                if jintent.find('POINTING')!=-1: scan_intent.append('Cal Pointing')

                if jintent.find('FOCUS')!=-1: scan_intent.append('Cal Focus')

                if jintent.find('ATMOSPHERE')!=-1: scan_intent.append('Cal atmos=Tsys')

                if jintent.find('BANDPASS')!=-1: scan_intent.append('Cal Bandpass')

                if jintent.find('PHASE')!=-1: scan_intent.append('Cal Phase')

                if jintent.find('AMPLI')!=-1: scan_intent.append('Cal Flux')

                if jintent.find('SIDEBAND')!=-1: scan_intent.append('Cal Sideband')

                if jintent.find('TARGET')!=-1: scan_intent.append('Obs Target')

                f.write (' %4d %4d %4d   %13s  %s - %s  %5.2f %6.1f  %s\n' % (isc, field, source_id, field_name.ljust(16), qa.time(q_start,prec=7), qa.time(q_stop,prec=7), integration_time, elev, string.join(scan_intent,', ')))
        #
        #
        #
        #      Get source information
        f.write( '\n')
        f.write( '\n')
        f.write( '                               FIELD INFORMATION \n')
        f.write( '\n')
        f.write( ' Fid  Srd  Field                       RA  (J2000)    DEC       Fld Time  #Scans\n')
        f.write( '                                                                  (min)\n')
        temp = timeOnSource(ms=ms2,verbose=False)
        tb.open(ms2+'/FIELD')
        name = tb.getcol('NAME')
        sid = tb.getcol('SOURCE_ID')
        pos = tb.getcol('PHASE_DIR')
        tb.close()
        nfld = len(name)
        n_of_scans = len(field_list)
        for i in range(0,nfld):
                ra=str(pos[0][0][i])+'rad'
                rra=qa.time(ra,prec=11)
                dec=str(pos[1][0][i])+'rad'
                rdec=qa.angle(dec,prec=10)
                total_time = 0.0
                num_scans = 0
                for ix in range(0,n_of_scans):
                    if field_list[ix] == i:
                        total_time = total_time + time_on_field[ix]
                        num_scans = num_scans + 1

                f.write( '%4d %4d  %22s %14s %1s%12s  %5.2f    %3d\n' % (i,sid[i],name[i].ljust(22), rra, rdec[0:1],rdec[2:],total_time, num_scans))
        #	print '%4d %4d  %22s %14s %1s%12s  %5.2f    %3d\n' % (i,sid[i],name[i].ljust(22), rra, rdec[0:1],rdec[2:],total_time, num_scans)



        #      Get spectral window information
        f.write( '\n')
        f.write( '\n')
        f.write( '                        FREQUENCY INFORMATION \n')
        f.write( '\n')
        f.write( 'spw  nchan      -----Frequencies (GHz)-------     --Channel Width-- \n')
        f.write( '               First         Last     Bandwidth     MHz     km/s     POLN \n')
        f.write( '\n')
        spw_info=vm.spwInfo
        spw_size = len(spw_info)

        #  Getting polarizations
        polId = []
        tb.open(ms2+'/DATA_DESCRIPTION')
        for spwId in range(0,spw_size):
                polId.append(tb.query('SPECTRAL_WINDOW_ID == '+str(spwId)).getcell('POLARIZATION_ID', 0))

        tb.close()

        polId = sorted(dict.fromkeys(polId).keys())[0]

        tb.open(ms2 +'/POLARIZATION')
        corrProd = tb.selectrows(polId)
        corrProd = corrProd.getcell('CORR_PRODUCT', 0)
        tb.close()
        corrProd = corrProd.transpose()

        numCorr = corrProd.shape[0]

        if numCorr == 2:
                polProdNames = ['XX', 'YY']
        elif numCorr == 4:
                polProdNames = ['XX', 'XY', 'YX', 'YY']
        else:
                sys.exit("Number of correlations not supported")

        #  Does spw0 have wvr?

        if spw_info.keys()[0] == 0:
            Nwvr = 0
        else:
            Nwvr = 1
            f.write ("  0     4   184.550       189.550     7.500                       ['I'] \n") 

        for i in range(Nwvr,spw_size):
                bandwidth = spw_info[i]['bandwidth']/1.0E9
                num_chan = spw_info[i]['numChannels']
                chan_width = spw_info[i]['chanWidth']/1.0E6
                chan_1_freq = spw_info[i]['chanFreqs'][0]/1.0E9
                chan_L_freq = spw_info[i]['chanFreqs'][num_chan-1]/1.0E9
                vel_res = chan_width / chan_1_freq * 299.7
                f.write( '%3d %5d %12.6f %12.6f %7.3f %11.3f %8.2f  %s \n' % (i,num_chan,chan_1_freq,chan_L_freq,bandwidth,chan_width,vel_res,polProdNames))

        #      Get antenna information
        f.write( '\n')
        f.write( '\n')
        f.write( '                      ANTENNA INFORMATION \n')
        f.write( '\n')
        f.write(  ' ID  Name    Pad   Size     Longitude   Latitude      E-off   N-off  Elev \n') 
        f.write( '                    (m)                                 (m)     (m)   (m) \n')
        tb.open(ms2+'/ANTENNA')
        position = tb.getcol('POSITION')
        diam = tb.getcol('DISH_DIAMETER')
        station = tb.getcol('STATION')
        antenna = tb.getcol('NAME')
        nant = len(antenna)
        tb.close()
        lon0 = -1.1825470
        lat0 = -0.3994900
        plotx = []
        ploty = []
        for i in range (0,nant):
                xx = position[0][i]
                yy = position[1][i]
                zz = position[2][i]
                elev = sqrt(xx**2+yy**2+zz**2)-6379960.0
                lat = math.asin(zz/sqrt(xx**2+yy**2+zz**2))
                lon = math.atan2(yy, xx)
                zlat = str(lat)+'rad'
                zlon = str(lon)+'rad'
                qlat = qa.angle(zlat,prec=8)
                qlon = qa.angle(zlon,prec=8)
                qqlat=qlat[0]+qlat[2:]
                qqlon=qlon[0]+qlon[2:]
                n_off = (lat - lat0)*6379960.0 +215.0
                e_off = (lon - lon0)*6379960.0 * cos(lat) - 3.0
                f.write( '%3d  %4s %6s %6.1f  %12s %12s  %7.1f %7.1f %5.1f \n' %(i,antenna[i],station[i],diam[i],qqlon,qqlat,e_off,n_off,elev))
                plotx.append (e_off)
                ploty.append (n_off)
        #
        #  Find size of plot
        pxmin = numpy.min(plotx)
        pymin = numpy.min(ploty)
        pxmax = numpy.max(plotx)
        pymax = numpy.max(ploty)
        psize = numpy.sqrt((pxmax-pxmin)**2 + (pymax+pymin)**2)
        mssize = 30.0/300.0*psize

        if makeplot:
                pl.close()
                pl.plot(plotx,ploty,'go',ms=mssize,mfc='#FFB6C1')
                for i in range(0,len(plotx)):
                        pl.text(plotx[i],ploty[i]-3.0,str(i)+'='+antenna[i])

                pl.title(ms2)
                myfigfile = qa2_output_dir+'antenna_config.png'
                pl.savefig(myfigfile,format='png',density=108)
                print 'saved antenna plot in Antenna_config.png'

        f.close()
        os.system('cat '+zfileRes)
        ms.close()

    def mosaic_plot(self, ms1='', qa2_output_dir=''):

        """
        mosaic_plot.py

        This program plots the mosaic region.

        INPUTS NEEDED:

           vis = desired ms

        OUTPUTS:

        The mosaic plot is put in 'qa2_mosaic_plot.png'


        USEAGE: 

        vis = ms1
        execfile ('qa2_mosaic_plot_.py')

        """

        # Is this a mosaic?

        a=self.getIntentsAndSourceNames(ms1)
        idstr = a['OBSERVE_TARGET']['idstring']
        sid =   a['OBSERVE_TARGET']['sourceid']
        if idstr != sid:
            plotmosaic(vis=ms1, sourceid=sid[0], figfile = qa2_output_dir+'mosaic_plot.png')

    def phase_cal_check(self, ms2='', phase_cal='', qa2_output_dir=''):

        """
        qa2_phase_cal_check.py

        This script plots the phase calibration
          amp and phase versus uvdist for each spw
          amp and phase versys frequency for each spw

        INPUTS NEEDED:

           Assumes data base is ms2
           Needs field number, phase_cal


        OUTPUTS:

        The calibrated and model amplitudes for the selected field
        number is placed in

            qa2_output_dir+'phase_cal_uvdist.png'
            qa2_output_dir+'phase_cal_freq.png'


        USEAGE: assumes ms2 as the visibility data set.  

        phase_cal = '2'
        execfile ('phase_cal_check.py')

        """

        from vishead import vishead

        #  Get number of spw

        a=vishead(vis=ms2,
                mode = 'get',
                hdkey = 'spw_name')
        nspw = len(a[0])

        #  Plot amp, phase versus uvdist for calibrator

        nsubplot = 100*nspw+20
        for i in range(0,nspw):
            nsubplot = nsubplot+1
            plotxy(vis=ms2,
               xaxis = 'uvdist', yaxis = 'amp',
               datacolumn = 'corrected', spw=str(i),
               averagemode = 'vector', width = '4000', plotsymbol = ',',
               field = phase_cal, timebin = '60',subplot=nsubplot)
            nsubplot = nsubplot+1
            if i != nspw-1:
                plotxy(vis=ms2,
                   xaxis = 'uvdist', yaxis = 'phase',
                   datacolumn = 'corrected', spw=str(i),
                   averagemode = 'vector', width = '4000', plotsymbol = ',',
                   field = phase_cal, timebin = '60', subplot=nsubplot)
            else:
                plotxy(vis=ms2,
                   xaxis = 'uvdist', yaxis = 'phase',
                   datacolumn = 'corrected', spw=str(i),
                   averagemode = 'vector', width = '4000', plotsymbol = ',',
                   field = phase_cal, timebin = '60', subplot=nsubplot,
                   figfile = qa2_output_dir+'phase_cal_uvdist.png')

        #  Plot amp, phase versus freq for calibrator

        #  Get number of channels
        vm = ValueMapping(ms2)
        spw_Info = vm.spwInfo
        nchan = spw_Info[0]['numChannels']
        nchan = nchan / 128
        if nchan < 2: nchan = 1

        nwidth = str(nchan)

        nsubplot = 100*nspw+20
        for i in range(0,nspw):
            nsubplot = nsubplot+1
            plotxy(vis=ms2,
               xaxis = 'freq', yaxis = 'amp',
               datacolumn = 'corrected', spw=str(i),
               averagemode = 'vector', width = nwidth,timebin='100000',
               crossscans = True, crossbls = True, interactive = False,
               field = phase_cal,subplot=nsubplot)
            nsubplot = nsubplot+1
            if i != nspw-1:
                plotxy(vis=ms2,
                   xaxis = 'freq', yaxis = 'phase',
                   datacolumn = 'corrected', spw=str(i),
                   averagemode = 'vector', width = nwidth,timebin='100000',
                   crossscans = True, crossbls = True, interactive = False,
                   field = phase_cal,subplot=nsubplot)
            else:
                plotxy(vis=ms2,
                   xaxis = 'freq', yaxis = 'phase',
                   datacolumn = 'corrected', spw=str(i),
                   averagemode = 'vector', width = nwidth,timebin='100000',
                   crossscans = True, crossbls = True, interactive = False,
                   field = phase_cal,subplot=nsubplot,
                   figfile = qa2_output_dir+'phase_cal_freq.png')

    def sensitivity_calculator(self, ms2='', caltable='', s_id='', tsys_field='', qa2_output_dir=''):

        """
        Sensitivity calculator

        This is an experimental python script that determines
        the expected sensitivity for observations of a science
        target field.

        The default source_id is the first OBSERVE_TARGET.  If a
        mosaic, the sensitivity for one field in the source_id is
        determined.  source_id can be overwritten as an INPUT.


        INPUTS:  vis:            the calibrated measurement set name
                 ctable:         tsys table name
                 s_id:           s_id = ''.  Use first OBSERVE_TARGET and
                                   one field if mosaic.
                                 s_id = 'N'. Use this source number.  Will
                                   include all fields if mosaic.

          Examples:  s_id='';execfile('sensitivity_calculator.py')
                          will determine expected rms for the first target

                     s_id='4';execfile('sensitivity_calculator.py')
                          will determine expected rms for source id 4.


        OUTPUTS:  The expected rms sensitivity for one channel in
                 each spw, plus other derived values

                 obs_spw:                   A numerical list of the spectral windows
                 chan_sensitivity[obs_spw]: The rms sensitivity in mJy for one channel
                                            per spw
                 chan_width[obs_spw]:       The width of one channel (GHz) per spw
                 num_chan[obs_spw]:         The number of channels per spw
                 spw_sensitivity[obs_spw]:  The rms sensitivity in mJy per spw
                 mfs_sensitivity:           The rms sensitivity in mJy for all spw
        """

        import numpy

        # Initialization of normalized values for each band
        # Taken from alma sensitivity calculator

        band_low = [75.0, 200.0, 300.0, 600.0]
        band_high = [120.0, 300.0, 500.0, 1000.0]
        band_name = ['3','6','7','9']
        tsys_nominal = [75.0, 90.0, 150.0, 1200.0]
        sensitivities = [0.20, 0.27, 0.50, 5.32]

        #  Assume apropri sensitivity for band 3,6,7,9
        #  Sensitivity units are in mJy and are normalized to:
        #    16 12-m antennas,
        #    8 GHz banwidth
        #    Dual freq bandwidth
        #    for tsys_nominal given above
        #    Integration time of one minute

        #  Determine the time on first target source field

        tos = self.getIntentsAndSourceNames(ms2)

        if s_id == '':
            tos['OBSERVE_TARGET'].keys()
            source_id = tos['OBSERVE_TARGET']['sourceid'][0]
            field_all = tos['OBSERVE_TARGET']['id']
            nfield = len(field_all)
            time_os = timeOnSource(ms2)
            min_all = time_os[source_id]['minutes_on_source']
            min_per_field = min_all / nfield
        else:
            time_os = timeOnSource(ms2)
            all_scans = numpy.unique(time_os['source_ids'])
            source_id = int(s_id)
            if source_id in all_scans:
                min_per_field = time_os[source_id]['minutes_on_source']
            else:
                print '*************************************************'
                print '******* source_id =', source_id, ' NOT FOUND'
                print '*************************************************'


        #  Determine the number of antennas

        tb.open(ms2+'/ANTENNA')
        antenna = tb.getcol('NAME')
        tb.close()
        nant = len(numpy.unique(antenna))

        #  Get frequency information

        vm = ValueMapping(ms2)

        npol = vm.nPolarizations
        d_pol = 'single_pol'
        if npol >= 2: d_pol = 'dual_pol'

        spw_info=vm.spwInfo
        obs_spw = spw_info.keys()
        n_spw = len(obs_spw)
        chan_width = []
        mean_freq = []
        band_index = []
        num_chan = []
        for dspw in obs_spw:
            chan_width.append(spw_info[dspw]['chanWidth']/1.0E9)
            num_chan.append(spw_info[dspw]['numChannels'])
            freq = spw_info[dspw]['meanFreq']/1.0E9
            mean_freq.append(freq)
            for i in range(0,4):
                if (freq>band_low[i]) and (freq<band_high[i]):
                    band_index.append(i)
                    break



        #  Get tsys for each spw

        #caltable = ms1+'.tsys'
        tb.open(caltable)
        ss = tsys_field
        tsys_spw = []
        for dspw in obs_spw:
            tb1 = tb.query('FIELD_ID=='+ss+' and CAL_DESC_ID=='+str(dspw))
            gain = numpy.real(tb1.getcol('GAIN'))
            tsys_spw.append(numpy.median(gain))
            
        tb.close()

        #  Determine expected sensitivities for each spw for each channel

        chan_sensitivity = []
        spw_sensitivity = []
        for dspw in obs_spw:
            rel_tsys = tsys_spw[dspw] / tsys_nominal[band_index[dspw]]
            s_temp = sensitivities[band_index[dspw]] * rel_tsys            #  scale by tsys
            s_temp = s_temp / numpy.sqrt(min_per_field)                    #  scale by inverse sqrt time
            print "nant = %d" % nant
            s_temp = s_temp * 16.0 / nant                                  #  scale by antenna number
            s_temp = s_temp / numpy.sqrt(abs(chan_width[dspw]) / 8.0)           #  scale by chan bandwidth
            if d_pol != 'dual_pol': s_temp = s_temp * 1.414                #  not dual frequency?
            chan_sensitivity.append(s_temp)
            spw_sensitivity.append(s_temp / numpy.sqrt(num_chan[dspw]))

        #  Results

        print '\n SENSITIVITY CALCULATION:\n     Number of spw             %2d\n     Polarization               %s \n     Number of Antennas        %3d \n     Source_Id             %6d \n     Field Integration Time   %7.2f min \n' % (n_spw, d_pol, nant, source_id, min_per_field)

        print '\n spw   mean freq      T_sys avg  chan width    channel rms     # chan    spw rms\n'

        mfs_rms = 0.0
        for dspw in obs_spw:
            mfs_rms = mfs_rms + 1.0 / spw_sensitivity[dspw]**2
            print ' %2d   %6.3f GHz    %6.1f K   %7.4f MHz   %7.2f mJy     %5d    %6.2f mJy' %  (dspw, mean_freq[dspw], tsys_spw[dspw],chan_width[dspw]*1.0E3, chan_sensitivity[dspw], num_chan[dspw], spw_sensitivity[dspw])

        mfs_sensitivity = numpy.sqrt(1.0/mfs_rms)
        print ' ALL   %70.2f mJy ' % (mfs_sensitivity)

        return {'min_per_field': min_per_field, 'mfs_sensitivity': mfs_sensitivity}

    def shadowed_ant(self, ms2='', qa2_output_dir=''):

        """
        shadowed_ant.py

        This is an experimental qa2 python script that determines the antenna
        and time range the were flagged because of shadowing.

        Must be run just after the shadowing flag command

        INPUTS NEEDED:

           ms2 = visibility data set (usual default)
           Assumes flag file of ms2+flagversions/flags.BeforeBandpassCalibration'

        OUTPUTS:

        The ascii output file is placed in

        qa2_output_dir+'shadowed_ant.txt'


        USEAGE:

        execfile (shadowed_ant.py)

        HISTORY

        29 Apr - wrapping issue patched

        """


        import numpy as np
        import pylab as pl

        tb.open(ms2)
        time=tb.getcol('TIME')
        ant1 = tb.getcol('ANTENNA1')
        ant2 = tb.getcol('ANTENNA2')
        flag=tb.getcol('FLAG_ROW')
        tb.done()

        #  Get saved flagcolumn after shadowing only
        fg_ver = ms2+'.flagversions/flags.BeforeBandpassCalibration'
        tb.open(fg_ver)
        flag = tb.getcol('FLAG_ROW')
        tb.done()

        # DERIVE THE UNIQUE TIME STAMPS
        uniqt = np.unique(time)
        uniqt.sort()
        n_time = uniqt.shape[0]

        # DERIVE THE UNIQUE ANTENNAS
        uniqa = np.unique(np.append(ant1,ant2))
        n_ant = uniqa.shape[0]

        #   Open file for writing
        zfileRes = qa2_output_dir+'shadowed_ant.txt'
        if os.path.exists: os.system('rm '+zfileRes)

        f = open (zfileRes, 'w')
        print 'opening file '+zfileRes
        #
        f.write( '\n\n')
        f.write( '* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n')
        f.write( '\n')
        f.write( '              SHADOWING OF ANTENNAS\n')

        # -------------------------------------------------------------------
        # PROBLEM 1 ... EXTRACT "ANTENNA" FLAGS
        # -------------------------------------------------------------------

        # ... so this is easy if we have an array that is n_time x n_ant x
        # n_ant in shape. We just collapse along the second axis and look for
        # where the number of flags at a timestep equals (n_ant-1). That is,
        # just look for the unique time stamps where baselines with all other
        # antennas are flagged. The problem is that constructing this array is
        # painful because (at least as I set it up here) you have to
        # constantly requery the array. Still, for a small dataset this is
        # pretty fast in numpy. Here goes.

        # (1) Build a flags-by-baseline 3-d array (time, ant1, ant2)

        flag_base = np.zeros([n_time, n_ant, n_ant],dtype=np.bool)
        if (len(flag) != len(time)):
            print "WARNING: There is a mismatch between the number of rows in %s (%d) and %s (%d)" % (ms2,len(time), fg_ver, len(flag))
        for i in np.arange(n_time):
            # ... find rows with this timestamp and a flag
#            print "shape(time) = ", np.shape(time)
#            print "shape(uniqt) = ", np.shape(uniqt)
#            print "flag = ", np.shape(flag)
            ind = ((time == uniqt[i])*(flag == 1)).nonzero()
            
            # ... continue if no flags
            if ind[0].shape[0] == 0: continue
            # ... note each flagged row in our new array
            for row in ind[0]:
                flag_base[i,ant1[row],ant2[row]] = True
                flag_base[i,ant2[row],ant1[row]] = True

        # (2) Collapse this to a flags-by-antenna array. Sum along one of the
        # antenna axes and then not where all baselines show a flag.

        flag_ant =  np.sum(flag_base, axis=2) >= (n_ant-1)

        # we now have antenna flags in this data set!

        # To look at this visually do this:
        # pl.imshow(np.transpose(flag_ant), aspect='auto')
        # ... this is time on x and antenna on y

        # -------------------------------------------------------------------
        # PROBLEM 2 ... EXTRACT TIME RANGES
        # -------------------------------------------------------------------

        # This is also pretty easy in principle once we have the previous
        # array. We have the unique time stamps and for each antenna. We
        # consider each antenna in turn and step through our array of True
        # (flagged) and False (unflagged) vs time. When flags flip from True
        # to False (or vice verse) in contiguous elements we want to include
        # that in our output.

        didshadow = 0
        for ant in uniqa:
            # this is flagged (True/False) vs. time
#            print "np.shape(flag_ant) = ", np.shape(flag_ant)
#            print "len(flag_ant[0]) = ", len(flag_ant[0])
#            print "ant = ", ant

            # Todd added the following line to prevent a crash on
            # uid___A002_X436934_X48c.ms
            if (ant >= len(flag_ant[0])): break

            flag_v_time = flag_ant[:,ant]

            if np.sum(flag_v_time) == 0:
        #        print "No flags for antenna "+str(ant)
                continue

            # Note where flags start
            flag_started = (flag_v_time == False)* \
                (np.roll(flag_v_time, shift=-1,axis=0) == True)

            # Take care of first time stamp
            if flag_v_time[0] == True:
                flag_started[0] = True

            # Take care of last time stamp (the roll wraps)
            if flag_v_time[0] == True and flag_v_time[-1] == False:
                flag_started[-1] = False

            # Note where flags stop
            flag_stopped = (flag_v_time == True)* \
                (np.roll(flag_v_time, shift=-1,axis=0) == False)

            # Take care of last time stamp
            if flag_v_time[-1] == True:
                flag_stopped[-1] = True

            if np.sum(flag_started) != np.sum(flag_stopped):
                print "Something weird is happening."
                print np.sum(flag_started), np.sum(flag_stopped)

            # Use our "starts" and "stops" to report time ranges
            start_ind = (flag_started).nonzero()
            start_times = uniqt[start_ind]
            for start_time in start_times:
                stop_ind = (flag_stopped*(uniqt >= start_time)).nonzero()
                stop_time = np.min(uniqt[stop_ind[0]])
                mjd_start = start_time / 3600. / 24.
                mjd_stop = stop_time / 3600. / 24.
                q_start = qa.quantity(mjd_start, unitname='d')
                q_stop = qa.quantity(mjd_stop, unitname='d')
                didshadow = 1
                f.write("     antenna: "+str(ant)+" shadowed "+qa.time(q_start)+" to "+qa.time(q_stop))
                f.write('\n')

        if didshadow == 0:
            f.write('\n           No shadowed antennas\n')

        f.close()
        #os.system('cat '+zfileRes)

    def target_check(self, ms2='', target='', target_source='', tsys_caltable='', tsys_field='', qa2_output_dir=''):

        """
        target_check.py

        One target is chosen.  A full bandwidth image is made
        and the peak and rms of the image is determined.  Assuming ALMA
        parameters and valid tsys observations, the expected sensitivity is
        determined and compared with the image sensitivity.  The psf and
        the uv-coverage are also made.


        INPUTS NEEDED:

           ms2 is the assumed data set
           target = 'xx'  is the field number of the target
           tsys_field = 'yy' is the field number with the relevant tsys
              if tsys_field = '', tsys_field = target.


        OUTPUTS:

        The calibrated and model amplitudes for the selected field
        number is placed in

            qa2_output_dir+'sensitivity.txt'    contains information about the image
            qa2_output_dir+'target.image.png' is a display of the target image (done by hand)
            qa2_output_dir+'target.psf.png'   is a display of the target psf (done by hand)
            qa2_output_dir+'target.uvcov.png'  is a display of the target uv coverage



        USEAGE: assumes ms2 as the visibility data set.  

        target = '4'
        execfile ('target_check.py')




        """


        import numpy
        from clean import clean
        from imstat import imstat
        from imhead import imhead
        
        #  Image on target and check on sensitivity
        #
        # Initializations

        vm = ValueMapping(ms2)
        spw_info=vm.spwInfo

        #  Get a frequency
        freq = spw_info[0]['meanFreq'] / 1.0E9

        image_sizes = [216,256,360,432,640,800,1000,1296,1600,2048]

        tb.open(ms2+'/ANTENNA')
        antenna = tb.getcol('NAME')
        tb.close()
        nant = len(numpy.unique(antenna))

        #  Get band

        zfileRes = qa2_output_dir+'sensitivity.txt'
        os.system('rm '+zfileRes)
        f = open (zfileRes, 'w')
        print 'opening file '+zfileRes

        #  Get image parameters

        print 'getting imaging parameters'

        #  Find longest baseline

        baselines = getBaselineLengths(ms2)
        ll = baselines[len(baselines)-1]
        baseline_max = ll[1]
        print 'longest baselines is ',ll[0] ,'with length ', baseline_max
        b = 15000.0 / baseline_max / freq
        print 'theoretical pixel separation ', b

        #  Round to nearest unit
        if b > 0.20:
            b = numpy.int(b*20)/20.0
        else:
            b = numpy.int(b*100.0)/100.0

        zcell = str(b)+'arcsec'

        #  Field of view

        pbsize = spw_info[0]['chanFreqs'][0]*1.0E-9 / 20.0

        #  Find best image size that is >2*pbsize/cellsize
        possible_size = 2.0*pbsize/b

        for zimsize in image_sizes:
            if zimsize > possible_size: break

        f.write('\n\n')
        f.write('************************************************************* \n')
        f.write('\n')
        f.write('        CHECK OF A TARGET IMAGE AND SENSITIVITY\n\n')
        f.write('    longest baseline       = %7.1f (meters)\n'% (baseline_max))
        f.write('    recommended cellsize   = %6.2f (arcsec)\n'% (b))
        f.write('    synthesized beam size  = %6.2f (arcsec)\n'% (pbsize))
        f.write('    recommended image size = %5d\n\n'  % (zimsize))

        #  Make the image of selected target

        print 'making mfs image of selected target'

        os.system('rm -rf '+qa2_output_dir+'target_check*')
        clean(vis=ms2,
              field = target,
              imagename = qa2_output_dir+'target_check',
              imsize = zimsize, cell = zcell,
              interactive = False, niter=100)

        #  Get rms and peak box information

        zbox1 = str(int(0.1*zimsize)); zbox2 = str(int(0.9*zimsize))

        a=imstat(imagename=qa2_output_dir+'target_check.image',
               box = zbox1+','+zbox1+','+zbox2+','+zbox2)

        amax = a['max'][0]

        zbox1 = str(int(0.1*zimsize)); zbox2 = str(int(0.3*zimsize))

        a=imstat(imagename=qa2_output_dir+'target_check.image',
               box = zbox1+','+zbox1+','+zbox2+','+zbox2)
        arms = a['rms'][0]

        clev = float(arms*2.5)

        #  Make plot of object and store

        imview(raster = {'file':qa2_output_dir+'target_check.image'}, 
               contour = {'file':qa2_output_dir+'target_check.image',
                          'levels':[-1,1,2,4,6,8,10,12,16,20,30,50],
                          'unit':clev},
               zoom = 1,
               out = qa2_output_dir+'target_image.png')

        imview(raster = {'file':qa2_output_dir+'target_check.psf'}, 
               contour = {'file':qa2_output_dir+'target_check.psf',
                          'levels':[-3,-2,-1,1,2,3,5,7,9,9.5],
                          'unit':0.1},
               zoom = 1,
               out = qa2_output_dir+'target_psf.png')

        plotxy(vis = ms2,
               xaxis = 'u', yaxis = 'v',
               spw = '0:100~100',
               field = target,
               interactive = False,
               figfile = qa2_output_dir+'target_uv.png')


        #  Get image parameters

        a = imhead(imagename = qa2_output_dir+'target_check.image',
                   mode = 'list')
        freq = a['crval4']*1.0E-9
        bwidth = a['cdelt4']*1.0E-9
        bmaj = a['beammajor']
        bmin = a['beamminor']
        bpa = a['beampa']

        #  Get expected Sensitivities of mfs image

        #vis=ms2
        #s_id = ''
        #execfile(qa2_scripts_dir+'sensitivity_calculator.py')
#        print "Calling: sensitivity_calculator(ms2='%s', caltable='%s', s_id='%s', tsys_field='%s', qa2_output_dir='%s')" % (ms2,tsys_caltable,str(target_source),str(tsys_field),qa2_output_dir)
        
        tmp1 = self.sensitivity_calculator(ms2=ms2, caltable=tsys_caltable, s_id=target_source, tsys_field=tsys_field, qa2_output_dir=qa2_output_dir)

        f.write('    target                 = %2s \n' % (target)) 
        f.write('    resolution             = %5.2f x%5.2f in pa %6.1f\n' % (bmaj, bmin, bpa)) 
        f.write('    time on target         = %6.2f (min)\n'  % (tmp1['min_per_field']))
        f.write('    peak on image          = %6.2f (mJy)\n'  % (amax*1000.0))
        f.write('    rms on image           = %6.2f (mJy)\n'  % (arms*1000.0))
        f.write('    expected sensitivity   = %6.2f (mJy)\n'  % (tmp1['mfs_sensitivity']))


        f.close()
        os.system('cat '+zfileRes)

    def target_spectrum(self, ms2='', target='', dospw='', qa2_output_dir='',uvrange='0~30m'):

        """
        target_spectrum.py

        One target is chosen.  The spectrum for the short spacings
        is made for all spw's.


        INPUTS NEEDED:

           ms2 is the assumed data set
           target = 'xx'  is the field number of the target
           dospw = '0'    put in spw's and channel range needed



        OUTPUTS:

        The source spectrum is placed in 

            qa2_output_dir+'target.spectrum.png'



        USEAGE: assumes ms2 as the visibility data set.  

        target = '7'
        dospw = '0'
        execfile ('target_spectrum.py')


        """

        sep_spw = dospw.split(',')
        n_spw = len(sep_spw)
        subplot = 100*n_spw + 10
        for spw in sep_spw:
            subplot = subplot + 1
            plotxy (vis = ms2,
                xaxis = 'freq', yaxis = 'amp', datacolumn = 'corrected',
                field = target, plotsymbol = 'o',
                averagemode = 'vector', timebin = '100000',
                crossscans = True, crossbls = True, width = '5',
                uvrange = uvrange,
                spw = spw, subplot = subplot,
                figfile = qa2_output_dir+'target_spectrum.png', interactive = False)

    def tsys_stat(self, ms1='', tsys_field='', makeplot=True, qa2_output_dir=''):

        """
        tsys_stat.py

        This is an experimental python script that determines the statistics,
        the average and outliers for the tsys.  Plots are made of the TDM
        tsys determination.

        INPUTS NEEDED:

           None.  assumes visibility data set is ms1 and the tdm tsys in
                  in ms1+.tsys.png'

        OUTPUTS:

        The ascii output file lists the scans for each tsys observation, and
        any outliers for each antenna/spw.

        <qa2_output_dir>+'tsys_stat.txt'
        <qa2_output_dir>+'tsys_plot.png'


        USEAGE: assumes ms1 as the visibility data set and the TDM tsys
                is in ms1+'.tsys'.
                makeplot = T for plots, makeplot = F for no plots

        makeplot = T
        execfile ('qa2_scripts/tsys_stat.py')

        """

        #######################################

        import numpy


        #  Determines rms and average value of Tsys (TDM)

        #  Get antenna paramters

        tb.open(ms1+'/ANTENNA')
        ant_names=tb.getcol('NAME')
        ant_pos=tb.getcol('POSITION')
        nant = len(ant_names)
        tb.close()

        #  Get amplitude and other information

        caltable = ms1+'.tsys'               #  Put in caltable name

        tb.open(caltable)
        time=tb.getcol('TIME')
        time = time-time[0]+5.0
        antenna1 = tb.getcol('ANTENNA1')
        gain = tb.getcol('GAIN')
        nchan = gain.shape[1]
        gain_amp = numpy.abs(gain)
        fldid = tb.getcol('FIELD_ID')
        scanno = tb.getcol('SCAN_NUMBER')
        spw = tb.getcol('CAL_DESC_ID')
        spw_un = numpy.unique(spw)
        nspw = len(spw_un)
        tb.close()
        nstream = len(antenna1)
        ntimes = 0
        dotime = 1
        fld_un = numpy.unique(fldid)

        zfileRes = qa2_output_dir+'tsys_stat.txt'
        if os.path.exists (zfileRes): os.system('rm '+zfileRes)

        f = open (zfileRes, 'w')
        print 'opening file '+zfileRes

        #  Get average tsys per each scan/elevation

        vm = ValueMapping(ms1)
        scans = unique(scanno)
        nscans = len(scans)
        #   Loop over each scan
        TX_save = []
        TY_save = []
        elev_save = []
        conditions = listconditions(vis=ms1,byscan=True,scan='',antenna='0',verbose=False)
        for isc in range (0,nscans):
            tsys_x = []
            tsys_y = []
            for ist in range (0,nstream):
                if scans[isc] == scanno[ist]:
                    for nc in range(0.2*nchan, 0.8*nchan):
                        tsys_x.append(gain_amp[0,nc,ist])
                        tsys_y.append(gain_amp[1,nc,ist])


            T_x = numpy.median(tsys_x)
            T_y = numpy.median(tsys_y)
            Tmin = numpy.min(tsys_x+tsys_y)*0.7
            Tmax = numpy.max(tsys_x+tsys_y)*1.00
            TX_save.append(T_x)
            TY_save.append(T_y)
            elev_save.append(conditions[scans[isc]]['elevation'])
        #    sfield = vm.getFieldsForScan(scans[isc])

        nscans = len(elev_save)
        scan_tsys=[]
        f.write('\n\n')
        f.write('*********************************************************** \n')
        f.write('\n')
        f.write('              MEDIAN TSYS WITH SOURCE/ELEVATION \n')
        f.write('                  (see plots for details) \n')
        f.write('Scan   Fid                Source      Elev    Median T_x  Median T_y \n')
        for isc in range (0,nscans):
            sfield = vm.getFieldsForScan(scans[isc])
            fid = vm.getFieldIdsForFieldName(sfield)
            f.write('%4d  %4d %24s %6.1f   %7d     %7d \n'% (scans[isc], fid[0], sfield[0], elev_save[isc], TX_save[isc], TY_save[isc]))
        #    print '%4d  %4d %24s %6.1f   %7d     %7d \n'% (scans[isc], sfield[0], elev_save[isc], TX_save[isc], TY_save[isc])
            scan_tsys.append(sfield[0])


        #  Find outlier Tsys's

        #  Set up averages

        max_spw = []
        for ispw in range(0,nspw):
            varx = []
            avgx = []
            vary = []
            avgy = []
            for iant in range(0,nant):
                ggx = []
                ggy = []
                for id in range(0,nstream):
                    if (antenna1[id] == iant) & (spw[id] == ispw):
                        for ic in range(0.2*nchan, 0.8*nchan):
                            ggx.append(gain_amp[0][ic][id])
                            ggy.append(gain_amp[1][ic][id])

                varx.append(numpy.var(ggx))
                avgx.append(numpy.average(ggx))
                vary.append(numpy.var(ggy))
                avgy.append(numpy.average(ggy))

            medavgx = numpy.median(avgx)
            medavgy = numpy.median(avgy)
            medvarx = 0.0
            medvary = 0.0
            nx = 0
            ny = 0
            for iant in range(0,nant):
                difx = numpy.abs(avgx[iant]-medavgx)
                dify = numpy.abs(avgy[iant]-medavgy)
                if (difx < 5.0*medavgx):
                    medvarx = medvarx + numpy.abs(avgx[iant]-medavgx)
                    nx = nx + 1

                if (dify < 5.0*medavgy):
                    medvary = medvary + numpy.abs(avgy[iant]-medavgy)
                    ny = ny + 1


            medvarx = medvarx / numpy.sqrt(float(nx))
            medvary = medvary / numpy.sqrt(float(ny))
            max_spw.append(0.5*(medavgx+medavgy)+2.5*(medvarx+medvary))
            if ispw == 0:
                f.write('\n\n\n')
                f.write('            MEDIAN TSYS versus ANTENNA/SPW and OUTLIERS \n\n')
                f.write('              TSYS MEDIAN                >3-sigma OUTLIERS \n')
                f.write('       XPOL              YPOL                OUTLIERS \n')
                f.write(' SPW    T   rms        T   rms      antenna  Pol  tsys  n-sigma \n\n')

            f.write('%3d  %5.0f %4.0f     %5.0f %4.0f \n' %(ispw, medavgx, medvarx, medavgy, medvary))
            for iant in range(0,nant):
                offx = numpy.abs(avgx[iant]-medavgx)/medvarx
                offy = numpy.abs(avgy[iant]-medavgy)/medvarx
                if offx > 3.0: f.write('                                    %3d %4s   X %5d  %5.1f \n'%(iant, ant_names[iant],avgx[iant], offx))
                if offy > 3.0: f.write('                                    %3d %4s   Y %5d  %5.1f \n'%(iant, ant_names[iant],avgy[iant], offy))


        f.close()

        os.system('cat '+zfileRes)

        if makeplot == True:
            lchan = int(0.1*nchan); hchan = int(0.9*nchan); chanstr = str(lchan)+'~'+str(hchan)
            tsys_spw = self.getSpwInfo(ms1,intent='CALIBRATE_ATMOSPHERE').keys()
            numspw = len(tsys_spw)
            for i in range (0,numspw):
                if i != numspw-1:
                    plotcal(caltable = caltable, xaxis = 'freq', yaxis = 'amp', field = tsys_field,
                        spw = str(tsys_spw[i])+':'+chanstr, showgui = False, plotsymbol = ',',
                        plotrange = [0,0,0,max_spw[i]], subplot=221 + i)
                else:
                    plotcal(caltable = caltable, xaxis = 'freq', yaxis = 'amp', field = tsys_field,
                        spw = str(tsys_spw[i])+':'+chanstr, showgui = False, plotsymbol = ',',
                        plotrange = [0,0,0,max_spw[i]], subplot=221 + i,
                        figfile = qa2_output_dir+'tsys_plot.png')

    def wvr_stat(self, ms1='', refAnt='', qa2_output_dir=''):

        """
        wvr_stat.py

        This is an experimental qa2 python script that analyzes the
        bandpass calibrator data to determine the phase rms before and
        after application of wvr.  This is useful to determine if the
        the wvr corrections are reasonable, and to suggests flags or
        averaging of any bad antenna wvrgcals.  Uses spw 0, 'XX' only.

        INPUTS NEEDED:

           assumes visibility data set is ms1
           refAnt can be defined before entry, otherwise uses '0'
           makeplot can be defined before use, otherwise = T

        OUTPUTS:

        The ascii output file contains the statistics for each antenna (wrt
        to refAnt) and the output from wvrgcal in:

        <qa2_output_dir>/wvr_stat.txt'

        The corrected (green) and uncorrected (blue) phases are
        shown for each antenna in

        <qa2_output_dir>/wvr_plot.png'

        USEAGE: assumes ms1 as the visibility data set
                assumes refAnt='0', if not defined

        execfile (wvr_stat.py)

        """

        import numpy
        import pylab as pl
        from clearplot import clearplot
        from gaincal import gaincal

        try:
            refAnt
        except:
            refAnt = '0'
            refAntID = '0'
        else:
            refAntID = getAntennaIndex(ms1,refAnt)


        makeplot = True

        vm = ValueMapping(ms1)

        #  Get antenna parameters

        tb.open(ms1+'/ANTENNA')
        ant_names=tb.getcol('NAME')
        ant_pos=tb.getcol('POSITION')
        nant = len(ant_names)
        tb.close()


        #    Get bandpass scan

        scan_list = vm.uniqueScans
        for isc in scan_list:
            intent = vm.getIntentsForScan(isc)
            jintent = string.join(intent, '')
            if jintent.find('BANDPASS')!=-1:
                cscan = str(isc)
                break

        #    Get a science spw
        a = self.getSpwInfo(ms1); jspw = a.keys()[0]
        cspw = str(jspw)
        cpol = 'X'


        #   Remove previous phase gain solutions:
        os.system('rm -rf '+qa2_output_dir+'wvr_before')
        os.system('rm -rf '+qa2_output_dir+'wvr_after')

        if makeplot == True:
            clearplot()


        #   Open file for writing
        zfileRes = qa2_output_dir+'wvr_stat.txt'
        if os.path.exists (zfileRes): os.system('rm '+zfileRes)

        f = open (zfileRes, 'w')
        print 'opening file '+zfileRes

        #   Make phase gain solution with and withouth wvr correction

        gaincal(vis=ms1,
                refant=refAnt,calmode='p', selectdata=True,
                scan=cscan,spw=cspw,solint='int',
                caltable=qa2_output_dir+'wvr_before')

        #   Which gain table.  Smoothed or not?

        wvr_gaintable = ms1+'.wvr.smooth'
        if not os.path.exists(wvr_gaintable): wvr_gaintable = ms1+'.wvr'

        gaincal(vis=ms1,
                refant=refAnt,calmode='p',selectdata=True,
                scan=cscan,spw=cspw,solint='int',
                caltable=qa2_output_dir+'wvr_after',gaintable = wvr_gaintable)


        f.write('number of antennas=%3d  refAnt=%4s   \n' % (nant, refAnt))



        #  Get antenna separations

        ant_sep=[]
        for i in range(0,nant):
            temp = (ant_pos[0,i]-ant_pos[0,refAntID])**2 + (ant_pos[1,i]-ant_pos[1,refAntID])**2 +(ant_pos[2,i]-ant_pos[2,refAntID])**2
            ant_sep.append(numpy.sqrt(temp))

        #  Get phases before wvrgcal

        tb.open(qa2_output_dir+'wvr_before')
        time=tb.getcol('TIME')
        time = time-time[0]+0.0
        antenna1 = tb.getcol('ANTENNA1')
        gain = tb.getcol('GAIN')
        p_1 = numpy.arctan2(numpy.imag(gain),numpy.real(gain))*180.0 / 3.14159
        np=len(antenna1)
        #print 'number of phases ', np
        tb.close()

        #  Get phases after wvrgcal

        tb.open(qa2_output_dir+'wvr_after')
        time=tb.getcol('TIME')
        time = time-time[0]+0.0
        antenna1 = tb.getcol('ANTENNA1')
        gain = tb.getcol('GAIN')
        p_2 = numpy.arctan2(numpy.imag(gain),numpy.real(gain))*180.0 / 3.14159
        np2=len(antenna1)
        #print 'number of phases ', np2
        tb.close()

        #  get a phase stream for each antenna

        var_1=[]
        var_2=[]
        if makeplot: pl.close()


        #  Formatting of plots
        nsub3 = 0
        if nant <26: nsub1 = 5; nsub2 = 5

        if nant <21: nsub1 = 4; nsub2 = 5

        if nant <17: nsub1 = 4; nsub2 = 4

        if nant <13: nsub1 = 3; nsub2 = 4

        for iant in range(0,nant):
            if cpol == 'X':
                pa_1 = p_1[0][0]
                pa_2 = p_2[0][0]
            else:
                pa_1 = p_1[1][0]
                pa_2 = p_2[1][0]

            np = len(pa_1)
            pb_1 = []
            pb_2 = []
            tt = []

            for j in range(0,np):
                if iant == antenna1[j]:
                    pb_1.append(pa_1[j])
                    pb_2.append(pa_2[j])
                    tt.append(time[j])
            #
        #
            npc = len(pb_1)
            pc_1 = pb_1
            pc_2 = pb_2
        #           Remove lobe ambiguities
            for i in range(1,npc):
                pdiff = pc_1[i]-pc_1[i-1]
                pdiff = numpy.mod(pdiff+900.0,360.0)-180.0
                pc_1[i] = pc_1[i-1] + pdiff
                pdiff = pc_2[i]-pc_2[i-1]
                pdiff = numpy.mod(pdiff+900.0,360.0)-180.0
                pc_2[i] = pc_2[i-1] + pdiff

        #     plot phases
            if makeplot == True:
                nsub3 = nsub3 + 1
                pmax = numpy.max([numpy.max(pc_1), numpy.max(pc_2)])
                pmin = numpy.min([numpy.min(pc_1), numpy.min(pc_2)])
                pdiff = pmax-pmin
                if pdiff <10: pdiff = 10

                pmax = pmax + 0.25*pdiff
                pmin = pmin - 0.25*pdiff
                pl.subplot(nsub1,nsub2,nsub3)
                pl.plot(tt,pc_1,'b.')
                pl.ylim (pmin,pmax)
                pl.yticks([])
                pl.xticks([])
                pl.title('WVR '+ant_names[iant])
                pl.subplot(nsub1,nsub2,nsub3)
                pl.plot(tt,pc_2,'g.')
                pl.ylim (pmin,pmax)
                pl.yticks([])
                pl.xticks([])

            var_1.append(numpy.var(pc_1))
            var_2.append(numpy.var(pc_2))

        if makeplot: pl.savefig(qa2_output_dir+'wvr_plot.png')


        f.write('\n****************************************************************** \n\n')
        scan_string = '      Bandpass scan='+cscan+'; spw='+cspw+'; pol=X \n\n'
        f.write(scan_string)
        f.write('\n          PHASE FLUCTUATIONS OVER BANDPASS SCAN\n')
        f.write('          BEFORE AND AFTER WVR CORRECTION: '+wvr_gaintable+'\n\n')
        f.write('Ant      spacing  rms_before   rms_after     Percent      Coherence\n') 
        f.write('            (m)      (deg)      (deg)      before/after    percent\n\n') 
        for i in range(0,nant):
            coher = cos(sqrt(var_2[i])*pi/180.0)**2
            improved_string = '            '
            if var_1[i]<var_2[i]: improved_string = 'NOT IMPROVED'
            if var_1[i] > 0.1:
                f.write( '%4s   %8.1f    %6.1f     %6.1f        %4.0f          %5.0f  %12s\n' %(ant_names[i], ant_sep[i], sqrt(var_1[i]),sqrt(var_2[i]),100.0*sqrt(var_2[i]/var_1[i]),100.0*coher,improved_string))
        #
        #  Add log output from wvrgcal
        f.write ('\n\n  wvrgcal output of average wvr for each antenna\n\n')
        zwvrgcal_log = ms1+'.wvrgcal'
        f1 = open (zwvrgcal_log, 'r')
        fc = f1.readlines()
        f1.close
        for i in range(len(fc)):
            if (re.findall('Flag?',fc[i])):
                f.write(fc[i]);break


        for j in range(i+1,len(fc)):
            f.write(fc[j]) 

        f.close()
        #os.system('cat '+zfileRes)

    def glue_qa2(self, qa2_output_dir=''):

        cwd1 = os.getcwd()
        os.chdir(qa2_output_dir)

        os.system("rm qa2.pdf textfile.txt textfile.ps")

        os.system("cat wvr_stat.txt   tsys_stat.txt  NewListObs.txt   ant_gain_check.txt   bandpass_rms.txt  flux.txt   flag_stat.txt   sensitivity.txt  >  textfile.txt")

        os.system("enscript -Bc -fCourier5 --columns=2 -ptextfile.ps textfile.txt")

        os.system("ps2pdf textfile.ps qa2.pdf")

        #  combine all plots

        os.system("rm *part*png")

        #  CONVERT IS USED TO ADD IN LABELS

        os.system("convert obs_display.png -font Utopia-Regular -pointsize 48 label:'Observing Schedule' -gravity Center -append obs_display_labeled.png")
        os.system("convert mosaic_plot.png -font Utopia-Regular -pointsize 48 label:'Mosaic Pointing Configuration' -gravity Center -append mosaic_plot_labeled.png")
        os.system("convert antenna_config.png -font Utopia-Regular -pointsize 48 label:'Antenna Configuration' -gravity Center -append antenna_config_labeled.png")
        os.system("convert wvr_plot.png -font Utopia-Regular -pointsize 48 label:'Phase: before/after WVR' -gravity Center -append wvr_plot_labeled.png")
        os.system("convert ant_amp_temporal.png -font Utopia-Regular -pointsize 48 label:'Temporal gain calibration' -gravity Center -append ant_amp_temporal_labeled.png")
        os.system("convert ant_phase_temporal.png -font Utopia-Regular -pointsize 48 label:'Temporal phase calibration' -gravity Center -append ant_phase_temporal_labeled.png")

        os.system("convert bandpass_avg.png -font Utopia-Regular -pointsize 48 label:'Average Bandpass for each spw' -gravity Center -append bandpass_avg_labeled.png")
        os.system("convert bandpass_bad.png -font Utopia-Regular -pointsize 48 label:'Poor Bandpass' -gravity Center -append bandpass_bad_labeled.png")
        os.system("convert tsys_plot.png -font Utopia-Regular -pointsize 48 label:'Composite Tsys for each spw' -gravity Center -append tsys_plot_labeled.png")
        os.system("convert target_spectrum.png -font Utopia-Regular -pointsize 48 label:'Target Spectrum for each spw' -gravity Center -append target_spectrum_labeled.png")

        os.system("convert ampcal_uvdist.png -font Utopia-Regular -pointsize 48 label:'Flux calibration model and data' -gravity Center -append ampcal_uvdist_labeled.png")
        os.system("convert phase_cal_uvdist.png -font Utopia-Regular -pointsize 48 label:'Cal amp/phase vs uvdist for each spw' -gravity Center -append phase_cal_uvdist_labeled.png")
        os.system("convert phase_cal_freq.png -font Utopia-Regular -pointsize 48 label:'Cal amp/phase vs freq for each spw' -gravity Center -append phase_cal_freq_labeled.png")
        os.system("convert target_uv.png -font Utopia-Regular -pointsize 48 label:'Target u-v coverage' -gravity Center -append target_uv_labeled.png")
        os.system("convert target_image.png -font Utopia-Regular -pointsize 48 label:'Target Image' -gravity Center -append target_image_labeled.png")
        os.system("convert target_psf.png -font Utopia-Regular -pointsize 48 label:'Target psf' -gravity Center -append target_psf_labeled.png")

        os.system("montage -tile 2X3 -geometry 1000x1000+0+0   obs_display_labeled.png   mosaic_plot_labeled.png   antenna_config_labeled.png   wvr_plot_labeled.png  ant_amp_temporal_labeled.png  ant_phase_temporal_labeled.png   qa2_part1.png")

        os.system("montage -tile 2X3 -geometry 1000x1000+0+0   bandpass_avg_labeled.png   tsys_plot_labeled.png  target_spectrum_labeled.png  qa2_part2.png")

        os.system("montage -tile 2x3 -geometry 1000x1000+0+0  ampcal_uvdist_labeled.png  phase_cal_uvdist_labeled.png  phase_cal_freq_labeled.png    target_uv_labeled.png   target_image_labeled.png  target_psf_labeled.png   qa2_part3.png")

        #os.system("display *qa2_part*png &")

        os.chdir(cwd1)

    def generateQA2Report(self, ms1='', ms2='', phase_cal='', target='', target_source='', tsys_field='', dospw='', refAnt='', qa2_output_dir='', uvrange='0~30m'):

        """

        QA2 Package to interface with Eric's latest scripts.  Older scripts will
        work, but some names of calibration files may be different.

        Version 2.1:  Ed Fomalont, Feb 27, 2012
        Version 2.2:  streamlined, Amy Kimball, May 1, 2012


        INSTRUCTIONS:



        1.  Run Eric's latest reduction script in an appropriate directory.

            You can Eric's script from the beginning to the end, and,then run
        the entire qa2 script, or you can run Eric's script by cutting and
        pasting, and adding the appropriate qa2 scripts as you go.

        2.  Make a directory <qa2_output> in an appropriate parent <QA2>
        directory.

        3.  Modify 'sample_qa2.py' to point to the script directory, data
        reduction directories, and output directories.  Set the global
        parameter values appropriate to your data set.

        4.  execfile ('sample_qa2.py') to create the qa2 txt and png files.
        This will take about 10 minutes to run.

        5.  Go to the qa2 directory and glue the txt and png files using
        execfile ('glue_qa2.py').  It may be safer to run each step
        separately.



        """

        #  qa2 plots and text Generating script

        #  Any initialization stuff

        #import analysisUtils as aU
        import numpy
        import time
        import os

        #es = aU.stuffForScienceDataReduction()

        ####  Point to directory that contains all of the qa2 mini-scripts
        #qa2_scripts_dir = './'

        ####  Point to data directories
        ## PreCalibration directory should contain .ms, .ms.wvr table, .ms.tsys table
        #PreCal_dir = '../reduction_X69/'
        ## Calibration directory should contain .ms.split, 
        #Cal_dir = '../reduction_X69/'
        ## output directory
        #qa2_output_dir = '../reduction_X69/qa2/'

        ####  Global parameters specific to data set:

        #asdmName = 'uid___A002_X383b50_X69'  #  base name of data
        #phase_cal = '3'                       #  Need field number of main phase cal
        #target = '4'                          #  Target field for quick image check
        #tsys_field = '4'                      #  field with tsys.  needed for sensit.
        #refAnt = 'DV05'                       #  reference antenna by name
        #dospw = '0,1' # for target_spectrum.py (put each spw separately)

        #### End of needed parameters

        #ms1 = PreCal_dir+asdmName+'.ms'
        #ms2 = Cal_dir+asdmName+'.ms.split'

	#-------------

 	ms2 = ms1+'.split'

	phaseCalInfo = self.getPhaseCal(ms1)

	intentsAndSourcesInfo = self.getIntentsAndSourceNames(ms1)
	if len(intentsAndSourcesInfo['OBSERVE_TARGET']['sourceid']) > 1:
            print "# Note: there are more than one science target: i'm picking the first one."
        if (target_source == ''):
            target_source = str(intentsAndSourcesInfo['OBSERVE_TARGET']['sourceid'][0])  # ID as string
	target_source_name = intentsAndSourcesInfo['OBSERVE_TARGET']['name'][0]      # name as string
	if target_source_name in intentsAndSourcesInfo['CALIBRATE_ATMOSPHERE']['name']:
            ij = intentsAndSourcesInfo['CALIBRATE_ATMOSPHERE']['name'].index(target_source_name)
            tsys_field = intentsAndSourcesInfo['CALIBRATE_ATMOSPHERE']['idstring'][ij]
	else:
	    print "Note: no Tsys measurement on science target: using phase calibrator."
	    if phaseCalInfo[target_source_name]['phaseCalName'] not in intentsAndSourcesInfo['CALIBRATE_ATMOSPHERE']['name']: sys.exit('Error')
            ij = intentsAndSourcesInfo['CALIBRATE_ATMOSPHERE']['name'].index(phaseCalInfo[target_source_name]['phaseCalName'])
	    tsys_field = intentsAndSourcesInfo['CALIBRATE_ATMOSPHERE']['idstring'][ij]

	phase_cal = str(phaseCalInfo[target_source_name]['phaseCalId'])
        if (target == ''):
            target = str(phaseCalInfo[target_source_name]['sciFieldIds'][0])   # ID as string

	spwInfo = self.getSpwInfo(ms1).keys()
	spwInfo = range(len(spwInfo))
	dospw = ','.join([str(k) for k in spwInfo])

	if refAnt == '': refAnt = self.getRefAntenna(ms1)

	if qa2_output_dir == '':
    	    msdir = os.path.dirname(os.path.abspath(ms1))
	    if os.path.exists(msdir+'/qa2') == False:
	        os.makedirs(msdir+'/qa2')
	    qa2_output_dir = msdir+'/qa2/'
	else:
	    qa2_output_dir += '/'

	#-------------

        print '# Get Colorful Diagram of Observations'

        #makeplot = T
        self.listobs3(msName=ms1,figfile=qa2_output_dir+'obs_display.png')

        print '# Get listings of experiment and antenna-config plot'

        print "# If mosaic, make this plot"

        #execfile (qa2_scripts_dir+'mosaic_plot.py')
        self.mosaic_plot(ms1=ms1, qa2_output_dir=qa2_output_dir)

        print '# Determine wvr correction from bandpass obs'

        #execfile (qa2_scripts_dir+'wvr_stat.py')
        self.wvr_stat(ms1=ms1, refAnt=refAnt, qa2_output_dir=qa2_output_dir)

        print '# Tsys statistics and plots'

        #makeplot = T
        #execfile (qa2_scripts_dir+'tsys_stat.py')
        self.tsys_stat(ms1=ms1, tsys_field=tsys_field, makeplot=True, qa2_output_dir=qa2_output_dir)

        print '# listing of split data set'

        #makeplot = T
        #execfile (qa2_scripts_dir+'listobs2.py')
        self.listobs2(ms2=ms2, makeplot=True, qa2_output_dir=qa2_output_dir)

        print '# Any antennas shadowed?'

        #execfile (qa2_scripts_dir+'shadowed_ant.py')
        self.shadowed_ant(ms2=ms2, qa2_output_dir=qa2_output_dir)

        print '# Check antenna amp of bandpass scan'

        #execfile (qa2_scripts_dir+'ant_gain_check.py')
        self.ant_gain_check(ms2=ms2, qa2_output_dir=qa2_output_dir)

        print '# Check channel to channel rms of bandpass scan and plot outliers'

        #execfile (qa2_scripts_dir+'bandpass_rms.py')
        self.bandpass_rms(ms2=ms2, refAnt=refAnt, qa2_output_dir=qa2_output_dir)

        print '# Make an spw average bandpass plots for atmosphere effects'

        #execfile (qa2_scripts_dir+'bandpass_plot.py')
        self.bandpass_plot(ms2=ms2, qa2_output_dir=qa2_output_dir)

        print '# Printout of flux density determinations'

        #execfile (qa2_scripts_dir+'flux_values.py')
        self.flux_values(ms2=ms2, qa2_output_dir=qa2_output_dir)

        print '# Plot temporal antenna amp calibration of phase cal'

        caltable = ms2+'.flux_inf'
        #execfile (qa2_scripts_dir+'ant_amp_temporal.py')
        self.ant_amp_temporal(ms2=ms2, phase_cal=phase_cal, caltable=caltable, qa2_output_dir=qa2_output_dir)

        print '# Plot temporal antenna phase calibration of phase cal'

        caltable = ms2+'.phase_inf'
        #execfile (qa2_scripts_dir+'ant_phase_temporal.py')
        self.ant_phase_temporal(ms2=ms2, caltable=caltable, phase_cal=phase_cal, qa2_output_dir=qa2_output_dir)

        print '# Plot fluxcal object model and calibrated data'

        #execfile (qa2_scripts_dir+'ampcal_uvdist.py')
        self.ampcal_uvdist(ms2=ms2, qa2_output_dir=qa2_output_dir)

        print '# Check uvdist and freq response of uv calibration phase cal'

        #execfile (qa2_scripts_dir+'phase_cal_check.py')
        self.phase_cal_check(ms2=ms2, phase_cal=phase_cal, qa2_output_dir=qa2_output_dir)

        print '# Obtain flagging statistics'

        #execfile (qa2_scripts_dir+'flag_stats.py')
        self.flag_stats(ms2=ms2, qa2_output_dir=qa2_output_dir)

        print '#  Check target properties'

        #execfile (qa2_scripts_dir+'target_check.py')
        self.target_check(ms2=ms2, target=target, target_source=target_source, tsys_caltable=ms1+'.tsys', tsys_field=tsys_field, qa2_output_dir=qa2_output_dir)

        print '#  Get a spectrum on the target'

        #execfile (qa2_scripts_dir+'target_spectrum.py')
        self.target_spectrum(ms2=ms2, target=target, dospw=dospw, qa2_output_dir=qa2_output_dir, uvrange=uvrange)

        #  Make overall pdf file of text file using
        #  glue_qa2.py
        #  See instructions in README
        self.glue_qa2(qa2_output_dir=qa2_output_dir)

    def generateScriptForPI(self, fname=''):

        f = open(fname, 'r')
        fc = f.read()
        f.close()

        casaCmd = re.findall('^ *(?:split|applycal) *\( *vis.*?\)', fc, re.DOTALL|re.MULTILINE|re.IGNORECASE)

        name1, ext1 = os.path.splitext(fname)
        fname1 = name1 + '_forPI' + ext1

        f = open(fname1, 'w')
        for i in casaCmd:
            print >> f, i
            print >> f, ''
        f.close()

# end of class definition for stuffForScienceDataReduction

class linfit:
  def pickRandomError(self):
    """
    Picks a random value from a Gaussian distribution with mean 0 and standard deviation = 1.
    """
    w = 1.0
    while ( w >= 1.0 ):
      x1 = 2.0 * random.random() - 1.0
      x2 = 2.0 * random.random() - 1.0
      w = x1 * x1 + x2 * x2

    w = np.sqrt( (-2.0 * np.log( w ) ) / w )
    y1 = x1 * w
    y2 = x2 * w
    return(y1)

  def readFluxscaleResult(self,xfilename, yfilename, source, verbose=False,
                          maxpoints=0,spwlist=[],referenceFrame='TOPO'):
    """
    Specific function to read CASA output files from listobs and fluxscale.
    It returns the log10() of the frequency and flux densities read. The
    flux density uncertainties returned are scaled by the flux density values.
    """
    fx = open(xfilename,'r')
    fy = open(yfilename,'r')
    lines = fy.readlines()
    fy.close()
    x = []
    y = []
    yerror = []
    skiplist = ''
    trueSource = ''
    sourcesFound = []
    ignoreSpw = []  # This will be a list of spws with "Insufficient data"
    for line in lines:
        if (line.find('Flux')>=0 and (source=='' or line.find(source)>=0)):
            tokens = line.split()
            for t in range(len(tokens)):
                if (tokens[t].find('SpW')>=0):
                    #  We have specified the spws to include
                    spw = int(tokens[t].split('=')[1])
                    if (line.find('INSUFFICIENT DATA')>=0):
                        ignoreSpw.append(spw)
                        print "Skipping spw %d due to INSUFFICIENT DATA result from fluxscale." % (spw)
                        break
                if (tokens[t]=='is:'):
                    if (spw not in spwlist and spwlist != []):
                      skiplist += str(spw) + ','
                    else:
#                      print "Using spw %d" % spw
                      if (tokens[t-4] != 'for'):
                        # there is a blank in the name
                        trueSource = tokens[t-4] + ' ' + tokens[t-3]
                      else:
                        trueSource = tokens[t-3]  # no blanks were found in the name
                      sourcesFound.append(trueSource)
                      y.append(float(tokens[t+1]))
                      yerror.append(float(tokens[t+3]))
                      break
            if (len(y) == maxpoints and maxpoints>0): break
    if (len(y) == 0):
        if (trueSource == ''):
          print "Did not find any flux densities for source = %s" % (source)
        else:
          print "Did not find any flux densities for source = %s" % (trueSource)
        return([],[],[],[],[])
    if (len(skiplist) > 0):
      print "Skipping spw ", skiplist[0:-1]
    lines = fx.readlines()
    fx.close()
    bw = []
    freqUnits = ''
    spwsKept = []
    for line in lines:
        loc=line.find('Ch1(')
        if (loc>=0):
            freqUnits = line[loc+4:loc+7]
            if (verbose):
                print "Read frequency units = ", freqUnits
        if (line.find(referenceFrame)>=0):
            tokens = line.split()
            for t in range(len(tokens)):
                    if (tokens[t]==referenceFrame):
                        spw = int(tokens[t-2])
                        if ((spw not in spwlist and spwlist != []) or spw in ignoreSpw):
#                            print "Skipping spw %d" % spw
                            continue
                        else:
                            spwsKept.append(spw)
                            bw.append(float(tokens[t+3]))
                            x.append(float(tokens[t+1])+bw[-1]*0.5*0.001)
                        break
                    if (len(x) == maxpoints and maxpoints>0):
                        print "Stopping after readings %d points ---------------" % (maxpoints)
                        break
    if (len(y) != len(x)):
        print "There is a mismatch between the number of spws in the %s frame (%d" % (referenceFrame,len(x))
        print "and the number of valid flux densities (%d)." % (len(y))
        if (len(np.unique(sourcesFound)) > 1):
            print "Please limit the fit to one source: ", np.unique(sourcesFound)
        return([],[],[],[],[],[])
    if (verbose):
        print "Read %d x values for %s = " % (len(x),trueSource), x
        print "Read %d y values for %s = " % (len(y),trueSource), y
    else:
        print "Read %d x values for %s (avg=%.3f)" % (len(x),trueSource,np.mean(x))
        print "Read %d y values for %s (avg=%.3f)" % (len(y),trueSource,np.mean(y))
    if (freqUnits.find('MHz') >= 0):
        x = np.array(x)*0.001
        freqUnits = 'GHz'
    logx = np.log10(x)
    logyerror = list(np.array(yerror) / np.array(y))
    logy = np.log10(y)
    return (logx,logy,logyerror,trueSource,freqUnits,spwsKept)
# end of readFluxscaleResult

  def parse_spw_argument(self,spw):
    """
    # returns an integer list of spws based on a string entered by the user
    #  e.g.  '1~7,9~15' yields [1,2,3,4,5,6,7,9,10,11,12,13,14,15]
    #        [1,2,3] yields [1,2,3]
    #        1 yields [1]
    """
    if (type(spw) == list):
        return(spw)
    elif (type(spw) == int):
        return([spw])
    sublists = spw.split(',')
    spwlist = []
    for s in sublists:
        spwrange = s.split('~')
        if (len(spwrange) > 1):
            try:
                firstSpw = int(spwrange[0])
                try:
                    secondSpw = int(spwrange[1])
                    for w in range(int(spwrange[0]),int(spwrange[1])+1):
                        spwlist.append(w)
                except:
                    print "Unrecognized element in spw string: %s" % (spwrange[1])
            except:
                print "Unrecognized element in spw string: %s" % (spwrange[0])
        else:
            try:
                spwlist.append(int(spwrange[0]))
            except:
                print "Unrecognized element in spw string: %s" % (spwrange[0])
    return spwlist

  def spectralIndexFitterMonteCarlo(self,filename,yfilename,degree=1,source='',
                          verbose=False,maxpoints=0,trials=2000,spw='',
                          referenceFrame='TOPO'):
    """
    # If error bars are included with the data, then the return values are:
    #     p, covar contain the results of the weighted fit
    #     p2 contains the results of the unweighted fit
    # If no error bars are included, then
    #     p contains the results of the unweighted fit
    """
    spwlist = []
    nullReturn = ([],[],[],[],[], [],[],[],[],[], [])
    if (spw != [] and spw != ''):
      spwlist = self.parse_spw_argument(spw)
      if (len(spwlist) < 2):
          print "I need more than 1 spws to fit a spectral index."
          return(nullReturn)
      else:
          print "Will use spw = ", spwlist
    (x,y,yerror,source,freqUnits,spwsKept) = self.readFluxscaleResult(filename,yfilename,source,verbose,maxpoints,spwlist,referenceFrame=referenceFrame)
    if (len(x) == 0 or len(y) == 0):
        return(nullReturn)

    # normalize the X values about 0 such that the uncertainties between slope
    # and intercept will be uncorrelated
    meanOfLogX = np.mean(x)
    x = x-meanOfLogX

    if (len(yerror)>0):
        fitfunc = lambda p, x: p[0]*x + p[1]
        errfunc = lambda p,x,y,err: (y-fitfunc(p,x))/err
        pinit = [1.0,-1.0]
        slope = []
        yoffset = []
        errors = np.zeros(len(y))
        # first trial is with no errors
        for t in range(trials):
            ytrial = list(np.array(y) + np.array(yerror)*np.array(errors))
            out = optimize.leastsq(errfunc, pinit, args=(x,ytrial,yerror),full_output=1)
            p = out[0]
            if (t==0):
                covar = out[1]
            slope.append(p[0])
            yoffset.append(p[1])
            errors = []
            for e in range(len(y)):
                errors.append(self.pickRandomError())
        p = [slope[0], yoffset[0]]
        pmedian = [np.median(slope), np.median(yoffset)]
        perror = [np.std(slope), np.std(yoffset)]
        p2 = np.polyfit(x,y,degree)
    else:
        p = np.polyfit(x,y,degree)
        covar = []
        p2 = []
        perror = []
        pmedian = []

    return(p,covar,x,y,yerror,p2,source,freqUnits, pmedian, perror, spwsKept, meanOfLogX)
  # end of spectralIndexFitterMonteCarlo

  def spectralIndexFitter2(self,filename,yfilename,degree=1,source='',
                          verbose=False,maxpoints=0,spw='',
                          referenceFrame='TOPO'):
    """
    # If error bars are included with the data, then the return values are:
    #     p, covar contain the results of the weighted fit
    #     p2 contains the results of the unweighted fit
    # If no error bars are included, then
    #     p contains the results of the unweighted fit
    """
    spwlist = []
    nullReturn = ([],[],[],[],[], [],[],[],[],[], [])
    if (spw != [] and spw != ''):
      spwlist = self.parse_spw_argument(spw)
      if (len(spwlist) < 2):
          print "I need more than 1 spws to fit a spectral index."
          return(nullReturn)
      else:
          print "Will use spw = ", spwlist

    # yerror returned by readFluxsacleResult = y_uncertainty / y
    (x,y,yerror,source,freqUnits,spwsKept) = self.readFluxscaleResult(filename,yfilename,source,
                                         verbose,maxpoints,spwlist,referenceFrame=referenceFrame)
    if (len(x) == 0 or len(y) == 0):
        return(nullReturn)

    # normalize the X values about 0 such that the uncertainties between slope
    # and intercept will be uncorrelated
    meanOfLogX = np.mean(x)
#    print "Mean of log(x) = ", meanOfLogX
    x = x-meanOfLogX
#    print "x = ", x

    # simple fit with no errors
    p2 = np.polyfit(x,y,degree)

    x = 10**x
    y = 10**y

    if (len(yerror)>0):
        fitfunc = lambda p, x: p[0] + p[1]*np.log10(x)
        errfunc = lambda p,x,y,err: (np.log10(y)-fitfunc(p,x))/err
        pinit = [1.0,-1.0]
        out = optimize.leastsq(errfunc, pinit, args=(x,y,yerror),full_output=1)
        p = out[0]
        covar = out[1]
    else:
        print "No uncertainties were found for the flux density measurements."

    return(p,x,y,yerror,covar,source,freqUnits,spwsKept,p2,meanOfLogX)
  # end of spectralIndexFitter2

  def spectralIndex(self,filename,yfilename,source='',verbose=False,
                    maxpoints=0,trials=0,spw='',help=False,plotdir='',
                    labelspw=False,referenceFrame='TOPO'):
    # This is just a synonym for spectralindex()
    """
    Usage: spectralIndex(filename='',yfilename='',source='',verbose=False,
                         maxpoints=0,trials=0,spw='',help=False,plotdir='')
    filename: contains a listobs output file
    yfilename: contains a fluxscale output file
    source: sourcename to choose from the (possibly) multi-source fluxscale file
    maxpoints: the maximum number of spws to select for the fit (0=no max)
    trials: if > 0, use a Monte-Carlo technique estimate the fit uncertainties,
       otherwise, use the sqrt(covar_matrix) from scipy.optimize.leastsq (default)
       There is a minimum number of 100 trials, and ~1000 is recommended.
    spw: the spws to use, e.g. ''=all, '1~3,5,6~8'=[1,2,3,5,6,7,8]
    plotdir: the directory in which to write the plotfile
    labelspw: draw spw numeric labels adjacent to each point
    referenceFrame: the frequency reference frame, if not 'TOPO'
    -- Todd Hunter
    """
    return(self.spectralindex(filename,yfilename,source,verbose=verbose,
                              maxpoints=maxpoints,trials=trials,spw=spw,
                              help=help,plotdir=plotdir,labelspw=False,
                              referenceFrame=referenceFrame))

  def spectralIndexCovarMatrix(self,filename='',yfilename='',source='',verbose=False,
                               maxpoints=0,spw='',help=False,plotdir='',
                               labelspw=False,referenceFrame='TOPO'):
    """
    This function is designed to fit the spectral index to the results
    output from casa's fluxscale task, and make a summary plot. Currently,
    it requires the output from the casa listobs task to determine the center
    frequencies of each spectral window.  It calls scipy.optimize.leastsq()
    and uses the sqrt(covariance matrix) to determine the uncertainty on the fitted
    slope on the basis of the error bars on each flux density. -- Todd Hunter
    Usage: spectralIndexCovarMatrix(filename='',yfilename='',source='',verbose=False,
                         maxpoints=0,spw='',help=False,plotdir='',labelspw=False)
    filename: contains a listobs output file
    yfilename: contains a fluxscale output file
    source: sourcename to choose from the (possibly) multi-source fluxscale file
    maxpoints: the maximum number of spws to select for the fit (0=no max)
    spw: the spws to use, e.g. ''=all, '1~3,5,6~8'=[1,2,3,5,6,7,8]
    plotdir: the directory in which to write the plotfile
    labelspw: draw spw numeric labels adjacent to each point
    referenceFrame: the frequency reference frame, if not 'TOPO'
    """
    if (help):
        print "Usage: spectralIndexCovarMatrix(filename='',yfilename='',source='',verbose=False,"
        print "          maxpoints=0,spw='',help=False,plotdir='',labelspw=False,"
        print "          referenceFrame='TOPO')"
        print "  filename: contains a listobs output file"
        print "  yfilename: contains a fluxscale output file"
        print "  source: sourcename to choose from the (possibly) multi-source fluxscale file"
        print "  maxpoints: the maximum number of spws to select for the fit (0=no max)"
        print "  spw: the spws to use, e.g. ''=all, '1~3,5,6~8'=[1,2,3,5,6,7,8]"
        print "  plotdir: the directory in which to write the plotfile"
        print "  labelspw: draw spw numeric labels adjacent to each point"
        print "  referenceFrame: the frequency reference frame, if not 'TOPO'"
        return
    # x & y are returned in linear units of frequency (normalized about 1 MHz)
    # and flux density (usually Jy)
    # yerror is the (uncertainty in y) divided by y
    (p,x,y,yerror,covar,source,freqUnits,spwsKept,p2,meanOfLogX) = \
        self.spectralIndexFitter2(filename,yfilename,degree=1,source=source,
                                 verbose=verbose,maxpoints=maxpoints,
                                 spw=spw, referenceFrame=referenceFrame)
    if (plotdir == ''):
        plotdir = './'
    if (plotdir[-1] != '/'):
        plotdir += '/'
    if (p == []):
        # If the data import failed, then the fit will be blank, so return.
        return(p)

    # Get the result of the fit
    yoffset = p[0]
    slope = p[1]
    slopeFitError = np.sqrt(covar[1][1])
    freq = x[0]
    freqUnNormalized = 10**(np.log10(freq) + meanOfLogX)

    amp = 10**(yoffset + np.log10(freq)*slope)
#    print "x=",x
#    print "amp at (freq=%f) = %f" %(freq,amp)
#    print "covar = ", covar
    ampError = amp * np.sqrt(covar[0][0])

    print "Error-weighted fit: Slope: %.3f+-%.3f  Flux D. @ %.3f%s: %.3f+-%.3f" % (slope,
                                 slopeFitError, freqUnNormalized, freqUnits, amp, ampError)
    amp2 = (10.0**p2[1]) * (freq**p2[0])
    print "   Un-weighted fit: Slope: %.3f         Flux D. @ %.3f%s: %.3f" % (p2[0], freqUnNormalized, freqUnits, amp2)
    rc('text',usetex=True)
    pb.clf()
    desc = pb.subplot(111)
    desc.xaxis.grid(which='major')
    desc.yaxis.grid(which='major')
    logx = np.log10(x)
    logy = np.log10(y)
    yfit = yoffset + logx*slope

    # Calculate curves for the 1-sigma slope solutions
    minslope = logx*(slope-slopeFitError) + yoffset
    maxslope = logx*(slope+slopeFitError) + yoffset

    # Now apply the scaled error in the yoffset
    minslope2 = np.log10(10**minslope - ampError)
    minslope3 = np.log10(10**minslope + ampError)
    maxslope2 = np.log10(10**maxslope - ampError)
    maxslope3 = np.log10(10**maxslope + ampError)
    minvalue = np.minimum(np.minimum(minslope2,minslope3),np.minimum(maxslope2,maxslope3))
    maxvalue = np.maximum(np.maximum(minslope2,minslope3),np.maximum(maxslope2,maxslope3))
    
    loglogplot=False
    if (loglogplot):
        # experimental!!!
        pb.loglog(x,y,'ko',x,10**yfit,'r-', x,10**minslope,'r--', x,10**maxslope,'r--')
        lower = y*yerror
        upper = y*yerror
        print "lower, upper = ", lower, upper
        pb.errorbar(x,y,yerr=[lower,upper],color='k',ls='None')
        if (freqUnits != ''):
            pb.xlabel('Frequency(%s)'%freqUnits)
        else:
            pb.xlabel('Frequency')
        pb.ylabel('FluxDensity(Jy)')
        x0 = np.min(x)
        x1 = np.max(x)
        xr = x1-x0
        y0 = np.min(y)
        y1 = np.max(y)
        yr = y1-y0
        r = range(int(np.floor(np.min(x)*0.001)), int(np.ceil(np.max(x)*0.001)))
        print "xticks = ", r
        desc.set_xticks(r)
        desc.set_yticks(range(int(np.floor(np.min(y)*1)), int(np.ceil(np.max(y)*1))))
        desc.set_xscale("log",subsx=range(2,10))
        desc.set_yscale("log",subsy=range(2,10))
        pb.xlim([x0-xr*0.1, x1+xr*0.1])
        pb.ylim([y0-yr*0.1, y1+yr*0.1])
    else:
        logx += meanOfLogX
        pb.plot(logx,logy,'ko',logx,yfit,'r-',# logx,minslope,'r--', logx,maxslope,'r--',
                logx, minvalue,'r--',logx,maxvalue,'r--')
        lower = np.log10(y) - np.log10(y-y*yerror) 
        upper = np.log10(y+y*yerror) - np.log10(y)
        pb.errorbar(logx,logy,yerr=[lower,upper],color='k',ls='None')
        if (freqUnits != ''):
            pb.xlabel('Log$_{10}$(Frequency(%s))'%freqUnits)
        else:
            pb.xlabel('Log$_{10}$(Frequency)')
        pb.ylabel('Log$_{10}$(FluxDensity(Jy))')
        pb.axis('equal')
    pb.title(source+'  spectral index=%+.3f$\pm$%.3f, F(%.2f%s)=%.3f$\pm$%.3f Jy'%(slope,slopeFitError,freqUnNormalized,freqUnits,amp,ampError),size=14)
    if (labelspw):
        for i in range(len(x)):
            xlab = np.log10(x[i])+meanOfLogX
            ylab = np.log10(y[i])
            pb.text(xlab, ylab,'%d'%(spwsKept[i]),size=10)
    ytext = self.replaceUnderscores(yfilename)
    pb.text(0.01, 0.01, 'data from %s'%(ytext), transform=desc.transAxes,size=10)
#    pb.text(0.01, 0.01, 'data from %s, spw=%s'%(ytext,','.join(map(str,spwsKept))), transform=desc.transAxes,size=10)
    pngname = '%s%s.%s.png' % (plotdir,yfilename,source)
    pb.savefig(pngname)
    print "Plot saved in %s" % (pngname)
    pb.draw()
    return
# end of spectralIndexCovarMatrix()


  def spectralindex(self,filename='',yfilename='',source='',verbose=False,
                    maxpoints=0,trials=0,spw='',help=False,plotdir='',
                    labelspw=False,referenceFrame='TOPO'):
    """
    This function is designed to fit the spectral index to the results
    output from casa's fluxscale task. Currently, it requires the output
    from the listobs task to determine the center frequencies of each spw.
    Usage: spectralIndex(filename='',yfilename='',source='',verbose=False,
                         maxpoints=0,trials=0,spw='',help=False,plotdir='')
    filename: contains a listobs output file
    yfilename: contains a fluxscale output file
    source: sourcename to choose from the (possibly) multi-source fluxscale file
    maxpoints: the maximum number of spws to select for the fit (0=no max)
    trials: if > 0, use a Monte-Carlo technique estimate the fit uncertainties,
       otherwise, use the sqrt(covar_matrix) from scipy.optimize.leastsq (default).
       There is a minimum number of 100 trials, and ~1000 is recommended.
    spw: the spws to use, e.g. ''=all, '1~3,5,6~8'=[1,2,3,5,6,7,8]
    plotdir: the directory in which to write the plotfile
    labelspw: draw spw numeric labels adjacent to each point
    referenceFrame: the frequency reference frame, if not 'TOPO'
    """
    if (help):
        print "Usage: spectralindex(filename='',yfilename='',source='',verbose=False,"
        print "          maxpoints=0,trials=0,spw='',help=False,plotdir='',labelspw=False,"
        print "          referenceFrame='TOPO')"
        print "  filename: contains a listobs output file"
        print "  yfilename: contains a fluxscale output file"
        print "  source: sourcename to choose from the (possibly) multi-source fluxscale file"
        print "  maxpoints: the maximum number of spws to select for the fit (0=no max)"
        print "  trials: if > 0, use a Monte-Carlo technique estimate the fit uncertainties,"
        print "     otherwise, use the sqrt(covarMatrix) from scipy.optimize.leastsq (default)."
        print "     (There is a minimum number of 100 trials, and ~1000 is recommended.)"
        print "  spw: the spws to use, e.g. ''=all, '1~3,5,6~8'=[1,2,3,5,6,7,8]"
        print "  plotdir: the directory in which to write the plotfile"
        print "  labelspw: draw spw numeric labels adjacent to each point"
        print "  referenceFrame: the frequency reference frame, if not 'TOPO'"
        return
    if (trials > 0):
          if (trials < 100):
              trials = 100
          self.spectralIndexMonteCarlo(filename,yfilename,source,verbose,maxpoints,
                                       trials,spw,help,plotdir,labelspw,referenceFrame)
    else:
          self.spectralIndexCovarMatrix(filename,yfilename,source,verbose,maxpoints,
                                        spw,help,plotdir,labelspw,referenceFrame)
          
  def spectralIndexMonteCarlo(self,filename='',yfilename='',source='',verbose=False,
                              maxpoints=0,trials=2000,spw='',help=False,plotdir='',
                              labelspw=False,referenceFrame='TOPO'):
    """
    This function is designed to fit the spectral index to the results
    output from casa's fluxscale task. Currently, it requires the output
    from the listobs task to determine the center frequencies of each
    spectral window.  It runs a brief Monte-Carlo series of fits to
    determine the uncertainty on the fitted slope on the basis of the
    error bars on each flux density.    -- Todd Hunter
    Usage: spectralIndexMonteCarlo(filename='',yfilename='',source='',verbose=False,
           maxpoints=0,trials=2000,spw='',help=False,plotdir='',labelspw=False)
    filename: contains a listobs output file
    yfilename: contains a fluxscale output file
    source: sourcename to choose from the (possibly) multi-source fluxscale file
    maxpoints: the maximum number of spws to select for the fit (0=no max)
    trials: number of Monte-Carlo fits to run to estimate the fit uncertainties
    spw: the spws to use, e.g. ''=all, '1~3,5,6~8'=[1,2,3,5,6,7,8]
    plotdir: the directory in which to write the plotfile
    labelspw: draw spw numeric labels adjacent to each point
    referenceFrame: the frequency reference frame, if not 'TOPO'
    """
    if (help):
        print "Usage: spectralIndexMonteCarlo(filename='',yfilename='',source='',verbose=False,"
        print "       maxpoints=0,trials=2000,spw='',help=False,plotdir='',labelspw=False,"
        print "       referenceFrame='TOPO')"
        print "  filename: contains a listobs output file"
        print "  yfilename: contains a fluxscale output file"
        print "  source: sourcename to choose from the (possibly) multi-source fluxscale file"
        print "  maxpoints: the maximum number of spws to select for the fit (0=no max)"
        print "  trials: number of Monte-Carlo fits to run to estimate the fit uncertainties"
        print "  spw: the spws to use, e.g. ''=all, '1~3,5,6~8'=[1,2,3,5,6,7,8]"
        print "  plotdir: the directory in which to write the plotfile"
        print "  labelspw: draw spw numeric labels adjacent to each point"
        print "  referenceFrame: the frequency reference frame, if not 'TOPO'"
        return
    (p,covar,x,y,yerror,p2,source,freqUnits,pmedian,perror,spwsKept,meanOfLogX) = \
        self.spectralIndexFitterMonteCarlo(filename,yfilename,degree=1,source=source,
                                           verbose=verbose,maxpoints=maxpoints,
                                           trials=trials,spw=spw,
                                           referenceFrame=referenceFrame)
    if (plotdir == ''):
        plotdir = './'
    if (plotdir[-1] != '/'):
        plotdir += '/'
    if (p == []):
        return(p)
    if (p2 != []):
        # Then we have two solutions, where the first one is the error-weighted fit.
        slope = p[0]
        print "Completed %d Monte-Carlo error trials." % (trials)
        slopeErr = perror[0]
        freq = 10**x[0]
        amp = 10**(p[1] + x[0]*slope)
        ampError = amp * perror[1]
        freqUnNormalized = 10**(x[0] + meanOfLogX)
        print "Error-weighted fit: Slope: %.3f+-%.3f  Flux D. @ %.3f%s: %.3f+-%.3f" % (slope, slopeErr, freqUnNormalized, freqUnits, amp, ampError)
        if (verbose):
            yfit = x*pmedian[0] + pmedian[1]
            print "Predicted values = ", yfit
    else:
        # We only have one solution, so put it into p2.
        p2 = p
    amp2 = 10.0**p2[1]
    amp2 *= freq**p2[0]
    print "   Un-weighted fit: Slope: %.3f         Flux D. @ %.3f%s: %.3f" % (p2[0], freqUnNormalized, freqUnits, amp2)
    rc('text',usetex=True)
    pb.clf()
    desc = pb.subplot(111)
    desc.xaxis.grid(which='major')
    desc.yaxis.grid(which='major')
    yfit = x*p[0]+p[1]  # the fit result with zero Monte-Carlo errors added to the data
    if (pmedian != []):
        minslope = x*(pmedian[0]-perror[0]) + pmedian[1]
        maxslope = x*(pmedian[0]+perror[0]) + pmedian[1]

        # Now apply the scaled error in the yoffset
        minslope2 = np.log10(10**minslope - ampError)
        minslope3 = np.log10(10**minslope + ampError)
        maxslope2 = np.log10(10**maxslope - ampError)
        maxslope3 = np.log10(10**maxslope + ampError)
        minvalue = np.minimum(np.minimum(minslope2,minslope3),np.minimum(maxslope2,maxslope3))
        maxvalue = np.maximum(np.maximum(minslope2,minslope3),np.maximum(maxslope2,maxslope3))

        x += meanOfLogX
        pb.plot(x,y,'ko',x,yfit,'r-', x,minvalue,'r--',x,maxvalue,'r--')
        pb.title(source+'  spectral index=%+.3f$\pm$%.3f, F(%.2f%s)=%.3f$\pm$%.3f Jy'%(slope,slopeErr,freqUnNormalized,freqUnits,amp,ampError),size=14)
    else:
        pb.plot(x,y,'ko', x,yfit,'r-')
        pb.title(source+' index=%.3f, F(%.2f%s)=%.3f' % (slope,freqUnNormalized,freqUnits,amp))
    if (labelspw):
        for i in range(len(x)):
            pb.text(x[i],y[i],'%d'%(spwsKept[i]),size=10)
    originalYerror = yerror*(10**y)
    lower = y-np.log10(10**y-originalYerror)
    upper = np.log10(10**y+originalYerror)-y
    pb.errorbar(x,y,yerr=[lower,upper],color='k',ls='None')
    if (freqUnits != ''):
        pb.xlabel('Log$_{10}$(Frequency(%s))'%freqUnits)
    else:
        pb.xlabel('Log$_{10}$(Frequency)')
    pb.ylabel('Log$_{10}$(FluxDensity(Jy))')
    pb.axis('equal')
    ytext = self.replaceUnderscores(yfilename)
    pb.text(0.01, 0.01, 'data from %s'%(ytext), transform=desc.transAxes,size=10)
    pngname = '%s%s.%s.png' % (plotdir,yfilename,source)
    pb.savefig(pngname)
    print "Plot saved in %s" % (pngname)
    pb.draw()
    return
# end of spectralIndexMonteCarlo

  def replaceUnderscores(self,y):
    """
    Replaced underscores with \_ to avoid strange effects when plotting text with pylab.text()
    """
    newy = ''
    for i in range(len(y)):
        if (y[i]=='_'):
            newy += '\_'
        else:
            newy += y[i]
    return(newy)
    
# end of definition of class     linfit

def uvplot(msfile, field='', plotrange=[0,0,0,0], figfile=False, markersize=2, 
           density=144):
  """
  A simple function to make u vs. v plot with points mirrored about the origin, similar to
  AIPS.  This functionality is not yet in casa's plotms, but it *is* in plotuv.
  For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/Uvplt
  Todd Hunter
  """
  fieldid = field
  try:
      tb.open(msfile)
      tb.close()
  except:
      print "Could not open ms = " % msfile
      return
  if (type(fieldid) == str):
      if (fieldid == ''):
          print "Showing points for all fields"
          tb1.open(msfile)
      elif (len(fieldid) == sum([m in [str(m) for m in range(10)] for m in fieldid])):
          fieldid = int(fieldid)
          vm = ValueMapping(msfile)
          fieldname = vm.getFieldNamesForFieldId(fieldid)
          if (fieldname == None):
              print "No such field in the ms."
              return
          print "Showing field id = %d = %s" % (fieldid, fieldname)
      else:
          # convert from name to ID
          fieldname = fieldid
          vm = ValueMapping(msfile)
          fieldid = vm.getFieldIdsForFieldName(fieldname)
          tb1 = tb.query('FIELD_ID == '+str(fieldid))
          print "Showing field id = %d = %s" % (fieldid, fieldname)
  elif (type(fieldid) == int):
      tb1 = tb.query('FIELD_ID == '+str(fieldid))
      vm = ValueMapping(msfile)
      fieldname = vm.getFieldNamesForFieldId(fieldid)
      if (fieldname == None):
          print "No such field in the ms."
          return
      print "Showing field id = %d = %s" % (fieldid, fieldname)
  else:
      print "unsupported field id type"
      return
          
  rawdata = tb1.getcol("UVW")
  if (len(rawdata) < 1):
      print "No data found for this field"
      return
  tb.close()
  pb.clf()
  adesc = pb.subplot(111)
  maxbase = max([max(rawdata[0,]),max(rawdata[1,])])  # in m
  pb.plot(rawdata[0,],rawdata[1,],'b.',markersize=markersize)
  pb.plot(-rawdata[0,],-rawdata[1,],'b.',markersize=markersize)
  if (plotrange[0] != 0 or plotrange[1] != 0):
      pb.xlim([plotrange[0],plotrange[1]])
  if (plotrange[2] != 0 or plotrange[3] != 0):
      pb.ylim([plotrange[2],plotrange[3]])
  pb.gca().set_aspect(aspect='equal')
  pb.xlabel('u (meter)',fontsize='medium')
  pb.ylabel('v (meter)',fontsize='medium')
  if (fieldid==''):
      pb.title(msfile + '  all fields')
  else:
      pb.title(msfile + '  field %d = %s' % (fieldid,fieldname),fontsize=12)
  if (1==1):
        yFormatter = matplotlib.ticker.ScalarFormatter(useOffset=False)
        adesc.yaxis.set_major_formatter(yFormatter)
        adesc.yaxis.grid(True,which='major')
        xFormatter = matplotlib.ticker.ScalarFormatter(useOffset=False)
        adesc.xaxis.set_major_formatter(xFormatter)
        adesc.xaxis.grid(True,which='major')
  if (figfile == True):
      if (msfile.find('/')==len(msfile)-1):
          msfile = msfile[0:len(msfile)-1]
      myfigfile = msfile + '.uvplot.png'
      pb.savefig(myfigfile,format='png',density=density)
  elif (figfile != False):
      myfigfile = figfile
      pb.savefig(myfigfile,format='png',density=density)
  else:
      print "To generate a .png file, re-run with: figfile=T or figfile='my.png'"
      return
  pb.draw()
  print "Plot left in = %s" % (myfigfile)

def editIntents (msName='', field='', newintents='', help=False):
    """
    Change the observation intents for a specified field.  Adapted from
    John Lockman's interactive function for the ALMA pipeline.
    For further help and examples, run editIntents(help=True) or
    see http://casaguides.nrao.edu/index.php?title=EditIntents
    - T. Hunter
    """
    validIntents = ['AMPLITUDE','ATMOSPHERE','BANDPASS','DELAY','PHASE','SIDEBAND_RATIO','TARGET','WVR']
    if (help == True):
        print "Usage: editIntents(ms, field, newintents)"
        print "valid intents = ", validIntents
        print "Enter as list like above, or as single comma-delimited string"
        return

    tb.open(msName + '/FIELD')
    fieldnames = tb.getcol('NAME')
    tb.close()

    tb.open(msName + '/STATE')
    intentcol = tb.getcol('OBS_MODE')
    intentcol = intentcol
    tb.close()

    tb.open(msName, nomodify=False)
    naddedrows = 0
    if (type(newintents)==list):
        desiredintents = ''
        for n in newintents:
            desiredintents += n
            if (n != newintents[-1]):
                desiredintents += ','
    else:
        desiredintents = newintents
    desiredintentsList = desiredintents.split(',')
        
    for intent in desiredintentsList:
      if ((intent in validIntents)==False):
        print "Invalid intent = ", intent
        return

    for id,name in enumerate(fieldnames):
      if (name == field or id == field):
        print 'FIELD_ID %s has name %s' % (id, name)
 
        s = tb.query('FIELD_ID==%s' % id)
        state_ids = s.getcol('STATE_ID')

        states = []
        for state_id in state_ids:
            if state_id not in states:
                states.append(state_id)

        print 'current intents are:'
        for state in states:
            print intentcol[state]

        states = []
        for desiredintent in desiredintentsList:
	    if desiredintent == 'TARGET':
	        states.append('OBSERVE_TARGET#ON_SOURCE')
	    elif desiredintent == 'BANDPASS':
	        states.append('CALIBRATE_BANDPASS#ON_SOURCE')
	    elif desiredintent == 'PHASE':
	        states.append('CALIBRATE_PHASE#ON_SOURCE')
	    elif desiredintent == 'AMPLITUDE':
	        states.append('CALIBRATE_AMPLI#ON_SOURCE')
	    elif desiredintent == 'ATMOSPHERE':
	        states.append('CALIBRATE_ATMOSPHERE#ON_SOURCE')
	    elif desiredintent == 'WVR':
	        states.append('CALIBRATE_WVR#ON_SOURCE')
	    elif desiredintent == 'SIDEBAND_RATIO':
	        states.append('CALIBRATE_SIDEBAND_RATIO#ON_SOURCE')
	    elif desiredintent == 'DELAY':
	        states.append('CALIBRATE_DELAY#ON_SOURCE')
            else:
                print "Unrecognized intent = %s" % desiredintent
                continue
            print 'setting %s' % desiredintent
            


        if states != []:
            state = reduce(lambda x,y: '%s,%s' % (x,y), states)
            if state not in intentcol:
                print 'adding intent to state table'
                intentcol = list(intentcol)
                intentcol.append(state)
                intentcol = np.array(intentcol)
                state_id = len(intentcol) - 1
                naddedrows += 1
                print 'state_id is', state_id
   
                state_ids[:] = state_id
            else:
                print 'intent already in state table'
                state_id = np.arange(len(intentcol))[intentcol==state]

                print 'state_id is', state_id
                state_ids[:] = state_id

            s.putcol('STATE_ID', state_ids)

    print 'writing new STATE table'
    tb.open(msName + '/STATE', nomodify=False)
    print tb.nrows()
    if naddedrows > 0:
        tb.addrows(naddedrows)
    print tb.nrows()
    print len(intentcol)
    tb.putcol('OBS_MODE', intentcol)
    tb.close()

class getNearestFlux:
    """
    Class that can be used in CASA to quickly look up the fluxes of a quasar at a certain date.
    After initializing analysisUtils you can call it
    
    gn = au.getNearestFlux()
    gn.searchFlux('NAME','DATA', freq=YYY)
    NAME - string. E.g. '3c454.3' Name of the quasar. Note that a precise name must be given.
    DATE - string. E.g. '20120101' for Jan 1st 2012. Must be a string of the form YearMonthDay
    frequency - Optional float. Give the frequency in GHz. (e.g. 345 for 345 GHz) you want to give the flux. It will identify the band.
    For high frequencies everything upwards from 300 GHz will be given (e.g. if frequency = 690, all measurements between 300 and 1000 GHz will be displayed.
    If not frequency is given, the frequency is set at 100 GHz (Band 3). 
    Catalogues included: ALMA, SMA, VLA, ATCA, OVRO.

    Added March 19 (tkempen)
    """
    def __init__(self, name=None, tunnel=False) :
        if (tunnel):
            # This option was added by T. Hunter for access from outside JAO.
            self.s = ServerProxy('http://localhost:8080/sourcecat/xmlrpc')
        else:
            self.s = ServerProxy('http://aurora.osf.alma.cl:8080/sourcecat/xmlrpc')
        try:
            self.catalogList=self.s.sourcecat.listCatalogues()
        except:
            print "Connection failed."
            if (tunnel):
                print "Before importing this class, be sure to set up an ssh tunnel like so:"
                print "    ssh -L 8080:aurora.osf.alma.cl:8080 %s@tatio.aiv.alma.cl" % os.getenv("USER")
                print "(or ssh -L 8080:aurora.osf.alma.cl:8080 %s@login.alma.cl)" % os.getenv("USER")
                print "Alternatively, to access the webpage manually, open a SOCKS v5 proxy tunnel by:"
                print "(1) Use FoxyProxy to set pattern http://aurora.osf.alma.cl:8080/* to localhost port 8080"
                print "(2) ssh -D 8080 tatio.aiv.alma.cl"
                print "(3) surf to http://aurora.osf.alma.cl:8080/sourcecatweb/sourcecat/"
            else:
                print "If you are outside the JAO network, use au.getNearestFlux(tunnel=True)"
            return
        self.catalogues=[]
        for i in range(size(self.catalogList)):
            self.catalogues.append(self.catalogList[i]['catalogue_id'])
            
        self.typesList=self.s.sourcecat.listTypes()
        self.types=[]
        for i in range(size(self.typesList)):
            self.types.append(self.typesList[i]['type_id'])

            
    def wrapSearch(self,limit = 10,catalogues = None,types = None,name = '',ra = -1.0,
                   dec = -1.0, radius = -1.0,ranges = [],fLower = 0.0, fUpper = -1.0,
                   fluxMin = -1.0, fluxMax = -1.0, degreeMin = -1.0, degreeMax = -1.0,
                   angleMin = -361.0, angleMax = -361.0, sortBy = 'source_id', asc = True,
                   searchOnDate=False, dateCriteria=0, date=''):
        """
        This is the basic search. It is a wrapper around the  catalog's
        searching function
        """
        
        if catalogues == None:
            catalogues=self.catalogues
        elif isinstance(catalogues,list) == False :
            print "Catalgues must be a list of integers ([1,2,3]. Try again. Available catalogues are:"
            print self.catalogList
            sys.exit()
        
        if types == None:
            types=self.types
        elif isinstance(catalogues,list) == False :
            print "Types must be a list of integers ([1,2,3]. Try again. Available types are:"
            print self.typesList
            sys.exit()     
  
        self.measurements = self.s.sourcecat.searchMeasurements(limit,catalogues,types,name,ra, dec,
                                                                radius,ranges,fLower, fUpper, fluxMin,
                                                                fluxMax, degreeMin,degreeMax, angleMin,
                                                                angleMax ,sortBy ,asc, searchOnDate,
                                                                dateCriteria, date)
        return self.measurements

    def getBandfromfreq(self,frequency):
        #print "Assigning Frequency to:"
        if frequency < 31.3:
            print "Not an ALMA Band. Please give a different frequency (in GHz units)"
            sys.exit()
        if frequency > 1000:
            print "Not an ALMA Band. Please give a different frequency (in GHz units)"
            sys.exit()

        if frequency > 31.2 and frequency < 56:
#            print " ALMA Band 1"
            self.flower = 31.3e9
            self.fupper = 45.0e9
        if frequency > 55.9999 and frequency < 84:
#            print " ALMA Band 2"
            self.flower = 67e9
            self.fupper = 84e9
        if frequency > 84 and frequency < 120:
#            print " ALMA Band 3"
            self.flower = 84e9
            self.fupper = 108e9 #is not 116 to prevent the double frequencies at 109 and 98 to be given
        if frequency > 120 and frequency < 163:
#            print " ALMA Band 4"
            self.flower = 125e9
            self.fupper = 163e9
        if frequency > 163 and frequency < 211:
#            print " ALMA Band 5"
            self.flower = 163e9
            self.fupper = 211e9
        if frequency > 211 and frequency < 275:
#            print "ALMA Band 6"
            self.flower = 211e9
            self.fupper = 275e9
        if frequency > 275 and frequency < 377:
#            print "ALMA Band 7"
            self.flower = 275e9
            self.fupper = 370e9
        if frequency > 377 and frequency < 1000:
            print "ALMA Bands 8,9 or 10 requested. Giving all measurements of Band 7 and higher"
            self.flower = 300e9
            self.fupper = 1.0e12

        return self
    def getSourceIdFromSourceName(self,name) :
        """
        Searches for all sources giveen the name
        Returns sourceId
        Note that this search only finds sources which have a valid measurement
        
        """
        measurements=self.wrapSearch(name=name,catalogues=[1,5,4,21,3,2])
        ids=[]
        for i in range(size(measurements)):
            ids.append(measurements[i]['source_id'])

        sourceId=unique(ids)
        if size(sourceId) == 0:
            print "Sorry, could not find any sources with name %s.Try to add wildcard %% before or after source name" %name
            return
        else:
            return int(sourceId[0])

    def strDate2MJD(self,d):
        """
        converts date in string format 20110809 to MJD
        """
        date='%s-%s-%s'%(d[0:4],d[4:6],d[6:])
        mjd=me.epoch('utc',date)['m0']['value']         

        return mjd

    def getJulianDates(self,measurements):
        julian =[]
        for m in measurements:
            date = ('%s' % m['date_observed']).split('T')[0]
            mjdate = self.strDate2MJD(date)
            julian.append(mjdate)
            
        return julian

    def searchFlux(self,sourcename, date, frequency = None):
        """
        date format: 'YYYYMMDD'
        frequency in GHz (default is 100)
        """
        sourceID = self.getSourceIdFromSourceName(sourcename)
        if sourceID == None:
            #print "Source Name not found in Catalog. Please check spelling. If you are sure, please contact Tim (tkempen@alma.cl) for info"
            return
#            sys.exit()

        # check Frequencies and generate fupper
        if frequency == None:
            print " No Frequency given, returning nearest Band 3 flux"
            frequency = 100
        self.getBandfromfreq(frequency)
 

        #convert date to MJD
        mjd = self.strDate2MJD(date)

        # get all measurements in the band for that source
        measurements = self.wrapSearch(limit = 500, fLower = self.flower, name = sourcename, fUpper = self.fupper)

        size_m = len(measurements)
        if size_m == 0:
            print 'No measurements found at this frequency'
            sys.exit()
            
#        print size_m
        print 'Source : ', sourcename, ' Requested Frequency : ',frequency,' GHz , Date Requested : ',date
        print ' Rank  | Flux      | Date      | Meas. Freq'
        if size_m >= 4 :
            #print 'Do 4!'
            for i in range(4):
                julian_dates = self.getJulianDates(measurements)
                julian_min = [abs(x-mjd) for x in julian_dates]
                index = julian_min.index(min(julian_min))
                m=measurements[index]
                print '   ',i+1,' | ', m['flux'], 'Jy |', ('%s' % m['date_observed']).split('T')[0],' | ', m['frequency']/1e9, ' GHz'
                measurements.remove(m)
        else:
            for i in range(size_m):
                julian_dates = self.getJulianDates(measurements)
                julian_min = [abs(x-mjd) for x in julian_dates]
                index = julian_min.index(min(julian_min))
                m=measurements[index]
                print '   ',i+1,' | ', m['flux'], 'Jy |', ('%s' % m['date_observed']).split('T')[0],' | ', m['frequency']/1e9, ' GHz'
                measurements.remove(m)
            print 'Only ', str(size_m),' measurements found in total'


def primaryBeamArcsec(vis='', spw='', frequency='',wavelength='', diameter=12.0, verbose=False, help=False):
  """
  Implements the formula: 1.18 * lambda / D.  Specify one of the following combinations:
    1) vis and spw (uses median dish diameter)
    2) frequency in GHz (assumes 12m)"
    3) wavelength in mm (assumes 12m)"
    4) frequency in GHz and diameter (m)
    5) wavelength in mm and diameter (m)
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/PrimaryBeamArcsec
    -- Todd Hunter
    """
  
  if (help == True):
      print "Usage: primaryBeamArcsec(vis='', spw='', frequency='',wavelength='', diameter=12.0, verbose=False, help=False)"
      print "Implements the formula: 1.18 * lambda / D.  Specify one of the following combinations:"
      print "     1) vis and spw (uses median dish diameter)"
      print "     2) frequency in GHz (assumes 12m)"
      print "     3) wavelength in mm (assumes 12m)"
      print "     4) frequency in GHz and diameter"
      print "     5) wavelength in mm and diameter"
      return (0)
  if (vis!='' and spw!=''):
      try:
          antennaTable = vis+'/ANTENNA'
          tb.open(antennaTable)
      except:
          print "Could not open table = %s" % antennaTable 
          return(0)
      diameter = np.median(np.unique(tb.getcol('DISH_DIAMETER')))
      tb.close()
      try:
          spwTable = vis+'/SPECTRAL_WINDOW'
          tb.open(spwTable)
          num_chan = tb.getcol('NUM_CHAN')
          refFreqs = tb.getcol('REF_FREQUENCY')
          tb.close()
      except:
          print "Could not open table = %s" % antennaTable 
          return(0)
      frequencyHz = refFreqs[spw]
      frequency = frequencyHz*1e-9
      if (verbose):
          print "Found frequency = %f GHz and dish diameter = %.1fm" % (frequency,diameter)
  else:
      if (frequency != '' and wavelength != ''):
          print "You must specify either frequency or wavelength, not both!"
          return(0)
  if (frequency != ''):
      lambdaMeters = c_mks/(frequency*1e9)
  elif (wavelength != ''):
      lambdaMeters = wavelength*0.001
  else:
      print "You must specify either frequency (in GHz) or wavelength (in mm)"
      return(0)
#  print "wavelength = %.3f mm" % (lambdaMeters*1000)
  return(1.18*lambdaMeters*3600*180/diameter/math.pi)

def TimeOnSource(ms='',field='',includeLatency=False,verbose=True,asdm='',help=False,vm=''):
    """
    Uses ValueMapping to get the integration timestamps and computes a
    list of durations on the fields specified, attempting to detect and
    account for the inter-subscan latency.  Returns a dictionary indexed
    by the source ID integer:
    {0: {'num_of_subscans': 2,
         'scans': [5],
         'field_id': [0],
         'source_name': '3c279',
         'minutes_on_source': 58.716000556945801,
         'minutes_on_source_with_latency': 65.664000511169434
         },
     ...
     'clock_time': 120
     'minutes_on_science': 58.716
     'minutes_on_science_with_latency': 65.66
     'percentage_time_on_science': 20.40
    }

It also prints a string convenient to a wikitable of format:
| date | SB name | exec UID | UT start-end | LST start-end | Total time | Time on source | el range | med pwv | antennas | 

Notes on this string:
1) If the pointing table has been deleted it will print "pointing table empty" in the elevation range column.
2) If there is no on-source time, it will print "no onsource time" in the elevation range column.
3) If neither the CalWVR.xml nor ASDM_CALWVR files are present, it will print "unknown" in the pwv column.
4) The median pwv is over the entire dataset, not just the on-source scans.
5) For the number of antennas, it does not detect whether an antenna has been totally flagged!

    For further help and examples, see http://casaguides.nrao.edu/index.php?title=TimeOnSource
    -- Todd Hunter
    """
    return(timeOnSource(ms,field,includeLatency,verbose,asdm,help,vm))

def computeClockTimeOfMS(vm):
    """
    Computes delta time from first to last scan in a ValueMapping structure.
    This assumes that the times and scans are in ascending order.
    Not presently used by any other functions.
    - Todd Hunter
    """
    times = vm.getTimesForScans(np.unique(vm.scans))
    mjdsecmin = 1e12
    mjdsecmax = 0
    for t in times:
#  This is too slow:
#        mjdsecmin = np.amin([np.amin(t),mjdsecmin])
#        mjdsecmax = np.amax([np.amax(t),mjdsecmax])
#  Assume the times are in ascending order:
        mjdsecmin = np.amin([t[0],mjdsecmin])
        mjdsecmax = np.amax([t[-1],mjdsecmax])
    [mjdmin,utmin] = mjdSecondsToMJDandUT(mjdsecmin)
    [mjdmax,utmax] = mjdSecondsToMJDandUT(mjdsecmax)
    clockTimeMinutes = (mjdmax-mjdmin)*1440.
    return(clockTimeMinutes)

def timeOnSource(ms='',field='',includeLatency=False,verbose=True,asdm='',help=False,vm=''):
    """
    Uses ValueMapping to get the integration timestamps and computes a
    list of durations on the fields specified, attempting to detect and
    account for the inter-subscan latency.  Returns a dictionary indexed
    by the source ID integer:
    {0: {'num_of_subscans': 2,
         'scans': [5],
         'field_id': [0],
         'source_name': '3c279',
         'minutes_on_source': 58.716000556945801,
         'minutes_on_source_with_latency': 65.664000511169434
         },
     ...
     'clock_time': 120
     'minutes_on_science': 58.716
     'minutes_on_science_with_latency': 65.66
     'percentage_time_on_science': 20.40
    }

It also prints a string convenient to a wikitable of format:
| date | SB name | exec UID | UT start-end | LST start-end | Total time | Time on source | el range | med pwv | antennas | 

Notes on this string:
1) If the pointing table has been deleted it will print "pointing table empty" in the elevation range column.
2) If there is no on-source time, it will print "no onsource time" in the elevation range column.
3) If neither the CalWVR.xml nor ASDM_CALWVR files are present, it will print "unknown" in the pwv column.
4) The median pwv is over the entire dataset, not just the on-source scans.
5) For the number of antennas, it does not detect whether an antenna has been totally flagged!

    For further help and examples, see http://casaguides.nrao.edu/index.php?title=TimeOnSource
    -- Todd Hunter
    """
    if (help or ms==''):
        print "Usage: timeOnSource(ms,field='',verbose=True,help=False,vm='')"
        print "  Returns a list of durations (in sec) on the fields specified, attempting"
        print "  to detect and account for inter-subscan latency, unless includeLatency=T"
        print "  in which case it will return the total time from start to end of scan."
        print "  Fields can be specified by a single name or ID, or a list of IDs, e.g. '2,3,4'"
        return
    if (os.path.exists(ms) == False):
        print "Could not open ms = %s" % (ms)
        return({})
    if (vm==''):
        if (verbose):
            print "Running ValueMapping... (this may take a minute)"
        vm = ValueMapping(ms)
    tb.open(ms+'/FIELD')
    sourceIDs = tb.getcol('SOURCE_ID')
#    print "sourceIDs: ", sourceIDs
    tb.close()
    telescopeName = getObservatoryName(ms)
    mydict = {}
    if (field == ''):
        field = range(len(sourceIDs))
    elif (type(field) == int):
        field = [field]
    elif (type(field) == str):
        if (field.find(',') >= 0):
            field = field.split(',')
            fd = []
            for f in field:
                fd.append(int(f))
            field = fd
        else:
            try:
                field = int(field)
                field = [field]
            except:
                field = vm.getFieldIdsForFieldName(field)
                if (len(field) < 1):
                    print "No match for field name = " % (field)
                    return({})
    elif (type(field) == list):
        if (type(field[0]) == str):
            nf = []
            for f in field:
                nf.append(vm.getFieldIdsForFieldName(f))
            field = nf
    if (verbose):
        print "Non-CalAtmosphere, Non-CalPointing, Non-CalSideband scans:"
        print "Total time on scans (including inter-subscan latency):"
    durations = []
    subscansAll = []
    fieldId = -1
    previousField = -1
    legend = "Field "
    multiField = False
    totalMinutes = 0
    scienceMinutesWithLatency = 0
    mydict = {}
#    print "Fields to show: ", field
    legend = ""
    multiFieldScansObserved = []
    for findex in range(len(field)):
        f = field[findex]
        durationWithLatency = 0
        scienceDurationWithLatency = 0
        scans = vm.getScansForField(f)
#        print scans, "= scans on field ", f
        scansObserved = []
        totalSubscans = 0
        multiField = False
        if (findex < len(field)-1):
          if (sourceIDs[f] == sourceIDs[field[findex+1]]): # and scans[0] == vm.getScansForField(field[findex+1])[0]):
              multiField = True
              legend += '%2d,' % (f)
#              print multiFieldScansObserved, scans, len(scans)
              for s in scans:
                  multiFieldScansObserved.append(s)
              continue
          else:
              legend += '%2d' % (f)
        else:
          if (sourceIDs[f] == sourceIDs[field[findex-1]]): #  and scans[0] == vm.getScansForField(field[findex-1])[0]):
              multiField = True
              for s in scans:
                  multiFieldScansObserved.append(s)
              legend += '%2d' % (f)
          else:
              legend += '%2d' % (f)
        subscansPerScan = []
        if (multiField):
            scans = np.unique(multiFieldScansObserved)
        for s in scans:
            intents = vm.getIntentsForScan(s)
            skip = False
            for i in intents:
                if (i.find('CALIBRATE_ATMOSPHERE') >= 0 or i.find('CALIBRATE_POINTING') >= 0 or i.find('CALIBRATE_SIDEBAND') >= 0):
                    skip = True
                    
            if (skip):
#                print "Skipping scan %d" % (s)
                continue
            scansObserved.append(s)
            times = vm.getTimesForScans(s)
            for t in times:
                (d,subscans) = computeDurationOfScan(t,s,ms)
                durationWithLatency += np.max(t) - np.min(t)
                totalSubscans += subscans
                subscansPerScan.append(subscans)
                if ('OBSERVE_TARGET#ON_SOURCE' in intents):
                    scienceDurationWithLatency += np.max(t) - np.min(t)
        totalMinutes += durationWithLatency/60.
        scienceMinutesWithLatency += scienceDurationWithLatency/60.
        if (verbose):
            if (totalSubscans > 1 and multiField==False):
                print "Source %2d = Field %s = %s: %.1f sec = %.1f min (%d scan%s: %s, %d subscans)" % (sourceIDs[f],
                   legend, vm.getFieldNamesForFieldId(f),
                   durationWithLatency, durationWithLatency/60.,
                   len(scansObserved), 's' if len(scansObserved)>1 else '', scansObserved, totalSubscans)
            else:
                print "Source %2d = Field %s = %s: %.1f sec = %.1f min (%d scan%s: %s)" % (sourceIDs[f],
                   legend, vm.getFieldNamesForFieldId(f),
                   durationWithLatency, durationWithLatency/60.,
                   len(scansObserved), 's' if len(scansObserved)>1 else '', scansObserved)
#        if (includeLatency == True):
#            durations.append(durationWithLatency)
#            subscansAll.append(totalSubscans)
        if (len(legend.split(',')) > 1):
            myfield = [int(x) for x in legend.split(',')]
        else:
            myfield = [f]
        mydict[sourceIDs[f]] = {'field_ids': myfield,
                     'source_name':vm.getFieldNamesForFieldId(f),
                     'scans': scansObserved,
                     'num_of_scans': len(scansObserved),
                     'num_of_fields': len(legend.split(',')),
                     'num_of_subscans': subscansPerScan,
                     'minutes_on_source_with_latency': durationWithLatency/60.,
                     'minutes_on_source': 0  # fill this in later
                     }
        legend = ""
    fullreport = True
    if (fullreport):
        [wikiline2,wikiline3,clockTimeMinutes,csvline] = lstrange(ms,verbose=False,vm=vm)
    else:
        clockTimeMinutes = computeClockTimeOfMS(vm)
    print "Clock time = %.2f min, Total time = %.2f min,  science time = %.2f min" % (clockTimeMinutes, totalMinutes, scienceMinutesWithLatency)
    csvline += ',%.2f,%.2f' % (totalMinutes, scienceMinutesWithLatency)
  
    if (verbose):
        print "\nMy attempt to detect and account for inter-subscan latency:"

    legend = ""
    multiField = False
    totalMinutes = 0
    scienceMinutes = 0
    fid = 0
    multiFieldScansObserved = []
    for findex in range(len(field)):
        f = field[findex]
        # f is the field ID
        duration = 0
        scienceDuration = 0
        scans = vm.getScansForField(f)
        scansObserved = []
        totalSubscans = 0
        multiField = False
        if (findex < len(field)-1):
          if (sourceIDs[f] == sourceIDs[field[findex+1]]): #  and scans[0] == vm.getScansForField(field[findex+1])[0]):
            multiField = True
            legend += '%2d,' % (f)
            for s in scans:
                multiFieldScansObserved.append(s)
            continue
          else:
            legend += '%2d' % (f)
        else:
          if (sourceIDs[f] == sourceIDs[field[findex-1]]): #  and scans[0] == vm.getScansForField(field[findex-1])[0]):
            multiField = True
            legend += '%2d' % (f)
            for s in scans:
                multiFieldScansObserved.append(s)
          else:
            legend += '%2d' % (f)
        
        if (multiField):
            scans = np.unique(multiFieldScansObserved)
        for s in scans:
            intents = vm.getIntentsForScan(s)
            skip = False
            for i in intents:
                if (i.find('CALIBRATE_ATMOSPHERE') >= 0 or i.find('CALIBRATE_POINTING') >= 0 or
                    i.find('CALIBRATE_SIDEBAND') >= 0):
                    skip = True
                
            if (skip):
#                print "Skipping scan %d" % (s)
                continue
            scansObserved.append(s)
            times = vm.getTimesForScans(s)
            for t in times:
                (d,subscans) = computeDurationOfScan(t,s,ms)
                duration += d
                totalSubscans += subscans
                if ('OBSERVE_TARGET#ON_SOURCE' in intents):
                    scienceDuration += d
        totalMinutes += duration/60.
        scienceMinutes += scienceDuration/60.
        if (verbose):
            fieldWarning = ""
            if (totalSubscans > 1 and multiField==False):
                print "Source %2d = Field %s = %s: %.1f sec = %.1f min (%d scan%s: %s, %d subscans%s)" % (sourceIDs[f],
                 legend, vm.getFieldNamesForFieldId(f), duration, duration/60.,
                 len(scansObserved), 's' if len(scansObserved)>1 else '', scansObserved, totalSubscans, fieldWarning)
            else:
                print "Source %2d = Field %s = %s: %.1f sec = %.1f min (%d scan%s: %s)" % (sourceIDs[f],
                    legend, vm.getFieldNamesForFieldId(f), duration, duration/60.,
                    len(scansObserved), 's' if len(scansObserved)>1 else '', scansObserved)
        legend = ""
        mydict[sourceIDs[f]]['minutes_on_source'] = duration/60.
        mydict['minutes_on_science'] = scienceMinutes
        mydict['minutes_on_science_with_latency'] = scienceMinutesWithLatency
        mydict['percentage_time_on_science'] = 100*scienceMinutes/clockTimeMinutes
        mydict['clock_time'] = clockTimeMinutes
        mydict['source_ids'] = list(sourceIDs)
        mydict['num_of_sources'] = len(sourceIDs)
    if (fullreport):
        wikiline2 += '%.1f | %.1f | ' % (totalMinutes, scienceMinutes)
        wikiline2 += wikiline3
    # Now get the PWV if possible
    pwvmean = -1
    if (telescopeName.find('ALMA')>=0):
      if (os.path.exists('ASDM_CALWVR') or os.path.exists(ms+'/ASDM_CALWVR')):
        if (os.path.exists(ms+'/ASDM_CALWVR')):
            [pwvmean, pwvstd]  = getMedianPWV(ms)
        else:
            [pwvmean, pwvstd]  = getMedianPWV('.')
      elif (os.path.exists(ms+'/CalWVR.xml')):
        [pwvtime, pwv, antenna] = readpwv(ms)
        pwvmean = np.mean(pwv)
      elif (os.path.exists('CalWVR.xml')):
        [pwvtime, pwv, antenna] = readpwv('.')
        pwvmean = np.mean(pwv)
      else:
        print "No ASDM_CALWVR or CalWVR.xml table found.  You should importasdm with asis='*' or copy the CalWVR.xml file from your ASDM to your working directory (or your ms directory)."
    tb.open(ms+'/ANTENNA')
    nAntennas = len(tb.getcol('NAME'))
    tb.close()
    if (fullreport): 
        if (pwvmean < 0):
            wikiline2 += ' unknown_PWV | %d ' % (nAntennas)
        else:
            wikiline2 += ' %.2f | %d |' % (pwvmean, nAntennas)
        wikiline2 += '   |'   # Requested by Andreas Lundgren on 2012-05-23  
    print "Latency removed: Total time = %.1f min,   science time = %.1f min" % (totalMinutes, scienceMinutes)
    csvline += ',%.1f,%.1f' % (totalMinutes, scienceMinutes)
    if (fullreport): 
        print "wikiline = %s" % (wikiline2)
        print csvline
    print "WARNING: This task does not account for any flagging."
    return(mydict) 

def computeDurationOfScan(t,scanNumber,ms):
    """
    This function is used by timeOnSource().
    t = a sequence of integration timestamps
    scanNumber = the scan number, simply for generating a file list of timestamps
    -- Todd Hunter
    """
    t = np.unique(t)
    d = np.max(t) - np.min(t)
    # initial estimate for interval
    diffs = []
    for i in range(1,len(t)):
        diffs.append(t[i]-t[i-1])
    avgInterval = np.median(diffs)
#    print "median interval = %.3f sec" % (avgInterval)
    startTime = previousTime = t[0]
    subscans = 1
    if (avgInterval > 1):
        gapFactor = 2
    else:
        gapFactor = 3.
    startTime = previousTime = t[0]
    duration = 0
    tdiffs = []
#    fid = open("%s.scan%d"%(ms.split('/')[-1],scanNumber), "w")
    s = ''
    for i in range(1,len(t)):
        s += "%.2f " % (t[i]-t[0])
        tdiff = t[i]-previousTime
#        fid.write("%.4f %.5f\n" % (t[i],tdiff))
        tdiffs.append(tdiff)
        if (tdiff > gapFactor*avgInterval):
#            print "Dropped %.1f seconds" % (t[i]-previousTime+avgInterval)
            duration += t[i-1] - startTime + avgInterval
            startTime = t[i]
#            print "Subscan %d: %s" % (subscans,s)
            s = ''
            subscans += 1
        previousTime = t[i]
#    fid.close()
    duration += t[len(t)-1] - startTime # + avgInterval
#    print "Median time difference = ", np.median(tdiff)
    return(duration, subscans)
#

def plotWVRSolutions(caltable='',spw=[],field=[],subplot=22,sort='number',antenna='',xrange=[0,0],
                     yrange=[0,0],figfile='',unwrap=False,help=False,xaxis='ut',interactive=True,ms=''):
    """
    Because running plotcal on WVR tables includes the common mode delay,
    it is not so useful, as all antennas often appear very similar.  In
    this function, we compute the phase difference for each baseline and
    plot this vs. time.  -  Todd Hunter
    Options:
    antenna: can be ID or name; single value, or list: 0,'0','0,1','DV04,DV05'
    field: must be field ID, or list of IDs, or comma-delimited string of IDs
    figfile: will append .0.png, .1.png, etc. for successive pages
    interactive: True or False (True will require user input after showing each page)
    ms: specify the ms name rather than look for it in the caltable
    sort: by baseline 'number',  or by baseline 'length'
    subplot:   valid values: 11,12,21,22,23,32,42  (any 3rd digit is ignored)
    unwrap: attempt to further unwrap the 360-deg phase wraps after
            Ed's algorithm
    xaxis: 'seconds' or 'ut' (default)
    xrange: min/max time to show:  specify in seconds if xaxis='seconds'
            or floating point UT hours if 'ut'.  The latter will 
            probably not work well if the data cross the 0/24 hour mark.
    yrange: min/max phase to show
    For further help and examples, see:
      https://safe.nrao.edu/wiki/bin/view/ALMA/PlotWVRSolutions
    --- Todd Hunter
    """
    if (help):
        print "Usage: plotWVRSolutions(caltable='', spw='', field='', subplot=22, sort='number', antenna='',"
        print "         xrange=[0,0], yrange=[0,0], figfile='', unwrap=False, help=False, xaxis='ut')"
        print "   antenna: can be id or name; single value, or list:  0,'!0','0,1','DV04,DV05'"
        print "   field: must be field ID, or list of IDs, or comma-delimited string of IDs"
        print "   interactive: True or False (True requires user input after showing each page)"
        print "   ms: specify the ms name associated with the table rather than look for it in the table"
        print "   sort: by baseline 'number',  or by unprojected baseline 'length' (shortest first)"
        print "   subplot:  valid values: 11,12,21,22,23,32,42  (any 3rd digit is ignored)"
        print "   unwrap: attempt to further unwrap the 360-deg phase wraps after Ed's algorithm"
        print "   xaxis: 'seconds' or 'ut' (default)"
        print "   xrange: min/max time to show:  specify in seconds if xaxis='seconds'"
        print "           or floating point UT hours if 'ut'.  The latter will not"
        print "           well if the data cross the 0/24 hour mark."
        print "   yrange: min/max phase to show (degrees)"
        return
    plotctr = 0
    myhspace = 0.25
    mywspace = 0.25

    if (antenna.find('&')>=0):
        print "The '&' character is not supported (yet)."
        return
    if (sort != '' and sort != 'length' and sort != 'number'):
        print "Invalid sort option.  Options are 'length' or 'number'."
        return
    if (xaxis != 'ut' and xaxis.find('sec')<0):
        print "Invalid xaxis option.  Options are 'ut' or 'seconds'."
        return

    validSubplots = [11,12,21,22,23,32,42]
    if (type(subplot) == str):
        subplot = int(subplot)
    if (subplot > 100):
        # allow 111, 221, etc. to be consistent with other casa commands
        subplot /= 10
    if ((subplot in validSubplots) == False):
        print "Invalid subplot option.  Options are: ", validSubplots
        print "With an optional trailing digit that will be ignored."
        return
    xframe = xframeStart = subplot*10
    xframeStop = xframeStart + ((subplot%10)*(subplot/10))
    # row column
    topRow = [111,121,122,211,221,222,231,232,233,321,322,421,422]
    bottomRow = [111,121,122,212,223,224,234,235,236,325,326,427,428]
    leftColumn = [111,121,211,212,221,223,231,234,321,323,325,421,423,425,427]
    subplotRows = subplot/10
    
    if (subplot == 11):
        mysrcsize = 10
        mysize = 10
        titlesize = 12
    else:
        mysrcsize = 8
        mysize = 8
        titlesize = 10
        
    if (os.path.exists(caltable)):
        try:
            print "Opening cal table..."
            tb.open(caltable)
        except:
            print "Could not open caltable = ", caltable
            return
    else:
        print "Caltable does not exist = ", caltable
        return
    times = tb.getcol("TIME")
    antennas = tb.getcol("ANTENNA1")
    uniqueAntennas = np.unique(antennas)
    fields = tb.getcol("FIELD_ID")
    uniqueFields = np.unique(fields)
    names = tb.colnames()
    if ('SNR' not in names):
        print "This does not appear to be a cal table. Is it the .ms?"
        return
    if ('SPECTRAL_WINDOW_ID' in names):
        msName = tb.getkeyword('MSName')      
        cal_desc_id = tb.getcol("SPECTRAL_WINDOW_ID")
        spectralWindowTable = tb.getkeyword('SPECTRAL_WINDOW').split()[1]
        ParType = tb.getkeyword('ParType')    # string = 'Complex'
        if (ParType != 'Complex'):
            print "This does not appear to be a gain calibration table because the data type is %s." % (ParType)
            return
        gain = tb.getcol('CPARAM')[0][0]  # Here we must be assuming single-pol corrections
        tb.close()
#        tb.open(spectralWindowTable)
    else:  # old-style cal table
        cal_desc_id = tb.getcol("CAL_DESC_ID")
        gain = tb.getcol("GAIN")[0][0]  # Here we must be assuming single-pol corrections
        tb.open(caltable+'/CAL_DESC')
        msName = tb.getcell('MS_NAME',0)
        tb.close()
    if (ms != ''):
        msName = ms
    print "Running ValueMapping on %s... (this may take a minute)" % (msName)
    if (os.path.exists(msName)):
        vm = ValueMapping(msName)
        msFound = True
    else:
        if (caltable.find('/')>=0):
            tokens = caltable.split('/')
            fname = ''
            for i in range(len(tokens)-1):
                fname += tokens[i] + '/'
            print "failed to open %s, will try %s" % (msName,fname+msName)
            if (os.path.exists(fname+msName)):
                vm = ValueMapping(fname+msName)
                msFound = True
                msName = fname+msName
            else:
                msFound = False
        else:
            msFound = False
    
    # Continue to parse the command line
    if (type(figfile) == str):
        if (figfile.find('/')>=0):
            directories = figfile.split('/')
            directory = ''
            for d in range(len(directories)-1):
                directory += directories[d] + '/'
            if (os.path.exists(directory)==False):
                print "Making directory = ", directory
                os.system("mkdir -p %s" % directory)

    # Parse the antenna string to emulate plotms
    myValidCharacterListWithBang = ['~', ',', ' ', '*', '!',] + [str(m) for m in range(10)]
    uniqueAntennaIds = uniqueAntennas
    if (type(antenna) == str):
      if (len(antenna) == sum([m in myValidCharacterListWithBang for m in antenna])):
         # a simple list of antenna numbers was given 
         tokens = antenna.split(',')
         antlist = []
         removeAntenna = []
         for token in tokens:
             if (len(token) > 0):
                 if (token.find('*')==0 and len(token)==1):
                     antlist = uniqueAntennaIds
                     break
                 elif (token.find('!')==0):
                     antlist = uniqueAntennaIds
                     removeAntenna.append(int(token[1:]))
                 elif (token.find('~')>0):
                     (start,finish) = token.split('~')
                     antlist +=  range(int(start),int(finish)+1)
                 else:
                     antlist.append(int(token))
         antlist = np.array(antlist)
         for rm in removeAntenna:
#             print "Removing antenna %d = %s" % (rm, vm.getAntennaNamesForAntennaId(rm))
             antlist = antlist[np.where(antlist != rm)[0]]
         antlist = list(antlist)
         if (len(antlist) < 1 and len(removeAntenna)>0):
             print "Too many negated antennas -- there are no antennas left to plot."
             return(vm)
      else:
         # The antenna name (or list of names) was specified
         tokens = antenna.split(',')
         if (msFound):
             antlist = []
             removeAntenna = []
             for token in tokens:
                 if (token in uniqueAntennas):
                     antlist = list(antlist)  # needed in case preceding antenna had ! modifier
                     antlist.append(vm.getAntennaIdsForAntennaName(token))
                 elif (token[0] == '!'):
                     if (token[1:] in uniqueAntennas):
                         antlist = uniqueAntennaIds
                         removeAntenna.append(vm.getAntennaIdsForAntennaName(token[1:]))
                     else:
                         print "Negated antenna %s is not in the ms. It contains: " % (token), uniqueAntennas
                         return(vm)
                 else:
                     twoants = token.split('&')
                     if (len(twoants) > 1):
                         myid = vm.getAntennaIdsForAntennaName(twoants[0])
                         if (twoants[0] in uniqueAntennas):
                             antlist.append(twoants[0])
                         if (twoants[1] in uniqueAntennas):
                             antlist.append(twoants[1])
                         print "The '&' character is not exactly supported"
                     else:
                         print "Antenna %s is not in the ms. It contains: " % (token), uniqueAntennas
                         return(vm)
             antlist = np.array(antlist)
             for rm in removeAntenna:
#                 print "Removing antenna %d = %s" % (rm, vm.getAntennaNamesForAntennaId(rm))
                 antlist = antlist[np.where(antlist != rm)[0]]
             antlist = list(antlist)
             if (len(antlist) < 1 and len(removeAntenna)>0):
                 print "Too many negated antennas -- there are no antennas left to plot."
                 return(vm)
         else:
             print "Antennas cannot be specified my name if the ms is not found."
             return(vm)
    elif (type(antenna) == list):
      # it's a list of integers
      antlist = antenna
    else:
      # It's a single, integer entry
      antlist = [antenna]

    if (len(antlist) > 0):
      antennasToPlot = np.intersect1d(uniqueAntennaIds,antlist)
    else:
      antennasToPlot = uniqueAntennaIds

    legend = ''
    for a in antennasToPlot:
        if (a != antennasToPlot[0]):
            legend += ', '
        if (msFound):
            legend += vm.getAntennaNamesForAntennaId(a)
    if (msFound):
        print "Baselines including these antennas will be plotted = ", legend

    spwlist = spw
    if (spwlist == []):
#        print "cal_desc_id = ", cal_desc_id
        spwlist = vm.spwInfo.keys()
    elif (type(spw) == int):
        spwlist = [spw]
    elif (type(spw) == str):
        if (str.isdigit(spw)):
            spwlist = [int(spw)]
        elif (spw.find(',')>=0):
            spwstrings = spw.split(',')
            spwlist = []
            for i in spwstrings:
                spwlist.append(int(i))
        else:
            spwlist = vm.spwInfo.keys()
#            print "spw is a blank string"
    else:
        print "Invalid spw"
        return
    print "spws in the dataset = ", vm.spwInfo.keys() # range(len(vm.spwInfo))
    print "spws to plot = ", spwlist
    
    for spw in spwlist:
        if (spw not in vm.spwInfo.keys() or spw < 0):
            print "spw %d is not in the dataset" % (spw)
            return
    if (len(spwlist) > 1):
        print "*** Note: WVR solutions for different spws differ only by the frequency ratio."
        print "*** So, examining only one of them is usually sufficient (and faster)."

    # Parse the field string to emulate plotms
    removeField = []
    if (type(field) == str):
     if (len(field) == sum([m in myValidCharacterListWithBang for m in field])):
         # a list of field numbers was given
         tokens = field.split(',')
         fieldlist = []
         for token in tokens:
             if (token.find('*')>=0):
                 fieldlist = uniqueFields
                 break
             elif (token.find('!')==0):
                 fieldlist = uniqueFields
                 removeField.append(int(token[1:]))
             elif (len(token) > 0):
                 if (token.find('~')>0):
                     (start,finish) = token.split('~')
                     fieldlist +=  range(int(start),int(finish)+1)
                 else:
                     fieldlist.append(int(token))
         fieldlist = np.array(fieldlist)
         for rm in removeField:
             fieldlist = fieldlist[np.where(fieldlist != rm)[0]]
         fieldlist = list(fieldlist)
         if (len(fieldlist) < 1 and len(removeField)>0):
             print "Too many negated fields -- there are no fields left to plot."
             return(vm)
     else:
         # The field name (or list of names, or wildcard) was specified
         tokens = field.split(',')
         if (msFound):
             fieldlist = []
             removeField = []
             for token in tokens:
                 myloc = token.find('*')
                 if (myloc > 0):
                     for u in uniqueFields:
                         if (token[0:myloc]==vm.getFieldNamesForFieldId(u)[0:myloc]):
                             if (DEBUG):
                                 print "Found wildcard match = %s" % vm.getFieldNamesForFieldId(u)
                             fieldlist.append(u)
                         else:
                             if (DEBUG):
                                 print "No wildcard match with = %s" % vm.getFieldNamesForFieldId(u)
                 elif (myloc==0):
                     for u in uniqueFields:
                         fieldlist.append(u)
                 elif (token in vm.uniqueFields):
                     fieldlist = list(fieldlist)  # needed in case preceding field had ! modifier
                     fieldlist.append(vm.getFieldIdsForFieldName(token))
                 elif (token[0] == '!'):
                     if (token[1:] in vm.uniqueFields):
                         fieldlist = uniqueFields
                         removeField.append(vm.getFieldIdsForFieldName(token[1:]))
                     else:
                         print "Field %s is not in the ms. It contains: " % (token), vm.uniqueFields
                         return(vm)
                 else:
                     print "Field %s is not in the ms. It contains: " % (token), vm.uniqueFields
                     return(vm)
             fieldlist = np.array(fieldlist)
             for rm in removeField:
                 fieldlist = fieldlist[np.where(fieldlist != rm)[0]]
             fieldlist = list(fieldlist)
             if (len(fieldlist) < 1 and len(removeField)>0):
                 print "Too many negated fields -- there are no fields left to plot."
                 return(vm)
         else:
             print "Fields cannot be specified my name if the ms is not found."
             return(vm)
    elif (type(field) == list):
      # it's a list of integers
      fieldlist = field
    else:
      # It's a single, integer entry
      fieldlist = [field]

    if (len(fieldlist) > 0):
      fieldsToPlot = np.intersect1d(uniqueFields,np.array(fieldlist))
      if (len(fieldsToPlot) < 1):
          print "Source not found in ms"
          return(vm)
    else:
      fieldsToPlot = uniqueFields  # use all fields if none are specified

    print "fields in the dataset = ", range(len(vm.uniqueFields))
    print "Fields to plot = ", fieldsToPlot
    for x in fieldsToPlot:
        if (x < 0 or x >= len(vm.uniqueFields)):
            print "Field %d is not in the dataset" % (x)
            return

    # Fill the array of baselineLengths
    antennaIndex1 = []
    antennaIndex2 = []
    baselines = []
    for i in range(len(uniqueAntennas)):
        for j in range(i,len(uniqueAntennas)):
            antennaIndex1.append(i)
            antennaIndex2.append(j)
    if (msFound):
        baselineLength = np.zeros([len(uniqueAntennas), len(uniqueAntennas)])
        for i in range(len(uniqueAntennas)):
            for j in range(i,len(uniqueAntennas)):
                if ((i != j) and (i not in removeAntenna) and (j not in removeAntenna)):
                    baselineLength[i][j] = getBaselineLength(msName,i,j,verbose=False)[0]
                    if (i in antennasToPlot or j in antennasToPlot):
                        baselines.append([i,j,baselineLength[i][j]])

    sortedBaselines = baselines
    # sort the baselineLengths of the antennas to plot
    if (sort.find('length')>=0):
        for i in range(1,len(sortedBaselines)):
            for j in range(1,len(sortedBaselines)):
                if (sortedBaselines[j][2] < sortedBaselines[j-1][2]):
                    swap = sortedBaselines[j-1]
                    sortedBaselines[j-1] = sortedBaselines[j]
                    sortedBaselines[j] = swap
    pb.ion()  # Added on April 26, 2012 to address Crystal's problem
    pb.clf()
    for s in range(len(spwlist)):
        spw = spwlist[s]
        matchField = np.where(cal_desc_id == spw)[0]
        index = 0
        for myi in range(len(baselines)):
            if (sort.find('length')<0):
                i = baselines[myi][0]
                j = baselines[myi][1]
            else:
                i = sortedBaselines[myi][0]
                j = sortedBaselines[myi][1]
            matches = np.intersect1d(matchField,np.where(antennas==i)[0])
            antenna1 = antennas[matches]
            time1 = times[matches]
            gain1 = gain[matches]
            fields1 = fields[matches]
            matches2 = np.intersect1d(matchField,np.where(antennas==j)[0])
            gain2 = gain[matches2]
# This was only needed to check that times agreed, and they do.
#            antenna2 = antennas[matches2]
#            time2 = times[matches2]
#            fields2 = fields[matches2]
#            (agreement, failures) = checkTimeAgreement(i,j,time1,time2,fid=0)
#            if (agreement == False):
#                print "Time mismatch at baseline %d-%d, " % (antenna1[0],antenna2[0])
#                continue
            startTime = time1[0]
            if (xaxis.find('sec')>=0):
                timeplot = time1 - startTime
            else:
#                print "converting starting MJD seconds (%f) to UT hours" % (startTime)
                timeplot = 24*(time1/86400. - floor(time1[0]/86400.))
            phasediff = np.arctan2(np.imag(gain1), np.real(gain1)) - np.arctan2(np.imag(gain2),np.real(gain2))
            phasediff = phaseUnwrap(phasediff)
            posvalues = phasediff[np.where(phasediff>0)[0]]
            if (np.median(posvalues) > math.pi/2.):
                phasediff -= np.median(posvalues)
            else:
                phasediff -= np.median(phasediff)
            if (unwrap):
                  for i in range(len(phasediff)):
                    while (phasediff[i] > math.pi):
                        phasediff[i] -= 2*math.pi
                    while (phasediff[i] < -math.pi):
                        phasediff[i] += 2*math.pi
            phasediff *= 180/np.pi
            xframe += 1
            if (xframe > xframeStop):
                DrawSourceLegend(yrange, fieldsToPlot, msFound, vm, subplotRows, mysrcsize)
                if (interactive):
                    pb.draw()
                    myinput = raw_input("Press return for next page (q to quit): ")
                else:
                    if (figfile!=''):
                        print "Plotting page %d" % (plotctr)
                    myinput = ''
                if (figfile!=''):
                    if (figfile == True):
                        pb.savefig(caltable+'.plotWVRSolutions.%d.png'%plotctr, density=108)
                    else:
                        pb.savefig(figfile.split('.png')[0]+'.%d.png'%plotctr, density=108)
                    plotctr += 1
                if (myinput.find('q')>=0):
                    return
                pb.clf()
                xframe = xframeStart+1
#            print "Plotting baseline %d-%d (%s-%s) on spw %d" % (i,j,
#                    vm.getAntennaNamesForAntennaId(i),vm.getAntennaNamesForAntennaId(j),spw)
            adesc = pb.subplot(xframe)
            pb.hold(True)
            resizeFonts(adesc,mysize)
            if ((xrange[0] != 0 or xrange[1] != 0) and yrange[0] ==0 and yrange[1]==0):
                # recalculate the y limits for the displayed range
                tmatches1 = np.where(timeplot > xrange[0])[0]
                tmatches2 = np.where(timeplot <= xrange[1])[0]
                tmatches = np.intersect1d(tmatches1,tmatches2)
                phasediff -= np.median(phasediff[tmatches])
                ylim = [np.min(phasediff[tmatches]), np.max(phasediff[tmatches])]
                myyrange = ylim[1]-ylim[0]
                ylim = [ylim[0]-0.1*myyrange,ylim[1]+0.1*myyrange]
            else:
                ylim = pb.ylim()
            list_of_date_times = mjdSecondsListToDateTime(time1)
            timeplotMatches = pb.date2num(list_of_date_times)
            for f in range(len(fieldsToPlot)):
                field = fieldsToPlot[f]
                matches = np.where(fields1 == field)[0]
                if (xaxis.find('ut') >= 0):
                    pdesc = adesc.plot_date(timeplotMatches[matches],phasediff[matches],'.')
                    adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,30)))
                    adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
                    adesc.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
                    adesc.fmt_xdata = matplotlib.dates.DateFormatter('%H:%M')
                    RescaleXAxisTimeTicks(pb.xlim(), adesc)
                else:
                    pdesc = pb.plot(timeplot[matches],phasediff[matches],'.')
                pb.setp(pdesc, color=overlayColors[field])
            if (xframe in bottomRow):
                if (xaxis.find('sec')>=0):
                    pb.xlabel("Time since start (sec)", size=mysize)
                else:
                    pb.xlabel("Time (UT on %s)"%(plotbp.utdatestring(startTime)), size=mysize)
            if (xframe in leftColumn):
                pb.ylabel("Phase difference (deg)", size=mysize)
            if (msFound):
                pb.title('%.0fm Baseline %d-%d=%s-%s, spw%d=%.1fGHz' % (baselineLength[antenna1[0]][antennas[matches2][0]],
                        antenna1[0], antennas[matches2][0],
                        vm.getAntennaNamesForAntennaId(antenna1[0]),
                        vm.getAntennaNamesForAntennaId(antennas[matches2][0]),spw, np.mean(getFrequencies(msName,spw)*1e-9)),
                        size=mysize)
            else:
                pb.title('Baseline %d-%d, spw %d' % (antenna1[0], antennas[matches2][0], spw), size=mysize)

            pb.subplots_adjust(hspace=myhspace, wspace=mywspace)
            yFormatter = ScalarFormatter(useOffset=False)
            adesc.yaxis.set_major_formatter(yFormatter)
            adesc.xaxis.grid(True,which='major')
            adesc.yaxis.grid(True,which='major')
            if (xrange[0] != 0 or xrange[1] != 0):
                if (xaxis.find('sec')>=0):
                    pb.xlim(xrange)
                else:
                    avgdatetime = num2date(np.mean(timeplotMatches[matches]))
                    avgdate = datetime.datetime.date(avgdatetime)
                    d1 = date2num(avgdate) + xrange[0]/24.
                    d2 = date2num(avgdate) + xrange[1]/24.
                    pb.xlim([d1,d2])
                pb.ylim(ylim)
            else:
                if (xaxis.find('sec')>=0):
                    pb.xlim(0, np.max(times)-np.min(times))

            xlim = pb.xlim()
            if (xlim[1] - xlim[0] < 10/1440.):
                adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,1)))
                adesc.xaxis.set_minor_locator(matplotlib.dates.SecondLocator(bysecond=range(0,60,30)))
            elif (xlim[1] - xlim[0] < 0.5/24.):
                adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,5)))
                adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,1)))
            elif (xlim[1] - xlim[0] < 1/24.):
                adesc.xaxis.set_major_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,10)))
                adesc.xaxis.set_minor_locator(matplotlib.dates.MinuteLocator(byminute=range(0,60,2)))
                
            if (yrange[0] != 0 or yrange[1] != 0):
                pb.ylim(yrange)
                
            if (xframe in leftColumn and xframe in topRow):
                (xlim,ylim,myxrange,myyrange) = GetLimits()
                pb.text(0, 1.12, caltable+' computed for '+msName.split('/')[-1],
                        size=titlesize, transform=adesc.transAxes)
        DrawSourceLegend(yrange, fieldsToPlot, msFound, vm, subplotRows, mysrcsize)
                        
    pb.draw()
    if (figfile!=''):
        if (figfile == True):
            try:
                pb.savefig(caltable+'.plotWVRSolutions.%d.png'%plotctr, density=108)
            except:
                print "WARNING:  Could not save plot file.  Do you have write permission here?"
        else:
            try:
                pb.savefig(figfile+'.%d.png'%plotctr, density=108)
            except:
                print "WARNING:  Could not save plot file.  Do you have write permission here?"
# end of plotWVRSolutions()

def DrawSourceLegend(yrange, fieldsToPlot, msFound, vm, subplotRows, mysrcsize):
                 (xlim,ylim,myxrange,myyrange) = GetLimits()
                 if (yrange[0] == 0 and yrange[1] == 0):
                    # Make space for source labels at the bottom of the plot
                    pb.ylim([ylim[0] - myyrange*0.25, ylim[1]])
                    (xlim,ylim,myxrange,myyrange) = GetLimits()
                 for f in range(len(fieldsToPlot)):
                    field = fieldsToPlot[f]
                    if (msFound):
                        myfieldstring = '%d: %s' % (field, vm.getFieldNamesForFieldId(field))
                    else:
                        myfieldstring = 'field %d' % (field)
                    if (f < (1+len(fieldsToPlot))/2):
                        myxpos = 0.05
                        myypos = (f+0.3)*0.03*subplotRows
                    else:
                        myxpos = 0.50
                        myypos = (f-(1+len(fieldsToPlot))/2+0.3)*0.03*subplotRows
                    pb.text(myxpos, myypos, myfieldstring, color=overlayColors[field],
                            size=mysrcsize, transform=pb.gca().transAxes)

def GetLimits():
    """
    Gets the current plot limits and ranges and returns them.
    -- Todd Hunter
    """
    xlim = pb.xlim()
    ylim = pb.ylim()
    myxrange = xlim[1]-xlim[0]
    myyrange = ylim[1]-ylim[0]
    return(xlim,ylim,myxrange,myyrange)

def phaseUnwrap(phase):
   """
   Algorithm from Ed Fomalont called from plotWVRSolutions.  Accepts angle in radians.
   Take phase difference, adds 4.5 turns, mods by 1 turn, subtract 0.5 turns. - Todd Hunter
   """
   for i in range(1,len(phase)):
       phaseDiff = phase[i]-phase[i-1]
       pd = np.mod(phaseDiff + 4.5*(2*np.pi), 2*np.pi) - np.pi
       phase[i] = phase[i-1]+pd
   return(phase)
   
def checkTimeAgreement(a,b,t1,t2, fid):
    """
    Initially used by plotWVRSolutions to confirm that times match between antennas.
    But not currently used.  -- Todd Hunter
    """
    failures = []
    for i in range(len(t1)):
        if (fid != 0):
            fid.write('%2d %2d %.2f %.2f\n'%(a,b,t1[i],t2[i]))
        if (np.abs(t1[i] - t2[i]) > 1):
            failures.append(i)
    if (len(failures) > 0):
        return(False, failures)
    else:
        return(True, failures)

def scaleweights(msfile,wtfac=1.0):
    """
    scaleweights:  scale data weights by a user supplied factor
    Created S.T. Myers 2011-08-23  v1.0 from flagaverage.py (CAS-2422)
    Usage:
          scaleweights(msfile,wtfac)
              wtfac =  multiplicative weight factor
    For help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/Scaleweights
    -- Todd Hunter
    """
    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)
#    mstool = casac.homefinder.find_home_by_name('msHome')
#    ms = casac.ms = mstool.create()
#    tbtool = casac.homefinder.find_home_by_name('tableHome')
#    tb = casac.tb = tbtool.create()

    import types
    if type(wtfac) is not types.FloatType and type(wtfac) is not types.IntType:
        print 'ERROR: wtfac must be a float or integer'
        return(1)

    print 'Will scale weights by factor '+str(wtfac)

    # Access the MS
    if (os.path.exists(msfile) == False):
        print "ms does not exist = %s" % (msfile)
        return(2)

    try:
        ms.open(msfile,nomodify=False)
    except:
        print "ERROR: failed to open ms tool on file "+msfile
        return(3)
    # Find number of data description IDs
    tb.open(msfile+"/DATA_DESCRIPTION")
    ddspwlist=tb.getcol("SPECTRAL_WINDOW_ID")
    ddpollist=tb.getcol("POLARIZATION_ID")
    tb.close()
    ndd = ddspwlist.__len__()
    print 'Found '+str(ndd)+' DataDescription IDs'
    # Now the polarizations (number of correlations in each pol id
    tb.open(msfile+"/POLARIZATION")
    ncorlist=tb.getcol("NUM_CORR")
    tb.close()
    #
    ntotrow=0
    ntotpts=0
    for idd in range(ndd):
        # Find number of correlations in this DD
        pid = ddpollist[idd]
        ncorr = ncorlist[pid]
        # Select this DD (after reset if needed)
        if idd>0: ms.selectinit(reset=True)
        ms.selectinit(idd)
        #ms.selecttaql('SUM(WEIGHT)<'+str(wtcut))
        nwchan=0
        #recf = ms.getdata(["flag"])
        #(nx,nc,ni) = recf['flag'].shape
        # get the weights
        recw = ms.getdata(["weight"])
        (nx,ni) = recw['weight'].shape
        for j in range(ni):
            for i in range(nx):
                recw['weight'][i,j]*=wtfac
                nwchan+=1
        ms.putdata(recw)
        ntotrow+=ni
        ntotpts+=nwchan
        print 'Scaled '+str(nwchan)+' spectral points in '+str(ni)+' rows in DD='+str(idd)
    #
    print 'Scaled Total '+str(ntotpts)+' spectral points in '+str(ntotrow)+' rows'
    ms.close()
    return

def reset_weights(msfile,wtval=1.0):
    """
    Reset weights to a fixed value. Hacked from scaleweights by Adam Leroy.
    wtval: The new value to set all the weights to.
    """

    ### Imports and loading
    import os

    try:
        import casac
    except ImportError, e:
        print "failed to load casa:\n", e
        exit(1)

#    mstool = casac.homefinder.find_home_by_name('msHome')
#    ms = casac.ms = mstool.create()

    ### Access the MS
    if (os.path.exists(msfile) == False):
        print "ms does not exist = %s" % (msfile)
        return(2)

    try:
        ms.open(msfile,nomodify=False)
    except:
        print "ERROR: failed to open ms tool on file "+msfile
        return(3)

    ### Read in the weights and reset to the fixed value

    weights = ms.getdata(["weight"])
    weights["weight"] = weights['weight']/weights['weight']*wtval
    ms.putdata(weights)

    ### Close
    ms.close()

def timeShift(vis='', seconds=''):
    """
    Shift the observation time of an ms by a specified number of seconds in
    the OBSERVATION table TIME_RANGE column.
    This is simply a debugging tool and is not meant to correct a dataset!
    Todd Hunter
    """

    if (os.path.exists(vis) == False):
        print "vis does not exist = %s" % (vis)
        return(2)
    try:
        tb.open(vis+'/OBSERVATION',nomodify=False)
    except:
        print "ERROR: failed to open OBSERVATION table on file "+vis
        return(3)

    time_range = tb.getcol('TIME_RANGE')
    print "Time range changed from: %f - %f (%s - %s)" % (time_range[0], time_range[1],
                                                          plotbp.utstring(time_range[0]),
                                                          plotbp.utstring(time_range[1]))
    time_range[0] += seconds
    time_range[1] += seconds
    tb.putcol('TIME_RANGE',time_range)
    tb.close()
    print "                     to: %f - %f (%s - %s)" % (time_range[0], time_range[1],
                                                          plotbp.utstring(time_range[0]),
                                                          plotbp.utstring(time_range[1]))

def timeDifference(vis='', vis2=''):
    """
    Find the difference in observation start time between two ms.
    using the OBSERVATION table TIME_RANGE column.
    Todd Hunter
    """

    if (os.path.exists(vis) == False):
        print "vis does not exist = %s" % (vis)
        return(2)
    if (os.path.exists(vis2) == False):
        print "vis2 does not exist = %s" % (vis2)
        return(2)
    try:
        tb.open(vis+'/OBSERVATION')
    except:
        print "ERROR: failed to open OBSERVATION table on file "+vis
        return(3)
    time_range = tb.getcol('TIME_RANGE')
    tb.close()
    try:
        tb.open(vis2+'/OBSERVATION')
    except:
        print "ERROR: failed to open OBSERVATION table on file "+vis2
        return(3)
    time_range2 = tb.getcol('TIME_RANGE')
    tb.close()
    print "Time difference (vis2-vis1) = %f sec" % (time_range2[0]-time_range[0])

def uvwDifference(vis='', vis2=''):
    """
    Find the difference in uvw's between two (nearly) identical ms.
    Useful for confirming that fixvis did the right thing.
    Todd Hunter
    """

    if (os.path.exists(vis) == False):
        print "vis does not exist = %s" % (vis)
        return(2)
    if (os.path.exists(vis2) == False):
        print "vis2 does not exist = %s" % (vis2)
        return(2)
    try:
        tb.open(vis)
    except:
        print "ERROR: failed to open file "+vis
        return(3)
    uvw1 = tb.getcol('UVW')
    tb.close()
    try:
        tb.open(vis2)
    except:
        print "ERROR: failed to open file "+vis2
        return(3)
    uvw2 = tb.getcol('UVW')
    field = tb.getcol('FIELD_ID')
    tb.close()
    if (len(uvw1) != len(uvw2)):
        print "These ms differ in their number of rows (%d vs. %d)." % (len(uvw1),len(uvw2))
        return
    else:
        print "Boths ms have %d rows. Good." % (len(uvw1))
    fields = []
    differences = 0
    u = []
    v = []
    w = []
    for i in range(len(uvw1)):
        deltaU = uvw1[i][0]-uvw2[i][0]
        deltaV = uvw1[i][1]-uvw2[i][1]
        deltaW = uvw1[i][2]-uvw2[i][2]
        if (abs(deltaU)>0 or abs(deltaV)>0 or abs(deltaW)>0):
            u.append(abs(deltaU))
            v.append(abs(deltaV))
            w.append(abs(deltaW))
            fields.append(field[i])
            differences += 1
    fields = np.unique(fields)
    print "Found %d rows that differ." % (differences)
    if (differences > 0):
        print "Fields that differ: ", fields
        print "Median absolute differences: %f,%f,%f" % (np.median(u),np.median(v),np.median(w))

def getRADecForField(ms, myfieldId):
    """
    Returns RA,Dec in radians for the specified field in the specified ms.
    -- Todd Hunter
    """
    try:
        tb.open(ms+'/FIELD')
    except:
        print "Could not open FIELD table for ms=%s" % (ms)
        return([0,0])
    mydir = tb.getcell('DELAY_DIR',myfieldId)
#    if (mydir[0] < 0):
#        mydir[0] += 2*math.pi
    tb.close()
    return(mydir)

def getObservatoryName(ms):
    """
    Returns the observatory name in the specified ms.
    -- Todd Hunter
    """
    try:
        antTable = ms+'/OBSERVATION'
        tb.open(antTable)
        myName = tb.getcell('TELESCOPE_NAME')
        tb.close()
    except:
        print "Could not open OBSERVATION table to get the telescope name: %s" % (antTable)
        myName = ''
    return(myName)
    
def computeAzElFromRADecMJD(raDec, mjd, observatory='ALMA'):
    """
    Computes the az/el for a specified J2000 RA/Dec, MJD and observatory.

    raDec must be in radians: [ra,dec]
    mjd must be in days
    returns the [az,el] in radians
    - Todd Hunter
    """
    mydir = me.direction('J2000', qa.quantity(raDec[0],'rad'), qa.quantity(raDec[1],'rad'))
    me.doframe(me.epoch('mjd', qa.quantity(mjd, 'd')))
    me.doframe(me.observatory(observatory))
    myazel = me.measure(mydir,'azel')
    return([myazel['m0']['value'], myazel['m1']['value']])

def unnormalize(vis='',spwID='', scan='', state=None):
    """
    Multiply the cross-correlation spectra by the autocorrelation.
    Under development....
    -- T. Hunter
    """
    antenna1 = '0'
    antenna2 = '1'
    data = Visibility(vis,antenna1=antenna1, antenna2=antenna2,spwID=spwID,scan=scan,state=state, cross_auto_all='all')
    data.setAntennaPair(antenna1,antenna2)
    data.getAmpAndPhase()
    data.getSpectralData()
    print "shape(specFreq, amp)", np.shape(data.specFreq), np.shape(data.amp[0])
    pb.plot(1e-9*data.specFreq,data.amp[0])
    pb.title('Scan %d, Antennas %s-%s' % (scan,antenna1,antenna2))
    pb.xtitle('Freq (GHz)')
    pb.draw()

def analyzeWeights(vis='',antenna1='', antenna2='',scan=''):
    """
    Compares the SIGMA and WEIGHT for the first row of the table and
    checks if weight = 1/sigma**2.  If no arguments are specified, it
    uses the antenna numbers and scan number in the first row.
    -- T. Hunter
    """
    tb.open(vis)
    pol = 0
    if (antenna1==''):
        antenna1 = tb.getcol('ANTENNA1')[0]
    if (antenna2==''):
        antenna2 = tb.getcol('ANTENNA2')[0]
    if (scan==''):
        scan = tb.getcol('SCAN_NUMBER')[0]
    queryString = "ANTENNA1==%d AND ANTENNA2==%d AND SCAN_NUMBER==%d"%(int(antenna1),int(antenna2),int(scan))
    print "Checking first row with %s." % (queryString)
    subtable = tb.query(queryString)
    npols = len(subtable.getcol('SIGMA')[:,0])
    for pol in range(npols):
      weight = subtable.getcol('WEIGHT')[pol][0]
      sigma = subtable.getcol('SIGMA')[pol][0]
      product = sigma*math.sqrt(weight)
      if (abs(product -1) > 1e-5):
        print "Pol%d: The weights differ from the 1/sigma**2 by a factor of %g." % (pol, product)
      else:
        print "Pol%d: The weights are equal to 1/sigma**2." % (pol)
      

def compareAntennaPositionCorrections(antenna1='', parameter1='',
                                      antenna2='', parameter2=''):
    """
    Computes the difference (in mm) between two antenna baseline solutions
    on a per antenna basis.  The input format is the same as for gencal.
    Returns dictionaries of the vector differences and magnitudes.
    Example:
    antenna1='DV03,DV04'
    antenna2='DV03,DV04'
    parameter1=[-0.00011,3.965e-08,0.000126, -0.0001939,-2.638e-05,0.0001716]
    parameter2=[-0.00012,3.966e-08,0.000136, -0.0001839,-2.648e-05,0.0002716]
    -- Todd Hunter
    """
    if (antenna1=='' or antenna2=='' or parameter1=='' or parameter2==''):
        print "Usage: compareAntennaPositionCorrections(antenna1='', parameter1='', antenna2='', parameter2='')"
        return
    p1 = np.array(parameter1).reshape(len(parameter1)/3, 3)
    p2 = np.array(parameter2).reshape(len(parameter2)/3, 3)
    differences = {}
    magnitudes = {}
    mag = []
    ant1 = antenna1.split(',')
    ant2 = antenna2.split(',')
    antennas = np.unique(ant1+ant2)
    antennasPrinted = []
    for a1 in range(len(ant1)):
        ant = ant1[a1]
        myindex = ant2.index(ant)
        if (myindex >= 0):
            antennasPrinted.append(ant)
            differences[ant] = (p1[a1] - p2[myindex])*1000
            mag.append(sqrt(sum(np.array([differences[ant]])**2)))
            magnitudes[ant] = mag[-1]
            print "%s %+.3f %+.3f %+.3f   total = %+.3f mm" % (ant,
                                                        differences[ant][0],
                                                        differences[ant][1],
                                                        differences[ant][2],
                                                        magnitudes[ant]
                                                        )
    print "Standard deviation = %+.3f mm" % (np.std(mag))
    firstTime = True
    for ant in antennas:
        if (ant not in antennasPrinted):
            if (firstTime):
                print "Solutions present in only one argument:"
                firstTime = False
            if (ant in ant1):
                myindex = ant1.index(ant)
                magnitude = sqrt(sum(np.array([p1[myindex]])**2))*1000
                print "%s %+.3f %+.3f %+.3f   total = %+.3f mm" % (ant,
                                                        p1[myindex][0]*1000,
                                                        p1[myindex][1]*1000,
                                                        p1[myindex][2]*1000,
                                                        magnitude)
            else:
                myindex = ant2.index(ant)
                magnitude = sqrt(sum(np.array([p2[myindex]])**2))*1000
                print "%s %+.3f %+.3f %+.3f   total = %+.3f mm" % (ant,
                                                        p2[myindex][0]*1000,
                                                        p2[myindex][1]*1000,
                                                        p2[myindex][2]*1000,
                                                        magnitude)
                   
    return(differences, magnitudes)
        

def buildtarfile(path=os.path.dirname(__file__), outpath='~'):
    """
    Create a tar file with the minimum required files to distribute casa-related
    tools. Include a README file containing the date and time of creation, and
    user who built them.  For further help and examples, see
      https://safe.nrao.edu/wiki/bin/view/ALMA/Buildtarfile
    -- Todd Hunter
    """
    pathbits = path.split('/')
    datestring = datetime.date.today().strftime('%Y-%m-%d %h:%m:%s')
    if (pathbits[-1] == 'analysis_scripts'):
        inpath = ''
        for p in range(len(pathbits)-1):
            if (pathbits[p] != ''):
                inpath += '/'
            inpath += pathbits[p]
    else:
        inpath = path
    cmd = 'echo Built on %s by %s > %s/analysis_scripts/README' % (datetime.datetime.now().ctime(), os.getenv('USER'),inpath)
    print cmd
    os.system(cmd)
    cmd = 'echo CVS ID%s >>  %s/analysis_scripts/README' % (version(), inpath)
    print cmd
    os.system(cmd)
    print "Extracting files from %s" % (inpath)
    tarfile = '%s/analysis_scripts.tar' % (outpath)
    cmd = 'cd %s ; tar cvf %s' % (inpath,tarfile)
    files = ['analysisUtils.py','fileIOPython.py','mpfit.py','plotbandpass.py',
             'plotbandpass2.py','readscans.py','XmlObjectifier.py','README']
    for f in files:
        cmd += ' analysis_scripts/%s' % (f)
    os.system(cmd) 
    print "Tar file left at = %s" % (tarfile)
    print "To deploy to ftpsite, run: cp %s /home/ftp/pub/casaguides/" % (tarfile)

def findClosestTime(mytimes, mytime):
    myindex = 0
    mysep = np.abs(mytimes[0]-mytime)
    for m in range(1,len(mytimes)):
        if (np.abs(mytimes[m] - mytime) < mysep):
            mysep = np.abs(mytimes[m] - mytime)
            myindex = m
    return(myindex)

def getChanRangeFromFreqRange(vis=None, fieldid=0, spwid=None, minf=None, maxf=None, refframe='TOPO'):

    """
    returns a list of the two channels in an SPW corresponding
       to the minimum and maximum frequency in the given ref frame
    
    vis - MS name
    fieldid - field id of the observed field (for reference frame calculations)
    spwid - id of the SPW in question
    minf - minimum freq in Hz
    maxf - maximum freq in Hz
    refframe - frequency reference frame

    written by Dirk Petry
    """

    if(vis==None or spwid==None or minf==None or maxf==None):
        raise Exception('getChanRangeFromFreqRange: need to provide values for vis, spwid, minf, and maxf')

    rval = [-1,-1]

    if(minf>maxf):
        return rval

    iminf = -1
    imaxf = -1

    ms.open(vis)
    a = ms.cvelfreqs(fieldids=[fieldid],spwids=[spwid],mode='frequency', outframe=refframe)
    ms.close()

    maxc = len(a)-1

    ascending = True
    lowedge = a[0]
    ilowedge = 0
    upedge = a[maxc]
    iupedge = maxc
    if(maxc>0 and (a[maxc]<a[0])): # frequencies are descending
        print "Frequencies are descending in spw ", spwid
        ascending = False
        lowedge = a[maxc]
        upedge = a[0]
        ilowedge = maxc
        iupedge = 0

    if(minf<lowedge):
        if(maxf>lowedge):
            if(maxf>upedge):
                iminf = ilowedge
                imaxf = iupedge
            else:
                iminf = ilowedge
                # use imaxf from below search
                imaxf = -2
        #else:
            # both imaxf and iminf are -1
    else:
        if(minf<upedge):
            if(maxf>=upedge):
                # take iminf from below search
                iminf = -2
                imaxf = iupedge
            else:
                #take both iminf and imaxf from above search 
                iminf = -2
                imaxf = -2
        #else:
            # both imaxf and iminf are -1

    if ascending:
        if iminf==-2:
            for i in xrange(0,len(a)):
                if a[i]>=minf:
                    print "Found ",i," ",a[i]
                    iminf = i
                    break
        if imaxf==-2:
            for j in xrange(iminf,len(a)):
                if a[j]>=maxf:
                    print "Found ",j," ",a[j]
                    imaxf = j
                    break
        rval = [iminf,imaxf]
    else:
        if iminf==-2:
            for i in xrange(len(a)-1, -1, -1):
                if a[i]>=minf:
                    print "Found ",i," ",a[i]
                    iminf = i
                    break
        if imaxf==-2:
            for j in xrange(iminf,-1,-1):
                if a[j]>=maxf:
                    print "Found ",j," ",a[j]
                    imaxf = j
                    break
        rval = [imaxf, iminf]

    return rval

def removeTsysSpike(calTable='', antenna=-1, spw=-1, startchan=-1, endchan=-1):
    """
    This is a utility to interpolate across a spike in the Tsys values for one antenna,
    one spw, both polarizations. The spw number should be
    the original number in the parent ms.  It automatically detects and processes
    old cal tables (casa 3.3) or new cal tables (casa 3.4).
    - Todd Hunter
    """
    antenna = int(antenna)
    spw = int(spw)
    tb.open(calTable,nomodify=False)
    antennas = tb.getcol('ANTENNA1')
    names = tb.colnames()
    if ('CAL_DESC_ID' not in names):
        spws = tb.getcol('SPECTRAL_WINDOW_ID')
        newCalTable = True
    else:
        spws = tb.getcol('CAL_DESC_ID')  # These start at zero
        tb.close()
        newCalTable = False
        tb.open(calTable+'/CAL_DESC')
        truespws = list(tb.getcol('SPECTRAL_WINDOW_ID')[0])
        tb.close()
        tb.open(calTable,nomodify=False)

    replaced = 0
    for i in range(len(antennas)):
        if (newCalTable):
            myspw = spw
            colname = 'FPARAM'
        else:
            myspw = truespws.index(spw)
            colname = 'GAIN'
        if (antennas[i] == antenna and spws[i] == myspw):
            print "Replacing row %d" % (i)
            gain = tb.getcell(colname, i)
            for j in range(len(gain)):
                for chan in range(startchan,endchan):
                    gain[j][chan] = 0.5*(gain[j][startchan-1] + gain[j][endchan])
            tb.putcell(colname,i,gain)
            replaced += 1
    #
    tb.close() 
    print "Replaced %d rows" % (replaced)

def replaceTsys(calTable='', antenna=-1, spw=-1, frompol='X', topol='Y', scaleFactor=1.0):
    """
    This is a utility to copy a scaled version of the Tsys values for one antenna,
    one spw, one polarization, to another polarization.  The spw number should be
    the original number in the parent ms.  It automatically detects and processes
    old cal tables (casa 3.3) or new cal tables (casa 3.4).
    - Todd Hunter
    """
    antenna = int(antenna)
    spw = int(spw)
    pols = ['X', 'Y']  # for now, assume this is always true
    frompol = pols.index(frompol)
    topol = pols.index(topol)
    tb.open(calTable,nomodify=False)
    antennas = tb.getcol('ANTENNA1')
    names = tb.colnames()
    if ('CAL_DESC_ID' not in names):
        spws = tb.getcol('SPECTRAL_WINDOW_ID')
        newCalTable = True
    else:
        spws = tb.getcol('CAL_DESC_ID')  # These start at zero
        tb.close()
        newCalTable = False
        tb.open(calTable+'/CAL_DESC')
        truespws = list(tb.getcol('SPECTRAL_WINDOW_ID')[0])
        tb.close()
        tb.open(calTable,nomodify=False)
    replaced = 0
    for i in range(len(antennas)):
        if (newCalTable):
            myspw = spw
            colname = 'FPARAM'
        else:
            myspw = truespws.index(spw)
            colname = 'GAIN'
        if (antennas[i] == antenna and spws[i] == myspw):
            print "Replacing row %d" % (i)
            gain = tb.getcell(colname,i)
            if (len(gain) > 2):
                print "This file has more than two polarizations, for which this script has not been tested."
                tb.close()
                return
            if (len(gain) < 2):
                print "This file has less than two polarizations, for which this script cannot be used."
                tb.close()
                return
            gain[topol] = gain[frompol] * scaleFactor
            tb.putcell(colname,i,gain)
            replaced += 1
    #
    tb.close()
    print "Replaced %d rows" % (replaced)
    
def buildPdfFromPngs(pnglist=[],pdfname='',convert='convert',gs='gs'):
    """
    Will convert a list of PNGs into PDF, then concatenate them into one PDF.
    Arguments:
    pnglist:  a list of PNG files ['a.png','b.png']
    pdfname:  the filename to produce (default = my.pdf)
    convert:  specify the full path to ImageMagick's convert command (if necessary)
    gs:  specify the full path to ghostscript's gs command (if necessary)
    """
    filelist = ''
    if (len(pnglist) < 1):
        print "You must specify at least one file with pnglist=['myfile1','myfile2',...]."
        return
    for p in pnglist:
        cmd = '%s %s %s.pdf'%(convert,p,p)
        print "Running command = ", cmd
        mystatus = os.system(cmd)
        if (mystatus == 0):
            filelist += p + '.pdf '
        elif (mystatus == 256):
            print "Could not find one or more of the png files."
            return
        else:
            print "ImageMagick's convert command is missing, no PDF created."
            print "You can set the full path to convert with convert=''"
            return
    if (pdfname == ''):
        pdfname = 'my.pdf'
    if (len(pnglist) > 1):
        cmd = '%s -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=%s %s' % (gs,pdfname,filelist)
    else:
        cmd = 'cp %s %s' % (filelist,pdfname)
    print "Running command = %s" % (cmd)
    mystatus = os.system(cmd)
    if (mystatus == 0):
        print "PDF left in %s" % (pdfname)
#        os.system("rm -f %s" % filelist)
    else:
        print "gs (ghostscript) is missing, no PDF created"
        print "You can set the full path to gs with gs=''"
    
def clearflags(caltable='', absoluteValue=False):
    """
    Clear all flags in a 3.4 tsys table, with an option to replace the
    FPARAM with the absolute value of the existing FPARAM.
    Todd Hunter
    """
    tb.open(caltable,nomodify=False)
    caltime = tb.getcol('TIME')
    for i in range(len(caltime)):
        flag = tb.getcell('FLAG',i)
        fparam = tb.getcell('FPARAM',i)
        c = 0
        for f in flag:
            lf = list(f)
            c += lf.count(1)
        if (c > 0):
            flag *= 0
            tb.putcell('FLAG',i,flag)
            if (absoluteValue):
                fparam = tb.getcell('FPARAM',i)
                fparam = np.abs(fparam)
                tb.putcell('FPARAM',i,fparam)
                print "Cleared %d flags in row %d and took absolute value of gain" % (c,i)
            else:
                print "Cleared %d flags in row %d" % (c,i)
    tb.close()

def listflags(ms='',maxrows=0, verbose=False, startrow=0):
    """
    Examine all rows in an ms that have flagged data, and report statistics.
    Options:
    verbose: if True, list all row numbers
    maxrows: if maxrows > 0, then stop after examining maxrows
    Todd Hunter
    """
    tb.open(ms)
    times = tb.getcol('TIME')
    totalrows = 0
    totalflags = 0
    totalpoints = 0
    interval = 1000
    lt = len(times)/interval
    print "Total rows in ms = %d" % (len(times))
    rows = []
    fo = open(ms.split('/')[-1]+'.listflags','w')
    for i in range(startrow,len(times)):
        flags = tb.getcell('FLAG',i)
        for f in flags:
            lf = list(f)
            c = lf.count(1)
            totalrows += 1
            if (c > 0):
                if (verbose):
                    print "Found %d flags in row %d" % (c,i)
                rows.append(i)
                fo.write('%d %d\n'%(i,c))
                totalflags += c
            totalpoints += len(lf)
        if (totalrows>=maxrows and maxrows>0): break
        if (i%lt == 0):
            print "Row %d: Done %.1f%% (%d flags found)" % (i,100.0*i/lt/interval, totalflags)
    tb.close()
    print "This dataset has %d rows." % (len(times))
    fo.write("This dataset has %d rows.\n" % (len(times)))
    print "Found %d flags (%.4f%%) in %d different rows." % (totalflags, 100*totalflags/(1.0*totalpoints), totalrows)
    fo.write("Found %d flags (%.4f%%) in %d different rows.\n" % (totalflags, 100*totalflags/(1.0*totalpoints), totalrows))
    fo.close()

def getFieldsForTime(ft, time):
    """
    Return the field with an observation time closest to the specified time.
    Input: a ValueMapping structure (vm.fieldsForTimes), and an MJD_seconds time
    Output: field name
    Called by detectNegativeTsys()
    Todd Hunter
    """
    mindiff = 1e8
    myfield = 'None'
    for src in ft.keys():
        diff = ft[src] - time
        if (np.min(np.abs(diff)) < mindiff):
            mindiff = np.min(np.abs(diff))
            myfield = src
    return(myfield)

def getScansForTime(ft, time):
    """
    Return the scan with an observation time closest to the specified time.
    Input: a ValueMapping structure (vm.scansForTimes), and an MJD_seconds time
    Output: field name
    Called by detectNegativeTsys()
    Todd Hunter
    """
    mindiff = 1e12
    myfield = 'None'
    for scan in ft.keys():
        diff = ft[scan] - time
        if (np.min(np.abs(diff)) < mindiff):
            mindiff = np.min(np.abs(diff))
            myscan = scan
    return(myscan)

def plotTsys(vis='', antenna = '', spw='', xaxis='freq', gs='gs',
             buildpdf=False, interactive=True, overlay='',
             plotrange=[0,0,0,0],pol=''):
    """
    Plot the Tsys from the SYSCAL table of the specified ms.  
    Produce a multi-page pdf.
       xaxis: 'freq' (default) or 'chan'
       gs: full path to ghostscript's gs
       overlay: '' or 'antenna'
       pol: '' or 'X' or 'Y'
       figfile:  '' (no png produced) or filename
    Consider moving this to class CalTableExplorer someday
    Todd Hunter
    """
    if (buildpdf and figfile==''):
        figfile = '%s.plotTsys.png' % (vis)
    plotTcal(vis=vis, antenna=antenna, spw=spw, xaxis=xaxis,t='tsys',gs=gs,
             buildpdf=buildpdf, interactive=interactive,figfile='',
             overlay=overlay)

def plotTrx(vis='', antenna = '', spw='', xaxis='freq', gs='gs',
            buildpdf=False, interactive=True, overlay='',
            plotrange=[0,0,0,0],pol=''):
    """
    Plot the Trx from the SYSCAL table of the specified ms.  
    Produce a multi-page pdf.
       xaxis: 'freq' (default) or 'chan'
       gs: full path to ghostscript's gs
       overlay: '' or 'antenna'
       pol: '' or 'X' or 'Y'
       figfile:  '' (no png produced) or filename
    Consider moving this to class CalTableExplorer someday
    Todd Hunter
    """
    if (buildpdf and figfile==''):
        figfile = '%s.plotTsys.png' % (vis)
    plotTcal(vis=vis, antenna=antenna, spw=spw, xaxis=xaxis,t='trx',gs=gs,
             buildpdf=buildpdf, interactive=interactive,figfile='',
             overlay=overlay)

def plotTsky(vis='', antenna = '', spw='', xaxis='freq', gs='gs',
             buildpdf=False, interactive=True, overlay='',
             plotrange=[0,0,0,0],pol='',figfile=''):
    """
    Plot the Tsky from the SYSCAL table of the specified ms.  
    Produce a multi-page pdf.
       xaxis: 'freq' (default) or 'chan'
       gs: full path to ghostscript's gs
       overlay: '' or 'antenna'
       pol: '' or 'X' or 'Y'
       figfile:  '' (no png produced) or filename
    Consider moving this to class CalTableExplorer someday
    Todd Hunter
    """
    if (buildpdf and figfile==''):
        figfile = '%s.plotTsys.png' % (vis)
    plotTcal(vis=vis, antenna=antenna, spw=spw, xaxis=xaxis,t='tsky',gs=gs,
             buildpdf=buildpdf, interactive=interactive, figfile=figfile,
             overlay=overlay)

def plotTcal(vis='', antenna = '', spw='', xaxis='freq', t='tsys', gs='gs',
             buildpdf=False, interactive=True, overlay='',
             plotrange=[0,0,0,0],pol='', figfile=''):
    """
    Plot either the Tsys, Trx or Tsky from the SYSCAL table of the specified
    ms.  Produce a multi-page pdf.
       t: 'tsys' (default), 'trx' or 'tsky'
       xaxis: 'freq' (default) or 'chan'
       gs: full path to ghostscript's gs
       overlay: '' or 'antenna'
       pol: '' or 'X' or 'Y'
       figfile:  '' (no png produced) or filename
    Consider moving this to class CalTableExplorer someday
    Todd Hunter
    """
    if (os.path.exists(vis) == False):
        print "The ms does not exist."
        return
    if (os.path.exists(vis+'/SYSCAL') == False):
        print "The SYSCAL table for that ms does not exist."
        return
    t = t.lower()
    if (t!='tsys' and t!='trx' and t!='tsky'):
        print "t must be either tsys, trx or tsky"
        return
    if (overlay != '' and overlay != 'antenna'):
        print "You can only overlay 'antenna' or nothing ('')."
        return
    pol = pol.upper()
    if (pol not in ['','X','Y']):
        print "pol must be X or Y or ''"
        return
    if (type(antenna) == str):
        if (len(antenna) > 0):
            if (antenna.isdigit()==False):
                antennaName = antenna
                antenna = getAntennaIndex(vis, antenna)
            else:
                antenna = int(antenna)
                antennaName = getAntennaName(vis, antenna)
                
    else:
        antennaName = getAntennaName(vis, antenna)
    antennaNames = getAntennaNames(vis)
    tb.open(vis+'/SYSCAL')
    antennas = tb.getcol('ANTENNA_ID')
    times = tb.getcol('TIME')
    intervals = tb.getcol('INTERVAL')
    times -= 0.5*intervals
    if (len(antennas) < 1):
        print "The SYSCAL table is blank"
        return
    print "Antennas = ", np.unique(antennas)
    spws = tb.getcol('SPECTRAL_WINDOW_ID')
    uniqueAntennas = np.unique(antennas)
    tb.close()
    print "spws = ", np.unique(spws)
    pdfs = ''
    lines = ['-','--','..','-.']
    pb.clf()
    adesc = pb.subplot(111)
    if (spw == ''):
        # default to all spws
        spwlist = np.unique(spws)
    elif (type(spw) == list):
        if (type(spw[0]) == int):
            spwlist = spw
        else:
            spwlist = map(int, spw)
    elif (type(spw) == str):
        if (spw.find(',')>=0):
            intstrings = spw.split(',')
            spwlist = map(int, intstrings)
        else:
            spwlist = [int(spw)]
    else:
        spwlist = [int(spw)]
    myinput = ''
    for spw in spwlist:
      colorctr = 0
      if (myinput == 'q'):
          break
      pb.clf()
      adesc = pb.subplot(111)
      medians = []
      for i in range(len(antennas)):
        if (antennas[0] == antennas[i]):
            colorctr = 0
        if (spws[i] == spw):
          if (antenna == '' or antennas[i] == antenna):
            if (overlay==''):
                pb.clf()
                adesc = pb.subplot(111)
                mycolor = 'k'
            else:
                colorctr += 1
                mycolor = overlayColors[colorctr]
            tb.open(vis+'/SYSCAL')
            if (t == 'tsys'):
                tsys = tb.getcell('TSYS_SPECTRUM',i)
                ylab = 'Tsys (K)'
            elif (t == 'trx'):
                tsys = tb.getcell('TRX_SPECTRUM',i)
                ylab = 'Trx (K)'
            elif (t == 'tsky'):
                tsys = tb.getcell('TSKY_SPECTRUM',i)
                ylab = 'Tsky (K)'
            tb.close()
            medians.append(np.median(tsys))
            freq = getFrequencies(vis, spws[i])*1e-9
            if (xaxis=='freq'):
                for j in range(len(tsys)):
                    if (pol=='' or (pol=='X' and j==0) or (pol=='Y' and j==1)):
                        pb.plot(freq,tsys[j],'-', color=mycolor, ls=lines[j])
                    pb.hold(True)
                pb.xlabel('Frequency (GHz)', size=12)
            else:
                chan = range(len(freq))
                for j in range(len(tsys)):
                    pb.plot(chan, tsys, '-', color=mycolor)
                    pb.hold(True)
                pb.xlabel('Channel',size=10)
            if (overlay==''):
                pb.hold(False)
            yFormat = matplotlib.ticker.ScalarFormatter(useOffset=False)
            adesc.yaxis.set_major_formatter(yFormat)
            pb.ylabel(ylab,size=12)
            adesc.xaxis.grid(True,which='major')
            adesc.yaxis.grid(True,which='major')
            (mjd, utstring) = mjdSecondsToMJDandUT(times[i])
            if (plotrange[0] != 0 or plotrange[1] != 0):
                pb.xlim([plotrange[0],plotrange[1]])
            if (plotrange[2] != 0 or plotrange[3] != 0):
                pb.ylim([plotrange[2],plotrange[3]])
            if (overlay=='antenna'):
                pb.title('%s  spw %d  time %s' % (vis.split('/')[-1],spws[i],utstring),size=10)
                for a in range(len(uniqueAntennas)):
                    if (a*0.06 > 1.02):
                        x = 1.02
                        y -= 0.03
                    else:
                        y = 1.05
                        x = a*0.06
                    pb.text(x, y, '%s'%antennaNames[uniqueAntennas[a]], 
                            color=overlayColors[a], transform=pb.gca().transAxes,size=8)
            else:
                pb.title('%s  antenna %d=%s  spw %d  time %s' % (vis.split('/')[-1],antennas[i],
                                     antennaNames[antennas[i]],spws[i],utstring),size=10)
            png = vis + 'ant%d.spw%d.tsys.png'
            if (overlay==''):
                if (figfile != ''):
                    pb.savefig(png)
                    if (buildpdf):
                        os.system('convert %s %s.png' % (png,png))
                    pdfs += png + ' '
                if (interactive):
                    pb.draw()
                    if (i < len(antennas)-1):
                        myinput = raw_input("Press return for next page (q to quit): ")
                    else:
                        myinput = 'q'
                    if (myinput == 'q'):
                        break
      # end 'for' loop over rows in the table
      print "Median value for spw %d = %.1f K" % (spw, np.median(medians))
      if (interactive):
          pb.draw()
          if (myinput != 'q'):
              if (spwlist[-1] != spw):
                  myinput = raw_input("Press return for next spw (q to quit): ")
              else:
                  myinput = 'q'
          if (myinput == 'q'):
              break
    if (buildpdf):
        pdf = vis + '.tsys.pdf'
        cmd = '%s -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=%s %s' % (gs,pdf,pdfs)
        os.system(cmd)

def detectNegativeTsys(vis='', antenna = '', spw='', showfield=False, vm=''):
    """
    Reads the SYSCAL table of the specified ms and searches for negative
    values of TSYS_SPECTRUM, printing a summary for each row as it finds
    them.  If an antenna
    is specified (ID or name), then only that antenna will be displayed.  If an
    spw is specified (integer or string), then only that spw will be displayed.
    -Todd Hunter
    """
    if (os.path.exists(vis) == False):
        print "The ms does not exist."
        return
    if (os.path.exists(vis+'/SYSCAL') == False):
        print "The SYSCAL table for that ms does not exist."
        return
    if (type(antenna) == str):
        if (len(antenna) > 0):
            if (antenna.isdigit()==False):
                antennaName = antenna
                antenna = getAntennaIndex(vis, antenna)
            else:
                antenna = int(antenna)
                antennaName = getAntennaName(vis, antenna)
                
    else:
        antennaName = getAntennaName(vis, antenna)
    antennaNames = getAntennaNames(vis)
    if (showfield and vm==''):
        print "Running ValueMapping (because showfield=True)..."
        vm = ValueMapping(vis)
    tb.open(vis+'/SYSCAL')
    antennas = tb.getcol('ANTENNA_ID')
    spws = tb.getcol('SPECTRAL_WINDOW_ID')
    times = tb.getcol('TIME')
    intervals = tb.getcol('INTERVAL')
    times = times-0.5*intervals
    total = 0
    maxFieldLength = 16
    for i in range(len(spws)):
        tsys = tb.getcell('TSYS_SPECTRUM',i)
        neg = len(np.where(tsys<0)[0])
        possible = np.size(tsys)
        if (neg > 0):
          if (antenna=='' or antennas[i]==int(antenna)):
            if (spw=='' or spws[i] == int(spw)):
              if (showfield):
                myfield = getFieldsForTime(vm.fieldsForTimes,times[i])
                if (len(myfield) < maxFieldLength):
                    myfield += ' '*(maxFieldLength-len(myfield))
                if (len(myfield) > maxFieldLength):
                    myfield = myfield[0:maxFieldLength]
                myscan = getScansForTime(vm.scansForTimes,times[i])
                result = "Row%4d (ant%2d=%s, spw%2d, scan %2d, field=%16s) found %4d of %4d" % (i,antennas[i], antennaNames[antennas[i]], spws[i], myscan, myfield, neg, possible)
              else:
                result = "In row %4d (ant %2d=%s, spw %2d), found %4d negatives out of %4d" % (i,
                          antennas[i],antennaNames[antennas[i]],spws[i], neg, possible)
              print result
              total += neg
    tb.close()
    if (total == 0):
        print "No negatives found!"
    else:
        print "A total of %d negative values encountered." % (total)
    return(vm)

def detectNegativeTrx(vis='', antenna = '', spw='', showfield=False, vm=''):
    """
    Reads the SYSCAL table of the specified ms and searches for negative
    values of TRX_SPECTRUM, printing a summary for each row as it finds
    them.  If an antenna
    is specified (ID or name), then only that antenna will be displayed.  If an
    spw is specified (integer or string), then only that spw will be displayed.
    -Todd Hunter
    """
    if (os.path.exists(vis) == False):
        print "The ms does not exist."
        return
    if (os.path.exists(vis+'/SYSCAL') == False):
        print "The SYSCAL table for that ms does not exist."
        return
    if (type(antenna) == str):
        if (len(antenna) > 0):
            if (antenna.isdigit()==False):
                antennaName = antenna
                antenna = getAntennaIndex(vis, antenna)
            else:
                antenna = int(antenna)
                antennaName = getAntennaName(vis, antenna)
                
    else:
        antennaName = getAntennaName(vis, antenna)
    antennaNames = getAntennaNames(vis)
    if (showfield and vm==''):
        print "Running ValueMapping (because showfield=True)..."
        vm = ValueMapping(vis)
    tb.open(vis+'/SYSCAL')
    antennas = tb.getcol('ANTENNA_ID')
    spws = tb.getcol('SPECTRAL_WINDOW_ID')
    times = tb.getcol('TIME')
    intervals = tb.getcol('INTERVAL')
    times = times-0.5*intervals
    total = 0
    maxFieldLength = 16
    for i in range(len(spws)):
        trx = tb.getcell('TRX_SPECTRUM',i)
        neg = len(np.where(trx<0)[0])
        possible = np.size(trx)
        if (neg > 0):
          if (antenna=='' or antennas[i]==int(antenna)):
            if (spw=='' or spws[i] == int(spw)):
              if (showfield):
                myfield = getFieldsForTime(vm.fieldsForTimes,times[i])
                if (len(myfield) < maxFieldLength):
                    myfield += ' '*(maxFieldLength-len(myfield))
                if (len(myfield) > maxFieldLength):
                    myfield = myfield[0:maxFieldLength]
                myscan = getScansForTime(vm.scansForTimes,times[i])
                result = "Row%4d (ant%2d=%s, spw%2d, scan %2d, field=%16s) found %4d of %4d" % (i,antennas[i], antennaNames[antennas[i]], spws[i], myscan, myfield, neg, possible)
              else:
                result = "In row %4d (ant %2d=%s, spw %2d), found %4d negatives out of %4d" % (i,
                          antennas[i],antennaNames[antennas[i]],spws[i], neg, possible)
              print result
              total += neg
    tb.close()
    if (total == 0):
        print "No negatives Trx found!"
    else:
        print "A total of %d negative Trx values encountered." % (total)
    return(vm)

def detectLoadTemp(vis='', showfield=False, vm='',antenna=''):
    """
    Reads the CALDEVICE table of the specified ms and searches for unusual
    values of temperatureLoad, printing a summary for each row as it finds
    them.  Unusual is Tamb outside range 10-20C or Thot outside range 80-90C.
    -Bill Dent
    """
    Tamb_lo=10.0
    Tamb_hi=20.0
    Thot_lo=70.0
    Thot_hi=90.0

    if (os.path.exists(vis) == False):
        print "The ms does not exist."
        return
    if (os.path.exists(vis+'/CALDEVICE') == False):
        print "The CALDEVICE table for that ms does not exist."
        return
    if (type(antenna) == str):
        if (len(antenna) > 0):
            if (antenna.isdigit()==False):
                antennaName = antenna
                antenna = getAntennaIndex(vis, antenna)
            else:
                antenna = int(antenna)
                antennaName = getAntennaName(vis, antenna)
                
    else:
        antennaName = getAntennaName(vis, antenna)
    antennaNames = getAntennaNames(vis)
    print "Antennas being checked: ", antennaNames
    if (showfield and vm==''):
        print "Running ValueMapping (because showfield=True)..."
        vm = ValueMapping(vis)
    tb.open(vis+'/CALDEVICE')
    antennas = tb.getcol('ANTENNA_ID')
    times = tb.getcol('TIME')
    intervals = tb.getcol('INTERVAL')
    times = times-0.5*intervals
    total = 0
    maxFieldLength = 1
    nrows=len(antennas)
    print 'nrows ', nrows
    for i in range(0,nrows):
       T_amb= tb.getcell('TEMPERATURE_LOAD',i)[0]
       T_hot= tb.getcell('TEMPERATURE_LOAD',i)[1]
       if(T_amb < Tamb_lo or T_amb > Tamb_hi):
           antbad=tb.getcell('ANTENNA_ID',i)
           times[0]=tb.getcell('TIME',i)
           if(showfield):
               myscan = getScansForTime(vm.scansForTimes,times[0])
               myfield = getFieldsForTime(vm.fieldsForTimes,times[0])
               print "Scan", myscan," field",myfield," Antenna:", antennaNames[antbad]," Ambient load out of range. T_amb:", T_amb
           else:
               print "Row ", i, " Ambient load out of range. T_amb: ", T_amb," Antenna: ", antennaNames[antbad]
           total += 1
       if(T_hot < Thot_lo or T_hot > Thot_hi):
           antbad=tb.getcell('ANTENNA_ID',i)
           times[0]=tb.getcell('TIME',i)
           if(showfield):
               myscan = getScansForTime(vm.scansForTimes,times[0])
               myfield = getFieldsForTime(vm.fieldsForTimes,times[0])
               print "Scan", myscan," field",myfield," Antenna:", antennaNames[antbad]," Hot load out of range. T_hot:", T_hot
           else:
               print "Row ", i, " Hot load out of range. T_hot: ", T_hot," Antenna:", antennaNames[antbad]
           total += 1

    tb.close()
    if (total == 0):
        print "No problems in load temperatures found!"
    else:
        print "A total of %d load temperature problems encountered." % (total)
    return(vm)

def listflagscal(caltable='',maxrows=0, verbose=False):
    """
    Examine all rows in a caltable ms that have flagged data, and report statistics.
    Options:
    verbose: if True, list all row numbers
    maxrows: if maxrows > 0, then stop after examining maxrows
    Todd Hunter
    """
    tb.open(caltable)
    times = tb.getcol('TIME')
    fields = tb.getcol('FIELD_ID')
    antennas = tb.getcol('ANTENNA1')
    totalrows = 0
    totalflags = 0
    totalpoints = 0
    rows = []
    fo = open(caltable.split('/')[-1]+'.listflags','w')
    for i in range(len(times)):
        flags = tb.getcell('FLAG',i)
        for f in flags:
            lf = list(f)
            c = lf.count(1)
            if (c > 0):
                if (verbose):
                    print "Found %d flags in row %d, antenna=%d, field=%d" % (c,i,antennas[i],fields[i])
                totalrows += 1
                rows.append(i)
                fo.write('%d %d\n'%(i,c))
                totalflags += c
            totalpoints += len(lf)
        if (totalrows>=maxrows and maxrows>0): break
    tb.close()
    print "This dataset has %d rows." % (len(times))
    fo.write("This dataset has %d rows.\n" % (len(times)))
    print "Found %d flags (%.4f%%) in %d different rows." % (totalflags, 100*totalflags/(1.0*totalpoints), totalrows)
    fo.write("Found %d flags (%.4f%%) in %d different rows.\n" % (totalflags, 100*totalflags/(1.0*totalpoints), totalrows))
    fo.close()
    
def findChannelRanges(filename='', threshold=0.0, minwidth=1, startchan=0, 
            chanavg=1, nchan=0, absorption=False, extrachan=0, figfile='',
            spw='', outfile='', subcube=False, invertstring=''):
    """
    Takes as input an ascii spectrum output from an image cube at a specific
    location:
       column 0 is the frequency/velocity axis
       column 1 is the flux density axis.
    This function finds all channel ranges wider than 'minwidth' channels
    that are below a 'threshold' flux density.  It can thus be used to
    find a list of channel ranges that are free of *emission* lines. Using
    'startchan' and 'chanavg', it can convert the resulting channel ranges
    back to the parent uv dataset, accounting for any channel averaging that
    was used in CLEAN.  It also reports a list of the inverse channel ranges
    (in this case, minwidth is not applied).  It returns these two lists as
    strings.
    * The 'nchan' argument can be used to tell it how many channels
    were in the original dataset in case a final block of channels was not
    CLEANed. Note that if you are processing multiple subcubes from a single
    spw and will combine the channel lisst, then you probably do not want
    to use this parameter.
    * The 'absorption' argument can be used if you are trying to
    avoid *absorption* lines instead of emission lines.  In this case the
    threshold should be set to just below the typical continuum level, as
    it will simply negate the data and the threshold and process as normal.
    * The 'extrachan' argument will remove additional channels from both sides
    of each offending region.
    * If present, the 'spw' argument will be inserted at the beginning of
    the string followed by a colon.
    * If absorption=True and threshold is a list of two values, the more
    negative one will be interpreted as the absorption threshold and the more
    positive one will be interpreted as the emission threshold.
    * If 'outfile' is present, the printed results will be written to this
    file.
    * If 'subcube' is True, then the linefull region will not propagate to
    channels less than the startchan.  Otherwise it will fill to channel zero.
    * If 'invertstring' is a non-empty string, it will simply invert this
    list of channel ranges and stop.
    Todd Hunter
    """
    if (len(invertstring) > 0):
      myspw = spw
      checkForMultipleSpw = invertstring.split(',')
      mystring = ''
      for c in range(len(checkForMultipleSpw)):
        checkspw = checkForMultipleSpw[c].split(':')
        if (len(checkspw) > 1 and spw==''):
            spw = checkspw[0]
            checkForMultipleSpw[c] = checkspw[1]
        goodstring = checkForMultipleSpw[c]
        goodranges = goodstring.split(';')
        if (str(spw) != ''):
            mystring += str(spw) + ':'
        if (int(goodranges[0].split('~')[0]) > startchan):
            mystring += '%d~%d;' % (startchan,int(goodranges[0].split('~')[0])-1)
        multiWindows = False
        for g in range(len(goodranges)-1):
            r = goodranges[g]
            s = goodranges[g+1]
            mystring += '%d~%d' % (int(r.split('~')[1])+1, int(s.split('~')[0])-1)
            multiWindows = True
            if (g < len(goodranges)-2):
                mystring += ';'
        if (int(goodranges[-1].split('~')[-1]) < startchan+nchan-1):
            if (multiWindows):
                mystring += ';'
            mystring += '%d~%d' % (int(goodranges[-1].split('~')[-1])+1, startchan+nchan-1)
        if (c < len(checkForMultipleSpw)-1):
            mystring += ','
        spw = myspw  # reset to the initial value
      print "inverse = '%s'" % mystring
      return(mystring)
    if (filename == ''):
        print "Usage: findChannelRanges(filename='', threshold=0.0, minwidth=1, startchan=0,"
        print "                         chanavg=1, extrachan=0, figfile='', spw='', outfile='',"
        print "                         subcube=False, invertstring='')"
        return
    if (os.path.exists(filename) == False):
        print "File does not exist = ", filename
        return
    if (chanavg < 1):
        print "chanavg must be 1 (i.e. no averaging) or greater"
        return
    f = open(filename,'r')
    lines = f.readlines()
    velocity = []
    flux = []
    for line in lines:
        if (line.find('#') >= 0):
            continue
        (a,b) = line.split()
        velocity.append(float(a))
        if (absorption):
            flux.append(-float(b))
        else:
            flux.append(float(b))
    f.close()
    pb.clf()
    desc = pb.subplot(111)
    if (type(threshold) == list):
        # reduce a list of one to a single value
        if (len(threshold) < 2):
            threshold = threshold[0]
    thresholdList = threshold
    if (absorption):
        if (type(threshold) == list):
            threshold = -np.min(thresholdList)
            emissionThreshold = -np.max(thresholdList)
            print "Setting emissionThreshold to ", emissionThreshold
        else:
            threshold *= -1
        pb.plot(range(len(velocity)),-np.array(flux),'r-')
    else:
        pb.plot(range(len(velocity)),flux,'r-')
    pb.xlabel('Channel')
    pb.hold(True)
    w0 = -1
    goodranges = []
    for ch in range(len(velocity)):
        if ((flux[ch] < threshold) and
            (type(thresholdList) != list or flux[ch] > emissionThreshold)):
            if (w0 < 0):
                # A good range just started
                w0 = ch
        elif (w0 >= 0):
            # A good range just ended
            w1 = ch-1-extrachan
            w0 += extrachan
            if (w0 < len(velocity) and w1 >= 0):
                if ((w1-w0+1) >= minwidth):
                    goodranges.append([w0,w1])
            w0 = -1
    if (w0 >= 0):
        w1 = len(velocity)-1
        if (len(goodranges) > 0):
            w0 += extrachan
        if ((w1-w0+1) >= minwidth):
            goodranges.append([w0,w1])
    f = open(filename+'.ranges','w')
    mystring = "'"
    if (str(spw) != ''):
        mystring += str(spw) + ':'
    imagechans = 0
    uvchans = 0
    goodranges_in_uv = []
    linefreefluxes = []
    for i in range(len(goodranges)):
        if (i>0):
            mystring += ';'
        c0 = startchan+goodranges[i][0]*chanavg
        c1 = startchan+(goodranges[i][1])*chanavg
        goodranges_in_uv.append([c0,c1])
        linefreefluxes += flux[c0:c1+1]
        imagechans += (goodranges[i][1]+1-goodranges[i][0])
        uvchans += (c1-c0+1)
        mystring += '%d~%d' % (c0,c1)
        if (absorption):
            pb.plot(range(goodranges[i][0],goodranges[i][1]+1),
                    -np.array(flux[goodranges[i][0]:goodranges[i][1]+1]),
                    'k-')
        else:
            pb.plot(range(goodranges[i][0],goodranges[i][1]+1),
                    flux[goodranges[i][0]:goodranges[i][1]+1],
                    'k-')

    if (absorption):
        pb.plot([0,len(velocity)], [-threshold,-threshold],'b--')
        if (type(thresholdList) == list):
            pb.plot([0,len(velocity)], [-emissionThreshold,-emissionThreshold],'g--')
    else:
        pb.plot([0,len(velocity)], [threshold,threshold],'b--')
    goodstring = mystring + "'"
    print "linefree = ", goodstring
    if (outfile != ''):
        try:
            output = open(outfile,'w')
        except:
            print "Could not open output file = ", outfile
            return
        output.write("linefree = %s\n" % goodstring)
    percent = 100.*imagechans/len(velocity)
    print "%d/%d image channels selected (%.2f%%)" % (imagechans,len(velocity),percent)
    if (outfile != ''):
        output.write("%d/%d image channels selected (%.2f%%)\n" % (imagechans,len(velocity),percent))

    # Invert the channels selected
    badranges = []
    mystring = "'"
    if (str(spw) != ''):
        mystring += str(spw) + ':'
    if (1==1):
        if (goodranges_in_uv[0][0] > 0):
            if (subcube==False):
                # This was the original logic
                badranges.append([0,goodranges_in_uv[0][0]-1])
            else:
                # This is the new logic, meant for when you have split up
                # your spw into two chunks.
                if (startchan <= goodranges_in_uv[0][0]-1):
                    badranges.append([startchan,goodranges_in_uv[0][0]-1])
        for i in range(len(goodranges_in_uv)-1):
            badranges.append([goodranges_in_uv[i][1]+1, goodranges_in_uv[i+1][0]-1])
        if (goodranges_in_uv[len(goodranges_in_uv)-1][1] < nchan-1):
            badranges.append([goodranges_in_uv[len(goodranges_in_uv)-1][1]+1, nchan-1])
        # Now build a string of the inverted list
        for i in range(len(badranges)):
            if (i>0):
                mystring += ';'
            c0 = badranges[i][0]
            c1 = badranges[i][1]
            mystring += '%d~%d' % (c0,c1)

    badstring = mystring + "'"
    print "linefull = ", badstring
    if (outfile != ''):
        output.write("linefull = %s\n" % badstring)
    desc.xaxis.grid(True,which='major')
    desc.yaxis.grid(True,which='major')
    pb.draw()
    sigma = np.std(linefreefluxes)
    pb.title('%s:  linefree(%.0f%%)=black (sigma=%g),  linefull=red'%(filename,
                                                                      percent,
                                                                      sigma),
             size=12)
    if (len(figfile) > 0):
        if (figfile.find('.png') < 0):
            figfile += '.png'
        pb.savefig(figfile)
        print "Figured saved to %s" % (figfile)
    return(goodstring[1:-1],badstring[1:-1])

def makeSimulatedImage(phasecenter ='J2000 16h25m46.98000s -25d27m38.3300s',
              sourcecenter='J2000 16h25m46.891639s -25d27m38.326880s',
              flux=1.0, freq=690, field='', pointingspacing=2.0,
              configuration='alma.out10.cfg', bandwidth='1GHz', niter=100,
              imsize=256, mapsize='', integration='10m',
              totaltime='200m', cellsize='auto', threshold='10mJy/beam',
              shape='point', majoraxis=0, minoraxis=0, positionangle=0,
              refdate='2012/05/21', hourangle='transit'
              ):
    """
    Create a simulated ALMA image of a point source or uniform disk at a 
    specified direction and flux density (in Janskys). Here are the parameters:

    phasecenter: required argument, example: 'J2000 06h00m00.00s -04d00m00.0s'
    sourcecenter: if not specified, then assumed to be same as phasecenter
    flux: flux density of point source (in Jy)
    freq: frequency (in GHz)
    field: field name to use for simobserve project directory name
    configuration: anything in /usr/lib64/casapy/data/alma/simmos/
    bandwidth: in GHz
    imsize: in pixels
    mapsize: '' or value in arcsec
    totaltime: e.g. '200m'  for 200 minutes
    integration: e.g. '20s'  for 200 seconds
    cellsize: floating point value or 'auto' (~4 pix per beam)
    pointingspacing: units of primary beam, default = 2.0 (i.e. no mosaic)
    threshold: for CLEAN
    shape: 'point' or 'disk'
    majoraxis: value in arcsec
    minoraxis: value in arcsec
    positionangle: value in degrees
    refdate:  'YYYY/MM/DD'
    hourangle: 'transit' or '-3h'
    
    --Todd Hunter
    """
    if (field == ''):
        field = 'makeimage'
        project = field
    else:
        project = field + '.makeimage'
    if (phasecenter.find(',') > 0):
        print "phasecenter must not have a comma"
        return
    if (sourcecenter.find(',') > 0):
        print "sourcecenter must not have a comma"
        return
    if (phasecenter.find(':') > 0):
        print "phasecenter must be entered in this format: HHhMMmSS.SSs +DDdMMmSS.SSs"
        return
    if (sourcecenter.find(':') > 0):
        print "sourcecenter must be entered in this format: HHhMMmSS.SSs +DDdMMmSS.SSs"
        return
    if (sourcecenter == ''):
        sourcecenter = phasecenter
    if (shape != 'point' and shape != 'disk'):
        print "The shape argument must be either 'point' or 'disk'."
        return
    cl.done()
    os.system('rm -rf %s.cl'%field)
    if (majoraxis != 0 and minoraxis == 0):
        print "Setting minor axis equal to major axis."
        minoraxis = majoraxis
    cl.addcomponent(dir=sourcecenter, flux=flux, fluxunit='Jy',
                    freq='%.1fGHz'%freq, shape=shape,
                    majoraxis='%farcsec' % majoraxis,
                    minoraxis='%farcsec' % minoraxis,
                    positionangle='%fdeg' % positionangle)
    cl.rename("%s.cl"%field)
    cl.close()
    version = casalog.version().split()[2]
    if (cellsize == 'auto'):
        cellsize = 25.0/freq
    else:
        if (type(cellsize) == str):
            print "Cellsize must be 'auto' or a floating point value"
            return
    graphics = 'none'
    config = configuration
    if (config.find('.cfg')>0):
        config = config[0:-4]
    imagename = '%s/%s.%s' % (project,project,config)
    if (version == '3.3.0'):
        halfpower = 0.4
        simobserve(project=project,complist='%s.cl'%field,
               antennalist=configuration, totaltime=totaltime,
               mapsize=[mapsize,mapsize],graphics=graphics,
               compwidth=bandwidth,overwrite=True,
               direction=phasecenter, integration=integration,
               pointingspacing='%.2fPB'%pointingspacing,
               setpointings=True, hourangle=hourangle, refdate=refdate)
    else:
        halfpower = 0.5
        simobserve(project=project,complist='%s.cl'%field,
               antennalist=configuration, totaltime=totaltime,
               mapsize=[mapsize,mapsize],graphics=graphics,
               obsmode="int", compwidth=bandwidth,overwrite=True,
               direction=phasecenter, integration=integration,
               pointingspacing='%.2fPB'%pointingspacing,
               setpointings=True, hourangle=hourangle, refdate=refdate)
    vis = '%s.%s.ms'%(project,config)
    simanalyze(project=project, vis=vis,
               imsize=[imsize,imsize], imdirection=phasecenter,
               cell='%.2farcsec'%(cellsize), niter=niter,overwrite=True,
               analyze=False, threshold=threshold,graphics=graphics)
    imview(raster={'file':'%s.image' % (imagename), 'colorwedge':True},
           out='%s.image.png' % (imagename)
#          , contour={'file':'%s.flux.pbcoverage' % (imagename),
#                    'levels':[halfpower]}
           )
    imview(raster={'file':'%s.psf' % (imagename), 'colorwedge':True},
           out='%s.psf.png' % (imagename)
#           ,contour={'file':'%s.flux.pbcoverage' % (imagename),
#                    'levels':[halfpower]}
           )
    print "Visibility data left in: %s/%s" % (project,vis)
    print "Images left in: %s.image and %s.psf" % (imagename,imagename)
    print "Pngs left in: %s.image.png and %s.psf.png" % (imagename,imagename)
        
def antposcorrmm(parameter=[], antenna='', sort=''):
    """
    Converts a list of antenna position corrections into human-readable
    millimeters.
    sort='name': will sort the output by antenna name
    sort='magnitude': will sort the output by the magnitude of the correction
    Todd Hunter
    """
    if (type(parameter) == list):
        if (len(parameter) < 3):
            print "Invalid parameter"
            return
    elif (type(parameter) == str):
        strings = parameter.split(',')
        parameter = map(float, strings)
    else:
        print "Parameter must be a list of floats, or a string list of floats"
        return
    if (type(antenna) == list):
        if (len(antenna) < len(parameter)/3):
            print "Mismatch between number of parameters (%d) and antennas (%d)" % (len(parameter), len(antenna))
            return
    elif (type(antenna) == str):
        if (antenna != ''):
            antstrings = antenna.split(',')
            if (len(antstrings) < len(parameter)/3):
                print "Mismatch between number of parameters (%d) and antennas (%d)" % (len(parameter), len(antstrings))
                return
        elif (sort != ''):
            print "You cannot sort if no antenna list is provided."
            return
    else:
        print "If specified, antenna must be a list of strings, or a string list of strings"
        return
    parameter = 1000*np.array(parameter)  # convert to mm
    totals = []
    if (antenna != ''):
        print "Antenna  X      Y      Z       Total"
    else:
        print "  X      Y      Z       Total"

    if (sort.find('name')>=0 or sort.find('antenna')>=0):
        sortedAntennas = []
        for s in antstrings:
            sortedAntennas.append(s)
        sortedAntennas.sort()
    elif (sort.find('mag')>=0):
        sortedAntennas = []
        sortedTotals = []
        for i in range(len(parameter)/3):
            total = (parameter[i*3]**2+parameter[i*3+1]**2+parameter[i*3+2]**2)**0.5
            sortedTotals.append(total)
        indices = np.argsort(sortedTotals)
        sortedAntennas = list(np.array(antstrings)[indices])
    elif (sort != ''):
        print "Unrecognized sorting scheme: ", sort
        sort = ''
    for j in range(len(parameter)/3):
        if (sort != ''):
            i = antstrings.index(sortedAntennas[j])
        else:
            i = j
        total=(parameter[i*3]**2+parameter[i*3+1]**2+parameter[i*3+2]**2)**0.5
        totals.append(total)
        outline = ''
        if (antenna != ''):
            outline += antstrings[i] + "  " # antstrings[i] + "  "
        outline += "%+.3f %+.3f %+.3f     %.3f mm" % (parameter[i*3],
                       parameter[i*3+1], parameter[i*3+2], total)
        print outline
    print "Abs(min), Abs(max) = %.3f/%.3f mm,  Median = %.3f mm   st.dev = %.3f mm" % (np.min(np.abs(totals)),np.max(np.abs(totals)),np.median(totals),np.std(totals))

def locateSciencePath(pathEnding):
        # This method will locate any file in the active analysisUtils
        # "science" subdirectory tree. You need to call it with the
        # trailing part of the path beyond the ~/AIV/science/ directory.
        # e.g. 'PadData/almaAntPos.txt'
        #
        tokens = __file__.split('/')
        mypath = ''
        for i in range(len(tokens)-len(pathEnding.split('/'))):
            mypath += tokens[i] + '/'
        mypath += pathEnding
        return(mypath)
        
def checkAntennaPositionFiles(historyLength=6, antenna=''):
    """
    Shows how recently the antenna position files have been updated
    by displaying the most recent entries.    It finds these files
    (antennaMoves.txt and almaAntPos.txt) in the same directory tree
    as your analysisUtils was found.
    Optional: supply an antenna (or pad) name, and the antenna moves output 
              will be restricted to that antenna (or pad).
    -- Todd Hunter
    """
    mypath = locateSciencePath('PadData/antennaMoves.txt')
    print "Recent antenna moves (assumed to be in chronological order):"
    if (antenna == ''):
        os.system('tail -%d %s' % (historyLength,mypath))
    else:
        os.system('grep %s %s | tail -%d' % (antenna,mypath,historyLength))
    mypath = locateSciencePath('PadData/almaAntPos.txt')
    dates = []
    print "Recent baseline tracks (sorted by measTime):"
    for c in commands.getoutput('grep measTime %s' % mypath).split('\n'):
        dates.append(c.split()[1])
    dates.sort()
    for c in dates[-historyLength:]:
        print c
    
def sensitivity(freq, bandwidth, etime, elevation=None, pwv=None, 
#                telescope=None, diam=None, nant=None, 
                antennalist=None, doimnoise=None, mode='tsys-atm',
                integration=None, debug=None, t_amb=270, airmass=None,
                tau=None, tsys_only=False, t_rx=None, h0=2.0, 
                tau0=None):
    """
    This is a wrapper for simutil.sensitivity, for those who don't know 
    about it.  For ALMA, it also calculates the Tsys.
    -Todd Hunter

    Parameters:
    freq:      value in GHz, or string with units
    bandwidth: value in GHz, or string with units
    time:      string with units of seconds
    elevation: value in degrees, or string with units
    airmass:   airmass (alternative to specifying the elevation)
        (if neither elevation nor airmass is specified, then zenith is used)
    pwv:         value in mm
    antennalist: telescope antenna configuration file
    domimnoise:  default is None
    integration: length of individual integration (string with units)
    debug: T/F
    mode:      'tsys-atm' (default) or 'tsys-manual'
         for tsys-atm, also specify pwv
         for tsys-manual, also specify tau0 & t_sky (as t_atmosphere, e.g. 260K)
    tau:       opacity to use (instead of the atm model result)
    tau0:      zenith opacity to use (instead of the atm model result)
    t_amb:     physical temperature of atmosphere in K
    tsys_only: T/F, T=only compute and return the Tsys, not the sensitivity
    t_rx:      receiver temperature to use (instead of tabulated value)
    h0:        water vapor scale height in km (default=2.0)

    Examples:
    au.sensitivity('660GHz', '3.75GHz', '60s', elevation='80deg', pwv=0.472,
            antennalist = 'alma_cycle1_2.cfg')
    au.sensitivity('22GHz', '1.0GHz', '60s', elevation='80deg', pwv=5,
            antennalist= 'vla.a.cfg')
    """
    # Check casa version
    casaVersionString = casalog.version()
    if (int(casalog.version().split()[2].split('.')[1]) > 3 and tsys_only==False):
        print "This function does not yet work in %s due to the cal table format" % casaVersionString
        print "change, unless tsys_only=True."
        return
    if (tau != None and tau0 != None):
        print "You can only specify one of: tau and tau0 (zenith)"
        return
    if (pwv==None and mode != 'tsys-manual'):
        print "You need to specify a PWV."
        return
    if (elevation==None and airmass==None):
        elevation = 90
    if (elevation != None):
        elevation = float(str(elevation).split('deg')[0])
        if (airmass != None):
            print "Ignoring airmass because elevation was specified"
        airmass = 1/np.sin(elevation*np.pi/180.)
    else:
        if (airmass < 1 or airmass==None):
            print "Invalid airmass"
            return
        elevation = 180*math.asin(1/airmass)/np.pi
    print "elevation = %.2f, airmass = %.3f" % (elevation,airmass)
    elevation = '%fdeg' % elevation
    if (antennalist != None):
        if (antennalist.find('.cfg') < 0):
            antennalist += '.cfg'
        if (os.path.exists(antennalist) == False):
            repotable=os.getenv("CASAPATH").split()[0]+"/data/alma/simmos/"
            antennalist = repotable + antennalist
    elif (tsys_only == False):
        print "You need to provide an antennalist parameter, unless tsys_only=True"
        return
    telescope=None
    diam=None
    nant=None
    su = simutil.simutil()
    if (type(freq) == float):
        freq = '%fGHz' % freq
    elif (type(freq) == int):
        freq = '%dGHz' % freq

    if (type(bandwidth) == float):
        bandwidthGHz = bandwidth
        bandwidth = '%fGHz' % bandwidth
    elif (type(bandwidth) == int):
        bandwidthGHz = bandwidth
        bandwidth = '%dGHz' % bandwidth
    else:
        if (bandwidth.find('GHz') >= 0):
            bandwidthGHz = float(bandwidth.split('GHz')[0])
        elif (bandwidth.find('MHz') >= 0):
            bandwidthGHz = float(bandwidth.split('MHz')[0])*1e-3
        elif (bandwidth.find('kHz') >= 0):
            bandwidthGHz = float(bandwidth.split('kHz')[0])*1e-6
        elif (bandwidth.find('Hz') >= 0):
            bandwidthGHz = float(bandwidth.split('Hz')[0])*1e-9
        else:
            bandwidthGHz = float(bandwidth)
    casaVersion = int(casalog.version().split()[4][1:-1])
    if (tsys_only == False):
        if (casaVersion >= 20186):
            jansky = simutil.simutil.sensitivity(su, freq, bandwidth, etime, elevation,
                                             pwv, telescope, diam, nant,
                                             antennalist, doimnoise, integration,
                                             debug, mode, tau0, t_amb)
        else:
            if (tau0 != None or mode != 'tsys-atm'):
                print "The tau0 and mode options only work for casa version >= 20186"
                return
            jansky = simutil.simutil.sensitivity(su, freq, bandwidth, etime, elevation,
                                             pwv, telescope, diam, nant,
                                             antennalist, doimnoise, integration,
                                             debug)

    if (antennalist == None):
        thisIsAlma = False
    else:
        thisIsAlma = (antennalist.lower().split('/')[-1].find('alma') >= 0) \
            or (antennalist.lower().split('/')[-1].find('aca') >= 0)
    # Compute wavelength in microns
    if (freq.find('GHz') >= 0):
        wavelength = 1e4*c/(float(freq.split('GHz')[0])*1e9)
    elif (freq.find('MHz') >= 0):
        wavelength = 1e4*c/(float(freq.split('MHz')[0])*1e6)
    elif (freq.find('kHz') >= 0):
        wavelength = 1e4*c/(float(freq.split('kHz')[0])*1e3)
    elif (freq.find('Hz') >= 0):
        wavelength = 1e4*c/float(freq.split('Hz')[0])
    else:
        wavelength = 1e4*c/(float(freq)*1e9)
        freq = '%sGHz' % (freq)

    frequencyGHz = 1e-5*c/wavelength
    if (thisIsAlma and t_rx==None):
        telescope = "ALMA"
        su.telescopename = telescope  # prevents crash of noisetemp()
        (eta_p, eta_s, eta_b, eta_t, eta_q, t_rx) = simutil.simutil.noisetemp(su, telescope, freq, diam)
        print "Frequency: %f GHz    Bandwidth: %.9f GHz" % (frequencyGHz,bandwidthGHz)
        if (wavelength > 0):
            microns = (-wavelength**2*np.log(eta_p)/(16*np.pi**2))**0.5
            print "Antenna Ruze efficiency:          eta_p=%.4f (%.1f um rms)" % (eta_p,microns)
        else:
            print "Antenna Ruze efficiency:          eta_p=%.4f" % (eta_p)
        print "Forward efficiency:               eta_s=%.4f" % (eta_s)
        print "Subreflector blockage efficiency: eta_b=%.4f" % (eta_b)
        print "Illumination efficiency:          eta_t=%.4f" % (eta_t)
        print "Aperture efficiency:              eta_p*s*b*t = eta_a=%.4f" % (eta_p*eta_s*eta_b*eta_t)
        print "Correlator quantum efficiency:    eta_q=%.4f" % (eta_q)
        t_rx_casa = t_rx
        if (frequencyGHz > 600):
            g = 1
            if (frequencyGHz > 750):
                if (t_rx > 300):
                    t_rx *= 0.5
            elif (t_rx > 160):
                t_rx *= 0.5
        else:
            g = 0
        print "Using tabulated receiver temperature: T_rx = %.1fK" % (t_rx)
    else:
        eta_s = 0.95
        if (frequencyGHz > 600):
            g = 1
        else:
            g = 0
        if (t_rx != None):
            print "Using user-specified receiver temperature: T_rx = %.1fK" % (t_rx)
        else:
            print "T_rx is unknown. You must specify either the value or the telescope (via the antennalist parameter)."
            return

    if (thisIsAlma or tsys_only==True):
        elevationDegrees = float(elevation.split('deg')[0])
        airmass = 1.0/math.sin(elevationDegrees*math.pi/180.)
        if (mode != 'tsys-manual'):
            my_tau, my_t_sky, my_t_sky_RJ = estimateALMAOpacity(pwv,frequencyGHz,airmass,h0,verbose=False)
            print "Computed tau=%f, tau0=%f, t_sky=%f (airmass=%f)" % (my_tau, my_tau/airmass, 
                                                                   my_t_sky,airmass)
        if (tau == None and tau0 == None):
            tau = my_tau
            tau0 = my_tau/airmass
        elif (tau == None):
            tau = tau0*airmass
        else:
            tau0 = tau/airmass
        if (mode != 'tsys-manual'):
            t_sky = my_t_sky
            t_sky_RJ = my_t_sky_RJ
        else:
            t_sky = t_amb*(1-np.exp(-tau))
            t_sky_RJ = 0
            
        print "   Using tau=%f" % tau
        print "         tau0=%f" % tau0
        print "         t_amb=%f" % t_amb
        print "         t_sky=%f" % t_sky
        print "             g=%f" % g
        print "         eta_s=%f" % eta_s
        print "          t_rx=%f" % (t_rx)
        tsys = (1+g)*(t_rx + t_sky*eta_s + (1-eta_s)*t_amb) / \
                   (eta_s*np.exp(-tau*airmass))
        if (t_sky_RJ == 0):
            print "Tsys_correct = %.2f K,  Tsky = %.2f K" % (tsys, t_sky)
        else:
            print "Tsys_correct = %.2f K,  Tsky = %.2f K (Rayleigh-Jean = %.2f K)" % (tsys, t_sky, t_sky_RJ)
        tsys_casa = (t_rx_casa + t_sky*eta_s + (1-eta_s)*t_amb) / \
                   (eta_s*np.exp(-tau*airmass))
        tsys_casa2 = (t_rx_casa) / \
                   (eta_s*np.exp(-tau*airmass))
        if (casaVersion < 20000):
            print "Tsys_casa3.4 = %.2f K,  Tsky = %.2f K" % (tsys_casa, t_sky)
            print "Ratio (Tsys_correct/Tsys_casa) = %f" % (tsys /tsys_casa)

    if (tsys_only == False):
        if (jansky > 0.1):
            print "Sensitivity = %g Jy" % (jansky)
        elif (jansky > 1e-4):
            print "Sensitivity = %g mJy" % (jansky*1000)
        elif (jansky > 1e-7):
            print "Sensitivity = %g uJy" % (jansky*1000000)
        else:
            print "Sensitivity = %g nJy" % (jansky*1000000000)
        return(jansky)
    else:
        return(tsys)

def plotAtmosphere(pwv=None,frequency=[0,1000],bandwidth=None,telescope='ALMA',
                   temperature=None, altitude=None, latitudeClass=None,
                   pressure=None, humidity=None, numchan=1000, airmass=1.0,
                   elevation=0, figfile='', plotrange=None,
                   quantity='transmission', h0=None):
    """
    Simple plotter of atmospheric transmission. The units for input are:
    pwv in mm
    frequency in GHz (either a single value, or a tuple for the range)
    bandwidth in GHz (only used if frequency is a single value)
    temperature in K
    altitude in m
    latitudeClass ('tropical', 'midLatitudeWinter'(default), or 'midLatitudeSummer')
    pressure in mbar
    humidity in percentage
    elevation in degrees
    plotrange (for y axis)
    telescope (if not '', then apply nominal values for 'ALMA' or 'EVLA')
    quantity ('transmission', 'opacity', or 'tsky')
    h0 (scale height of H2O in km)

    Todd Hunter
    """
    if (elevation > 0):
        airmass = 1/np.sin(elevation*np.pi/180.)
    else:
        if (airmass < 1):
            print "Invalid airmass"
            return
        elevation = 180*math.asin(1/airmass)/np.pi
    tropical = 1
    midLatitudeWinter = 3
    if (latitudeClass == None):
        latitudeClass = midLatitudeWinter
    if (telescope == 'ALMA'):
        if (temperature == None):
            temperature = 270
        if (altitude == None):
            altitude = 5059
        if (pressure == None):
            pressure = 563
        if (humidity == None):
            humidity = 20
        if (pwv == None):
            pwv = 1.0
        if (h0 == None):
            h0 = 2.0
    elif (telescope.find('VLA') >= 0):
        if (temperature == None):
            temperature = 280
        if (altitude == None):
            altitude = 2124
        if (pressure == None):
            pressure = 785.5
        if (humidity == None):
            humidity = 20
        if (pwv == None):
            pwv = 5.0
        if (h0 == None):
            h0 = 2.0
    elif (telescope.find('SMA') >= 0):
        latitudeClass = tropical
        if (temperature == None):
            temperature = 280
        if (altitude == None):
            altitude = 4072
        if (pressure == None):
            pressure = 629.5
        if (humidity == None):
            humidity = 20
        if (pwv == None):
            pwv = 1.0
        if (h0 == None):
            h0 = 2.0
    else:
        if (temperature == None or altitude==None or pressure==None or humidity == None):
            print "If telescope is not specified, then you must specify pwv, temperature,"
            print " altitude, barometric pressure and relative humidity."
            return
        if (h0 == None):
            h0 = 2.0

    if (type(frequency) == list):
        frequencyRange = frequency
    elif (bandwidth>0):
        if (numchan > 1):
            frequencyRange = [frequency-0.5*bandwidth, frequency+0.5*bandwidth]
        else:
            frequencyRange = [frequency]
    else:
        print "Bandwidth must be non-zero if frequency is a single value"
        return
    reffreq = np.mean(frequencyRange)
    chansep = (np.max(frequencyRange)-np.min(frequencyRange))/(1.0*numchan)
    if (numchan > 1):
        freqs = np.arange(frequencyRange[0], frequencyRange[1], chansep)
    else:
        freqs = frequencyRange
    fCenter = casac.Quantity(reffreq,'GHz')
    if (numchan > 1):
        fResolution = casac.Quantity(chansep,'GHz')
        fWidth = casac.Quantity(numchan*chansep,'GHz')
    else:
        fResolution = casac.Quantity(bandwidth,'GHz')
        fWidth = casac.Quantity(bandwidth,'GHz')
    nbands = 1
#    print "center=%f, resolution=%f, bandwidth=%f" % (fCenter.value,fResolution.value,fWidth.value)
    at.initAtmProfile(humidity=humidity,temperature=casac.Quantity(temperature,"K"),
                      altitude=casac.Quantity(altitude,"m"),
                      pressure=casac.Quantity(pressure,'mbar'),
                      h0=casac.Quantity(h0,"km"),
                      atmType=latitudeClass)
    at.initSpectralWindow(nbands,fCenter,fWidth,fResolution)
    at.setUserWH2O(casac.Quantity(pwv,'mm'))

    dry = np.array(at.getDryOpacitySpec(0)['dryOpacity'])
    wet = np.array(at.getWetOpacitySpec(0)['wetOpacity'].value)
    try:
       TebbSky = at.getTebbSkySpec(spwid=0).value
    except:
#       print "Getting Tsky, channel by channel"
       TebbSky = []
       for chan in range(numchan):
           TebbSky.append(at.getTebbSky(nc=chan, spwid=0).value)
       TebbSky = np.array(TebbSky)
    opacity = airmass*(wet+dry)
    transmission = np.exp(-opacity)
    TebbSky *= (1-np.exp(-airmass*(wet+dry)))/(1-np.exp(-wet-dry))
    pb.clf()
    desc = pb.subplot(111)
    if (quantity == 'opacity'):
        if (numchan > 1):
            pb.plot(freqs, opacity)
        else:
            pb.plot(freqs, opacity, 'bo')
        pb.ylabel('Opacity')
        if (plotrange != None):
            pb.ylim(plotrange) 
        else:
            pb.plot(freqs, opacity, 'bo')
    elif (quantity == 'transmission'):
        if (numchan > 1):
            pb.plot(freqs, transmission)
        else:
            pb.plot(freqs, transmission, 'bo')
        pb.ylabel('Transmission')
        if (plotrange == None):
            plotrange = [0,1]
        pb.ylim(plotrange)
    elif (quantity == 'tsky'):
        if (numchan > 1):
            pb.plot(freqs, TebbSky)
        else:
            pb.plot(freqs, TebbSky, 'bo')
        pb.ylabel('Sky temperature (K)')
        if (plotrange != None):
            pb.ylim(plotrange)
    else:
        print "Unrecognized quantity: %s" % (quantity)
        return
    pb.xlabel('Frequency (GHz)')
    desc.xaxis.grid(True,which='major')
    desc.yaxis.grid(True,which='major')
    title = ''
    if (telescope != ''):
        title += '%s: ' % telescope
    pb.title('%sPWV=%.3fmm, alt.=%.0fm, h0=%.2fkm, elev=%.0fdeg, T=%.0fK, P=%.0fmb, H=%.0f%%' % (title,pwv,altitude,h0,elevation,temperature,pressure,humidity), size=12)
    pb.draw()
    h = 6.626e-27
    k = 1.38e-16
    hvkT = h*np.mean(freqs)*1e9/(k*np.mean(TebbSky))
    J = hvkT / (np.exp(hvkT) - 1)
    print "Mean opacity = %.4f, transmission = %.4f, Tsky = %.4f, Tsky_planck=%g" % (np.mean(opacity), np.mean(transmission), np.mean(TebbSky), np.mean(TebbSky) * J)
    if (figfile != ''):
        pb.savefig(figfile)


def scaleImage(myimage, boxes=[], factors=[]):
    """
    Reads in a CASA single-plane image, and scales one or more rectangular
    regions by a scale factor.
    Example:  au.scaleImage('B9scaled.image',[[147,91,181,123]],[50])
    Todd Hunter
    """
    if (len(boxes) < 1 or len(factors) < 1):
        print "You must specify at least one box and factor"
        return
    for b in range(len(boxes)):
        if (boxes[b] == []):
            continue
        if (len(boxes[b]) < 4):
            print "Box %d must have 4 values (blc_x, blc_y, trc_x, trc_y)"
            return
    if (os.path.exists(myimage) == False):
        print "Image does not exist = ", myimage
        return
    ia.open(myimage)
    i = ia.getregion()
    previousMax = np.max(i)
    for b in range(len(boxes)):
        if (boxes[b] == []):
            print "Scaling whole image"
            box = [0, 0, len(i),len(i[0])]
        else:
            box = boxes[b]
        i[box[0]:box[2], box[1]:box[3]] *= factors[b]
    newMax = np.max(i)
    print "Prior max=%f,  new max=%f" % (previousMax, newMax)
    ia.putregion(i)
    ia.close()
