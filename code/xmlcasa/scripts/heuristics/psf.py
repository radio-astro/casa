"""Module to provide images of the point spread function."""

# History:
# 26-Sep-2008 jfl First version.
# 10-Oct-2008 jfl complex display release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import math
from numpy import *

# alma modules

from baseImage import *

class Psf(BaseData):
    """Class to provide images of the point spread function."""

    def calculate(self, field, spw, cell, nx):
        """Method to calculate the p.s.f. for the specified field and spw.

        Keyword arguments:
        field     -- The field id.
        spw       -- The spectral window id.
        cell      -- The cell size to be used.
        nx        -- The number of pixels along each side of the square.
        """

#        print 'Psf.calculate called'
        self._htmlLogger.timing_start('Psf.calculate')

# this class does not store itself in the BookKeeper.

        parameters = {'nx':nx,
                      'cell':cell,
                      'commands':[],
                      'error':None}
        commands = []

# select the data to be used
 
        self._imager.open(thems=self._msName)
        self._imager.selectvis(spw=int(spw), field=int(field))

        commands.append('imager.open(thems=%s)' % self._msName)
        commands.append('imager.selectvis(spw=int(%s), field=int(%s))' % 
         (spw, field))

# set the image parameters for the 'msf' result

        self._imager.defineimage(nx=nx, ny=nx, cellx=cell, celly=cell,
         stokes='I', phasecenter=int(field), mode='mfs', nchan=-1,
         spw=[int(spw)])
        self._imager.summary()

        commands.append('''imager.defineimage(nx=%s, ny=%s, cellx=%s, celly=%s,
         stokes='I', phasecenter=int(%s), mode='mfs', nchan=-1, spw=[int(%s)])''' % (nx,
         nx, cell, cell, field, spw))
        commands.append('imager.summary()')

# compose output file name

        flag_mark = self._msFlagger.getFlagMarkDict(field)
        psfMapName = 'psf.%s.f%s.spw%s.fm%s' % (self._base_msName, 
         self._fieldName[field], spw, flag_mark)
        psfMapName.replace(' ', '')

# compute the psf

        self._imager.makeimage(type='psf', image=psfMapName)
        parameters['mapName'] = psfMapName

        commands.append("imager.makeimage(type='psf', image=%s)" % psfMapName)

# fit the beam, may fail

        try:
            rtn = self._imager.fitpsf(psf=psfMapName)
            commands.append('imager.fitpsf(psf=%s)' % psfMapName)

            parameters['bmaj'] = '%4.1f%s' % (rtn['bmaj'].value,
             rtn['bmaj'].units)
            parameters['bmin'] = '%4.1f%s' % (rtn['bmin'].value, rtn['bmin'].units)
            parameters['bpa'] = '%4.1f%s' % (rtn['bpa'].value, rtn['bpa'].units)

            print '..beam parameters :(bmaj=%s, bmin=%s, bpa=%s)' % (
             parameters['bmaj'], parameters['bmin'], parameters['bpa'])

        except KeyboardInterrupt:
            self._imager.close()
            commands.append('imager.close()')
            raise
        except:
            parameters['error'] = 'beam fit failed'

# now construct the psf cube

        self._imager.defineimage(nx=nx, ny=nx, cellx=cell, celly=cell,
         stokes='I', phasecenter=int(field), mode='channel', 
         nchan=self._results['summary']['nchannels'][spw], spw=[int(spw)])
        self._imager.summary()

        commands.append('''imager.defineimage(nx=%s, ny=%s, cellx=%s, celly=%s,
         stokes='I', phasecenter=int(%s), mode='channel', nchan=%s,
         spw=[int(%s)])''' % (nx, nx, cell, cell, field,
         self._results['summary']['nchannels'][spw], spw))
        commands.append('imager.summary()')

# compose output file name

        flag_mark = self._msFlagger.getFlagMarkDict(field)
        psfCubeName = 'psfCube.%s.f%s.spw%s.fm%s' % (self._base_msName, 
         self._fieldName[field], spw, flag_mark)
        psfCubeName.replace(' ', '')

# compute the psf

        self._imager.makeimage(type='psf', image=psfCubeName)
        parameters['cubeName'] = psfCubeName

        commands.append("imager.makeimage(type='psf', image=%s)" % psfCubeName)

        self._imager.close()
        commands.append('imager.close()')

        parameters['commands'] = commands

        self._htmlLogger.timing_stop('Psf.calculate')
        return parameters


    def createGeneralHTMLDescription(self, stageName):
        """Write a general description of the class.

        Keyword arguments:
        stageName -- The name of the stage using the class.
        """
        description = 'The point spread function was calculated'
        return description


    def createDetailedHTMLDescription(self, stageName):
        """Write a detailed description of the class.

        Keyword arguments:
        stageName -- The name of the stage using the class.
        """
        description = """
         The point spread function was constructed (see Casapy Calls for 
         details)."""
#         This work was done by Python class Psf."""

        return description

