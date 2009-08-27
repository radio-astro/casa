"""Module to supply cleaned images."""

# History:
# 26-Sep-2008 jfl First version.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.
# 31-Jul-2009 jfl no maxPixels release.

# package modules

import math
from numpy import *

# alma modules

from cleanImageV2 import *

class ContinuumSubtractedCleanImage(CleanImageV2):
    """Class to supply cleaned images.
    """

    def _imagePlaneclean(self, dirtyMapName, psfMapName, mode, nchan,
     bmaj, bmin, bpa, boxes, quarter_box, modelMapName, cleanMapName,
     residualMapName, target_rms):
        """Private method to clean an image.

        Keyword arguments:
        dirtyMapName  -- File containing dirty map.
        psfMapName    -- File containing map of psf.
        mode          -- 'mfs' or 'channel'.
        nchan         -- number of channels in cube.
        bmaj          -- major axis of beam.
        bmin          -- minor axis of beam.
        bpa           -- position angle of beam major axis.
        boxes         -- list of clean boxes (corners, in pixels).     
        quarter_box   -- box covering a quarter of the map area.
        modelMapName  -- file to hold model.
        cleanMapName  -- file to hold clean map.
        residualMapName -- file to hold residuals.
        target_rms    -- target rms of residual.
        """
        commands = []

# make clean region from boxes

        self._image.open(dirtyMapName)
        csys = self._image.coordsys()
        self._image.close()

        clean_regions = {}
        for i,box in enumerate(boxes):
            boxname = 'r%s' % i
            boxregion = self._regionmanager.wbox(
             blc='%spix %spix' % (box[0], box[1]),
             trc='%spix %spix' % (box[2], box[3]),
             pixelaxes=[0,1], csys=csys.torecord())
            clean_regions[boxname] = boxregion

        if len(clean_regions) > 1:
            clean_region_union = self._regionmanager.makeunion(clean_regions)
        else:
            clean_region_union = clean_regions['r0']

        quarter_region = self._regionmanager.wbox(
         blc='%spix %spix' % (quarter_box[0], quarter_box[1]),
         trc='%spix %spix' % (quarter_box[2], quarter_box[3]),
         pixelaxes=[0,1], csys=csys.torecord())

        clean_region = self._regionmanager.intersection(
         {'r1':clean_region_union, 'r2':quarter_region})

# construct clean mask name, remove chars that confuse 'mask' parameter later

        cleanmaskName = 'cleanmask.%s' % cleanMapName
        cleanmaskName = cleanmaskName.replace('-','_')
        cleanmaskName = cleanmaskName.replace('+','_')
        cleanmaskName = cleanmaskName.replace('(','_')
        cleanmaskName = cleanmaskName.replace(')','_')

# remove any previous mask files and model

        if os.path.exists(cleanmaskName):
            self._rmall(cleanmaskName)
        if os.path.exists(modelMapName):
            self._rmall(modelMapName)

# open deconvolver, make cleanmask

        self._deconvolver.open(dirty=dirtyMapName, psf=psfMapName)
        self._deconvolver.regionmask(mask=cleanmaskName, region=clean_region)

        self._image.open(cleanmaskName)
        maskStats = self._image.statistics()
        self._image.close()
        maxMask = maskStats['max']
        minMask = maskStats['min']

        if maxMask[0] < 1.0e-7:
            raise RuntimeError, 'clean mask is empty'

# clean

        threshold = 2 * target_rms

        if self._verbose:
            print 'clean to threshold', threshold
        print 'clean to threshold', threshold

        threshold_reached = self._deconvolver.clean(algorithm=self._algorithm,
         mask=cleanmaskName, niter=1000, gain=0.2,
         threshold="%sJy" % (threshold), displayprogress=False,
         model=modelMapName)

# smooth the model with the beam, add the residuals

        self._deconvolver.restore(image=cleanMapName, model=modelMapName,
         bmaj=bmaj, bmin=bmin, bpa=bpa)

# calculate the residuals

        self._deconvolver.residual(model=modelMapName,
         image=residualMapName)
        self._deconvolver.close()

