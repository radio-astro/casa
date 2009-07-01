# History:
#  6-Nov-2007 jfl Best bandpass release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import copy as python_copy
import math
from numpy import *
import pickle

# alma modules

from baseDataModifier import *

class MedianJump(BaseDataModifier):

    def description(self):
        description = self._view.description()
        description.insert(0,
         'MedianJump - calculate the median jump in a sequence')
        return description


    def getData(self, topLevel=False):
        """
        Keyword arguments:
        topLevel -- True if this is the data view to be directly displayed.
        """

#        print 'MedianJump.getData called'
        self._htmlLogger.timing_start('MedianJump.getdata')

# have these data been calculated already?

        entryID,results = self._bookKeeper.available(
         objectType='MedianJump', sourceType=None,
         furtherInput={},
         outputFiles=[],
         dependencies=[self._view.inputs()])

        if entryID == None:

# get the target view

            view_results = self._view.getData()
 
            results = {
             'parameters':{'history':'',
                           'dependencies':{}},
             'data':{}}

            results['parameters']['history'] = self._fullStageName
            results['parameters']['dependencies']['view'] = \
             view_results['parameters']

# modify the last gain results returned to contain the median jump between 
# data points instead of the data themselves

            for k,v in view_results['data'].iteritems():
                description = pickle.loads(k)
                description['TITLE'] = '%s - median jump' % (
                 description['TITLE'])
                kout = pickle.dumps(description)

                data = v[-1]['data']
                flag = v[-1]['flag']
                flagVersions = v[-1]['flagVersions']
                nantennas = shape(data)[1]
                median_data_jump = zeros([nantennas,1], float)
                median_data_jump_flag = []
                for fv in flagVersions:
                    median_data_jump_flag.append(ones([nantennas,1], int))

                for antenna in range(nantennas):

# calculate results for Current flags

                    valid_data = compress(logical_not(flag[-1][:,antenna]),
                     data[:,antenna])
                    if len(valid_data) > 1:
                        data_jump = valid_data[1:] - valid_data[:-1]
                        median_data_jump[antenna,0] = median(abs(data_jump))

# now calculate flags for each flagVersion

                    for fi,fv in enumerate(flagVersions):
                        valid_data = compress(logical_not(flag[fi][:,antenna]),
                         data[:,antenna])
                        if len(valid_data) > 1:
                            median_data_jump_flag[fi][antenna,0] = 0

                result = {}
                result['chunks'] = []
                result['dataType'] = 'median jump in %s' % v[-1]['dataType']
                result['dataUnits'] = 'median jump in %s' % v[-1]['dataUnits']
                result['ytitle'] = 'ANTENNA'
                result['y'] = array(v[-1]['x'])
                result['xtitle'] = 'undefined'
                result['x'] = [0] 

                result['data'] = median_data_jump
                result['flag'] = median_data_jump_flag
                result['flagVersions'] = flagVersions
                result['mad_floor'] = zeros([nantennas,1], float) 

# store in dictionary

                results['data'][kout] = result

# store the object info in the BookKeeper

            self._bookKeeper.enter(objectType='MedianJump',
             sourceType=None,
             furtherInput={},
             outputFiles=[],
             outputParameters=results,
             dependencies=[self._view.inputs()])

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

# add history and dependency info
             
        self._results['parameters'] = results['parameters']
 
        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('MedianJump.getdata')
        return temp


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

# this class modifies data from other classes and it's difficult to construct
# automatically a readable general description of the 'view'. If possible,
# such a description should be written as part of the recipe.

        if self._description != None:
            self._htmlLogger.logHTML('<p> %s' % self._description)


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a detailed description of the class to html.

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

# this class modifies data from other classes and it's difficult to construct
# automatically a readable general description of the 'view'. If possible,
# such a description should be written as part of the recipe.

            if self._description != None:
                self._htmlLogger.logHTML('<p> %s' % self._description)

        self._htmlLogger.logHTML("""
         <p>The data view is a modification of
         the input data view described below. Each 2-d array from the
         input data view is replaced by a 1-d array with values at each x
         equal to the median absolute difference between pixels along the
         corresponding y-cut through the 2-d array.

         <h5>The Data View input to MedianJump</h5>""")

        self._view.writeDetailedHTMLDescription(stageName, False, parameters=
         parameters['dependencies']['view'])

