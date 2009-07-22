"""Module to calculate closure errors."""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Remove references to hut.methods.
#  6-Nov-2007 jfl Best bandpass release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import copy as python_copy
from numpy import *
import pickle

# alma modules

import casac
from baseData import *

class ClosureError(BaseData):
    """Class to calculate closure errors."""

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, errorType, view):
        """Constructor.

        Keyword arguments:
        tools        -- BaseTools object.
        bookKeeper   -- BookKeeper object.
        msCalibrater -- MSCalibrater object.
        msFlagger    -- MSFlagger object.
        htmlLogger   -- Route for logging to html structure.
        msName       -- Name of MeasurementSet
        stageName    -- Name of stage using this object.
        errorType    -- 'amplitude', 'phase' or 'magnitude' (distance from 1+0i) of 
                        error.
        view         -- View for calculating model.
        """
#        print 'ClosureError.__init__ called'
        BaseData.__init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
         msName, stageName)
        self._view = view
        self._errorType = errorType
        self._sourceType = self._view._sourceType
        self._antennaRange = self._results['summary']['antenna_range']

        target_field_ids = self.getFieldsOfType(self._sourceType)
        field_spw = self._valid_field_spw
        self._target_field_spw = []
        for key in self._valid_field_spw:
            field = key[0]
            if target_field_ids.count(field) > 0:
                self._target_field_spw.append(key)

        self._first = True


    def __del__(self):
        """Destructor
        """

# delete the flag state holding the PreviousClosureError flags.

        self._msFlagger.deleteFlagState('PreviousClosureError')


    def getData(self, topLevel=False):
        """Public method to return the closure error as a 'view' of the data.
                
        Keyword arguments:
        topLevel -- True if this is the data 'view' to be displayed directly.
        """

#        print 'ClosureError.getData called'
        self._htmlLogger.timing_start('ClosureError.getData')

# have these data been calculated already?

        inputs = self.inputs()
        entryID,results = self._bookKeeper.available(
         objectType=inputs['objectType'], 
         sourceType=inputs['sourceType'],
         furtherInput=inputs['furtherInput'],
         outputFiles=inputs['outputFiles'],
         dependencies=inputs['dependencies'])

        if entryID == None:

# calculate the clean image

            view_parameters = self._view.calculate()

            results = {
             'parameters':{'history':'',
                           'dependencies':{}},
             'data':{}}
 
            results['parameters']['history'] = self._fullStageName
            results['parameters']['dependencies']['view'] = view_parameters

# loop through data_desc_id and field_id

            self._ms.open(thems=self._msName)

            for key in self._target_field_spw:
                field_id = key[0]
                data_desc_id = key[1]

                self._ms.selectinit(reset=True)
                self._ms.selectinit(datadescid = data_desc_id)
                query = "FIELD_ID==%s AND (ANTENNA1!=ANTENNA2)" % (field_id)
                rtn = self._ms.selecttaql(msselect = query)
                if self._ms.nrow(selected = True) == 0:
                    self._log('..no data for field_id %s data_desc_id %s' %
                     (field_id, data_desc_id))
                    continue

                self._log('..looking at field_id %s data_desc_id %s' %
                 (field_id, data_desc_id))

# get the 'corrected' and 'model' data for the flag states of interest

                flagVersions = ['BeforeHeuristics', 'StageEntry',
                 'CleanImageCalibration', 'Current']
                if not self._first:
                    flagVersions.append('PreviousClosureError') 

                corr_axis, chan_freq, times, chunks, antenna1, \
                 antenna2, corrected_real, corrected_imag, corrected_flag,\
                 corrected_flag_row = \
                 self._getBaselineData(field_id, data_desc_id, 
                 self._antennaRange, 'corrected', flagVersions)

                corr_axis, chan_freq, times, chunks, antenna1, \
                 antenna2, model_real, model_imag, model_flag, \
                 model_flag_row = \
                 self._getBaselineData(field_id, data_desc_id,
                 self._antennaRange, 'model', flagVersions)

# if this is not the first iteration, have any flags changed since the last

                current = flagVersions.index('Current')
                calibration = flagVersions.index('CleanImageCalibration')

                calculate = True
                if not self._first:
                    previous = flagVersions.index('PreviousClosureError')

                    if all(corrected_flag_row[previous] == \
                     corrected_flag_row[current]) and \
                     all(corrected_flag[previous] == \
                     corrected_flag[current]):
                        calculate = False

                if calculate:

# declare arrays

                    ncorr = len(shape(corr_axis))
                    closure_error = zeros([ncorr, max(self._antennaRange)+1,
                     len(times), max(self._antennaRange)+1], float)
                    closure_error_flag = []
                    for fi,fs in enumerate(flagVersions):
                        closure_error_flag.append(
                         ones([ncorr, max(self._antennaRange)+1,
                         len(times), max(self._antennaRange)+1], int))
                    no_data_flag = ones([ncorr, max(self._antennaRange)+1,
                     len(times), max(self._antennaRange)+1], int)

                    mad_floor = zeros([ncorr, max(self._antennaRange)+1,
                     len(times), max(self._antennaRange)+1], float)
                    new_values = zeros([ncorr, max(self._antennaRange)+1,
                     len(times), max(self._antennaRange)+1], bool)

# loop through antennas and for each build up 'baseline v time'
# closure error

                    for antenna in self._antennaRange:
                        ifr_range = arange(len(antenna1))
                        ifr_range = compress(antenna1==antenna, ifr_range)

                        for corr in range(ncorr):
                            for ifr in ifr_range:
                                other_ant = antenna2[ifr]

                                for t in range(len(times)):
 
# set flags

                                    no_data_flag[corr,antenna,t,other_ant] = \
                                     no_data_flag[corr,other_ant,t,antenna] = \
                                     False

# idea here is that closure_error_flag is set for all levels 'beyond' that where
# it is first set. This caters for the fact that cleanImageCalibration may be set
# even when the later 'Current' flag is not; in this case we want both levels of
# closure_error_flag to be set.

                                    for fi,fs in enumerate(flagVersions):
                                        if corrected_flag_row[fi][ifr,t] or \
                                         all(corrected_flag[fi][corr,:,ifr,t]) or \
                                         model_flag_row[fi][ifr,t] or \
                                         all(model_flag[fi][corr,:,ifr,t]):
                                            break

                                        closure_error_flag[fi][corr,antenna,t,other_ant] = \
                                        closure_error_flag[fi][corr,other_ant,t,antenna] = \
                                         False

# nothing to do if 'current' flag is True or if 'CleanImageCalibration' is True

                                    if closure_error_flag[current]\
                                     [corr,antenna,t,other_ant]:
                                        continue

                                    if closure_error_flag[calibration]\
                                     [corr,antenna,t,other_ant]:
                                        continue

                                    valid_corrected_real = compress(logical_not(
                                     corrected_flag[current][corr,:,ifr,t]),
                                     corrected_real[corr,:,ifr,t])
                                    if len(valid_corrected_real) > 0:
                                        median_corrected_real = median(
                                         valid_corrected_real)
                                        valid_corrected_imag = compress(
                                         logical_not(
                                         corrected_flag[current][corr,:,ifr,t]),
                                         corrected_imag[corr,:,ifr,t])
                                        median_corrected_imag = median(
                                         valid_corrected_imag)
                                        median_model_real = median(compress(
                                         logical_not(
                                         model_flag[current][corr,:,ifr,t]),
                                         model_real[corr,:,ifr,t]))
                                        median_model_imag = median(compress(
                                         logical_not(
                                         model_flag[current][corr,:,ifr,t]),
                                         model_imag[corr,:,ifr,t]))

# divide data by model, store amplitude or phase of result

                                        corrected_complex = complex(
                                         median_corrected_real,
                                         median_corrected_imag)
                                        model_complex = complex(
                                         median_model_real,
                                         median_model_imag)

                                        try:
                                            closure = corrected_complex /\
                                             model_complex
 
                                            if self._errorType == 'magnitude':
                                                error = \
                                                 closure_error[corr,antenna,t,other_ant] = \
                                                 closure_error[corr,other_ant,t,antenna] = \
                                                 abs(closure - 1.0)

# mad floor not calculate correctly for 'magnitude'

                                                mad_floor[corr,antenna,t,other_ant] = \
                                                 mad_floor[corr,other_ant,t,antenna] = 0.0
                                            elif self._errorType == 'amplitude':
                                                error = \
                                                 closure_error[corr,antenna,t,other_ant] = \
                                                 closure_error[corr,other_ant,t,antenna] = \
                                                 abs(closure)

# mad floor not calculate correctly for 'amplitude'

                                                mad_floor[corr,antenna,t,other_ant] = \
                                                 mad_floor[corr,other_ant,t,antenna] = 0.0
                                            elif self._errorType == 'phase':
                                                phase = \
                                                 closure_error[corr,antenna,t,other_ant] = \
                                                 closure_error[corr,other_ant,t,antenna] = \
                                                 arctan2(closure.imag, closure.real)

