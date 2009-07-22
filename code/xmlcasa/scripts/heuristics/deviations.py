# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Pad spw and antenna to ensure correct order in html.
# 30-Aug-2007 jfl Flux calibrating version.
#  6-Nov-2007 jfl Best bandpass release.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import copy as python_copy
from numpy import *
import pickle

# alma modules

from baseTools import *
from baseData import *
from bandpassCalibration import *
from gainCalibration import *

class DataDeviationPerBaseline(BaseData):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, dataType, ampOrPhase, bandpassCal=None,
     gainCal=None):
        """Constructor.
  
        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- Name of stage using this object.
        sourceType  -- type of source whose data are to be used.
        dataType    -- 'raw' or 'corrected' - the calibration state of the data
                       for which the deviations are calculated.
        ampOrPhase  -- 'amp' or 'phase' - the aspect of the data for which the
                       deviations are to be calculated.
        bandpassCal -- Class to be used to calculate bandpass calibration,
                       if any.
        gainCal     -- Class to be used to calculate gain calibration, if any.
        """
#        print 'DataDeviationPerBaseline constructor called: %s %s %s %s' % (
#         msName, sourceType, dataType, ampOrPhase)
        BaseData.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName)
        self._className = 'DataDeviationPerBaseline'
        self._sourceType = sourceType
        self._dataType = dataType
        self._ampOrPhase = ampOrPhase
        self._target_field_ids = self.getFieldsOfType(self._sourceType)
        if bandpassCal != None:
            if len(bandpassCal) == 1:
                self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater,
                 msFlagger, htmlLogger, msName, stageName)
            else:
                self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater,
                 msFlagger, htmlLogger, msName, stageName,
                 viewClassList=bandpassCal[1:])
        if gainCal != None:
            self._gainCal = gainCal(tools, bookKeeper, msCalibrater, 
             msFlagger, htmlLogger, msName, stageName, bandpassCal)
        self._getDataCalls = 0
        self._first = True
        self._next_target_field_spw = []


    def __del__(self):
        """Destructor
        """

# delete the flag state holding the PreviousIteration flags.

        self._msFlagger.deleteFlagState('PreviousIteration')


    def description (self):
        description = [
         'DataDeviationPerBaseline - deviations from chunk median']
        return description


    def getData(self, topLevel=False):
        """
        """

#        print 'DataDeviationPerBaseline.getData called'
        self._getDataCalls += 1
        self._htmlLogger.timing_start('DataDeviationPerBaseline.getData_%s' %
         self._getDataCalls)

# have the data been calculated already?
    
        dependencies = []
        if self._dataType=='corrected':
            dependencies = [self._bpCal.inputs(), self._gainCal.inputs()]

        entryID,results = self._bookKeeper.available(
         objectType='DataDeviationPerBaseline', sourceType=self._sourceType,
         furtherInput={'dataType':self._dataType,
                       'ampOrPhase':self._ampOrPhase},
         outputFiles=[],
         dependencies=dependencies)

        if entryID == None:

            results = {
             'parameters':{'history':{}, 'dependencies':{}},
             'data':{}}
            results['parameters']['history'] = self._fullStageName

            antenna_range = self._results['summary']['antenna_range']

            if self._dataType=='corrected':

# get calibration data if required
        
                bpCalParameters = self._bpCal.calculate()
                gainCalParameters = self._gainCal.calculate()

                results['parameters']['dependencies']['bpCal'] = bpCalParameters
                results['parameters']['dependencies']['gainCal'] = \
                 gainCalParameters

# loop through data_desc_id and field_id

            if self._first:
               field_spw = self._valid_field_spw
               target_field_spw = []
               for key in self._valid_field_spw:
                   field = key[0]
                   if self._target_field_ids.count(field) > 0:
                       target_field_spw.append(key)
            else:
               target_field_spw = list(self._next_target_field_spw)

            self._next_target_field_spw = []

            self._ms.open(self._msName)
            for key in target_field_spw:
                field_id = key[0]
                data_desc_id = key[1]

                results['parameters'][(field_id,data_desc_id)] = {}

# set the flag states we want to look at 

                flagVersions = ['BeforeHeuristics', 'StageEntry', 'Current']
                if not self._first:
                    flagVersions.append('PreviousIteration')

                if self._dataType=='corrected':
                    try:

# apply the calibration, this may fail

                        self._bpCal.setapply(spw=data_desc_id, field=field_id)
                        self._gainCal.setapply(spw=data_desc_id, field=field_id)
                        commands += self._msCalibrater.correct(spw=data_desc_id,
                         field=field_id)

                    except KeyboardInterrupt:
                        raise
                    except:
                        results['parameters'][(field_id,data_desc_id)]\
                         ['error'] = 'failed to calibrate data'
                        continue

# get the current data

                corr_axis, chan_freq, times, chunks, antenna1, antenna2, \
                 ifr_real, ifr_imag, ifr_flag, ifr_flag_row = \
                 self._getBaselineData(field_id, data_desc_id, 
                 antenna_range, self._dataType, flagVersions)

# declare arrays

                ncorr = shape(corr_axis)[0]
                data_dev = zeros([max(antenna_range)+1, ncorr, 
                 max(antenna_range)+1, len(times)], float)
                data_dev_flag = []
                for fi,fs in enumerate(flagVersions):
                    data_dev_flag.append(ones([max(antenna_range)+1, 
                     ncorr, max(antenna_range)+1, len(times)], int))
                no_data_flag = ones([max(antenna_range)+1, 
                 ncorr, max(antenna_range)+1, len(times)], int)

                mad_floor = zeros([max(antenna_range)+1, ncorr,
                 max(antenna_range)+1, len(times)], float)
                new_values = zeros([max(antenna_range)+1, ncorr,
                 max(antenna_range)+1, len(times)], bool)

# if this is not the first iteration, have any flags changed since the last

                calculate = True
                if not self._first:
                    previous = flagVersions.index('PreviousIteration')
                    current = flagVersions.index('Current')

                    if all(ifr_flag_row[previous] == ifr_flag_row[current]) and \
                     all(ifr_flag[previous] == ifr_flag[current]):
                        calculate = False                                   

                if calculate:
                    self._next_target_field_spw.append([field_id, data_desc_id])

                    chunk_median_data = zeros([max(antenna_range)+1, ncorr, max(antenna_range)+1, len(chunks)], float)
                    chunk_radial_mad = zeros([max(antenna_range)+1, ncorr, max(antenna_range)+1, len(chunks)], float)
                    chunk_receiver_radial_mad = zeros([max(antenna_range)+1, ncorr, max(antenna_range)+1, len(chunks)], float)

# loop through antennas and for each build up 'baseline v time'
# deviation image - deviation from the chunk median position

                    for antenna in antenna_range:
                        ifr_range = arange(len(antenna1))
                        ifr_range = compress(antenna1==antenna, ifr_range)

                        for corr in range(ncorr):
                            for ifr in ifr_range:
                                other_ant = antenna2[ifr]

                                median_real = zeros([len(times)], float)
                                median_imag = zeros([len(times)], float)
                                median_flag = ones([len(times)], int)
  
                                for chunk_id, chunk in enumerate(chunks):
                                    if not self._first:

# first time round, need to calculate all data. Otherwise,
# have any flags in this chunk changed since the PreviousIteration?

                                        previous = flagVersions.index('PreviousIteration')
                                        current = flagVersions.index('Current')

                                        if all(ifr_flag_row[previous][ifr,chunk] == ifr_flag_row[current][ifr,chunk]) and \
                                         all(ifr_flag[previous][corr,:,ifr,chunk] == ifr_flag[current][corr,:,ifr,chunk]):
                                            continue                                   

# accumulate real,imaginary data for each chunk

                                    chunk_reals = []
                                    chunk_imags = []
                                    chunk_receiver_reals = []
                                    chunk_receiver_imags = []
     
                                    for t in chunk:
                                        new_values[antenna,corr,other_ant,t] = new_values[other_ant,corr,antenna,t] = True
                                        median_flag[t] = True
  
# set flag arrays for each flagversion, abandon calculation
# if there are fewer than 2 time samples in the chunk

                                        no_data_flag[antenna,corr,other_ant,t] = no_data_flag[other_ant,corr,antenna,t] = False
                                        if len(chunk) < 2:
                                            continue

                                        for fi,fs in enumerate(flagVersions):
                                            if ifr_flag_row[fi][ifr,t]:
                                                continue
                                            if all(ifr_flag[fi][corr,:,ifr,t]):
                                                continue
                                            else:
                                                data_dev_flag[fi][antenna,corr,other_ant,t] = data_dev_flag[fi][other_ant,corr,antenna,t] = False

