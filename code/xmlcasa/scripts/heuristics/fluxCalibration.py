"""Module to supply the F calibration results."""

# History:
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 17-Dec-2007 jfl Add calibration method info.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
#  1-May-2008 jfl selectvis removed in setApply. This reset solutions to be
#                 applied.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release.
# 25-Jun-2008 jfl regression release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import copy as python_copy
import math
from numpy import *
import os
import pickle

# alma modules

from baseData import *
from gainCalibration import * 

class FluxCalibration(GainCalibration):
    """Class providing F calibration results.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, gainSourceType='*GAIN*',
     fluxSourceType='*FLUX*', timesol=300.0, dataType='complex',
     bandpassCal=None, bandpassFlaggingStage=None):
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
        gainSourceType      -- Type of source to be used for the G calibration.
        fluxSourceType      -- Type of source to be used for the F calibration.
        timesol             -- 'timesol' parameter passed to calibrater.
        dataType            -- The type of data to be returned by getdata;
                               'amplitude', 'phase', 'complex' or 'SNR'.
        bandpassFlaggingStage -- Name of stage where bandpass edge channels
                                 were detected. If not None then these
                                 channels will be flagged in the FLUX and
                                 GAIN calibrator
                                 data before the gain calibrations are
                                 calculated. The initial flag state of the
                                 MS will be restored afterward.
        """

        GainCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, bandpassCal=bandpassCal,
         sourceTypes=[gainSourceType, fluxSourceType], timesol=timesol,
         bandpassFlaggingStage=bandpassFlaggingStage)
        self._gainSourceType = gainSourceType
        self._fluxSourceType = fluxSourceType
        self._timesol = timesol
        self._dataType = dataType


    def setapply(self, spw, field):
        """Method to schedule the current calibration to be applied to the 
        specified spw and field.
        """
        table = self._parameters['data'][spw]['table']
        self._msCalibrater.setapply(type='G', table=table)


    def calculate(self):
        """Calculate F from the MS using the calibrater tool. A different 
        file is output for each spw.
        """

#        print 'FluxCalibration.calculate called'
        self._htmlLogger.timing_start('FluxCalibration.calculate')

        data_desc_ids = self._results['summary']['data_desc_range']

# are the data already available

        inputs = self.inputs()
        flag_marks = inputs['flag_marks']

        entryID,parameters = self._bookKeeper.available(
         objectType=inputs['objectType'], sourceType=inputs['sourceType'],
         furtherInput=inputs['furtherInput'],
         dependencies=inputs['dependencies'], outputFiles=[])

        if entryID == None:
            self._htmlLogger.timing_start('FluxCalibration.calculateNew')
            parameters = {'history':self._fullStageName,
             'data':{}, 'solve':{}, 'dependencies':{}}

# get ids and names of flux and gain calibrators

            flux_field_ids = self.getFieldsOfType(self._fluxSourceType)
            gain_field_ids = self.getFieldsOfType(self._gainSourceType)

            field_names = self._results['summary']['field_names']
            flux_field_names = []
            for field in flux_field_ids:
                flux_field_names.append(field_names[field])
            gain_field_names = []
            for field in gain_field_ids:
                gain_field_names.append(field_names[field])

# calculate the gains for GAIN and FLUX sources into one file

            gainCalParameters = GainCalibration.calculate(self)
            parameters['solve']['gainCalParameters'] = gainCalParameters.copy()

# calculate flux G for each spw separately

            parameters['solve']['fluxscale'] = {}
            parameters['solve']['error'] = {}
            valid_field_spw = self._msFlagger.getValidFieldSpw()

            for data_desc_id in data_desc_ids:
                parameters['data'][data_desc_id] = {}

# sometimes get situation where not all spw are observed in all FLUX and GAIN
# fields. Cut names down to ones that do apply to this spw.

                spw_gain_field_ids = []
                spw_flux_field_ids = []
                spw_gain_field_names = []
                spw_flux_field_names = []

                for field in gain_field_ids:
                    if valid_field_spw.count([field,data_desc_id]) > 0:
                        spw_gain_field_ids.append(field)
                        spw_gain_field_names.append(field_names[field])

                for field in flux_field_ids:
                    if valid_field_spw.count([field,data_desc_id]) > 0:
                        spw_flux_field_ids.append(field)
                        spw_flux_field_names.append(field_names[field])

