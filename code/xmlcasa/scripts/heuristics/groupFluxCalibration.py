"""Module to supply the F calibration results."""

# History:
#  6-Nov-2007 jfl Best bandpass release.

# package modules

import copy as python_copy
import math
from numpy import *
import os
import pickle

# alma modules

from baseData import *
from groupGainCalibration import * 

class GroupFluxCalibration(GroupGainCalibration):
    """Class providing F calibration results.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, gainSourceType='*GAIN*', fluxSourceType='*FLUX*',
     bandpassCal=None, bandpassFlaggingStage=None, dataType='complex'):
        """Constructor.

        Keyword arguments:
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- The name of the stage using the object.
        gainSourceType      -- Type of source to be used for the G calibration.
        fluxSourceType      -- Type of source to be used for the F calibration.
        bandpassCal         -- ignored
        bandpassFlaggingStage -- ignored
        dataType            -- The type of data to be returned by getdata;
                               'amplitude', 'phase', 'complex' or 'SNR'.
        """

        GroupGainCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName,
         sourceTypes=[gainSourceType, fluxSourceType], bandpassCal=bandpassCal,
         bandpassFlaggingStage=bandpassFlaggingStage, dataType=dataType)
        self._gainSourceType = gainSourceType
        self._fluxSourceType = fluxSourceType
        self._dataType = dataType


    def setapply(self, spw, field):
        """Method to schedule the current calibration to be applied to the 
        specified spw and field.
        """
        print 'GroupFluxCalibration.setapply', spw, field

# locate the group_id asociated with this spw
# first is it in a continuum group?

        target_group = None
        spwmap = None
        for group_id in self._continuumGroups.keys():
            if self._continuumGroups[group_id].count(spw) > 0:
                target_group = group_id
                break

        if target_group == None:

# is it in a line group?

            for group_id in self._lineGroups.keys():
                if self._lineGroups[group_id].count(spw) > 0:
                    target_group = group_id

# in this case also need to specify spwmap

                    data_desc_ids = self._results['summary']['data_desc_range']
                    max_spw = max(data_desc_ids)
                    spwmap = zeros([max_spw+1], int)
                    spwmap[spw] = self._continuumGroups[group_id][0]
                    break

        print 'spw, group and spwmap', spw, target_group, spwmap

        if target_group != None:
            table = self._parameters['data'][target_group]['table']
            self._msCalibrater.setapply(type='GSPLINE', table=table,
             spwmap=spwmap)


    def calculate(self):
        """Calculate F from the MS using the calibrater tool. A different 
        file is output for each spw.
        """

        print 'GroupFluxCalibration.calculate called'
        self._htmlLogger.timing_start('GroupFluxCalibration.calculate')

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
             'data':{}, 'commands':{}, 'dependencies':{}}

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

            gainCalParameters = GroupGainCalibration.calculate(self)
            parameters['gainCalParameters'] = gainCalParameters

# calculate flux G for each spw separately

            for group_id,group_spw in enumerate(self._continuumGroups):
                parameters['data'][group_id] = {}
                commands = []

# now transfer the flux calibration from the FLUX fields to the GAIN fields -
# get names of gain calibrators.

                ftab = 'ftab.%s.continuum%s.fm%s' % (self._base_msName,
                 self._groupNames[group_id], flag_marks)
                tablein = gainCalParameters['data'][group_id]['table']

                try:
                    print 'not running fluxscale'
                    self._rmall(ftab)
#                    fluxd = self._msCalibrater.fluxscale(tablein=tablein,
#                     reference=flux_field_names, tableout=ftab, 
#                     transfer=gain_field_names, append=False)
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

                finally:
                    parameters['commands'][group_id] = commands

                print 'using gtab directly'
                parameters['data'][group_id]['table'] = tablein
                parameters['data'][group_id]['group_name'] = \
                 self._groupNames[group_id]
                parameters['data'][group_id]['group_spw'] = group_spw

# store the object info in the BookKeeper

            self._bookKeeper.enter(
             objectType=inputs['objectType'], sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             dependencies=inputs['dependencies'],
             outputFiles=[], outputParameters=parameters)
            self._htmlLogger.timing_stop('FluxCalibration.calculateNew')
             
        self._parameters = parameters

        self._htmlLogger.timing_stop('GroupFluxCalibration.calculate')
        return parameters


    def getData(self): 
        """Public method to return the F calibration valuees.
        """

#        print 'GroupFluxCalibration.getData called'
        self._htmlLogger.timing_start('GroupFluxCalibration.getdata')

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
            print 'key', k
            print 'current', gains['data'][k]
            print 'stage entry', stageEntryGains['data'][k]
            print 'original', originalGains['data'][k]
            description = {}
            description['CALIBRATION_GROUP_ID'] = int(k)
            description['CALIBRATION_GROUP_NAME'] = originalGains['data'][k]\
             ['group_name']
            description['CALIBRATION_GROUP_SPWS'] = originalGains['data'][k]\
             ['group_spw']
            description['TITLE'] = description['CALIBRATION_GROUP_NAME']
            result = {}
            result['dataType'] = 'Name of gain file'
            result['data'] = [originalGains['data'][k]['table']]
            result['data'].append(stageEntryGains['data'][k]['table'])
            result['data'].append(gains['data'][k]['table'])
            result['flagVersions'] = ['BeforeHeuristics', 'StageEntry',
             'Current']

            pickled_description = pickle.dumps(description)
            results[pickled_description] = result

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

        self._htmlLogger.timing_stop('GroupFluxCalibration.getdata')
        return temp


    def inputs(self):
        """Method to return the interface parameters of this object.
        """
        data_desc_ids = self._results['summary']['data_desc_range']

        result = {}
        result['objectType'] = 'GroupFluxCalibration'
        result['sourceType'] = [self._gainSourceType, self._fluxSourceType]
        result['furtherInput'] = {}

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
        result['dependencies'] = []

        return result


    def createGeneralHTMLDescription(self, stageName):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = {}

        fluxDescription = """
         The flux calibration was calculated:
         <ul>
          <li>The gains for both FLUX and GAIN calibraters were calculated."""

        fluxDescription += \
         GroupGainCalibration.createGeneralHTMLDescription(self, stageName)\
          ['gain calibration']

        fluxDescription += """
          <li>The flux scale was NOT transferred from FLUX calibrater to GAIN.
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
            return description

        fluxDescription = """
         The flux calibration was calculated:
         <ul>
          <li>The gains for both FLUX and GAIN calibraters were calculated."""

        fluxDescription += \
         GroupGainCalibration.createDetailedHTMLDescription(self, stageName,
         parameters=parameters['gainCalParameters'])\
         ['gain calibration']

        fluxDescription += """
          <li>The flux scale was NOT transferred from FLUX calibrater to GAIN.

#              <table CELLPADDING="5" BORDER="1"
#               <tr>
#                 <th>SpW</th>
#                 <th>Casapy Call</th>
#                 <th>Error?</th>
#               </tr>"""

