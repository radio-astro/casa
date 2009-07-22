"""Module to calculate median and MAD od a data 'view'."""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Change profiling.
#  6-Nov-2007 jfl Best bandpass release.
# 20-Mar-2008 jfl BookKeeper release.
# 13-May-2008 jfl 13 release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import copy as python_copy
from numpy import *
import pickle

# alma modules

from baseDataModifier import *

class MedianAndMAD(BaseDataModifier):
    """Class to calculate the median and MAD of a data 'view' and return it 
    as a 'view'.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, collapseAxis, view, dataType='MedianAndMAD',
     description=None):
        """Constructor.
        tools        -- BaseTools object.
        bookKeeper   -- BookKeeper object.
        msCalibrater -- MSCalibrater object.
        msFlagger    -- MSFlagger object.
        htmlLogger   -- Conduit for loggin to HTML.
        msName       -- Name of MeasurementSet.
        stageName    -- Name of stage using this object.
        collapseAxis -- Name of axis in data 'view' along which the medians
                        and MADs are to be calculated.
        view         -- Giving the input data 'view'.
        dataType     -- 'MedianAndMAD' or 'MAD'.
        description  -- Comment to be written out to html as a preface to
                        the automatic description.
        """
#        print 'MedianAndMAD.__init__ called'
        BaseDataModifier.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, view, description)
        self._collapseAxis = collapseAxis
        self._dataType = dataType


    def description(self):
        description = self._view.description()
        if self._dataType == 'MedianAndMAD':
            description.insert(0, 
             'MedianAndMAD - calculate the median along 1 axis')
        elif self._dataType == 'MAD':
            description.insert(0,
             'MedianAndMAD - calculate the MAD along 1 axis')
        return description


    def getData(self, topLevel=False):
        """Public method that returns the data 'view'.
  
        Keyword arguments:
        topLevel -- True if this is the data view to be directly displayed.
        """

#        print 'MedianAndMAD.getData called'
        self._htmlLogger.timing_start('MedianAndMAD.getData')

# have these data been calculated already?

        inputs = self.inputs()
        entryID,results = self._bookKeeper.available(
         objectType=inputs['objectType'], sourceType=inputs['sourceType'],
         furtherInput=inputs['furtherInput'], outputFiles=inputs['outputFiles'],
         dependencies=inputs['dependencies'])

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

# iterate through data descriptions

            for k,v in view_results['data'].iteritems():
                description = pickle.loads(k)
                if self._dataType == 'MedianAndMAD':
                    description['TITLE'] = '%s - Median' % description['TITLE']
                elif self._dataType == 'MAD':
                    description['TITLE'] = '%s - MAD' % description['TITLE']

# look only at last version of data

                data = v[-1]['data']
                dataUnits = v[-1]['dataUnits']
                dataType = v[-1]['dataType']
                flag = v[-1]['flag']
                flagVersions = v[-1]['flagVersions']
                x = v[-1]['x']
                xtitle = v[-1]['xtitle']
                y = v[-1]['y'] 
                ytitle = v[-1]['ytitle']

# look only at 'Current' flag version

                if flagVersions.count('Current') > 0:
                    i_current = flagVersions.index('Current')
                else:
                    raise ValueError, "data has 'Current' flagversion"

                if self._collapseAxis == v[-1]['xtitle']:
                    outData = zeros([shape(data)[0],1], float)
                    outMAD = zeros([shape(data)[0],1], float)
                    outFlag = []
                    for fi,fs in enumerate(flagVersions):
                       outFlag.append(ones([shape(data)[0],1], int))
                    chunks = v[-1]['chunks']
                    x = [0]
                    xtitle = 'median'

                    for ix in range(shape(data)[0]):
                        for fi,fs in enumerate(flagVersions):
                            if all(flag[fi][ix,:]):
                                continue
                            else:
                                outFlag[fi][ix,0] = 0

                        if outFlag[i_current][ix,0]:
                            outData[ix,0] = 0.0
                            outMAD[ix,0] = 0.0
                            continue

                        validData = compress(flag[i_current][ix,:]==0,
                         data[ix,:])
                        if len(validData) > 0:
                            outData[ix,0] = median(validData)
                            outMAD[ix,0] = median(abs(validData - 
                             outData[ix,0]))

                elif self._collapseAxis == v[-1]['ytitle']:
                    outData = zeros([1,shape(data)[1]], float)
                    outMAD = zeros([1,shape(data)[1]], float)
                    outFlag = []
                    for fi,fs in enumerate(flagVersions):
                       outFlag.append(ones([1,shape(data)[1]], int))
                    chunks = []
                    y = [0]
                    ytitle = 'median'

                    for ix in range(shape(data)[1]):
                        for fi,fs in enumerate(flagVersions):
                            if all(flag[fi][:,ix]):
                                continue
                            else:
                                outFlag[fi][0,ix] = 0

                        if outFlag[i_current][0,ix]:
                            outData[0,ix] = 0.0
                            outMAD[0,ix] = 0.0
                            continue

                        validData = compress(flag[i_current][:,ix]==0,
                         data[:,ix])
                        if len(validData) > 0:
                            outData[0,ix] = median(validData)
                            outMAD[0,ix] = median(abs(validData -
                             outData[0,ix]))

                else:
                    raise NameError, 'bad collapseAxis: %s' % \
                     self._collapseAxis

# calculate results

                result = {}
                if self._dataType == 'MedianAndMAD':
                    result['dataType'] = 'median over %s of %s' % (
                     self._collapseAxis, v[-1]['dataType'])
                    result['data'] = outData
                    result['mad_floor'] = outMAD
                    result['dataType'] = 'median of %s' % dataType
                elif self._dataType == 'MAD':
                    result['dataType'] = 'MAD over %s of %s' % (
                     self._collapseAxis, v[-1]['dataType'])
                    result['data'] = outMAD
                    result['mad_floor'] = zeros(shape(outMAD))
                    result['dataType'] = 'MAD of %s' % dataType
                result['dataUnits'] = dataUnits
                result['flag'] = outFlag
                result['flagVersions'] = flagVersions
                result['x'] = x
                result['xtitle'] = xtitle
                result['y'] = y
                result['ytitle'] = ytitle
                result['chunks'] = chunks
                if v[-1].has_key('noisyChannels'):
                    result['noisyChannels'] = v[-1]['noisyChannels']
                if v[-1].has_key('bandpassFlaggingStage'):
                    result['bandpassFlaggingStage'] = v[-1]['bandpassFlaggingStage']
  
# and store in dictionary

                kout = pickle.dumps(description)
                results['data'][kout] = result

# store the object info in the BookKeeper

            self._bookKeeper.enter(
             objectType=inputs['objectType'], sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'], 
             outputFiles=inputs['outputFiles'],
             outputParameters=results,
             dependencies=inputs['dependencies'])

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
        self._htmlLogger.timing_stop('MedianAndMAD.getData')
        return temp


    def inputs(self):
        """Method to return the input settings for this object.
        """
        result = {}
        result['objectType'] = self._dataType
        result['sourceType'] = None
        result['furtherInput'] = {'collapseAxis':self._collapseAxis,
         'dataType':self._dataType},
        result['outputFiles'] = []
        result['dependencies'] = [self._view.inputs()]
        result['flag_marks'] = {}

        return result


    def writeGeneralHTMLDescription(self, stageName):
        """Write a description of the class to html.

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
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
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
         <p>The data view is a list of 1-d arrays (actually 2-d but the length
         of one axis is 1). Each point in an array is the
         median of the sequence of points from a cut down the '%s' axis 
         of the input 'view object' described below. Associated with each
         point is an error estimate that is the median absolute deviation
         (%s) of the sequence.""" % (self._collapseAxis,
         self._htmlLogger.glossaryLink('MAD')))

        self._htmlLogger.logHTML("""
         <h5>The 'View Object' of MedianAndMAD</h5>""")
        self._view.writeDetailedHTMLDescription(stageName, False, 
         parameters=parameters['dependencies']['view'])


class MAD(MedianAndMAD):
    """Class to calculate the MAD of a data 'view' and return it as a 'view'.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, collapseAxis, view, description=None):
        """Constructor.
        tools        -- BaseTools object.
        bookKeeper   -- BookKeeper object.
        msCalibrater -- MSCalibrater object.
        msFlagger    -- MSFlagger object.
        htmlLogger   -- Conduit for loggin to HTML.
        msName       -- Name of MeasurementSet.
        stageName    -- Name of stage using this object.
        collapseAxis -- Name of axis in data 'view' along which the medians
                        and MADs are to be calculated.
        view         -- Giving the input data 'view'.
        description  -- Comment to be written out to html as a preface to
                        the automatic description.
     """

        MedianAndMAD.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName, collapseAxis, view, dataType='MAD',
         description=description)


    def writeGeneralHTMLDescription(self, stageName):
        """Write a description of the class to html.

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

# this class modifies data from other classes and it's difficult to construct
# automatically a readable general description of the 'view'. If possible,
# such a description should be written as part of the recipe.

            if self._description != None:
                self._htmlLogger.logHTML('<p> %s' % self._description)

        self._htmlLogger.logHTML("""
         <p>The data view is a list of 1-d arrays, derived by collapsing 
         the %s axis of each 2-d array in the list generated by the input 
         'view object' described below. Each 'collapsed' point is the
         median absolute deviation (%s) of the input sequence of points.""" % 
         (self._htmlLogger.glossaryLink('MAD'), self._collapseAxis))

        self._htmlLogger.logHTML("""
         <h5>The 'View Object' of MAD</h5>""")

        self._view.writeDetailedHTMLDescription(stageName, False, 
         parameters=parameters['dependencies']['view'])
