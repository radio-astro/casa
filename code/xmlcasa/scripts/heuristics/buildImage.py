"""Module with class to build a 2-d array from a series of 1-d arrays."""

# History:
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release.
# 25-Jun-2008 jfl regression release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.

# package modules

import copy as python_copy
from numpy import *
import pickle
from types import *

# alma modules

from baseDataModifier import *

class BuildImage(BaseDataModifier):
    """Class to build a 2-d array from a series of 1-d arrays."""

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, yAxis, view, description=None):
        """Constructor.

        Keyword arguments:
        tools       -- BaseTools object.
        bookKeeper  -- BookKeeper object.
        msCalibrater-- MSCalibrater object.
        msFlagger   -- MSFlagger object.
        htmlLogger  -- Route for logging to html structure.
        msName      -- Name of MeasurementSet
        stageName   -- The name of the stage using this object.
        yAxis       -- Name of description item of 1-d data that is to be used
                       as its y coordinate in the 2-d array.
        view        -- Object giving access to 1-d data.
        description -- Explanatory comment to be written out to html as
                       a preface to the automatic html description.
        """

#        print 'BuildImage.__init__ called'
        BaseDataModifier.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, view, description)
        self._yAxis = yAxis


    def description(self):
        description = self._view.description()
        description.insert(0, 'BuildImage - construct an image from slices')
        return description


    def getData(self, topLevel=False):
        """Public method to build the 2-d array and return it as the data
        'view'.

        Keyword arguments:
        topLevel -- True if this is the data 'view' to be displayed directly.
        """

#        print 'BuildImage.getData called'
        self._htmlLogger.timing_start('BuildImage.getData')

# have these data been calculated already?

        entryID,results = self._bookKeeper.available(
         objectType='BuildImage', sourceType=None,
         furtherInput={'yAxis':self._yAxis},
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

# iterate through data descriptions, assembling data for the images

            imagesToBuild = {}
            descriptionList = []
            flagVersions = None

            for key,value in view_results['data'].iteritems():
                description = pickle.loads(key)
                description['dataType'] = value[-1]['dataType']
                if flagVersions == None:
                    flagVersions = value[-1]['flagVersions']
                dataUnits = value[-1]['dataUnits']

# remove TITLE as this can change with the set of data that we want to combine

                ignore = description.pop('TITLE')

# iterate through different values of the _yAxis element

                if description.has_key(self._yAxis):
                    yVal = description.pop(self._yAxis)
                    if type(yVal) == ListType:
                        if len(yVal) > 1:
                            print 'WARNING - yVal is list with > 1 element'
                        yVal = yVal[0]

                    if descriptionList.count(description) == 0:
                        descriptionList.append(description)
                    i = descriptionList.index(description)  

                    if not(imagesToBuild.has_key(i)):
                        imagesToBuild[i] = {}
                        imagesToBuild[i]['yVal'] = []
                        imagesToBuild[i]['data'] = []
                        imagesToBuild[i]['mad_floor'] = []
                        imagesToBuild[i]['flag'] = []

                    if imagesToBuild[i].has_key('x'):
                        if not(all(imagesToBuild[i]['x'] == value[-1]['x'])):
                            raise NameError, 'x axes do not match'   
                    else:
                        imagesToBuild[i]['x'] = value[-1]['x']
                    if imagesToBuild[i].has_key('xtitle'):
                        if imagesToBuild[i]['xtitle'] != value[-1]['xtitle']:
                            raise NameError, 'x titles do not match'   
                    else:
                        imagesToBuild[i]['xtitle'] = value[-1]['xtitle']

                    imagesToBuild[i]['yVal'].append(int(yVal))
                    imagesToBuild[i]['data'].append(value[-1]['data'])
                    imagesToBuild[i]['mad_floor'].append(value[-1]['mad_floor'])
                    imagesToBuild[i]['flag'].append(value[-1]['flag'])

# now assemble the images

            data = {}
            flag = {}
            mad_floor = {}
            x = {}
            xtitle = {}
            y = {}
            ytitle = {}
            iy = 0
            for i,d in enumerate(descriptionList):
                value = imagesToBuild[i]
                data[i] = zeros([len(value['x']), len(value['yVal'])], float)
                mad_floor[i] = zeros([len(value['x']), len(value['yVal'])],
                 float)
                flag[i] = []
                for fi,fs in enumerate(flagVersions):
                    flag[i].append(
                     ones([len(value['x']), len(value['yVal'])], bool))
                x[i] = value['x']
                xtitle[i] = value['xtitle']
                y[i] = array(value['yVal'], int)
                ytitle[i] = self._yAxis

                sy = argsort(y[i])
                for iy in range(len(sy)):
                    data[i][:,iy] = ravel(array(
                     imagesToBuild[i]['data'][sy[iy]]))
                    mad_floor[i][:,iy] = ravel(array(
                     imagesToBuild[i]['mad_floor'][sy[iy]]))
                    for fi,fs in enumerate(flagVersions):
                        flag[i][fi][:,iy] = ravel(
                         array(imagesToBuild[i]['flag'][sy[iy]][fi]))
                    y[i][iy] = int(value['yVal'][sy[iy]])

# calculate results

            for i,description in enumerate(descriptionList):
                title = ''
                if description.has_key('FIELD_ID'):
                    field_id = description['FIELD_ID']
                    title = '%s\nField:%s (%s %s)' % (title,
                     self._pad(field_id),
                     description['FIELD_TYPE'], description['FIELD_NAME'])
                if description.has_key('DATA_DESC_ID'):
                    title = '%s\nSpW:%s' % (title,
                     self._pad(description['DATA_DESC_ID']))
                if description.has_key('POLARIZATION_ID'):
                    title = '%s\nPol:%s' % (title,
                     description['POLARIZATION_ID'])
                if description.has_key('dataType'):
                    title = '%s\n%s' % (title, description['dataType'])
                description['TITLE'] = title

                result = {}
                result['dataType'] = description['dataType']
                result['data'] = data[i]
                result['mad_floor'] = mad_floor[i]
                result['dataUnits'] = dataUnits
                result['flag'] = flag[i]
                result['flagVersions'] = flagVersions
                result['x'] = x[i]
                result['xtitle'] = xtitle[i]
                result['y'] = y[i]
                result['y_interval'] = zeros(shape(result['y']), float) + abs(
                 y[i][1] - y[i][0])
                result['ytitle'] = ytitle[i]
                result['chunks'] = []

                pickled_description = pickle.dumps(description)
                results['data'][pickled_description] = result

# store the object info in the BookKeeper

            self._bookKeeper.enter(objectType='BuildImage',
             sourceType=None,
             furtherInput={'yAxis':self._yAxis},
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

# return a copy of the data list, otherwise operating on it outside this class
# will corrupt it

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('BuildImage.getData')
        return temp


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

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
             <p>The data view is a 2-d array that was built by 
                packing side by side items from the list of 1-d results 
                generated by the 'view object' described next. 
                Each item in the list from the 'view object' is for a 
                different value of %s, and this value is the y coordinate 
                of the slice in the built 2-d array.
             <p>The view was constructed by Python class BuildImage.
         """ % self._yAxis)

        self._htmlLogger.logHTML("""
         <h5>The 'View Object' of BuildImage</h5>""")

        self._view.writeDetailedHTMLDescription(stageName, False,
         parameters=parameters['dependencies']['view'])

