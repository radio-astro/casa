# History:
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release. writeHTMLDescription got working.
# 25-Jun-2008 jfl regression release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import copy as python_copy
from numpy import *
import pickle

# alma modules

from baseDataModifier import *


class BandpassMerit(BaseDataModifier):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, methodForEachSpw, viewClassList, description=None):
        """Constructor.
        tools            -- BaseTools object.
        bookKeeper       -- BookKeeper object.
        msCalibrater     -- MSCalibrater object.
        msFlagger        -- MSFlagger object.
        htmlLogger       -- Conduit for logging to HTML.
        msName           -- Name of MeasurementSet.
        stageName        -- Name of stage using this object.
        methodForEachSpw -- Dictionary specifying method to use for each
                            data_desc_id.
        viewClassList    -- List of classes used to create the data view whose
                            merit is to be calculated.
        description      -- Explanatory comment to be written out to html as
                            a preface to the automatic html description.
        """

#        print 'BandpassMerit.__init__ called '
        BaseDataModifier.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, None, description)
        self._methodForEachSpw = methodForEachSpw

        if len(viewClassList) == 1:
            self._view = viewClassList[0](tools, bookKeeper, msCalibrater,
             msFlagger, htmlLogger, msName, stageName, methodForEachSpw) 
        else:
            self._view = viewClassList[0](tools, bookKeeper, msCalibrater,
             msFlagger, htmlLogger, msName, stageName, methodForEachSpw,
             viewClassList[1:])


    def description(self):
        description = self._view.description()
        description.insert(0,
         'BandpassMerit - calculate merit of bandpass solution')
        return description


    def getData(self):
        """
        """

#        print 'BandpassMerit.getData called' 
        self._htmlLogger.timing_start('BandpassMerit.getdata')

# don't store results in the bookKeeper as it seems unlikely to be useful.

# construct the view object and get results from it.

        view_results = self._view.getData()

        self._results['parameters'] = {'history':'',
                                       'dependencies':{}}

        self._results['summary'] = view_results['summary'].copy()
        self._results['methodForEachSpw'] = self._methodForEachSpw
        self._results['parameters']['history'] = self._fullStageName
        self._results['parameters']['dependencies']['view'] = \
         view_results['parameters']

        antenna_range = self._results['summary']['antenna_range']
        data_desc_range = self._results['summary']['data_desc_range']
        corr_axis = self._results['summary']['corr_axis']
        nchannels = self._results['summary']['nchannels']
        max_corr = 0
        for data_desc_id in data_desc_range:
           max_corr = max(max_corr, len(corr_axis[data_desc_id]))
        ndd = len(data_desc_range)

# iterate through the results, calculate the merit information

        merit = zeros([ndd, max_corr, max(antenna_range)+1, 
         max(antenna_range)+1], float)
        merit_flag = False

        for k,v in view_results['data'].iteritems():
            description = pickle.loads(k)
            field = description['method']['test_field_id']
            method = description['method']
            dd = description['DATA_DESC_ID']
            corr = list(corr_axis[dd]).index(description['CORR'])
            antenna1 = description['ANTENNA1']
            antenna2_range = v[-1]['y']

            flag = v[-1]['flag']
            flagVersions = v[-1]['flagVersions']
            data = v[-1]['data']
            data_stddev = v[-1]['mad_floor']

            if merit_flag == False:
                merit_flag = []
                for fi,fs in enumerate(flagVersions):
                    merit_flag.append(ones([ndd, max_corr, 
                     max(antenna_range)+1, max(antenna_range)+1], int))

            for antenna2 in antenna2_range:
                valid_data = compress(logical_not(flag[-1][antenna2,:]),
                 data[antenna2,:])
                valid_stddev = compress(logical_not(flag[-1][antenna2,:]),
                 data_stddev[antenna2,:])
                if len(valid_data) > 0:
                    data_median = median(valid_data)
                    merit[dd, corr, antenna1, antenna2] = \
                     merit[dd, corr, antenna2, antenna1] = sum(abs(
                     valid_data - data_median)) / sum(abs(valid_stddev))

# derive merit_flag for each flag version

                for fi,fs in enumerate(flagVersions):
                    merit_flag[fi][dd, corr, antenna1, antenna2] = \
                     merit_flag[fi][dd, corr, antenna2, antenna1] = all(
                     flag[fi][antenna2,:])

# loop through data_desc_ids and corr types, writing the data out to the
# result structure

        for dd in data_desc_range:
            if nchannels[dd] == 1:
                continue
            for corr in range(max_corr):
                description = {}
                title = '''Bandpass calibration merit - Field:%s Spw:%s Corr:%s''' % (
                 self._pad(field), self._pad(dd), self._pad(corr))

# build title to appear in link, formatted with html

#                title = '''Bandpass calibration merit - Field:%s Spw:%s 
#                 Corr:%s <ul>''' % (self._pad(field), self._pad(dd),
#                 self._pad(corr))
#                keys = method.keys()
#                keys.sort()
#                for k in keys:
#                    title += '''<li>%s: %s''' % (k, method[k])
#                title += '</ul>'
                description['TITLE'] = title 
                description['method'] = method
                description['FIELD_ID'] = field
                description['DATA_DESC_ID'] = dd
                description['CORR'] = corr

                result = {}
                result['dataType'] = 'figure of merit'
                result['dataUnits'] = 'sigma'
                result['xtitle'] = 'ANTENNA2'
                result['x'] = arange(max(antenna_range)+1)
                result['ytitle'] = 'ANTENNA1'
                result['y'] = arange(max(antenna_range)+1)
                result['y_interval'] = ones(shape(result['y']))
                result['data'] = merit[dd, corr]
                result['mad_floor'] = zeros(shape(merit[dd, corr]))
                temp = []
                for fi,fs in enumerate(flagVersions):
                    temp.append(merit_flag[fi][dd, corr])
                result['flag'] = temp
                result['flagVersions'] = flagVersions
                result['chunks'] = []

                pickled_description = pickle.dumps(description)
                if self._results['data'].has_key(pickled_description):
                    self._results['data'][pickled_description].append(result)
                else:
                    self._results['data'][pickled_description] = [result]

# add the flagging info - used by display objects

        flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()
        self._results['flagging'].append(flagging)
        self._results['flaggingReason'].append(flaggingReason)
        self._results['flaggingApplied'].append(flaggingApplied)

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('BandpassMerit.getdata')
        return temp


    def createGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = {'bandpass calibration':''}
        return description


    def createDetailedHTMLDescription(self, stageName, parameters=None):
        """Write a detailed description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        parameters -- The dictionary that holds the descriptive information.
        """

        if parameters == None:
            parameters = self._results['parameters']

        description = '''<h4>Figure of Merit</h4>
         Because a good bandpass calibration  
         should produce a flat corrected spectrum, the 'figure of merit' is
         a simple measure of spectral flatness derived
         using the following formula:<pre>

            sum(abs(data - median(data))) / sum(sigma)

         </pre>
         <p>The figure of merit was calculated by Python class BandpassMerit.'''

        description += """<p>""" + self._view.createDetailedHTMLDescription(
         stageName, parameters=parameters['dependencies']['view'])\
         ['bandpass calibration']

        meritDescription = {'bandpass calibration':description}
        return meritDescription


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a detailed description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        """

        description = self.createDetailedHTMLDescription(stageName,
         parameters=parameters)['bandpass calibration']

        if topLevel:
            self._htmlLogger.logHTML("""
             <h3>Data View</h3>
                 <p>The data view is a bandpass merit number.""")

        self._htmlLogger.logHTML(description)
