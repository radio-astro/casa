"""Module with classes to select best method of obtaining some kind of data.
"""

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

# package modules

from numpy import *
import os
import pickle

# alma modules

from baseDataModifier import *

def _table_compare(x, y):
    if x['rank'] < y['rank']:
        return -1
    elif x['rank'] > y['rank']:
        return 1
    else:
        return 0


class BestMethodSelector(BaseDataModifier):
    """Class to select best method of obtaining some kind of data.
    """

    def description(self):
        description = self._view.description()
        description.insert(0,
         'BestMethodSelector - select method with lowest figure of merit')
        return description


    def getData(self, topLevel=False):
        """Public method to take a range of results and select the one with
        the lowest merit value.

        Keyword arguments:
        topLevel -- True if this is the data 'view' to be displayed directly.
        """

#        print 'BestMethodSelector.getData called'
        self._htmlLogger.timing_start('BestMethodSelector.getData')

# This class does not use the BookKeeper to store results as it seems unlikely
# it will increase performance in this case.

# get the range of results to be selected from and build up the result table

        view_results = self._view.getData()

        self._results['parameters'] = {'history':self._fullStageName}
        self._results['parameters']['dependencies'] = {'merit':
         view_results['parameters']}

        table_entries = []
        collection = view_results['data']
        for pickled_description,results in collection.iteritems():
            if not(results[-1].has_key('dataType')):
                continue
            if results[-1]['dataType'] != 'figure of merit':
                continue
            entry = pickle.loads(pickled_description)
            merit_data = ravel(results[-1]['data'])
            merit_flag = ravel(results[-1]['flag'][-1])
            valid_data = compress(merit_flag==0, merit_data)
            if len(valid_data):
                entry['merit_mean'] = mean(valid_data)
                entry['merit_std'] = std(valid_data)
                entry['merit_median'] = median(valid_data)
            entry['rank'] = entry['TITLE']
            entry['pickled'] = pickled_description
            entry['methodForEachSpw'] = results[-1]['methodForEachSpw'] 
            table_entries.append(entry)

# sort the table entries on 'title', containing field, spw, Gain t and method

        table_entries.sort(_table_compare)

# find the lowest merit value for each SpW

        minVal = {}
        minIndex = {}
        for i,entry in enumerate(table_entries):
            if not entry.has_key('merit_mean'):
                continue

            if minVal.has_key(entry['DATA_DESC_ID']):
                if entry['merit_mean'] < minVal[entry['DATA_DESC_ID']]:
                    minIndex[entry['DATA_DESC_ID']] = i
                    minVal[entry['DATA_DESC_ID']] = entry['merit_mean']
            else:
                minIndex[entry['DATA_DESC_ID']] = i
                minVal[entry['DATA_DESC_ID']] = entry['merit_mean']

# generate the table in HTML

        description = """
         <h4 align="left">Merit Results</h4>
         <table CELLPADDING="5" BORDER="1"
          <tr>
           <th>Spw</th>
           <th>Corr</th>
           <th>Method</th>
           <th>Merit mean</th>
           <th>Merit std</th>
           <th>Merit median</th>
           </tr>"""

        for i,entry in enumerate(table_entries):
            description += '<tr>'
            description += '<td>%s</td>' % entry['DATA_DESC_ID']
            description += '<td>%s</td>' % entry['CORR']
            description += '<td>%s</td>' % entry['method']
            if i in minIndex.values():
                description += '''
                 <td bgcolor="lawngreen">%s</td>''' % entry['merit_mean']
                description += '<td>%s</td>' % entry['merit_std']
                description += '<td>%s</td>' % entry['merit_median']
            else:
                if entry.has_key('merit_mean'):
                    description += '<td>%s</td>' % entry['merit_mean']
                    description += '<td>%s</td>' % entry['merit_std']
                    description += '<td>%s</td>' % entry['merit_median']
            description += '</tr>'
        description += '</table>'
        self._resultsDescription = description

