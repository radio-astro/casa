"""Module to derive the spectrum the point of maximum continuum emission in
   the data 'view'."""

# History:
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release, renamed from spectrumAtMax.
#  2-Jun-2008 jfl 2-jun release.
# 25-Jun-2008 jfl regression release.
# 26-Jun-2008 jfl added source offset.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import copy as python_copy
from numpy import *
import pickle
import types

# alma modules

from baseDataModifier import *

class SourceSpectra(BaseDataModifier):
    """Class to calculate the median and MAD of a data 'view' and return it 
    as a 'view'.
    """

    def _pixelSpectrum(self, v, data_desc_id, description, results):  
        """Private method to calculate the spectrum from the pixel closest
        to the centre of a source.
  
        Keyword arguments:
        v           -- dictionary with cube info.
        description -- dictionary with description of data item.
        results     -- structure holding data results.
        """

# were any sources found in this area

        if v.has_key('sources') and len(v['sources']) > 0:
            self._image.open(infile=v['cleanMapName'])
            csys = self._image.coordsys()
            crpix = csys.referencepixel()['numeric'];
            cdelt = csys.increment()['numeric']; 
            image_shape = self._image.shape()
            empty_channels = v['emptyChannels']
            noisy_channels = v['noisyChannels']
            bandpassFlaggingStage = v['bandpassFlaggingStage']

            for i,source in enumerate(v['sources']):
                if source['outsideMap']:
                    continue

# centre pixel

                maxPos = [self.nint(source['xPosition']),
                 self.nint(source['yPosition'])]

                xaxis = (arange(image_shape[0]) - crpix[0]) * \
                 cdelt[0] * 180.0 * 3600.0 / math.pi
                xunits = 'arcsec'  
                yaxis = (arange(image_shape[1]) - crpix[1]) * \
                 cdelt[1] * 180.0 * 3600.0 / math.pi
                yunits = 'arcsec'
 
                if v.has_key('fieldName'):
                    description['TITLE'] = \
                     'Field:%s SpW:%s Stokes:I Source:%s (xoff=%5.2f%s,yoff=%5.2f%s) - pixel spectrum' % (
                     v['fieldName'], self._pad(data_desc_id), self._pad(i),
                     xaxis[maxPos[0]], xunits, yaxis[maxPos[1]], yunits)
                else:
                    description['TITLE'] = \
                     'SpW:%s Stokes:I Source:%s (xoff=%5.2f%s, yoff=%5.2f%s) - pixel spectrum' % (
                     self._pad(data_desc_id), self._pad(i),
                     xaxis[maxPos[0]], xunits, yaxis[maxPos[1]], yunits)

                spectrum = self._image.getchunk(blc=maxPos, trc=maxPos, dropdeg=True)
                if len(spectrum) < 2:
                    continue               # continuum

                spec = zeros([1, len(spectrum)])
                spec[0] = spectrum
                flag = zeros([1,len(spectrum)], int)
                flag[0, empty_channels] = 1

                result = {}
                result['dataType'] = 'flux'
                result['dataUnits'] = 'Jy'
                result['xtitle'] = 'channel'
                result['x'] = arange(len(spectrum))
                result['ytitle'] = 'flux'
                result['y'] = arange(1)
                result['data'] = spec
                      
                result['chunks'] = []
                result['mad_floor'] = zeros([1, len(spectrum)])
                result['flag'] = [zeros([1,len(spectrum)], int), flag]
                result['flagVersions'] = ['BeforeHeuristics','Current']
                result['noisyChannels'] = noisy_channels
                result['bandpassFlaggingStage'] = bandpassFlaggingStage

                pickled_description = pickle.dumps(description)
                results['data'][pickled_description] = result

            self._image.close()


    def _boxSpectrum(self, v, data_desc_id, description, results): 
        """Private method to calculate the spectrum from the pixel averaged
        over the clean box associated with each source.
  
        Keyword arguments:
        v           -- dictionary with cube info.
        description -- dictionary with description of data item.
        results     -- structure holding data results.
        """