# if 'Current' flag is set then need do no more

                                        current = flagVersions.index('Current')
                                        if data_dev_flag[current][antenna,corr,other_ant,t]:
                                            continue
  
# only get this far if 'Current' data of interest are not flagged.

                                        reals = compress(logical_not(ifr_flag[current][corr,:,ifr,t]), ifr_real[corr,:,ifr,t])
  
                                        if len(reals) > 0:
                                            median_flag[t] = False
                                            median_real[t] = median(reals)
                                            imags = compress(logical_not(ifr_flag[current][corr,:,ifr,t]), ifr_imag[corr,:,ifr,t])
                                            median_imag[t] = median(imags)
                                            chunk_reals += list(reals)
                                            chunk_imags += list(imags)
                                            receiver_reals = reals - median_real[t]
                                            receiver_imags = imags - median_imag[t]
                                            chunk_receiver_reals += list(receiver_reals)
                                            chunk_receiver_imags += list(receiver_imags)

# are there 2 _valid_ samples in the chunk, if not abandon 
# calculation for this chunk

                                    valid = nonzero(logical_not(median_flag[chunk]))
                                    if len(median_flag[valid]) < 2:
                                        current = flagVersions.index('Current')
                                        data_dev_flag[current][antenna,corr,other_ant,t] = data_dev_flag[current][other_ant,corr,antenna,t] = True
                                        continue

# get median position for the chunk

                                    chunk_reals = array(chunk_reals)
                                    chunk_imags = array(chunk_imags)
                                    if len(chunk_reals) == 0:
                                        continue

                                    median_chunk_real = median(chunk_reals)
                                    median_chunk_imag = median(chunk_imags)
                                    if self._ampOrPhase == 'amp':
                                        chunk_median_data[antenna,corr,other_ant,chunk_id] = \
                                        chunk_median_data[other_ant,corr,antenna,chunk_id] = \
                                         sqrt(pow(median_chunk_real,2) + pow(median_chunk_imag,2))
                                    elif self._ampOrPhase == 'phase':
                                        chunk_median_data[antenna,corr,other_ant,chunk_id] = \
                                        chunk_median_data[other_ant,corr,antenna,chunk_id] = \
                                         arctan2(median_chunk_imag, median_chunk_real)
                                    else:
                                        raise NameError, 'bad ampOrPhase: %s' % self._ampOrPhase

# and a measure of the scatter of points about the 'receiver' position,
# with sky variation removed. This is the noise 'floor' of the measurement.

                                    chunk_receiver_reals = array(chunk_receiver_reals)
                                    chunk_receiver_imags = array(chunk_receiver_imags)
                                    chunk_receiver_radii = sqrt(pow(chunk_receiver_reals,2) + pow(chunk_receiver_imags,2))
                                    chunk_receiver_radial_mad[antenna,corr,other_ant,chunk_id] = \
                                    chunk_receiver_radial_mad[other_ant,corr,antenna,chunk_id] = \
                                     median(chunk_receiver_radii)

# derive the deviation in amplitude/phase at each timestamp from
# the chunk median position

                                    for t in chunk:
                                        if median_flag[t]:
                                            continue

                                        if self._ampOrPhase == 'amp':
                                            data_dev[antenna,corr,other_ant,t] = data_dev[other_ant,corr,antenna,t] = \
                                             sqrt(pow(median_real[t],2) + pow(median_imag[t],2)) - \
                                             sqrt(pow(median_chunk_real,2) + pow(median_chunk_imag,2))

# estimate intrinsic noise of each measurement from the variation across channels. 

                                            nchannels = len(compress(logical_not(ifr_flag[-1][corr,:,ifr,t]), ifr_real[corr,:,ifr,t]))
                                            mad_floor[antenna,corr,other_ant,t] = mad_floor[other_ant,corr,antenna,t] = \
                                             chunk_receiver_radial_mad[antenna,corr,other_ant,chunk_id] / sqrt(float(nchannels))

                                        elif self._ampOrPhase == 'phase':
                                            rotated_slot_real = median_real[t] * cos(chunk_median_data[antenna,corr,other_ant,chunk_id]) + \
                                             median_imag[t] * sin(chunk_median_data[antenna,corr,other_ant,chunk_id])
                                            rotated_slot_imag = -median_real[t] * sin(chunk_median_data[antenna,corr,other_ant,chunk_id]) + \
                                             median_imag[t] * cos(chunk_median_data[antenna,corr,other_ant,chunk_id])

