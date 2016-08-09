#*******************************************************************************
# ALMA - Atacama Large Millimeter Array
# Copyright (c) NAOJ - National Astronomical Observatory of Japan, 2011
# (in the framework of the ALMA collaboration).
# All rights reserved.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#*******************************************************************************
#
# $Revision: 1.15.2.3.2.1 $
# $Date: 2012/09/03 14:55:15 $
# $Author: tnakazat $
#
import SDTool as SDT
from SDTool import SDLogger, dec_engines_logging
from SDEngine import NoData
from SDPlotter import NChannelMap
#import casac
from taskinit import gentools
from rmtables import rmtables
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
#self._im.open('M16-CO43.ASAP.baseline.MS')
#srcdir='J2000 18:18:52 -13.49.40'
#srcdir='J2000 18:39:08.6 -13:49:40'

# 2010/6/10 TN  defined as a class
stokestype={0: 'Undefined',
            1: 'I',
            2: 'Q',
            3: 'U',
            4: 'V',
            5: 'RR',
            6: 'RL',
            7: 'LR',
            8: 'LL',
            9: 'XX',
            10: 'XY',
            11: 'YX',
            12: 'YY'}
            
class SDImaging( SDLogger ):
    """
    """
    def __init__( self, sdMS, edge=[], LogLevel=2, ConsoleLevel=3, LogFile=False ):
        """
        Constructor
        
        sdMS: input MS data to be imaged
        edge: channel edge
        """
        # local tool
        #self._tbtool = casac.homefinder.find_home_by_name('tableHome')
        #self._tb = self._tbtool.create()
        #self._metool = casac.homefinder.find_home_by_name('measuresHome')
        #self._me = self._metool.create()
        #self._iatool = casac.homefinder.find_home_by_name('imageHome')
        #self._ia = self._iatool.create()
        #self._imtool = casac.homefinder.find_home_by_name('imagerHome')
        #self._im = self._imtool.create()
        self._tb,self._me,self._ia,self._im = gentools(['tb','me','ia','im'])
        
        SDLogger.__init__( self, level=LogLevel, consolelevel=ConsoleLevel, origin='SDImaging', fileout=LogFile, Global = True )
        self.sdMS = sdMS
        self.mapCenter = None
        self.nGridX = 0
        self.nGridY = 0
        self.cellSize = 0
        self.startChans = []
        self.nChans = []
        self.chanSteps = []
        self.edge = edge
        self.spwIds = []
        self.spwChans = []
        self.lineImages = []
        self.stokes = 'I'

        self.coords = None

    def __del__( self ):
        """
        """
        #del self._tbtool
        del self._tb
        #del self._metool
        del self._me
        #del self._iatool
        del self._ia
        #del self._imtool
        del self._im
        

    def setSpatialPixelNum( self, nx, ny ):
        self.nGridX = nx
        self.nGridY = ny

    def setSpatialPixelWidth( self, width ):
        self.cellSize = width

    def setMapCenter( self, x, y, epoch='J2000' ):
        if type(x) != str:
            x = '%srad'%x
        if type(y) != str:
            y = '%srad'%y
        self.mapCenter = '%s %s %s'%(epoch,x,y)

    def getImParams(self, spwid, Lines, radius, spacing, singleimage=False, LogLevel=2, LogFile=False):
        """
        Determine imaging parameters from the inputs to the SDpipeline script
        and processed line detection parameters.
        Use as an internal method for SDpipeline or can be used
        independently with SDbookKeeper if the processed data exists
        """
        origin = 'getImParams()'
        #self.setFileOut( LogFile )
        
        import math
        import numpy
        #hardcoded parameter 
        # number of channel maps (defined in SDPlotter.py) to have
        # the same channel width for each channel map
        #NChannelMap=15

        # determine map center
        self._tb.open(self.sdMS)
        #dirtab=self._tb.getkeyword('POINTING').split()[-1]
        #obstab=self._tb.getkeyword('OBSERVATION').split()[-1]
        #ddtab=self._tb.getkeyword('DATA_DESCRIPTION').split()[-1]
        #poltab=self._tb.getkeyword('POLARIZATION').split()[-1]
        dirtab=self._tb.getkeyword('POINTING').lstrip('Table: ')
        obstab=self._tb.getkeyword('OBSERVATION').lstrip('Table: ')
        ddtab=self._tb.getkeyword('DATA_DESCRIPTION').lstrip('Table: ')
        poltab=self._tb.getkeyword('POLARIZATION').lstrip('Table: ')
        self._tb.close()
        self._tb.open(dirtab)
        dircol = self._tb.getcol('DIRECTION')
        timecol = self._tb.getcol('TIME')
        tmean = timecol.mean()
        measinfo = self._tb.getcolkeywords('DIRECTION')
        self._tb.close()
        epoch = measinfo['MEASINFO']['Ref']
        raunit = measinfo['QuantumUnits'][0]
        decunit = measinfo['QuantumUnits'][1]
        if epoch != 'J2000':
            self._tb.open(obstab)
            antname=self._tb.getcell('TELESCOPE_NAME',0)
            self._tb.close()
            siteName=None
            if (antname.find('APEX') != -1):
                siteName = 'APEX'
            elif (antname.find('GBT') != -1):
                siteName = 'GBT'
            elif (antname.find('NRO') != -1):
                siteName = 'NRO'
            elif (antname.find('ASTE') != -1):
                siteName = 'ASTE'
            elif ((antname.find('ALMA') != -1)
                  or (antname.find('OSF') != -1)
                  or (antname.find('DV') != -1)
                  or (antname.find('PM') != -1)):
                sitename = 'ALMA'
            elif (antname.find('30M-') != -1):
                siteName = 'IRAM_30m'
            else:
                siteName = antname
            for idir in xrange(dircol.shape[2]):
                self._me.doframe(self._me.epoch('utc','%ss'%(timecol[idir])))
                #print self._me.epoch('utc','%sd'%(timecol[idir]/86400.0))
                self._me.doframe(self._me.observatory(siteName))
                dir0=self._me.direction(epoch,str(dircol[0][0][idir])+raunit,str(dircol[1][0][idir])+decunit)
                dir1=self._me.measure(dir0,'J2000')
                #print dir1
                dircol[0][0][idir] = dir1['m0']['value']
                dircol[1][0][idir] = dir1['m1']['value']
                self._me.done()
            raunit = 'rad'
            decunit = 'rad'
            epoch = 'J2000'
        dra = dircol[0][0].max() - dircol[0][0].min()
        ddec = dircol[1][0].max() - dircol[1][0].min()
        racen = dircol[0][0].min() + dra/2.0
        deccen = dircol[1][0].min() + ddec/2.0
