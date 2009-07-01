import os
import casac
import numpy
import math
#################################################
# SDImaging.py                                  #
# Single dish imaging using casapy imager tool  #
# last modified: 2008-09-19   (T.T)             #
#################################################
# example paramters:
# sdMS = 'M16-CO43mod.ASAP.baseline.MS'
# outImage = 'M16-CO43mod.image1'
# mapCenter = 'J2000 18:18:52 -13.49.40'
# nchan = 40
# startchan = 1800
# chanstep = 2
# field = 0
# ngridx=24
# ngridy=24
# cellx = '7arcsec'
# celly = '7arcsec'

#data='M16-CO43mod.ASAP.baseline.MS'
#im.open('M16-CO43.ASAP.baseline.MS')
#srcdir='J2000 18:18:52 -13.49.40'
#srcdir='J2000 18:39:08.6 -13:49:40'

def getImParams(sdMS, Lines, radius, spacing, singleimage=False):
    """
    Determine imaging parameters from the inputs to the SDpipeline script
    and processed line detection parameters.
    Use as an internal method for SDpipeline or can be used
    independently with SDbookKeeper if the processed data exists
    """
    #hardcoded parameter 
    # number of channel maps (defined in SDPlotter.py) to have
    # the same channel width for each channel map
    NChannelMap=15
    
    # determine map center
    tbtool = casac.homefinder.find_home_by_name('tableHome') 
    tb = tbtool.create()
    tb.open(sdMS+'/POINTING')
    dircol=tb.getcol('DIRECTION')
    measinfo=tb.getcolkeywords('DIRECTION')
    tb.close()
    (l,n,m) = dircol.shape
    if n == 1: dircol = dircol.reshape(l,m)
    ras = dircol[0]
    decs = dircol[1]
    dra = ras.max() - ras.min()
    ddec = decs.max() - decs.min()
    racen = ras.min() + dra/2.0
    deccen = decs.min() + ddec/2.0
    epoch = measinfo['MEASINFO']['Ref']
    mapCenter = epoch+' '+str(racen)+'rad '+str(deccen)+'rad'

    #RA size should be corrected with DEC
    DecCorrection = 1.0 / math.cos(deccen)
    #### GridSpacingRA = GridSpacing * DecCorrection

    #grid sizes
    GridSpacing = spacing * numpy.pi/180.
    rradius = radius * numpy.pi/180.
    #NGridRA = int(dra/GridSpacing) + 2
    NGridRA = int((dra+2*rradius)/(GridSpacing*DecCorrection)) + 1
    #NGridDec = int(ddec/GridSpacing) + 2
    NGridDec = int((ddec+2*rradius)/GridSpacing) + 1
    #cellsize = str(spacing) + 'deg'
    cellsizeArcsec = int(spacing * 3600.0)+1
    cellsize = str(cellsizeArcsec) + 'arcsec'

    tb.open(sdMS)
    datacol=tb.getcol('FLOAT_DATA')
    (np,nchan,rws) = datacol.shape
    tb.close()
    #channel selections
    # use predefined  no. channel maps
    # determine line region(s)
    # determine start channel from line region info + extra channels
    # determine width (-> step, nchan)
    # Lines to get line regions
    Ncluster = len(Lines)
    Lines=Lines
    startchans = [] 
    endchans = []
    nchans = []
    chansteps= []
    # channel parameters for each of the clusters
    for nc in range(Ncluster):
	if Lines[nc][2] !=True: continue
	chanc=int(Lines[nc][0] + 0.5)
	#chanhw=max(int(Lines[nc][1]*1.4/2.0),1)
	chanstep=max(int(Lines[nc][1]*1.4/NChannelMap+0.5),1)
        chan0=int(chanc-NChannelMap/2.0*chanstep)
	#chan0=max(chanc-chanhw,0)
	#chan1=min(chanc+chanhw,nchan)
	startchans.append(chan0)
        Nchan=NChannelMap
        if chan0 < 0:
           shift=-chanc/chanstep+1
           Nchan=max(NChannelMap-shift*2,1)
	nchans.append(Nchan)
        chansteps.append(chanstep)
        endchans.append(chan0+chanstep*Nchan)
    if singleimage:
	startchans = min(startchans)
        chansteps = max(chansteps)
        nchans = int((max(endchans)-startchans)/chansteps +0.5)
    del tbtool, tb
    return mapCenter, NGridRA, NGridDec, cellsize, startchans, nchans, chansteps


def MakeImage(sdMS, outImagename=None, ngridx=100, ngridy=100, cellx='1arcsec', celly='1arcsec', mapCenter=None, nchan=1, startchan=0, chanstep=1, field=0, convsupport=5, spw=0, moments=[], showImage=True, LogLevel=2, LogFile=False): 
    """
    Make an single dish image using CASA imager tool
    """
    imtool = casac.homefinder.find_home_by_name('imagerHome')
    iatool = casac.homefinder.find_home_by_name('imageHome')
    im = imtool.create()
    ia = iatool.create()
    if outImagename is None:
        msg='outImagename is undefined'
        raise Exception, msg
    if os.path.isdir(outImagename):
        print '%s exists, overwrite...' % outImagename
        os.system('rm -rf %s' % outImagename)

    im.open(sdMS)
    im.selectvis(nchan=nchan,start=startchan,step=chanstep,field=field,spw=spw)
    im.defineimage(nx=ngridx,ny=ngridy,cellx=cellx, celly=celly, phasecenter=mapCenter,mode='channel',start=startchan,nchan=nchan,step=chanstep)
    #im.setoptions(ftmachine='sd', cache=1000000000)
    # 2009/2/5 invalid keyword applypointingcorrections for casapy-23.1.6826
    #im.setoptions(ftmachine='sd', applypointingoffsets=False, applypointingcorrections=False)
    im.setoptions(ftmachine='sd', applypointingoffsets=False)
    #im.setoptions(ftmachine='sd')
    im.setsdoptions(convsupport=convsupport) # set convolution support to 5 pixels (default)
    im.makeimage(type='singledish',image=outImagename)
    im.close()
    print 'Image, %s, was created' % outImagename

    if type(moments)==list and len(moments) >0:
	ia.open(outImagename)
	ia.moments(moments=moments, overwrite=True)
        print "Moment image(s) were created"
    if showImage:
	from viewer_cli import  viewer_cli as viewer
	viewer(outImagename)
    del imtool, im, iatool, ia
    return


def MakeImages(sdMS, outImagename=None, ngridx=100, ngridy=100, cellx='1arcsec', celly='1arcsec', mapCenter=None, nchans=[], startchans=[], chansteps=[], field=0, convsupport=5, spw=0, moments=[], showImage=False, LogLevel=2, LogFile=False):
    """
    Generate multiple single dish images 
    """
    if outImagename is None:
        outImagename=sdMS.rstrip('.baseline.MS')
    for i in range(len(nchans)):
	outImagename2 = outImagename+'.'+str(i)+'.image' 
        print outImagename2
	MakeImage(sdMS, outImagename2, ngridx, ngridy, cellx, celly, mapCenter, nchans[i], startchans[i], chansteps[i], field, convsupport, spw, moments, showImage,LogLevel, LogFile)
    return
