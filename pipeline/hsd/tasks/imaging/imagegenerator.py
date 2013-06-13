from __future__ import absolute_import

import os
import math
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from .. import common

LOG = infrastructure.get_logger(__name__)

NoData = common.NoData

class SDImageGeneratorResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDImageGeneratorResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        pass

    def _outcome_name(self):
        return self.outcome


class SDImageGenerator(object):
    StokesType={0: 'Undefined',
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

    def __init__(self, spectral_data, edge=[]):
        self.data = numpy.array(spectral_data) #(npol,nrow,nchan)
        self.nchan = self.data.shape[-1]
        # Check if channel averaged data
        self.isaveraged = (self.nchan == 1)
        
        self.edge = list(common.parseEdge(edge))
        
    def full_channel_image(self, imagename=None):
        # reshape data from (npol,nrow,nchan) to (nx,ny,nchan,npol)
        self._reshape_data()

        # drop channels
        work_array = self._drop_channels()

        # imaging
        if imagename is None:
            imagename = 'full_channel_image.image'
        coordsys = self.coords_rec
        self._create_image(imagename, work_array, coordsys, NoData)

        result = SDImageGeneratorResults(self.__class__,
                                         success=True,
                                         outcome=imagename)
        result.task = self.__class__
        return result

    def _reshape_data(self):
        if len(self.data.shape) < 4:
            # let's reshape self.data
            (npol,nrow,nchan) = self.data.shape
            self.data = self.data.transpose(0,2,1)
            self.data = self.data.reshape((npol,nchan,self.ny,self.nx))
            self.data = self.data.transpose() # (nx,ny,nchan,npol)
            self.data = numpy.flipud(self.data) # flip x-axis

    def _drop_channels(self):
        if self.isaveraged or len(self.edge) == 0 or self.edge == [0,0]:
            return self.data
        else:
            return self.data[:,:,self.edge[0]:self.nchan-self.edge[1],:]
        
    def _create_image(self, imagename, imagedata, coordsys, nodata=None):
        casatools.image.fromarray(outfile=imagename, pixels=imagedata, overwrite=True)
        LOG.info('Created %s'%(imagename))

        with casatools.ImageReader(imagename) as ia:
            # mask blank value
            if nodata is None:
                blankval = NoData
            else:
                blankval = nodata
            ia.calcmask('"%s"!=%s'%(imagename,blankval), name='mask0', asdefault=True)

            # set coordinate system
            ia.setcoordsys(coordsys)
            
            # set brightness unit
            ia.setbrightnessunit('K')
            
    def define_image(self, grid_table, freq_refpix, freq_refval, freq_increment, rest_frequency=None, antenna='', observer='', obs_date=0.0, stokes='XX YY'):
        self.nx = 1
        while grid_table[self.nx][5] == grid_table[0][5]:
            self.nx += 1
        self.ny = len(grid_table) / self.nx
        dx = grid_table[0][4] - grid_table[1][4]
        #self.cellx = casatools.quanta.quantity(x,'deg')
        #self.cellx = '%sdeg'%(x)
        dy = grid_table[self.nx][5] - grid_table[0][5]
        #self.celly = casatools.quanta.quantity(y,'deg')
        self.celly = '%sdeg'%(dy)
        # 2013/06/11 TN
        # cellx and celly (CDELT for direction axes) should
        # be a physical size (no DEC correction on R.A.).
        self.cellx = '%sdeg'%(abs(dy) * (1.0 if dx > 0.0 else -1.0))
        x = 0.5 * (grid_table[0][4] + grid_table[-1][4])
        y = 0.5 * (grid_table[0][5] + grid_table[-1][5])
        #self.center = [casatools.quanta.quantity(x,'deg'),
        #               casatools.quanta.quantity(y,'deg')]
        self.center = ['%sdeg'%(x), '%sdeg'%(y)]
        self.coords_rec = self._configure_coords(self.nx, self.ny, self.cellx, self.celly, self.center, freq_refpix, freq_refval, freq_increment, rest_frequency, antenna, observer, obs_date, stokes)
        
    def _configure_coords(self, nx, ny, cellx, celly, center, freq_refpix, freq_refval, freq_increment, rest_frequency=None, antenna='', observer='', obs_date=0.0, stokes='XX YY'):
        """
        Configure coordinate system from filename.
        """
        # create coordinate system
        coords=casatools.coordsys.newcoordsys()

        # add direction and spectral coordinate system
        coords.addcoordinate(direction=True, spectral=True)
        coords.addcoordinate(stokes=stokes)
        
        # Observer
        LOG.debug('observer=%s'%observer)
        coords.setobserver(observer)

        # Telescope
        LOG.debug('telescope=%s'%antenna)
        coords.settelescope(antenna)

        # Direction
        # axes are automatically set to RA and DEC
        # equinox is automatically set to J2000
        refpix=[0.5*float(nx-1),0.5*float(ny-1)]
        LOG.debug('center=%s'%center)
        LOG.debug('refpix=%s'%refpix)
        if not casatools.quanta.isquantity(cellx):
            cellx = '%sdeg'%(cellx)
        if not casatools.quanta.isquantity(celly):
            celly = '%sdeg'%(celly)
        LOG.debug('cell=%s'%([cellx,celly]))
        coords.setdirection(refpix=refpix, refval=center, incr=[cellx,celly])

        # Spectral
        if rest_frequency is None or len(rest_frequency) == 0:
            LOG.warning('rest frequency is set to CRVAL (%sHz)'%(freq_refval))
            rest_frequency = freq_refval
        else:
            LOG.todo('Pick up correct rest frequency if spw has multiple rest frequencies')
            rest_frequency = rest_frequency[0] 
        LOG.debug('restfreq=%s'%rest_frequency)
        coords.setspectral(restfreq=casatools.quanta.quantity(rest_frequency,'Hz'),doppler='radio')
        coords.setreferencepixel(value=[freq_refpix],type='spectral')
        coords.setreferencevalue(value='%sHz'%freq_refval,type='spectral')
        coords.setincrement(value='%sHz'%freq_increment,type='spectral')
        
        # to record
        coords_rec=coords.torecord()

        # find keys
        coord_types = coords.coordinatetype()
        dirkey = 'direction%s'%(coord_types.index('Direction'))
        spekey = 'spectral%s'%(coord_types.index('Spectral'))
        stokey = 'stokes%s'%(coord_types.index('Stokes'))

        # Telescope position
        position=casatools.measures.observatory(antenna)
        LOG.debug('position=%s'%position)
        if len(position) != 0:
            coords_rec['telescopeposition']=position

            # Rest of spectral axis
            coords_rec[spekey]['conversion']['position']=position

        # Pointing center
        coords_rec['pointingcenter']['value']=coords_rec[dirkey]['crval']

        # Observing date
        if casatools.measures.ismeasure(obs_date):
            coords_rec['obsdate'] = obs_date
        else:
            if obs_date > 1.0e9:
                # sec->day
                obs_date /= 86400.0
            coords_rec['obsdate']['m0']['value']=obs_date

        # world replace
        coords_rec['worldreplace0']=coords_rec[dirkey]['crval']
        coords_rec['worldreplace1']=coords_rec[spekey]['wcs']['crval']
        coords_rec['worldreplace2']=coords_rec[stokey]['crval']
        coords_rec['pixelreplace0']=coords_rec[dirkey]['crpix']
        coords_rec['pixelreplace1']=coords_rec[spekey]['wcs']['crpix']
        coords_rec['pixelreplace2']=coords_rec[stokey]['crpix']

        #channel width in velocity (km/s/ch)
        self.chanwidthvel=abs(freq_increment)/rest_frequency*299792.458
        #LOG.debug('channel width in velocity: %s km/s'%self.chanwidthvel)

        return coords_rec

class OldSDFitsCubeGenerator(object):
    """
    Create image from regridded spectra (numpy array) directly.
    """
    def __init__( self, spfiles=None, contfiles=None, edge=[], LogLevel=2, ConsoleLevel=3, LogFile=False ):
        """
        Constructor
        
        sdMS: input MS data to be imaged
        edge: channel edge
        """
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
            #LOG.debug('self.nChans=%s'%self.nChans)
            #LOG.debug('self.startChans=%s'%self.startChans)
            #LOG.debug('self.chanSteps=%s'%self.chanSteps)
        
    def fromfile( self, outprefix='' ):
        """
        Create image from spectral array that is taken from grid ASAP table.

        Number of pixels should be given externally.
        """
        # configure coordinate system
        self.configurecoords()
        
        # Imaging
        arrs=[]
        spwarr=[]
        spwintegarr=[]
        spwarr = [[] for i in xrange(len(self.nChans))]
        spwintegarr = sp[:]
        polids = []
        #LOG.debug('self.spfiles=%s'%self.spfiles)
        for i in xrange(len(self.spfiles.keys())):
            key = self.spfiles.keys()[i]
            #self._tb.open(self.spfiles[key])
            with casatools.TableReader(self.spfiles[key]) as tb:
                sp=tb.getcol('SPECTRA')
                polids.append(tb.getcell('POLNO',0))
            #LOG.debug('self.spfiles[%s]: %s, nrow=%s, nGridX=%s, nGridY=%s'%(i,self.spfiles[key],self._tb.nrows(),self.nGridX,self.nGridY))
            #self._tb.close()
            shape=sp.shape # (nchan,nrow)
            #LOG.debug('sp.shape=%s'%([shape]))
            arr=sp.reshape((1,shape[0],self.nGridY,self.nGridX))
            arr=arr.transpose()
            arr=numpy.flipud(arr)
            #LOG.debug('arr.shape=%s'%([arr.shape]))

            # for full channel image
            if (self.isTP is False) and (len(self.edge) > 0) and (self.edge[0:2].count(0) != 2):
                LOG.info('Removed edge: %s'%(self.edge))
                if self.edge[1] == 0:
                    arr2=arr[:,:,self.edge[0]:,:]
                else:
                    arr2=arr[:,:,self.edge[0]:-self.edge[1],:]
                #LOG.debug('arr.shape=%s'%([arr2.shape]))
            else:
                arr2=arr
            arrs.append(arr2)

            # for line window image
            for j in xrange(len(self.nChans)):
                spwarr[j].append( self.bin( arr, self.startChans[j], self.nChans[j], self.chanSteps[j] ) )
                #LOG.debug('spwarr[%s][%s].shape=%s'%(j,len(spwarr[j])-1,[spwarr[j][len(spwarr[j])-1].shape]))
                spwintegarr[j].append( self.bin( arr, self.startChans[j], 1, self.nChans[j]*self.chanSteps[j] ) )
                #LOG.debug('spwintegarr[%s][%s].shape=%s'%(j,len(spwintegarr[j])-1,[spwintegarr[j][len(spwintegarr[j])-1].shape]))
                    

        # Create full channel image
        map=numpy.concatenate(arrs,axis=3)
        #LOG.debug('map.shape=%s'%([map.shape]))
        # 2011/03/15 TN
        # Data product naming convention
        imagename=outprefix.rstrip('/')+'.'+self.postfix+'.image'
        if self.productName is not None:
            #imagename = self.productName.getImageName( self.antennaid, self.poltype, polids=polids, spwid=self.spwid )
            imagename = self.productName.getImageName( self.antennaname, self.poltype, polids=polids, spwid=self.spwid ) 
        casatools.image.fromarray(outfile=imagename,pixels=map,overwrite=True)
        self.LogMessage('INFO',Origin=origin,Msg='Created %s'%imagename)
        # 2011/03/15 TN
        # Data product naming convention
        self.imagename = imagename
        #self.imagename = self.productName.getImagePrefix( self.antennaid, self.spwid, self.poltype, polids=polids )
        
        # mask blank value
        casatools.image.open(imagename)
        casatools.image.calcmask('"%s"!=%s'%(imagename,NoData),name='mask0',asdefault=True)

        # set coordinate system
        casatools.image.setcoordsys( self.coordsrec )

        # set brightness unit
        casatools.image.setbrightnessunit('K')

        casatools.image.close()
        casatools.image.done()

        # fits image
        fitsname = imagename.replace('.im','.fits')
        self._createfits( imagename, fitsname, False )

        # Create line window image
        self.lineimanges = []
        if len(self.nChans) != 0:
            self.LogMessage('INFO',Origin=origin,Msg='Creating line window images')
        for i in xrange(len(self.nChans)):
            map=numpy.concatenate(spwarr[i],axis=3)
            #LOG.debug('lines=%s: map.shape=%s'%(i,[map.shape]))
            spwname=outprefix.rstrip('/')+'.'+self.postfix+'.line%s.image'%i
            # 2011/03/15 TN
            # Data product naming convention
            if self.productName is not None:
                #spwname = self.productName.getImageName( self.antennaid, self.poltype, polids=polids, line=True, linetype='spw%s.line%s'%(self.spwid,i) )
                spwname = self.productName.getImageName( self.antennaname, self.poltype, polids=polids, line=True, linetype='spw%s.line%s'%(self.spwid,i) )
            casatools.image.fromarray(outfile=spwname,pixels=map,overwrite=True)
            self.LogMessage('INFO',Origin=origin,Msg='Created %s'%spwname)
            self.lineimages.append(spwname)

            # mask blank value
            casatools.image.open(spwname)
            blankval=casatools.image.getchunk().min()
            #LOG.debug('spw[%s]: blankval %s'%(i,blankval))
            casatools.image.calcmask('"%s"!=%s'%(spwname,blankval),name='mask0',asdefault=True)
##             casatools.image.calcmask('"%s"==%s'%(spwname,blankval),name='imask',asdefault=False)

            # multiply channel width [km/s]
            casatools.image.calc('"%s"*%s'%(spwname,self.chanwidthvel))

##             # masked pixel should have blank value
##             casatools.image.maskhandler(op='set',name='imask')
##             casatools.image.calc( 'max( "%s", %s )'%(spwname,NoData) )
##             casatools.image.maskhandler(op='set',name='mask0')
##             casatools.image.maskhandler(op='delete',name='imask')
        
            # modify spectral coordinate
            coords=casatools.coordsys.newcoordsys()
            coords.fromrecord(self.coordsrec)
            step=self.chanSteps[i]
            nchan=self.nChans[i]
            startchan=self.startChans[i]
            rfpix=coords.referencepixel(type='spectral')['numeric'][0]
            rfinc=coords.increment(type='spectral')['numeric'][0]
            rfval=coords.referencevalue(type='spectral')['numeric'][0]
            ### for relative velocity 2010/11/8 TN
            chanc=self.chanCs[i]
            #LOG.debug('line%s: chanc is %s (before removing edge)'%(i,chanc))
            if (self.isTP is False) and (len(self.edge) != 0) and (self.edge[0] != 0):
                chanc = chanc - self.edge[0]
            names=coords.names()
            idx=[0,0,0,0]
            spid=names.index('Frequency')
            idx[spid]=chanc
            newrestf=coords.toworld(idx)['numeric'][spid]
            #LOG.debug('line%s: new restfrequency is %sHz'%(i,newrestf))
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
            casatools.image.setcoordsys( csrec )
            
            # set brightness unit
            casatools.image.setbrightnessunit('K km/s')
            #casatools.image.setbrightnessunit('K')

            casatools.image.close()
            casatools.image.done()

            # fits image
            fitsname = spwname.replace('.im','.fits')
            self._createfits( spwname, fitsname, False )

            # integrated intensity image over line window
            map=numpy.concatenate(spwintegarr[i],axis=3)
            #LOG.debug('integ: map.shape=%s'%([map.shape]))
            spwname=outprefix.rstrip('/')+'.'+self.postfix+'.line%s.integ.image'%i
            # 2011/03/15 TN
            # Data product naming convention
            if self.productName is not None:
                #spwname = self.productName.getImageName( self.antennaid, self.poltype, polids=polids, line=True, linetype='spw%s.line%s.integ'%(self.spwid,i) )
                spwname = self.productName.getImageName( self.antennaname, self.poltype, polids=polids, line=True, linetype='spw%s.line%s.integ'%(self.spwid,i) )
            casatools.image.fromarray(outfile=spwname,pixels=map,overwrite=True)
            self.LogMessage('INFO',Origin=origin,Msg='Created %s'%spwname)
            self.lineimages.append(spwname)
        
            # mask blank value
            casatools.image.open(spwname)
            blankval=casatools.image.getchunk().min()
            #LOG.debug('spw[%s] integ: blankval %s'%(i,blankval))
            casatools.image.calcmask('"%s"!=%s'%(spwname,blankval),name='mask0',asdefault=True)
##             casatools.image.calcmask('"%s"==%s'%(spwname,blankval),name='imask',asdefault=True)

            # multiply channel width [km/s]
            casatools.image.calc('"%s"*%s'%(spwname,self.chanwidthvel))
 
##             # masked pixel should have blank value
##             casatools.image.maskhandler(op='set',name='imask')
##             casatools.image.calc( 'max( "%s", %s )'%(spwname,NoData) )
##             casatools.image.maskhandler(op='set',name='mask0')
##             casatools.image.maskhandler(op='delete',name='imask')

            # modify spectral coordinate
            coords=casatools.coordsys.newcoordsys()
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
            casatools.image.setcoordsys( csrec )
            
            # set brightness unit
            casatools.image.setbrightnessunit('K km/s')
            #casatools.image.setbrightnessunit('K')
        
            casatools.image.close()
            casatools.image.done()

            # fits image
            fitsname = spwname.replace('.im','.fits')
            self._createfits( spwname, fitsname, False )

        
        # Continuum image
        if (self.isTP is False) and (self.contfiles is not None):
            self.LogMessage('INFO',Origin=origin,Msg='Creating continuum image')
            #LOG.debug('self.contfiles=%s'%self.contfiles)
            arrs=[]
            for i in xrange(len(self.contfiles.keys())):
                key = self.contfiles.keys()[i]
                #self._tb.open(self.contfiles[key])
                with casatools.TableReader(self.contfiles[key]) as tb:
                    sp=tb.getcol('SPECTRA')
                    #LOG.debug('self.contfiles[%s]: %s, nrow=%s, nGridX=%s, nGridY=%s'%(i,self.contfiles[i],self._tb.nrows(),self.nGridX,self.nGridY))
                #self._tb.close()
                shape=sp.shape # (nchan,nrow)
                #LOG.debug('sp.shape=%s'%([shape]))
                arr=sp.reshape((1,shape[0],self.nGridY,self.nGridX))
                arr=arr.transpose()
                arr=numpy.flipud(arr)
                #LOG.debug('arr.shape=%s'%([arr.shape]))

                cntarr = self.bin( arr, self.startChanCont, self.nChanCont, self.chanStepCont )
                #LOG.debug('cntarr.shape=%s'%[cntarr.shape])
                arrs.append(cntarr)
                            
            # Create continuum image
            map=numpy.concatenate(arrs,axis=3)
            contname=outprefix.rstrip('/')+'.'+self.postfix+'.cont.image'
            # 2011/03/15 TN
            # Data product naming convention
            if self.productName is not None:
                #contname = self.productName.getImageName( self.antennaid, self.poltype, polids=polids, spwid=self.spwid, cont=True )
                contname = self.productName.getImageName( self.antennaname, self.poltype, polids=polids, spwid=self.spwid, cont=True )
            #LOG.debug('map.shape=%s'%([map.shape]))
            casatools.image.fromarray(outfile=contname,pixels=map,overwrite=True)
            self.LogMessage('INFO',Origin=origin,Msg='Created %s'%contname)
            self.contimage=contname
            
            # mask blank value
            casatools.image.open(contname)
            blankval=casatools.image.getchunk().min()
            #LOG.debug('cont: blankval %s'%blankval)
            casatools.image.calcmask('"%s"!=%s'%(contname,blankval),name='mask0',asdefault=True)
##             casatools.image.calcmask('"%s"==%s'%(contname,blankval),name='imask',asdefault=True)

            # multiply channel width [km/s]
            # casatools.image.calc('"%s"*%s'%(contname,self.chanwidthvel))

##             # masked pixel should have blank value
##             casatools.image.maskhandler(op='set',name='imask')
##             casatools.image.calc( 'max( "%s", %s )'%(contname,NoData) )
##             casatools.image.maskhandler(op='set',name='mask0')
##             casatools.image.maskhandler(op='delete',name='imask')

            # divide by number of channel summed
            casatools.image.calc('"%s"/%s'%(contname,self.chanStepCont))

            # modify spectral coordinate
            coords=casatools.coordsys.newcoordsys()
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
            casatools.image.setcoordsys( csrec )
        
            # set brightness unit
            casatools.image.setbrightnessunit('K')

            casatools.image.close()
            casatools.image.done()

            # fits image
            fitsname = contname.replace('.im','.fits')
            self._createfits( contname, fitsname, False )

    def configurecoords( self ):
        """
        Configure coordinate system from filename.
        """

        origin='configurecoords()'

        filename=self.spfiles[self.spfiles.keys()[0]]

        # create coordinate system
        coords=casatools.coordsys.newcoordsys()

        # add direction and spectral coordinate system
        coords.addcoordinate( direction=True, spectral=True )
        
        # get meta data for coordinate system
        #self._tb.open(filename)
        with casatools.TableReader(filename) as tb:
            antname=tb.getkeyword('AntennaName')
            if antname.find('OSF') != -1:
                antname='OSF'
            elif antname.find('ALMA') != -1:
                antname='ALMA'
            elif antname.find('APEX') != -1:
                antname='APEX'
            elif antname.find('MRT') != -1:
                antname='IRAM_30m'
            observer=tb.getkeyword('Observer')
            obsdate=tb.getkeyword('UTC')
            if obsdate > 1.0e9:
                # sec->day
                obsdate /= 86400.0
            molid=tb.getcell('MOLECULE_ID',0)
            freqid=tb.getcell('FREQ_ID',0)
            self.nchan=len(tb.getcell('SPECTRA',0))
            freqtab=tb.getkeyword('FREQUENCIES').lstrip('Table: ')
            moltab=tb.getkeyword('MOLECULES').lstrip('Table: ')
        #self._tb.close()

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
        #LOG.debug('stokes=%s'%stokes)
        #coords.setstokes(stokes)
        coords.addcoordinate(stokes=stokes2)

        # Direction
        # axes are automatically set as RA and DEC
        # conversion system is automatically set as J2000
        refpix=[float(self.nGridX-1)/2.0,float(self.nGridY-1)/2.0]
        #LOG.debug('cell=%s'%self.cellSize)
        #LOG.debug('center=%s'%self.mapCenter)
        #LOG.debug('refpix=%s'%refpix)
        coords.setdirection( refpix=refpix, refval=self.mapCenter, incr=self.cellSize)

        # Spectral
        #self._tb.open(freqtab)
        with casatools.TableReader(freqtab) as tb:
            idCol=tb.getcol('ID')
            idx=-1
            for id in idCol:
                if id == freqid:
                    idx=id
                    break
            frefpix=tb.getcell('REFPIX',idx)
            if (self.isTP == False) and (len(self.edge) != 0) and (self.edge[0] != 0):
                frefpix = frefpix - self.edge[0]
            frefval=tb.getcell('REFVAL',idx)
            fincr=tb.getcell('INCREMENT',idx)
        #self._tb.close()
        #self._tb.open(moltab)
        with casatools.TableReader(moltab) as tb:
            rfs=tb.getcell('RESTFREQUENCY',molid)
            rf=frefval
            if len(rfs) != 0 :
                rf=rfs[0]
        #self._tb.close()
        #LOG.debug('restfreq=%s'%rf)
        coords.setspectral(restfreq=rf,doppler='radio')
        coords.setreferencepixel(value=[frefpix],type='spectral')
        coords.setreferencevalue(value='%sHz'%frefval,type='spectral')
        coords.setincrement(value='%sHz'%fincr,type='spectral')
        

        # Observer
        #LOG.debug('observer=%s'%observer)
        coords.setobserver( observer )

        # Telescope
        #LOG.debug('telescope=%s'%antname)
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
        #LOG.debug('antname=%s'%antname)
        position=casatools.measures.observatory(antname)
        #LOG.debug('position=%s'%position)
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
        #LOG.debug('channel width in velocity: %s km/s'%self.chanwidthvel)

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
            #LOG.debug('self.startChanCont=%s'%self.startChanCont)
            #LOG.debug('self.nChanCont=%s'%self.nChanCont)
            #LOG.debug('self.chanStepCont=%s'%self.chanStepCont)            


    def postfix( self ):
        origin='postfix()'

        filename=self.spfiles[self.spfiles.keys()[0]]

        #self._tb.open(filename)
        with casatools.TableReader(filename) as tb:
            ifno=tb.getcell('IFNO',0)
        #self._tb.close()

        return 'spw%s'%ifno

    def moments( self, moments=[] ):
        """
        Create moment images for created images.
        """
        origin='moments()'

        # remove moments=4 (median coordinate) since it causes exception 
        badmoments = [4]
        for moment in badmoments:
            if moment in moments:
                LOG.warning('Moment %s may cause exception, remove from the moment list.'%moment)
                while ( moment in moments ): 
                    moments.pop(moments.index(moment))

        # skip TP data
        if self.isTP:
            LOG.info('Moment images are not created since it is total power data.')
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

    def calcmoments( self, filename=None, moments=[] ):
        """
        Create moment images.
        """
        origin='calcmoments()'

        if (filename is not None) and (type(moments)==list) and (len(moments) >0):
            LOG.info('Creating moment image(s): moments=%s'%moments)
            
            casatools.image.open(filename)
            if len(moments) == 1:
                # If len(moments) is 1 and outfile is empty, output image is
                # created only on the memory temporarily. Thus, outfile must
                # be explicitly given to create output image on disk.
                # 2010/02/16 TN
                moutname = filename + '.moment%s'%(moments[0])
                mimage = casatools.image.moments(moments=moments, outfile=moutname, overwrite=True)
                mimage.close()
                del mimage
                
                # fits image
                fitsname = moutname.replace('.im','.fits')
                self._createfits( moutname, fitsname, False )

            else:
                mimage = casatools.image.moments(moments=moments, overwrite=True)
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
            LOG.info('Moment image(s) were created')
            casatools.image.close()
            casatools.image.done()
        else:
            LOG.info('Moment image(s) were not created because of bad arguments: filename=%s, moments=%s'%(filename,moments))
            
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
            LOG.warning('No images in %s'%dirname )
            return False

        # create FITS images
        for fname in images:
            infile = dirname.rstrip('/')+'/'+fname
            outfile = dirname.rstrip('/')+'/'+fname.rstrip('/')+'.fits'
            LOG.info('Creating %s'%outfile )
            casatools.image.open(infile)
            casatools.image.tofits(outfile=outfile,optical=False,overwrite=True)
            casatools.image.close()
            
        return True

    def _createfits( self, imagename, fitsname, rmtable=True ):
        """
        """
        origin='_createfits'
        LOG.info('Creating %s'%fitsname )
        casatools.image.open(imagename)
        casatools.image.tofits(outfile=fitsname,optical=False,overwrite=True)
        casatools.image.close()
        if rmtable:
            rmtables([imagename])

    def setAntennaName( self, ant ):
        self.antennaname = ant

    def setProductName( self, product ):
        """
        """
        self.productName = product

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

    def setSpectralWindowId( self, id ):
        """
        """
        self.spwid = id