# were any sources found in this area

        if v.has_key('sources') and len(v['sources']) > 0:
            self._image.open(infile=v['cleanMapName'])
            csys = self._image.coordsys()
            crpix = csys.referencepixel()['numeric'];
            cdelt = csys.increment()['numeric']; 
            image_shape = self._image.shape()
            empty_channels = v['emptyChannels']
            noisy_channels = v['noisyChannels']
            bandpassFlaggingStage = v['bandpassFlaggingStage']

            for i,source in enumerate(v['sources']):
                if source['outsideMap']:
                    continue

                if source.has_key('box'):
                    maxPos = [self.nint(source['xPosition']),
                     self.nint(source['yPosition'])]

                    xaxis = (arange(image_shape[0]) - crpix[0]) * \
                     cdelt[0] * 180.0 * 3600.0 / math.pi
                    xunits = 'arcsec'
                    yaxis = (arange(image_shape[1]) - crpix[1]) * \
                     cdelt[1] * 180.0 * 3600.0 / math.pi
                    yunits = 'arcsec'

                    if v.has_key('fieldName'):
                        description['TITLE'] = \
                         'Field:%s SpW:%s Stokes:I Source:%s (xoff=%5.2f%s,yoff=%5.2f%s) - box spectrum' % (
                         v['fieldName'], self._pad(data_desc_id),
                         self._pad(i), xaxis[maxPos[0]], xunits,
                         yaxis[maxPos[1]], yunits)
                    else:
                        description['TITLE'] = \
                         'SpW:%s Stokes:I Source:%s (xoff=%5.2f%s,yoff=%5.2f%s) - box spectrum' % (
                         self._pad(data_desc_id),
                         self._pad(i), xaxis[maxPos[0]], xunits,
                         yaxis[maxPos[1]], yunits)

                    blc = [int(source['box'][0]), int(source['box'][1])]
                    trc = [int(source['box'][2]), int(source['box'][3])]

                    spectrum = self._image.getchunk(blc=blc, trc=trc,
                     dropdeg=True)
                    if len(shape(spectrum)) < 3:
                        continue           # box covers 1 pixel

                    spec = zeros([1, shape(spectrum)[2]])
                    for ispec in range(shape(spectrum)[2]):
                       spec[0,ispec] = mean(spectrum[:,:,ispec].ravel())

                    flag = zeros(shape(spec), int)
                    flag[0, empty_channels] = 1

                    result = {}
                    result['dataType'] = 'flux'
                    result['dataUnits'] = 'Jy'
                    result['xtitle'] = 'channel'
                    result['x'] = arange(shape(spectrum)[2])
                    result['ytitle'] = 'flux'
                    result['y'] = arange(1)
                    result['data'] = spec
                    
                    result['chunks'] = []
                    result['mad_floor'] = zeros([1, shape(spectrum)[2]])
                    result['flag'] = [
                     zeros([1,shape(spectrum)[2]], int), flag]
                    result['flagVersions'] = ['BeforeHeuristics',
                     'Current']
                    result['noisyChannels'] = noisy_channels
                    result['bandpassFlaggingStage'] = \
                     bandpassFlaggingStage

                    pickled_description = pickle.dumps(description)
                    results['data'][pickled_description] = result

            self._image.close()


    def description(self):
        description = ['SourceSpectra - spectra from detected sources']
        return description


    def getData(self, topLevel=False):
        """Public method that returns the data 'view'.
  
        Keyword arguments:
        topLevel -- True if this is the data view to be directly displayed.
        """

#        print 'SourceSpectra.getData called'
        self._htmlLogger.timing_start('SourceSpectra.getData')

# have these data been calculated already?

        entryID,results = self._bookKeeper.available(
         objectType='SourceSpectra', sourceType=None,
         furtherInput={},
         outputFiles=[],
         dependencies=[self._view.inputs()])

        if entryID == None:

# calculate the target view

            viewParameters = self._view.calculate()

            results = {
             'parameters':{'history':'',
                           'dependencies':{}},
             'data':{}}

            results['parameters']['history'] = self._fullStageName
            results['parameters']['dependencies']['view'] = viewParameters

            for k,v in viewParameters['data'].iteritems():
                if type(k) == types.TupleType:

# results for a data_desc_id/field_id

                    field_id = k[0]
                    data_desc_id = k[1]
                    description = {}
                    description['FIELD_ID'] = str(field_id)
                    description['DATA_DESC_ID'] = str(data_desc_id)

                    self._pixelSpectrum(v, data_desc_id, description, results)
                    self._boxSpectrum(v, data_desc_id, description, results)

                elif type(k) == types.IntType:
                    data_desc_id = k

# results for a series of mosaic groups in each data_desc_id

                    for kk,vv in v.iteritems():
                        description = {}
                        description['DATA_DESC_ID'] = str(data_desc_id)

                        self._pixelSpectrum(vv, data_desc_id, description, results)
                        self._boxSpectrum(vv, data_desc_id, description, results)

                else:
                    raise NameError, 'unknown key type: %s' % k

# store the object info in the BookKeeper
                
            self._bookKeeper.enter(objectType='SourceSpectra',
             sourceType=None,
             furtherInput={},
             outputFiles=[],
             outputParameters=results,
             dependencies=[self._view.inputs()])

# now add the latest results to the returned structure

        flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()
        self._results['flagging'].append(flagging)
        self._results['flaggingReason'].append(flaggingReason)
        self._results['flaggingApplied'].append(flaggingApplied)

        for k in results['data'].keys():
            if self._results['data'].has_key(k):
                self._results['data'][k].append(results['data'][k])
            else:
                self._results['data'][k] = [results['data'][k]]
             
# add history and dependency info
             
        self._results['parameters'] = results['parameters']

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('SourceSpectra.getData')
        return temp


    def inputs(self):
        """Method to return the input settings for this object.
        """
        result = {}
        result['objectType'] = 'SourceSpectra'
        result['sourceType'] = None
        result['furtherInput'] = {}
        result['outputFiles'] = []
        result['dependencies'] = [self._view.inputs()]
        result['flag_marks'] = {}

        return result


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

# Output any description written as part of the recipe.

        if self._description != None:
            self._htmlLogger.logHTML('<p> %s' % self._description)

        self._htmlLogger.logHTML("""
         <p>The data view was a list of spectra taken from a cleaned
         cube. Each 'pixel spectrum' was taken from the cube at the 
         pixel position of a 'source' detected during the cleaning
         process. Each 'box spectrum' was the mean spectrum over the
         pixels in the corresponding clean box.""")


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        topLevel   -- True if this data 'view' is to be displayed directly,
                      not passing through a data modifier object.
        parameters -- Dictionary that holds the descriptive information.
        """

        if parameters == None:
            parameters = self._results['parameters']

        if topLevel:
            self._htmlLogger.logHTML("""
             <h3>Data View</h3>""")

# Output any description written as part of the recipe.

            if self._description != None:
                self._htmlLogger.logHTML('<p> %s' % self._description)

        self._htmlLogger.logHTML("""
         <p>The data view was a list of spectra taken from a cleaned
         cube. Each 'pixel spectrum' was taken from the cube at the 
         pixel position of a 'source' detected during the cleaning
         process. Each 'box spectrum' was the mean spectrum over the
         pixels in the corresponding clean box.""")

#         <p>The view was calculated by Python class SourceSpectra.""")

        self._htmlLogger.logHTML("""
         <h5>The Clean Cube</h5>""")
        self._view.writeDetailedHTMLDescription(stageName, False,
         parameters=parameters['dependencies']['view'])
