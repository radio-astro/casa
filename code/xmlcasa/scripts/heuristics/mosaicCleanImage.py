"""Module to supply cleaned images."""

# History:
#  8-Feb-2008 jfl First (not quite working) version.
# 26-Sep-2008 jfl mosaic release.
# 23-Oct-2008 jfl rms2d bug fix.
#  3-Nov-2008 jfl amalgamated stage release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.
# 31-Jul-2009 jfl no maxPixels release.

# package modules

import math
from numpy import *
import types

# alma modules

from cleanImageV2 import *

class MosaicCleanImage(CleanImageV2):
    """Class to supply cleaned images.
    """

    def _approximatePsf(self, data_desc_id, centre_measure, field_ids, nx, ny,
     cell, psfName):
        """Private method to calculate an approximate psf.

        Keyword arguments:
        data_desc_id   -- The SpW
        centre_measure -- Direction Measure specifying centre of mosaic.
        field_ids      -- Fields contributing to mosaic.
        nx             -- Number of pixels in x.
        ny             -- Number of pixels in y.
        cell           -- cell size.
        psfName        -- Name of file to contain psf.
        """
        commands = []

        self._imager.open(thems=self._msName)
        self._imager.selectvis(spw=int(data_desc_id), field=field_ids)

# set the image parameters

        self._imager.defineimage(nx=nx, ny=ny, cellx=cell, celly=cell,
         stokes='I', phasecenter=centre_measure, spw=[int(data_desc_id)])

# image weight

        self._imager.weight('natural')

# use the default primary beam correction for this telescope

        self._imager.setvp(dovp=True, usedefaultvp=True)

# set ftmachine following example

        self._imager.setoptions(ftmachine='mosaic')

# calculate the psf

        self._imager.approximatepsf(psf=psfName)
        self._imager.close()

# update commands list

        commands.append('imager.open(thems=%s)' % self._msName)
        commands.append('imager.selectvis(spw=int(%s), field=%s)' % (
         data_desc_id, field_ids))
        commands.append("""imager.defineimage(nx=%s, ny=%s, cellx=%s, celly=%s,
         stokes='I', phasecenter=%s, spw=[int(%s)])""" % (nx, ny,
         cell, cell, centre_measure, data_desc_id))
        commands.append("imager.weight('natural')")
        commands.append('imager.setvp(dovp=True, usedefaultvp=True)')
        commands.append("imager.setoptions(ftmachine='mosaic')")
        commands.append("imager.approximatepsf(psf='%s')" % psfName)
        commands.append("imager.close()")

        return commands


    def _clean(self, data_desc_id, centre_measure, field_ids, nx, ny, cell,
     mode, nchan, boxes, modelName, cleanMapName, residualMapName,
     fluxscaleName, integrated_rms=None):
        """Private method to clean an image.

        Keyword arguments:
        data_desc_id   -- The SpW
        centre_measure -- Direction Measure specifying centre of mosaic.
        field_ids      -- Fields contributing to mosaic.
        nx             -- Number of pixels in x.
        ny             -- Number of pixels in y.
        cell           -- cell size.
        mode           -- 'mfs' or 'channel'
        nchan          -- number of channels in cube. 
        boxes          -- list of clean boxes (corners, in pixels).
        modelName      -- file to hold model.
        cleanMapName   -- file to hold clean map.
        residualMapName-- file to hold residuals.
        fluxscaleName  -- File to hold fractional variation in flux scale.
        integrated_rms -- rms of residual from pilot 'mfs' clean.
        """
        commands = []

        self._imager.open(thems=self._msName)
        self._imager.selectvis(spw=int(data_desc_id), field=field_ids)

# set the image parameters

        self._imager.defineimage(nx=nx, ny=ny, cellx=cell, celly=cell,
         stokes='I', phasecenter=centre_measure, mode=mode, nchan=nchan,
         spw=[int(data_desc_id)])

# use the default primary beam correction for this telescope

        self._imager.setvp(dovp=True, usedefaultvp=True)

# scale the image so that the noise is uniform across it. This will prevent
# the noise from blowing up near the edges of the primary beam, at the expense
# of a non uniform flux scale. The flux scale is output to the fluxscale image.

        self._imager.setmfcontrol(cyclefactor=3.0, stoplargenegatives=-1,
         scaletype='SAULT', fluxscale=fluxscaleName)

# construct clean mask name, remove chars that confuse 'mask' parameter later
             
        cleanmaskName = 'cleanmask.%s' % cleanMapName
        cleanmaskName = cleanmaskName.replace('-','_')
        cleanmaskName = cleanmaskName.replace('+','_')

# make mask from boxes

# ..remove cleanmask explicitly if it exists - I have the feeling that
#   it does not get replaced if already present.

        if os.path.exists(cleanmaskName):
            self._rmall(cleanmaskName)

# ensure box coords are int - some casapy versions complain if not

        new_boxes = []
        for box in boxes:
            new_box = []
            for item in box:
                new_box.append(int(item))
            new_boxes.append(new_box)
        self._imager.regionmask(mask=cleanmaskName, boxes=new_boxes)

# remove any previous model

        if os.path.exists(modelName):
            self._rmall(modelName)

