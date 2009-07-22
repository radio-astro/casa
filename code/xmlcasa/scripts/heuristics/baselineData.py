# History:
# 10-Oct-2008 jfl First version.
# 16-Oct-2008 jfl Added CorrectedBaselineAmplitude and
#                 CorrectedBaselinePhase.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import copy as python_copy
from numpy import *
import pickle

# alma modules

from baseTools import *
from baseData import *
from bandpassCalibration import *
from gainCalibration import *

class BaselineData(BaseData):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, calibrationState, dataType,
     bandpassCal=None, gainCal=None, bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools            -- BaseTools object.
        bookKeeper       -- BookKeeper object.
        msCalibrater     -- MSCalibrater object.
        msFlagger        -- MSFlagger object.
        htmlLogger       -- Route for logging to html structure.
        msName           -- Name of MeasurementSet
        stageName        -- Name of stage using this object.
        sourceType       -- Type of source to be used for the G calibration.
        calibrationState -- data to be 'raw' or 'corrected'. 
        dataType         -- data to be 'complex', 'real', imaginary', 'amp'
                            or 'phase'.
        bandpassCal      -- class to be used for the bandpass calibration.
        gainCal          -- class to be used for the flux calibration
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels to be ignored in calculating
                              -- integrated map.
        """
#        print 'BaselineData constructor called: %s %s %s %s' % (
#         msName, sourceType, dataType, calibrationState)
        BaseData.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName)
        self._className = 'BaselineData'
        self._sourceType = sourceType
        self._dataType = dataType
        self._calibrationState = calibrationState
        self._field_ids = self.getFieldsOfType(self._sourceType)
        if bandpassCal != None:
            if len(bandpassCal) == 1:
                self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater,
                 msFlagger, htmlLogger, msName, stageName)
            else:
                self._bpCal = bandpassCal[0](tools, bookKeeper, msCalibrater,
                 msFlagger, htmlLogger, msName, stageName,
                 viewClassList=bandpassCal[1:])
        if gainCal != None:
            self._gainCal = gainCal(tools, bookKeeper, msCalibrater, msFlagger, 
             htmlLogger, msName, stageName, bandpassCal=bandpassCal,
             bandpassFlaggingStage=bandpassFlaggingStage)


    def description (self):
        description = [
         'BaselineData - data from baselines']
        return description


    def getData(self, topLevel=False):
        """
        """

#        print 'BaselineData.getData called'
        self._htmlLogger.timing_start('BaselineData.getData')

# have the data been calculated already?
    
        dependencies = []
        if self._calibrationState=='corrected':
            dependencies = [self._bpCal.inputs(), self._gainCal.inputs()]

        entryID,results = self._bookKeeper.available(
         objectType='BaselineData', sourceType=self._sourceType,
         furtherInput={'dataType':self._dataType,
                       'calibrationState':self._calibrationState},
         outputFiles=[],
         dependencies=dependencies)

        if entryID == None:

            results = {
             'parameters':{'history':{}, 'dependencies':{}},
             'data':{}}
            results['parameters']['history'] = self._fullStageName

            antenna_range = self._results['summary']['antenna_range']

            if self._calibrationState=='corrected':

# get calibration data if required
        
                bpCalParameters = self._bpCal.calculate()
                gainCalParameters = self._gainCal.calculate()

                results['parameters']['dependencies']['bpCal'] = bpCalParameters
                results['parameters']['dependencies']['gainCal'] = gainCalParameters

# ensure ms flag state is 'Current' and save it for restoration after calculation
# of the view - calibration of the data can flag it implicitly and we want to avoid
# carrying forward 'hidden' flagging like that.
         
                self._msFlagger.setFlagState('Current')
                self._msFlagger.saveFlagState('BaselineEntry')

# loop through valid data_desc_id and field_id

            self._ms.open(thems=self._msName)

            for field_id in self._field_ids:
                for data_desc_id in self._data_desc_ids:
                    if self._valid_field_spw.count([field_id,data_desc_id]) == 0:
                        continue

                    results['parameters'][(field_id,data_desc_id)] = {}

# set the flag states we want to look at 

                    flagVersions = ['BeforeHeuristics', 'StageEntry', 'Current']

                    if self._calibrationState=='corrected':
                        try:

# apply the calibration, this may fail

                            self._bpCal.setapply(spw=data_desc_id,
                             field=field_id)
                            self._gainCal.setapply(spw=data_desc_id,
                             field=field_id)
                            commands = []
                            commands += self._msCalibrater.correct(
                             spw=data_desc_id, field=field_id)
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
                     antenna_range, self._calibrationState, flagVersions)

# declare arrays

                    ncorr = shape(corr_axis)[0]
                    if self._dataType == 'complex':
                        data = zeros([max(antenna_range)+1, ncorr, max(antenna_range)+1, len(times)], complex)
                    else:
                        data = zeros([max(antenna_range)+1, ncorr, max(antenna_range)+1, len(times)], float)
                    data_flag = []
                    for fi,fs in enumerate(flagVersions):
                        data_flag.append(ones([max(antenna_range)+1, ncorr, max(antenna_range)+1, len(times)], int))
                    no_data_flag = ones([max(antenna_range)+1, ncorr, max(antenna_range)+1, len(times)], int)

# loop through antennas and for each build up 'baseline v time data images'

                    for antenna in antenna_range:
                        ifr_range = arange(len(antenna1))
                        ifr_range = compress(antenna1==antenna, ifr_range)

                        for corr in range(ncorr):
                            for ifr in ifr_range:
                                other_ant = antenna2[ifr]

                                for t in range(len(times)):
                                    no_data_flag[antenna,corr,other_ant,t] = no_data_flag[other_ant,corr,antenna,t] = False
                                    for fi,fs in enumerate(flagVersions):
                                        if ifr_flag_row[fi][ifr,t]:
                                            continue
                                        if all(ifr_flag[fi][corr,:,ifr,t]):
                                            continue
                                        else:
                                            data_flag[fi][antenna,corr,other_ant,t] = data_flag[fi][other_ant,corr,antenna,t] = False

                                    if not data_flag[-1][antenna,corr,other_ant,t]:

# only get this far if 'Current' data of interest are not flagged.

                                        reals = compress(logical_not(ifr_flag[-1][corr,:,ifr,t]), ifr_real[corr,:,ifr,t])
                                        imags = compress(logical_not(ifr_flag[-1][corr,:,ifr,t]), ifr_imag[corr,:,ifr,t])

                                        median_real = median(reals)
                                        median_imag = median(imags)

                                        if self._dataType == 'real':
                                            data[antenna,corr,other_ant,t] = data[other_ant,corr,antenna,t] = median_real
                                        elif self._dataType == 'imaginary':
                                            data[antenna,corr,other_ant,t] = data[other_ant,corr,antenna,t] = median_imag
                                        elif self._dataType == 'amp':
                                            data[antenna,corr,other_ant,t] = data[other_ant,corr,antenna,t] = \
                                             sqrt(pow(median_real,2) + pow(median_imag,2))
                                        elif self._dataType == 'phase':
                                            data[antenna,corr,other_ant,t] = arctan2(median_imag, median_real) * 180.0 / math.pi
                                            data[other_ant,corr,antenna,t] = -data[antenna,corr,other_ant,t]
                                        elif self._dataType == 'complex':
                                            data[antenna,corr,other_ant,t] = complex(median_real, median_imag)
                                            data[other_ant,corr,antenna,t] = complex(median_real, -median_imag)
                                        else:
                                            raise NameError, 'bad dataType: %s' % self._dataType

# set units
 
                    if self._dataType == 'phase':
                        dataUnits = 'degrees'
                    else:
                        if self._calibrationState=='corrected':
                            dataUnits = 'calibrated'
                        else:
                            dataUnits = 'raw'

                    for antenna in range(max(antenna_range)+1):
                        for corr in range(ncorr):
                            description = {}
                            description['FIELD_ID'] = int(field_id)
                            description['FIELD_NAME'] = self._fieldName[int(field_id)]
                            description['FIELD_TYPE'] = self._fieldType[int(field_id)]
                            description['DATA_DESC_ID'] = int(data_desc_id)
                            description['POLARIZATION_ID'] = corr_axis[corr]
                            description['ANTENNA1'] = [int(antenna)]
                            description['TITLE'] = \
                             'Field:%s (%s) Spw:%s Pol:%s Ant1:%s %s %s baseline data' \
                             % (self._fieldName[field_id], 
                             self._fieldType[field_id], 
                             self._pad(data_desc_id), 
                             corr_axis[corr], self._pad(antenna),
                             self._dataType, self._calibrationState)

                            result = {}
                            result['dataType'] = '%s %s data' % (
                             self._dataType, self._calibrationState)
                            result['xtitle'] = 'ANTENNA2'
                            result['x'] = arange(max(antenna_range)+1) 
                            result['ytitle'] = 'TIME'
                            result['y'] = times
                            result['chunks'] = chunks
                            result['data'] = transpose(data[antenna,corr])
                            result['mad_floor'] =  zeros(shape(result['data']))
                            result['dataUnits'] = dataUnits
                            temp_flag = []
                            for fi,fs in enumerate(flagVersions):
                                temp_flag.append(transpose(data_flag[fi][antenna,corr]))
                            temp_flag.append(transpose(no_data_flag[antenna,corr]))
                            result['flag'] = temp_flag
                            flagVersionsStored = list(flagVersions)
                            flagVersionsStored.append('NoData')
                            result['flagVersions'] = flagVersionsStored

                            pickled_description = pickle.dumps(description)
                            results['data'][pickled_description] = result

            if self._calibrationState=='corrected':

# restore the flag state on entry and adopt is as 'Current'
         
                self._msFlagger.setFlagState('BaselineEntry')
                self._msFlagger.adoptAsCurrentFlagState()

            self._ms.close()

# store the object info in the BookKeeper

            self._bookKeeper.enter(
             objectType='BaselineData',
             sourceType=self._sourceType,
             furtherInput={'dataType':self._dataType,
                           'calibrationState':self._calibrationState},
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
                self._results['data'][k].append(results['data'][k])
            else:
                self._results['data'][k] = [results['data'][k]]

# copy history and dependency info

        self._results['parameters'] = results['parameters']

# return a copy of the data list, otherwise operating on it outside this class
# will corrupt it

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('BaselineData.getData')
        return temp


    def inputs(self):
        """Method to return the input settings for this object.
        """
        dependencies = []
        if self._calibrationState=='corrected':
            dependencies = [self._bpCal.inputs(), self._gainCal.inputs()]

        result = {}
        result['objectType'] = 'BaselineData'
        result['sourceType'] = self._sourceType
        result['furtherInput'] = {'dataType':self._dataType,
                                  'calibrationState':self._calibrationState}
        result['outputFiles'] = []
        result['dependencies'] = dependencies

        flag_marks = {}
        ignore,flag_mark_col = self._msFlagger.getFlagMarkInfo()
        for field_id in self._field_ids:
            flag_marks[field_id] = flag_mark_col[field_id]
        result['flag_marks'] = flag_marks
        
        return result


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        self._htmlLogger.logHTML("""
          <p>The data view is a list of 2-d arrays, one for each value
          of ANTENNA1 in the measurement set. Each array has axes 
          x:ANTENNA2 and y:TIME.""")

        if self._dataType == 'amp':
            self._htmlLogger.logHTML("""
             Each pixel is the amplitude of the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        elif self._dataType == 'phase':
            self._htmlLogger.logHTML("""
             Each pixel is the phase (in degrees) of the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        elif self._dataType == 'real':
            self._htmlLogger.logHTML("""
             Each pixel is the real part the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        elif self._dataType == 'imaginary':
            self._htmlLogger.logHTML("""
             Each pixel is the imaginary part of the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        elif self._dataType == 'complex':
            self._htmlLogger.logHTML("""
             Each pixel is the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        if self._calibrationState == 'raw':
            self._htmlLogger.logHTML("""
             <p>The data were not calibrated.""")

        elif self._calibrationState == 'corrected':
            self._htmlLogger.logHTML("""
             <p>The data were calibrated.""")


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a detailed description of the class to html.

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
        self._htmlLogger.logHTML("""
          <p>The data view is a list of 2-d arrays, one for each value
          of ANTENNA1 in the measurement set. Each array has axes 
          x:ANTENNA2 and y:TIME.""")

        if self._dataType == 'amp':
            self._htmlLogger.logHTML("""
             Each pixel is the amplitude of the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        elif self._dataType == 'phase':
            self._htmlLogger.logHTML("""
             Each pixel is the phase of the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        elif self._dataType == 'real':
            self._htmlLogger.logHTML("""
             Each pixel is the real part the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        elif self._dataType == 'imaginary':
            self._htmlLogger.logHTML("""
             Each pixel is the imaginary part of the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        elif self._dataType == 'complex':
            self._htmlLogger.logHTML("""
             Each pixel is the complex 'datum' for that 
             baseline/timestamp (averaged over spectral channels if there
             is more than one).""")

        if self._calibrationState == 'raw':
            self._htmlLogger.logHTML("""
             <p>The data were not calibrated.""")

        elif self._calibrationState == 'corrected':
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

#        self._htmlLogger.logHTML("""
#         <p>This data view was calculated by Python class %s.""" % 
#         self._className)

           
class BaselineCorrectedAmplitude(BaselineData):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, bandpassCal, gainCal,
     bandpassFlaggingStage=None):
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
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels to be ignored in calculating
                              -- integrated map.
        """
#        print 'BaselineCorrectedAmplitude constructor called: %s %s' % (
#         msName, sourceType)
        BaselineData.__init__(self, tools, bookKeeper,
         msCalibrater, msFlagger, htmlLogger, msName, stageName, sourceType, 
         calibrationState='corrected', dataType='amp',
         bandpassCal=bandpassCal, gainCal=gainCal,
         bandpassFlaggingStage=bandpassFlaggingStage)
        self._className = 'BaselineCorrectedAmplitude'


class BaselineCorrectedComplexData(BaselineData):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, bandpassCal, gainCal,
     bandpassFlaggingStage=None):
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
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels to be ignored in calculating
                              -- integrated map.
        """
#        print 'BaselineCorrectedComplexData constructor called: %s %s' % (
#         msName, sourceType)
        BaselineData.__init__(self, tools, bookKeeper,
         msCalibrater, msFlagger, htmlLogger, msName, stageName, sourceType, 
         calibrationState='corrected', dataType='complex',
         bandpassCal=bandpassCal, gainCal=gainCal,
         bandpassFlaggingStage=bandpassFlaggingStage)
        self._className = 'BaselineCorrectedComplexData'


class BaselineCorrectedPhase(BaselineData):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType, bandpassCal, gainCal,
     bandpassFlaggingStage=None):
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
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels to be ignored in calculating
                              -- integrated map.
        """
#        print 'BaselineCorrectedPhase constructor called: %s %s' % (
#         msName, sourceType)
        BaselineData.__init__(self, tools, bookKeeper,
         msCalibrater, msFlagger, htmlLogger, msName, stageName, sourceType, 
         calibrationState='corrected', dataType='phase',
         bandpassCal=bandpassCal, gainCal=gainCal,
         bandpassFlaggingStage=bandpassFlaggingStage)
        self._className = 'BaselineCorrectedPhase'