##         mapCenter = epoch+' '+str(racen)+raunit+' '+str(deccen)+decunit
        self.mapCenter = epoch+' '+str(racen)+raunit+' '+str(deccen)+decunit

        #RA size should be corrected with DEC
        DecCorrection = 1.0 / math.cos(deccen)
        #### GridSpacingRA = GridSpacing * DecCorrection

        #grid sizes
        GridSpacing = spacing * numpy.pi/180.
        rradius = radius * numpy.pi/180.
        #NGridRA = int(dra/GridSpacing) + 2
##         NGridRA = int((dra+2*rradius)/(GridSpacing*DecCorrection)) + 1
        self.nGridX = int((dra+2*rradius)/(GridSpacing*DecCorrection)) + 1
        #NGridDec = int(ddec/GridSpacing) + 2
##         NGridDec = int((ddec+2*rradius)/GridSpacing) + 1
        self.nGridY = int((ddec+2*rradius)/GridSpacing) + 1
        #cellsize = str(spacing) + 'deg'
        cellsizeArcsec = int(spacing * 3600.0)+1
##         cellsize = str(cellsizeArcsec) + 'arcsec'
        self.cellSize = str(cellsizeArcsec) + 'arcsec'

        #channel selections
        # use predefined  no. channel maps
        # determine line region(s)
        # determine start channel from line region info + extra channels
        # determine width (-> step, nchan)
        # Lines to get line regions
        Ncluster = len(Lines)
##         startchans = [] 
##         endchans = []
##         nchans = []
##         chansteps= []
        self.startChans = []
        self.nChans = []
        self.chanSteps = []
        # channel parameters for each of the clusters
        for nc in range(Ncluster):
            if Lines[nc][2] !=True: continue
            chanc=int(Lines[nc][0] + 0.5)
            #chanhw=max(int(Lines[nc][1]*1.4/2.0),1)
            chanstep=max(int(Lines[nc][1]*1.4/NChannelMap+0.5),1)
            chan0=int(chanc-NChannelMap/2.0*chanstep)
            #chan0=max(chanc-chanhw,0)
            #chan1=min(chanc+chanhw,nchan)
##             startchans.append(chan0)
            self.startChans.append(chan0)
            Nchan=NChannelMap
            if chan0 < 0:
               shift=-chanc/chanstep+1
               Nchan=max(NChannelMap-shift*2,1)
            self.nChans.append(Nchan)
            self.chanSteps.append(chanstep)
##             nchans.append(Nchan)
##             chansteps.append(chanstep)
##             endchans.append(chan0+chanstep*Nchan)
        if singleimage:
##             startchans = 0
##             chansteps = 1
##             nchans = 0
            self.startChans = [0]
            self.chanSteps = [1]
            self.nChans = [1]