# and a measure of the scatter of points about the 'receiver' position,
# with sky variation removed. This is the noise 'floor' of the measurement.

                                                corrected_complex_array = zeros(
                                                 [len(valid_corrected_real)], complex)
                                                corrected_complex_array.real = \
                                                 valid_corrected_real
                                                corrected_complex_array.imag = \
                                                 valid_corrected_imag
                                                closure_array = \
                                                 corrected_complex_array /\
                                                model_complex

                                                relative_real = closure_array.real * \
                                                 cos(phase) + closure_array.imag * \
                                                 sin(phase)
                                                relative_imag = -closure_array.real * \
                                                 sin(phase) + closure_array.imag * \
                                                 cos(phase)

                                                relative_phase = arctan2(relative_imag,
                                                 relative_real)
                                                relative_phase_mad = median(abs(
                                                 relative_phase))

                                                nchannels = len(relative_phase)
                                                mad_floor[corr,antenna,t,other_ant] = \
                                                 mad_floor[corr,other_ant,t,antenna] = \
                                                 relative_phase_mad / sqrt(float(nchannels))
                                            else:
                                                raise NameError, 'bad errorType: %s' % self._errorType

                                        except ZeroDivisionError:

# there is a possible issue with the flagging of closure error results. Any flags
# put on the data as the calibration was applied will have been removed by the
# imaging class after the imaging has been done - to prevent such 'implicit'
# flags propagating. However, the corrected data will still be no good - here 
# we depend on such data being identically zero.

                                            closure_error_flag[current]\
                                             [corr,antenna,t,other_ant] = \
                                             closure_error_flag[current]\
                                             [corr,other_ant,t,antenna] = True

                    for antenna in self._antennaRange:
                        for corr in range(ncorr):
                            description = {}
                            if self._errorType == 'magnitude':
                                description['TITLE'] = \
                                 'Closure error magnitude - Antenna1:%s Field:%s Spw:%s Corr:%s' % (
                                 self._pad(antenna), self._fieldName[field_id], 
                                 self._pad(data_desc_id), corr_axis[corr])
                            elif self._errorType == 'amplitude':
                                description['TITLE'] = \
                                 'Closure amplitude - Antenna1:%s Field:%s Spw:%s Corr:%s' % (
                                 self._pad(antenna), self._fieldName[field_id], 
                                 self._pad(data_desc_id), corr_axis[corr])
                            elif self._errorType == 'phase':
                                description['TITLE'] = \
                                 'Closure phase (deg) - Antenna1:%s Field:%s Spw:%s Corr:%s' % (
                                 self._pad(antenna), self._fieldName[field_id], 
                                 self._pad(data_desc_id), corr_axis[corr])
                            description['DATA_DESC_ID'] = int(data_desc_id)
                            description['FIELD_ID'] = int(field_id)
                            description['FIELD_NAME'] = self._fieldName[
                             int(field_id)]
                            description['FIELD_TYPE'] = self._fieldType[
                             int(field_id)]
                            description['POLARIZATION_ID'] = corr_axis[corr]
                            description['ANTENNA1'] = int(antenna)

                            if calculate:

# store new data
                                result = {}
                                result['dataType'] = 'closure %s' % self._errorType
                                result['xtitle'] = 'ANTENNA2'
                                result['x'] = arange(max(self._antennaRange)+1) 
                                result['ytitle'] = 'TIME'
                                result['y'] = times
                                result['chunks'] = chunks
                                if self._errorType == 'magnitude':
                                    result['data'] = closure_error[corr,antenna]
                                    result['mad_floor'] =  mad_floor[corr,antenna]
                                    result['dataUnits'] = 'ratio'
                                if self._errorType == 'amplitude':
                                    result['data'] = closure_error[corr,antenna]
                                    result['mad_floor'] =  mad_floor[corr,antenna]
                                    result['dataUnits'] = 'ratio'
                                elif self._errorType == 'phase':
                                    result['data'] = closure_error[corr,antenna] * (180.0 / math.pi)
                                    result['mad_floor'] =  mad_floor[corr,antenna] * (180.0 / math.pi)
                                    result['dataUnits'] = 'degrees'

                                flagsToStore = ['BeforeHeuristics', 'StageEntry',
                                 'Current']
                                temp_flag = []
                                flagVersionsStored = []
                                for f in flagsToStore:
                                    if flagVersions.count(f) == 0:
                                        continue
                                    fi = flagVersions.index(f)
                                    temp_flag.append(closure_error_flag[fi]
                                     [corr,antenna])
                                    flagVersionsStored.append(f)
                                temp_flag.append(no_data_flag[corr,antenna])
                                flagVersionsStored.append('NoData')
                                result['flag'] = temp_flag
                                result['flagVersions'] = flagVersionsStored
                            else:

# now new data were calculated

                                result['dataType'] = 'copy'
 
                            pickled_description = pickle.dumps(description)
                            results['data'][pickled_description] = result

            success = self._ms.close()
            self._msFlagger.saveFlagState('PreviousClosureError')
            self._first = False

