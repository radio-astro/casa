# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Numerous bug fixes.
# 30-Aug-2007 jfl Flux calibrating release.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 14-Jan-2008 jfl getFieldsOfType checks source_type is valid.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release.
# 25-Jun-2008 jfl regression release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 26-Sep-2008 jfl mosaic release.
# 10-Oct-2008 jfl complex display release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.
# 31-Jul-2009 jfl no maxPixels release, more efficient.

# package modules

import copy as python_copy
import fnmatch
from numpy import *
import os
import pickle
import re
import sys
import traceback
import types

# alma modules

import util

class BaseData:

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, description=None, verbose=False):
        """Constructor.

        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- Name of stage using this object.
        description -- Comment to be written out to html as a preface to
                       the automatic description.
        verbose     -- True if you want some diagnostic blurb.
        """
#        print 'BaseData constructor called: %s' % msName
        tools.copy(self)
        self._bookKeeper = bookKeeper
        self._msCalibrater = msCalibrater
        self._msFlagger = msFlagger
        self._htmlLogger = htmlLogger
        self._htmlLogger.timing_start('BaseData.__init__')
        self._msName = msName
        self._base_msName = os.path.basename(msName)
        self._fullStageName = stageName
        self._stageName = stageName.replace(' ','')
        self._description = description
        self._verbose = verbose
 
        self._ms.open(self._msName)
        msInfo = self._ms.range(['data_desc_id', 'field_id'])
        self._data_desc_ids = msInfo['data_desc_id']
        self._all_field_ids = msInfo['field_id']
        self._ms.close()

# get names, types of fields

        self._table.open(self._msName + '/FIELD')
        self._fieldName = self._table.getcol('NAME')
        self._fieldType = util.util.get_source_types(self._table)
        self._table.close()

# get some 'clean' field names, without chars that might cause trouble in
# filenames

        self._cleanFieldName = []
        for fieldName in self._fieldName:
            cleanFieldName = fieldName.replace('-','_')
            cleanFieldName = cleanFieldName.replace('+','_')
            cleanFieldName = cleanFieldName.replace('(','_')
            cleanFieldName = cleanFieldName.replace(')','_')
            cleanFieldName = cleanFieldName.replace(' ','_')
            self._cleanFieldName.append(cleanFieldName)

# list of valid field/spw combinations

        self._valid_field_spw = self._msFlagger.getValidFieldSpw()

# list of antennas present in each valid_field_spw combination

        self._antenna_dictionary = {}
        self._table.open(self._msName)
        for field_spw in self._valid_field_spw:
            field = field_spw[0]
            spw = field_spw[1]
            subTable = self._table.query('FIELD_ID==%s && DATA_DESC_ID==%s' % (
             field, spw))
            antenna1 = subTable.getcol('ANTENNA1')
            antenna2 = subTable.getcol('ANTENNA2') 
            antennas_used = []
            for ant in antenna1:
                if antennas_used.count(ant) == 0:
                    antennas_used.append(ant)
            for ant in antenna2:
                if antennas_used.count(ant) == 0:
                    antennas_used.append(ant)
            antennas_used.sort()
            self._antenna_dictionary[(field,spw)] = list(antennas_used)
        self._table.close()

# basic data structure

        self._results = {}
        self._getSummary()
        self._results['data'] = {}
        self._results['flagging'] = []
        self._results['flaggingReason'] = []
        self._results['flaggingApplied'] = []
        self._htmlLogger.timing_stop('BaseData.__init__')


    def _findChunks(self, times):
        """Return a list of arrays, each containing the indeces of a chunk
        of data i.e. a sequence of equally spaced measurements separated
        from other chunks by larger time gaps.

        Keyword arguments:
        times -- Numeric array of times at which the measurements
                 were taken.
        """
        difference = times[1:] - times[:-1]
        median_diff = median(difference)

        chunks = []
        chunk = [0]
        for i in arange(len(difference)):
            if difference[i] < 1.5 * median_diff:
                chunk.append(i+1)
            else:
                chunks.append(array(chunk))
                chunk = [i+1]
        chunks.append(array(chunk))
        return chunks


    def _formatList(self, v, format):
        """
        the list v is converted to a string, with each number in the specified
        format.
        """
        result ='['
        empty = True
        for item in v:
            empty = False
            result += ' ' + (format % item) +','
        
        if empty:
            result += ']'
        else:
            result = result[:-1] + ']'

        return result


    def _getAntennaRange(self):
        """Get the range of antennas in this dataset.
        """

        self._ms.selectinit(reset=True)
        msInfo = self._ms.range(['antenna1', 'antenna2']) 
        antenna1 = list(msInfo['antenna1'])
        antenna2 = list(msInfo['antenna2'])