# update commands list

        commands.append('imager.open(thems=%s)' % self._msName)
        commands.append('imager.selectvis(spw=int(%s), field=%s)' % (
         data_desc_id, field_ids))
        commands.append("""imager.defineimage(nx=%s, ny=%s, cellx=%s, celly=%s,
         stokes='I', phasecenter=%s, mode=%s, nchan=%s, spw=[int(%s)])""" % (nx, ny,
         cell, cell, centre_measure, mode, nchan, data_desc_id))
        commands.append('imager.setvp(dovp=True, usedefaultvp=True)')
        commands.append("""imager.setmfcontrol(cyclefactor=3.0, 
         stoplargenegatives=-1, scaletype='SAULT', fluxscale=%s)""" % 
         fluxscaleName)
#        commands.append("# remove 'box.mask'")
        commands.append("imager.regionmask(mask='%s', boxes=%s)" %
         (cleanmaskName, new_boxes))
        commands.append('# remove %s' % modelName)

# start cleaning 

        cleaning = True
        nloop = 0
        cleanRmsRecord = []
        endState = 'coding error'
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

        while cleaning:
            if self._verbose:
                print 'clean to threshold', threshold
            print 'clean to threshold', threshold

            self._imager.clean(algorithm=self._algorithm,
             mask=cleanmaskName, niter=2000,
             gain=0.2, threshold="%sJy" % (threshold),
             model=modelName,
             image=cleanMapName, 
             residual=residualMapName,
             interactive=False)

# cleaned down to this level..

            newRms = -1
            sumChange = -1
            newCleanRms = -1
            rms2d = -1
            imMax = -1

# get the sum of the model image, which can be used to estimate how
# close we are to cleaning all the source that's to be found

            self._image.open(infile=modelName)
            model_stats = self._image.statistics(
             mask='%s > 0.1' % cleanmaskName, robust=False)
            newSum = model_stats['sum'][0]
            self._image.close()

# and get the rms of the residual image, inside clean region and outside

            self._image.open(infile=residualMapName)
            resid_clean_stats = self._image.statistics(
             mask='%s > 0.1' % cleanmaskName, robust=False)
            if resid_clean_stats.has_key('rms'):
                if len(resid_clean_stats['rms']) > 0:
                    newCleanRms = resid_clean_stats['rms'][0]
            else:
                print 'no rms'
                break

            resid_stats = self._image.statistics(
             mask='%s < 0.1' % cleanmaskName, robust=False)
            if resid_stats.has_key('rms'):
                if len(resid_stats['rms']) > 0:
                    newRms = resid_stats['rms'][0]
                    print 'rms', newRms
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
            if (abs(newSum) < 1.03 * abs(sum)):
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

            elif newCleanRms < 0.5 * newRms:
                endState = 'clean rms < 0.5 non-clean rms'
                break

            else:
                endState = 'not finished'

# we do need another bout of cleaning, set a new threshold.
# Ideally set this from the rms of the non-cleaned area as this is
# decoupled from the cleaning depth. Guard against the case where there
# is no non-cleaned area which would give a 0 threshold and
# lead to overcleaning
 
                if newRms > 0.0:
                    newThreshold = min(threshold, 2 * newRms)
                else:
                    newThreshold = min(threshold, 2 * newCleanRms)
                if threshold > 0.0:

# if the threshold is stagnating then force it down a little more
# to try to speed up convergence
 
                    change = (threshold - newThreshold) / threshold
                    if change < 0.1:
                        newThreshold = 0.8 * newThreshold

                nloop += 1
                threshold = newThreshold
                sum = newSum
                cleanRms = newCleanRms

        self._imager.close()

# ..get the restoring beam from the image

        self._image.open(infile=cleanMapName)
        restoringBeam = self._image.restoringbeam()
        self._image.close()

        commands.append('imager.close()')
        commands.append('..get restoring beam from clean image')
        commands.append('image.open(infile=%s)' % cleanMapName)
        commands.append('restoringBeam = image.restoringbeam()')
        commands.append('image.close()')

        bmaj = '%4.1f%s' % (restoringBeam['major']['value'],
         restoringBeam['major']['unit'])
        bmin = '%4.1f%s' % (restoringBeam['minor']['value'],
         restoringBeam['minor']['unit'])
        bpa = '%4.1f%s' % (restoringBeam['positionangle']['value'],
         restoringBeam['positionangle']['unit'])

        return threshold, newRms, newCleanRms, cleanRmsRecord, sumChange,\
         rms2d, imMax, commands, endState, bmaj, bmin, bpa


    def calculate(self, logName=None): 
        """Method to calculate the cleaned mosaic.

        Keyword arguments:
        logName -- The name of the casapy logname. This is only needed if
                   the results are required for 'getdata' and are going to
                   be displayed in html.
        """

#        print 'MosaicCleanImage.calculate called'
        self._htmlLogger.timing_start('MosaicCleanImage.calculate')

# have these data been calculated already?
        
        inputs = self.inputs()
        flag_marks = inputs['flag_marks']

        entryID,parameters = self._bookKeeper.available(
         objectType=inputs['objectType'], sourceType=inputs['sourceType'],
         furtherInput=inputs['furtherInput'],
         dependencies=inputs['dependencies'],
         outputFiles=[])

        if entryID == None:

            self._htmlLogger.timing_start('MosaicCleanImage.calculateNew')
 
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

# iterate through data_descs, field_ids calibrating each in turn

            results = {}
            for data_desc_id in self._data_desc_ids:

# apply the calibration. Do it to all fields in each spw at once for
# efficiency reasons

# assemble field list for this spw

                fields_to_calibrate = []
                for field_id in self._target_field_ids:
                    if self._valid_field_spw.count(
                     [field_id,data_desc_id]) > 0:
                        fields_to_calibrate.append(field_id)

                self._bpCal.setapply(spw=data_desc_id, field=fields_to_calibrate)
                self._gainCal.setapply(spw=data_desc_id, field=fields_to_calibrate)
                newCommands,error = self._msCalibrater.correct(
                 spw=data_desc_id, field=fields_to_calibrate)

# populate the results structure

                for field_id in self._target_field_ids:
                    results[(field_id,data_desc_id)] = {
                     'fieldName':self._fieldName[field_id],
                     'DATA_DESC_ID':data_desc_id,
                     'commands':newCommands,
                     'error':{'calibration':error}}

# iterate through data_descs, making a mosaic of all field_ids for each

            for data_desc_id in self._data_desc_ids:
                results[data_desc_id] = {}

# what are the fields that have data for this data_desc? Group the fields
# according to their source_type to cope with multiple mosaics. Do not add
# field/spw that already have an error from calibration.

                mosaic_groups = {}
                for field_id in self._target_field_ids:
                    if self._valid_field_spw.count([field_id,data_desc_id]) == 0:
                        continue
                    if results[(field_id,data_desc_id)]['error']['calibration']\
                     != None:
                        continue

                    mosaic_name = self._fieldType[field_id]
                    if not mosaic_groups.has_key(mosaic_name):
                        mosaic_groups[mosaic_name] = []
                    mosaic_groups[mosaic_name].append(int(field_id))

# loop through the mosaic groups, imaging each

                for group,mosaic_field_ids in mosaic_groups.iteritems(): 
                    results[data_desc_id][group] = {
                     'commands':[],
                     'error':{'psf':None, 'pilot_clean':None,
                     'final_clean':None}}

                    commands = []

                    if len(mosaic_field_ids) == 0:
                        results[data_desc_id][group]['error']['pilot_clean'] = \
                         'no data for this spw'
                        results[data_desc_id][group]['error']['final_clean'] = \
                         'no data for this spw'

                        continue

# use the imager.advise method to get the max cell size for each field, 
# choose the minimum one, round it down. The fieldofview does not need
# to be the final value here.

                    self._imager.open(thems=self._msName)
                    commands.append('imager.open(thems=%s)' % self._msName)

                    aipsfieldofview = '%4.1farcsec' % self._beamRadius[
                     data_desc_id]
                    advised_cell = []

                    self._imager.selectvis(spw=int(data_desc_id), 
                     field=mosaic_field_ids)
                    rtn = self._imager.advise(takeadvice=False,
                     amplitudeloss=0.5, fieldofview=aipsfieldofview)
                    advised_cell = rtn['cell'].value

                    commands.append('imager.selectvis(spw=int(%s), field=%s)' %
                     (data_desc_id, mosaic_field_ids))
                    commands.append('''imager.advise(takeadvice=False,
                     amplitudeloss=0.5, fieldofview=%s)''' % aipsfieldofview)

                    if advised_cell > 1:
                        cellv = math.floor(advised_cell)
                    elif advised_cell > 0.1:
                        cellv = math.floor(advised_cell*10.)/10.
                    elif advised_cell > 0.01:
                        cellv = math.floor(advised_cell*100.)/100.
                    else:
                        cellv = math.floor(advised_cell*1000.)/1000.
                    cell = '%sarcsec' % cellv

# get field centre of map

                    self._table.open('%s/FIELD' % self._msName)
                    phase_dir = self._table.getcol('PHASE_DIR')
                    phase_dir_keywords = self._table.getcolkeywords('PHASE_DIR')
                    self._table.close()

                    if len(mosaic_field_ids) == 1:
                        self._centre_measure = self._measures.direction(
                         phase_dir_keywords['MEASINFO']['Ref'],
                         '%s%s' % (phase_dir[0,0,field_id],
                         phase_dir_keywords['QuantumUnits'][0]),
                        '%s%s' % (phase_dir[1,0,field_id],
                         phase_dir_keywords['QuantumUnits'][1]))

                        self._centre_field_id = mosaic_field_ids[0]

                        fieldofview = 4.0 * self._beamRadius[data_desc_id]
                        nxpix = fieldofview / cellv
                        nypix = fieldofview / cellv
                    else:

# get field centres as measures

                        mdirections = []
                        for field_id in mosaic_field_ids:
                            mdirections.append(self._measures.direction(
                             phase_dir_keywords['MEASINFO']['Ref'],
                             '%s%s' % (phase_dir[0,0,field_id],
                             phase_dir_keywords['QuantumUnits'][0]),
                             '%s%s' % (phase_dir[1,0,field_id],
                             phase_dir_keywords['QuantumUnits'][1])))

# it should be easy to calculate some 'average' direction from the
# contributing fields but it doesn't seem to be at the moment - no
# conversion beween direction measures, no calculation of a direction
# from a direction and an offset. Consequently, what follows is a bit crude.