# now transfer the flux calibration from the FLUX fields to the GAIN fields -
# get names of gain calibrators.

                ftab = 'ftab.%s.spw%s.fm%s' % (self._base_msName, data_desc_id,
                 flag_marks)
                tablein = gainCalParameters['data'][data_desc_id]['table']

                parameters['solve']['fluxscale'][data_desc_id] = \
                 '''fluxscale(tablein=%s, reference=%s, tableout=%s,
                 transfer=%s, append=False)''' % (
                 gainCalParameters['data'][data_desc_id]
                 ['table'], spw_flux_field_names, ftab, spw_gain_field_names)
                parameters['solve']['error'][data_desc_id] = ''

                try:
                    self._rmall(ftab)
                    fluxd = self._msCalibrater.fluxscale(tablein=tablein,
                     reference=spw_flux_field_names, tableout=ftab, 
                     transfer=spw_gain_field_names, append=False)
                except KeyboardInterrupt:
                    raise
                except:
                    error_report = self._htmlLogger.openNode('exception',
                     '%s.flux_transfer_exception' % (self._stageName), True,
                     stringOutput = True)

                    self._htmlLogger.logHTML('Exception details<pre>')
                    traceback.print_exc()
                    traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                    self._htmlLogger.logHTML('</pre>')
                    self._htmlLogger.closeNode()

                    error_report += 'during flux calibration transfer'

                    parameters['solve']['error'][data_desc_id] = \
                     error_report

                parameters['data'][data_desc_id]['table'] = ftab

# store the object info in the BookKeeper

            self._bookKeeper.enter(
             objectType=inputs['objectType'], sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             dependencies=inputs['dependencies'],
             outputFiles=[], outputParameters=parameters)
            self._htmlLogger.timing_stop('FluxCalibration.calculateNew')
             
        self._parameters = parameters

        self._htmlLogger.timing_stop('FluxCalibration.calculate')
        return parameters


    def getData(self): 
        """Public method to return the F calibration valuees.
        """

#        print 'FluxCalibration.getData called'
        self._htmlLogger.timing_start('FluxCalibration.getdata')

# calculate the gains for flag states 'BeforeHeuristics', 'StageEntry' and
# 'Current'

        self._msFlagger.setFlagState('BeforeHeuristics')
        originalGains = self.calculate()
        self._msFlagger.setFlagState('StageEntry')
        stageEntryGains = self.calculate()
        self._msFlagger.setFlagState('Current')
        gains = self.calculate()

# now read the results from the tables and export them

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

# copy history and dependency info

        self._results['parameters'] = gains
        self._results['parameters']['dependencies']['originalGain'] = \
         originalGains

        for k in results.keys():
            if self._results['data'].has_key(k):
                self._results['data'][k].append(results[k])
            else:
                self._results['data'][k] = [results[k]]

        temp = python_copy.deepcopy(self._results)

        self._htmlLogger.timing_stop('FluxCalibration.getdata')
        return temp


    def inputs(self):
        """Method to return the interface parameters of this object.
        """
        data_desc_ids = self._results['summary']['data_desc_range']

        result = {}
        result['objectType'] = 'FluxCalibration'
        result['sourceType'] = [self._gainSourceType, self._fluxSourceType]
        result['furtherInput'] = {'timesol':self._timesol}

        flag_marks = {}
        ignore,flag_mark_col = self._msFlagger.getFlagMarkInfo()
        gain_field_ids = self.getFieldsOfType(self._gainSourceType)
        for field_id in gain_field_ids:
            flag_marks[field_id] = flag_mark_col[field_id]
        flux_field_ids = self.getFieldsOfType(self._fluxSourceType)
        for field_id in flux_field_ids:
            flag_marks[field_id] = flag_mark_col[field_id]

        flag_marks = str(flag_marks)