# take care over sign of phase w.r.t. ordering of antennas on baseline

                                            data_dev[antenna,corr,other_ant,t] = arctan2(rotated_slot_imag, rotated_slot_real) * 180.0 / math.pi
                                            data_dev[other_ant,corr,antenna,t] = -data_dev[antenna,corr,other_ant,t]

# estimate intrinsic noise of each measurement from the variation across channels.

                                            nchannels = len(compress(logical_not(ifr_flag[-1][corr,:,ifr,t]), ifr_real[corr,:,ifr,t]))
                                            amp = sqrt(pow(median_real[t],2) + pow(median_imag[t],2))
                                            mad_floor[antenna,corr,other_ant,t] = mad_floor[other_ant,corr,antenna,t] = \
                                             (chunk_receiver_radial_mad[antenna,corr,other_ant,chunk_id] / sqrt(float(nchannels))) / amp

                                        else:
                                            raise NameError, 'bad ampOrPhase: %s' % self._ampOrPhase

                if self._ampOrPhase == 'phase':
                    dataUnits = 'degrees'
                else:
                    if self._dataType=='corrected':
                        dataUnits = 'calibrated'
                    else:
                        dataUnits = 'raw'

                for antenna in range(max(antenna_range)+1):
                    for corr in range(ncorr):
                        description = {}
                        description['FIELD_ID'] = int(field_id)
                        description['FIELD_NAME'] = self._fieldName[
                         int(field_id)]
                        description['FIELD_TYPE'] = self._fieldType[
                         int(field_id)]
                        description['DATA_DESC_ID'] = int(data_desc_id)
                        description['POLARIZATION_ID'] = corr_axis[corr]
                        description['ANTENNA1'] = [int(antenna)]
                        description['TITLE'] = \
                         'Field:%s (%s) Spw:%s Ant1:%s Pol:%s %s %s deviation' \
                         % (self._fieldName[field_id],
                         self._fieldType[field_id], self._pad(data_desc_id), 
                         self._pad(antenna), corr_axis[corr],
                         self._dataType, self._ampOrPhase)

                        result = {}
                        result['dataType'] = '%s %s deviation' % (
                         self._dataType, self._ampOrPhase)
                        result['xtitle'] = 'ANTENNA2'
                        result['x'] = arange(max(antenna_range)+1) 
                        result['ytitle'] = 'TIME'
                        result['y'] = times
                        result['chunks'] = chunks
                        result['data'] = transpose(data_dev[antenna,corr])
                        result['mad_floor'] =  transpose(mad_floor[antenna,corr])
                        result['new_values'] = transpose(new_values[antenna,corr])
                        result['dataUnits'] = dataUnits

                        flagsToStore = ['BeforeHeuristics', 'StageEntry',
                         'Current']
                        temp_flag = []
                        flagVersionsStored = []
                        for f in flagsToStore:
                            if flagVersions.count(f) == 0:
                                continue
                            fi = flagVersions.index(f)
                            temp_flag.append(transpose(data_dev_flag[fi][antenna,corr]))
                            flagVersionsStored.append(f)
                        temp_flag.append(transpose(no_data_flag[antenna,corr]))
                        flagVersionsStored.append('NoData')
                        result['flag'] = temp_flag
                        result['flagVersions'] = flagVersionsStored

                        pickled_description = pickle.dumps(description)
                        results['data'][pickled_description] = result

            self._ms.close()
            self._msFlagger.saveFlagState('PreviousIteration')
            self._first = False

# store the object info in the BookKeeper

            self._bookKeeper.enter(
             objectType='DataDeviationPerBaseline',
             sourceType=self._sourceType,
             furtherInput={'dataType':self._dataType,
                           'ampOrPhase':self._ampOrPhase},
             outputFiles=[],
             outputParameters=results,
             dependencies=dependencies)