##         return self.mapCenter, self.nGridX, self.nGridY, self.cellSize, self.startChans, self.nChans, self.chanSteps

        # determine stokes parameter
        self._tb.open(ddtab)
        spwids=self._tb.getcol('SPECTRAL_WINDOW_ID')
        polids=self._tb.getcol('POLARIZATION_ID')
        self._tb.close()
        polid=None
        for ispw in xrange(len(spwids)):
            if spwids[ispw] == spwid:
                polid = polids[ispw]
                break
        if polid is not None:
            self._tb.open(poltab)
            pols=self._tb.getcell('CORR_TYPE',polid)
            self._tb.close()
            self.stokes=''
            for ipol in pols:
                self.stokes=self.stokes+stokestype[ipol]
        #self.LogMessage('DEBUG',Origin=origin,Msg='self.stokes=%s'%self.stokes)

    def MakeImage(self, outImagename=None, windowId=-1, field=0, convsupport=5, spw=0, moments=[], showImage=True, LogLevel=2, LogFile=False): 
        """
        Make an single dish image using CASA imager tool
        """
        origin = 'MakeImage()'
        #self.setFileOut( LogFile )

        startchan = 0
        nchan = 1
        chanstep = 1
        if windowId == -1:
            startchan = self.edge[0]
            nchan = self.spwChans[spw] - self.edge[0] - self.edge[1]
            chanstep = 1
        else:
            if windowId >= len(self.nChans):
                # Error
                return
            else:
                startchan = self.startChans[windowId]
                nchan = self.nChans[windowId]
                chanstep = self.chanSteps[windowId]

        import os
        if outImagename is None:
            msg='outImagename is undefined'
            raise Exception, msg
        if os.path.isdir(outImagename):
            self.LogMessage('INFO', Origin=origin, Msg='%s exists, overwrite...' % outImagename)
            os.system('rm -rf %s' % outImagename)

        self._im.open(self.sdMS)
        self._im.selectvis(nchan=nchan,start=startchan,step=chanstep,field=field,spw=spw)
        self._im.defineimage(nx=self.nGridX,ny=self.nGridY,cellx=self.cellSize, celly=self.cellSize, phasecenter=self.mapCenter,mode='channel',start=startchan,nchan=nchan,step=chanstep,spw=spw,stokes=self.stokes)
        #self._im.setoptions(ftmachine='sd', cache=1000000000)
        # 2009/2/5 invalid keyword applypointingcorrections for casapy-23.1.6826
        #self._im.setoptions(ftmachine='sd', applypointingoffsets=False, applypointingcorrections=False)
        # 2009/6/17 for BoxCar
        # 2010/06/25 default changed to SF (Spheroidal) again 
        self._im.setoptions(ftmachine='sd', applypointingoffsets=False)
        #self._im.setoptions(ftmachine='sd', applypointingoffsets=False, gridfunction='BOX')
        #self._im.setoptions(ftmachine='sd')
        self._im.setsdoptions(convsupport=convsupport) # set convolution support to 5 pixels (default)
        self._im.makeimage(type='singledish',image=outImagename)
        self._im.close()
        self.LogMessage('INFO',Origin=origin,Msg='Image, %s, was created' % outImagename)

        if type(moments)==list and len(moments) >0:
            self._ia.open(outImagename)
            if len(moments) == 1:
                # If len(moments) is 1 and outfile is empty, output image is
                # created only on the memory temporarily. Thus, outfile must
                # be explicitly given to create output image on disk.
                # 2010/02/16 TN
                moutname = outImagename + '.moment%s'%(moments[0])
                mimage = self._ia.moments(moments=moments, outfile=moutname, overwrite=True)
                mimage.close()
                del mimage
            else:
                mimage = self._ia.moments(moments=moments, overwrite=True)
                mimage.close()
                del mimage
            self.LogMessage('INFO',Origin=origin,Msg='Moment image(s) were created')
            self._ia.close()
        if showImage:
            from viewer_cli import  viewer_cli as viewer
            viewer(outImagename)
        return


    def MakeImages(self, outImagename=None, field=0, convsupport=5, spw=0, moments=[], showImage=False, LogLevel=2, LogFile=False):
        """
        Generate multiple single dish images 
        """
        origin = 'MakeImages()'
        #self.setFileOut( LogFile )

        if outImagename is None:
            outImagename=self.sdMS.rstrip('.baseline.MS')
        for i in range(len(self.nChans)):
            outImagename2 = outImagename+'.'+str(i)+'.image'
            self.lineImages.append( outImagename2 )
            print outImagename2
            self.MakeImage(outImagename2, i, field, convsupport, spw, moments, showImage,LogLevel, LogFile)
        return

    def getSpectralWindows( self, IFList=[], LogLevel=2, LogFile=False ):
        """
        Get informations from SPECTRAL_WINDOW table.
        """
        origin = 'getSpectralWindows()'
        #self.setFileOut( LogFile )
        
        import numpy
        self._tb.open(self.sdMS)
        ddids=numpy.unique(self._tb.getcol('DATA_DESC_ID'))
        #ddtab=self._tb.getkeyword('DATA_DESCRIPTION').split()[-1]
        #spwtab=self._tb.getkeyword('SPECTRAL_WINDOW').split()[-1]
        ddtab=self._tb.getkeyword('DATA_DESCRIPTION').lstrip('Table: ')
        spwtab=self._tb.getkeyword('SPECTRAL_WINDOW').lstrip('Table: ')
        self._tb.close()
        self._tb.open(ddtab)
        ids=[]
        for idd in ddids:
            ids.append(self._tb.getcell('SPECTRAL_WINDOW_ID',idd))
        self._tb.close()
        ids=numpy.unique(ids)
        self._tb.open(spwtab)
        nspw=self._tb.nrows()
        nchans=[]
        spwids=[]
        if len(IFList) == 0:
            spwids = ids.tolist()
            for irow in ids:
                nchans.append(self._tb.getcell('NUM_CHAN',irow))
        else:
            for irow in ids:
                if irow in IFList:
                    spwids.append(irow)
                    nchans.append(self._tb.getcell('NUM_CHAN',irow))
        self._tb.close()
        self.spwIds = spwids
        self.spwChans = nchans
        return (spwids,nchans)

    def putChunk( self, image, chunk=None ):
        origin = 'putChunk()'
        
        if type(image) == int:
            image = self.lineImages[image]
        import os
        retval = True
        if chunk == None:
            self.LogMessage( 'WARNING',Origin=origin,Msg='Nothing done.' )
            retval = False            
        elif os.path.exists( image ):
            self._ia.open( image )
            self._ia.putchunk( chunk )
            self._ia.close()
        else:
            self.LogMessage( 'ERROR',Origin=origin,Msg='%s does not exist.'%image )
            retval = False
        return retval 

    def modifyPhaseCenter( self, image, crpix=None, crval=None, cdelt=None, epoch='J2000' ):
        origin = 'modifyPhaseCenter()'

        if type(image) == int:
            image = self.lineImages[image]
        import os
        retval = True
        if crpix == None and crval == None and cdelt == None:
            self.LogMessage( 'WARNING',Origin=origin,Msg='Nothing done.' )
            retval = False
        elif os.path.exists( image ):
            self._tb.open( image, nomodify=False )
            coords = self._tb.getkeyword( 'coords' )
            key = 'direction0'
            if crpix != None:
                coords[key]['crpix'] = crpix
            if crval != None:
                coords[key]['crval'] = crval
            if cdelt != None:
                coords[key]['cdelt'] = cdelt
            self._tb.putkeyword( 'coords', coords )
            self._tb.flush()
            self._tb.close()
        else:
            self.LogMessage( 'ERROR',Origin=origin,Msg='%s does not exist.'%image )
            retval = False
        return retval 
            
        
    def modifySpectralAxis( self, image, restfreq=None, crpix=None, crval=None, cdelt=None, ctype=None ):
        origin = 'modifySpectralAxis()'

        if type(image) == int:
            image = self.lineImages[image]
        import os
        retval = True
        if crpix == None and crval == None and cdelt == None:
            self.LogMessage( 'WARNING',Origin=origin,Msg='Nothing done.' )
            retval = False
        elif os.path.exists( image ):
            self._tb.open( image, nomodify=False )
            coords = self._tb.getkeyword( 'coords' )
            key = 'spectral2'
            key2 = 'wcs'
            if restfreq != None:
                coords[key]['restfreq'] = restfreq
                coords[key]['restfreqs'][0] = restfreqs
            if crpix != None:
                coords[key][key2]['crpix'] = crpix
            if crval != None:
                coords[key][key2]['crval'] = crval
            if cdelt != None:
                coords[key][key2]['cdelt'] = cdelt
            if ctype != None:
                coords[key][key2]['ctype'] = ctype
            self._tb.putkeyword( 'coords', coords )
            self._tb.flush()
            self._tb.close()
        else:
            self.LogMessage( 'ERROR',Origin=origin,Msg='%s does not exist.'%image )
            retval = False
        return retval