# get the sum of the model image

        self._image.open(infile=modelMapName)
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

        if resid_stats.has_key('rms'):
            if len(resid_stats['rms']) > 0:
                newRms = resid_stats['rms'][0]
            else:
                print 'failed to measure rms', residualMapName
                print 'statistics', resid_stats
        else:
            print 'no rms'

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

        commands.append("# remove '%s'" % cleanmaskName)
        commands.append("# remove %s" % modelMapName)
        commands.append("deconvolver.open(dirty=%s, psf=%s)" % (
         dirtyMapName, psfMapName))
        commands.append("""deconvolver.regionmask(mask='%s',
         region=clean_region)""" % cleanmaskName)

        commands.append("# clean to threshold %s" % threshold)
        commands.append("""deconvolver.clean(algorithm=%s,
         mask='%s', niter=1000, gain=0.2, threshold="%sJy",
         displayprogress=False, model=%s)""" % (self._algorithm,
         cleanmaskName, threshold, modelMapName))
        commands.append("""deconvolver.restore(image=%s, model=%s,
         bmaj=%s, bmin=%s, bpa=%s)""" % (cleanMapName, modelMapName,
         bmaj, bmin, bpa))
        commands.append("""deconvolver.residual(model=%s, image=%s)""" % (
         modelMapName, residualMapName))
        commands.append("deconvolver.close()")

        commands.append("image.open(infile=%s)" % modelMapName)
        commands.append("""rtn=image.statistics(mask='%s > 0.1',
         robust=False""" % cleanmaskName)
        commands.append("image.close()")
        commands.append("..get model sum: %s" % newSum)

        commands.append("image.open(infile=%s)" % residualMapName)
        commands.append("""rtn=image.statistics(mask='%s < 0.1',
         robust=False""" % cleanmaskName)
        commands.append("..get rms outside cleaned area: %s" % newRms)
        commands.append("""rtn=image.statistics(mask='%s > 0.1',
         robust=False""" % cleanmaskName)
        commands.append("..get rms inside cleaned area: %s" % newCleanRms)
        commands.append("""rtn=image.getchunk(axes=[3], blc=%s, trc=%s)"""
         % (blc, trc))
        commands.append("..get 2d rms from chunk: %s" % rms2d)
        commands.append("image.close()")

        commands.append("image.open(infile=%s)" % cleanMapName)
        commands.append("""rtn=image.statistics()""")
        commands.append("..get clean image max: %s" % imMax)
        commands.append("image.close()'")
        if self._verbose:
            print 'new mad,imMax', newMad, imMax
        print 'newRms, newCleanRms', newRms, newCleanRms
        print 'newSum', newSum

        endState = 'OK'

# cleaned rms falling too far below non-cleaned rms (possibly
# cleaning too far)

        if newCleanRms < 0.5 * newRms:
            endState = 'clean rms < 0.5 non-clean rms'
        elif not threshold_reached:
            endState = 'threshold not reached'

        return threshold, newRms, newCleanRms, rms2d, imMax, commands, endState


    def calculate(self, logName=None): 
        """Method to calculate the cleaned images.

        Keyword arguments:
        logName -- The name of the casapy logname. This is only needed if
                   the results are required for 'getdata' and are going to
                   be displayed in html.
        """

#        print 'ContinuumSubtractedCleanImage.calculate called'
        self._htmlLogger.timing_start('ContinuumSubtractedCleanImage.calculate')

# have these data been calculated already?

        inputs = self.inputs()
        flag_marks = inputs['flag_marks']

        entryID,parameters = self._bookKeeper.available(
         objectType=inputs['objectType'], sourceType=inputs['sourceType'],
         furtherInput=inputs['furtherInput'],
         dependencies=inputs['dependencies'],
         outputFiles=[])

        if entryID == None:
            self._htmlLogger.timing_start(
             'ContinuumSubtractedCleanImage.calculateNew')
 
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

            results = {}
            for field_id in self._target_field_ids:
                for data_desc_id in self._data_desc_ids:
                    results[(field_id,data_desc_id)] = {
                     'fieldName':self._fieldName[field_id],
                     'DATA_DESC_ID':data_desc_id,
                     'commands':[],
                     'error':{'psf':None,
                     'pilot_clean':None,
                     'standard_clean':None,
                     'dirty_image':None, 
                     'continuum_subtraction':None,
                     'line_clean':None,
                     'continuum_clean':None}} 

# check that SpW has enough channels for processing to proceed

                    nchan = self._results['summary']['nchannels'][data_desc_id]
                    if nchan==1:
                        results[(field_id, data_desc_id)]['error']['psf'] = \
                         'continuum'
                        results[(field_id, data_desc_id)]['error']['pilot_clean'] = \
                         'continuum'
                        results[(field_id, data_desc_id)]['error']['standard_clean'] = \
                         'continuum'
                        results[(field_id, data_desc_id)]['error']['dirty_image'] = \
                         'continuum'
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_subtraction'] = 'continuum'
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_clean'] = 'continuum'
                        results[(field_id, data_desc_id)]['error']\
                         ['line_clean'] = 'continuum'
                        continue

                    elif nchan < 10 :
                        error = 'too few channels to fit continuum (%s)' % nchan
                        results[(field_id, data_desc_id)]['error']['psf'] = \
                         error
                        results[(field_id, data_desc_id)]['error']['pilot_clean'] = \
                         error
                        results[(field_id, data_desc_id)]['error']['standard_clean'] = \
                         error
                        results[(field_id, data_desc_id)]['error']['dirty_image'] = \
                         error
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_subtraction'] = error
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_clean'] = error
                        results[(field_id, data_desc_id)]['error']\
                         ['line_clean'] = error
                        continue

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

                        results[(field_id, data_desc_id)]['error']['psf'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']['pilot_clean'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']['standard_clean'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']['dirty_image'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_subtraction'] = error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_clean'] = error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['line_clean'] = error_report

                        continue

