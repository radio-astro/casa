"""Module with Base Class for image displays."""

# History:
# 16-Jul-2007 jfl First version.
#  9-Aug-2007 jfl image.csys() replaced by image.coordsys()
# 30-Aug-2007 jfl Flux calibrating release.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
#  9-Jan-2008 jfl Can handle list of clean boxes.
# 16-Jan-2008 jfl composite number stuff added.
# 31-Jan-2008 fb  File forms added for separate continuum/line cleaning. 
# 20-Mar-2008 jfl BookKeeper release.
#  1-May-2008 jfl maxNpixels removed.
# 13-May-2008 jfl 13 release.
# 16-May-2008 jfl image ref added.
# 25-Jun-2008 jfl regression release.
# 10-Sep-2008 jfl msCalibrater release.
# 26-Sep-2008 jfl mosaic release.
# 10-Oct-2008 jfl complex display release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import copy as python_copy
import math
from numpy import *
import os
import pickle

# alma modules

from baseData import *
import deviations

class BaseImage(BaseData):
    """Base Class for image displays."""

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, verbose=False):
        """Constructor.

        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- The name of the stage using the object
        sourceType  -- Type of source to be imaged.
        verbose     -- True if want more processing info logged to screen.
        """

#        print 'BaseImage.__init__ called: %s, %s' % (msName, sourceType)

        BaseData.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName, verbose)

        self._htmlLogger.timing_start('BaseImage.__init__')
        self._sourceType = sourceType

# get range of data_desc_ids - make sure they're int not int32

        success = self._ms.open(thems=self._msName)
        msInfo = self._ms.range(items=['data_desc_id'])
        self._data_desc_ids = map(int, list(msInfo['data_desc_id']))
        success = self._ms.close()
        
# get map of data_desc_id to spectral_window_id

        rtn = self._table.open(self._msName + '/DATA_DESCRIPTION')
        self._spectral_window_id = self._table.getcol(columnname=
         'SPECTRAL_WINDOW_ID')
        rtn = self._table.close()

# and reference frequencies for each spectral_window_id
# this will be required to compute the Field Of View

        rtn = self._table.open(self._msName + '/SPECTRAL_WINDOW')
        self._ref_frequency = self._table.getcol(columnname='REF_FREQUENCY')
        self._num_chan = self._table.getcol(columnname='NUM_CHAN')
        rtn = self._table.close()

# get antenna diameters

        rtn = self._table.open(self._msName + '/ANTENNA')
        dish_diameter = self._table.getcol(columnname='DISH_DIAMETER')
        rtn = self._table.close()
        smallestDiameter = min(dish_diameter)

# get names of fields

        rtn = self._table.open(self._msName + '/FIELD')
        self._fieldName = self._table.getcol('NAME')
        rtn = self._table.close()
 
# translate source type to field Ids

        self._target_field_ids = self.getFieldsOfType(self._sourceType)

# get fields of view for each data_desc

        self._fieldofview = {}
        for data_desc_id in self._data_desc_ids:

# use the smallest antenna diameter and the reference frequency to estimate the
# field of view with FOV = 2*1.22*lambda/D

            freq = self._ref_frequency[self._spectral_window_id[data_desc_id]]
            self._fieldofview[data_desc_id] = 2.0 * \
            (1.22 * (3.0e8/freq) / smallestDiameter) * \
            (180.0 * 3600.0 / math.pi)

# generate list of even composite numbers, max image size

        self._generateCompositeEvenNumbers(250000)

        self._htmlLogger.timing_stop('BaseImage.__init__')
  

    def _generateCompositeEvenNumbers(self, maxval):
        """Utility method to generate a range of composite even numbers up to 
        the limit given. These are made up of powers of 2, 3 and 5 in the 
        same ways as the C++ class CompositeNumber in CASA.
        """
        n2 = int(log(float(maxval)) / log(2.0)) + 1
        n3 = int(log(float(maxval)) / log(3.0)) + 1
        n5 = int(log(float(maxval)) / log(5.0)) + 1

        temp = []
        n = 0
        for i2 in range(1,n2):
            for i3 in range(n3):
                for i5 in range(n5):
                    newNumber = int(pow(2.0,i2) * pow(3.0,i3) * pow(5.0,i5))
                    if newNumber < maxval:
                        temp.append(newNumber)
        temp.sort()
        self._itsNumbers = array(temp)
        self._itsNumbersIndex = arange(len(self._itsNumbers))


    def _nextLargerCompositeNumber(self, val):
        """Returns the next larger number to val in the composite number list.
        """
        nextIndex = compress(self._itsNumbers>=val, self._itsNumbersIndex)
        if len(nextIndex) == 0:
             raise NameError, 'bad composite number value %s' % val
        return self._itsNumbers[nextIndex[0]]              


    def _fillData(self, description, dataType, dataDictionary, mapField, 
     error_key, cleanBoxField=None, quarterBoxField=None, thresholdField=None,
     rms2dField=None, 
     additional_info={}, noisyChannelField=None,
     bandpassFlaggingStageField=None, emptyChannelField=None):
        """Utility method to fill the results structure from the current 
        image.

        Keyword arguments:
        error_key         -- key in error dictionary that applies to this
                             data product.
        noisyChannelField -- Channels designated as lying at the 'edge' of the
                             bandpass where S/N is lower.
        bandpassFlaggingStageField -- Name of stage where noisy channels 
                             detected.
        emptyChannelField -- Channels for which there was no data in the MS.
                             These will have been set to zero in the image.
                             Don't include them in calculating the integrated
                             map.
        """

#        print 'BaseImage._fillData() called', description['TITLE']
        self._htmlLogger.timing_start('BaseImage._fillData')

        if dataDictionary['error'][error_key] == None:
            flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()

