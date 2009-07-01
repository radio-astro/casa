"""Module to modify a data view by replacing each 'chunk' by the median of 
the data in it."""

# History:
# 16-Jul-2007 jfl First version.
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

from baseDataModifier import *


class ChunkMedian(BaseDataModifier):
    """Class to modify a data view by replacing each 'chunk' by the median of
    the data in it. 
    """

    def description(self):
        description = self._view.description()
        description.insert(0,
         'ChunkMedian - calculate the median of each chunk')
        return description


    def getData(self, topLevel=False):
        """Public method to take a data 'view, replace each chunk by its 
        median, then return it.

        Keyword arguments:
        topLevel -- True if this is the data 'view' to be displayed directly.
        """

#        print 'ChunkMedian.getData called'
        self._htmlLogger.timing_start('ChunkMedian.getData')

# have these data been calculated already?

        entryID,results = self._bookKeeper.available(
         objectType='ChunkMedian', sourceType=None,
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

# calculate the medians of the last gain results returned

            for k,v in view_results['data'].iteritems():
                description = pickle.loads(k)
                description['TITLE'] = '%s - chunk median' % \
                 description['TITLE']

                data = v[-1]['data']
                dataUnits = v[-1]['dataUnits']
                flag = v[-1]['flag']
                flagVersions = v[-1]['flagVersions']
                chunks = v[-1]['chunks']
                times = v[-1]['y']

                chunkTime = zeros([len(chunks)], float)
                chunkInterval = zeros([len(chunks)], float)
                outChunks = []
                outData = zeros([len(chunks),shape(data)[1]], float)
                outFlag = [ones([len(chunks),shape(data)[1]], int),
                 ones([len(chunks),shape(data)[1]], int)]

                for i in range(len(chunks)):
                    chunk = chunks[i]
                    chunkTime[i] = mean(take(times, chunk))
                    chunkInterval[i] = times[chunk[-1]] - times[chunk[0]]
                    outChunk = [i]
                    outChunks.append(outChunk)
                    nantennas = shape(data)[1]
                    for antenna in range(nantennas):
                        chunkData = take(data[:,antenna], chunk)
                        for fi,fs in enumerate(flagVersions):
                            chunkFlag = take(flag[fi][:,antenna], chunk)
                            validData = compress(chunkFlag==0, chunkData)
                            if len(validData) > 0:
                                outFlag[fi][i,antenna] = 0
                            else:
                                outFlag[fi][i,antenna] = 1
                        chunkFlag = take(flag[-1][:,antenna], chunk)
                        if len(validData) > 0:
                            outData[i,antenna] = median(validData)
                        else:
                            outData[i,antenna] = 0.0
                
                result = {}      
                result['dataType'] = 'median per chunk of %s' % (
                 v[-1]['dataType'])
                result['data'] = outData
                result['dataUnits'] = dataUnits
                result['mad_floor'] = zeros(shape(outData))
                result['flag'] = outFlag
                result['flagVersions'] = flagVersions
                result['y'] = chunkTime
                result['y_interval'] = chunkInterval
                result['ytitle'] = v[-1]['ytitle']
                result['chunks'] = outChunks
                result['x'] = v[-1]['x']
                result['xtitle'] = v[-1]['xtitle']
   
                kout = pickle.dumps(description)
                results['data'][kout] = result

# store the object info in the BookKeeper

            self._bookKeeper.enter(objectType='ChunkMedian',
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
        self._htmlLogger.timing_stop('ChunkMedian.getData')
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
         <p>The data view is calculated from the input data view described
         below. Pixels along the TIME axis of the input are replaced by the
         median value for the %s that they belong to.
         <p>The data view is calculated by Python class ChunkMedian.

         <h5>The Input Data View</h5>""" % 
         self._htmlLogger.glossaryLink('chunk'))

        self._view.writeDetailedHTMLDescription(stageName, False,
         parameters=parameters['dependencies']['view']) 