# output the calibration method to use for each SpW

        bestMethodToUse = {}
        for key in minIndex.keys():
            bestMethodToUse[int(key)] = {}
            bestMethodToUse[int(key)] = \
             table_entries[minIndex[key]]['methodForEachSpw'][key]
        f = open('B_calibration.lis', 'w') 
        pickle.dump(bestMethodToUse, f)
        f.close()

# output the B calibration info to HTML

        bestMethodDescription = '''
         <table CELLPADDING="5" BORDER="1"
          <tr>
           <th>Spw</th>'''

        for k in bestMethodToUse.values()[0].keys():
            bestMethodDescription += '<th>%s</th>' % k
        bestMethodDescription += '</tr>'

        for k,v in bestMethodToUse.iteritems():
            bestMethodDescription += '''<tr>
             <td>%s</td>''' % k
            for w in v.values(): 
                bestMethodDescription += '<td>%s</td>' % w
            bestMethodDescription += '</tr>'
        bestMethodDescription += '</table>'
        self._bestMethodDescription = bestMethodDescription

# go through results and delete the sub-optimal entries

        min_descriptions = []
        for v in minIndex.values():
            min_descriptions.append(table_entries[v]['pickled'])

        collection_copy = collection.copy()
        for pickled_description in collection_copy.keys():
            if min_descriptions.count(pickled_description) == 0:
                ignore = collection.pop(pickled_description)

# data result

        self._results['data'] = view_results['data']

# add the flagging info - used by display objects

        flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()
        self._results['flagging'].append(flagging)
        self._results['flaggingReason'].append(flaggingReason)
        self._results['flaggingApplied'].append(flaggingApplied)

# return a copy of the data list

        temp = python_copy.deepcopy(self._results)
        self._htmlLogger.timing_stop('BestMethodSelector.getData')
        return temp


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description to html of the class.
        """

# this class modifies data from other classes and it's difficult to construct
# automatically a readable general description of the 'view'. If possible,
# such a description should be written as part of the recipe.

        if self._description != None:
            self._htmlLogger.logHTML('<p> %s' % self._description)

        self._htmlLogger.logHTML('''<p>The data view shown was calculated
        using the following parameters:''')
        self._htmlLogger.logHTML(self._bestMethodDescription)


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a detailed description to html of the class.
        
        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        """

        if topLevel:
            self._htmlLogger.logHTML('<h3>Data View</h3>')

# this class modifies data from other classes and it's difficult to construct
# automatically a readable general description of the 'view'. If possible,
# such a description should be written as part of the recipe.

        if self._description != None:
            self._htmlLogger.logHTML('<p> %s' % self._description)

# best method description

        self._htmlLogger.logHTML('''<p>The data view shown was 
        calculated using the best method found in the tests, which
        was the one using the following parameters:''')
        self._htmlLogger.logHTML(self._bestMethodDescription)

        self._htmlLogger.logHTML("""
         where:
         <ul>
          <li>G_t is the value of t used to calculate
              a phase-only G calibration of the bandpass data, which is applied
              to phase up that data before the bandpass calibration itself is
              calculated.
          <li>Mode is the type of bandpass solution obtained; 'CHANNEL',
              or 'POLYNOMIAL'.
          <li>'channel flags' gives the name of the stage that specified the
              channels to be flagged before solving, if any.
          <li>'Apply Table' is the name of the file containing the bandpass
              solution.
          <li>'Apply Type' gives the casapy type of the bandpass solution.
          <li>'Casapy calls' links to a list of the casapy calls used
              to calculate the bandpass solution.
          <li>'error?' describes any error that occurred during the
              calculation.
         </ul>""")
 
        self._htmlLogger.logHTML('''<p>
         The best calibration was the one in the following range of test 
         calibrations that had the lowest figure of merit.
         <p>The view was calculated by Python class BestMethodSelector.''')

        self._htmlLogger.logHTML(self._resultsDescription)

        self._view.writeDetailedHTMLDescription(stageName, False)
