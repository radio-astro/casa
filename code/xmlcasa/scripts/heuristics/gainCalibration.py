"""Module to supply G calibration results."""

# History:
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
#  1-May-2008 jfl Re-fixed bug where scratch columns not initialised before
#                 calibration solved.
# 13-May-2008 jfl 13 release.
# 16-May-2008 jfl phases in degrees
#  2-Jun-2008 jfl 2-jun release.
# 25-Jun-2008 jfl regression release.
# 14-Jul-2008 jfl last 4769 release.
# 15-Jul-2008 jfl solve2 parameters modified for 5653
# 10-Sep-2008 jfl msCalibrater release.
# 26-Sep-2008 jfl mosaic release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import copy as python_copy
import math
from numpy import *
import os
import pickle

# alma modules

from baseData import *


class GainCalibration(BaseData):
    """Class providing G calibration results.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, bandpassCal, sourceTypes=['GAIN'], timesol=300.0,
     dataType='amplitude', bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- The name of the stage using the object.
        bandpassCal         -- class to be used for the bandpass calibration.
        sourceTypes         -- Types of source to be used for the G calibration.
        timesol             -- 'timesol' parameter passed to calibrater.
        dataType            -- The type of data to be returned by getdata.
                               'amplitude', 'phase' or 'SNR'.
        bandpassFlaggingStage -- Name of stage where bandpass edge channels
                                 were detected. If not None then these
                                 channels will be flagged in the calibrator
                                 data before the gain calibrations are
                                 calculated. The initial flag state of the
                                 MS will be restored afterward.
        """

        BaseData.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName)
        if bandpassCal == None:
            self._bpCal = None
        elif len(bandpassCal) == 1:
            self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater, 
             msFlagger, htmlLogger, msName, stageName)
        else:
            self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater, 
             msFlagger, htmlLogger, msName, stageName, 
             viewClassList=bandpassCal[1:])
        self._sourceTypes = sourceTypes
        self._timesol = timesol
        self._dataType = dataType
        self._bandpassFlaggingStage = bandpassFlaggingStage


    def _level_results(self, results):
        """Utility method that goes through the data descriptions and ensures
        that there are the same number of entries in each series of results. 
        Fill any short series with empty results.

        This can be caused by the the calibrater outputting no results
        for some data_desc_ids if it suddenly decides there are not enough data
        left.
        """

        max_length = 0
        for k in results.keys():
            max_length = max(max_length, len(results[k]))
        for k in results.keys():
            if len(results[k]) < max_length:
                results[k].append([])


    def _read_results(self, gainTable, stageEntryGainTable, originalGainTable,
     requiredDataType, results):
        """Utility method to read the G results from the gain table(s).

        Keyword arguments:
        gainTable           -- Name of gain table for current MS flag state.
        stageEntryGainTable -- Name of gain table holding the results for the
                               MS with flag state on stage entry.
        originalGainTable   -- Name of gain table holding the results for the
                               raw MS (no heuristic flagging applied).
        requiredDataType -- The type of gain information to be returned;
                            'amplitude', 'phase' or 'SNR'.
        results          -- Dictionary to add results to.
        """

#        print 'GainCalibration._read_results called: %s %s' % (gainTable,
#         gainTableOnEntry)

        if not os.path.exists(gainTable):
            print 'WARNING - %s does not exist' % gainTable
            return
        if not os.path.exists(stageEntryGainTable):
            print 'WARNING - %s does not exist' % stageEntryGainTable
            return
        if not os.path.exists(originalGainTable):
            print 'WARNING - %s does not exist' % originalGainTable
            return

# get range of data_desc_ids, antennas

        data_desc_ids = self._results['summary']['data_desc_range']
        antenna_range = self._results['summary']['antenna_range']

# open the CAL_DESC sub-table and read the column that maps CAL_DESC_ID
# to DATA_DESC_ID

        self._table.open('%s/CAL_DESC' % gainTable)
        cal_desc_2_data_desc = self._table.getcol('SPECTRAL_WINDOW_ID')[0]
        self._table.close()

        field_ids = []
        for sourceType in self._sourceTypes:
            field_ids += self.getFieldsOfType(sourceType)

# loop through data_desc_id, query table for results with matching
# CAL_DESC_ID

        for data_desc_id in data_desc_ids:
            cal_desc_id = compress(cal_desc_2_data_desc==data_desc_id,
             arange(len(cal_desc_2_data_desc)))
            if len(cal_desc_id) < 1:
                continue
            cal_desc_id = cal_desc_id[0]

            for field_id in field_ids:
                if self._valid_field_spw.count([field_id,data_desc_id]) == 0:
                    continue

