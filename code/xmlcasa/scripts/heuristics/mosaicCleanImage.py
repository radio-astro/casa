"""Module to supply cleaned images."""

# History:
#  8-Feb-2008 jfl First (not quite working) version.
# 26-Sep-2008 jfl mosaic release.
# 23-Oct-2008 jfl rms2d bug fix.
#  3-Nov-2008 jfl amalgamated stage release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import math
from numpy import *
import types

# alma modules

from cleanImageV2 import *

class MosaicCleanImage(CleanImageV2):
    """Class to supply cleaned images.
    """

    def _clean(self, data_desc_id, centre_measure, field_ids, nx, ny, cell,
     mode, nchan, boxes, modelName, cleanMapName, residualMapName,
     integrated_rms=None):
        """Private method to clean an image.
        """
        commands = []

        self._imager.open(thems=self._msName)
        self._imager.selectvis(spw=data_desc_id, field=field_ids)

# set the image parameters

        self._imager.defineimage(nx=nx, ny=ny, cellx=cell, celly=cell,
         stokes='I', phasecenter=centre_measure, mode=mode, nchan=nchan,
         spw=[data_desc_id])

# do primary beam correction

        self._imager.setvp(dovp=True)
        self._imager.setmfcontrol(cyclefactor=3.0, stoplargenegatives=-1)

# make mask from boxes

# ..remove box.mask explicitly if it exists - I have the feeling that
#   it does not get replaced if already present.

        if os.path.exists('box.mask'):
            self._rmall('box.mask')

# ensure box coords are int - some casapy versions complain if not

        new_boxes = []
        for box in boxes:
            new_box = []
            for item in box:
                new_box.append(int(item))
            new_boxes.append(new_box)
        self._imager.regionmask(mask='box.mask', boxes=new_boxes)

# remove any previous model

        if os.path.exists(modelName):
            self._rmall(modelName)

