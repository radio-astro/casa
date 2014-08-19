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
        self.reshaped_data = None
        self._imagename = None
        self.edge = list(common.parseEdge(edge))

    @property
    def nchan(self):
        return self.data.shape[-1]

    @property
    def isaveraged(self):
        return (self.nchan == 1)

    @property
    def imagename(self):
        return self._imagename

    @imagename.setter
    def imagename(self, value):
        self._imagename = value
        
    def execute(self, dry_run=True):
        if self.imagename is None:
            imagename = 'temporary_image'
        else:
            imagename = self.imagename

        if dry_run:
            return imagename
        else:
            self.full_channel_image(imagename=imagename)
            return imagename
        
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

    def _reshape_data(self):
        if len(self.data.shape) < 4:
            # let's reshape self.data
            (npol,nrow,nchan) = self.data.shape
            self.reshaped_data = self.data.transpose(0,2,1)
            self.reshaped_data = self.reshaped_data.reshape((npol,nchan,self.ny,self.nx))
            self.reshaped_data = self.reshaped_data.transpose() # (nx,ny,nchan,npol)
            self.reshaped_data = numpy.flipud(self.reshaped_data) # flip x-axis
        else:
            self.reshaped_data = self.data
            
    def _drop_channels(self):
        if self.isaveraged or len(self.edge) == 0 or self.edge == [0,0]:
            return self.reshaped_data
        else:
            return self.reshaped_data[:,:,self.edge[0]:self.nchan-self.edge[1],:]
        
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
            
    def define_image(self, grid_table, freq_refpix, freq_refval, freq_increment, freq_frame='LSRK', rest_frequency=None, antenna='', observer='', obs_date=0.0, stokes='XX YY'):
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

        # shift refval according to the edge
        if not self.isaveraged:
            #freq_refpix = freq_refpix - self.edge[0]
            freq_refval = freq_refval + self.edge[0] * freq_increment
        
        self.coords_rec = self._configure_coords(self.nx, self.ny, self.cellx, self.celly, self.center, freq_refpix, freq_refval, freq_increment, freq_frame, rest_frequency, antenna, observer, obs_date, stokes)
        
    def _configure_coords(self, nx, ny, cellx, celly, center, freq_refpix, freq_refval, freq_increment, freq_frame='LSRK', rest_frequency=None, antenna='', observer='', obs_date=0.0, stokes='XX YY'):
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
        coords.setreferencecode(value=freq_frame, type='spectral', adjust=False)
        
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