# read the results themselves

                self._table.open(gainTable)
                taql = 'CAL_DESC_ID==%s && FIELD_ID==%s' % (cal_desc_id,
                 field_id)
                subTable = self._table.query(query=taql)
                antenna1 = subTable.getcol('ANTENNA1')
                gain = subTable.getcol('GAIN')
                snr = subTable.getcol('SNR')
                cal_flag = subTable.getcol('FLAG')
                field_id_column = subTable.getcol('FIELD_ID')
                time_column = subTable.getcol('TIME')
                self._table.close()

# and 'BeforeHeuristics' state info

                self._table.open(originalGainTable)
                taql = 'CAL_DESC_ID==%s && FIELD_ID==%s' % (cal_desc_id,
                 field_id)
                subTable = self._table.query(query=taql)
                antenna1_original = subTable.getcol('ANTENNA1')
                cal_flag_original = subTable.getcol('FLAG')
                time_column_original = subTable.getcol('TIME')
                self._table.close()

# and 'StageEntry' state info

                self._table.open(stageEntryGainTable)
                taql = 'CAL_DESC_ID==%s && FIELD_ID==%s' % (cal_desc_id,
                 field_id)
                subTable = self._table.query(query=taql)
                antenna1_stage_entry = subTable.getcol('ANTENNA1')
                cal_flag_stage_entry = subTable.getcol('FLAG')
                time_column_stage_entry = subTable.getcol('TIME')
                self._table.close()

# if timesol='int', get the timestamps of the data directly from the ms.
# This establishes the full extent of the data; the calibrator currently does
# not write out results for timestamps where no input data are valid.

                if self._timesol == 'int':
                    self._ms.open(self._msName)
                    self._ms.selectinit(datadescid=data_desc_id)
                    query = "FIELD_ID==%s AND (ANTENNA1!=ANTENNA2)" % (field_id)
                    rtn = self._ms.selecttaql(msselect = query)
                    times = self._ms.range(['times'])['times']
                    chunks = self._findChunks(times)
                else:

# for other timesol, use the 'on entry' time column, building array of
# discrete times in TIME column

                    times = []
                    for t in time_column_original:
                        if times.count(t) == 0:
                            times.append(t)
                    times = sort(array(times))

# construct 'chunks' to make each gain calibration a chunk - so that 
# the plotting methods will write out the associated time.

                    chunks = []
                    for it in range(len(times)):
                        chunk = [it]
                        chunks.append(chunk)

# get number of polarizations

                npol = 2
                try:
                    if alltrue(cal_flag[1,:,:]):
                        npol = 1
                except:
                    print 'exception'
                    continue

                for p in range(npol):

# declare arrays. flag has 4 versions; NoData, BeforeHeuristics, StageEntry and
# Current. There is no BeforeCalibration flag version because it is not 
# easy to separate pixels that cannot be calculated because the calibration
# method fails from those whose underlying data have all been flagged.

                    data = zeros([len(times), max(antenna_range)+1], float)
                    solution_time = [
                     zeros([len(times), max(antenna_range)+1], double),
                     zeros([len(times), max(antenna_range)+1], double),
                     zeros([len(times), max(antenna_range)+1], double),
                     zeros([len(times), max(antenna_range)+1], double)]
                    flag = [ones([len(times), max(antenna_range)+1], int),
                     ones([len(times), max(antenna_range)+1], int),
                     ones([len(times), max(antenna_range)+1], int),
                     ones([len(times), max(antenna_range)+1], int)]

# fill in NoData flags

                    for antenna in self._antenna_dictionary[
                     (field_id,data_desc_id)]:
                        flag[0][:,antenna] = 0

                    for antenna in range(max(antenna_range)):
                        solution_time[0][:, antenna] = times
                        solution_time[1][:, antenna] = times
                        solution_time[2][:, antenna] = times
                        solution_time[3][:, antenna] = times

                    for i in range(shape(gain)[2]):
                        antenna = antenna1[i]

# time sol enters here because the calibrater tool shifts the time of its
# results to the centroid of the unflagged points. The accurate time
# is stored in the solution_time array.

                        if self._timesol != 'int':
                            time_index = compress(abs(times - time_column[i])
                             < (self._timesol/2.0), arange(len(times)))
                        else:
                            time_index = compress(abs(times - time_column[i])
                             < 0.1, arange(len(times)))

                        if not(cal_flag[p,0,i]):
                            solution_time[3][time_index, antenna] = \
                             time_column[i]
                            if requiredDataType == 'amplitude':
                                data[time_index,antenna] = sqrt(
                                 gain[p,0,i].imag**2 + gain[p,0,i].real**2)
                            elif requiredDataType == 'phase':
                                data[time_index,antenna] = math.atan2(
                                 gain[p,0,i].imag, gain[p,0,i].real)
                            elif requiredDataType == 'SNR':
                                data[time_index,antenna] = snr[p,0,i]
                            else:
                                raise NameError, 'bad requiredDataType: %s' % \
                                 requiredDataType
                            flag[3][time_index, antenna] = 0

