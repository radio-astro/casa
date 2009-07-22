"""Module to supply dirty images."""

# History:
# 26-Sep-2008 jfl First version.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import math
from numpy import *

# alma modules

from baseImage import *

class DirtyImageV2(BaseImage):
    """Class to supply dirty images."""

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger,
     htmlLogger, msName, stageName, sourceType, mode):
        """Constructor.

        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- Name of stage using this object.
        sourceType  -- The type of source.
        mode        -- 'mode' parameter in imager tool.
        """

#        print 'DirtyImage.__init__ called'
        BaseImage.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName, sourceType)
        self._mode = mode


    def calculate(self, field, spw, cell, nx):
        """Method to calculate the dirty image for the specified field and spw.

        Keyword arguments:
        field -- The field id.
        spw   -- The spectral window id.
        cell  -- The cell size to be used.
        nx    -- The number of pixels along each side of the square.
        """
#        print 'DirtyImageV2.calculate called'
        self._htmlLogger.timing_start('DirtyImageV2.calculate')

# this class does not store itself in the BookKeeper.

        parameters = {'commands':[],
                      'error':None}
        commands = []

# select the data to be used

        self._imager.open(thems=self._msName)
        self._imager.selectvis(spw=int(spw), field=int(field))

        commands.append('imager.open(thems=%s)' % self._msName)
        commands.append('imager.selectvis(spw=int(%s), field=int(%s))' % 
         (spw, field))

# generate names of files to hold results

        flag_mark = self._msFlagger.getFlagMarkDict(field)
        dirtyMapName = 'dirty.%s.f%s.spw%s.fm%s' % (self._base_msName, 
         self._fieldName[field], spw, flag_mark)
        dirtyMapName.replace(' ', '')
        parameters['mapName'] = dirtyMapName

        complexDirtyMapName = 'complexDirty.%s.f%s.spw%s.fm%s' % (
         self._base_msName, self._fieldName[field], spw, flag_mark)
        complexDirtyMapName.replace(' ', '')
        parameters['complexMapName'] = complexDirtyMapName

# set spectral image properties

        if self._mode=='channel':
            nchan = self._num_chan[self._spectral_window_id[spw]]
        else:
            nchan = 1

# set the image parameters

        self._imager.defineimage(nx=nx, ny=nx, cellx=cell,celly=cell,
         stokes='I', phasecenter=int(field), mode=self._mode, nchan=nchan,
         spw=[int(spw)])
        self._imager.summary()

        commands.append('''imager.defineimage(nx=%s, ny=%s, cellx=%s, celly=%s,
         stokes='I', phasecenter=int(%s), mode=%s, nchan=%s, spw=[int(%s)])''' % (nx, nx,
         cell, cell, field, self._mode, nchan, spw))
        commands.append('imager.summary()')

        parameters['nx'] = nx
        parameters['cell'] = cell
        parameters['nchan'] = nchan

# make the dirty image

        self._imager.makeimage(type='corrected', image=dirtyMapName,
         compleximage=complexDirtyMapName)
        self._imager.close()

        commands.append("""imager.makeimage(type='corrected', image=%s,
         compleximage=%s)""" % (dirtyMapName, complexDirtyMapName))
        commands.append('imager.close()')

        parameters['commands'] = commands

        self._htmlLogger.timing_stop('DirtyImageV2.calculate')
        return parameters
 

    def createGeneralHTMLDescription(self, stageName):
        """Write a general description of the class.
   
        Keyword arguments: 
        stageName -- The name of the stage using the class.
        """
        description = 'The dirty image was calculated.'
        return description


    def createDetailedHTMLDescription(self, stageName):
        """Write a detailed description of the class.
   
        Keyword arguments: 
        stageName -- The name of the stage using the class.
        """
        description = """
         The dirty image was constructed (see Casapy Calls for details)."""
#         This work was done by Python class DirtyImageV2."""

        return description