# First, find the offset of all field from field 0.

                        xsep = []
                        ysep = []
                        for mdirection in mdirections:
                            pa = self._measures.posangle(mdirections[0],
                             mdirection)
                            sep = self._measures.separation(mdirections[0],
                             mdirection)
                            xs = self._quanta.mul(sep, self._quanta.sin(pa))
                            ys = self._quanta.mul(sep, self._quanta.cos(pa))
                            xs = self._quanta.convert(xs, 'arcsec')
                            ys = self._quanta.convert(ys, 'arcsec')
                            xsep.append(self._quanta.getvalue(xs))
                            ysep.append(self._quanta.getvalue(ys))

                        xsep = array(xsep)
                        ysep = array(ysep)
                        xspread = xsep.max() - xsep.min()
                        yspread = ysep.max() - ysep.min()
                        xcen = xsep.min() + xspread / 2.0
                        ycen = ysep.min() + yspread / 2.0

# get id of field closest to centre

                        cdist = pow(xsep-xcen, 2) + pow(ysep-ycen, 2)
                        self._centre_field_id = mosaic_field_ids[
                         argmin(cdist)]

# get direction of image centre crudely by adding offset of centre to ref
# values of field 0.

                        ref =  self._measures.getref(mdirections[0])

                        md = self._measures.getvalue(mdirections[0])
                        m0 = self._quanta.quantity(md['m0'])
                        m1 = self._quanta.quantity(md['m1'])
                        xadd = self._quanta.div('%sarcsec' % xcen,
                         self._quanta.cos(m1))
                        m0 = self._quanta.add(m0,
                         self._quanta.div('%sarcsec' % xcen, 
                         self._quanta.cos(m1)))
                        m1 = self._quanta.add(m1, '%sarcsec' % ycen)
                        self._centre_measure = self._measures.direction(
                         ref, m0, m1)

# set size of image to spread of field centres plus a border of 0.75 * beam
# radius (radius is to first null) wide

                        print 'beam radius', self._beamRadius[data_desc_id]
                        nxpix = (1.5 * self._beamRadius[data_desc_id] + \
                         xspread) / cellv
                        nypix = (1.5 * self._beamRadius[data_desc_id] + \
                         yspread) / cellv

                    nx = self._nextLargerCompositeNumber(int(nxpix))
                    ny = self._nextLargerCompositeNumber(int(nypix))
                    if (self._maxPixels != None) and (nx > self._maxPixels):
                        print "..WARNING: the ideal number of x pixels (%s)" % (nx)
                        print "     is greater than the maximum allowed (%s)" % (
                         self._maxPixels)
                        nx = self._maxPixels
 
                    print "..take cell= %s and nxpix= %s" % (cell, nx)
    
                    ny = self._nextLargerCompositeNumber(int(nypix))
                    if (self._maxPixels != None) and (ny > self._maxPixels):
                        print "..WARNING: the ideal number of y pixels (%s)" % (ny)
                        print "     is greater than the maximum allowed (%s)" % (
                         self._maxPixels)
                        ny = self._maxPixels

                    print "..take cell= %s and nypix= %s" % (cell, ny)

                    results[data_desc_id][group]['nx'] = nx
                    results[data_desc_id][group]['ny'] = ny
                    results[data_desc_id][group]['cell'] = cell

# find the noisy channels at the band edge; only gets the first
# flagging occurrence for each spw

                    noisy_channels = []
                    if self._bandpassFlaggingStage != None:
                        for row,val in enumerate(flaggingReason):
                            if val['stageDescription']['name'] == \
                             self._bandpassFlaggingStage:
                                for flagList in flagging[row].values():
                                    for flag in flagList:
                                        if data_desc_id == \
                                         flag['DATA_DESC_ID']:
                                            noisy_channels = list(
                                             flag['CHANNELS'])
                                            break
                                if noisy_channels != []:
                                    break
                            if noisy_channels != []:
                                break

# construct name of approximate psf
        
                    psfMapName = (
                     'approximatePsf.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, 
                     self._cleanFieldName[self._centre_field_id], data_desc_id,
                     flag_marks)).replace(' ','')

                    results[data_desc_id][group]['approximatePsfMapName'] = \
                     psfMapName

                    psf_commands = self._approximatePsf(data_desc_id,
                     self._centre_measure, mosaic_field_ids, nx, ny, cell,
                     psfMapName)

                    results[data_desc_id][group]['psf_commands'] = \
                     psf_commands

# construct names of integrated map
        
                    cleanIntegratedMapName = (
                     'integratedMosaicClean.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, 
                     self._cleanFieldName[self._centre_field_id], data_desc_id,
                     flag_marks)).replace(' ','')
                    cleanIntegratedModelName = (
                     'integratedMosaicModel.%s.f%s.spw%s.fm%s' % (self._base_msName,
                     self._cleanFieldName[self._centre_field_id], data_desc_id,
                     flag_marks)).replace(' ', '')
                    cleanIntegratedResidualMapName = (
                     'integratedMosaicResidual.%s.f%s.spw%s.fm%s' % (
                     self._base_msName,
                     self._cleanFieldName[self._centre_field_id], data_desc_id,
                     flag_marks)).replace(' ', '')
                    cleanIntegratedFluxscaleMapName = (
                     'integratedMosaicFluxscale.%s.f%s.spw%s.fm%s' % (
                     self._base_msName,
                     self._cleanFieldName[self._centre_field_id], data_desc_id,
                     flag_marks)).replace(' ', '')

                    results[data_desc_id][group]['cleanIntegratedMapName'] = \
                     cleanIntegratedMapName
                    results[data_desc_id][group]['cleanIntegratedModelName'] = \
                     cleanIntegratedModelName
                    results[data_desc_id][group]['cleanIntegratedResidualMapName'] = \
                     cleanIntegratedResidualMapName
                    results[data_desc_id][group]['cleanIntegratedFluxscaleMapName'] = \
                     cleanIntegratedFluxscaleMapName
 