# fill BeforeHeuristics and StageEntry flag info.

                    for i in range(shape(cal_flag_original)[2]):
                        if not(cal_flag_original[p,0,i]):
                            antenna = antenna1_original[i]
                            if self._timesol != 'int':
                                time_index = compress(
                                 abs(times - time_column_original[i])
                                 < (self._timesol/2.0), arange(len(times)))
                            else:
                                time_index = compress(
                                 abs(times-time_column_original[i]) < 0.1,
                                 arange(len(times)))
                            flag[1][time_index, antenna] = 0
                            solution_time[1][time_index, antenna] = \
                             time_column_original[i]

                    for i in range(shape(cal_flag_stage_entry)[2]):
                        if not(cal_flag_stage_entry[p,0,i]):
                            antenna = antenna1_stage_entry[i]
                            if self._timesol != 'int':
                                time_index = compress(
                                 abs(times - time_column_stage_entry[i])
                                 < (self._timesol/2.0), arange(len(times)))
                            else:
                                time_index = compress(
                                 abs(times-time_column_stage_entry[i]) < 0.1,
                                 arange(len(times)))
                            flag[2][time_index, antenna] = 0
                            solution_time[2][time_index, antenna] = \
                             time_column_stage_entry[i]

                    if requiredDataType == 'phase':

# deal with phase wrapping (method assumes good s/n). First unwrap along
# time sequences, then across antennas.

                        for antenna in antenna_range:
                            last_phase = None
                            for t in range(len(times)):
                                if not(flag[-1][t,antenna]):
                                    if last_phase != None:
                                        phases = (data[t,antenna] -
                                         last_phase) + arange(-5,6) * 2.0 * pi
                                        data[t,antenna] = last_phase + phases[
                                         argmin(abs(phases))]
                                    last_phase = data[t,antenna]

# actually don't unwrap across antennas - normally the data are plotted in
# slices for each antenna and people prefer low phase angles to phase wraps 
# between antennas which are invisible in the slice display

#                        phase_wraps = arange(-5,6) * 2.0 * pi
#                        last_phase = None
#                        for antenna in antenna_range:
#                            valid_data = compress(
#                             logical_not(flag[-1][:,antenna]), data[:,antenna])
#                            if len(valid_data):
#                                median_data = median(valid_data)
#                                if last_phase != None:
#                                    phases = (median_data - last_phase) + \
#                                     phase_wraps
#                                    data[:,antenna] += phase_wraps[
#                                         argmin(abs(phases))]
#                                last_phase = median_data + phase_wraps[
#                                 argmin(abs(phases))]
#                            print antenna, last_phase

# lastly ensure that the phase display is centred in the -180 to +180 range

#                        phase_wraps = arange(-5,6) * 2.0 * pi
#                        valid_data = compress(logical_not(ravel(flag[-1])),
#                         ravel(data))
#                        if len(valid_data):
#                            mean_phase = mean(valid_data)
#                            phases = mean_phase + phase_wraps
#                            data += phase_wraps[argmin(abs(phases))]

# instead, ensure that the phases for each antenna are centred in the -180 to
# +180 range

                        phase_wraps = arange(-5,6) * 2.0 * pi
                        for antenna in antenna_range:
                            valid_data = compress(
                             logical_not(flag[-1][:,antenna]), data[:,antenna])
                            if len(valid_data):
                                mean_phase = mean(valid_data)
                                phases = mean_phase + phase_wraps
                                data[:,antenna] += phase_wraps[argmin(
                                 abs(phases))]