class SDImaging2 ( SDLogger ):
    """
    Create image from regridded spectra (numpy array) directly.
    """
    def __init__( self, spfiles=None, contfiles=None, edge=[], LogLevel=2, ConsoleLevel=3, LogFile=False ):
        """
        Constructor
        
        sdMS: input MS data to be imaged
        edge: channel edge
        """
        # local tool
        #self._tbtool = casac.homefinder.find_home_by_name('tableHome')
        #self._tb = self._tbtool.create()
        #self._cstool = casac.homefinder.find_home_by_name('coordsysHome')
        #self._cs = self._cstool.create()
        #self._iatool = casac.homefinder.find_home_by_name('imageHome')
        #self._ia = self._iatool.create()
        #self._metool = casac.homefinder.find_home_by_name('measuresHome')
        #self._me = self._metool.create()
        self._tb,self._cs,self._ia,self._me = gentools(['tb','cs','ia','me'])

        SDLogger.__init__( self, level=LogLevel, consolelevel=ConsoleLevel, origin='SDImaging2', fileout=LogFile, Global = True )
        self.spfiles = spfiles
        self.contfiles = contfiles
        self.mapCenter = None
        self.nchan = 0
        self.nGridX = 0
        self.nGridY = 0
        self.cellSize = 0
        self.startChans = []
        self.nChans = []
        self.chanSteps = []
        self.chanCs = []
        if type(edge) == int or type(edge) == float:
            self.edge = [int(edge),int(edge)]
        else:
            if len(edge) == 0:
                self.edge = edge
            elif len(edge) == 1:
                self.edge = [int(edge[0]),int(edge[0])]
            else:
                self.edge = list(edge[0:2])
                self.edge[0]=int(edge[0])
                self.edge[1]=int(edge[1])
        #self.spwIds = []
        self.spwChans = []
        self.lineImages = []
        self.stokes = 'I'

        self.coordsrec = None
        self.postfix = self.postfix()
        self.imagename = None
        self.contimage = None
        self.lineimages = []
        self.chanwidthvel = 0.0

        self.startChanCont = None
        self.nChanCont = None
        self.chanStepCont = None

        self.isTP = False

        # for data product naming convention
        #self.antennaid = 0
        self.antennaname = 'NONE'
        self.productName = None
        self.poltype = 'none'
        self.spwid = 0 

    def __del__( self ):
        """
        """
        #del self._tbtool
        del self._tb
        #del self._cstool
        del self._cs
        #del self._iatool
        del self._ia
        #del self._metool
        del self._me

    @dec_engines_logging
    def setimparams( self, nx=None, ny=None, cell=None, center=None, lines=None ):
        """
        nx: number of pixels in RA direction
        ny: number of pixels in DEC direction
        cell: pixel width (degree)
        center: center position [RA[deg],DEC[deg]]
        """
        origin='setimparams()'
        
        if nx is not None:
            self.nGridX=nx
        if ny is not None:
            self.nGridY=ny
        if cell is not None:
            if type(cell) == list:
                self.cellSize=['%sdeg'%(-cell[0]),'%sdeg'%(cell[1])]
            else:
                self.cellSize=['%sdeg'%(-cell),'%sdeg'%(cell)]
        if center is not None:
            self.mapCenter='%sdeg %sdeg'%(center[0],center[1])
        if lines is not None:
            if lines == False:
                self.startChans = [0]
                self.chanSteps = [1]
                self.nChans = [1]
            else:
                Ncluster = len(lines)
                self.startChans = []
                self.nChans = []
                self.chanSteps = []
            
                # channel parameters for each of the clusters
                for nc in range(Ncluster):
                    if lines[nc][2] !=True: continue
                    chanc=int(lines[nc][0] + 0.5)
                    chanstep=max(int(lines[nc][1]*1.4/NChannelMap+0.5),1)
                    chan0=int(chanc-NChannelMap/2.0*chanstep)
                    self.startChans.append(chan0)
                    Nchan=NChannelMap
                    if chan0 < 0:
                        shift=-chanc/chanstep+1
                        Nchan=max(NChannelMap-shift*2,1)
                    #if chanstep > 1:
                    #    Nchan = Nchan * chanstep
                    self.nChans.append(Nchan)
                    #self.chanSteps.append(1)
                    self.chanSteps.append(chanstep)
                    self.chanCs.append(chanc)
            #self.LogMessage('DEBUG',Origin=origin,Msg='self.nChans=%s'%self.nChans)
            #self.LogMessage('DEBUG',Origin=origin,Msg='self.startChans=%s'%self.startChans)
            #self.LogMessage('DEBUG',Origin=origin,Msg='self.chanSteps=%s'%self.chanSteps)
        
    @dec_engines_logging
    def fromfile( self, outprefix='' ):
        """
        Create image from spectral array that is taken from grid ASAP table.

        Number of pixels should be given externally.
        """
        origin = 'fromfile()'
   
        import numpy

        # configure coordinate system
        self.configurecoords()
        
        # Imaging
        arrs=[]
        spwarr=[]
        spwintegarr=[]
        polids = []
        for i in xrange(len(self.nChans)):
            spwarr.append([])
            spwintegarr.append([])
        #self.LogMessage('DEBUG',Origin=origin,Msg='self.spfiles=%s'%self.spfiles)
        for i in xrange(len(self.spfiles.keys())):
            key = self.spfiles.keys()[i]
            self._tb.open(self.spfiles[key])
            sp=self._tb.getcol('SPECTRA')
            polids.append(self._tb.getcell('POLNO',0))
            #self.LogMessage('DEBUG',Origin=origin,Msg='self.spfiles[%s]: %s, nrow=%s, nGridX=%s, nGridY=%s'%(i,self.spfiles[key],self._tb.nrows(),self.nGridX,self.nGridY))
            self._tb.close()
            shape=sp.shape # (nchan,nrow)
            #self.LogMessage('DEBUG',Origin=origin,Msg='sp.shape=%s'%([shape]))
            arr=sp.reshape((1,shape[0],self.nGridY,self.nGridX))
            arr=arr.transpose()
            arr=numpy.flipud(arr)
            #self.LogMessage('DEBUG',Origin=origin,Msg='arr.shape=%s'%([arr.shape]))

            # for full channel image
            if (self.isTP is False) and (len(self.edge) > 0) and (self.edge[0:2].count(0) != 2):
                self.LogMessage('INFO',Origin=origin,Msg='Removed edge: %s'%(self.edge))
                if self.edge[1] == 0:
                    arr2=arr[:,:,self.edge[0]:,:]
                else:
                    arr2=arr[:,:,self.edge[0]:-self.edge[1],:]
                #self.LogMessage('DEBUG',Origin=origin,Msg='arr.shape=%s'%([arr2.shape]))
            else:
                arr2=arr
            arrs.append(arr2)

            # for line window image
            for j in xrange(len(self.nChans)):
                spwarr[j].append( self.bin( arr, self.startChans[j], self.nChans[j], self.chanSteps[j] ) )
                #self.LogMessage('DEBUG',Origin=origin,Msg='spwarr[%s][%s].shape=%s'%(j,len(spwarr[j])-1,[spwarr[j][len(spwarr[j])-1].shape]))
                spwintegarr[j].append( self.bin( arr, self.startChans[j], 1, self.nChans[j]*self.chanSteps[j] ) )
                #self.LogMessage('DEBUG',Origin=origin,Msg='spwintegarr[%s][%s].shape=%s'%(j,len(spwintegarr[j])-1,[spwintegarr[j][len(spwintegarr[j])-1].shape]))
                    

        # Create full channel image
        map=numpy.concatenate(arrs,axis=3)
        #self.LogMessage('DEBUG',Origin=origin,Msg='map.shape=%s'%([map.shape]))
        # 2011/03/15 TN
        # Data product naming convention
        imagename=outprefix.rstrip('/')+'.'+self.postfix+'.image'
        if self.productName is not None:
            #imagename = self.productName.getImageName( self.antennaid, self.poltype, polids=polids, spwid=self.spwid )
            imagename = self.productName.getImageName( self.antennaname, self.poltype, polids=polids, spwid=self.spwid ) 
        self._ia.fromarray(outfile=imagename,pixels=map,overwrite=True)
        self.LogMessage('INFO',Origin=origin,Msg='Created %s'%imagename)
        # 2011/03/15 TN
        # Data product naming convention
        self.imagename = imagename
        #self.imagename = self.productName.getImagePrefix( self.antennaid, self.spwid, self.poltype, polids=polids )
        
        # mask blank value
        self._ia.open(imagename)
        self._ia.calcmask('"%s"!=%s'%(imagename,NoData),name='mask0',asdefault=True)

        # set coordinate system
        self._ia.setcoordsys( self.coordsrec )

        # set brightness unit
        self._ia.setbrightnessunit('K')

        self._ia.close()
        self._ia.done()

        # fits image
        fitsname = imagename.replace('.im','.fits')
        self._createfits( imagename, fitsname, False )

        # Create line window image
        self.lineimanges = []
        if len(self.nChans) != 0:
            self.LogMessage('INFO',Origin=origin,Msg='Creating line window images')
        for i in xrange(len(self.nChans)):
            map=numpy.concatenate(spwarr[i],axis=3)
            #self.LogMessage('DEBUG',Origin=origin,Msg='lines=%s: map.shape=%s'%(i,[map.shape]))
            spwname=outprefix.rstrip('/')+'.'+self.postfix+'.line%s.image'%i
            # 2011/03/15 TN
            # Data product naming convention
            if self.productName is not None:
                #spwname = self.productName.getImageName( self.antennaid, self.poltype, polids=polids, line=True, linetype='spw%s.line%s'%(self.spwid,i) )
                spwname = self.productName.getImageName( self.antennaname, self.poltype, polids=polids, line=True, linetype='spw%s.line%s'%(self.spwid,i) )
            self._ia.fromarray(outfile=spwname,pixels=map,overwrite=True)
            self.LogMessage('INFO',Origin=origin,Msg='Created %s'%spwname)
            self.lineimages.append(spwname)

            # mask blank value
            self._ia.open(spwname)
            blankval=self._ia.getchunk().min()
            #self.LogMessage('DEBUG',Origin=origin,Msg='spw[%s]: blankval %s'%(i,blankval))
            self._ia.calcmask('"%s"!=%s'%(spwname,blankval),name='mask0',asdefault=True)