# calculate the cell and map sizes, using the advise method as a guide.

                    aipsfieldofview = '%4.1farcsec' % (4.0 * self._beamRadius[
                     data_desc_id])
                    results[(field_id,data_desc_id)]['fov'] = aipsfieldofview

                    rtn = self._imager.advise(takeadvice=False,
                     amplitudeloss=0.5, fieldofview=aipsfieldofview)
                    advised_cell = rtn['cell']
                    self._imager.close()

                    commands.append('''imager.advise(takeadvice=False,
                     amplitudeloss=0.5, fieldofview=%s)''' % aipsfieldofview)
                    commands.append('imager.close()')

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

                    npix = 4.0 * self._beamRadius[data_desc_id] / cellv
                    if self._maxPixels != None:
                        if npix > self._maxPixels:
                            print 'ContinuumSubtractedCleanImage: npix=%s, resetting to %s' % (
                             npix, self._maxPixels)
                            npix = self._maxPixels

# ensure number of pixels is a composite number, for speed of clean etc.

                    nx = self._nextLargerCompositeNumber(npix)

#                    print "..take cell= %s and npix= %s" % (cell, nx)
                    results[(field_id,data_desc_id)]['nx'] = nx
                    results[(field_id,data_desc_id)]['cell'] = cell

# get the psf

                    psfParameters = self._psf.calculate(field=field_id,
                     spw=data_desc_id, cell=cell, nx=nx)
                    results[(field_id, data_desc_id)]['psfParameters'] = \
                     psfParameters
                    psfMapName = psfParameters['mapName']
                    results[(field_id, data_desc_id)]['psfMapName'] = psfMapName

# get the beam dimensions from the psf. Fit may have failed, if so
# go to the next loop.

                    if psfParameters['error'] != None:

                        error_report = 'error fitting beam to psf'

                        results[(field_id, data_desc_id)]['error']['psf'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']['pilot_clean'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']['standard_clean'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']['dirty_image'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_subtraction'] = error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_clean'] = error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['line_clean'] = error_report

                        continue

                    bmaj = psfParameters['bmaj']
                    bmin = psfParameters['bmin']
                    bpa = psfParameters['bpa']
                    print 'beam pars', bmaj, bmin, bpa

# get the dirty image

                    dirtyImageParameters = self._dirtyImage.calculate(
                     field=field_id, spw=data_desc_id, cell=cell, nx=nx)
                    dirtyMapName = dirtyImageParameters['mapName']
                    results[(field_id, data_desc_id)]['dirtyMapName'] = \
                     dirtyMapName
                    results[(field_id, data_desc_id)]\
                     ['dirtyImageParameters'] = dirtyImageParameters