# replace unusual symbols to avoid problems with TaQL

        flag_marks = flag_marks.replace(' ', '')
        flag_marks = flag_marks.replace('{', '')
        flag_marks = flag_marks.replace('}', '')
        flag_marks = flag_marks.replace(':', '-')
        flag_marks = flag_marks.replace(',', '.')

        result['flag_marks'] = flag_marks
        result['outputFiles'] = []
        result['dependencies'] = [self._bpCal.inputs()]

        return result


    def createGeneralHTMLDescription(self, stageName):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = {}

        description['bandpass calibration'] = \
         self._bpCal.createGeneralHTMLDescription(stageName)\
         ['bandpass calibration']

        fluxDescription = """
         The flux calibration was calculated:
         <ul>
          <li>The gains for both FLUX and GAIN calibraters were calculated."""

        fluxDescription += \
         GainCalibration.createGeneralHTMLDescription(self, stageName)\
          ['gain calibration']

        fluxDescription += """
          <li>The flux scale was transferred from FLUX calibrater to GAIN.
         </ul>"""

        description['gain calibration'] = fluxDescription
        return description
 

    def createDetailedHTMLDescription(self, stageName, topLevel=False,
     parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        topLevel   -- True if this data 'view' is to be displayed directly,
                      not passing through a data modifier object.
        parameters -- The dictionary that holds the descriptive information.
        """
        if parameters == None:
            parameters = self._parameters

        description = {}

        if parameters.has_key('separate node'):
            description['gain calibration'] = parameters['separate node']
            description['bandpass calibration'] = 'no bandpass description'
            return description

        description['bandpass calibration'] = \
         self._bpCal.createDetailedHTMLDescription(stageName, parameters=
         parameters['solve']['gainCalParameters']['dependencies']['bpCal'])\
         ['bandpass calibration']

        fluxDescription = """
         The flux calibration was calculated:
         <ul>
          <li>The gains for both FLUX and GAIN calibraters were calculated."""

        fluxDescription += \
         GainCalibration.createDetailedHTMLDescription(self, stageName,
         parameters=parameters['solve']['gainCalParameters'])\
         ['gain calibration']

        fluxDescription += """
          <li>The flux scale was transferred from FLUX calibrater to GAIN.

              <table CELLPADDING="5" BORDER="1"
               <tr>
                 <th>SpW</th>
                 <th>Casapy Call</th>
                 <th>Error?</th>
               </tr>"""

# use &nbsp; to make empty cell look good

        for k,entry in parameters['solve']['fluxscale'].iteritems():
            fluxDescription += '<tr>'
            fluxDescription += '<td>%s</td>' % k
            fluxDescription += '<td>%s</td>' % entry
            error = parameters['solve']['error'][k]
            if error == '':
                fluxDescription += '<td>&nbsp;</td>'
            else:
                fluxDescription += '<td>%s</td>' % error
            fluxDescription += '</tr>'

        fluxDescription += """
              </table>
         </ul>"""
#         <p>The flux calibration was calculated by Python class 
#         FluxCalibration."""

        description['gain calibration'] = fluxDescription

        if topLevel:

# table of mean gain coefficient amplitudes for the first GAIN
# calibrator found in the results

            meanAmplitudes = {}
         
            gain_fields = self.getFieldsOfType('*GAIN*')

            for k,entry in self._results['data'].iteritems():
                desc = pickle.loads(k)
                spw = desc['DATA_DESC_ID']
                pol = desc['POLARIZATION_ID']
                if not meanAmplitudes.has_key((spw,pol)):
                    field_id = desc['FIELD_ID']
                    if gain_fields.count(field_id) > 0:
                        meanAmplitudes[(spw,pol)] = {
                         'data':entry[-1]['meanAmplitude'],
                         'flag':entry[-1]['meanAmplitudeFlag']}            

            coefficients = """
          <table CELLPADDING="5" BORDER="1"
           <tr>
             <th>Antenna</th>"""

            keys = meanAmplitudes.keys()
            keys.sort()
            for spw_pol in keys:
               coefficients += """
             <th>SpW %s Pol %s</th>""" % (spw_pol[0], spw_pol[1])

            coefficients += """
           </tr>"""

            antenna = 0
            looping = True
            while looping:
                looping = False
                start = True
                for spw_pol in keys:
                    if antenna < len(meanAmplitudes[spw_pol]['data']):
                        looping = True
                        if start:
                            coefficients += """
           <tr>"""
                            coefficients += """
            <td>%s</td>""" % antenna
                            start = False

                        if meanAmplitudes[spw_pol]['flag'][antenna] == 0:
                            coefficients += """
            <td>%.2e</td>""" % (meanAmplitudes[spw_pol]['data'][antenna])
                        else:
                            coefficients += """
            <td>-</td>"""

                antenna += 1
                coefficients += """
           </tr>"""

            coefficients += """
          </table>"""

            description['mean gain coefficients'] = coefficients

        return description
 

    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = self.createGeneralHTMLDescription(stageName)

        if self._dataType == 'complex':
            info = """<p>The data view shows the complex coefficients from a 
             flux calibration."""
        else:
            info = """<p>The data view shows the %ss of the coefficients from
             a flux calibration.""" % self._dataType

        self._htmlLogger.logHTML(info)


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        topLevel   -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        parameters -- The dictionary that holds the descriptive information.

        """

#        print 'writeDetailedHTMLDescription', topLevel, parameters

        if parameters == None:
            parameters = self._parameters

        description = self.createDetailedHTMLDescription(stageName,
         topLevel=topLevel, parameters=parameters)

        if description.has_key('separate node'):
            self._htmlLogger.logHTML(description['separate node'])
        else:

            if self._dataType == 'complex':
                info = """<p>The data view shows the complex coefficients from a 
                 flux calibration."""
            else:
                info = """<p>The data view shows the %ss of the coefficients from
                 a flux calibration.""" % self._dataType

            self._htmlLogger.logHTML("""
             <h3>Data View</h3>
                 %s The mean
                 amplitudes (averaged over time) of the flux calibrated
                 gain coefficients
                 derived from the GAIN calibrator for each antenna are shown
                 below:
                 %s
                 <p>The flux calibration was calculated as follows
                 <ul>""" % (info, description['mean gain coefficients']))

            self._htmlLogger.logHTML("""
                  <li>""" + description['bandpass calibration'] + """
                  <li>""" + description['gain calibration'] + """
                 </ul>""")


class FluxCalibrationAmplitude(FluxCalibration):
    """Class to get F calibration amplitudes.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, bandpassCal, gainSourceType='*GAIN*',
     fluxSourceType='*FLUX*', timesol=300.0, bandpassFlaggingStage=None):
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
        gainSourceType      -- Type of source to be used for the G calibration.
        fluxSourceType      -- Type of source to be used for the F calibration.
        timesol             -- 'timesol' parameter passed to calibrater.
        bandpassFlaggingStage -- Name of stage where bandpass edge channels
                                 were detected. If not None then these
                                 channels will be flagged in the FLUX and
                                 GAIN calibrator
                                 data before the gain calibrations are
                                 calculated. The initial flag state of the
                                 MS will be restored afterward.
        """

        FluxCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, bandpassCal, gainSourceType,
         fluxSourceType, timesol, dataType='amplitude', bandpassFlaggingStage=
         bandpassFlaggingStage)


class FluxCalibrationPhase(FluxCalibration):
    """Class to get F calibration phases.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, bandpassCal, gainSourceType='*GAIN*',
     fluxSourceType='*FLUX*', timesol=300.0, bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:   
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- Name of stage uisng this object.
        bandpassCal         -- class to be used for the bandpass calibration.
        gainSourceType      -- Type of source to be used for the G calibration.
        fluxSourceType      -- Type of source to be used for the F calibration.
        timesol             -- 'timesol' parameter passed to calibrater.
        bandpassFlaggingStage -- Name of stage where bandpass edge channels
                                 were detected. If not None then these
                                 channels will be flagged in the FLUX and
                                 GAIN calibrator
                                 data before the gain calibrations are
                                 calculated. The initial flag state of the
                                 MS will be restored afterward.
        """
        FluxCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, bandpassCal, gainSourceType,
         fluxSourceType, timesol, dataType='phase', bandpassFlaggingStage=
         bandpassFlaggingStage)