##             self._ia.calcmask('"%s"==%s'%(spwname,blankval),name='imask',asdefault=False)

            # multiply channel width [km/s]
            self._ia.calc('"%s"*%s'%(spwname,self.chanwidthvel))

##             # masked pixel should have blank value
##             self._ia.maskhandler(op='set',name='imask')
##             self._ia.calc( 'max( "%s", %s )'%(spwname,NoData) )
##             self._ia.maskhandler(op='set',name='mask0')
##             self._ia.maskhandler(op='delete',name='imask')
        
            # modify spectral coordinate
            coords=self._cs.newcoordsys()
            coords.fromrecord(self.coordsrec)
            step=self.chanSteps[i]
            nchan=self.nChans[i]
            startchan=self.startChans[i]
            rfpix=coords.referencepixel(type='spectral')['numeric'][0]
            rfinc=coords.increment(type='spectral')['numeric'][0]
            rfval=coords.referencevalue(type='spectral')['numeric'][0]
            ### for relative velocity 2010/11/8 TN
            chanc=self.chanCs[i]
            #self.LogMessage('DEBUG',Origin=origin,Msg='line%s: chanc is %s (before removing edge)'%(i,chanc))
            if (self.isTP is False) and (len(self.edge) != 0) and (self.edge[0] != 0):
                chanc = chanc - self.edge[0]
            names=coords.names()
            idx=[0,0,0,0]
            spid=names.index('Frequency')
            idx[spid]=chanc
            newrestf=coords.toworld(idx)['numeric'][spid]
            #self.LogMessage('DEBUG',Origin=origin,Msg='line%s: new restfrequency is %sHz'%(i,newrestf))
            numrf=len(coords.restfrequency()['value'])
            coords.setrestfrequency(value='%sHz'%newrestf,which=numrf,append=True)
            ###
            offset=startchan-rfpix
            if (self.isTP is False) and (len(self.edge) != 0) and (self.edge[0] != 0):
                offset = offset - self.edge[0]
            offset=offset+0.5*(step-1)
            newrfval=rfval+offset*rfinc
            newrfpix=0.0
            newrfinc=rfinc*step
            coords.setreferencepixel(value=newrfpix,type='spectral')
            coords.setreferencevalue(value='%sHz'%newrfval,type='spectral')
            coords.setincrement(value='%sHz'%newrfinc,type='spectral')
            csrec=coords.torecord()
            spekey=None
            for key in csrec.keys():
                if key.find( 'spectral' ) != -1:
                    spekey=key
            csrec['pixelreplace1']=csrec[spekey]['wcs']['crpix']
            csrec['worldreplace1']=csrec[spekey]['wcs']['crval']
            
            # set coordinate system
            self._ia.setcoordsys( csrec )
            
            # set brightness unit
            self._ia.setbrightnessunit('K km/s')
            #self._ia.setbrightnessunit('K')

            self._ia.close()
            self._ia.done()

            # fits image
            fitsname = spwname.replace('.im','.fits')
            self._createfits( spwname, fitsname, False )

            # integrated intensity image over line window
            map=numpy.concatenate(spwintegarr[i],axis=3)
            #self.LogMessage('DEBUG',Origin=origin,Msg='integ: map.shape=%s'%([map.shape]))
            spwname=outprefix.rstrip('/')+'.'+self.postfix+'.line%s.integ.image'%i
            # 2011/03/15 TN
            # Data product naming convention
            if self.productName is not None:
                #spwname = self.productName.getImageName( self.antennaid, self.poltype, polids=polids, line=True, linetype='spw%s.line%s.integ'%(self.spwid,i) )
                spwname = self.productName.getImageName( self.antennaname, self.poltype, polids=polids, line=True, linetype='spw%s.line%s.integ'%(self.spwid,i) )
            self._ia.fromarray(outfile=spwname,pixels=map,overwrite=True)
            self.LogMessage('INFO',Origin=origin,Msg='Created %s'%spwname)
            self.lineimages.append(spwname)
        
            # mask blank value
            self._ia.open(spwname)
            blankval=self._ia.getchunk().min()
            #self.LogMessage('DEBUG',Origin=origin,Msg='spw[%s] integ: blankval %s'%(i,blankval))
            self._ia.calcmask('"%s"!=%s'%(spwname,blankval),name='mask0',asdefault=True)
##             self._ia.calcmask('"%s"==%s'%(spwname,blankval),name='imask',asdefault=True)

            # multiply channel width [km/s]
            self._ia.calc('"%s"*%s'%(spwname,self.chanwidthvel))
 
