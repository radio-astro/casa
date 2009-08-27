"""Module to coadd amplitudes of all data from each antenna."""

# History:
# 28-Nov-2007 jfl Recipe release.
# 10-Apr-2008 jfl F2F release.
#  2-Jun-2008 jfl 2-jun release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
#  3-Nov-2008 jfl amalgamated stage release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
# 31-Jul-2009 jfl no maxPixels release, more efficient.

# package modules

from numpy import * 
import pickle

# alma modules

from baseData import *


class CoaddedAntennaAmplitude(BaseData):
    """Class to get coadded amplitudes of all data from each antenna. 
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, dataType, amplitudeType, bandpassCal=None,
     gainCal=None, bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools         -- BaseTools object.
        bookKeeper    -- BookKeeper object.
        msCalibrater  -- MSCalibrater object.
        msFlagger     -- MSFlagger object.
        htmlLogger    -- Route for logging to html structure.
        msName        -- Name of MeasurementSet
        stageName     -- The name of the stage using this object.
        sourceType    -- Type of source whose data are to be coadded.
        dataType      -- 'raw' or 'corrected'.
        amplitudeType -- 'scalar' or 'vector'.
        bandpassCal   -- Class for bandpass calibration if dataType is
                         'corrected'.
        gainCal       -- Class for gain calibration if dataType is
                         'corrected'.
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels to be ignored in calculating
                              -- integrated map.
        """

        BaseData.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName)
        self._sourceType = sourceType
        self._dataType = dataType
        self._amplitudeType = amplitudeType

        success = self._ms.open(self._msName)
        self._target_field_ids = self.getFieldsOfType(self._sourceType)
        self._ms.close()

        self._getDataCalls = 0 
        self._first = True
        self._next_target_field_spw = []

        if self._dataType == 'corrected':
            if len(bandpassCal) == 1:
                self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater,
                 msFlagger, htmlLogger, msName, stageName)
            else:
                self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater,
                 msFlagger, htmlLogger, msName, stageName,
                 viewClassList=bandpassCal[1:])
            self._gainCal = gainCal(tools, bookKeeper, msCalibrater,
             msFlagger, htmlLogger, msName, stageName, 
             bandpassCal=bandpassCal, 
             bandpassFlaggingStage=bandpassFlaggingStage)


    def __del__(self):
        """Destructor
        """

# delete the flag state holding the PreviousIteration flags.

        self._msFlagger.deleteFlagState('PreviousIteration')


    def getData(self):
        """Public method to return the coadded amplitudes in a time versus 
        antenna 'view' of the data.
        """

#        print 'CoaddedAntennaAmplitude.getData called'
        self._getDataCalls += 1
        self._htmlLogger.timing_start('CoaddedAntennaAmplitude.getData_%s' %
         self._getDataCalls)

# are the data already available

        inputs = self.inputs()
        entryID,results = self._bookKeeper.available(
         objectType=inputs['objectType'], sourceType=inputs['sourceType'],
         furtherInput=inputs['furtherInput'], outputFiles=[])

        if entryID == None:
            results = {
             'parameters':{'history':{}, 'dependencies':{}},
             'data':{}}
            results['parameters']['history'] = self._fullStageName

            antenna_range = self._results['summary']['antenna_range']

# get the bandpass calibrations to apply for each SpW

            if self._dataType == 'corrected':
                bpCalParameters = self._bpCal.calculate()
                gainCalParameters = self._gainCal.calculate()

                results['parameters']['dependencies']['bpCal'] = bpCalParameters
                results['parameters']['dependencies']['gainCal'] = \
                 gainCalParameters
                       
# which data_desc_id/field_id combinations do we want to look through?
        
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

# build the results structure

            for kk in target_field_spw:
                field_id = kk[0]
                data_desc_id = kk[1]
                results['parameters'][(field_id,data_desc_id)] = {}

            if self._dataType=='corrected':

# first, make sure the data are calibrated with the up-to-date calibration, if
# required

                calibrated_field_spw = {}
                for kk in target_field_spw:
                    key = tuple(kk)
                    if calibrated_field_spw.has_key(key):
                        continue

                    field_id = key[0]
                    data_desc_id = key[1]

