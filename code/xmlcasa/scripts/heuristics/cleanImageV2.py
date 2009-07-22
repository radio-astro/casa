"""Module to supply cleaned images."""

# History:
# 26-Sep-2008 jfl First version.
# 10-Oct-2008 jfl complex display release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
# 29-Jan-2009 jfl fixed? findsource by checking surface brightness.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import math
from numpy import *

# alma modules

from baseImage import *

class CleanImageV2(BaseImage):
    """Class to supply cleaned images.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, mode, algorithm, bandpassCal, gainCal, psf,
     dirtyImage, maxPixels=None, bandpassFlaggingStage=None, 
     bandpassCalDisplay=None, gainCalDisplay=None, verbose=False):
        """Constructor.

        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- Name of stage using this object.
        sourceType  -- Type of source to be used for the G calibration.
        mode        -- 'mode' parameter in imager tool methods.
        algorithm   -- 'algorithm' parameter in imager tool methods.
        bandpassCal -- class to be used for the bandpass calibration.
        gainCal     -- class to be used for the flux calibration
        psf         -- class to be use to calculate the psf.
        dirtyImage  -- class to be used to calculate the dirty image
        maxPixels   -- Max no of pixels on each dimension of map.
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels to be ignored in calculating
                              -- integrated map.
        bandpassCalDisplay    -- class to display the bandpass calibration.
        gainCalDisplay        -- class to display the gain calibration.
        verbose     -- True if want fuller progress reports.
        """

#        print 'CleanImageV2.__init__ called'
        BaseImage.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName, sourceType, verbose)
        self._tools = tools
        self._mode = mode
        self._algorithm = algorithm
        self._maxPixels = maxPixels

        self._nmax = 20
        self._bandpassFlaggingStage = bandpassFlaggingStage

        self._psf = psf(tools, bookKeeper, msCalibrater, msFlagger, 
         htmlLogger, msName, stageName, sourceType)
        self._dirtyImage = dirtyImage(tools, bookKeeper, msCalibrater, 
         msFlagger, htmlLogger, msName, stageName, sourceType, mode)
        if len(bandpassCal) == 1:
            self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater,
             msFlagger, htmlLogger, msName, stageName)
        else:
            self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater, 
             msFlagger, htmlLogger, msName, stageName,
             viewClassList=bandpassCal[1:])
        self._gainCal = gainCal(tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName, bandpassCal=bandpassCal,
         bandpassFlaggingStage=bandpassFlaggingStage)

        self._bandpassCalDisplay = bandpassCalDisplay
        self._gainCalDisplay = gainCalDisplay

        self._first = True


    def __del__(self):
        """Destructor
        """  

# delete the flag state holding the PreviousIteration flags.

        self._msFlagger.deleteFlagState('PreviousIteration')


    def _clean(self, data_desc_id, field_id, nx, cell, mode, nchan, 
     bmaj, bmin, bpa, boxes, modelName, cleanMapName,
     residualMapName, integrated_rms=None, quarter_box=None):
        """Private method to clean an image.

        Keyword arguments:
        data_desc_id  -- spw of map.
        field_id      -- field id of source
        nx            -- number of pixels on each side of square image.
        cell          -- string with cell size and units.
        mode          -- 'mfs' or 'channel'.
        nchan         -- number of channels in cube.
        bmaj          -- major axis of beam.
        bmin          -- minor axis of beam.
        bpa           -- position angle of beam major axis.
        boxes         -- list of clean boxes (corners, in pixels).
        modelName     -- file to hold model.
        cleanMapName  -- file to hold clean map.
        residualMapName -- file to hold residuals.
        integrated_rms  -- rms of residual from pilot 'mfs' clean.
        quarter_box   -- box with maximum extent of area to be cleaned
                         (pixels).
        """
        commands = []

        self._imager.open(self._msName)
        self._imager.selectvis(field=int(field_id), spw=int(data_desc_id))

# set the image parameters

        self._imager.defineimage(nx=nx, ny=nx, cellx=cell, celly=cell,
         stokes='I', phasecenter=int(field_id), mode=mode, nchan=nchan,
         spw=[int(data_desc_id)])

# update commands list

        commands.append('imager.open(%s)' % self._msName)
        commands.append('imager.selectvis(field=int(%s), spw=int(%s))' % 
         (field_id, data_desc_id))
        commands.append("""imager.defineimage(nx=%s, ny=%s, cellx=%s, celly=%s,
         stokes='I', phasecenter=int(%s), mode=%s, nchan=%s, spw=[int(%s)])""" % 
         (nx, nx, cell, cell, field_id, mode, nchan, data_desc_id))

# make mask from boxes - do this using 'regions' as it seems easier than
# using masks directly

        self._imager.make('empty_image')
        self._image.open('empty_image')
        csys = self._image.coordsys()
        self._image.close()

        commands.append('''# make cleaning mask, including clean boxes and maximum
         possible extent''')
        commands.append("imager.make('empty_image')")
        commands.append("image.open('empty_image')")
        commands.append("csys=image.coordsys()")
        commands.append("image.close()")

        clean_regions = {}
        for i,box in enumerate(boxes):
            boxname = 'r%s' % i
            boxregion = self._regionmanager.wbox( 
             blc='%spix %spix' % (box[0], box[1]),
             trc='%spix %spix' % (box[2], box[3]),
             pixelaxes=[0,1], csys=csys.torecord())
            clean_regions[boxname] = boxregion

            commands.append('# adding box %s' % i)
            commands.append("""boxregion=rg.wbox(blc='%spix %spix',
             trc='%spix %spic', pixelaxes=[0,1], cys=csys.torecord())""" % (
             box[0], box[1], box[2], box[3]))
            commands.append("clean_regions['%s'] = boxregion" % boxname)

        if len(clean_regions) > 1:
            clean_region_union = self._regionmanager.makeunion(clean_regions)
            commands.append("clean_region_union=rg.makeunion(clean_regions)")
        else:
            clean_region_union = clean_regions['r0']
            commands.append("clean_region_union=clean_regions['r0']")

# if necessary get the intersection of the boxes with the image quarter
# that is the maximum area to be cleaned

        if quarter_box != None:
            quarter_region = self._regionmanager.wbox(
            blc='%spix %spix' % (quarter_box[0], quarter_box[1]),
            trc='%spix %spix' % (quarter_box[2], quarter_box[3]),
            pixelaxes=[0,1], csys=csys.torecord())

            clean_region = self._regionmanager.intersection(
             {'r1':clean_region_union, 'r2':quarter_region})

            commands.append("# add maximum clean extent")
            commands.append("""clean_region=rg.wbox(blc='%spix %spix',
             trc='%spix %spic', pixelaxes=[0,1], cys=csys.torecord())""" % (
             quarter_box[0], quarter_box[1], quarter_box[2], quarter_box[3]))
            commands.append("""clean_region=rg.intersection(
             {'r1':clean_region_union, 'r2':quarter_region})""")
         
        else:
            clean_region = clean_region_union

# construct clean mask name, remove chars that confuse 'mask' parameter later

        cleanmaskName = 'cleanmask.%s' % cleanMapName
        cleanmaskName = cleanmaskName.replace('-','_')
        cleanmaskName = cleanmaskName.replace('+','_')
        cleanmaskName = cleanmaskName.replace('(','_')
        cleanmaskName = cleanmaskName.replace(')','_')

# remove previous incarnations as these do not get overwritten and can snarl
# things up 

        if os.path.exists(cleanmaskName):
            self._rmall(cleanmaskName)

        self._imager.regionmask(mask=cleanmaskName, region=clean_region)

# set the beam parameters for the clean restoration

        self._imager.setbeam(bmaj=bmaj, bmin=bmin, bpa=bpa)

# remove any previous model

        if os.path.exists(modelName):
            self._rmall(modelName)

        commands.append('# remove %s' % cleanmaskName)
        commands.append("imager.regionmask(mask='%s', region=clean_region)" %
         cleanmaskName)
        commands.append('imager.setbeam(bmaj=%s, bmin=%s, bpa=%s)' %
         (bmaj, bmin, bpa)) 
        commands.append('# remove %s' % modelName)

# start cleaning

        cleaning = True
        nloop = 0
        cleanRmsRecord = []
        if integrated_rms == None:
            rms = 0.05
            cleanRms = 1e6
            threshold = 0.1
            sum = 0.0
        else:
            rms = integrated_rms * math.sqrt(abs(float(nchan)))
            cleanRms = 1e6
            threshold = 2 * rms
            sum = 0.0
            if threshold == inf:
                print 'resetting threshold to 0.1'
                threshold = 0.1

        while cleaning:
            if self._verbose:
                print 'clean to threshold', threshold
            print 'clean to threshold', threshold

            self._imager.clean(algorithm=self._algorithm,
             mask=cleanmaskName, niter=1000,
             gain=0.2, threshold="%sJy" % (threshold),
             displayprogress=False,
             model=modelName, keepfixed=[False],
             image=cleanMapName, 
             residual=residualMapName)

# cleaned down to this level..

            newRms = -1
            sumChange = -1
            newCleanRms = -1
            rms2d = -1
            imMax = -1

# get the sum of the model image, which can be used to estimate how
# close we are to cleaning all the source that's to be found

            self._image.open(infile=modelName)
            model_stats = self._image.statistics(mask='%s > 0.1' % cleanmaskName,
             robust=False)
            newSum = model_stats['sum'][0]
            self._image.close()

# and get the rms of the residual image, inside clean region and outside

            self._image.open(infile=residualMapName)
            resid_stats = self._image.statistics(mask='%s < 0.1' % cleanmaskName,
             robust=False)
            resid_clean_stats = self._image.statistics(mask='%s > 0.1' % cleanmaskName,
             robust=False)

            if resid_clean_stats.has_key('rms'):
                if len(resid_clean_stats['rms']) > 0:
                    newCleanRms = resid_clean_stats['rms'][0]
            else:
                print 'no rms'
                break

            if resid_stats.has_key('rms'):
                if len(resid_stats['rms']) > 0:
                    newRms = resid_stats['rms'][0]
                else:
                    print 'failed to measure rms', residualMapName
                    print 'statistics', resid_stats
                    break
            else:
                print 'no rms'
                break

# get 2d rms of centre quarter of integrated image

            mapshape = self._image.shape()
            blc = []
            trc = []
            for i in range(len(mapshape)):
                if mapshape[i] > 10 :
                    dd = float(mapshape[i]) / 4.0
                    blc.append(int(dd))
                    trc.append(int(mapshape[i]-dd))
                else :
                    blc.append(int(0))
                    trc.append(int(mapshape[i]))
            pixels = self._image.getchunk(axes=[3], blc=blc, trc=trc)
            rms2d = std(pixels)
            self._image.close()

# get max of cleaned result

            self._image.open(infile=cleanMapName)
            clean_stats = self._image.statistics()
            imMax = clean_stats['max'][0]
            self._image.close()

# update commands list

            commands.append('# cleaning to threshold %s' % threshold) 
            commands.append("""imager.clean(algorithm=%s,
             mask='%s', niter=1000,
             gain=0.2, threshold="%sJy",
             displayprogress=False,
             model=%s, keepfixed=[False],
             image=%s, 
             residual=%s)""" % (self._algorithm, cleanmaskName, threshold,
             modelName, cleanMapName, residualMapName))

            commands.append('image.open(infile=%s)' % modelName)
            commands.append("""rtn=image.statistics(mask='%s > 0.1', 
             robust=False""" % cleanmaskName)
            commands.append('image.close()')
            commands.append('..get model sum: %s' % newSum)

            commands.append('image.open(infile=%s)' % residualMapName)
            commands.append("""rtn=image.statistics(mask='%s < 0.1', 
             robust=False""" % cleanmaskName)
            commands.append('..get rms outside cleaned area: %s' % newRms)
            commands.append("""rtn=image.statistics(mask='%s > 0.1', 
             robust=False""" % cleanmaskName)
            commands.append('..get rms inside cleaned area: %s' % newCleanRms)
            commands.append("""rtn=image.getchunk(axes=[3], blc=%s, trc=%s)"""
             % (blc, trc))
            commands.append('..get 2d rms from chunk: %s' % rms2d) 
            commands.append('image.close()')

            commands.append('image.open(infile=%s)' % cleanMapName)
            commands.append("""rtn=image.statistics()""")
            commands.append('..get clean image max: %s' % imMax)
            commands.append('image.close()')

            if self._verbose:
                print 'new mad,imMax', newMad, imMax 
            print 'newRms, newCleanRms', newRms, newCleanRms
            print 'sum, newSum', sum, newSum

            if sum > 0.0:
                sumChange = (newSum - sum)/sum
            else:
                sumChange = inf

# continue cleaning? End state is OK if cleaned flux is levelling off

            cleanRmsRecord.append(newCleanRms)
            if (newSum < 1.03 * sum):
                endState = 'OK'
                break

# not so good if we've run out of iterations.

            elif nloop > 10:
                endState = 'nloop > 10'
                break

# or clean diverging?

            elif (cleanRmsRecord[-1] > 5 * cleanRmsRecord[0]):
                endState = 'diverging'
                break

# or cleaned rms falling too far below non-cleaned rms (possibly
# cleaning too far)

            elif newCleanRms < 0.8 * newRms:
                endState = 'clean rms < 0.8 non-clean rms'
                break

            else:

# we do need another bout of cleaning, set a new threshold.
# Ideally set this from the rms of the non-cleaned area as this is
# decoupled from the cleaning depth. Guard against the case where there
# is no non-cleaned area which would give a 0 threshold and
# lead to overcleaning

                if newRms > 0.0:
                    newThreshold = min(threshold, 2 * newRms)
                else:
                    newThreshold = min(threshold, 2 * newCleanRms)

# sanity check

                if newThreshold == inf:
                    newThreshold = 0.1

# if the threshold is stagnating then force it down a little more
# to try to speed up convergence

                if threshold > 0.0:
                    change = (threshold - newThreshold) / threshold
                    if change < 0.1:
                        newThreshold = 0.8 * newThreshold

                nloop += 1
                threshold = newThreshold
                sum = newSum
                cleanRms = newCleanRms

# remove temporary files

        if os.path.exists('empty_image'):
            self._rmall('empty_image')
        if os.path.exists(cleanmaskName):
            self._rmall(cleanmaskName)

        return threshold, newRms, newCleanRms, cleanRmsRecord, sum, \
         sumChange, rms2d, imMax, commands, endState


    def _findSourcesAndCleanBoxes(self, cleanIntegratedMapName, residual_rms,
     bmaj, bmin, nx):
        """Private method to locate sources in an integrated map (1 channel)
        and setup clean boxes around them.

        Keyword arguments:
        cleanIntegratedMapName    -- Name of file containing clean map.
        residual_rms              -- Rms of clean residuals.
        bmaj                      -- major axis of beam.
        bmin                      -- minor axis of beam.
        nx                        -- number of pixels on each side of map.
        """
        commands = []

# get max value of clean results

        self._image.open(infile=cleanIntegratedMapName)
        clean_stats = self._image.statistics()
        integrated_max = clean_stats['max'][0]

# set the cutoff roughly, depending on rms

        cutoff = 5.0 * residual_rms / integrated_max

# ..find the sources (non point sources allowed) in the cleaned image

        self._image.open(infile=cleanIntegratedMapName)
        result = self._image.findsources(nmax=self._nmax, cutoff=cutoff, 
         point=False)
    
        commands.append('image.open(infile=%s)' % cleanIntegratedMapName)
        commands.append('''image.findsources(nmax=%s, cutoff=%s, 
         point=False)''' % (self._nmax, cutoff))

# now compose boxes

        sources = []
        if result.has_key('nelements'):
            nSource = result['nelements']
        else:
            nSource = 0

        if nSource == 0:

# just use default box covering centre quarter of image if no sources found

            boxes = [[nx/4, nx/4, 3*nx/4, 3*nx/4]]
        else:
            boxes = []

# get pixel sizes and ref pixel positions

            head = self._image.summary()
            commands.append('image.summary()')
            increment = head['header']['incr']
            refpix = head['header']['refpix']
            refval = head['header']['refval']
            imageShape = self._image.shape()
            commands.append('image.shape()') 

# convert positions and sizes to pixels

            for i in range(nSource):
                component = 'component%s' % i
                temp = {}
                sourceSize = 0
                if result[component]['shape'].has_key('majoraxis'):
                    qSize = self._quanta.quantity(
                     result[component]['shape']['majoraxis']['value'],
                     result[component]['shape']['majoraxis']['unit'])
                    self._quanta.convert(qSize, 'arcsec')
                    sourceMaj = self._quanta.getvalue(qSize)
                    sourceSize = sourceMaj
                    if result[component]['shape'].has_key('minoraxis'):
                        qSize = self._quanta.quantity(
                         result[component]['shape']['minoraxis']['value'],
                         result[component]['shape']['minoraxis']['unit'])
                        self._quanta.convert(qSize, 'arcsec')
                        sourceMin = self._quanta.getvalue(qSize)
                        sourceSize = max(sourceMaj, sourceMin)
                    temp['sourceSize'] = sourceSize / (abs(increment[1]) *
                     206265)

                    p = self._image.topixel(result[component]['shape'])
                    temp['xPosition'] = p['numeric'][0]
                    temp['yPosition'] = p['numeric'][1]
                    temp['flux'] = result[component]['flux']['value'][0]
                    temp['fluxUnit'] = result[component]['flux']['unit']
                    qbmaj = self._quanta.quantity(bmaj)
                    self._quanta.convert(qbmaj, 'arcsec')
                    vbmaj = self._quanta.getvalue(qbmaj)
                    qbmin = self._quanta.quantity(bmin)
                    self._quanta.convert(qbmin, 'arcsec')
                    vbmin = self._quanta.getvalue(qbmin)
                    temp['brightness'] = temp['flux'] * (vbmaj * vbmin) /\
                     (sourceMaj * sourceMin)
#                    print 'brightness', temp['brightness'], \
#                     temp['flux'], vbmaj, vbmin, sourceMaj, sourceMin

# flag if source is outside map area or box too big

                    if (temp['xPosition'] < 0) or \
                       (temp['xPosition'] > imageShape[0]-1) or \
                       (temp['yPosition'] < 0) or \
                       (temp['yPosition'] > imageShape[1]-1) or \
                       (3*temp['sourceSize'] > 0.75*imageShape[0]) or \
                       (3*temp['sourceSize'] > 0.75*imageShape[1]):
                        temp['outsideMap'] = True
                    elif temp['brightness'] < 5.0 * residual_rms:
                        temp['outsideMap'] = True
                    else:                    
                        temp['outsideMap'] = False

# construct box around the source if it is on the map

                    if not temp['outsideMap']:
                        box = \
                         [temp['xPosition'] - 1 * temp['sourceSize'],
                         temp['yPosition'] - 1 * temp['sourceSize'],
                         temp['xPosition'] + 1 * temp['sourceSize'],
                         temp['yPosition'] + 1 * temp['sourceSize']]
                        boxes.append(box)
                        temp['box'] = box

                    sources.append(temp)

            self._image.close()
            commands.append('image.close()')
#            print 'sources', sources

        return cutoff, nSource, sources, boxes, commands


    def calculate(self, logName=None): 
        """Method to calculate the cleaned images.

        Keyword arguments:
        logName -- The name of the casapy logname. This is only needed if
                   the results are required for 'getdata' and are going to
                   be displayed in html.
        """

#        print 'CleanImageV2.calculate called'
        self._htmlLogger.timing_start('CleanImageV2.calculate')

# have these data been calculated already?

        inputs = self.inputs()
        flag_marks = inputs['flag_marks']

        entryID,parameters = self._bookKeeper.available(
         objectType=inputs['objectType'], sourceType=inputs['sourceType'],
         furtherInput=inputs['furtherInput'],
         dependencies=inputs['dependencies'],
         outputFiles=[])

        if entryID == None:
            self._htmlLogger.timing_start('CleanImageV2.calculateNew')
 
            parameters = {'history':self._fullStageName,
                          'data':{},
                          'dependencies':{}}

# ensure ms flag state is 'Current' and save it for restoration afterward
# - calibration of the data can flag it implicitly and we want to avoid
# carrying forward 'hidden' flagging like that.

            self._msFlagger.setFlagState('Current')
            self._msFlagger.saveFlagState('CleanEntry')

# get flag history if will need to access the bandpass 'edge' flags later
 
            if self._bandpassFlaggingStage != None:
                flagging,flaggingReason,flaggingApplied = \
                 self._msFlagger.getFlags()

# get the bandpass calibration to apply for each SpW

            if self._bandpassCalDisplay == None or logName==None:

# results are not going to be displayed in html

                bpCalParameters = self._bpCal.calculate()
                parameters['dependencies']['bpCal'] = bpCalParameters
            else:

# results _are_ going to be diplayed in html.

                bandpassCalDisplay = self._bandpassCalDisplay(tasks={}, 
                 htmlLogger=self._htmlLogger, msName=self._msName)

                display = '''The bandpass calibration was calculated. The details
                 of the calculation and the results are shown '''
                display += self._htmlLogger.openNode('here.',
                 '%s.bandpass_calibration_details' % (self._stageName), True,
                 stringOutput=True)
                ignore,ignore,bpCalParameters = bandpassCalDisplay.display(
                 {'name':self._stageName}, self._bpCal, None, logName)
                self._htmlLogger.closeNode()
                parameters['dependencies']['bpCal'] = bpCalParameters
                parameters['dependencies']['bpCal']['separate node'] = display

# likewise, get the gain calibration

            if self._gainCalDisplay == None or logName==None:
                gainCalParameters = self._gainCal.calculate()
                parameters['dependencies']['gainCal'] = gainCalParameters
            else:
                gainCalDisplay = self._gainCalDisplay(tasks={}, 
                 htmlLogger=self._htmlLogger, msName=self._msName)

                display = '''The gain calibration was calculated. The details
                 of the calculation and the results are shown '''
                display += self._htmlLogger.openNode('here.',
                 '%s.gain_calibration_details' % (self._stageName), True,
                 stringOutput=True)
                ignore,ignore,gainCalParameters = gainCalDisplay.display(
                 {'name':self._stageName}, self._gainCal, None, logName)
                self._htmlLogger.closeNode()
                parameters['dependencies']['gainCal'] = gainCalParameters
                parameters['dependencies']['gainCal']['separate node'] = display

# iterate through fields and data_descs imaging each in turn

            self._ms.open(self._msName)

# first time around, calculate all field/spw. Subsequently, use the previous
# set of results as the basis for the current ones; only field/spw where the
# flagging has changed will be recalculated.

            if self._first:
                results = {}
            else:
                results = self._parameters['data']

            for fieldSpw in self._valid_field_spw:
#            for fieldSpw in [(1,18)]:
                field_id = fieldSpw[0]
                data_desc_id = fieldSpw[1]

# check field is desired

                if self._target_field_ids.count(field_id) == 0:
                    continue

# if the result has been calculated in a previous call, has any flagging
# changed? This may not spot a change in calibration. If no change then
# use the previous results

                if not self._first:
                    antenna_range = self._results['summary']['antenna_range']
                   
                    corr_axis, chan_freq, times, chunks, antenna1, antenna2, \
                     ifr_real, ifr_imag, ifr_flag, ifr_flag_row = \
                     self._getBaselineData(field_id, data_desc_id,
                     antenna_range, 'corrected', ['PreviousIteration','Current'])

                    if all(ifr_flag_row[0] == ifr_flag_row[1]) and \
                     all(ifr_flag[0] == ifr_flag[1]):
                        print 'no change to FLAG or FLAG_ROW for', fieldSpw
                        continue

                results[(field_id,data_desc_id)] = {
                 'fieldName':self._fieldName[field_id],
                 'DATA_DESC_ID':data_desc_id,
                 'commands':[],
                 'error':{'psf':None, 'dirty_image':None, 'pilot_clean':None,
                 'final_clean':None}}

                self._imager.open(thems=self._msName)
                self._imager.selectvis(spw=int(data_desc_id),
                 field=int(field_id))
                self._imager.weight(type='natural')

                commands = []
                commands.append('imager.open(thems=%s)' % self._msName)
                commands.append(
                 'imager.selectvis(spw=int(%s), field=int(%s))' % 
                 (data_desc_id, field_id))
                commands.append("imager.weight(type='natural')")

# apply the calibration, this may fail
                    
                try:    
                    self._bpCal.setapply(spw=data_desc_id, field=field_id)
                    self._gainCal.setapply(spw=data_desc_id, field=field_id)
                    commands += self._msCalibrater.correct(spw=data_desc_id,
                     field=field_id)
                except KeyboardInterrupt:
                    raise
                except:
                    error_report = self._htmlLogger.openNode('exception',
                     '%s.cal_apply_exception' % (self._stageName), True,
                     stringOutput = True)

                    self._htmlLogger.logHTML('Exception details<pre>')
                    traceback.print_exc()
                    traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                    self._htmlLogger.logHTML('</pre>')
                    self._htmlLogger.closeNode()

                    error_report += 'during calibration apply'
                    results[(field_id,data_desc_id)]['error']['psf'] = \
                     error_report
                    results[(field_id,data_desc_id)]['error']['dirty_image'] = \
                     error_report
                    results[(field_id,data_desc_id)]['error']['pilot_clean'] = \
                     error_report
                    results[(field_id,data_desc_id)]['error']['final_clean'] = \
                     error_report

                    continue

# calculate the cell and map sizes, using the advise method as a guide.

                aipsfieldofview = '%4.1farcsec' % self._fieldofview[
                 data_desc_id]
                results[(field_id,data_desc_id)]['fov'] = aipsfieldofview

                rtn = self._imager.advise(takeadvice=False,
                 amplitudeloss=0.5, fieldofview=aipsfieldofview)
                advised_cell = rtn['cell']
                commands.append('''imager.advise(takeadvice=False,
                 amplitudeloss=0.5, fieldofview=%s)''' % aipsfieldofview)

# get rounded values of cell size and npixels.

                cellv = advised_cell.value
                cellv = cellv / 2.0
                if cellv > 1:
                    cellv = math.floor(cellv)
                elif cellv > 0.1:
                    cellv = math.floor(cellv*10.)/10.
                elif cellv > 0.01:
                    cellv = math.floor(cellv*100.)/100.
                else:
                    cellv = math.floor(cellv*1000.)/1000.
                cell = '%sarcsec' % cellv

                npix = self._fieldofview[data_desc_id] / cellv
                if self._maxPixels != None:
                    if npix > self._maxPixels:
                        print 'CleanImageV2: npix=%s, resetting to %s' % (
                         npix, self._maxPixels)
                        npix = self._maxPixels

# ensure number of pixels is a composite number, for speed of clean etc.

                nx = self._nextLargerCompositeNumber(npix)

                print "..take cell= %s and npix= %s" % (cell, nx)
                results[(field_id,data_desc_id)]['nx'] = nx
                results[(field_id,data_desc_id)]['cell'] = cell

# generate channel flags from channels with no valid data in the MS

                empty_channels = None
                noisy_channels = None
                nchan = 1

                if self._mode=='channel':
                    nchan = self._num_chan[self._spectral_window_id[
                     data_desc_id]]

                    self._table.open(self._msName)
                    subTable = self._table.query('''FIELD_ID==%s &&
                     DATA_DESC_ID==%s && NOT FLAG_ROW && NOT ALL(FLAG)''' %
                     (field_id, data_desc_id))
                    empty_channels = range(nchan)
                    if subTable.nrows() > 0:
                        flag = subTable.getcol('FLAG')
                        for channel in range(nchan):
                            if not alltrue(flag[:,channel,:]): 
                                empty_channels.remove(channel)

# ..and get bandpass 'flagged' channels; only gets the first flagging occurrence
# for each spw

                    if self._bandpassFlaggingStage != None:
                        for row,val in enumerate(flaggingReason):
                            if val['stageDescription']['name'] == \
                             self._bandpassFlaggingStage:
                                for flagList in flagging[row].values():
                                    for flag in flagList:
                                        if data_desc_id == flag['DATA_DESC_ID']:
                                            noisy_channels = list(
                                             flag['CHANNELS'])
                                            break
                                if noisy_channels != None:
                                    break
                            if noisy_channels != None:
                                break

                results[(field_id,data_desc_id)]['nchan'] = nchan
                results[(field_id,data_desc_id)]['emptyChannels'] = \
                 empty_channels
                results[(field_id,data_desc_id)]['noisyChannels'] = \
                 noisy_channels
                results[(field_id,data_desc_id)]['bandpassFlaggingStage'] = \
                 self._bandpassFlaggingStage

# get the psf

                psfParameters = self._psf.calculate(field=field_id,
                 spw=data_desc_id, cell=cell, nx=nx)
                results[(field_id, data_desc_id)]['psfParameters'] = \
                 psfParameters
                results[(field_id, data_desc_id)]['psfMapName'] = \
                 psfParameters['mapName']

# get the beam dimensions from the psf. Fit may have failed, if so
# go to the next loop.

                if psfParameters['error'] != None:
                    results[(field_id,data_desc_id)]['error']['dirty_image'] = \
                     'error fitting beam to psf'
                    results[(field_id,data_desc_id)]['error']['pilot_clean'] = \
                     'error fitting beam to psf'
                    results[(field_id,data_desc_id)]['error']['final_clean'] = \
                     'error fitting beam to psf'

                    continue

                bmaj = psfParameters['bmaj']
                bmin = psfParameters['bmin']
                bpa = psfParameters['bpa']

# get the dirty image

                dirtyImageParameters = self._dirtyImage.calculate(
                 field=field_id, spw=data_desc_id, cell=cell, nx=nx)
                dirtyMapName = dirtyImageParameters['mapName']
                results[(field_id, data_desc_id)]['dirtyMapName'] = \
                 dirtyMapName
                results[(field_id, data_desc_id)]\
                 ['dirtyImageParameters'] = dirtyImageParameters

# construct names of integrated maps

                cleanIntegratedMapName = (
                 'cleanIntegrated.%s.f%s.spw%s.fm%s' % (
                 self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                 flag_marks)).replace(' ','')
                cleanIntegratedModelName = (
                 'integratedModel.%s.f%s.spw%s.fm%s' % (
                 self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                 flag_marks)).replace(' ', '')
                cleanIntegratedResidualMapName = (
                 'integratedResidual.%s.f%s.spw%s.fm%s' % (
                 self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                 flag_marks)).replace(' ', '')

                results[(field_id, data_desc_id)]\
                 ['cleanIntegratedMapName'] = cleanIntegratedMapName
                results[(field_id, data_desc_id)]\
                 ['cleanIntegratedModelName'] = cleanIntegratedModelName
                results[(field_id, data_desc_id)]\
                 ['cleanIntegratedResidualMapName'] = \
                 cleanIntegratedResidualMapName
 
# first make a clean integrated image and use it to find the sources
# ..set the image parameters

                start_boxes = [[nx/8, nx/8, 7*nx/8, 7*nx/8]]

# do the cleaning, this may fail

                try:    
                    integrated_threshold, integrated_rms,\
                     integrated_cleaned_rms, integrated_cleaned_rms_record,\
                     integrated_sum, integrated_sum_change, integrated_rms2d,\
                     integrated_max, integrated_commands,\
                     integrated_endState = \
                     self._clean(data_desc_id, field_id, nx, cell,
                     'mfs', -1, bmaj, bmin, bpa, start_boxes,
                     cleanIntegratedModelName,
                     cleanIntegratedMapName,
                     cleanIntegratedResidualMapName)
                except KeyboardInterrupt:
                    raise
                except:
                    error_report = self._htmlLogger.openNode('exception',
                     '%s.pilot_clean_exception' % (self._stageName), True,
                     stringOutput = True)

                    self._htmlLogger.logHTML('Exception details<pre>')
                    traceback.print_exc()
                    traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                    self._htmlLogger.logHTML('</pre>')
                    self._htmlLogger.closeNode()

                    error_report += 'during clean'

                    results[(field_id,data_desc_id)]['error']['pilot_clean'] = \
                     error_report
                    results[(field_id,data_desc_id)]['error']['final_clean'] = \
                     error_report

                    continue

                results[(field_id,data_desc_id)]['integrated_boxes'] = \
                 start_boxes
                results[(field_id,data_desc_id)]['integrated_threshold'] = \
                 integrated_threshold
                results[(field_id,data_desc_id)]['integrated_rms'] = \
                 integrated_rms
                results[(field_id,data_desc_id)]['integrated_cleaned_rms']\
                 = integrated_cleaned_rms
                results[(field_id,data_desc_id)]['integrated_cleaned_rms_record']\
                 = integrated_cleaned_rms_record
                results[(field_id,data_desc_id)]['integrated_sum']\
                 = integrated_sum
                results[(field_id,data_desc_id)]['integrated_sum_change']\
                 = integrated_sum_change
                results[(field_id,data_desc_id)]['integrated_rms2d'] = \
                 integrated_rms2d
                results[(field_id,data_desc_id)]['integrated_max'] = \
                 integrated_max
                results[(field_id,data_desc_id)]['integrated_endState'] = \
                 integrated_endState

                cutoff, nSource, sources, boxes, source_commands = \
                 self._findSourcesAndCleanBoxes(cleanIntegratedMapName,
                 integrated_rms, bmaj, bmin, nx)

                for item in source_commands:
                    integrated_commands.append(item)
                results[(field_id,data_desc_id)]['integrated_commands'] = \
                 integrated_commands
                results[(field_id,data_desc_id)]['cutoff'] = cutoff
                results[(field_id,data_desc_id)]['nSource'] = nSource
                results[(field_id,data_desc_id)]['boxes'] = boxes
                results[(field_id,data_desc_id)]['sources'] = sources

# construct names for clean maps, models, residuals

                cleanMapName = ('clean.%s.f%s.spw%s.fm%s' % (
                 self._base_msName,
                 self._cleanFieldName[field_id], data_desc_id, flag_marks)
                 ).replace(' ', '')
                cleanModelName = ('cleanModel.%s.f%s.spw%s.fm%s' % (
                 self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                 flag_marks)).replace(' ', '')
                cleanResidualMapName = ('residual.%s.f%s.spw%s.fm%s' % (
                 self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                 flag_marks)).replace(' ', '')

                results[(field_id,data_desc_id)]['cleanMapName'] = cleanMapName
                results[(field_id,data_desc_id)]['cleanModelName'] = \
                 cleanModelName
                results[(field_id,data_desc_id)]['cleanResidualMapName'] = \
                 cleanResidualMapName

# set the number of channels for the final maps

                if self._mode=='channel':
                    nchan = self._num_chan[self._spectral_window_id[
                     data_desc_id]]
                else:
                    nchan = 1

# do the cleaning, this may fail

                try:    
                    print 'second clean'
                    threshold, rms, cleaned_rms, cleaned_rms_record, sum,\
                     sum_change, rms2d, cleaned_max, finalCleanCommands,\
                     finalCleanEndState = self._clean(
                     data_desc_id, field_id, nx, cell, self._mode,
                     nchan, bmaj, bmin, bpa, boxes,
                     cleanModelName, cleanMapName, cleanResidualMapName,
                     integrated_rms=integrated_rms)
                except KeyboardInterrupt:
                    raise
                except:
                    error_report = self._htmlLogger.openNode('exception',
                     '%s.standard_clean_exception' % (self._stageName), True,
                     stringOutput = True)

                    self._htmlLogger.logHTML('Exception details<pre>')
                    traceback.print_exc()
                    traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                    self._htmlLogger.logHTML('</pre>')
                    self._htmlLogger.closeNode()

                    error_report += 'during clean'

                    results[(field_id,data_desc_id)]['error']['final_clean'] = \
                     error_report

                    continue

                results[(field_id,data_desc_id)]['threshold'] = threshold
                results[(field_id,data_desc_id)]['rms'] = rms
                results[(field_id,data_desc_id)]['cleaned_rms'] = cleaned_rms
                results[(field_id,data_desc_id)]['cleaned_rms_record'] = \
                 cleaned_rms_record
                results[(field_id,data_desc_id)]['sum'] = sum
                results[(field_id,data_desc_id)]['sum_change'] = sum_change
                results[(field_id,data_desc_id)]['rms2d'] = rms2d
                results[(field_id,data_desc_id)]['max'] = cleaned_max
                results[(field_id,data_desc_id)]['finalCleanCommands'] = \
                 finalCleanCommands
                results[(field_id,data_desc_id)]['finalCleanEndState'] = \
                 finalCleanEndState

# store the object info in the BookKeeper.
# copying by reference the history dictionaries may lead to problems.

            parameters['data'] = results
            self._ms.close()

            self._bookKeeper.enter(objectType=inputs['objectType'],
             sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             outputFiles=[],
             outputParameters=parameters,
             dependencies=inputs['dependencies'])
            self._htmlLogger.timing_stop('CleanImageV2.calculateNew')

# save the 'calibrated' flag state in case something else (e.g. ClosureError)
# needs it

            self._msFlagger.deleteFlagState('CleanImageCalibration')
            self._msFlagger.saveFlagState('CleanImageCalibration')

# restore the flag state on entry and adopt it as 'Current'

            self._msFlagger.setFlagState('CleanEntry')
            self._msFlagger.adoptAsCurrentFlagState()
            self._msFlagger.saveFlagState('PreviousIteration')
            self._first = False

        self._parameters = parameters
        self._htmlLogger.timing_stop('CleanImageV2.calculate')
        return parameters


    def description(self):
        description = ['CleanImageV2 - cleaned images']
        return description


    def getData(self, logName=None):
        """Public method to return the cleaned images as a 'view' of the data.
        
        Keyword arguments:
        logName -- The name of the casapy logfile.
        """

#        print 'CleanImageV2.getData called'

        self._htmlLogger.timing_start('CleanImageV2.getdata')
        parameters = self.calculate(logName=logName)
        self._results['parameters'] = parameters

# iterate through fields and data_descs extracting the image for each in turn

        for k,v in parameters['data'].iteritems():
            field_id = k[0]
            data_desc_id = k[1] 

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (a) Stokes:I - Clean image' % (
             self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'clean map',
             v, mapField='cleanMapName', cleanBoxField='boxes',
             thresholdField='threshold', rms2dField='rms2d',
             noisyChannelField='noisyChannels',
             bandpassFlaggingStageField = 'bandpassFlaggingStage',
             emptyChannelField='emptyChannels', error_key='final_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (b) Stokes:I - Clean residual' % (
             self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'clean residual',
             v, mapField='cleanResidualMapName', cleanBoxField='boxes',
             thresholdField='threshold', rms2dField='rms2d',
             noisyChannelField='noisyChannels',
             bandpassFlaggingStageField = 'bandpassFlaggingStage',
             emptyChannelField='emptyChannels', error_key='final_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (c) Stokes:I - Pilot integrated clean image' \
             % (self._fieldName[field_id], self._fieldType[field_id],
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'clean integrated map',
             v, mapField='cleanIntegratedMapName', 
             cleanBoxField='integrated_boxes',
             thresholdField='integrated_threshold',
             rms2dField='integrated_rms2d', error_key='pilot_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (d) Stokes:I - Pilot integrated residual image' \
             % (self._fieldName[field_id], self._fieldType[field_id],
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'residual integrated map',
             v, mapField='cleanIntegratedResidualMapName', 
             cleanBoxField='integrated_boxes', 
             thresholdField='integrated_threshold',
             rms2dField='integrated_rms2d', error_key='pilot_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (e) Point Spread Function' \
             % (self._fieldName[field_id], self._fieldType[field_id],
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'point spread function',
             v, mapField='psfMapName', error_key='psf')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (f) Stokes:I - DirtyImage' \
             % (self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'clean integrated map',
             v, mapField='dirtyMapName',
             noisyChannelField='noisyChannels',
             bandpassFlaggingStageField = 'bandpassFlaggingStage',
             emptyChannelField='emptyChannels', error_key='dirty_image')

# return a copy of the data list, otherwise operating on it outside this class

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('CleanImageV2.getdata')
        return temp


    def inputs(self):
        """
        """
        result = {}
        result['objectType'] = 'CleanImageV2'
        result['sourceType'] = self._sourceType
        result['furtherInput'] = {'mode':self._mode,
                                 'algorithm':self._algorithm,
                                 'maxPixels':self._maxPixels,
                                 'bandpassFlaggingStage':
                                 self._bandpassFlaggingStage}

        flag_marks = {}
        ignore,flag_mark_col = self._msFlagger.getFlagMarkInfo()
        field_ids = self.getFieldsOfType(self._sourceType)
        for field_id in field_ids:
            flag_marks[field_id] = flag_mark_col[field_id]
        flag_marks = str(flag_marks)

# replace unusual symbols to avoid problems with TaQL

        flag_marks = self._removeProblemTaQLCharacters(flag_marks)

        result['flag_marks'] = flag_marks
        result['outputFiles'] = []
        result['dependencies'] = [self._bpCal.inputs(),
                                  self._gainCal.inputs()]
        return result


    def createGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.
                
        Keyword arguments:
        stageName -- Name of the recipe stage using this object.  
        """

        cleanImageDescription = {}

        cleanImageDescription['bandpass calibration'] = \
         self._bpCal.createGeneralHTMLDescription(stageName)\
         ['bandpass calibration']

        cleanImageDescription['gain calibration'] = \
         self._gainCal.createGeneralHTMLDescription(stageName)\
         ['gain calibration']

        cleanDescription = """
         <p>The clean image was constructed as follows:
         <ul>
          <li>The image weight was set to 'natural'.
          <li>Iterating through the fields and spws to be imaged:
              <ul>
               <li>The calibration results for bandpass and gain
                   were applied to the data.
               <li>The cell size and number of pixels along each side of the
                   square image to be produced were calculated."""

        cleanDescription += "<li>" + \
         self._psf.createGeneralHTMLDescription(stageName)
        cleanDescription += "<li>" + \
         self._dirtyImage.createGeneralHTMLDescription(stageName)

        cleanDescription += """
               <li>A 'pilot' integrated image was made and searched for 
                   sources so that clean boxes could be allocated for
                   cleaning the cube.
               <li>If no sources were found in the 'pilot' then the 
                   final image 
                   was cleaned using a box covering the centre quarter of the 
                   image. Otherwise, square boxes were centred on
                   each detected source.
              </ul>
         </ul>"""

        cleanImageDescription['clean image'] = cleanDescription
        return cleanImageDescription


    def createDetailedHTMLDescription(self, stageName, parameters=None):
        """Write a description of the class to html.
                
        Keyword arguments:
        stageName -- Name of the recipe stage using this object.  
        parameters -- The dictionary that holds the descriptive information.
        """

        if parameters == None:
            parameters = self._parameters

        cleanImageDescription = {}

        cleanImageDescription['bandpass calibration'] = \
         self._bpCal.createDetailedHTMLDescription(stageName, parameters=
         parameters['dependencies']['bpCal'])['bandpass calibration']

        cleanImageDescription['gain calibration'] = \
         self._gainCal.createDetailedHTMLDescription(stageName, parameters=
         parameters['dependencies']['gainCal'])['gain calibration']

        cleanDescription = """
         <p>The clean image was constructed as follows:
         <ul>
          <li>The 'image.weight' method set the weight to 'natural'.
          <li>Iterating through the fields and spws to be imaged:
           <ul>
            <li>The relevant data were selected.
            <li>The calibration results for bandpass and gain described
             above were applied to the data.
            <li>The image cell size was set to 0.5 the 'maximum' cell size
             returned by the 'imager.advise' method, rounded down to 
             make the
             value stop at the most significant digit e.g. 0.015 would
             become 0.01. The 'maximum' cell size itself is:
            <pre> 
         max cell size = 1.0 / (2.0 * maximum uv distance in wavelengths)
            </pre>
             The number of pixels along each side of the
             square image to be produced was calculated so that 
             with the adopted cell size it would cover the field of 
             view of the primary beam, derived from:
            <pre>
         fov = 2.0 * (1.22 * 3e8/frequency) / dish_diameter
            </pre>
             This number was reset to the next greater
             'composite' number to ensure the efficient operation
             of the casapy methods used for the imaging. Here a
             'composite' number is defined as being even and a multiple 
             of powers of 2, 3 and 5. """

        if self._maxPixels != None:
            cleanDescription += """
             Lastly, if the number of pixels along 
             each side exceeded %s then it was reset to that ceiling
             value.""" % (self._maxPixels)

        cleanDescription += """
            <li>"""

        cleanDescription += self._psf.createDetailedHTMLDescription(stageName)

        cleanDescription += """
             <p>p.s.f. parameters:
             <table border='1'>
              <tr>
               <td>Field</td>
               <td>Spectral Window</td>
               <td>F.O.V.</td>
               <td>Nx</td>
               <td>Cell</td>
               <td>Map Name</td>
               <td>Bmaj</td>
               <td>Bmin</td>
               <td>Bpa</td>
               <td>Casapy Calls</td>
               <td>Error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for k in keys:
            v = parameters['data'][k]
            if v['error']['psf'] == None:
                w = v['psfParameters']
                cleanDescription += """
              <tr>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>
               <td>%s</td>
               <td>%s</td>
               <td>%s</td>""" % (k[0], k[1], v['fov'], w['nx'], w['cell'],
               w['mapName'])
                if w.has_key('bmaj'):
                    cleanDescription += """
               <td>%s</td>
               <td>%s</td>
               <td>%s</td>""" % (w['bmaj'], w['bmin'], w['bpa'])
                else:
                    cleanDescription += """
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>"""

# casapy calls in a separate node      

                cleanDescription += '''
               <td>'''
                cleanDescription += self._htmlLogger.openNode('casapy calls',
                 '%s.field%sspw%s.%s' % (stageName, k[0], k[1], 
                 'psf_casapy_calls'), True, stringOutput=True)
                for line in w['commands']:
                    self._htmlLogger.logHTML('<br>%s' % line)
                self._htmlLogger.closeNode()
                cleanDescription += '''
               </td>
               <td>&nbsp;</td>'''

            else:
                cleanDescription += """
              <tr>
               <td>%i</td>
               <td>%i</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>%s</td>
              </tr>""" % (k[0], k[1], v['error']['psf'])
        cleanDescription += """
             </table>"""

        cleanDescription += """
            <li>""" + self._dirtyImage.createDetailedHTMLDescription(stageName)

        cleanDescription += """
             <p>dirty image parameters:
             <table border='1'>
              <tr>
               <td>Field</td>
               <td>Spectral Window</td>
               <td>Nx</td>
               <td>Cell</td>
               <td>Nchan</td>
               <td>Map Name</td>
               <td>Casapy Calls</td>
               <td>Error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for k in keys:
            v = parameters['data'][k]
            if v['error']['dirty_image'] == None:
                w = v['dirtyImageParameters']
                cleanDescription += """
              <tr>
               <td>%i</td>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>
               <td>%i</td>
               <td>%s</td>""" % (k[0], k[1], w['nx'], w['cell'], w['nchan'],
               w['mapName'])

# casapy calls in a separate node      

                cleanDescription += '''
               <td>'''
                cleanDescription += self._htmlLogger.openNode('casapy calls',
                 '%s.field%sspw%s.%s' % (stageName, k[0], k[1], 
                 'dirtyimage_casapy_calls'), True, stringOutput=True)
                for line in w['commands']:
                    self._htmlLogger.logHTML('<br>%s' % line)
                self._htmlLogger.closeNode()
                cleanDescription += '''
               </td>
               <td>&nbsp;</td>'''

            else:
                cleanDescription += """
              <tr>
               <td>%i</td>
               <td>%i</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>%s</td>
              </tr>""" % (k[0], k[1], v['error']['dirty_image'])
        cleanDescription += """
             </table>"""

        cleanDescription += """
            <li>A 'pilot' integrated image was made and searched for 
             sources so that clean boxes could be allocated for
             cleaning the cube. The clean box for the pilot image
             covered the centre 3/4 of the whole. 
             <ul>
              <li>A loop was entered with the image being cleaned down 
               to a lower threshold each time:
               <ul>
                <li>The first threshold was 0.1Jy.
                <li>'imager.clean' was called to clean the
                 image down to the threshold.
                <li>'image.statistics' was used on the residual
                 image to derive the rms of the pixels
                 outside the cleaned area and of those inside the
                 cleaned area. The same method was used on the
                 model image to derive the total cleaned flux.
                 The clean loop could be exited for one of three reasons. 
                 <ul>
                  <li>If the cleaned residual rms was below the non cleaned
                   residual rms, and the cleaned flux had climbed less than 3
                   per cent in the last clean. This would indicate that most 
                   source flux had been cleaned and that further cleaning was 
                   likely to harvest mostly noise. This is the preferred exit
                   mode that marks the end of a successful clean.
                  <li>If the cleaned residual rms fell below
                   0.8 times the non cleaned residual rms, with  the cleaned
                   flux still climbing. If a source is visible in the cleaned
                   map then these measures are not consistent and
                   imply that something has gone wrong; however, if the field
                   is empty then the cleaned flux is meaningless and the result
                   may be OK.
                  <li>If the cleaned residual rms climbed during 
                   a loop iteration then this might indicate that the clean
                   algorithm itself had begun to diverge.
                 </ul>
                 If no exit criterion was fulfilled then the clean would
                 continue toward a lower threshold. The new threshold
                 would be min(current threshold, 2 * rms outside
                 cleaned area). If that rms was not available then 
                 the rms from inside the cleaned area would be used.
               </ul> 
              <li>'image.findSources' was called to find any sources
               present whose total flux was greater than the 'cutoff'
               fraction of the strongest source found. The value of 
               'cutoff' is given in the table below and was set to 
               10 times the rms of the pixels divided by the maximum
               pixel value. The sources found were examined and those
               whose mean surface brightness per beam fell below 5 times 
               the pixel rms were excluded.
               <pre>
      brightness = total flux * (beam major axis * beam minor axis)
                   ------------------------------------------------
                      source major axis * source minor axis
               </pre> 
              <li>If no sources are found then then a default clean
               box covering the centre quarter of the image is used.
              <li>If sources are found then square boxes centred on
               each detected source with sides 2 * the maximum source 
               dimension will be used in the construction of the final images.
             </ul>"""

        cleanDescription += """
             <p>Pilot image parameters.
             <table border='1'>
              <tr>
               <td>Field</td>
               <td>Spectral Window</td>
               <td>Nx</td>
               <td>Cell</td>
               <td>Threshold</td>
               <td>Quality</td>
               <td>Non-cleaned rms</td>
               <td>Cleaned rms</td>
               <td>Cleaned rms Record</td>
               <td>Cleaned Flux</td>
               <td>Flux Change</td>
               <td>2d rms</td>
               <td>Max</td>
               <td>Cutoff</td>
               <td>Sources</td>
               <td>Casapy Calls</td>
               <td>Error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for k in keys:
            v = parameters['data'][k]
            if v['error']['pilot_clean'] == None:
                cleanDescription += """
              <tr>
               <td>%i</td>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>""" % (
                 k[0], k[1], v['nx'], v['cell'], v['integrated_threshold'],
                 v['integrated_endState'], v['integrated_rms'],
                 v['integrated_cleaned_rms'],
                 self._formatList(v['integrated_cleaned_rms_record'], '%.2g'),
                 v['integrated_sum'], v['integrated_sum_change'],
                 v['integrated_rms2d'], v['integrated_max'], v['cutoff'])

                if v['nSource'] == 0:
                    cleanDescription += '''
               <td>&nbsp;</td>'''

                else:

# sources in a separate node to save space on the main page

                    cleanDescription += '''
               <td>'''
                    cleanDescription += self._htmlLogger.openNode('sources',
                     '%s.spw%s.%s' % (stageName, k,
                     'pilot_image_sources'), True, stringOutput=True)
                    self._htmlLogger.logHTML('''
                <table border='1'>
                 <tr>
                  <td>Source #</td>
                  <td>x offset (pix)</td>
                  <td>y offset (pix)</td>\
                  <td>Size (pix)</td>
                  <td>Flux</td>
                 </tr>''')

                    for i,source in enumerate(v['sources']):
                        self._htmlLogger.logHTML("""
                 <tr>
                  <td>%i</td>
                  <td>%4.1f</td>
                  <td>%4.1f</td>
                  <td>%4.1f</td>
                  <td>%5.3f %s</td>
                 </tr>""" % (i, source['xPosition'], 
                         source['yPosition'], source['sourceSize'],
                         source['flux'], source['fluxUnit']))

                    self._htmlLogger.logHTML('''
                </table>''')

                    self._htmlLogger.closeNode()
                    cleanDescription += ('''
               </td>''')

# casapy calls in a separate node      

                cleanDescription += '''
               <td>'''
                cleanDescription += self._htmlLogger.openNode('casapy calls',
                '%s.field%sspw%s.%s' % (stageName, k[0], k[1], 
                'pilot_image_casapy_calls'), True, stringOutput=True)
                for line in v['integrated_commands']:
                    self._htmlLogger.logHTML('<br>%s' % line)
                self._htmlLogger.closeNode()
                cleanDescription += '''
               </td>
               <td>&nbsp;</td>
              </tr>'''

            else:
                cleanDescription += '''
              <tr>
               <td>%i</td>
               <td>%i</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>%s</td>
              </tr>''' % (k[0], k[1], v['error']['pilot_clean'])

        cleanDescription += """
             </table>
            <li>Final images were constructed using the clean boxes
             derived from the 'pilot':
             <ul>
              <li>As for the pilot integrated image, a loop was 
               entered with the image cleaned down to a lower 
               threshold each time. For each loop 'imager.regionmask'
               set the clean boxes derived from the pilot image.
             </ul>

             <p>Final clean image parameters:
             <table border='1'>
              <tr>
               <td>Field</td> 
               <td>Spectral Window</td>
               <td>Nx</td>
               <td>Cell</td>
               <td>Threshold</td>
               <td>Quality</td>
               <td>Non-cleaned rms</td>
               <td>Cleaned rms</td>
               <td>Cleaned rms Record</td>
               <td>Cleaned Flux</td>
               <td>Flux Change</td>
               <td>2d rms</td>
               <td>Max</td>
               <td>Casapy Calls</td>
               <td>Error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for k in keys:
            v = parameters['data'][k]
            if v['error']['final_clean'] == None:
                cleanDescription += """
              <tr>
               <td>%i</td>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>""" % (k[0], k[1], v['nx'], v['cell'],
                 v['threshold'], v['finalCleanEndState'], v['rms'],
                 v['cleaned_rms'],
                 self._formatList(v['cleaned_rms_record'], '%.2g'),
                 v['sum'], v['sum_change'], v['rms2d'], v['max'])

# casapy calls in a separate node      

                cleanDescription += '''
               <td>'''
                cleanDescription += self._htmlLogger.openNode('casapy calls',
                 '%s.field%sspw%s.%s' % (stageName, k[0], k[1], 
                 'final_image_casapy_calls'), True, stringOutput=True)
                for line in v['finalCleanCommands']:
                    self._htmlLogger.logHTML('<br>%s' % line)
                self._htmlLogger.closeNode()
                cleanDescription += '''
               </td>
               <td>&nbsp;</td>'''

            else:
                cleanDescription += '''
              <tr>
               <td>%i</td>
               <td>%i</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>&nbsp;</td>
               <td>%s</td>
              </tr>''' % (k[0], k[1], v['error']['final_clean'])

        cleanDescription += """
             </table>
             <ul>
              <li>'Threshold' gives the target threshold of the last clean.
              <li>'Non-cleaned rms' was calculated using all data in the
               residual image/cube outside the cleaned area.
              <li>'Cleaned rms' was calculated using data inside the cleaned
               area.
              <li>'Cleaned rms Record' records the change in the 
               rms of the cleaned area through successive cleaning loops.
              <li>'Flux Change' gives the factional increase in cleaned flux in 
               the last clean.
              <li>'2d rms' was calculated from pixels in the centre quarter
               of the continuum residual image.
             </ul>
            <li>Flat representations of the clean cubes were calculated
             by averaging them over the 'channel' axis."""

        if self._bandpassFlaggingStage != None:
            cleanDescription += """
             Channels that were flagged at stage '%s' were not included in 
             the average.""" % self._bandpassFlaggingStage

        cleanDescription += """
           </ul>
         </ul>"""
#         <p>The clean image was constructed by Python class CleanImageV2."""

        cleanImageDescription['clean image'] = cleanDescription
        return cleanImageDescription


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.
                
        Keyword arguments:
        stageName -- Name of the recipe stage using this object.  
        """

        if self._mode == 'channel':
            self._htmlLogger.logHTML("""<p>The data view
             has been produced by cleaning a cube channel by channel
             then integrating the result to produce a 2-d map.""")
        elif self._mode == 'mfs':
            self._htmlLogger.logHTML("""<p>The data view is a map that
             has been produced by combining all spectral data into
             one continuum channel before the clean.""")


    def writeDetailedHTMLDescription(self, stageName, topLevel, 
     parameters=None):
        """Write a description of the class to html.
                
        Keyword arguments:
        stageName -- Name of the recipe stage using this object.  
        topLevel  -- True if this data 'view' is to be displayed directly,   
                     not passing through a data modifier object.
        parameters -- The dictionary that holds the
                      descriptive information.
        """

        description = self.createDetailedHTMLDescription(stageName, 
         parameters=parameters)

        if topLevel:
            self._htmlLogger.logHTML("<h3>Data View</h3>")

        self._htmlLogger.logHTML("""
             <p>The clean image was constructed as follows:
             <ul>""")

        self._htmlLogger.logHTML("""
              <li>""" + description['bandpass calibration'] + """
              <li>""" + description['gain calibration'] + """
              <li>""" + description['clean image'] + """
             </ul>""")