##             # masked pixel should have blank value
##             self._ia.maskhandler(op='set',name='imask')
##             self._ia.calc( 'max( "%s", %s )'%(spwname,NoData) )
##             self._ia.maskhandler(op='set',name='mask0')
##             self._ia.maskhandler(op='delete',name='imask')

            # modify spectral coordinate
            coords=self._cs.newcoordsys()
            coords.fromrecord(self.coordsrec)
            step=self.chanSteps[i]*self.nChans[i]
            startchan=self.startChans[i]
            rfpix=coords.referencepixel(type='spectral')['numeric'][0]
            rfinc=coords.increment(type='spectral')['numeric'][0]
            rfval=coords.referencevalue(type='spectral')['numeric'][0]
            offset=startchan-rfpix
            if (self.isTP is False) and (len(self.edge) != 0) and (self.edge[0] != 0):
                offset = offset - self.edge[0]
            offset=offset+0.5*(step-1)
            newrfval=rfval+offset*rfinc
            newrfpix=0.0
            newrfinc=rfinc*step
            coords.setreferencepixel(value=newrfpix,type='spectral')
            coords.setreferencevalue(value='%sHz'%newrfval,type='spectral')
            coords.setincrement(value='%sHz'%newrfinc,type='spectral')
            csrec=coords.torecord()
            spekey=None
            for key in csrec.keys():
                if key.find( 'spectral' ) != -1:
                    spekey=key
            csrec['pixelreplace1']=csrec[spekey]['wcs']['crpix']
            csrec['worldreplace1']=csrec[spekey]['wcs']['crval']

            # set coordinate system
            self._ia.setcoordsys( csrec )
            
            # set brightness unit
            self._ia.setbrightnessunit('K km/s')
            #self._ia.setbrightnessunit('K')
        
            self._ia.close()
            self._ia.done()

            # fits image
            fitsname = spwname.replace('.im','.fits')
            self._createfits( spwname, fitsname, False )

        
        # Continuum image
        if (self.isTP is False) and (self.contfiles is not None):
            self.LogMessage('INFO',Origin=origin,Msg='Creating continuum image')
            #self.LogMessage('DEBUG',Origin=origin,Msg='self.contfiles=%s'%self.contfiles)
            arrs=[]
            for i in xrange(len(self.contfiles.keys())):
                key = self.contfiles.keys()[i]
                self._tb.open(self.contfiles[key])
                sp=self._tb.getcol('SPECTRA')
                #self.LogMessage('DEBUG',Origin=origin,Msg='self.contfiles[%s]: %s, nrow=%s, nGridX=%s, nGridY=%s'%(i,self.contfiles[i],self._tb.nrows(),self.nGridX,self.nGridY))
                self._tb.close()
                shape=sp.shape # (nchan,nrow)
                #self.LogMessage('DEBUG',Origin=origin,Msg='sp.shape=%s'%([shape]))
                arr=sp.reshape((1,shape[0],self.nGridY,self.nGridX))
                arr=arr.transpose()
                arr=numpy.flipud(arr)
                #self.LogMessage('DEBUG',Origin=origin,Msg='arr.shape=%s'%([arr.shape]))

                cntarr = self.bin( arr, self.startChanCont, self.nChanCont, self.chanStepCont )
                #self.LogMessage('DEBUG',Origin=origin,Msg='cntarr.shape=%s'%[cntarr.shape])
                arrs.append(cntarr)
                            
            # Create continuum image
            map=numpy.concatenate(arrs,axis=3)
            contname=outprefix.rstrip('/')+'.'+self.postfix+'.cont.image'
            # 2011/03/15 TN
            # Data product naming convention
            if self.productName is not None:
                #contname = self.productName.getImageName( self.antennaid, self.poltype, polids=polids, spwid=self.spwid, cont=True )
                contname = self.productName.getImageName( self.antennaname, self.poltype, polids=polids, spwid=self.spwid, cont=True )
            #self.LogMessage('DEBUG',Origin=origin,Msg='map.shape=%s'%([map.shape]))
            self._ia.fromarray(outfile=contname,pixels=map,overwrite=True)
            self.LogMessage('INFO',Origin=origin,Msg='Created %s'%contname)
            self.contimage=contname
            
            # mask blank value
            self._ia.open(contname)
            blankval=self._ia.getchunk().min()
            #self.LogMessage('DEBUG',Origin=origin,Msg='cont: blankval %s'%blankval)
            self._ia.calcmask('"%s"!=%s'%(contname,blankval),name='mask0',asdefault=True)
##             self._ia.calcmask('"%s"==%s'%(contname,blankval),name='imask',asdefault=True)

            # multiply channel width [km/s]
            # self._ia.calc('"%s"*%s'%(contname,self.chanwidthvel))