# put data into result structure

                    description = {}
                    description['DATA_DESC_ID'] = int(data_desc_id)
                    description['FIELD_ID'] = int(field_id)
                    description['POLARIZATION_ID'] = int(p)
                    description['TITLE'] = \
                     'Field:%s (%s) Spw:%s Pol:%s - gain calibration %s' % (
                     self._fieldName[int(field_id)],
                     self._fieldType[int(field_id)], self._pad(data_desc_id),
                     p, requiredDataType)

                    result = {}
                    result['dataType'] = 'gain calibration %s' % (
                     requiredDataType)
                    result['xtitle'] = 'ANTENNA'
                    result['x'] = arange(max(antenna_range)+1)
                    result['ytitle'] = 'TIME'
                    result['y'] = times
                    if self._timesol != 'int':
                        result['y_interval'] = zeros([len(times)], float) + abs(
                         self._timesol)
                    else:
                        result['y_interval'] = zeros([len(times)], float) + 1.0
                    if requiredDataType == 'phase':
                        result['data'] = data * (180.0 / math.pi)
                        result['dataUnits'] = 'degrees'
                    else:
                        result['data'] = data
                        result['dataUnits'] = ''
                    result['mad_floor'] = zeros([len(times),
                     max(antenna_range)+1], float)
                    result['flag'] = flag
                    result['flagVersions'] = ['NoData', 'BeforeHeuristics',
                     'StageEntry', 'Current']
                    result['data_time'] = solution_time
                    result['chunks'] = chunks

                    pickled_description = pickle.dumps(description)
                    results[pickled_description] = result


    def calculate(self, input_flag_marks=None):
        """Calculate G from the MS using the calibrater tool. A different
        file is output for each spw.

        Keyword Arguments:
        input_flag_marks -- Value of flag marks to use when building 
                            a gain table for more than 1 source, e.g. for
                            GAIN and FLUX calibrators.
        """

#        print 'GainCalibration.calculate called: %s' % self._sourceTypes

        self._htmlLogger.timing_start('GainCalibration.calculate')
   
        data_desc_ids = self._results['summary']['data_desc_range']

# are the data already available

        inputs = self.inputs()
        if input_flag_marks != None:

# this happens in a flux calibration when FLUX and GAIN gains are output
# to the same file, whose flag marks reflect the state of both fields.

            inputs['flag_marks'] = input_flag_marks

        flag_marks = inputs['flag_marks']

        entryID,parameters = self._bookKeeper.available(
         objectType=inputs['objectType'], sourceType=inputs['sourceType'],
         furtherInput=inputs['furtherInput'], 
         dependencies=inputs['dependencies'], outputFiles=[])
       
        if entryID == None:
            parameters = {'history':self._fullStageName, 'solve':{}, 
             'data':{}, 'command':{}, 'dependencies':{}}

# make sure the bandpass calibration is available

            if self._bpCal != None:
                bpCalParameters = self._bpCal.calculate()
                parameters['dependencies']['bpCal'] = bpCalParameters
            else:
                parameters['dependencies']['bpCal'] = None

            field_ids = []
            field_names = self._results['summary']['field_names']

            parameters['solve']['setjy'] = {}
            self._imager.open(thems=self._msName, compress=False)

            for sourceType in self._sourceTypes:
                new_field_ids = self.getFieldsOfType(sourceType)
                field_ids += new_field_ids

# make sure MODEL_DATA columns are set correctly - FLUX takes precedence
# over GAIN

                if sourceType.count('FLUX') > 0:

# FLUX from catalogue, or hardwired for PdB sources

                    for field in new_field_ids:
                        if field_names[field] == 'MWC349':
                            self._table.open(self._msName + '/SPECTRAL_WINDOW')
                            ref_frequency = self._table.getcol(
                             columnname='REF_FREQUENCY')
                            self._table.close()

                            for spw in data_desc_ids:
                                flux = 0.95 * (ref_frequency[spw] / 87e9)**0.6

                                parameters['solve']['setjy'][(field,spw)] = \
                                 self._msCalibrater.setjy(field=field, spw=spw,
                                  fluxdensity=[flux, 0.0, 0.0, 0.0],
                                  standard='CLIC')

                        elif field_names[field] == '2200+420':
                            parameters['solve']['setjy'][(field,'all')] = \
                             self._msCalibrater.setjy(field=field, spw=-1,
                             fluxdensity=[3.0, 0.0, 0.0, 0.0],
                             standard='MYGUESS')

                        elif field_names[field] == 'CRL618':
                            parameters['solve']['setjy'][(field,'all')] = \
                             self._msCalibrater.setjy(field=field, spw=-1,
                             fluxdensity=[1.55, 0.0, 0.0, 0.0], standard='CLIC')

                        else:

# otherwise force the tool to read the data from the catalogue (VLA)

                            parameters['solve']['setjy'][(field,'all')] = \
                             self._msCalibrater.setjy(field=field, spw=-1,
                             fluxdensity=[-1.0, 0.0, 0.0, 0.0],
                             standard='Perley-Taylor99')

                elif sourceType.count('GAIN') > 0:

# GAIN fields to 1Jy
  
                    for field in new_field_ids:
                        parameters['solve']['setjy'][(field,'all')] = \
                         self._msCalibrater.setjy(field=field, spw=-1,
                         fluxdensity=[1.0, 0.0, 0.0, 0.0],
                         standard='Perley-Taylor99')
            self._imager.close()

# save the on-entry flag state if will be flagging edges

            if self._bandpassFlaggingStage != None:
                self._msFlagger.rememberFlagState()