# calibrate all fields for this spw at once - it is more efficient to use the
# calibrater tool this way and msCalibrater will prevent the tool being used
# when not necessary

                    field_list = [field_id]
                    for kktemp in target_field_spw:
                        if (kktemp[1]==data_desc_id) and \
                         (field_list.count(kktemp[0])==0): 
                            field_list.append(kktemp[0])

# apply the calibration

                    self._bpCal.setapply(spw=data_desc_id, field=field_list)
                    self._gainCal.setapply(spw=data_desc_id, field=field_list)
                    newCommands,error = self._msCalibrater.correct(
                     spw=data_desc_id, field=field_list)

                    for temp_field in field_list:
                        results['parameters'][(temp_field,data_desc_id)]\
                         ['commands'] = newCommands
                        results['parameters'][(temp_field,data_desc_id)]\
                         ['error'] = error
                        calibrated_field_spw[(temp_field,data_desc_id)] = True


# read into a dictionary all the data for the measured spw/fields - do it
# this way because reading it in piecemeal for each spw/field can be slow
# as the flagversion gets changed several times for each little bit.

# set the flag states we want to look at, then read in data for each

            flagVersions = ['BeforeHeuristics', 'StageEntry', 'Current']
            if not self._first:
                flagVersions.append('PreviousIteration')

            self._ms.open(self._msName)
            data_in = {}
            for fv,flagVersion in enumerate(flagVersions):
                for kk in target_field_spw:
                    key = tuple(kk)
                    field_id = key[0]
                    data_desc_id = key[1]
                        
                    if fv == 0:

# get the data and first flagVersion  

                        data_in[key] = {}
                        corr_axis, chan_freq, times, chunks, antenna1,\
                         antenna2, ifr_real, ifr_imag, ifr_flag,\
                         ifr_flag_row = self._getBaselineData(field_id,
                         data_desc_id, antenna_range, self._dataType,
                         [flagVersion])

                        data_in[key]['corr_axis'] = corr_axis
                        data_in[key]['chan_freq'] = chan_freq
                        data_in[key]['times'] = times
                        data_in[key]['chunks'] = chunks
                        data_in[key]['antenna1'] = antenna1
                        data_in[key]['antenna2'] = antenna2
                        data_in[key]['ifr_real'] = ifr_real
                        data_in[key]['ifr_imag'] = ifr_imag
                        data_in[key]['ifr_flag'] = ifr_flag
                        data_in[key]['ifr_flag_row'] = ifr_flag_row
                    else:

# get flags only for subsequent flagversions

                        ifr_flag, ifr_flag_row = self._getBaselineData(
                         field_id, data_desc_id, antenna_range,
                         self._dataType, [flagVersion], flagsOnly=True)

                        data_in[key]['ifr_flag'] += ifr_flag
                        data_in[key]['ifr_flag_row'] += ifr_flag_row

            self._msFlagger.setFlagState('Current')

# now do the calculations

            for kk in target_field_spw:
                key = tuple(kk)
                field_id = key[0]
                data_desc_id = key[1]

                corr_axis = data_in[key]['corr_axis']
                chan_freq = data_in[key]['chan_freq']
                times = data_in[key]['times']
                chunks = data_in[key]['chunks']
                antenna1 = data_in[key]['antenna1']
                antenna2 = data_in[key]['antenna2']
                ifr_real = data_in[key]['ifr_real']
                ifr_imag = data_in[key]['ifr_imag']
                ifr_flag = data_in[key]['ifr_flag']
                ifr_flag_row = data_in[key]['ifr_flag_row']


                for corr in range(len(corr_axis)):

# declare arrays

                    antenna_amplitude = zeros([len(times),
                     max(antenna_range)+1], float)
                    antenna_amplitude_std = zeros([len(times), 
                     max(antenna_range)+1], float)
                    antenna_amplitude_flag = []
                    for fi,fs in enumerate(flagVersions):
                        antenna_amplitude_flag.append(ones([len(times), 
                         max(antenna_range)+1], int))
                    no_data_flag = ones([len(times), max(antenna_range)+1], int)
                    n_sample = zeros([len(times), max(antenna_range)+1], int)
                    new_values = zeros([len(times), max(antenna_range)+1], bool)