# now add the latest results to the returned structure

        flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()
        self._results['flagging'].append(flagging)        
        self._results['flaggingReason'].append(flaggingReason)
        self._results['flaggingApplied'].append(flaggingApplied)

        for k in results['data'].keys():
            if self._results['data'].has_key(k):

# just add the new values to a copy of the previous result

                baseResult = python_copy.deepcopy(self._results['data'][k][-1])
                new_values = results['data'][k]['new_values']

# data

                baseResult['data'][new_values] = results['data'][k]['data']\
                 [new_values]
                baseResult['mad_floor'][new_values] = results['data'][k]\
                 ['mad_floor'][new_values]

# flags

                for fi in arange(len(self._results['data'][k][-1]['flagVersions'])):
                    baseResult['flag'][fi][new_values] = results['data'][k]['flag'][fi]\
                     [new_values]
 
                self._results['data'][k].append(baseResult)
            else:
                self._results['data'][k] = [results['data'][k]]

# copy history and dependency info

        self._results['parameters'] = results['parameters']

# return a copy of the data list, otherwise operating on it outside this class
# will corrupt it

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('DataDeviationPerBaseline.getData_%s' %
         self._getDataCalls)

        return temp


    def inputs(self):
        """Method to return the input settings for this object.
        """
        dependencies = []
        if self._dataType=='corrected':
            dependencies = [self._bpCal.inputs(), self._gainCal.inputs()]

        result = {}
        result['objectType'] = 'DataDeviationPerBaseline'
        result['sourceType'] = self._sourceType
        result['furtherInput'] = {'dataType':self._dataType,
                                  'ampOrPhase':self._ampOrPhase}
        result['outputFiles'] = []
        result['dependencies'] = dependencies

        flag_marks = {}
        ignore,flag_mark_col = self._msFlagger.getFlagMarkInfo()
        for field_id in self._target_field_ids:
            flag_marks[field_id] = flag_mark_col[field_id]
        result['flag_marks'] = flag_marks
        
        return result


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        if self._description != None:
            self._htmlLogger.logHTML('<p> %s' % self._description)

        self._htmlLogger.logHTML("""
          <p>The data view is a list of 2-d arrays, one for each value
          of ANTENNA1 in the measurement set. Each array has axes 
          x:ANTENNA2 and y:TIME.""")

        if self._ampOrPhase == 'amp':
            self._htmlLogger.logHTML("""
             Each pixel is the difference between the complex amplitude
             of the 'datum' for that baseline/timestamp and that
             of the the %s for the time %s to which 
             it belongs (all data first averaged over spectral channels if there
             is more than one).""" % (
              self._htmlLogger.glossaryLink('complex median'),
              self._htmlLogger.glossaryLink('chunk')))

        elif self._ampOrPhase == 'phase':
            self._htmlLogger.logHTML("""
             Each pixel is the phase of the 'datum' for that 
             baseline/timestamp relative to that of the the %s
             for the time %s to which 
             it belongs (all data first averaged over spectral channels if there
             is more than one).""" % (
              self._htmlLogger.glossaryLink('complex median'),
              self._htmlLogger.glossaryLink('chunk')))

        if self._dataType == 'raw':
            self._htmlLogger.logHTML("""
             <p>The data were not calibrated.""")

        elif self._dataType == 'corrected':
            description = {}

            description['bandpass calibration'] = \
             self._bpCal.createGeneralHTMLDescription(stageName, parameters=
             parameters['dependencies']['bpCal'])['bandpass calibration']

            description['gain calibration'] = \
             self._gainCal.createGeneralHTMLDescription(stageName, parameters=
             parameters['dependencies']['gainCal'])['gain calibration']

            self._htmlLogger.logHTML("""
             <p>The data were calibrated as follows:
             <ul>
              <li>""" + description['bandpass calibration'] + """
              <li>""" + description['gain calibration'] + """
             </ul>""")


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        parameters -- The dictionary that holds the descriptive information.
        """

        if parameters == None:
            parameters = self._results['parameters']

        if topLevel:
            self._htmlLogger.logHTML("""
             <h3>Data View</h3>""")

            if self._description != None:
                self._htmlLogger.logHTML('<p> %s' % self._description)

        self._htmlLogger.logHTML("""
          <p>The data view is a list of 2-d arrays, one for each value
          of ANTENNA1 in the measurement set. Each array has axes 
          x:ANTENNA2 and y:TIME.""")

        if self._ampOrPhase == 'amp':
            self._htmlLogger.logHTML("""
             Each pixel is the difference between the complex amplitude
             of the 'datum' for that baseline/timestamp and that
             of the the %s for the time %s to which 
             it belongs (all data first averaged over spectral channels if there
             is more than one).""" % (
              self._htmlLogger.glossaryLink('complex median'),
              self._htmlLogger.glossaryLink('chunk')))

        elif self._ampOrPhase == 'phase':
            self._htmlLogger.logHTML("""
             Each pixel is the phase of the 'datum' for that 
             baseline/timestamp relative to that of the the %s
             for the time %s to which 
             it belongs (all data first averaged over spectral channels if there
             is more than one).""" % (
              self._htmlLogger.glossaryLink('complex median'),
              self._htmlLogger.glossaryLink('chunk')))

        if self._dataType == 'raw':
            self._htmlLogger.logHTML("""
             <p>The data were not calibrated.""")

        elif self._dataType == 'corrected':
            description = {}

            description['bandpass calibration'] = \
             self._bpCal.createHTMLDescription(stageName, parameters=
             parameters['dependencies']['bpCal'])['bandpass calibration']

            description['gain calibration'] = \
             self._gainCal.createHTMLDescription(stageName, parameters=
             parameters['dependencies']['gainCal'])['gain calibration']

            self._htmlLogger.logHTML("""
             <p>The data were calibrated as follows:
             <ul>
              <li>""" + description['bandpass calibration'] + """
              <li>""" + description['gain calibration'] + """
             </ul>""")

#        self._htmlLogger.logHTML("""
#         <p>This data view was calculated by Python class %s.""" % 
#         self._className)

           
class RawAmplitudeDeviationPerBaseline(DataDeviationPerBaseline):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType):
        """Constructor.
        
        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- Name of stage using this object.
        sourceType  -- type of source whose data are to be used.  
        """