# get the current flagging info 

                flagging,flaggingReason,flaggingApplied = \
                 self._msFlagger.getFlags()

# calculate G for each spw separately, enclosed in a try block so that
# an exception will not prevent the flagging state being restored

            try:
                for data_desc_id in data_desc_ids:
                    parameters['data'][data_desc_id] = {}
                    parameters['command'][data_desc_id] = []

# use an alias for brevity

                    command = parameters['command'][data_desc_id]

# if not continuum flag the bandpass edge channels to improve S/N
 
                    flag_channels = None
                    nchannels = self._results['summary']['nchannels']\
                     [data_desc_id]
                    if nchannels > 1 and self._bandpassFlaggingStage != None:
                        for row,val in enumerate(flaggingReason):
                            if val['stageDescription']['name'] == \
                             self._bandpassFlaggingStage:
                                for flagList in flagging[row].values():
                                    for flag in flagList:
                                        if data_desc_id == flag['DATA_DESC_ID']:
                                            flag_channels = flag['CHANNELS']
                                            break
                                break

                        self._msFlagger.apply_bandpass_flags(data_desc_id,
                         field_ids, flag_channels)
                        command.append('...apply channel flags')

# calculate the gains

                    gtab = '%s.gtab.spw%s.fm%s' % (self._base_msName,
                     data_desc_id, flag_marks)
                    if input_flag_marks != None:
                        append = True
                    else:
                        append = False

                    try:
                        if self._bpCal != None:
                            self._bpCal.setapply(spw=data_desc_id,
                             field=field_ids)
                        self._msCalibrater.solve(field=field_ids,
                         spw=data_desc_id, type='G', t=self._timesol,
                         combine='scan', table=gtab, append=append, apmode='AP',
                         solnorm=False, minsnr = 0.0, commands=command)

                    except KeyboardInterrupt:
                        raise
                    except:
                        parameters['data'][data_desc_id]['error'] = \
                         'gain calibration failed' 
                        print 'exception', sys.exc_info()

                    parameters['data'][data_desc_id]['table'] = gtab 
                    parameters['data'][data_desc_id]['solve'] = {} 
                    parameters['data'][data_desc_id]['solve']['type'] = 'G' 
                    parameters['data'][data_desc_id]['solve']['field_id'] = \
                     field_ids 
                    parameters['data'][data_desc_id]['solve']['t'] = \
                     self._timesol 
                    parameters['data'][data_desc_id]['solve']['apmode'] = 'AP'
                    parameters['data'][data_desc_id]['type'] = 'G'
                    parameters['data'][data_desc_id]['bandpassFlaggingStage']\
                     = self._bandpassFlaggingStage
                    if self._bpCal != None and \
                     bpCalParameters['data'].has_key(data_desc_id):
                        if bpCalParameters['data'][data_desc_id].has_key(
                         'table'):
                            parameters['data'][data_desc_id]['solve']['btab']\
                             = bpCalParameters['data'][data_desc_id]['table']
                    else:
                            parameters['data'][data_desc_id]['solve']['btab']\
                             = None

            finally:

# restore the on-entry flagging state

                if self._bandpassFlaggingStage != None:
                    self._msFlagger.recallFlagState()

# store the object info in the BookKeeper

            self._bookKeeper.enter(
             objectType=inputs['objectType'], sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             dependencies=inputs['dependencies'], outputFiles=[],
             outputParameters=parameters)

        self._parameters = parameters
        self._htmlLogger.timing_stop('GainCalibration.calculate')

        return parameters


    def description(self):
        description = ['GainCalibration - calculate the gain solutions']
        return description


    def getData(self):
        """Public method to return the G calibration as a 'view' of the data.
        """

#        print 'GainCalibration.getData called: %s ' % (self._dataType)
        self._htmlLogger.timing_start('GainCalibration.getdata')

# calculate the G gains for 'BeforeHeuristics', 'StageEntry' and 'Current' - 
# 'BeforeHeuristics' assumes no bandpassFlaggingStage

        self._msFlagger.setFlagState('BeforeHeuristics')
        bandpassFlaggingStage = self._bandpassFlaggingStage
        self._bandpassFlaggingStage = None
        originalGains = self.calculate()
        self._msFlagger.setFlagState('StageEntry')
        self._bandpassFlaggingStage = bandpassFlaggingStage
        stageEntryGains = self.calculate()
        self._msFlagger.setFlagState('Current')
        self._parameters = gains = self.calculate()

# now read the results from the tables

        results = {}
        for k in originalGains['data'].keys():
            self._read_results(gains['data'][k]['table'],
             stageEntryGains['data'][k]['table'],
             originalGains['data'][k]['table'], self._dataType, results)

        self._level_results(results)
        