# merge the antenna1 and antenna2 lists

        antenna_range = antenna1
        for antenna in antenna_range:
            if antenna2.count(antenna):
                antenna2.remove(antenna)
        antenna_range += antenna2
        antenna_range.sort()

        return antenna_range


    def _getBaselineData(self, field_id, data_desc_id, antenna_range, dataType,
     flagVersions, flagsOnly=False):
        """Get data.
        """

#        print 'getBaselineData'
        self._ms.selectinit(reset=True)
        self._ms.selectinit(datadescid = data_desc_id)

# select subset of MS for this field 

        query = "FIELD_ID==%s" % (field_id)
        rtn = self._ms.selecttaql(msselect = query)
        if self._ms.nrow(selected = True) > 0:
            if not flagsOnly:
                if dataType == 'raw':
                    msInfo = self._ms.getdata(['real','imaginary'],
                     ifraxis=True)
                    ifr_real = msInfo['real']
                    ifr_imag = msInfo['imaginary']
                elif dataType == 'corrected':
                    msInfo = self._ms.getdata(['corrected_real',
                     'corrected_imaginary'], ifraxis=True)
                    ifr_real = msInfo['corrected_real']
                    ifr_imag = msInfo['corrected_imaginary']
                elif dataType == 'model':
                    msInfo = self._ms.getdata(['model_real',
                     'model_imaginary'], ifraxis=True)
                    ifr_real = msInfo['model_real']
                    ifr_imag = msInfo['model_imaginary']
                else:
                    raise NameError, 'bad dataType: %s' % dataType

# get ancillary data

                msInfo = self._ms.getdata(['axis_info', 'antenna1',
                 'antenna2', 'scan_number'], ifraxis=True)
                antenna1 = msInfo['antenna1']
                antenna2 = msInfo['antenna2']
                corr_axis = msInfo['axis_info']['corr_axis']
                times = msInfo['axis_info']['time_axis']['MJDseconds']
                chan_freq = msInfo['axis_info']['freq_axis']['chan_freq']
                scan = msInfo['scan_number']

# identify chunks

                chunks = self._findChunks(times)

# use flagger tool to restore the demand flag states, then get the flags.

            ifr_flag = []
            ifr_flag_row = []

            for version in flagVersions:
                self._msFlagger.setFlagState(version)
                msInfo = self._ms.getdata(['flag', 'flag_row'], ifraxis=True)
                ifr_flag.append(msInfo['flag'])
                ifr_flag_row.append(msInfo['flag_row'])

## now put the flags back as they were

#            self._msFlagger.setFlagState('Current')

        else:
            if not flagsOnly:
                corr_axis = array(int)
                chan_freq = array(float)
                times = array(float)
                chunks = array(int)
                antenna1 = array(int)
                antenna2 = array(int)
                ifr_real = array(float)
                ifr_imag = array(float)

            ifr_flag = [array(bool)]
            ifr_flag_row = [array(bool)]

        if flagsOnly:
            return ifr_flag, ifr_flag_row
        else:
            return corr_axis, chan_freq, times, chunks, antenna1, antenna2, \
             ifr_real, ifr_imag, ifr_flag, ifr_flag_row

 
    def _getSummary(self):
        """
        """
        self._results['summary'] = {}

        self._ms.open(self._msName)
        antenna_range = self._getAntennaRange()
        self._results['summary']['antenna_range'] = antenna_range

        self._results['summary']['data_desc_range'] = self._data_desc_ids

        self._results['summary']['corr_axis'] = {}
        self._results['summary']['nchannels'] = {}
        self._results['summary']['chan_freq'] = {}
        for data_desc_id in self._data_desc_ids:
            self._ms.selectinit(data_desc_id)
            msInfo = self._ms.getdata(['axis_info'])
            self._results['summary']['corr_axis'][data_desc_id] = \
             msInfo['axis_info']['corr_axis']
            self._results['summary']['nchannels'][data_desc_id] = len(
             msInfo['axis_info']['freq_axis']['chan_freq'])
            self._results['summary']['chan_freq'][data_desc_id] = \
             msInfo['axis_info']['freq_axis']['chan_freq']
        self._ms.close()

        self._table.open(self._msName + '/OBSERVATION')
        telescope_name =  self._table.getcol('TELESCOPE_NAME')[0]
        self._table.close()
        self._results['summary']['telescope_name'] = telescope_name

# general field info
           
        self._table.open('%s/FIELD' % self._msName)
        fieldNames = self._table.getcol('NAME')
        fieldTypes = util.util.get_source_types(self._table)
        self._table.close()
        self._results['summary']['field_names'] = fieldNames
        self._results['summary']['field_types'] = fieldTypes


    def _log(self, message):
        """  
        """