# loop through antennas and build up an amplitude 'antenna v time' image

                    for antenna in antenna_range:
                        ifr_range = range(len(antenna1))
                        ifr_range = compress(logical_or((antenna1==antenna),
                         (antenna2==antenna)), ifr_range)

# get a list of amplitudes for each valid timestamp and ifr in the dataset

                        for t in range(len(times)):
                            current = flagVersions.index('Current')
                            if not self._first:

# first time round, need to calculate all data. Otherwise,
# have any flags in this timestamp changed since the PreviousIteration?

                                previous = flagVersions.index(
                                 'PreviousIteration')
                                if all(ifr_flag_row[previous][:,t] == \
                                 ifr_flag_row[current][:,t]) and \
                                 all(ifr_flag[previous][corr,:,:,t] == \
                                 ifr_flag[current][corr,:,:,t]):
#                                     print 'no new flags corr, antenna', corr, antenna
                                    continue

# 'next_target_field' indicates that we'll have to check the flags of these data
# next time around as well

                            if self._next_target_field_spw.count([field_id,
                             data_desc_id]) == 0:
                                self._next_target_field_spw.append(
                                 [field_id, data_desc_id])
                            new_values[t,antenna] = True
                            t_amp_sample = []
                            t_real_sample = []
                            t_imag_sample = []
                            for ifr in ifr_range:
                                no_data_flag[t,antenna] = False
                                for fi,fs in enumerate(flagVersions):
                                    if not(ifr_flag_row[fi][ifr,t]) and \
                                     not(alltrue(ifr_flag[fi][corr,:,ifr,t])):
                                        antenna_amplitude_flag[fi][t,antenna] = 0

                                if not(ifr_flag_row[current][ifr,t]):
                                    if self._amplitudeType == 'scalar':
                                        t_amp = sqrt(
                                         pow(ifr_real[corr,:,ifr,t],2) + 
                                         pow(ifr_imag[corr,:,ifr,t],2))
                                        valid_data = list(compress(
                                         logical_not(ifr_flag[current]
                                         [corr,:,ifr,t]), t_amp))
                                        t_amp_sample += valid_data
                                    elif self._amplitudeType == 'vector':
                                        valid_data = list(compress(
                                         logical_not(
                                         ifr_flag[current][corr,:,ifr,t]),
                                         ifr_real[corr,:,ifr,t]))
                                        t_real_sample += valid_data
                                        valid_data = list(compress(
                                         logical_not(
                                         ifr_flag[current][corr,:,ifr,t]),
                                         ifr_imag[corr,:,ifr,t]))
                                        t_imag_sample += valid_data
                                    else:
                                        raise Exception, \
                                         'bad amplitude type: %s' % \
                                         self._amplitudeType

                            if len(t_amp_sample) > 0:
                                n_sample = len(t_amp_sample)
                                antenna_amplitude[t,antenna] = median(t_amp_sample)
                                if len(t_amp_sample) > 1:
                                    antenna_amplitude_std[t,antenna] = std(
                                     t_amp_sample) / sqrt(n_sample-1)
                            elif len(t_real_sample) > 0:
                                n_sample = len(t_real_sample)
                                antenna_amplitude[t,antenna] = sqrt(
                                 pow(median(t_real_sample),2) + 
                                 pow(median(t_imag_sample),2))
                                if len(t_real_sample) > 1:
                                    t_real_std = std(t_real_sample) / sqrt(n_sample-1)
                                    t_imag_std = std(t_imag_sample) / sqrt(n_sample-1)
                                    antenna_amplitude_std[t,antenna] = sqrt(
                                     pow(t_real_std,2) + pow(t_imag_std,2))

