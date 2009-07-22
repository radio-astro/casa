# History:
# 16-Jul-2007 jfl First version.
# 30-Aug-2007 jfl Flux calibrating version.
#  6-Nov-2007 jfl Best bandpass version.
# 28-Nov-2007 jfl Recipe release.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
# 10-Sep-2008 jfl msCalibrater release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

# package modules

import copy as python_copy
from numpy import *

# alma modules

import baseData
import util

class BaseDataModifier:

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, view, description=None):
        """Constructor.

        Keyword arguments:
        tools       -- BaseTools object.        
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.        
        msFlagger   -- MSFlagger object.        
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- Name of stage using this object.
        view        -- Object giving access to 1-d data.
        description -- Comment to be written out to html as a preface to
                       the automatic description.
        """
#        print 'BaseDataModifier.__init__ called'
        tools.copy(self)
        self._tools = tools
        self._bookKeeper = bookKeeper
        self._msCalibrater = msCalibrater
        self._msFlagger = msFlagger
        self._htmlLogger = htmlLogger
        self._msName = msName
        self._fullStageName = stageName
        self._stageName = stageName.replace(' ','')
        if view == None:
            self._view = baseData.BaseData(tools, bookKeeper, msCalibrater,
             msFlagger, htmlLogger, msName, stageName)
        else:
            self._view = view
        self._description = description

        self._results = {}
        self._getSummary()
        self._results['data'] = {}
        self._results['flagging'] = [] 
        self._results['flaggingReason'] = []   
        self._results['flaggingApplied'] = []


    def _getSummary(self):
        """
        """
        self._results['summary'] = {}

        success = self._ms.open(self._msName)

# get the range of antennas

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
        self._results['summary']['antenna_range'] = antenna_range

# data_desc_ids

        msInfo = self._ms.range(['data_desc_id'])
        data_desc_ids = msInfo['data_desc_id']
        self._results['summary']['data_desc_range'] = data_desc_ids

# spw info

        self._results['summary']['corr_axis'] = {}
        self._results['summary']['nchannels'] = {}
        for data_desc_id in data_desc_ids:
            self._ms.selectinit(data_desc_id)
            msInfo = self._ms.getdata(['axis_info'])
            self._results['summary']['corr_axis'][data_desc_id] = \
             msInfo['axis_info']['corr_axis']
            self._results['summary']['nchannels'][data_desc_id] = len(
             msInfo['axis_info']['freq_axis']['chan_freq'])
        self._ms.close()

# telescope name

        success = self._table.open(self._msName + '/OBSERVATION')
        telescope_name =  self._table.getcol('TELESCOPE_NAME')[0]
        self._table.close()
        self._results['summary']['telescope_name'] = telescope_name

# general field info

        success = self._table.open('%s/FIELD' % self._msName)
        fieldNames = self._table.getcol('NAME')
        fieldTypes = util.util.get_source_types(self._table)
        self._table.close()
        self._results['summary']['field_names'] = fieldNames
        self._results['summary']['field_types'] = fieldTypes


    def _pad(self, id):
        """
        _ids have a space inserted if a single digit to ensure sort comes out
        1,2,3 instead of 1,10,11,.. 2
        """
        result = '%4d' % id
        return result


    def _removeFlags(self, stageNameList):
        """
        """
        self._view._removeFlags(stageNameList)


    def getFieldsOfType(self, source_type):
        """Return the list of field IDs in msfile that have the specified
        source type.

        Keyword arguments:
        source_type -- The type of source whose field_ids are to be found.
        """
        result = self._view.getFieldsOfType(source_type)
        return result


    def getFlags(self):
        """
        """
        result = self._view.getFlags()
        return result


    def nint(self, number):
        """Return the nearest integer to the input number.
        
        Keyword arguments:
        number -- the number to be converted
        """
        result = int(floor(number))
        if (number - result) > 0.5:
            result = result + 1
        return result


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
        self._msFlagger.setFlags(stageDescription, rules, flags, apply)


    def setStageName(self, name):
        """ 
        """
        self._fullStageName = str(name)
        self._stageName = name.replace(' ','')


    def transferFlags(self, stageDescription, rule):
        """
        """
        result = self._view.transferFlags(stageDescription, rule)
        return result


    def writeFlaggingStatistics(self, stageDescription, rules, flagTargetIDs):
        """Method to log to html the number of flags set by the various
        rules used in this reduction stage.

        Keyword arguments:
        stageDescription -- Dictionary containing a description of the
                            calling stage.
        rules            -- List of the rules applied.
        flagTargetIDs    -- List of IDs of fields potentially flagged.
        """

        flagMessage,colour = self._view.writeFlaggingStatistics(stageDescription, rules,
         flagTargetIDs)

        return flagMessage, colour


    def writeHTMLDescription(self, stageName, topLevel):
        """
        """
        if topLevel:
            self._htmlLogger.logHTML("""
             <h3>Data View</h3>""")
        self._htmlLogger.logHTML("""No description available.""")
