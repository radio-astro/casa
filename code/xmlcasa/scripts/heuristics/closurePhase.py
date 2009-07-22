"""Module to calculate closure phase errors."""

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

# package modules

import copy as python_copy
from numpy import *
import pickle

# alma modules

import casac
from baseData import *

class ClosurePhase(BaseData):
    """Class to calculate closure phase errors."""

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger, msName,
     stageName, view):
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
#        print 'ClosurePhase.__init__ called'
        BaseData.__init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
         msName, stageName)
        self._view = view
        self._sourceType = self._view._sourceType
        self._field_ids = self.getFieldsOfType(self._sourceType)
        self._antennaRange = self._results['summary']['antenna_range']


    def getData(self, topLevel=False):
        """Public method to return the phase error as a 'view' of the data.
                
        Keyword arguments:
        topLevel -- True if this is the data 'view' to be displayed directly.
        """

#        print 'ClosurePhase.getData called'
        self._htmlLogger.timing_start('ClosurePhase.getData')

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
            for data_desc_id in self._data_desc_ids:
                for field_id in self._field_ids:
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

# get the 'corrected' and 'model' data

                    flagVersions = ['BeforeHeuristics', 'Current']
                    corr_axis, chan_freq, times, chunks, antenna1, \
                     antenna2, corrected_real, corrected_imag, corrected_flag, \
                     corrected_flag_row = \
                     self._getBaselineData(field_id, data_desc_id, 
                     self._antennaRange, 'corrected', flagVersions)

                    corr_axis, chan_freq, times, chunks, antenna1, \
                     antenna2, model_real, model_imag, model_flag, \
                     model_flag_row = \
                     self._getBaselineData(field_id, data_desc_id,
                     self._antennaRange, 'model', flagVersions)

# declare arrays

                    ncorr = len(shape(corr_axis))
                    closure_phase = zeros([ncorr, max(self._antennaRange)+1,
                     len(times), max(self._antennaRange)+1], float)
                    closure_phase_flag = [
                     ones([ncorr, max(self._antennaRange)+1,
                     len(times), max(self._antennaRange)+1], int),
                     ones([ncorr, max(self._antennaRange)+1,
                     len(times), max(self._antennaRange)+1], int)]
                    mad_floor = zeros([ncorr, max(self._antennaRange)+1,
                     len(times), max(self._antennaRange)+1], float)

# loop through antennas and for each build up 'baseline v time'
# closure phase

                    for antenna in self._antennaRange:
#                        self._writeOnLine('building image for antenna %s' %
#                         antenna)
                        ifr_range = arange(len(antenna1))
                        ifr_range = compress(antenna1==antenna, ifr_range)

                        for corr in range(ncorr):
                            for ifr in ifr_range:
                                other_ant = antenna2[ifr]

                                for t in range(len(times)):
#                                    self._writeOnLine(
#                                     'calculating %s %s %s %s' % 
#                                     (corr, antenna, other_ant, t))

                                    if corrected_flag_row[0][ifr,t] or \
                                     model_flag_row[0][ifr,t]:
                                        continue

                                    closure_phase_flag[0][corr,antenna,t,other_ant] = \
                                     closure_phase_flag[0][corr,other_ant,t,antenna] = \
                                     False

                                    if corrected_flag_row[-1][ifr,t] or \
                                     model_flag_row[-1][ifr,t]:
                                        continue

                                    valid_corrected_real = compress(logical_not(
                                     corrected_flag[-1][corr,:,ifr,t]),
                                     corrected_real[corr,:,ifr,t])
                                    if len(valid_corrected_real) > 0:
                                        median_corrected_real = median(
                                         valid_corrected_real)
                                        valid_corrected_imag = compress(
                                         logical_not(corrected_flag[-1][corr,:,ifr,t]),
                                         corrected_imag[corr,:,ifr,t])
                                        median_corrected_imag = median(
                                         valid_corrected_imag)
                                        median_model_real = median(compress(
                                         logical_not(model_flag[-1][corr,:,ifr,t]),
                                         model_real[corr,:,ifr,t]))
                                        median_model_imag = median(compress(
                                         logical_not(model_flag[-1][corr,:,ifr,t]),
                                         model_imag[corr,:,ifr,t]))

# divide data by model, store amplitude and phase of result

                                        corrected_complex = complex(
                                         median_corrected_real,
                                         median_corrected_imag)
                                        model_complex = complex(
                                         median_model_real,
                                         median_model_imag)
                                        try:
                                            closure = corrected_complex /\
                                             model_complex
                                            phase = \
                                             closure_phase[corr,antenna,t,other_ant] = \
                                             closure_phase[corr,other_ant,t,antenna] = \
                                             arctan2(closure.imag, closure.real)
                                            closure_phase_flag[-1][corr,antenna,t,other_ant] = \
                                             closure_phase_flag[-1][corr,other_ant,t,antenna] = \
                                             False

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
                                        
                                        except ZeroDivisionError:
                                            pass

                    for antenna in self._antennaRange:
                        for corr in range(ncorr):
                            description = {}
                            description['TITLE'] = \
                             'Closure phase (deg) - Antenna1:%s Field:%s Spw:%s Corr:%s' % (
                             self._pad(antenna), self._fieldName[field_id], 
                             self._pad(data_desc_id), corr_axis[corr])
                            description['DATA_DESC_ID'] = str(data_desc_id)
                            description['FIELD_ID'] = str(field_id)
                            description['POLARIZATION_ID'] = corr_axis[corr]
                            description['ANTENNA1'] = str(antenna)

                            result = {}
                            result['dataType'] = 'closure phase'
                            result['xtitle'] = 'ANTENNA2'
                            result['x'] = arange(max(self._antennaRange)+1) 
                            result['ytitle'] = 'TIME'
                            result['y'] = times
                            result['chunks'] = chunks
                            result['data'] = closure_phase[corr,antenna] * (180.0 / math.pi)
                            result['mad_floor'] =  mad_floor[corr,antenna] * (180.0 / math.pi)
                            result['dataUnits'] = 'degrees'
                            result['flag'] = [
                             closure_phase_flag[0][corr,antenna],
                             closure_phase_flag[1][corr,antenna]]
                            result['flagVersions'] = flagVersions

                            pickled_description = pickle.dumps(description)
                            results['data'][pickled_description] = result

            success = self._ms.close()

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
                self._results['data'][k].append(results['data'][k])
            else:
                self._results['data'][k] = [results['data'][k]]

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('ClosurePhase.getData')
        return temp


    def inputs(self):
        """Method to return the interface parameters of this object.
        """
        result = {}
        result['objectType'] = 'ClosurePhase'
        result['sourceType'] = None
        result['furtherInput'] = {}
        result['outputFiles'] = []
        result['dependencies'] = [self._view.inputs()]

        flag_marks = ''
        for field_id in self._field_ids:
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
         Each pixel is the phase (in degrees) of the result of the complex
         division 'corrected data'/'model data' for that time/baseline.
         For multi-channel spectral windows the median is taken of the data 
         across the channels.""")


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
         Each pixel is the phase (in degrees) of the result of the complex
         division 'corrected data'/'model data' for that time/baseline.
         For multi-channel spectral windows the median is taken of the data 
         across the channels.

         <h5>The Model that is compared to the Data</h5>""")

        self._view.writeDetailedHTMLDescription(stageName, False, parameters=
         parameters['dependencies']['view'])