# get the continuum and line dirty images

                    lineDirtyMapName = ('lineDirty.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                     flag_marks)).replace(' ','')
                    continuumDirtyMapName = (
                     'continuumDirty.%s.f%s.spw%s.fm%s' % (
                    self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                     flag_marks)).replace(' ','')

# ..Assume the first and last 4th of the spectral band are line free

                    self._image.open(infile=dirtyMapName)
                    linefreechans=range(nchan/4) + range(3*nchan/4,nchan)
                    self._image.continuumsub(outline=lineDirtyMapName,
                     outcont=continuumDirtyMapName,
                     channels=linefreechans, fitorder=1, overwrite=True)
                    self._image.close()

                    results[(field_id, data_desc_id)]\
                     ['lineDirtyMapName'] = lineDirtyMapName
                    results[(field_id, data_desc_id)]\
                     ['continuumDirtyMapName'] = continuumDirtyMapName

# average continuum cube over all channels

                    self._image.open(continuumDirtyMapName)
                    pixels = self._image.getchunk(axes=[3], list=True,
                     dropdeg=False)
                    csys=self._image.coordsys()
                    self._image.close()
                    flatMap = self._image.newimagefromarray(
                     outfile=continuumDirtyMapName, pixels=pixels,
                     csys=csys.torecord(), overwrite=True)

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
 
# first make a clean integrated image and use it to find the sources, this
# may fail

                    start_boxes = [[nx/4, nx/4, 3*nx/4, 3*nx/4]]
                    quarter_box = [nx/4, nx/4, 3*nx/4, 3*nx/4]

                    try:
                        integrated_threshold, integrated_rms,\
                         integrated_cleaned_rms, integrated_cleaned_rms_record,\
                         integrated_sum, integrated_sum_change,\
                         integrated_rms2d, integrated_max, integrated_commands,\
                         integrated_endState = \
                         self._clean(data_desc_id, field_id, nx, cell,
                         'mfs', -1, bmaj, bmin, bpa, start_boxes,
                         quarter_box,
                         cleanIntegratedModelName, cleanIntegratedMapName,
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

                        results[(field_id, data_desc_id)]['error']['pilot_clean'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']['standard_clean'] = \
                         error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_clean'] = error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['line_clean'] = error_report

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
                    results[(field_id,data_desc_id)]['integrated_sum_change']\
                     = integrated_sum_change
                    results[(field_id,data_desc_id)]['integrated_rms2d'] = \
                     integrated_rms2d
                    results[(field_id,data_desc_id)]['integrated_max'] = \
                     integrated_max
                    results[(field_id,data_desc_id)]['integrated_endState'] = \
                     integrated_endState
                    results[(field_id,data_desc_id)]['integratedQuarterBox'] = \
                     quarter_box

                    cutoff, nSource, sources, boxes, source_commands = \
                     self._findSourcesAndCleanBoxes(cleanIntegratedMapName,
                     integrated_rms, bmaj, bmin, nx)

                    for item in source_commands:
                        integrated_commands.append(item)
                    results[(field_id,data_desc_id)]['integrated_commands'] = \
                     integrated_commands
                    results[(field_id,data_desc_id)]['cutoff'] = cutoff
                    results[(field_id,data_desc_id)]['nSource'] = nSource
                    results[(field_id, data_desc_id)]['boxes'] = boxes
                    results[(field_id, data_desc_id)]['sources'] = sources

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

                    results[(field_id, data_desc_id)]\
                     ['cleanMapName'] = cleanMapName
                    results[(field_id, data_desc_id)]\
                     ['cleanModelName'] = cleanModelName
                    results[(field_id, data_desc_id)]\
                     ['cleanResidualMapName'] = cleanResidualMapName

# set the image parameters for the cleaned cube

                    empty_channels = None
                    if self._mode=='channel':
                        nchan = self._num_chan[self._spectral_window_id[
                         data_desc_id]]

# and generate channel flags from channels with no valid data in the MS

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
                    else:
                        nchan = 1
                    results[(field_id,data_desc_id)]['nchan'] = nchan
                    results[(field_id, data_desc_id)]\
                     ['emptyChannels'] = empty_channels

# get bandpass 'flagged' channels, ..only gets the first flagging occurrence
# for each spw

                    noisy_channels = None
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

                    results[(field_id, data_desc_id)]\
                     ['noisyChannels'] = noisy_channels
                    results[(field_id, data_desc_id)]\
                     ['bandpassFlaggingStage'] = self._bandpassFlaggingStage

# do the cleaning, this may fail

                    print 'integrated threshold', integrated_threshold
                    print 'second clean'
                    try:
                        threshold, rms, cleaned_rms, cleaned_rms_record, \
                         sum, sum_change, rms2d,\
                         cleaned_max, finalCleanCommands, finalCleanEndState = \
                         self._clean(
                         data_desc_id, field_id, nx, cell, self._mode,
                         nchan, bmaj, bmin, bpa, boxes, quarter_box,
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

                        results[(field_id, data_desc_id)]['error']\
                         ['standard_clean'] = error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_clean'] = error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['line_clean'] = error_report

                        continue

                    results[(field_id,data_desc_id)]['threshold'] = threshold
                    results[(field_id,data_desc_id)]['rms'] = rms
                    results[(field_id,data_desc_id)]['cleaned_rms'] = \
                     cleaned_rms
                    results[(field_id,data_desc_id)]['cleaned_rms_record'] = \
                     cleaned_rms_record
                    results[(field_id,data_desc_id)]['sum_change'] = sum_change
                    results[(field_id,data_desc_id)]['rms2d'] = rms2d
                    results[(field_id,data_desc_id)]['max'] = cleaned_max
                    results[(field_id,data_desc_id)]['finalCleanCommands'] = \
                     finalCleanCommands
                    results[(field_id,data_desc_id)]['finalCleanEndState'] = \
                     finalCleanEndState
                    results[(field_id,data_desc_id)]['finalQuarterBox'] = \
                     quarter_box

# now do image plane clean of continuum dirty image

                    continuumCleanMapName = (
                     'continuumClean.%s.f%s.spw%s.fm%s' % (self._base_msName,
                     self._cleanFieldName[field_id], data_desc_id, flag_marks)
                     ).replace(' ', '')
                    continuumModelMapName = (
                    'continuumModel.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                     flag_marks)).replace(' ', '')
                    continuumResidualMapName = (
                     'continuumResidual.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                     flag_marks)).replace(' ', '')

                    results[(field_id, data_desc_id)]\
                     ['continuumCleanMapName'] = continuumCleanMapName
                    results[(field_id, data_desc_id)]\
                     ['continuumModelMapName'] = continuumModelMapName
                    results[(field_id, data_desc_id)]\
                     ['continuumResidualMapName'] = continuumResidualMapName

                    print 'continuum clean', integrated_rms
                    try:
                        continuum_threshold, continuum_rms, continuum_cleaned_rms,\
                         continuum_rms2d, continuum_cleaned_max,\
                         continuumCleanCommands, continuumCleanEndState = \
                         self._imagePlaneclean(continuumDirtyMapName, psfMapName, 
                         'mfs', 1, bmaj, bmin, bpa, boxes, quarter_box, 
                         continuumModelMapName, continuumCleanMapName,
                         continuumResidualMapName, target_rms=integrated_rms)
                    except KeyboardInterrupt:
                        raise
                    except:
                        error_report = self._htmlLogger.openNode('exception',
                         '%s.continuum_clean_exception' % (self._stageName), True,
                         stringOutput = True)

                        self._htmlLogger.logHTML('Exception details<pre>')
                        traceback.print_exc()
                        traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                        self._htmlLogger.logHTML('</pre>')
                        self._htmlLogger.closeNode()

                        error_report += 'during clean'

                        results[(field_id, data_desc_id)]['error']\
                         ['continuum_clean'] = error_report
                        results[(field_id, data_desc_id)]['error']\
                         ['line_clean'] = error_report

                        continue

                    results[(field_id,data_desc_id)]['continuum_threshold'] = \
                     continuum_threshold
                    results[(field_id,data_desc_id)]['continuum_rms'] = \
                     continuum_rms
                    results[(field_id,data_desc_id)]['continuum_cleaned_rms'] = \
                     continuum_cleaned_rms
                    results[(field_id,data_desc_id)]['continuum_rms2d'] = \
                     continuum_rms2d
                    results[(field_id,data_desc_id)]['continuum_max'] = \
                     continuum_cleaned_max
                    results[(field_id,data_desc_id)]['continuumCleanCommands'] = \
                     continuumCleanCommands
                    results[(field_id,data_desc_id)]['continuumCleanEndState'] = \
                     continuumCleanEndState
                    results[(field_id,data_desc_id)]['continuumQuarterBox'] = \
                     quarter_box

# now do image plane clean of the line dirty image 

                    lineCleanMapName = ('lineClean.%s.f%s.spw%s.fm%s' % (
                     self._base_msName,
                     self._cleanFieldName[field_id], data_desc_id, flag_marks)
                     ).replace(' ', '')
                    lineModelMapName = ('lineCleanModel.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                     flag_marks)).replace(' ', '')
                    lineResidualMapName = (
                     'lineResidual.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._cleanFieldName[field_id], data_desc_id,
                     flag_marks)).replace(' ', '')

                    results[(field_id, data_desc_id)]\
                     ['lineCleanMapName'] = lineCleanMapName
                    results[(field_id, data_desc_id)]\
                     ['lineModelMapName'] = lineModelMapName
                    results[(field_id, data_desc_id)]\
                     ['lineResidualMapName'] = lineResidualMapName

                    print 'line clean', integrated_rms
                    try:
                        line_threshold, line_rms, line_cleaned_rms,\
                         line_rms2d, line_cleaned_max, lineCleanCommands, \
                         lineCleanEndState = \
                         self._imagePlaneclean(lineDirtyMapName, psfMapName,
                         'channel', nchan, bmaj, bmin, bpa, boxes, quarter_box,
                         lineModelMapName, lineCleanMapName, lineResidualMapName,
                         target_rms=rms)
                    except KeyboardInterrupt:
                        raise
                    except:
                        error_report = self._htmlLogger.openNode('exception',
                         '%s.line_clean_exception' % (self._stageName), True,
                         stringOutput = True)

                        self._htmlLogger.logHTML('Exception details<pre>')
                        traceback.print_exc()
                        traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                        self._htmlLogger.logHTML('</pre>')
                        self._htmlLogger.closeNode()

                        error_report += 'during clean'

                        results[(field_id, data_desc_id)]['error']\
                         ['line_clean'] = error_report

                        continue

                    results[(field_id,data_desc_id)]['line_threshold'] = \
                     line_threshold
                    results[(field_id,data_desc_id)]['line_rms'] = line_rms
                    results[(field_id,data_desc_id)]['line_cleaned_rms'] = \
                     line_cleaned_rms
                    results[(field_id,data_desc_id)]['line_rms2d'] = line_rms2d
                    results[(field_id,data_desc_id)]['line_max'] = line_cleaned_max
                    results[(field_id,data_desc_id)]['lineCleanCommands'] = \
                     lineCleanCommands
                    results[(field_id,data_desc_id)]['lineCleanEndState'] = \
                     lineCleanEndState
                    results[(field_id,data_desc_id)]['lineQuarterBox'] = \
                     quarter_box

# store the object info in the BookKeeper.
# copying by reference the history dictionaries may lead to problems.

            parameters['data'] = results

            self._bookKeeper.enter(objectType=inputs['objectType'],
             sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             outputFiles=[],
             outputParameters=parameters,
             dependencies=inputs['dependencies'])
            self._htmlLogger.timing_stop(
             'ContinuumSubtractedCleanImage.calculateNew')

# restore the flag state on entry and adopt is as 'Current'

            self._msFlagger.setFlagState('CleanEntry')
            self._msFlagger.adoptAsCurrentFlagState()

        self._parameters = parameters
        self._htmlLogger.timing_stop('ContinuumSubtractedCleanImage.calculate')
        return parameters


    def description(self):
        description = \
         ['ContinuumSubtractedCleanImage - cleaned images with continuum subtracted']
        return description


    def getData(self, logName=None):
        """Public method to return the cleaned images as a 'view' of the data.
        
        Keyword arguments:
        logName -- The name of the casapy log file.
        """

#        print 'ContinuumSubtractedCleanImage.getData called'

        self._htmlLogger.timing_start('ContinuumSubtractedCleanImage.getdata')
        parameters = self.calculate(logName=logName)
        self._results['parameters'] = parameters

# iterate through fields and data_descs extracting the image for each in turn

        for k,v in parameters['data'].iteritems():
            field_id = k[0]
            data_desc_id = k[1] 

# for a continuum spw there will be no results

            nchan = self._results['summary']['nchannels'][data_desc_id]
            if nchan==1:
                continue

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (a) Stokes:I - Standard clean image' % (
             self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'clean map',
             v, mapField='cleanMapName', cleanBoxField='boxes',
             thresholdField='threshold', rms2dField='rms2d',
             noisyChannelField='noisyChannels',
             bandpassFlaggingStageField = 'bandpassFlaggingStage',
             quarterBoxField='finalQuarterBox',
             emptyChannelField='emptyChannels', error_key='standard_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (b) Stokes:I - Standard clean residual' % (
             self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'clean residual',
             v, mapField='cleanResidualMapName', cleanBoxField='boxes',
             thresholdField='threshold', rms2dField='rms2d',
             noisyChannelField='noisyChannels',
             bandpassFlaggingStageField = 'bandpassFlaggingStage',
             quarterBoxField='finalQuarterBox',
             emptyChannelField='emptyChannels', error_key='standard_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (c) Stokes:I - ContinuumCleanImage' \
             % (self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description,
             'continuum clean integrated map',
             v, mapField='continuumCleanMapName', cleanBoxField='boxes',
             quarterBoxField='continuumQuarterBox',
             error_key='continuum_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (d) Stokes:I - ContinuumCleanResidualImage' \
             % (self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description,
             'continuum clean residual integrated map',
             v, mapField='continuumResidualMapName', cleanBoxField='boxes',
             quarterBoxField='continuumQuarterBox',
             error_key='continuum_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (e) Stokes:I - LineCleanImage' \
             % (self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description,
             'line clean integrated map',
             v, mapField='lineCleanMapName', cleanBoxField='boxes',
             quarterBoxField='lineQuarterBox',
             noisyChannelField='noisyChannels',
             bandpassFlaggingStageField = 'bandpassFlaggingStage',
             emptyChannelField='emptyChannels', error_key='line_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (f) Stokes:I - LineCleanResidualImage' \
             % (self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description,
             'line clean residual integrated map',
             v, mapField='lineResidualMapName', cleanBoxField='boxes',
             quarterBoxField='lineQuarterBox',
             noisyChannelField='noisyChannels',
             bandpassFlaggingStageField = 'bandpassFlaggingStage',
             emptyChannelField='emptyChannels', error_key='line_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (g) Stokes:I - Pilot integrated clean image' \
             % (self._fieldName[field_id], self._fieldType[field_id],
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'clean integrated map',
             v, mapField='cleanIntegratedMapName', 
             cleanBoxField='integrated_boxes',
             thresholdField='integrated_threshold',
             quarterBoxField='integratedQuarterBox',
             rms2dField='integrated_rms2d', error_key='pilot_clean')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (h) Stokes:I - Pilot integrated residual image' \
             % (self._fieldName[field_id], self._fieldType[field_id],
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'residual integrated map',
             v, mapField='cleanIntegratedResidualMapName', 
             cleanBoxField='integrated_boxes', 
             thresholdField='integrated_threshold',
             quarterBoxField='integratedQuarterBox',
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
             'Field:%s (%s) SpW:%s (i) Stokes:I - DirtyImage' \
             % (self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'dirty integrated map',
             v, mapField='dirtyMapName',
             noisyChannelField='noisyChannels',
             bandpassFlaggingStageField = 'bandpassFlaggingStage',
             emptyChannelField='emptyChannels', error_key='dirty_image')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (j) Stokes:I - LineDirtyImage' \
             % (self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description, 'line dirty integrated map',
             v, mapField='lineDirtyMapName',
             noisyChannelField='noisyChannels',
             bandpassFlaggingStageField = 'bandpassFlaggingStage',
             emptyChannelField='emptyChannels', error_key='continuum_subtraction')

            description = {}
            description['TITLE'] = \
             'Field:%s (%s) SpW:%s (k) Stokes:I - ContinuumDirtyImage' \
             % (self._fieldName[field_id], self._fieldType[field_id], 
             self._pad(data_desc_id))
            BaseImage._fillData(self, description,
             'continuum dirty integrated map',
             v, mapField='continuumDirtyMapName',
             error_key='continuum_subtraction')

# return a copy of the data list, otherwise operating on it outside this class

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('ContinuumSubtractedCleanImage.getdata')
        return temp


    def inputs(self):
        """
        """
        result = {}
        result['objectType'] = 'ContinuumSubtractedCleanImage'
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
         <p>The cleaned images were constructed as follows:
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
          <li>The flagging state was set to 'Current', and a copy saved
           as 'CleanEntry' so that the MS could be restored to that state
           at the end of the cleaning process.
          <li>The bandpass and gain calibrations were calculated. 
          <li>Iterating through the fields and spws to be imaged:
           <ul>
            <li>The relevant data were selected.
            <li>The image 'weight' was set to 'natural'.
            <li>The calibrations for bandpass and gain described
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
               <td>nx</td>
               <td>cell</td>
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
               <td>%s (%s)</td>
               <td>%i</td>
               <td>%s</td>
               <td>%s</td>
               <td>%s</td>
               <td>%s</td>""" % (self._fieldName[k[0]], self._fieldType[k[0]],
               k[1], v['fov'], w['nx'], w['cell'],
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
               <td>nx</td>
               <td>cell</td>
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
               <td>%s (%s)</td>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>
               <td>%i</td>
               <td>%s</td>""" % (self._fieldName[k[0]], self._fieldType[k[0]],
               k[1], w['nx'], w['cell'], w['nchan'], w['mapName'])

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
            <li>Separate dirty images for the continuum and line emission
             were calculated (see Casapy Calls for details).
             It was assumed that the first and last quarters of the 
             spectral band were line free. 'image.continuumsub' was called to 
             fit a 1st order polynomial
             to each spectrum in the dirty map and write this to the
             continuum dirty cube, and the residual to the line dirty
             cube."""

        cleanDescription += """
             <p>continuum and line dirty image parameters:
             <table border='1'>
              <tr>
               <td>Field</td>
               <td>Spectral Window</td>
               <td>nx</td>
               <td>cell</td>
               <td>Nchan</td>
               <td>Continuum Map Name</td>
               <td>Line Map Name</td>
               <td>Casapy Calls</td>
               <td>Error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for k in keys:
            v = parameters['data'][k]
            if v['error']['continuum_subtraction'] == None:
                w = v['dirtyImageParameters']
                cleanDescription += """
              <tr>
               <td>%s (%s)</td>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>
               <td>%i</td>
               <td>%s</td>
               <td>%s</td>""" % (self._fieldName[k[0]], self._fieldType[k[0]],
               k[1], w['nx'], w['cell'], w['nchan'],
               v['continuumDirtyMapName'], v['lineDirtyMapName'])

# casapy calls in a separate node      

                cleanDescription += '''
               <td>'''
                cleanDescription += self._htmlLogger.openNode('casapy calls',
                 '%s.field%sspw%s.%s' % (stageName, k[0], k[1], 
                 'continuumsub_casapy_calls'), True, stringOutput=True)
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
               <td>%s</td>
              </tr>""" % (k[0], k[1], v['error']['continuum_subtraction'])
        cleanDescription += """
             </table>"""

        cleanDescription += """
            <li>A 'pilot' integrated image was made and searched for 
             sources so that clean boxes could be allocated for
             cleaning the cube. The clean box for the pilot image
             covered the centre 3/4 of the whole. 
             <ul>
              <li>'imager.defineimage' set the cell and map sizes.
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
                 continue toward a lower threshold.
               </ul> 
              <li>'image.findSources' was called to find any sources
               present whose total flux was greater than the 'cutoff'
               fraction of the strongest source found. The value of 
               'cutoff' is given in the table below and was set to 
               5 times the rms of the pixels divided by the maximum
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
               <td>nx</td>
               <td>cell</td>
               <td>threshold</td>
               <td>Quality</td>
               <td>Non-cleaned rms</td>
               <td>Cleaned rms</td>
               <td>Cleaned rms Record</td>
               <td>Flux Change</td>
               <td>2d rms</td>
               <td>Cutoff</td>
               <td>Sources</td>
               <td>Casapy Calls</td>
               <td>Error</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for k in keys:
            v = parameters['data'][k]
            if v['error']['pilot_clean'] == None:
                cleanDescription += """
              <tr>
               <td>%s (%s)</td>
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
               <td>%.1e</td>""" % (
                 self._fieldName[k[0]], self._fieldType[k[0]],
                 k[1], v['nx'], v['cell'], v['integrated_threshold'],
                 v['integrated_endState'], v['integrated_rms'],
                 v['integrated_cleaned_rms'],
                 self._formatList(v['integrated_cleaned_rms_record'], '%.2g'),
                 v['integrated_sum_change'], v['integrated_rms2d'], v['cutoff'])

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
                  <td>size (pix)</td>
                  <td>flux</td>
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
               <td>%s</td>
              </tr>''' % (k[0], k[1], v['error']['pilot_clean'])

        cleanDescription += """
             </table>
             <ul>
              <li>'threshold' gives the target threshold of the last clean.
              <li>'Non-cleaned rms' was calculated using all data in the
               residual image/cube outside the cleaned area.
              <li>'Cleaned rms' was calculated using data inside the cleaned
               area.
              <li>'Flux Change' gives the factional increase in cleaned flux in 
               the last clean.
              <li>'2d rms' was calculated from pixels in the centre quarter
               of the continuum residual image.
             </ul>


            <li>An image plane clean of the 'line' dirty cube was done
             using the 'deconvolver' tool (see Casapy Calls for details).
             The clean threshold was 2 * rms of the residual on the
             corresponding standard cleaned image.

             <p>Line clean cube parameters:
             <table border='1'>
              <tr>
               <td>Field</td> 
               <td>Spectral Window</td>
               <td>nx</td>
               <td>cell</td>
               <td>threshold</td>
               <td>Quality</td>
               <td>Non-cleaned rms</td>
               <td>Cleaned rms</td>
               <td>rms 2d</td>
               <td>Max</td>
               <td>Casapy Calls</td>
               <td>Error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for k in keys:
            v = parameters['data'][k]
            if v['error']['line_clean'] == None:
                cleanDescription += """
              <tr>
               <td>%s (%s)</td>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>""" % (self._fieldName[k[0]], self._fieldType[k[0]],
                      k[1], v['nx'], v['cell'],
                      v['line_threshold'], v['lineCleanEndState'], 
                      v['line_rms'], v['line_cleaned_rms'],
                      v['line_rms2d'], v['line_max'])

# casapy calls in a separate node      

                cleanDescription += '''
               <td>'''
                cleanDescription += self._htmlLogger.openNode('casapy calls',
                 '%s.field%sspw%s.%s' % (stageName, k[0], k[1], 
                 'line_cube_casapy_calls'), True, stringOutput=True)
                for line in v['lineCleanCommands']:
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
               <td>%s</td>
              </tr>''' % (k[0], k[1], v['error']['line_clean'])

        cleanDescription += """
             </table>
            <li>Flat representations of the clean cubes were calculated
             by averaging them over the 'channel' axis."""

        if self._bandpassFlaggingStage != None:
            cleanDescription += """
             Channels that were flagged at stage '%s' were not included in 
             the average.""" % self._bandpassFlaggingStage

        cleanDescription += """
             </table>
            <li>An image plane clean of the 'continuum' dirty integrated map 
             was done using the 'deconvolver' tool, using a method similar to
             that described for the line cube.

             <p>Continuum clean map parameters:
             <table border='1'>
              <tr>
               <td>Field</td> 
               <td>Spectral Window</td>
               <td>nx</td>
               <td>cell</td>
               <td>threshold</td>
               <td>Quality</td>
               <td>Non-cleaned rms</td>
               <td>Cleaned rms</td>
               <td>rms 2d</td>
               <td>Max</td>
               <td>Casapy Calls</td>
               <td>Error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for k in keys:
            v = parameters['data'][k]
            if v['error']['continuum_clean'] == None:
                cleanDescription += """
              <tr>
               <td>%s (%s)</td>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>""" % (self._fieldName[k[0]], self._fieldType[k[0]],
                      k[1], v['nx'], v['cell'],
                      v['continuum_threshold'],
                      v['continuumCleanEndState'],
                      v['continuum_rms'], v['continuum_cleaned_rms'],
                      v['continuum_rms2d'],
                      v['continuum_max'])

# casapy calls in a separate node      

                cleanDescription += '''
               <td>'''
                cleanDescription += self._htmlLogger.openNode('casapy calls',
                 '%s.field%sspw%s.%s' % (stageName, k[0], k[1], 
                 'continuum_map_casapy_calls'), True, stringOutput=True)
                for line in v['continuumCleanCommands']:
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
               <td>%s</td>
              </tr>''' % (k[0], k[1], v['error']['continuum_clean'])

        cleanDescription += """
             </table>
             <ul>
              <li>'threshold' gives the target threshold of the last clean.
              <li>'Non-cleaned rms' was calculated using all data in the
               residual image/cube outside the cleaned area.
              <li>'Cleaned rms' was calculated using data inside the cleaned
               area.
              <li>'2d rms' was calculated from pixels in the centre quarter
               of the continuum residual image.
             </ul>"""

        cleanDescription += """
           </ul>
         </ul>"""
#         <p>The clean image was constructed by Python class 
#         ContinummSubtractedCleanImage."""

        cleanImageDescription['clean image'] = cleanDescription
        return cleanImageDescription


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.
                
        Keyword arguments:
        stageName -- Name of the recipe stage using this object.  
        """

        description = self.createGeneralHTMLDescription(stageName)
        self._htmlLogger.logHTML("""<p>The data view is a clean image.""")


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