# first make a clean integrated image and use it to find the sources
# ..set the image parameters

# apply flags to bandpass edges to remove noise spikes there

                    self._msFlagger.rememberFlagState()
                    self._msFlagger.apply_bandpass_flags(data_desc_id,
                     mosaic_field_ids, noisy_channels)

                    start_boxes = [[nx/8, nx/8, 7*nx/8, 7*ny/8]]

# do the cleaning, this may fail

                    try:
                        integrated_threshold, integrated_rms,\
                         integrated_cleaned_rms, integrated_cleaned_rms_record,\
                         integrated_sum_change,\
                         integrated_rms2d, integrated_max, integrated_commands,\
                         integrated_endState, integrated_bmaj, integrated_bmin,\
                         integrated_bpa = \
                         self._clean(data_desc_id, self._centre_measure, 
                         mosaic_field_ids, nx, ny, cell,
                         'mfs', -1, start_boxes, cleanIntegratedModelName,
                         cleanIntegratedMapName,
                         cleanIntegratedResidualMapName,
                         cleanIntegratedFluxscaleMapName,
                         integrated_rms=0.1)
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

                        results[data_desc_id][group]['error']['pilot_clean'] = \
                         error_report
                        results[data_desc_id][group]['error']['final_clean'] = \
                         error_report

                        continue
                    finally:
                        self._msFlagger.recallFlagState()

                    integrated_commands.insert(0, '...apply channel flags')

                    results[data_desc_id][group]['integrated_boxes'] = \
                     start_boxes
                    results[data_desc_id][group]['integrated_threshold'] = \
                     integrated_threshold
                    results[data_desc_id][group]['integrated_rms'] = \
                     integrated_rms
                    results[data_desc_id][group]['integrated_cleaned_rms'] = \
                     integrated_cleaned_rms
                    results[data_desc_id][group]['integrated_cleaned_rms_record'] = \
                     integrated_cleaned_rms_record
                    results[data_desc_id][group]['integrated_sum_change'] = \
                     integrated_sum_change
                    results[data_desc_id][group]['integrated_rms2d'] = \
                     integrated_rms2d
                    results[data_desc_id][group]['integrated_max'] = \
                     integrated_max
                    results[data_desc_id][group]['integrated_endState'] = \
                     integrated_endState
                    results[data_desc_id][group]['integrated_bmaj'] = integrated_bmaj
                    results[data_desc_id][group]['integrated_bmin'] = integrated_bmin
                    results[data_desc_id][group]['integrated_bpa'] = integrated_bpa

# ..find the sources (non point sources allowed) in the cleaned image

                    cutoff, nSource, sources, boxes, source_commands = \
                     self._findSourcesAndCleanBoxes(cleanIntegratedMapName,
                     integrated_rms, integrated_bmaj, integrated_bmin, nx)

                    for item in source_commands:
                        integrated_commands.append(item)
                    results[data_desc_id][group]['integrated_commands'] = \
                     integrated_commands
                    results[data_desc_id][group]['cutoff'] = cutoff
                    results[data_desc_id][group]['nSource'] = nSource
                    results[data_desc_id][group]['boxes'] = boxes
                    results[data_desc_id][group]['sources'] = sources

# construct names for clean maps, models, residuals

                    cleanMapName = ('mosaicClean.%s.f%s.spw%s.fm%s' % (
                     self._base_msName,
                     self._cleanFieldName[self._centre_field_id], data_desc_id, 
                     flag_marks)).replace(' ', '')
                    cleanModelName = ('mosaicModel.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._cleanFieldName[self._centre_field_id],
                     data_desc_id, flag_marks)).replace(' ', '')
                    cleanResidualMapName = (
                     'mosaicResidual.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._cleanFieldName[self._centre_field_id],
                     data_desc_id, flag_marks)).replace(' ', '')
                    cleanFluxscaleMapName = (
                     'mosaicFluxscale.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._cleanFieldName[self._centre_field_id],
                     data_desc_id, flag_marks)).replace(' ', '')

                    results[data_desc_id][group]['cleanMapName'] = \
                     cleanMapName  
                    results[data_desc_id][group]['cleanModelName'] = \
                     cleanModelName
                    results[data_desc_id][group]['cleanResidualMapName'] = \
                     cleanResidualMapName
                    results[data_desc_id][group]['cleanFluxscaleMapName'] = \
                     cleanFluxscaleMapName

# set nchan for final map/cube

                    if self._mode=='channel':
                        nchan = self._num_chan[self._spectral_window_id
                         [data_desc_id]]
                    else:
                        nchan = 1

                    empty_channels = []

                    results[data_desc_id][group]['nchan'] = nchan
                    results[data_desc_id][group]['emptyChannels'] = \
                     empty_channels
                    results[data_desc_id][group]['noisyChannels'] = \
                     noisy_channels
                    results[data_desc_id][group]['bandpassFlaggingStage'] = \
                     self._bandpassFlaggingStage