# store the results in the output structure

                    description = {}
                    description['FIELD_ID'] = int(field_id)
                    description['DATA_DESC_ID'] = int(data_desc_id)
                    description['POLARIZATION_ID'] = corr_axis[corr]
                    description['TITLE'] = \
                     'Field:%s Spw:%s Pol:%s coadded %s amplitude' \
                     % (self._fieldName[field_id], self._pad(data_desc_id),
                     corr_axis[corr], self._dataType)

                    result = {}
                    result['dataType'] = '%s coadded %s amplitude' % (
                     self._dataType, self._amplitudeType)
                    result['dataUnits'] = ''
                    result['xtitle'] = 'ANTENNA'
                    result['x'] = arange(max(antenna_range)+1)
                    result['ytitle'] = 'TIME'
                    result['y'] = times
                    result['chunks'] = chunks
                    result['new_values'] = new_values

                    flagsToStore = ['BeforeHeuristics', 'StageEntry', 'Current']
                    temp_flag = []
                    flagVersionsStored = []
                    for f in flagsToStore:
                        if flagVersions.count(f) == 0:
                            continue
                        fi = flagVersions.index(f)
                        temp_flag.append(antenna_amplitude_flag[fi])
                        flagVersionsStored.append(f)
                    temp_flag.append(no_data_flag)
                    flagVersionsStored.append('NoData')
                    result['flag'] = temp_flag
                    result['flagVersions'] = flagVersionsStored
                    result['data'] = antenna_amplitude
                    result['mad_floor'] =  antenna_amplitude_std

                    pickled_description = pickle.dumps(description)
                    results['data'][pickled_description] = result

            self._ms.close()
            self._msFlagger.saveFlagState('PreviousIteration')
            self._first = False

# store the object info in the BookKeeper

            self._bookKeeper.enter(
             objectType=inputs['objectType'], sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'], outputFiles=[],
             outputParameters=results)

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
                    baseResult['flag'][fi][new_values] = \
                     results['data'][k]['flag'][fi][new_values]

                self._results['data'][k].append(baseResult)
            else:           
                self._results['data'][k] = [results['data'][k]]

# copy history and dependency info

        self._results['parameters'] = results['parameters']
        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('CoaddedAntennaAmplitude.getData_%s' %
         self._getDataCalls)

        return temp


    def inputs(self):
        """Method to return the input settings for this object.
        """
        dependencies = []
        if self._dataType=='corrected':
            dependencies = [self._bpCal.inputs(), self._gainCal.inputs()]

        result = {}
        result['objectType'] = 'CoaddedAntennaAmplitude'
        result['sourceType'] = self._sourceType
        result['furtherInput'] = {'dataType':self._dataType,
                                  'amplitudeType':self._amplitudeType}
        result['outputFiles'] = []
        result['dependencies'] = dependencies

        flag_marks = {}
        ignore,flag_mark_col = self._msFlagger.getFlagMarkInfo()
        for field_id in self._target_field_ids:
            flag_marks[field_id] = flag_mark_col[field_id]
        flag_marks = str(flag_marks)

# replace unusual symbols to avoid problems with TaQL

        flag_marks = self._removeProblemTaQLCharacters(flag_marks)
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
          <p>The data view is a list of 2-d arrays, each with axes
          x:ANTENNA and y:TIME. Each pixel is the %s coadd of the
          amplitudes of all the data for that antenna/timestamp.""" %
          self._amplitudeType)

        if self._dataType == 'raw':
            self._htmlLogger.logHTML("""The coadded data were not 
             calibrated.""")
        elif self._dataType == 'corrected':
            description = {}

            description['bandpass calibration'] = \
             self._bpCal.createGeneralHTMLDescription(stageName)\
             ['bandpass calibration']

            description['gain calibration'] = \
             self._gainCal.createGeneralHTMLDescription(stageName)\
             ['gain calibration']

            self._htmlLogger.logHTML("""
             <p>The data were calibrated as follows:
             <ul>
              <li>""" + description['bandpass calibration'] + """
              <li>""" + description['gain calibration'] + """
             </ul>""")


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a detailed description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        parameters -- The dictionary that holds the descriptive information.
        """

        if parameters == None:
            parameters = self._results['parameters']

        if topLevel:
            self._htmlLogger.logHTML("<h3>Data View</h3>")

        self._htmlLogger.logHTML("""
          <p>The data view is a list of 2-d arrays, each with axes
          x:ANTENNA and y:TIME. Each pixel is the %s coadd of the
          amplitudes of all the data for that antenna/timestamp.""" %
          self._amplitudeType)

        if self._dataType == 'raw':
            self._htmlLogger.logHTML("""The coadded data were not 
             calibrated.""")
        elif self._dataType == 'corrected':
            description = {}

            description['bandpass calibration'] = \
             self._bpCal.createDetailedHTMLDescription(stageName, parameters=
             parameters['dependencies']['bpCal'])['bandpass calibration']

            description['gain calibration'] = \
             self._gainCal.createDetailedHTMLDescription(stageName, parameters=
             parameters['dependencies']['gainCal'])['gain calibration']

            self._htmlLogger.logHTML("""
             <p>The data were calibrated as follows:
             <ul>
              <li>""" + description['bandpass calibration'] + """
              <li>""" + description['gain calibration'] + """
             </ul>""")

        applyErrors = []
        for k,v in parameters.iteritems():
            if type(k) == types.TupleType:
                if v.has_key('error'):
                    applyErrors.append(k)

        if len(applyErrors) == 0:
            self._htmlLogger.logHTML('''<p>All calibrations were applied
             successfully''')
        else:
            self._htmlLogger.logHTML('''There were errors applying 
             calibrations to the following (field_id, data_desc_id):''')
            for item in applyErrors:
                self._htmlLogger.logHTML(item)