# use &nbsp; to make empty cell look good

#        for k,entry in parameters['solve']['fluxscale'].iteritems():
#            fluxDescription += '<tr>'
#            fluxDescription += '<td>%s</td>' % k
#            fluxDescription += '<td>%s</td>' % entry
#            error = parameters['solve']['error'][k]
#            if error == '':
#                fluxDescription += '<td>&nbsp;</td>'
#            else:
#                fluxDescription += '<td>%s</td>' % error
#            fluxDescription += '</tr>'

#        fluxDescription += """
#              </table>
#         </ul>"""
##         <p>The flux calibration was calculated by Python class 
##         FluxCalibration."""

        description['gain calibration'] = fluxDescription

        if topLevel:

# table of mean gain coefficient amplitudes

            meanAmplitudes = {}
         
#            for k,entry in self._results['data'].iteritems():
#                desc = pickle.loads(k)
#                spw = desc ['DATA_DESC_ID']
#                meanAmplitudes[spw] = {'data':entry[-1]['meanAmplitude'],
#                 'flag':entry[-1]['meanAmplitudeFlag']}            

#            coefficients = """
#          <table CELLPADDING="5" BORDER="1"
#           <tr>
#             <th>Antenna</th>"""

#            keys = meanAmplitudes.keys()
#            keys.sort()
#            for spw in keys:
#               coefficients += """
#             <th>SpW %s</th>""" % spw

#            coefficients += """
#           </tr>"""

#            antenna = 0
#            looping = True
#            while looping:
#                looping = False
#                start = True
#                for spw in keys:
#                    if antenna < len(meanAmplitudes[spw]['data']):
#                        looping = True
#                        if start:
#                            coefficients += """
#           <tr>"""
#                            coefficients += """
#            <td>%s</td>""" % antenna
#                            start = False
#
#                        if meanAmplitudes[spw]['flag'][antenna] == 0:
#                            coefficients += """
#            <td>%.2e</td>""" % (meanAmplitudes[spw]['data'][antenna])
#                        else:
#                            coefficients += """
#            <td>-</td>"""

#                antenna += 1
#                coefficients += """
#           </tr>"""

#            coefficients += """
#          </table>"""

#            description['mean gain coefficients'] = coefficients

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

#            self._htmlLogger.logHTML("""
#             <h3>Data View</h3>
#                 %s The mean
#                 amplitudes (averaged over time) for each antenna are shown below:
#                 %s
#                 <p>The flux calibration was calculated as follows
#                 <ul>""" % (info, description['mean gain coefficients']))

            self._htmlLogger.logHTML("""
             <h3>Data View</h3>
                 The flux calibration was calculated as follows:
                 <ul>""")

            self._htmlLogger.logHTML("""
                  <li>""" + description['gain calibration'] + """
                 </ul>""")


class GroupFluxCalibrationAmplitude(GroupFluxCalibration):
    """Class to get F calibration amplitudes.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, gainSourceType='*GAIN*',
     fluxSourceType='*FLUX*'):
        """Constructor.

        Keyword arguments:
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- Name of stage using this object.
        gainSourceType      -- Type of source to be used for the G calibration.
        fluxSourceType      -- Type of source to be used for the F calibration.
        """

        GroupFluxCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, gainSourceType,
         fluxSourceType, dataType='amplitude')


class GroupFluxCalibrationPhase(GroupFluxCalibration):
    """Class to get F calibration phases.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, gainSourceType='*GAIN*',
     fluxSourceType='*FLUX*'):
        """Constructor.

        Keyword arguments:   
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- Name of stage uisng this object.
        gainSourceType      -- Type of source to be used for the G calibration.
        fluxSourceType      -- Type of source to be used for the F calibration.
        """
        GroupFluxCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, gainSourceType,
         fluxSourceType, dataType='phase')