##             # masked pixel should have blank value
##             self._ia.maskhandler(op='set',name='imask')
##             self._ia.calc( 'max( "%s", %s )'%(contname,NoData) )
##             self._ia.maskhandler(op='set',name='mask0')
##             self._ia.maskhandler(op='delete',name='imask')

            # divide by number of channel summed
            self._ia.calc('"%s"/%s'%(contname,self.chanStepCont))

            # modify spectral coordinate
            coords=self._cs.newcoordsys()
            coords.fromrecord(self.coordsrec)
            rfpix=coords.referencepixel(type='spectral')['numeric'][0]
            rfval=coords.referencevalue(type='spectral')['numeric'][0]
            rfinc=coords.increment(type='spectral')['numeric'][0]
            offset=self.startChanCont-rfpix
            if (self.isTP == False) and (len(self.edge) != 0) and (self.edge[0] != 0):
                offset = offset - self.edge[0]
            step=self.chanStepCont
            offset=offset+0.5*(step-1)
            newrfpix=0.0
            newrfval=rfval+offset*rfinc
            newrfinc=rfinc*step
            coords.setreferencepixel(value=newrfpix,type='spectral')
            coords.setreferencevalue(value='%sHz'%newrfval,type='spectral')
            coords.setincrement(value='%sHz'%newrfinc,type='spectral')
            csrec=coords.torecord()
            spekey=None
            for key in csrec.keys():
                if key.find( 'spectral' ) != -1:
                    spekey=key
            csrec['pixelreplace1']=csrec[spekey]['wcs']['crpix']
            csrec['worldreplace1']=csrec[spekey]['wcs']['crval']

            # set coordinate system
            self._ia.setcoordsys( csrec )
        
            # set brightness unit
            self._ia.setbrightnessunit('K')

            self._ia.close()
            self._ia.done()

            # fits image
            fitsname = contname.replace('.im','.fits')
            self._createfits( contname, fitsname, False )

    @dec_engines_logging
    def configurecoords( self ):
        """
        Configure coordinate system from filename.
        """

        origin='configurecoords()'

        filename=self.spfiles[self.spfiles.keys()[0]]

        # create coordinate system
        coords=self._cs.newcoordsys()

        # add direction and spectral coordinate system
        coords.addcoordinate( direction=True, spectral=True )
        
        # get meta data for coordinate system
        self._tb.open(filename)
        antname=self._tb.getkeyword('AntennaName')
        if antname.find('OSF') != -1:
            antname='OSF'
        elif antname.find('ALMA') != -1:
            antname='ALMA'
        elif antname.find('APEX') != -1:
            antname='APEX'
        elif antname.find('MRT') != -1:
            antname='IRAM_30m'
        observer=self._tb.getkeyword('Observer')
        obsdate=self._tb.getkeyword('UTC')
        if obsdate > 1.0e9:
            # sec->day
            obsdate /= 86400.0
        molid=self._tb.getcell('MOLECULE_ID',0)
        freqid=self._tb.getcell('FREQ_ID',0)
        self.nchan=len(self._tb.getcell('SPECTRA',0))
        #freqtab=self._tb.getkeyword('FREQUENCIES').split()[-1]
        #moltab=self._tb.getkeyword('MOLECULES').split()[-1]
        freqtab=self._tb.getkeyword('FREQUENCIES').lstrip('Table: ')
        moltab=self._tb.getkeyword('MOLECULES').lstrip('Table: ')
        self._tb.close()

        # Check if total power data
        if self.nchan == 1:
            self.isTP = True

        # Stokes (assumed linear polarization)
        stokes=''
        stokes2=[]
        for i in xrange(len(self.spfiles.keys())):
            if self.spfiles.keys()[i] == 0:
                stokes=stokes+stokestype[9]+' '
                stokes2.append(stokestype[9])
            elif self.spfiles.keys()[i] == 1:
                stokes=stokes+stokestype[12]+' '
                stokes2.append(stokestype[12])
            elif self.spfiles.keys()[i] == 2:
                stokes=stokes+stokestype[10]+' '
                stokes2.append(stokestype[10])
            elif self.spfiles.keys()[i] == 3:
                stokes=stokes+stokestype[11]+' '
                stokes2.append(stokestype[11])
        stokes=stokes.rstrip()
        #self.LogMessage('DEBUG',Origin=origin,Msg='stokes=%s'%stokes)
        #coords.setstokes(stokes)
        coords.addcoordinate(stokes=stokes2)

        # Direction
        # axes are automatically set as RA and DEC
        # conversion system is automatically set as J2000
        refpix=[float(self.nGridX-1)/2.0,float(self.nGridY-1)/2.0]
        #self.LogMessage('DEBUG',Origin=origin,Msg='cell=%s'%self.cellSize)
        #self.LogMessage('DEBUG',Origin=origin,Msg='center=%s'%self.mapCenter)
        #self.LogMessage('DEBUG',Origin=origin,Msg='refpix=%s'%refpix)
        coords.setdirection( refpix=refpix, refval=self.mapCenter, incr=self.cellSize)

        # Spectral
        self._tb.open(freqtab)
        idCol=self._tb.getcol('ID')
        idx=-1
        for id in idCol:
            if id == freqid:
                idx=id
                break
        frefpix=self._tb.getcell('REFPIX',idx)
        if (self.isTP == False) and (len(self.edge) != 0) and (self.edge[0] != 0):
            frefpix = frefpix - self.edge[0]
        frefval=self._tb.getcell('REFVAL',idx)
        fincr=self._tb.getcell('INCREMENT',idx)
        self._tb.close()
        self._tb.open(moltab)
        rfs=self._tb.getcell('RESTFREQUENCY',molid)
        rf=frefval
        if len(rfs) != 0 :
            rf=rfs[0]
        self._tb.close()
        #self.LogMessage('DEBUG',Origin=origin,Msg='restfreq=%s'%rf)
        coords.setspectral(restfreq=rf,doppler='radio')
        coords.setreferencepixel(value=[frefpix],type='spectral')
        coords.setreferencevalue(value='%sHz'%frefval,type='spectral')
        coords.setincrement(value='%sHz'%fincr,type='spectral')
        

        # Observer
        #self.LogMessage('DEBUG',Origin=origin,Msg='observer=%s'%observer)
        coords.setobserver( observer )

        # Telescope
        #self.LogMessage('DEBUG',Origin=origin,Msg='telescope=%s'%antname)
        coords.settelescope(antname)

        # to record
        csrec=coords.torecord()

        # find keys
        dirkey=None
        spekey=None
        stokey=None
        for key in csrec.keys():
            if key.find( 'direction' ) != -1:
                dirkey=key
            elif key.find( 'spectral' ) != -1:
                spekey=key
            elif key.find( 'stokes' ) != -1:
                stokey=key

        # Telescope position
        #self.LogMessage('DEBUG',Origin=origin,Msg='antname=%s'%antname)
        position=self._me.observatory(antname)
        #self.LogMessage('DEBUG',Origin=origin,Msg='position=%s'%position)
        if len(position) != 0:
            csrec['telescopeposition']=position

            # Rest of spectral axis
            csrec[spekey]['conversion']['position']=position

        # Pointing center
        csrec['pointingcenter']['value']=csrec[dirkey]['crval']

        # Observing date
        csrec['obsdate']['m0']['value']=obsdate

        # world replace
        csrec['worldreplace0']=csrec[dirkey]['crval']
        csrec['worldreplace1']=csrec[spekey]['wcs']['crval']
        csrec['worldreplace2']=csrec[stokey]['crval']
        csrec['pixelreplace0']=csrec[dirkey]['crpix']
        csrec['pixelreplace1']=csrec[spekey]['wcs']['crpix']
        csrec['pixelreplace2']=csrec[stokey]['crpix']

        # some coordsys info will be useful at later imaging
        self.coordsrec=csrec

        #channel width in velocity (km/s/ch)
        self.chanwidthvel=abs(fincr)/rf*299792.458
        #self.LogMessage('DEBUG',Origin=origin,Msg='channel width in velocity: %s km/s'%self.chanwidthvel)

        # from SDPlotter.DrawContImage
        if self.contfiles is not None:
            NContMap=3
            NCHAN=self.nchan
            ChanC = int(NCHAN/2)
            ChanW = int(NCHAN/(NContMap + 1))
            ChanB = int(ChanC - NContMap / 2.0 * ChanW)
            self.startChanCont = ChanB
            self.nChanCont = NContMap
            self.chanStepCont = ChanW
            #self.LogMessage('DEBUG',Origin=origin,Msg='self.startChanCont=%s'%self.startChanCont)
            #self.LogMessage('DEBUG',Origin=origin,Msg='self.nChanCont=%s'%self.nChanCont)
            #self.LogMessage('DEBUG',Origin=origin,Msg='self.chanStepCont=%s'%self.chanStepCont)            


    def postfix( self ):
        origin='postfix()'

        filename=self.spfiles[self.spfiles.keys()[0]]

        self._tb.open(filename)
        ifno=self._tb.getcell('IFNO',0)
        self._tb.close()

        return 'spw%s'%ifno

    @dec_engines_logging
    def moments( self, moments=[] ):
        """
        Create moment images for created images.
        """
        origin='moments()'

        # remove moments=4 (median coordinate) since it causes exception 
        badmoments = [4]
        for moment in badmoments:
            if moment in moments:
                self.LogMessage('WARNING',Origin=origin,Msg='Moment %s may cause exception, remove from the moment list.'%moment)
                while ( moment in moments ): 
                    moments.pop(moments.index(moment))

        # skip TP data
        if self.isTP:
            self.LogMessage('INFO',Origin=origin,Msg='Moment images are not created since it is total power data.')
            #rmtables([self.imagename])
        else:
            # for full channel image
            if self.imagename is not None:
                self.calcmoments( self.imagename, moments )
                # CASA image is only for moment calculation
                #rmtables([self.imagename])

            # for continuum image
            if self.contimage is not None:
                self.calcmoments( self.contimage, moments )
                # CASA image is only for moment calculation
                #rmtables([self.contimage])

    @dec_engines_logging
    def calcmoments( self, filename=None, moments=[] ):
        """
        Create moment images.
        """
        origin='calcmoments()'

        if (filename is not None) and (type(moments)==list) and (len(moments) >0):
            self.LogMessage('INFO',Origin=origin,Msg='Creating moment image(s): moments=%s'%moments)
            
            self._ia.open(filename)
            if len(moments) == 1:
                # If len(moments) is 1 and outfile is empty, output image is
                # created only on the memory temporarily. Thus, outfile must
                # be explicitly given to create output image on disk.
                # 2010/02/16 TN
                moutname = filename + '.moment%s'%(moments[0])
                mimage = self._ia.moments(moments=moments, outfile=moutname, overwrite=True)
                mimage.close()
                del mimage
                
                # fits image
                fitsname = moutname.replace('.im','.fits')
                self._createfits( moutname, fitsname, False )

            else:
                mimage = self._ia.moments(moments=moments, overwrite=True)
                mimage.close()
                del mimage

                # fits images
                import commands
                import os
                files = commands.getoutput('ls %s.*'%filename).split('\n')
                for f in files:
                    if os.path.isdir(f):
                        ff = open(f+'/table.info')
                        typestr=f.readline().rstrip('\n').split()[-1]
                        ff.close()
                        if typestr == 'Image':
                            fitsname = f.replace('.im','.fits')
                            self._createfits( f, fitsname, False )
            self.LogMessage('INFO',Origin=origin,Msg='Moment image(s) were created')
            self._ia.close()
            self._ia.done()
        else:
            self.LogMessage('INFO',Origin=origin,Msg='Moment image(s) were not created because of bad arguments: filename=%s, moments=%s'%(filename,moments))
            
    def bin( self, arr, startchan, nchan, chanstep ):
        """
        Channel binning.

        Assumed that frequency axis is 2.
        """
        import numpy
        arr2=[]
        for k in xrange(nchan):
            c0=startchan+k*chanstep
            c1=c0+chanstep
            tmparr=arr[:,:,c0:c1,:]
            tmparr2=tmparr.sum(axis=2)
            tshape=tmparr2.shape
            arr2.append( tmparr2.reshape(tshape[0],tshape[1],1,tshape[2]) )
        return numpy.concatenate(arr2,axis=2)
        

    @dec_engines_logging
    def createfits( self, dirname='./', prefix='something' ):
        """
        Try to find CASA images in directory specified by dirname,
        then create FITS image for them. 
        """
        origin = 'createfits()' 

        import os
        
        # get whole list of files
        files=os.listdir(dirname)
        #print files

        # only retrieve CASA image
        images=[]
        #outprefix=outprefix.rstrip('/').split('/')[-1]
        outprefix=prefix
        #print outprefix
        for i in xrange(len(files)):
            #print files[i]
            if ( os.path.isdir( dirname+'/'+files[i] ) ) and ( files[i].find(outprefix) != -1 ):
                tableinfo=dirname+'/'+files[i].rstrip('/')+'/table.info'
                #print tableinfo
                if os.path.exists( tableinfo ):
                    f=open(tableinfo)
                    typestr=f.readline().rstrip('\n').split()[-1]
                    f.close()
                    #print typestr
                    if typestr == 'Image':
                        images.append(files[i])
        if len(images) == 0:
            self.LogMessage( 'WARNING', Origin=origin, Msg='No images in %s'%dirname )
            return False

        # create FITS images
        for fname in images:
            infile = dirname.rstrip('/')+'/'+fname
            outfile = dirname.rstrip('/')+'/'+fname.rstrip('/')+'.fits'
            self.LogMessage( 'INFO', Origin=origin, Msg='Creating %s'%outfile )
            self._ia.open(infile)
            self._ia.tofits(outfile=outfile,optical=False,overwrite=True)
            self._ia.close()
            
        return True

    @dec_engines_logging
    def _createfits( self, imagename, fitsname, rmtable=True ):
        """
        """
        origin='_createfits'
        self.LogMessage( 'INFO', Origin=origin, Msg='Creating %s'%fitsname )
        self._ia.open(imagename)
        self._ia.tofits(outfile=fitsname,optical=False,overwrite=True)
        self._ia.close()
        if rmtable:
            rmtables([imagename])

    #@dec_engines_logging
    #def setAntennaId( self, antid ):
    #    """
    #    """
    #    self.antennaid = antid

    @dec_engines_logging
    def setAntennaName( self, ant ):
        self.antennaname = ant

    @dec_engines_logging
    def setProductName( self, product ):
        """
        """
        self.productName = product

    @dec_engines_logging
    def setPolType( self, type ):
        """
        """
        ptype = type.lower()
        if ptype == 'stokes':
            self.poltype = 'stokes'
        elif ptype == 'linear':
            self.poltype = 'linear'
        elif ptype == 'circular':
            self.poltype = 'circular'
        else:
            self.poltype = 'none'

    @dec_engines_logging
    def setSpectralWindowId( self, id ):
        """
        """
        self.spwid = id