class CoaddedAntennaRawScalarAmplitude(CoaddedAntennaAmplitude):
    """Class to get coadded scalar amplitudes of all raw data from each
    antenna. 
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType):
        """Constructor.

        Keyword arguments:
        tools         -- BaseTools object.
        bookKeeper    -- BookKeeper object.
        msCalibrater  -- MSCalibrater object.
        msFlagger     -- MSFlagger object.
        htmlLogger    -- Route for logging to html structure.
        msName        -- Name of MeasurementSet
        stageName     -- The name of the stage using this object.
        sourceType    -- Type of source whose data are to be coadded.
        """

        CoaddedAntennaAmplitude.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, sourceType, 'raw', 'scalar')


class CoaddedAntennaCorrectedScalarAmplitude(CoaddedAntennaAmplitude):
    """Class to get coadded scalar amplitudes of all raw data from each
    antenna. 
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, bandpassCal, gainCal,
     bandpassFlaggingStage):
        """Constructor.

        Keyword arguments:
        tools         -- BaseTools object.
        bookKeeper    -- BookKeeper object.
        msCalibrater  -- MSCalibrater object.
        msFlagger     -- MSFlagger object.
        htmlLogger    -- Route for logging to html structure.
        msName        -- Name of MeasurementSet
        stageName     -- The name of the stage using this object.
        sourceType    -- Type of source whose data are to be coadded.
        bandpassCal   -- Class for bandpass calibration if dataType is
                         'corrected'.
        gainCal       -- Class for gain calibration if dataType is
                         'corrected'.
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels to be ignored in calculating
                              -- integrated map.
        """

        CoaddedAntennaAmplitude.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, sourceType, 'corrected',
         'scalar', bandpassCal=bandpassCal, gainCal=gainCal,
         bandpassFlaggingStage=bandpassFlaggingStage)


class CoaddedAntennaCorrectedVectorAmplitude(CoaddedAntennaAmplitude):
    """Class to get coadded vector amplitudes of all raw data from each
    antenna. 
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, bandpassCal, gainCal,
     bandpassFlaggingStage):
        """Constructor.

        Keyword arguments:
        tools         -- BaseTools object.
        bookKeeper    -- BookKeeper object.
        msCalibrater  -- MSCalibrater object.
        msFlagger     -- MSFlagger object.
        htmlLogger    -- Route for logging to html structure.
        msName        -- Name of MeasurementSet
        stageName     -- The name of the stage using this object.
        sourceType    -- Type of source whose data are to be coadded.
        bandpassCal   -- Class for bandpass calibration if dataType is
                         'corrected'.
        gainCal       -- Class for gain calibration if dataType is
                         'corrected'.
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels to be ignored in calculating
                              -- integrated map.
        """

        CoaddedAntennaAmplitude.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, sourceType, 'corrected',
         'vector', bandpassCal=bandpassCal, gainCal=gainCal,
         bandpassFlaggingStage=bandpassFlaggingStage)