# get the current data

            map_msName = dataDictionary[mapField]
            self._image.open(map_msName)

# get data shape

            data_shape = self._image.shape();

# get image summary information

            rtn=self._image.summary(doppler='RADIO', list=False,
             pixelorder=True)

# get coordinate system

            csys = self._image.coordsys()
            crval = csys.referencevalue()['numeric'];
            crpix = csys.referencepixel()['numeric'];
            cdelt = csys.increment()['numeric'];
            image_ref = self._image.toworld(format='s')['string']

# get the whole image averaging over axis 3 - exclude flagged channels

# first, find the 'noisy' channels

            noisy_channels = None
            bandpassFlaggingStage = None
            if noisyChannelField != None:
                noisy_channels = dataDictionary[noisyChannelField]
                bandpassFlaggingStage = dataDictionary[
                 bandpassFlaggingStageField]

# and those channels known to have no data

            empty_channels = None
            if emptyChannelField != None:
                empty_channels = dataDictionary[emptyChannelField]

# ..both should be flagged in the integrated map

            flag_channels = None
            if noisy_channels != None and empty_channels==None:
                flag_channels = list(noisy_channels)
            elif noisy_channels == None and empty_channels!=None:
                flag_channels = list(empty_channels)
            elif noisy_channels != None and empty_channels!=None:
                flag_channels = list(noisy_channels) + list(empty_channels)

            print 'edge channels', noisy_channels
            print 'empty channels', empty_channels
            print 'flag_channels', flag_channels

# remove duplicate channels and sort

            temp = []
            if flag_channels != None:
                for channel in flag_channels:
                    if temp.count(channel) == 0:
                        temp.append(channel)
                flag_channels = temp
                flag_channels.sort()

            if flag_channels != None:
                good_channels = range(self._image.shape()[3])
                for flag_chan in flag_channels:
                    good_channels.remove(flag_chan)

# find limits of channel regions

                channelRegions = []
                startChannel = None
                for channel in good_channels:
                    if startChannel == None:
                        startChannel = channel
                        endChannel = channel
                    else:
                        if channel == endChannel + 1:
                            endChannel = channel
                        else:
                            channelRegions.append([startChannel,endChannel])
                            startChannel = channel
                            endChannel = channel
                if startChannel != None:
                    channelRegions.append([startChannel,endChannel])

# use indexin to exclude bandpass flagged channels from map integration

                dims = self._image.shape()
                mask = 'indexin(3, ['
                for region in channelRegions:
                    mask += '%s:%s,' % (region[0], region[1])
                mask = mask[:-1] + '])'
  
                pixels = self._image.getregion(mask=mask, axes=[3],
                 list=True, dropdeg=True)
            else:

# no channel flags, just get the whole image averaged over all channels

                pixels = self._image.getchunk(axes=[3], list=True, dropdeg=True)
            self._image.close()

            result = {}
            result['dataType'] = dataType
            result['dataUnits'] = 'Jy/beam'
            result['xtitle'] = 'RA offset'
            result['x'] = (arange(shape(pixels)[0]) - crpix[0]) * \
             cdelt[0] * 180.0 * 3600.0 / math.pi
            result['xunits'] = 'arcsec'
            result['ytitle'] = 'Dec offset'
            result['y'] = (arange(shape(pixels)[1]) - crpix[1]) * \
             cdelt[1] * 180.0 * 3600.0 / math.pi
            result['yunits'] = 'arcsec'  
            result['image_ref'] = image_ref
            if thresholdField != None:
                result['threshold'] = dataDictionary[thresholdField]
            if rms2dField != None:
                result['rms2d'] = dataDictionary[rms2dField] 
            result['data'] = pixels
            result['flag'] = zeros(shape(pixels), int)
            result['flag_on_entry'] = zeros(shape(pixels), int)
            result['flagging'] = flagging
            result['flaggingReason'] = flaggingReason
            result['flaggingApplied'] = flaggingApplied
            result['emptyChannels'] = empty_channels
            result['noisyChannels'] = noisy_channels
            result['bandpassFlaggingStage'] = bandpassFlaggingStage
            if cleanBoxField != None and dataDictionary.has_key(cleanBoxField):
                clean_box = dataDictionary[cleanBoxField]
                result['clean_box'] = []
                for box in clean_box:
                    result['clean_box'].append([
                     (box[0] - crpix[0]) * cdelt[0] * 180.0 * 3600.0 / math.pi,
                     (box[1] - crpix[1]) * cdelt[1] * 180.0 * 3600.0 / math.pi,
                     (box[2] - crpix[0]) * cdelt[0] * 180.0 * 3600.0 / math.pi,
                     (box[3] - crpix[1]) * cdelt[1] * 180.0 * 3600.0 / math.pi])
            if quarterBoxField != None and dataDictionary.has_key(quarterBoxField):
                box = dataDictionary[quarterBoxField]
                result['clean_limit_box'] = [
                 (box[0] - crpix[0]) * cdelt[0] * 180.0 * 3600.0 / math.pi,
                 (box[1] - crpix[1]) * cdelt[1] * 180.0 * 3600.0 / math.pi,
                 (box[2] - crpix[0]) * cdelt[0] * 180.0 * 3600.0 / math.pi,
                 (box[3] - crpix[1]) * cdelt[1] * 180.0 * 3600.0 / math.pi]

            for k, v in additional_info.iteritems():
                result[k] = v
        else:
            result = {}
        result['error'] = dataDictionary['error'][error_key]

        pickled_description = pickle.dumps(description)
        if self._results['data'].has_key(pickled_description):
            self._results['data'][pickled_description].append(result)
        else:
            self._results['data'][pickled_description] = [result]

        self._htmlLogger.timing_stop('BaseImage._fillData')