# now add the latest results to the returned structure

        flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()
        self._results['flagging'].append(flagging)
        self._results['flaggingReason'].append(flaggingReason)
        self._results['flaggingApplied'].append(flaggingApplied)

# copy history an dependency info

        self._results['parameters'] = self._parameters
        self._results['parameters']['dependencies']['originalGain'] = \
         originalGains
        self._results['parameters']['dependencies']['stageEntryGain'] = \
         stageEntryGains

        for k in results.keys():
            if self._results['data'].has_key(k):
                self._results['data'][k].append(results[k])
            else:
                self._results['data'][k] = [results[k]]

        temp = python_copy.deepcopy(self._results)

        self._htmlLogger.timing_stop('GainCalibration.getdata')
        return temp


    def inputs (self):
        """
        """
        data_desc_ids = self._results['summary']['data_desc_range']

        result = {}
        result['objectType'] = 'GainCalibration'
        result['sourceType'] = self._sourceTypes
        result['furtherInput'] = {'timesol':self._timesol}
        result['furtherInput'] = {'bandpassFlaggingStage':
         self._bandpassFlaggingStage}

        flag_mark = {}
        ignore,flag_mark_col = self._msFlagger.getFlagMarkInfo()
        field_ids = []
        for sourceType in self._sourceTypes:
            field_ids += self.getFieldsOfType(sourceType)
        for field_id in field_ids:
            flag_mark[field_id] = flag_mark_col[field_id]
        flag_mark = str(flag_mark)

