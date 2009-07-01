"""Module with class to use the 'best' method to calculate another data object.
"""

# History:
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release. writeHTMLDescription got working.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

from numpy import *
import os
import pickle

# alma modules

import baseData
from baseDataModifier import *


class BestMethod(BaseDataModifier):
    """Class that uses the 'best' method to calculate a given data object.
    """

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, viewClassList, description=None):
        """Constructor.

        Keyword arguments:
        tools         -- BaseTools object.
        bookKeeper    -- BookKeeper object.
        msCalibrater  -- MSCalibrater object.
        msFlagger     -- MSFlagger object.
        htmlLogger    -- Route for logging to html structure.
        msName        -- Name of MeasurementSet
        stageName     -- Name of stage using this object.
        viewClassList -- List of classes put together to calculate the 
                         required view.
        description   -- Explanatory comment to be written out to html as
                         a preface to the automatic html description.
        """

#        print 'BestMethod.__init__ called'
        BaseDataModifier.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, baseData.BaseData(tools,
         bookKeeper, msCalibrater, msFlagger, htmlLogger, msName, stageName,
         description))

# read the calibration method to use for each SpW

        f = open('B_calibration.lis') 
        self._methodForEachSpw = pickle.load(f)
        f.close()

        if len(viewClassList) == 1:
            self._view = viewClassList[0](tools, bookKeeper, msCalibrater,
             msFlagger, htmlLogger, msName, stageName, self._methodForEachSpw) 
        else:
            self._view = viewClassList[0](tools, bookKeeper, msCalibrater,
             msFlagger, htmlLogger, msName, stageName, self._methodForEachSpw,
             viewClassList[1:])
 

    def calculate(self):
        """
        """

#        print 'BestMethod.calculate called'

# the BookKeeper is not used in this class.

        self._htmlLogger.timing_start('BestMethod.calculate')

# calculate what is required and return the results.

        results = self._view.calculate()
        results['BestMethod'] = {}
        results['BestMethod']['file'] = 'B_calibration.lis'
        results['BestMethod']['method'] = self._methodForEachSpw

        self._bestMethodParameters = results.copy()
        self._htmlLogger.timing_stop('BestMethod.calculate')
        return results


    def description(self):
        description = self._view.description()
        description.insert(0,
         'BestMethod - use best method found by BestMethodSelector')
        return description


    def getData(self, topLevel=False):
        """Public method to return the optimal version of the data 'view'.

        Keyword arguments:
        topLevel -- True if this is the data 'view' to be displayed directly.
        """

#        print 'BestMethod.getData called'

# the BookKeeper is not used in this class.

        self._htmlLogger.timing_start('BestMethod.getData')
        results = self._view.getData()

        results['parameters']['BestMethod'] = {}
        results['parameters']['BestMethod']['file'] = 'B_calibration.lis'
        results['parameters']['BestMethod']['method'] = self._methodForEachSpw
        self._results = python_copy.deepcopy(results)

# return a copy of the data list

        temp = python_copy.deepcopy(results)
        self._htmlLogger.timing_stop('BestMethod.getData')
        return temp


    def inputs(self):
        """
        """
        results = self._view.inputs()
        return results


    def setapply(self, spw, field):
        """
        """
        results = self._view.setapply(spw, field)
        return results


    def createGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to HTML.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
         """
# describe where the method was stored and what it was.

        description = """
         <p>The 'best' bandpass method to be used for each spectral window was 
         retrieved. The bandpass calibration was calculated. 
         """

# now add the 'view' description

        description += self._view.createGeneralHTMLDescription(stageName)\
         ['bandpass calibration']

        bestMethodDescription = {'bandpass calibration':description}

        return bestMethodDescription


    def createDetailedHTMLDescription(self, stageName, parameters=None):
        """Write a detailed description of the class to HTML.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        parameters -- Dictionary with details of object.
        """

        if parameters == None:
            parameters = self._results['parameters']
 
# this class modifies data from other classes and it's difficult to construct
# automatically a readable general description of the 'view'. If possible,
# such a description should be written as part of the recipe.

        if self._description != None:
            self._htmlLogger.logHTML('<p> %s' % self._description)

# describe where the method was stored and what it was.

        description = """
         The bandpass method used for each spectral window was retrieved 
         from file '%s'.""" % (parameters['BestMethod']['file'])

# now add the 'view' description

        description += self._view.createDetailedHTMLDescription(stageName,
         parameters=parameters)['bandpass calibration']

        bestMethodDescription = {'bandpass calibration':description}
        return bestMethodDescription


    def writeGeneralHTMLDescription(self, stageName):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = self.createGeneralHTMLDescription(stageName)
        self._htmlLogger.logHTML(description['bandpass calibration'])


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        topLevel   -- True if this data 'view' is to be displayed directly,
                      not passing through a data modifier object.
        parameters -- Dictionary with details of object.
        """

        if parameters==None:
             parameters = self._results['parameters']

        if topLevel:
            self._htmlLogger.logHTML("<h3>Data View</h3>")

        description = self.createDetailedHTMLDescription(stageName,
         parameters=parameters)
        self._htmlLogger.logHTML(description['bandpass calibration'])