# update commands list

        commands.append('imager.open(thems=%s)' % self._msName)
        commands.append('imager.selectvis(spw=%s, field=%s)' % (data_desc_id,
         field_ids))
        commands.append("""imager.defineimage(nx=%s, ny=%s, cellx=%s, celly=%s,
         stokes='I', phasecenter=%s, mode=%s, nchan=%s, spw=[%s])""" % (nx, ny,
         cell, cell, centre_measure, mode, nchan, data_desc_id))
        commands.append('imager.setvp(dovp=True)')
        commands.append("""imager.setmfcontrol(cyclefactor=3.0, 
         stoplargenegatives=-1)""")
        commands.append("# remove 'box.mask'")
        commands.append("imager.regionmask(mask='box.mask', boxes=%s)" %
         new_boxes)
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
#            threshold = min(0.1, 2 * rms)
            threshold = 2 * rms
            sum = 0.0

        while cleaning:
            if self._verbose:
                print 'clean to threshold', threshold
            print 'clean to threshold', threshold

            self._imager.clean(algorithm=self._algorithm,
             mask='box.mask', niter=2000,
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
            model_stats = self._image.statistics(mask='box.mask > 0.1',
             robust=False)
            newSum = model_stats['sum'][0]
            self._image.close()

# and get the rms of the residual image, inside clean region and outside

            self._image.open(infile=residualMapName)
            resid_clean_stats = self._image.statistics(mask='box.mask > 0.1',
             robust=False)
            if resid_clean_stats.has_key('rms'):
                if len(resid_clean_stats['rms']) > 0:
                    newCleanRms = resid_clean_stats['rms'][0]
            else:
                print 'no rms'
                break

            resid_stats = self._image.statistics(mask='box.mask < 0.1',
             robust=False)
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
             mask='box.mask', niter=1000,
             gain=0.2, threshold="%sJy",
             displayprogress=False,
             model=%s, keepfixed=[False],
             image=%s,
             residual=%s)""" % (self._algorithm, threshold, modelName,
             cleanMapName, residualMapName))

            commands.append('image.open(infile=%s)' % modelName)
            commands.append("""rtn=image.statistics(mask='box.mask > 0.1',
             robust=False""")
            commands.append('image.close()')
            commands.append('..get model sum: %s' % newSum)

            commands.append('image.open(infile=%s)' % residualMapName)
            commands.append("""rtn=image.statistics(mask='box.mask < 0.1',
             robust=False""")
            commands.append('..get rms outside cleaned area: %s' % newRms)
            commands.append("""rtn=image.statistics(mask='box.mask > 0.1',
             robust=False""")
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

# continue cleaning? set new threshold

            
            cleanRmsRecord.append(newCleanRms)
#            if (newCleanRms < 0.8 * newRms):
#                break
#            if ((newCleanRms < newRms) and (newSum < 1.03 * sum)):
            if (newSum < 1.03 * sum):
                endState = 'OK'
                break
            elif nloop > 20:
                endState = 'nloop > 20'
                break
            else:
#                newThreshold = min(0.1, threshold, 2 * newRms)
                newThreshold = min(threshold, 2 * newRms)
                if threshold > 0.0:
                    change = (threshold - newThreshold) / threshold
                    if change < 0.1:
                        newThreshold = 0.8 * newThreshold

                nloop += 1
                threshold = newThreshold
                sum = newSum
                cleanRms = newCleanRms

# lastly, check for clean diverging?

                if (cleanRmsRecord[-1] > 5 * cleanRmsRecord[0]):
                    endState = 'diverging'
                    print 'increase', cleanRmsRecord
                    break

        return threshold, newRms, newCleanRms, cleanRmsRecord, sumChange,\
         rms2d, imMax, commands, endState


    def calculate(self): 
        """Method to calculate the cleaned mosaic.
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

            bpCalParameters = self._bpCal.calculate()
            parameters['dependencies']['bpCal'] = bpCalParameters

# get the gain calibration

            gainCalParameters = self._gainCal.calculate()
            parameters['dependencies']['gainCal'] = gainCalParameters

# iterate through data_descs, field_ids calibrating each in turn

            results = {}
            for data_desc_id in self._data_desc_ids:
                for field_id in self._target_field_ids:

# ignore if a field/spw combination with no data

                    if self._valid_field_spw.count(
                     [field_id,data_desc_id]) == 0:
                        continue

                    results[(field_id,data_desc_id)] = {
                     'fieldName':self._fieldName[field_id],
                     'DATA_DESC_ID':data_desc_id,
                     'commands':[],
                     'error':None}

# apply the calibration, this may fail

                    commands = []
                    try:
                        self._bpCal.setapply(spw=data_desc_id, field=field_id)
                        self._gainCal.setapply(spw=data_desc_id, field=field_id)
                        self._msCalibrater.correct(spw=data_desc_id,
                         field=field_id, commands=commands)
                    except KeyboardInterrupt:
                        raise
                    except:
                        results[(field_id,data_desc_id)]['error'] = \
                         'failed to calibrate'
                        continue

# iterate through data_descs, making a mosaic of all field_ids for each

            for data_desc_id in self._data_desc_ids:
                results[data_desc_id] = {}

# what are the fields that have data for this data_desc? Group the fields
# according to their source_type to cope with multiple mosaics

                mosaic_groups = {}
                for field_id in self._target_field_ids:
                    if self._valid_field_spw.count([field_id,data_desc_id]) > 0:
                        mosaic_name = self._fieldType[field_id]
                        if not mosaic_groups.has_key(mosaic_name):
                            mosaic_groups[mosaic_name] = []
                        mosaic_groups[mosaic_name].append(field_id)

# loop through the mosaic groups, imaging each

                for group,mosaic_field_ids in mosaic_groups.iteritems(): 
                    results[data_desc_id][group] = {
                     'commands':[],
                     'error':None}

                    commands = []

                    if len(mosaic_field_ids) == 0:
                        results[data_desc_id][group]['error'] = \
                         'no data for this spw'
                        continue

# use the imager.advise method to get the max cell size for each field, 
# choose the minimum one, round it down.

                    self._imager.open(thems=self._msName)
                    commands.append('imager.open(thems=%s)' % self._msName)

                    aipsfieldofview = '%4.1farcsec' % self._fieldofview[
                     data_desc_id]
                    advised_cell = []
                    for field_id in mosaic_field_ids:
                        self._imager.selectvis(spw=data_desc_id, field=field_id)
                        self._imager.weight(type='natural')
                        rtn = self._imager.advise(takeadvice=False,
                         amplitudeloss=0.5, fieldofview=aipsfieldofview)
                        advised_cell.append(rtn['cell'].value)

                        commands.append('imager.selectvis(spw=%s, field=%s)' % (
                         data_desc_id, field_id))
                        commands.append('''imager.advise(takeadvice=False,
                         amplitudeloss=0.5, fieldofview=%s)''' % 
                         aipsfieldofview)

                    advised_cell = min(advised_cell)

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
                        nxpix = self._fieldofview[data_desc_id] / cellv
                        nypix = self._fieldofview[data_desc_id] / cellv
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

# set size of image to spread of field centres plus f.o.v./2

                        nxpix = (self._fieldofview[data_desc_id] + \
                         xspread) / cellv
                        nypix = (self._fieldofview[data_desc_id] + \
                         yspread) / cellv

                    nx = self._nextLargerCompositeNumber(int(nxpix))
                    ny = self._nextLargerCompositeNumber(int(nypix))
                    if nx > self._maxPixels:
                        print "..WARNING: the ideal number of x pixels (%s)" % (nx)
                        print "     is greater than the maximum allowed (%s)" % (
                         self._maxPixels)
                        nx = self._maxPixels
 
                    print "..take cell= %s and nxpix= %s" % (cell, nx)
    
                    ny = self._nextLargerCompositeNumber(int(nypix))
                    if ny > self._maxPixels:
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

# get the psf and beam from the first field in the mosaic where the beam fit
# succeeds

                    bmaj = None
                    bmin = None
                    bpa = None
                    for field_id in mosaic_field_ids:
                        print 'calculating psf', data_desc_id, field_id
                        psfParameters = self._psf.calculate(field=field_id,
                         spw=data_desc_id, cell=cell, nx=nx)

# get the beam dimensions from the psf. Fit may have failed, if so
# go to the next loop.
                    
                        if psfParameters['error'] != None:
                            continue

                        bmaj = psfParameters['bmaj']
                        bmin = psfParameters['bmin']
                        bpa = psfParameters['bpa']
                        break

# construct names of integrated map
        
                    cleanIntegratedMapName = (
                     'cleanIntegrated.%s.f%s.spw%s.fm%s' % (self._base_msName,
                     self._fieldName[self._centre_field_id], data_desc_id,
                     flag_marks)).replace(' ','')
                    cleanIntegratedModelName = (
                     'integratedModel.%s.f%s.spw%s.fm%s' % (self._base_msName,
                     self._fieldName[self._centre_field_id], data_desc_id,
                     flag_marks)).replace(' ', '')
                    cleanIntegratedResidualMapName = (
                     'integratedResidual.%s.f%s.spw%s.fm%s' % (self._base_msName,
                     self._fieldName[self._centre_field_id], data_desc_id,
                     flag_marks)).replace(' ', '')

                    results[data_desc_id][group]['cleanIntegratedMapName'] = \
                     cleanIntegratedMapName
                    results[data_desc_id][group]['cleanIntegratedModelName'] = \
                     cleanIntegratedModelName
                    results[data_desc_id][group]['cleanIntegratedResidualMapName'] = \
                     cleanIntegratedResidualMapName
 
# first make a clean integrated image and use it to find the sources
# ..set the image parameters

# apply flags to bandpass edges to remove noise spikes there

                    self._msFlagger.rememberFlagState()
                    self._msFlagger.apply_bandpass_flags(data_desc_id,
                     mosaic_field_ids, noisy_channels)
                    print "command.append('...apply channel flags') needs to be added"

                    start_boxes = [[nx/8, ny/8, 7*nx/8, 7*ny/8]]

# do the cleaning, this may fail

                    try:
                        integrated_threshold, integrated_rms,\
                         integrated_cleaned_rms, integrated_cleaned_rms_record,\
                         integrated_sum_change,\
                         integrated_rms2d, integrated_max, integrated_commands,\
                         integrated_endState = \
                         self._clean(data_desc_id, self._centre_measure, 
                         mosaic_field_ids, nx, ny, cell,
                         'mfs', -1, start_boxes, cleanIntegratedModelName,
                         cleanIntegratedMapName,
                         cleanIntegratedResidualMapName,
                         integrated_rms=0.0)
                    except KeyboardInterrupt:
                        raise
                    except:
                        results[data_desc_id][group]['error'] = \
                         'pilot clean failed'
                        continue
                    finally:
                        self._msFlagger.recallFlagState()

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

# ..find the sources (non point sources allowed) in the cleaned image

                    cutoff, nSource, sources, boxes, source_commands = \
                     self._findSourcesAndCleanBoxes(cleanIntegratedMapName,
                     integrated_rms, bmaj, bmin, nx)

                    for item in source_commands:
                        integrated_commands.append(item)
                    results[data_desc_id][group]['integrated_commands'] = \
                     integrated_commands
                    results[data_desc_id][group]['cutoff'] = cutoff
                    results[data_desc_id][group]['nSource'] = nSource
                    results[data_desc_id][group]['boxes'] = boxes
                    results[data_desc_id][group]['sources'] = sources

# construct names for clean maps, models, residuals

                    cleanMapName = ('clean.%s.f%s.spw%s.fm%s' % (
                     self._base_msName,
                     self._fieldName[self._centre_field_id], data_desc_id, 
                     flag_marks)).replace(' ', '')
                    cleanModelName = ('cleanModel.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._fieldName[self._centre_field_id],
                     data_desc_id, flag_marks)).replace(' ', '')
                    cleanResidualMapName = ('residual.%s.f%s.spw%s.fm%s' % (
                     self._base_msName, self._fieldName[self._centre_field_id],
                     data_desc_id, flag_marks)).replace(' ', '')

                    results[data_desc_id][group]['cleanMapName'] = \
                     cleanMapName  
                    results[data_desc_id][group]['cleanModelName'] = \
                     cleanModelName
                    results[data_desc_id][group]['cleanResidualMapName'] = \
                     cleanResidualMapName

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
                        threshold, rms, cleaned_rms, cleaned_rms_record,\
                         sum_change, rms2d, cleaned_max, finalCleanCommands,\
                         endState = \
                         self._clean(data_desc_id, self._centre_measure,
                         mosaic_field_ids, nx, ny, cell, self._mode, nchan,
                         boxes, cleanModelName, cleanMapName,
                         cleanResidualMapName,
                         integrated_rms=integrated_rms)
                    except KeyboardInterrupt:
                        raise
                    except:
                        results[data_desc_id][group]['error'] = \
                         'second clean failed'
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

# store the object info in the BookKeeper.
# copying by reference the history dictionaries may lead to problems.

            parameters['data'] = results

            self._bookKeeper.enter(objectType=inputs['objectType'],
             sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             outputFiles=[],
             outputParameters=parameters,
             dependencies=inputs['dependencies'])

# restore the flag state on entry and adopt is as 'Current'

            self._msFlagger.setFlagState('CleanEntry')
            self._msFlagger.adoptAsCurrentFlagState()

        self._parameters = parameters
        self._htmlLogger.timing_stop('MosaicCleanImage.calculate')
        return parameters


    def getData(self, topLevel=False):
        """Public method to return the cleaned images as a 'view' of the data.
        
        Keyword arguments:
        topLevel -- True if this is the data 'view' to be displayed directly.
        """

#        print 'MosaicCleanImage.getData called'

        self._htmlLogger.timing_start('MosaicCleanImage.getdata')
        parameters = self.calculate()
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
                 thresholdField='threshold', rms2dField='rms2d')

                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (b) Stokes:I - Clean residual' % (
                 self._pad(data_desc_id), k)
                BaseImage._fillData(self, description, 'clean residual',
                 v, mapField='cleanResidualMapName', cleanBoxField='boxes',
                 thresholdField='threshold', rms2dField='rms2d')

                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (c) Stokes:I - Pilot integrated clean image' \
                 % (self._pad(data_desc_id), k)
                BaseImage._fillData(self, description, 'clean integrated map',
                 v, mapField='cleanIntegratedMapName', 
                 cleanBoxField='integrated_boxes',
                 thresholdField='integrated_threshold',
                 rms2dField='integrated_rms')

                description = {}
                description['TITLE'] = \
                 'SpW:%s Group:%s (c) Stokes:I - Pilot integrated residual image' \
                 % (self._pad(data_desc_id), k)
                BaseImage._fillData(self, description,
                 'residual integrated map',
                 v, mapField='cleanIntegratedResidualMapName', 
                 cleanBoxField='integrated_boxes',
                 thresholdField='integrated_threshold',
                 rms2dField='integrated_rms')

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
            <li>Iterate through the mosaic 'groups', imagin each in turn.
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

        cleanDescription += """
            <li>The p.s.f. was calculated for each field in the mosaic
             group and the 'beam' taken to be that of the first 
             field for which the beam fit succeeded. This beam was used
             later during the process of locating sources in the
             'pilot' cleaned image."""

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
              <li>'imager.defineimage' set the cell and map sizes.
              <li>'imager.setvp' set the primary beam correction.
              <li>A loop was entered with the image being cleaned down
               further each time:
               <ul>
                <li>'imager.clean' was called to perform 2000 clean
                 iterations towards a threshold of 0Jy.
                <li>'image.statistics' was used on the model
                 image to derive the sum of the components cleaned
                 from the data.
                <li>The clean loop could be exited for one of two reasons.
                 <ul>
                  <li>If the cleaned flux had
                   climbed less than 3 per cent in the last clean, 
                   which would indicate that most source flux had been
                   cleaned and that further cleaning was
                   likely to harvest mostly noise. This is the 
                   preferred exit mode that marks the end of a
                   successful clean. In this case the 'quality'
                   entry in the results table is marked as 'OK'.
                  <li>If clean had looped more than 20 times. In this case
                   the 'quality' entry is marked as 'nloop > 20'.
                 </ul>
                 If no exit criterion was fulfilled then the clean would
                 continue.
               </ul>
              <li>'image.findSources' was called to find any sources
               present whose total flux was greater than the 'cutoff'
               fraction of the strongest source found. The value of
               'cutoff' is given in the table below and was set to
               10 times the rms of the pixels divided by the maximum
               pixel value. The sources found were examined and those
               whose mean surface brightness per beam fell below 10 times
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
             <p>Pilot image parameters.
             <table border='1'>
              <tr>
               <td>Spectral Window</td>
               <td>Mosaic Group</td>
               <td>nx</td>
               <td>ny</td>
               <td>cell</td>
               <td>threshold</td>
               <td>quality</td>
               <td>non cleaned rms</td>
               <td>cleaned rms</td>
               <td>cleaned rms record</td>
               <td>flux change</td>
               <td>2d rms</td>
               <td>cutoff</td>
               <td>Sources</td>
               <td>casapy calls</td>
               <td>error</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for spw in keys:
            if type(spw) != types.IntType:
                continue

            for k in parameters['data'][spw].keys():
                v = parameters['data'][spw][k]
                if v['error'] == None:
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
                     v['integrated_cleaned_rms_record'],
                     v['integrated_sum_change'], v['integrated_rms2d'],
                     v['cutoff'])

                    if v['nSource'] == 0:
                        cleanDescription += '''
               <td>None</td>'''

                    else:
                        cleanDescription += """
               <td>
                <table border='1'>
                 <tr>
                  <td>Source #</td>
                  <td>x offset (pix)</td>
                  <td>y offset (pix)</td>\
                  <td>size (pix)</td>
                  <td>flux</td>
                 </tr>"""

                        for i,source in enumerate(v['sources']):
                            cleanDescription += """
                 <tr>
                  <td>%i</td>
                  <td>%4.1f</td>
                  <td>%4.1f</td>
                  <td>%4.1f</td>
                  <td>%5.3f %s</td>
                 </tr>""" % (i, source['xPosition'],
                             source['yPosition'], source['sourceSize'],
                             source['flux'], source['fluxUnit'])

                        cleanDescription += '''
                </table>
               </td>'''

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
               <td></td>
              </tr>'''

                else:
                    cleanDescription += '''
              <tr>
               <td>%i</td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td>%s</td>
              </tr>''' % (k, v['error'])

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
               <td>nx</td>
               <td>ny</td>
               <td>cell</td>
               <td>threshold</td>
               <td>quality</td>
               <td>non cleaned rms</td>
               <td>cleaned rms</td>
               <td>cleaned rms record</td>
               <td>flux change</td>
               <td>rms 2d</td>
               <td>max</td>
               <td>casapy calls</td>
               <td>error?</td>
              </tr>"""

        keys = parameters['data'].keys()
        keys.sort()
        for spw in keys:
            for k in parameters['data'][spw].keys():
                if type(k) != types.IntType:
                    continue

                v = parameters['data'][spw][k]
                if v['error'] == None:
                    cleanDescription += """
              <tr>
               <td>%i</td>
               <td>%s</td>
               <td>%i</td>
               <td>%i</td>
               <td>%s</td>
               <td>
                <table>""" % (spw, k, v['nx'], v['ny'], v['cell'])

#                    for box in v['boxes']:
#                        cleanDescription += """
#                 <tr>
#                  <td>[%4.1f, %4.1f, %4.1f, %4.1f]</td>
#                 </tr>""" % (
#                         box[0], box[1], box[2], box[3])

                    cleanDescription += """
                </table>
               </td>
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%s</td>
               <td>%.1e</td>
               <td>%.1e</td>
               <td>%.1e</td>""" % (v['threshold'], v['endState'], v['rms'],
                      v['cleaned_rms'], v['cleaned_rms_record'], 
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
               <td></td>'''

                else:
                    cleanDescription += '''
              <tr>
               <td>%i</td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td></td>
               <td>%s</td>
              </tr>''' % (k, v['error'])

        cleanDescription += """
             </table>
             <ul>
              <li>'threshold' gives the target threshold of the last clean.
              <li>'quality' says why the clean loop stopped.
              <li>'non cleaned rms' was calculated using all data in the
               residual image/cube outside the cleaned area.
              <li>'cleaned rms' was calculated using data inside the cleaned
               area.
              <li>'flux change' gives the factional increase in cleaned flux in
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
         </ul>
         <p>The clean image was constructed by Python class MosaicCleanImage."""
        cleanImageDescription['clean image'] = cleanDescription
        return cleanImageDescription