# store the object info in the BookKeeper

            self._bookKeeper.enter(
             objectType=inputs['objectType'], 
             sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             outputFiles=inputs['outputFiles'],
             outputParameters=results,
             dependencies=inputs['dependencies'])

# now add the latest results to the returned structure

        flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()
        self._results['flagging'].append(flagging)
        self._results['flaggingReason'].append(flaggingReason)
        self._results['flaggingApplied'].append(flaggingApplied)

        self._results['parameters'] = results['parameters']

        for k in results['data'].keys():
            if self._results['data'].has_key(k):
                if results['data'][k]['dataType'] == 'copy':
                    new_data = python_copy.deepcopy(
                     self._results['data'][k][-1])
                    self._results['data'][k].append(new_data)
                else:
                    self._results['data'][k].append(results['data'][k])
            else:
                if results['data'][k]['dataType'] == 'copy':
                    raise Exception, 'no earlier versio of data available'
                self._results['data'][k] = [results['data'][k]]

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('ClosureError.getData')
        return temp


    def inputs(self):
        """Method to return the interface parameters of this object.
        """
        result = {}
        result['objectType'] = 'ClosureError'
        result['sourceType'] = None
        result['furtherInput'] = {'errorType':self._errorType}
        result['outputFiles'] = []
        result['dependencies'] = [self._view.inputs()]

        flag_marks = ''
        for field_spw in self._target_field_spw:
            field_id = field_spw[0]
            flag_marks = '%s%s' % (flag_marks, 
             self._msFlagger.getFlagMarkDict(field_id))

        result['flag_marks'] = flag_marks

        return result


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

# write any description given as part of the recipe.

        if self._description != None:
            self._htmlLogger.logHTML('<p> %s' % self._description)

        self._htmlLogger.logHTML("""
         <p>The data view is a list of 2-d arrays, one for each
         value of ANTENNA1 in the MeasurementSet. Each array has axes 
         ANTENNA2 and TIME.
         Each pixel is the %s of the result of the complex
         division 'corrected data'/'model data' for that time/baseline.
         For multi-channel spectral windows the median is taken of the data 
         across the channels.""" % self._errorType)


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

        self._htmlLogger.logHTML("""
         <p>The data view is a list of 2-d arrays, one for each
         value of ANTENNA1 in the MeasurementSet. Each array has axes 
         ANTENNA2 and TIME.
         Each pixel is the %s of the result of the complex
         division 'corrected data'/'model data' for that time/baseline.
         For multi-channel spectral windows the median is taken of the data 
         across the channels.

         <h5>The Model that is compared to the Data</h5>""" % self._errorType)

        self._view.writeDetailedHTMLDescription(stageName, False, parameters=
         parameters['dependencies']['view'])


class ClosureErrorAmplitude(ClosureError):
    """Class to calculate amplitude of closure errors."""

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, view):
        """Constructor.

        Keyword arguments:
        tools        -- BaseTools object.
        bookKeeper   -- BookKeeper object.
        msCalibrater -- MSCalibrater object.
        msFlagger    -- MSFlagger object.
        htmlLogger   -- Route for logging to html structure.
        msName       -- Name of MeasurementSet
        stageName    -- Name of stage using this object.
        view         -- View for calculating model.
        """
#        print 'ClosureErrorAmplitude.__init__ called'
        ClosureError.__init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
         msName, stageName, 'amplitude', view)


class ClosureErrorMagnitude(ClosureError):
    """Class to calculate magnitude of closure errors."""

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, view):
        """Constructor.

        Keyword arguments:
        tools        -- BaseTools object.
        bookKeeper   -- BookKeeper object.
        msCalibrater -- MSCalibrater object.
        msFlagger    -- MSFlagger object.
        htmlLogger   -- Route for logging to html structure.
        msName       -- Name of MeasurementSet
        stageName    -- Name of stage using this object.
        view         -- View for calculating model.
        """
#        print 'ClosureErrorAmplitude.__init__ called'
        ClosureError.__init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
         msName, stageName, 'magnitude', view)


class ClosureErrorPhase(ClosureError):
    """Class to calculate phase of closure errors."""

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, view):
        """Constructor.

        Keyword arguments:
        tools        -- BaseTools object.
        bookKeeper   -- BookKeeper object.
        msCalibrater -- MSCalibrater object.
        msFlagger    -- MSFlagger object.
        htmlLogger   -- Route for logging to html structure.
        msName       -- Name of MeasurementSet
        stageName    -- Name of stage using this object.
        view         -- View for calculating model.
        """
#        print 'ClosureErrorAmplitude.__init__ called'
        ClosureError.__init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
         msName, stageName, 'phase', view)