#        print 'RawAmplitudeDeviationPerBaseline constructor called: %s %s' % (
#         msName, sourceType)
        DataDeviationPerBaseline.__init__(self, tools, bookKeeper,
         msCalibrater, msFlagger, htmlLogger, msName, stageName, sourceType,
         'raw', 'amp')
        self._className = 'RawAmplitudeDeviationPerBaseline'


class RawPhaseDeviationPerBaseline(DataDeviationPerBaseline):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType):
        """Constructor.
        
        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- Name of stage using this object.
        sourceType  -- type of source whose data are to be used.  
        """
#        print 'RawPhaseDeviationPerBaseline constructor called: %s %s' % (
#         msName, sourceType)

        DataDeviationPerBaseline.__init__(self, tools, bookKeeper, 
         msCalibrater, msFlagger, htmlLogger, msName, stageName, sourceType,
         'raw', 'phase')
        self._className = 'RawPhaseDeviationPerBaseline'


class CorrectedAmplitudeDeviationPerBaseline(DataDeviationPerBaseline):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, bandpassCal, gainCal):
        """Constructor.
        
        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- Name of stage using this object.
        sourceType  -- type of source whose data are to be used.  
        """
#        print 'CorrectedAmplitudeDeviationPerBaseline constructor called: %s %s' % (
#         msName, sourceType)
        DataDeviationPerBaseline.__init__(self, tools, bookKeeper, 
         msCalibrater, msFlagger, htmlLogger, msName, stageName, sourceType,
         'corrected', 'amp', bandpassCal, gainCal)
        self._className = 'CorrectedAmplitudeDeviationPerBaseline'


class CorrectedPhaseDeviationPerBaseline(DataDeviationPerBaseline):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, bandpassCal, gainCal):
        """Constructor.
        
        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- Name of stage using this object.
        sourceType  -- type of source whose data are to be used.  
        """
#        print 'CorrectedPhaseDeviationPerBaseline constructor called: %s %s' % (
#         msName, sourceType)
        DataDeviationPerBaseline.__init__(self, tools, bookKeeper, 
         msCalibrater, msFlagger, htmlLogger, msName, stageName, sourceType,
         'corrected', 'phase', bandpassCal, gainCal)
        self._className = 'CorrectedPhaseDeviationPerBaseline'
