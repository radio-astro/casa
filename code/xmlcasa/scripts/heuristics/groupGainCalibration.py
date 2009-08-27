"""Module to supply G calibration results."""

# History:

# package modules

import copy as python_copy
import math
from numpy import *
import os
import pickle

# alma modules

from gainCalibration import *


class GroupGainCalibration(GainCalibration):
    """Class providing G calibration results for groups of spw sharing an IF chain.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceTypes=['GAIN'], bandpassCal=None, 
     bandpassFlaggingStage=None, dataType='complex'):
        """Constructor.

        Keyword arguments:
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- The name of the stage using the object.
        sourceTypes         -- Types of source to be used for the G calibration.
        bandpassCal         -- ignored
        bandpassFlaggingStage -- ignored
        dataType            -- The type of data to be returned by getdata.
                               'amplitude', 'phase', 'complex' or 'SNR'.
        """

        BaseData.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName)

        self._verbose = True

        self._sourceTypes = sourceTypes
        self._dataType = dataType

        self._continuumGroups, self._lineGroups, self._groupNames = \
         self._getBands()


    def _getBands(self):
        """Utility method to find SpW groups.
        """

# find spw that belong in the same FREQ_GROUP. These are associated for
# calibration purposes.

        self._table.open('%s/SPECTRAL_WINDOW' % self._msName)
        groups = self._table.getcol('FREQ_GROUP')
        freq_group_names = self._table.getcol('FREQ_GROUP_NAME')
        nchan = self._table.getcol('NUM_CHAN')
        self._table.close()

# construct dictionaries containing continuum and line spw for each
# group

        continuum_groups = {}
        line_groups = {}
        group_names = {}
        for spw,group in enumerate(groups):
            if not continuum_groups.has_key(group):
                continuum_groups[group] = []
                line_groups[group] = []
                group_names[group] = freq_group_names[spw]
            if nchan[spw]==1:
                continuum_groups[group].append(spw)
            elif nchan[spw]>1:
                line_groups[group].append(spw)

        if self._verbose:
            print 'continuum groups', continuum_groups
            print 'line groups' , line_groups
            print 'group names', group_names

        return continuum_groups, line_groups, group_names


    def calculate(self, input_flag_marks=None):
        """Calculate G from the MS using the calibrater tool. A different
        file is output for each spw group.

        Keyword Arguments:
        input_flag_marks -- Value of flag marks to use when building 
                            a gain table for more than 1 source, e.g. for
                            GAIN and FLUX calibrators.
        """

        print 'GroupGainCalibration.calculate called: %s' % self._sourceTypes

        self._htmlLogger.timing_start('GroupGainCalibration.calculate')
   
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
            if self._verbose:
                print 'GroupGainCalibration calculating new gains'

            parameters = {'history':self._fullStageName,
             'data':{}, 
             'command':{},
             'dependencies':{}}

            field_ids = []
            field_names = self._results['summary']['field_names']

            commands = {}

            self._imager.open(thems=self._msName, compress=False)

            for group_id in self._continuumGroups.keys():
                commands[group_id] = ["""imager.open(thems='%s', compress=False)"""
                 % self._msName]
     
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

                            for group_id in self._continuumGroups.keys():
                                for spw in self._continuumGroups[group_id]:
                                    flux = 0.95 * (ref_frequency[spw] / 87e9)**0.6

                                    new_commands = self._msCalibrater.setjy(
                                     field=field, spw=spw,
                                     fluxdensity=[flux, 0.0, 0.0, 0.0],
                                     standard='CLIC')

                                    commands[group_id] += new_commands

                        elif field_names[field] == '2200+420':
                            for group_id in self._continuumGroups.keys():
                                for spw in self._continuumGroups[group_id]:

                                    new_commands = self._msCalibrater.setjy(
                                     field=field, spw=spw,
                                     fluxdensity=[3.0, 0.0, 0.0, 0.0],
                                     standard='MYGUESS')

                                    commands[group_id] += new_commands

                        elif field_names[field] == 'CRL618':
                            for group_id in self._continuumGroups.keys():
                                for spw in self._continuumGroups[group_id]:

                                    new_commands = self._msCalibrater.setjy(
                                     field=field, spw=spw,
                                     fluxdensity=[1.55, 0.0, 0.0, 0.0], standard='CLIC')

                                    commands[group_id] += new_commands
                        else:

# otherwise force the tool to read the data from the catalogue (VLA)

                            for group_id in self._continuumGroups.keys():
                                for spw in self._continuumGroups[group_id]:

                                    new_commands = self._msCalibrater.setjy(
                                     field=field, spw=spw,
                                     fluxdensity=[-1.0, 0.0, 0.0, 0.0],
                                     standard='Perley-Taylor99')

                                    commands[group_id] += new_commands

                else:

# GAIN fields to 1Jy, and others e.g. for measuring TARGET claibration per 
# timestamp
  
                    for field in new_field_ids:
                        for group_id in self._continuumGroups.keys():
                            for spw in self._continuumGroups[group_id]:

                                new_commands = self._msCalibrater.setjy(
                                 field=field, spw=spw,
                                 fluxdensity=[1.0, 0.0, 0.0, 0.0],
                                 standard='Perley-Taylor99')

                                commands[group_id] += new_commands

                print 'checking cal setup'

# make sure CORRECTED_DATA columns are set to same as DATA - managed without
# doing this for a long time so perhaps it is being done somewhere else too.

                self._table.open(self._msName, nomodify=False)

                for field in new_field_ids:
                    for group_id in self._continuumGroups.keys():
                        for spw in self._continuumGroups[group_id]:
                            s = self._table.query('FIELD_ID==%s && DATA_DESC_ID==%s' % 
                             (field, spw))
                            data_col = s.getcol('DATA')
                            corrected_data_col = s.getcol('CORRECTED_DATA')
                            if not all(abs(data_col - corrected_data_col) < 1.0e-7):
                                print 'calibrator discrepency fixed', field, spw

                self._table.close()
            self._imager.close()

# get the current flagging info 

            flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()

# calculate G for each continuum spw group separately

            try:

                for group_id in self._continuumGroups.keys():
                    parameters['data'][group_id] = {}

# calculate the gains

                    gtab = '%s.gtab.continuum%s.fm%s' % (self._base_msName,
                     self._groupNames[group_id], flag_marks)
                    if input_flag_marks != None:
                        append = True
                    else:
                        append = False

                    try:
                        new_commands = self._msCalibrater.groupSolve(
                         field=field_ids, 
                         spw=self._continuumGroups[group_id], table=gtab,
                         append=append, splinetime=3600.0,
                         preavg=0.0, npointaver=3, phasewrap=180.0)

                        commands[group_id] += new_commands

                    except KeyboardInterrupt:
                        raise
                    except:
                        parameters['data'][group_id]['error'] = \
                         'gain calibration failed' 
                        print 'exception', sys.exc_info()

                    parameters['data'][group_id]['table'] = gtab 
                    parameters['data'][group_id]['type'] = 'GSPLINE'
            finally:

# nothing required for now

                pass

            parameters['command'] = commands

# store the object info in the BookKeeper

            if self._verbose:
                for line in commands[0]:
                    print line

            self._bookKeeper.enter(
             objectType=inputs['objectType'], sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             dependencies=inputs['dependencies'], outputFiles=[],
             outputParameters=parameters)

        self._parameters = parameters
        self._htmlLogger.timing_stop('GroupGainCalibration.calculate')

        return parameters


    def description(self):
        description = [
         'GroupGainCalibration - calculate the gain solution for a group of continuum spw']
        return description


    def getData(self):
        """Public method to return the G calibration as a 'view' of the data.
        """

#        print 'GroupGainCalibration.getData called: %s ' % (self._dataType)
        self._htmlLogger.timing_start('GroupGainCalibration.getdata')

# calculate the G gains for 'BeforeHeuristics', 'StageEntry' and 'Current' - 

        self._msFlagger.setFlagState('BeforeHeuristics')
        originalGains = self.calculate()
        self._msFlagger.setFlagState('StageEntry')
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

        self._htmlLogger.timing_stop('GroupGainCalibration.getdata')
        return temp


    def inputs (self):
        """
        """
        data_desc_ids = self._results['summary']['data_desc_range']

        result = {}
        result['objectType'] = 'GroupGainCalibration'
        result['sourceType'] = self._sourceTypes
        result['furtherInput'] = {}

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
        result['dependencies'] = []

        return result


    def setapply(self, spw):
        """Method to schedule the current calibration to be applied to the
        specified spw.
        """
        self.msCalibrater.setapply(type='GSPLINE', 
         table=self._parameters['data'][spw]['table'])


    def createGeneralHTMLDescription(self, stageName):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = {}

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

        gainDescription = """
         The method was as follows (see Casapy Calls for details):
        <ul>     
         <li>The flux values of the calibrator(s) were set according to their 
          type.
         <li>The gains were calculated.

         <li>
          <table CELLPADDING="5" BORDER="1"
           <tr>
            <th>Group ID</th>
            <th>Group Name</th>
            <th>SpWs</th>
            <th>Apply Table</th>
            <th>Apply Type</th>
            <th>Casapy Calls</th>
            <th>Error?</th>
           </tr>"""

        for i,entry in parameters['data'].iteritems():
            print 'entry', entry.keys()
            gainDescription += "<tr><td>%s</td>" % i
            gainDescription += "<td>%s</td>" % self._groupNames[i]
            gainDescription += "<td>%s</td>" % self._continuumGroups[i]
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
                gainDescription += '<td>&nbsp;</td>'
            gainDescription += '</tr>'

        gainDescription += """
          </table>
         where:
          <ul>
           <li>'Apply Table' is the name of the file containing the gain
                solution.
           <li>'Apply Type' gives the casapy type of the gain solution.
          </ul>
        </ul>"""
#        <p>The gain was calculated by Python class GainCalibration."""

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
              <li>""" + description['gain calibration'] + """
             </ul>""")


class GroupGainCalibrationSNR(GroupGainCalibration):
    """Class to get G calibration signal to noise ratio.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, sourceType='GAIN'):
        """Constructor.

        Keyword arguments:
        tools               -- BaseTools object.
        bookKeeper          -- BookKeeper object.
        msCalibrater        -- MSCalibrater object.
        msFlagger           -- MSFlagger object.
        htmlLogger          -- Route for logging to html structure.
        msName              -- Name of MeasurementSet
        stageName           -- Name of stage using this object.
        sourceType          -- Type of source to be used for the G calibration.
        """
        GainCalibration.__init__(self, tools, bookKeeper, msCalibrater, 
         msFlagger, htmlLogger, msName, stageName, [sourceType],
         dataType='SNR')


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