# replace unusual symbols to avoid problems with TaQL

        flag_mark = flag_mark.replace(' ', '')
        flag_mark = flag_mark.replace('{', '')
        flag_mark = flag_mark.replace('}', '')
        flag_mark = flag_mark.replace(':', '-')
        flag_mark = flag_mark.replace(',', '.')

        result['flag_marks'] = flag_mark
        result['outputFiles'] = []
        if self._bpCal == None:
            result['dependencies'] = []
        else:
            result['dependencies'] = [self._bpCal.inputs()]

        return result


    def setapply(self, spw):
        """Method to schedule the current calibration to be applied to the
        specified spw.
        """
        self.msCalibrater.setapply(type='G', 
         table=self._parameters['data'][spw]['table'])


    def createGeneralHTMLDescription(self, stageName):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = {}

        if self._bpCal == None:
            description['bandpass calibration'] = \
             'There was no bandpass calibration'
        else:
            description['bandpass calibration'] = \
             self._bpCal.createGeneralHTMLDescription(stageName)\
             ['bandpass calibration']

        gainDescription = ''

        description['gain calibration'] = gainDescription
        return description


    def createDetailedHTMLDescription(self, stageName, parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        parameters -- The dictionary that holds the descriptive information.
        """

        if parameters == None:
            parameters = self._parameters

        description = {}

        if self._bpCal == None:
            description['bandpass calibration'] = \
             'There was no bandpass calibration'
        else:
            description['bandpass calibration'] = \
             self._bpCal.createDetailedHTMLDescription(stageName, parameters=
             parameters['dependencies']['bpCal'])['bandpass calibration']

        gainDescription = """
         The gain was calculated as follows:
        <ul>     
         <li>The 'imager' tool was used to set the flux values of the 
          calibrator(s) according to their type, as described in the 
          following table.

          <table CELLPADDING="5" BORDER="1"'
           <tr>
            <th>Field</th>
            <th>SpW</th>
            <th>Casapy call</th>
           </tr>"""

        for k,entry in parameters['solve']['setjy'].iteritems():
            gainDescription += '<tr>'
            gainDescription += '<td>%s (%s)</td>' % (k[0],self._fieldType[k[0]])
            gainDescription += '<td>%s</td>' % k[1]
            gainDescription += '<td>%s</td>' % entry
            gainDescription += '</tr>'

        gainDescription += """</table>
         <li>The gains were calculated.
          <table CELLPADDING="5" BORDER="1"
           <tr>
            <th>Spw</th>
            <th>Field</th>
            <th>t</th>
            <th>apmode</th>
            <th>BP table</th>
            <th>channel flags</th>
            <th>Apply Table</th>
            <th>Apply Type</th>
            <th>Casapy calls</th>
            <th>error?</th>
           </tr>"""

        for i,entry in parameters['data'].iteritems():
            gainDescription += "<tr><td>%s</td>" % i
            gainDescription += '<td>%s</td>' % entry['solve']['field_id']
            gainDescription += '<td>%s</td>' % entry['solve']['t']
            gainDescription += '<td>%s</td>' % entry['solve']['apmode']
            if entry['solve'].has_key('btab'):
                gainDescription += '<td>%s</td>' % entry['solve']['btab']
            else:
                gainDescription += '<td>None</td>'
            gainDescription += '<td>%s</td>' % entry['bandpassFlaggingStage']
            gainDescription += "<td>%s</td>" % entry['table']
            gainDescription += "<td>%s</td>" % entry['type']

# casapy calls, in a separate node to save space.

            gainDescription += '<td>'
            gainDescription += self._htmlLogger.openNode('casapy calls',
             '%s.spw%s.%s' % (stageName, i, 'gainCal_casapy_calls'), True,
             stringOutput=True)
            for line in parameters['command'][i]:
                self._htmlLogger.logHTML('<br>%s' % line)
            self._htmlLogger.closeNode()
            gainDescription += '</td>'

            if entry.has_key('error'):
                gainDescription += '<td>%s</td>' % entry['error']
            else:
                gainDescription += '<td></td>'
            gainDescription += '</tr>'

        gainDescription += """
          </table>
         where:
          <ul>
           <li>'t' is the time resolution of the gain calibration.
           <li>'apmode' specifies whether amplitude (A), phase (P) or both
               (AP) are to be calculated.
           <li>'BP Table' is the name of the file containing the bandpass
               correction applied to the data before the gain calculation.
           <li>'channel flags' gives the name of the stage that specified
               the channels to be flagged before solving, if any.
           <li>'Apply Table' is the name of the file containing the gain
                solution.
           <li>'Apply Type' gives the casapy type of the gain solution.
           <li>'Casapy calls' links to alist of the casapy calls used to
               calculate the gain solution.
           <li>'error?' describes any occur that occurred.
          </ul>
        </ul>     
        <p>The gain was calculated by Python class GainCalibration."""

        description['gain calibration'] = gainDescription
        return description


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = self.createGeneralHTMLDescription(stageName)

        self._htmlLogger.logHTML("""The gain calibration was calculated as
             follows:
             <ul>""")

        self._htmlLogger.logHTML("""
              <li>""" + description['bandpass calibration'] + """
              <li>""" + description['gain calibration'] + """
             </ul>""")


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a detailed description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        """

        description = self.createDetailedHTMLDescription(stageName, 
         parameters=parameters)

        if topLevel:
            self._htmlLogger.logHTML("""
             <h3>Data View</h3>
                 <p>The data view is a gain calibration.""")

        self._htmlLogger.logHTML("""The gain calibration was calculated as
             follows:
             <ul>""")

        self._htmlLogger.logHTML("""
              <li>""" + description['bandpass calibration'] + """
              <li>""" + description['gain calibration'] + """
             </ul>""")


class GainCalibrationAmplitudePerTimestamp(GainCalibration):
    """Class to get G calibration amplitudes for each timestamp of the data.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, bandpassCal, sourceType='GAIN', timesol=None,
     bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- Name of stage using this object.
        bandpassCal         -- class to be used for the bandpass calibration.
        sourceType          -- Type of source to be used for the G calibration.
        bandpassFlaggingStage -- Name of stage where bandpass edge channels
                                 were detected. If not None then these
                                 channels will be flagged in the calibrator
                                 data before the gain calibrations are
                                 calculated. The initial flag state of the
                                 MS will be restored afterward.
        """
        GainCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, bandpassCal, [sourceType],
         timesol='int', dataType='amplitude', bandpassFlaggingStage=
         bandpassFlaggingStage)


    def description(self):
        description = [
         'GainCalibrationAmplitudePerTimestamp - calculate the gain amplitude for each timestamp']
        return description


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        self._htmlLogger.logHTML('''
         <p>The view is a 2-d array with axes ANTENNA and TIME. Each
         pixel shows the gain calibration amplitude for that antenna and 
         timestamp, calculated assuming that the target is a point source.''')


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a detailed description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        topLevel   -- True if this data 'view' is to be displayed directly,
                      not passing through a data modifier object.
        parameters -- Dictionary that holds the descriptive information.
         """
        if topLevel:
            self._htmlLogger.logHTML('<h3>Data View</h3>')

        self._htmlLogger.logHTML('''
         <p>The view is a 2-d array with axes ANTENNA and TIME. Each
         pixel shows the gain calibration amplitude for that antenna and 
         timestamp, assuming that the target is a point source.''')

        GainCalibration.writeDetailedHTMLDescription(self, stageName, False,
         parameters=parameters)


class GainCalibrationPhase(GainCalibration):
    """Class to get G calibration phases.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, bandpassCal, sourceType='*GAIN*', timesol=300.0,
     bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- Name of stage using this object.
        bandpassCal         -- class to be used for the bandpass calibration.
        sourceType          -- Type of source to be used for the G calibration.
        bandpassFlaggingStage -- Name of stage where bandpass edge channels
                                 were detected. If not None then these
                                 channels will be flagged in the calibrator
                                 data before the gain calibrations are
                                 calculated. The initial flag state of the
                                 MS will be restored afterward.
        """
        GainCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, bandpassCal, [sourceType],
         timesol, dataType='phase', bandpassFlaggingStage=bandpassFlaggingStage)


    def description(self):
        description = [
         'GainCalibrationPhase - calculate the gain phase']
        return description


    def writeHTMLDescription(self, stageName, topLevel, parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        """
        if topLevel:
            self._htmlLogger.logHTML('<h3>Data View</h3>')

        self._htmlLogger.logHTML('''
         <p>The view is a 2-d array with axes ANTENNA and TIME. Each
         pixel shows the gain calibration phase for that antenna and 
         timestamp.''')

        GainCalibration.writeHTMLDescription(self, stageName, False)



