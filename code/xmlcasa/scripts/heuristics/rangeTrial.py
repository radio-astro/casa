"""Module to run a mesh of trial calculations."""

# History:
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import copy as python_copy
from numpy import *
import pickle
import types

# alma modules

from baseDataModifier import *


class RangeTrial(BaseDataModifier):
    """Class to run a mesh of trial calculations."""

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, parameterRange, viewClassList, description=None):
        """Constructor.

        Keyword arguments:
        tools          -- BaseTools object.
        bookKeeper     -- BookKeeper object.
        msCalibrater   -- MSCalibrater object.
        msFlagger      -- MSFlagger object.
        htmlLogger     -- Object for logging to html structure.
        msName         -- Name of MeasurementSet containing the data.
        stageName      -- Name of stage using this object.
        parameterRange -- Dictionary whose keys are the names of
                          parameters to be varied, values are lists
                          giving the range of values to be tried.
        viewClassList  -- List of classes to be used to construct class 
                          of object
                          providing a data 'view' for each of
                          the parameter settings in the range.
        description    -- Explanatory comment to be written out to html as
                          a preface to the automatic html description.
        """
#        print 'RangeTrial.__init__ called'
        BaseDataModifier.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, None, description)
        self._parameterRange = parameterRange
        self._viewClassList = viewClassList
        self._results['data'] = {}
        self._results['history'] = {'origin':self._fullStageName}


    def description(self):
        if len(self._viewClassList) == 1:
            temp_view = self._viewClassList[0](self._tools,
             self._bookKeeper, self._msCalibrater, self._msFlagger, 
             self._htmlLogger, self._msName, self._fullStageName)
        else:
            self._view = self._viewClassList[0](self._tools,
             self._bookKeeper, self._msCalibrater, self._msFlagger,
             self._htmlLogger, self._msName, self._fullStageName, None,
             self._viewClassList[1:])

        description = self._view.description()
        description.insert(0,
         'RangeTrial - calculate the target object on a mesh of input parameters')
        return description


    def getData(self, topLevel=False):
        """Public method to get the data 'view'.

        Keyword arguments:
        topLevel -- True if this is data 'view' being passed to the operator
                    and display objects.
        """

#        print 'RangeTrial.getData called'

# This class does not use the BooKeeper as if it used properly ther would not
# be much gain from using it.

        self._htmlLogger.timing_start('RangeTrial.getData')

        self._results['parameters'] = {}
        self._results['parameters']['history'] = self._fullStageName
        self._results['parameters']['dependencies'] = {}

# get the field_ids of any SOURCE_TYPE entry, remove duplicates
# as this can lead to confusion (and wastes time)

#        print 'input', self._parameterRange
        for k,v in self._parameterRange.iteritems():
            if k=='source_type':
                field_ids = []
                for w in v:
                    field_ids += self.getFieldsOfType(w)
                field_ids_copy = list(field_ids)
                for field_id in field_ids_copy:
                    for i in range(field_ids.count(field_id)-1):
                        field_ids.remove(field_id)

                ignore = self._parameterRange.pop('source_type')
                self._parameterRange['field_id'] = field_ids

            if k=='test_source_type':
                field_ids = []
                for w in v:
                    field_ids += self.getFieldsOfType(w)
                field_ids_copy = list(field_ids)
                for field_id in field_ids_copy:
                    for i in range(field_ids.count(field_id)-1):
                        field_ids.remove(field_id)

                ignore = self._parameterRange.pop('test_source_type')
                self._parameterRange['test_field_id'] = field_ids

#        print 'SOURCE_TYPE massaged', self._parameterRange

# expand the parameterRange into a list of dictionaries that specify each
# calculation in the range

        methods = [self._parameterRange]
        expanding = True
        while expanding:
            expanding = False
            methods_copy = list(methods)
            for method in methods:
                method_copy = method.copy()
                for k,v in method.iteritems():
                    if type(v) == types.ListType:
                        if methods_copy.count(method) > 0:
                            methods_copy.remove(method)
                        method_copy.pop(k)
                        for w in v:
                            method_copy[k] = w
                            methods_copy.append(method_copy.copy()) 
                            expanding = True
                        break
            methods = methods_copy

#        print 'parameters', self._parameterRange

# build list with methods for each spw to pass to the dataView

        valid_field_spw = self._msFlagger.getValidFieldSpw()

        data_desc_ids = self._results['summary']['data_desc_range']
        methodsForEachSpw = []
        for method in methods:
            methodForEachSpw = {}
            for data_desc_id in data_desc_ids:

# check that the bandpass field and test field both have data for this spw

                if \
                 (valid_field_spw.count([method['field_id'],data_desc_id]) > 0)\
                 and \
                 (valid_field_spw.count([method['test_field_id'],data_desc_id]) > 0):
                    methodForEachSpw[data_desc_id] = method

            if len(methodForEachSpw) > 0:
                methodsForEachSpw.append(methodForEachSpw)
#        print 'expanded', len(methodsForEachSpw)

# iterate through each specification in the range

        for methodForEachSpw in methodsForEachSpw:

# construct the test object and obtain data from it

            if len(self._viewClassList) == 1:
                self._view = self._viewClassList[0](self._tools,
                 self._bookKeeper, self._msCalibrater, self._msFlagger,
                 self._htmlLogger, self._msName, self._fullStageName, 
                 methodForEachSpw)
            else:
                self._view = self._viewClassList[0](self._tools,
                 self._bookKeeper, self._msCalibrater, self._msFlagger,
                 self._htmlLogger, self._msName, self._fullStageName,
                 methodForEachSpw, self._viewClassList[1:])

            newResults = self._view.getData()
            if not self._results.has_key('summary'):
                self._results['summary'] = python_copy.deepcopy(
                 newResults['summary'])
            if not self._results['parameters']['dependencies'].has_key(
             'view'):
                self._results['parameters']['dependencies']['view'] = \
                 python_copy.deepcopy(newResults['parameters'])

            for k,v in newResults['data'].iteritems():
                if not(self._results['data'].has_key(k)):
                    self._results['data'][k] = []
                for w in v:
                    w['methodForEachSpw'] = newResults['methodForEachSpw']
                    self._results['data'][k].append(w)

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('RangeTrial.getData')
        return temp


    def writeHTMLDescription(self, stageName):
        """Method to write a general description of the data view to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        """

# this class modifies data from other classes and it's difficult to construct
# automatically a readable general description of the 'view'. If possible,
# such a description should be written as part of the recipe.

        if self._description != None:
            self._htmlLogger.logHTML('<p> %s' % self._description)


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Method to write a description of the data view' to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        parameters -- Dictionary that holds the descriptive information.
        """

        if parameters == None:
            parameters = self._results['parameters']

        if topLevel:
            self._htmlLogger.logHTML('<h3>Data View</h3>')

# this class modifies data from other classes and it's difficult to construct
# automatically a readable general description of the 'view'. If possible,
# such a description should be written as part of the recipe.

            if self._description != None:
                self._htmlLogger.logHTML('<p> %s' % self._description)

        self._htmlLogger.logHTML('''<p>
         The list of calibration methods tested was generated by expanding
         the following dictionary from the recipe:<br><br>%s<br>
         to produce a list of dictionaries where all combinations of 
         keyword:value pairs are covered.''' % self._parameterRange)

        self._view.writeDetailedHTMLDescription(stageName, False,
         parameters=parameters['dependencies']['view'])