#        print '(dataView) %s' % message


    def _pad(self, id):
        """
        _ids have a space inserted if a single digit to ensure sort comes out
        1,2,3 instead of 1,10,11,.. 2
        """
        result = '%4d' % id
        return result


    def _removeProblemTaQLCharacters(self, taqlstring):
        """Method to remove characters from strings that confuse TaQL.

        Keyword arguments:
        taqlString  -- the string to be checked.
        """
        result = taqlstring
        result = result.replace(' ', '')
        result = result.replace('{', '')
        result = result.replace('}', '')
        result = result.replace(':', '-')
        result = result.replace(',', '.')
 
        return result


    def _rmall(self, dirPath):
        """
        """

# do nothing if directory does not exist

        if not os.path.exists(dirPath):
            return

        namesHere = os.listdir(dirPath)
        for name in namesHere:                  # remove all contents first
            path = os.path.join(dirPath, name)
            if not os.path.isdir(path):         # remove simple files
                os.remove(path)
            else:                               # recur to remove subdirs
                self._rmall(path)
        os.rmdir(dirPath)                       # remove now-empty dirPath


    def _writeOnLine(self, value):
        sys.stdout.write(
         '\r                                                                 \r')
        sys.stdout.write(str(value) + '\r')
        sys.stdout.flush()


    def description(self):
        description = ['BaseData - basic access to the MS']
        return description


    def getData(self):
        """
        """

#        print 'BaseData.getData called'

# return a copy of the data list, otherwise operating on it outside this class
# will corrupt it

        temp = python_copy.deepcopy(self._results)
        return temp


    def getDataDescRange(self):
        """
        """
        return self._data_desc_ids


    def getFieldsOfType (self, source_type):
        """Return the list of field IDs in msfile that have the specified 
        source type.

        Keyword arguments:
        source_type -- the type of source; '*BANDPASS*', '*GAIN*', '*FLUX*' or
                       '*SOURCE*'

        """

        result_ids = []

        if type(source_type) == types.ListType:
            for source in source_type:
                new_results = self.getFieldsOfType(source)
                for r in new_results:
                    if not result_ids.count(r):
                        result_ids.append(r)
        else:

# verify that the source type is allowed; *BANDPASS*, *GAIN*, *FLUX* or *SOURCE*

            if source_type not in ['*BANDPASS*', '*GAIN*', '*FLUX*', '*SOURCE*']:
                raise NameError, 'bad source_type %s' % source_type

# Open the FIELD sub-Table and from it get the field type.

            self._table.open(self._msName + '/FIELD')
            source_type_col = util.util.get_source_types(self._table)

            for field_id in range(len(source_type_col)):
                column_type = source_type_col[field_id]
                column_type = column_type.strip()
                column_type = column_type.upper()
                pattern = fnmatch.translate(source_type)
                if re.match(pattern, column_type):
                    result_ids.append (field_id)

            self._table.close()

        return result_ids


    def writeFlaggingStatistics(self, stageDescription, rules, flagTargetIDs):
        """Method to log to html the number of flags set by the various
        rules used in this reduction stage.

        Keyword arguments:
        stageDescription -- Dictionary containing a description of the
                            calling stage.
        rules            -- List of the rules applied.
        flagTargetIDs    -- List of IDs of fields potentially flagged.
        """

        flagMessage,colour = self._msFlagger.writeFlaggingStatistics(stageDescription,
         rules, flagTargetIDs)

        return flagMessage, colour


    def setFlags(self, stageDescription, rules, flags, apply=True):
        """
        Keyword arguments:
        stageDescription -- Dictionary containing a description of the
                            calling stage.
        rules            -- List of the rules applied.
        flags            -- list of flags to be applied.
        apply            -- True if flags are to be applied, otherwise
                            do not apply them but store the flags in
                            the flag table.
        """
#        print 'BaseData.setFlags called'
        self._msFlagger.setFlags(stageDescription, rules, flags)


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        """

#        self._htmlLogger.logHTML('''<p>The data 'view' in this case simply
#         allowed the 'flagger' direct access to the MeasurementSet.''')
        return


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a detailed description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        topLevel   -- True if this is the data 'view' to be displayed directly.
        """
        if topLevel:
            self._htmlLogger.logHTML("""
             <h3>Data View</h3>""")

        self._htmlLogger.logHTML('''<p>The 'view' in this case simply
         allowed the 'flagger' direct access to the MeasurementSet.''')
#         <p>The view was constructed by Python class BaseData.''')
        return