class GainCalibrationPhasePerTimestamp(GainCalibration):
    """Class to get G calibration phases for each timestamp of the data.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, bandpassCal, sourceType='GAIN', timesol=300.0,
     bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- Name of stage using this object.
        bandpassCal         -- class to be used for the bandpass calibration.
        sourceType          -- Type of source to be used for the G calibration.
        bandpassFlaggingStage -- Name of stage where bandpass edge channels
                                 were detected. If not None then these
                                 channels will be flagged in the calibrator
                                 data before the gain calibrations are
                                 calculated. The initial flag state of the
                                 MS will be restored afterward.
        """
        GainCalibration.__init__(self, tools, bookKeeper, msCalibrater, 
         msFlagger, htmlLogger, msName, stageName, bandpassCal, [sourceType],
         timesol='int', dataType='phase', bandpassFlaggingStage=
         bandpassFlaggingStage)


    def description(self):
        description = [
         'GainCalibrationPhasePerTimestamp - calculate the gain phase for each timestamp']
        return description


    def writeGeneralHTMLDescription(self, stageName):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        self._htmlLogger.logHTML('''
         <p>The view is a 2-d array with axes ANTENNA and TIME. Each
         pixel shows the gain calibration phase for that antenna and 
         timestamp, calculated assuming that the target is a point source.''')


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        topLevel   -- True if this data 'view' is to be displayed directly,
                      not passing through a data modifier object.
        parameters -- Dictionary that holds the descriptive information.
        """
        if topLevel:
            self._htmlLogger.logHTML('<h3>Data View</h3>')

        self._htmlLogger.logHTML('''
         <p>The view is a 2-d array with axes ANTENNA and TIME. Each
         pixel shows the gain calibration phase for that antenna and 
         timestamp, assuming that the target is a point source.''')

        GainCalibration.writeDetailedHTMLDescription(self, stageName, False,
         parameters=parameters)


class GainCalibrationSNR(GainCalibration):
    """Class to get G calibration signal to noise ratio.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, bandpassCal, sourceType='GAIN', timesol=300.0,
     bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- Name of stage using this object.
        bandpassCal         -- class to be used for the bandpass calibration.
        sourceType          -- Type of source to be used for the G calibration.
        bandpassFlaggingStage -- Name of stage where bandpass edge channels
                                 were detected. If not None then these
                                 channels will be flagged in the calibrator
                                 data before the gain calibrations are
                                 calculated. The initial flag state of the
                                 MS will be restored afterward.
        """
        GainCalibration.__init__(self, tools, bookKeeper, msCalibrater, 
         msFlagger, htmlLogger, msName, stageName, bandpassCal, [sourceType],
         timesol, dataType='SNR', bandpassFlaggingStage=bandpassFlaggingStage)


    def description(self):
        description = [
         'GainCalibrationSNR - calculate the gain signal to noise ratio']
        return description


    def writeGeneralHTMLDescription(self, stageName):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        self._htmlLogger.logHTML('''
         <p>The view is a 2-d array with axes ANTENNA and TIME. Each
         pixel shows the signal to noise ratio of the the gain calibration
         solution for that antenna.''')


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        topLevel   -- True if this data 'view' is to be displayed directly,
                      not passing through a data modifier object.
        parameters -- Dictionary that holds the descriptive information.
        """
        if topLevel:
            self._htmlLogger.logHTML('<h3>Data View</h3>')

        self._htmlLogger.logHTML('''
         <p>The view is a 2-d array with axes ANTENNA and TIME. Each
         pixel shows the signal to noise ratio of the the gain calibration
         solution for that antenna.''')

        GainCalibration.writeDetailedHTMLDescription(self, stageName, False,
         parameters=parameters)