# do the cleaning, this may fail

                    try:
                        print 'second clean'

# guard against rms of 0 which would arise if none of the source map 
# lay outside the area cleaned in the pilot map

                        if integrated_rms > 0.0:
                            pilot_rms = integrated_rms
                        else:
                            pilot_rms = integrated_cleaned_rms

                        threshold, rms, cleaned_rms, cleaned_rms_record,\
                         sum_change, rms2d, cleaned_max, finalCleanCommands,\
                         endState, bmaj, bmin, bpa = \
                         self._clean(data_desc_id, self._centre_measure,
                         mosaic_field_ids, nx, ny, cell, self._mode, nchan,
                         boxes, cleanModelName, cleanMapName,
                         cleanResidualMapName, cleanFluxscaleMapName,
                         integrated_rms=pilot_rms)
                    except KeyboardInterrupt:
                        raise
                    except:
                        error_report = self._htmlLogger.openNode('exception',
                         '%s.final_clean_exception' % (self._stageName), True,
                         stringOutput = True)
        
                        self._htmlLogger.logHTML('Exception details<pre>')
                        traceback.print_exc()
                        traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                        self._htmlLogger.logHTML('</pre>')
                        self._htmlLogger.closeNode()

                        error_report += 'during clean'

                        results[data_desc_id][group]['error']['final_clean'] = \
                         error_report

                        continue

                    results[data_desc_id][group]['threshold'] = threshold
                    results[data_desc_id][group]['rms'] = rms
                    results[data_desc_id][group]['cleaned_rms'] = cleaned_rms
                    results[data_desc_id][group]['cleaned_rms_record'] = \
                     cleaned_rms_record
                    results[data_desc_id][group]['sum_change'] = sum_change
                    results[data_desc_id][group]['rms2d'] = rms2d
                    results[data_desc_id][group]['max'] = cleaned_max
                    results[data_desc_id][group]['finalCleanCommands'] = \
                     finalCleanCommands
                    results[data_desc_id][group]['endState'] = endState
                    results[data_desc_id][group]['bmaj'] = bmaj
                    results[data_desc_id][group]['bmin'] = bmin
                    results[data_desc_id][group]['bpa'] = bpa

#                break

# store the object info in the BookKeeper.
# copying by reference the history dictionaries may lead to problems.

            parameters['data'] = results

            self._bookKeeper.enter(objectType=inputs['objectType'],
             sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             outputFiles=[],
             outputParameters=parameters,
             dependencies=inputs['dependencies'])

# save the 'calibrated' flag state in case something else (e.g. ClosureError)
# needs it

            self._msFlagger.deleteFlagState('CleanImageCalibration')
            self._msFlagger.saveFlagState('CleanImageCalibration')

# restore the flag state on entry and adopt is as 'Current'

            self._msFlagger.setFlagState('CleanEntry')
            self._msFlagger.adoptAsCurrentFlagState()

        self._parameters = parameters
        self._htmlLogger.timing_stop('MosaicCleanImage.calculate')
        return parameters


    def getData(self, logName=None):
        """Public method to return the cleaned images as a 'view' of the data.
        
        Keyword arguments:
        logName -- The name of the casapy logname. This is only needed if
                   the results are required for 'getdata' and are going to
                   be displayed in html.
        """

#        print 'MosaicCleanImage.getData called'

        self._htmlLogger.timing_start('MosaicCleanImage.getdata')
        parameters = self.calculate(logName=logName)
        self._results['parameters'] = parameters

# iterate through data_descs extracting the image for each in turn

        for kk,vv in parameters['data'].iteritems():
            if type(kk) != types.IntType:
                continue

            data_desc_id = kk

# and iterate through mosaic groups

            for k,v in vv.iteritems():
                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (a) Stokes:I - Clean image' % (
                 self._pad(data_desc_id), k)
                BaseImage._fillData(self, description, 'clean map',
                 v, mapField='cleanMapName', cleanBoxField='boxes', 
                 thresholdField='threshold', rms2dField='rms2d',
                 error_key='final_clean')

                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (b) Stokes:I - Clean residual' % (
                 self._pad(data_desc_id), k)
                BaseImage._fillData(self, description, 'clean residual',
                 v, mapField='cleanResidualMapName', cleanBoxField='boxes',
                 thresholdField='threshold', rms2dField='rms2d',
                 error_key='final_clean')

                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (c) Stokes:I - Clean fluxscale' % (
                 self._pad(data_desc_id), k)
                BaseImage._fillData(self, description, 'clean fluxscale',
                 v, mapField='cleanFluxscaleMapName', cleanBoxField='boxes',
                 error_key='final_clean', dataUnits='flux scale factor')

                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (d) Stokes:I - Pilot integrated clean image' \
                 % (self._pad(data_desc_id), k)
                BaseImage._fillData(self, description, 'clean integrated map',
                 v, mapField='cleanIntegratedMapName', 
                 cleanBoxField='integrated_boxes',
                 thresholdField='integrated_threshold',
                 rms2dField='integrated_rms',
                 error_key='pilot_clean')

                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (e) Stokes:I - Pilot integrated residual image' \
                 % (self._pad(data_desc_id), k)
                BaseImage._fillData(self, description,
                 'residual integrated map',
                 v, mapField='cleanIntegratedResidualMapName', 
                 cleanBoxField='integrated_boxes',
                 thresholdField='integrated_threshold',
                 rms2dField='integrated_rms',
                 error_key='pilot_clean')

                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (f) Stokes:I - Pilot integrated fluxscale image' \
                 % (self._pad(data_desc_id), k)
                BaseImage._fillData(self, description,
                 'residual integrated map',
                 v, mapField='cleanIntegratedFluxscaleMapName', 
                 cleanBoxField='integrated_boxes',
                 error_key='pilot_clean', dataUnits='flux scale factor')

                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (g) Stokes:I - approximate psf' \
                 % (self._pad(data_desc_id), k)
                BaseImage._fillData(self, description,
                 'approximate psf',
                 v, mapField='approximatePsfMapName',
                 error_key='psf', dataUnits='normalised')

# return a copy of the data list, otherwise operating on it outside this class

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('MosaicCleanImage.getdata')
        return temp


    def inputs(self):
        """
        """
        result = {}
        result['objectType'] = 'MosaicCleanImage'
        result['sourceType'] = self._sourceType
        result['furtherInput'] = {'mode':self._mode,
                                 'algorithm':self._algorithm,
                                 'maxPixels':self._maxPixels}

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
          <li>Iterating through the fields and spws to be imaged:
           <ul>
            <li>The calibration results for bandpass and gain described
             above were applied to the data.
           </ul>
          <li>Iterating through the spws to be imaged:
           <ul>
            <li>The group of field ids to be mosaiced together in this
             spw was generated. The group contains fields that
             have data for this spw and have the same source 'type'.
            <li>Iterate through the mosaic 'groups', imaging each in turn.
             <ul>
              <li>The cell size for the mosaic image was calculated.
              <li>The centre of the
               mosaic image was set to the mean of the offset
               positions.
              <li>The number of pixels along the edges of the rectangular
               mosaic image were calculated.
              <li>A 'pilot' integrated image was made and searched for
               sources so that clean boxes could be allocated for
               cleaning the cube.
              <li>If no sources were found then then a default clean
               box covering the centre quarter of the image was set.
               Otherwise, square boxes were centred on each detected source.
              <li>A final image was constructed using the clean boxes
               derived from the 'pilot'.
             </ul>
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
          <li>Iterating through the fields and spws to be imaged:
           <ul>
            <li>The calibration results for bandpass and gain described
             above were applied to the data.
           </ul>
          <li>Iterating through the spws to be imaged:
           <ul>
            <li>The group of field ids to be mosaiced together in this
             spw was generated. The group contains fields that
             have data for this spw and have the same source 'type'.
            <li>The cell size for the mosaic image was calculated.
             It was set to the minimum of the values recommended by
             'imager.advise' for the contributing fields, rounded down to 
             make the value stop at the most significant digit e.g.
             0.015 would become 0.01.
            <li>The offsets of each contributing field relative to the 
             first were calculated. The centre of the
             mosaic image was set to the mean of the offset positions.
            <li>The number of pixels along the edges of the rectangular
             mosaic image were calculated. Ideally the image would cover
             all the field centres plus a boundary of thickness equal to
             the the primary beam diameter. The number of pixels was initially
             set to the length of each 'ideal' side divided by the cell size.
             This number is reset to the next greater
             'composite' number to ensure the efficient operation
             of the casapy methods used for the imaging. Here a
             'composite' number is defined as being even and a multiple
             of powers of 2, 3 and 5."""

        if self._maxPixels != None:
            cleanDescription += """
             Lastly, if the number of pixels along
             each side exceeded %s then it was reset to that ceiling
             value.""" % (self._maxPixels)

        if self._bandpassFlaggingStage != None:
            cleanDescription += """
            <li>The edge channels detected at stage %s were
             flagged in the fields to be imaged. This was to
             reduce the effect of noise spikes at the ends of the bandpass
             on the continuum image.""" % (self._bandpassFlaggingStage)

        cleanDescription += """
            <li>A 'pilot' integrated image was made and searched for
             sources so that clean boxes could be allocated for
             cleaning the cube. The clean box for the pilot image
             covered the centre 3/4 of the whole.
             <ul>
              <li>'imager.setvp' set the primary beam correction.
              <li>'imager.setmfcontrol' was used to specify a clean
               image with uniform noise, at the expense of a non uniform
               fluxscale.
              <li>A loop was entered with the image being cleaned down
               further each time:
               <ul>
                <li>'imager.clean' was called to perform 2000 clean
                 iterations towards a threshold of initially 0.1Jy.
                <li>'image.statistics' was used on the model
                 image to derive the sum of the components cleaned
                 from the data, and on the residual image to determine
                 the rms of the residuals both inside and outside the 
                 cleaned area.
                <li>The clean loop could be exited for one of several reasons.
                 <ul>
                  <li>If the cleaned flux had
                   climbed less than 3 per cent in the last clean loop, 
                   which would indicate that most source flux had been
                   cleaned and that further cleaning was
                   likely to harvest mostly noise. This is the 
                   preferred exit mode that marks the end of a
                   successful clean. In this case the 'quality'
                   entry in the results table is marked as 'OK'.
                  <li>If clean had looped more than 10 times. In this case
                   the 'quality' entry is marked as 'nloop > 10'.
                  <li>If the residual rms inside the cleaned area
                   was below 0.5 of the residual rms outside the
                   cleaned area, indicating that the clean
                   was starting to excavate the noise. In this case
                   the 'quality' entry is marked as
                   'clean rms < 0.5 * non-clean rms'.
                  <li>If the residual rms inside the cleaned area was
                   more than 5 times that after the initial
                   clean loop. In this case the 'quality' entry is marked
                   as 'diverging'.
                 </ul>
                <li>If no exit criterion was fulfilled then the clean would
                 continue, normally to a lower threshold. The new
                 threshold was set to 2 times the residual rms outside 
                 the cleaned area, unless this was 0 as could happen if
                 the clean box encompassed the entire map. In this case
                 the threshold was set to 2 times the residual
                 rms inside the cleaned area. If the new threshold
                 was greater than 0.9 of the old then
                 the new threshold was reset to 0.8 times the old, to try
                 to speed up convergence.
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
               dimension will be used in the construction of the final
               images.
             </ul>"""

        cleanDescription += """
             <p>Pilot Image Parameters.
             <table border='1'>
              <tr>
               <td>Spectral Window</td>
               <td>Mosaic Group</td>
               <td>Nx</td>
               <td>Ny</td>
               <td>Cell</td>
               <td>Threshold</td>
               <td>Quality</td>
               <td>Non-cleaned rms</td>
               <td>Cleaned rms</td>
               <td>Cleaned rms Record</td>
               <td>Flux Change</td>
               <td>2d rms</td>
               <td>Cutoff</td>
               <td>Sources</td>
               <td>Casapy Calls</td>
               <td>Error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for spw in keys:
            if type(spw) != types.IntType:
                continue

            for k in parameters['data'][spw].keys():
                v = parameters['data'][spw][k]
                if v['error']['pilot_clean'] == None:
                    cleanDescription += """
              <tr>
               <td>%i</td>
               <td>%s</td>
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
                     spw, k, v['nx'], v['ny'], v['cell'],
                     v['integrated_threshold'],
                     v['integrated_endState'], v['integrated_rms'],
                     v['integrated_cleaned_rms'],
                     self._formatList(v['integrated_cleaned_rms_record'], '%.2g'), 
                     v['integrated_sum_change'], v['integrated_rms2d'],
                     v['cutoff'])

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
                    '%s.spw%s.%s' % (stageName, k,
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
               <td>%s</td>
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
              </tr>''' % (spw, k, v['error']['pilot_clean'])

        cleanDescription += """
             </table>
            <li>Final images were constructed using the clean boxes
             <ul>"""

        if self._bandpassFlaggingStage != None:
            cleanDescription += """
              <li>The channel flagging was returned to the state
               it had prior to any flagging of edge channels done for
               the calculation of the 'pilot' image. Normally
               this just means that edge channels are unflagged."""

        cleanDescription += """
              <li>'imager.defineimage' set the cell and map sizes
               and number of channels of the cube to be created.
              <li>'imager.setvp' set the primary beam correction.
              <li>'imager.setmfcontrol' was used to specify a clean
               image with uniform noise, at the expense of a non uniform
               fluxscale.
              <li>As for the pilot integrated image, a loop was
                entered with the image cleaned further 
                each time. For each loop 'imager.regionmask'
                set the clean boxes derived from the pilot image.
             </ul> 

             <p>Final clean image parameters:
             <table border='1'>
              <tr>
               <td>Spectral Window</td>
               <td>Mosaic Group</td>
               <td>Nx</td>
               <td>Ny</td>
               <td>Cell</td>
               <td>Threshold</td>
               <td>Quality</td>
               <td>Non-cleaned rms</td>
               <td>Cleaned rms</td>
               <td>Cleaned rms Record</td>
               <td>Flux Change</td>
               <td>rms 2d</td>
               <td>Max</td>
               <td>Casapy Calls</td>
               <td>Error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for spw in keys:
            if type(spw) != types.IntType:
                continue

            for k in parameters['data'][spw].keys():
                v = parameters['data'][spw][k]
                if v['error']['final_clean'] == None:
                    cleanDescription += """
              <tr>
               <td>%i</td>
               <td>%s</td>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>""" % (spw, k, v['nx'], v['ny'], v['cell'])

                    cleanDescription += """
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>""" % (v['threshold'], v['endState'], v['rms'],
                      v['cleaned_rms'],
                      self._formatList(v['cleaned_rms_record'], '%.2g'), 
                      v['sum_change'], v['rms2d'], v['max'])

# casapy calls in a separate node

                    cleanDescription += '''
               <td>'''
                    cleanDescription += self._htmlLogger.openNode('casapy calls',
                     '%s.spw%s.%s' % (stageName, k,
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
               <td>%s</td>
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
              </tr>''' % (spw, k, v['error']['final_clean'])

        cleanDescription += """
             </table>
             <ul>
              <li>'Threshold' gives the target threshold of the last clean.
              <li>'Quality' says why the clean loop stopped.
              <li>'Non-cleaned rms' was calculated using all data in the
               residual image/cube outside the cleaned area.
              <li>'Cleaned rms' was calculated using data inside the cleaned
               area.
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
#         <p>The clean image was constructed by Python class MosaicCleanImage."""
        cleanImageDescription['clean image'] = cleanDescription
        return cleanImageDescription
