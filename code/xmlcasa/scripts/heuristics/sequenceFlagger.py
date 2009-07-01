"""Module that can calculate the statistics of data sequences and flag them 
using the results.
"""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Changed profiling.
# 30-Aug-2007 jfl Flux calibrating version.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 13-May-2008 jfl 13 release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# standard library modules

import copy as python_copy
import pickle

# package modules

from numpy import * 

# alma modules

from baseFlagger import *

class SequenceFlagger(BaseFlagger):
    """Class to allow detection and flagging of bad data in sequences."""

    def _add_flag_description(self, new_flags_set, new_flags, flag, y2flag,
     data_description, stageDescription, rule, flag_target_ids):
        """Utility method to construct a dictionary describing some
        new flags to be set and append it to new_flags.

        Keyword arguments: 
        """
        if len(y2flag) > 0:
            new_flags_set = True
            put(flag, y2flag, ones([len(y2flag)], int))
            flag_description = python_copy.deepcopy(data_description)
            field_id = int(flag_description['FIELD_ID'])
            if flag_target_ids.count(field_id) > 0:
                flag_description['FIELD_ID'] = flag_target_ids
            else:
                flag_description['FIELD_ID'] = [field_id] + flag_target_ids
            if rule.has_key('axis'):
                flag_description['rule'] = '%s - %s' % (rule['rule'],
                 rule['axis'])
            else:
                flag_description['rule'] = rule['rule']
            flag_description['colour'] = rule['colour']
            flag_description['stageName'] = stageDescription['name']
            flag_description[data_description['ytitle']] = take(
             data_description['y'], y2flag)
            if data_description.has_key('y_interval'):
                flag_description['%s_INTERVAL' % data_description['ytitle']] = \
                 take(data_description['y_interval'], y2flag)
            flag_description[data_description['xtitle']] = \
             [data_description['x']]

            new_flags[rule['rule']].append(flag_description)
        return 


    def _flagData(self, data, flag, no_data_flag, chunks, rules,
     data_description, stageDescription, flag_target_ids):
        """Utility method to calculate the statistics of a sequence
        and flag the data according to the specified 'rules'.

        Keyword arguments:
        data             - the data to be checked.
        flag             - the flags currentl on the data.
        no_data_flag     - flags on the data to signify there is _no_
                           contributing data in the MS; for example,
                           a baseline that was simply not measured.
        chunks           -
        rules            -
        data_description - dictionary describing the data.
        stageDescription - dictionary describing the stage calling
                           this method.
        flag_target_ids  - ids of fields to be flagged in the same way
                           as the data.
        """

        new_flags = {}
        for rule in self._rules:
            new_flags[rule['rule']] = []

        for mad_max_multiplier in [9, 3, 1]:
            new_flags_set = True
            while new_flags_set:
                new_flags_set = False
                valid_data = compress(logical_not(flag), data)
                data_median, data_mad = self.median_mad(valid_data)
                data_indeces = arange(len(data))

                chunk_median = zeros([len(chunks)], float)
                chunk_mad = zeros([len(chunks)], float)
                for ichunk, chunk in enumerate(chunks):
                    chunk_data = take(data, chunk)
                    chunk_flag = take(flag, chunk)
                    valid_data = compress(logical_not(chunk_flag),
                     chunk_data)
                    chunk_median[ichunk], chunk_mad[ichunk] = self.median_mad(
                     valid_data)

# flag data according to each rule in turn

                for ri, rule in enumerate(rules):
                    if rule['rule'] == 'outlier in chunk':
                        for ichunk,chunk in enumerate(chunks):

                            mad_max = mad_max_multiplier * rule['limit']
                            chunk_data = take(data, chunk)
                            chunk_flag = take(flag, chunk)        

                            min_N = rule['min_N']
                            if len(logical_not(chunk_flag)) < min_N:
                                continue

                            y2flag = compress(logical_and(
                             logical_not(chunk_flag),
                             abs(chunk_data - chunk_median[ichunk]) > 
                             mad_max * chunk_mad[ichunk]), chunk)

                            self._add_flag_description(new_flags_set,
                             new_flags, flag, y2flag, data_description,
                             stageDescription, rule, flag_target_ids)

                    elif rule['rule'] == 'outlier':
                        min_N = rule['min_N']
                        if len(logical_not(flag)) < min_N:
                            continue

                        mad_max = mad_max_multiplier * rule['limit']

                        y2flag = compress(logical_and(logical_not(flag),
                         abs(data - data_median) > mad_max * data_mad),
                         data_indeces)

                        self._add_flag_description(new_flags_set,
                         new_flags, flag, y2flag, data_description,
                         stageDescription, rule, flag_target_ids)

                    elif rule['rule'] == 'low outlier':
                        min_N = rule['min_N']
                        if len(logical_not(flag)) < min_N:
                            continue

                        mad_max = mad_max_multiplier * rule['limit']

                        y2flag = compress(logical_and(logical_not(flag),
                         (data_median - data) > mad_max * data_mad),
                         data_indeces)

                        self._add_flag_description(new_flags_set,
                         new_flags, flag, y2flag, data_description,
                         stageDescription, rule, flag_target_ids)

                    elif rule['rule'] == 'high outlier':
                        min_N = rule['min_N']
                        if len(logical_not(flag)) < min_N:
                            continue

                        mad_max = mad_max_multiplier * rule['limit']

                        y2flag = compress(logical_and(logical_not(flag),
                         (data - data_median) > mad_max * data_mad),
                         data_indeces)

                        self._add_flag_description(new_flags_set,
                         new_flags, flag, y2flag, data_description,
                         stageDescription, rule, flag_target_ids)

                    elif rule['rule'] == 'too many flags':
                        if all(flag):
                           continue

                        max_factor = rule['limit']
                        len_no_data = float(len(compress(no_data_flag, data)))
                        len_flagged = float(len(compress(flag, data)))
                        factor = (len_flagged - len_no_data) /\
                         (float(len(data)) - len_no_data)

                        if factor > max_factor:
                            y2flag = compress(logical_not(flag), data_indeces)
                            self._add_flag_description(new_flags_set,
                             new_flags, flag, y2flag, data_description,
                             stageDescription, rule, flag_target_ids)

                    else:           
                        raise NameError, 'bad rule: %s' % rules[ri]

        return new_flags


    def operate(self, stageDescription, dataView):
        """Public method to apply the flag rules to the data 'view'.
        
        Keyword arguments:
        stageDescription -- Dictionary with a description of the calling 
                            reduction stage.
        dataView         -- Object providing the data 'view'.
        """

#        print 'SequenceFlagger.operate called'
        self._htmlLogger.timing_start('sequenceFlagger.operate')

# get list of field ids to which flags are to be applied

        flag_target_ids = []
        for flag_target in self._flag_targets:
            target_ids = dataView.getFieldsOfType(flag_target)
            for target_id in target_ids:
                if not(flag_target_ids.count(target_id)):
                    flag_target_ids.append(target_id)

# add these to the list of fields potentially flagged

        for flag_target in flag_target_ids:
            if self._potentially_flagged_target_ids.count(flag_target) == 0:
                self._potentially_flagged_target_ids.append(flag_target) 

# loop until no further flags raised

        flagging = True
        while flagging:
            flags = {}
            for rule in self._rules:
                flags[rule['rule']] = []

# get latest data

            collection = dataView.getData()['data']
            for key, results in collection.iteritems():
                if len(results[-1]) == 0:
                    continue
                description = pickle.loads(key)

# add field to those potentially being flagged

                newField = int(description['FIELD_ID'])
                if self._potentially_flagged_target_ids.count(newField) == 0:
                    self._potentially_flagged_target_ids.append(newField) 

                x = results[-1]['x']
                y = results[-1]['y']
                if results[-1].has_key('y_interval'):
                    y_interval = results[-1]['y_interval']
                else:
                    y_interval = None
                xtitle = results[-1]['xtitle']
                ytitle = results[-1]['ytitle']
                chunks = results[-1]['chunks']
                data2d = results[-1]['data']

# get current flags on data
                
                flagVersions = results[-1]['flagVersions']
                if flagVersions.count('Current') == 0:
                    raise NameError, 'no Current flag version'
                else:
                    i_current = flagVersions.index('Current')
                    flag2d = results[-1]['flag'][i_current]
        
# and get flags signalling 'no data'
                
                if flagVersions.count('NoData') == 0:  
                    raise NameError, 'no NoData flag version'
                else:
                    i_nodata = flagVersions.index('NoData')
                    no_data_flag2d = results[-1]['flag'][i_nodata]

# run set of rules that apply to sequence in y-axis, if any

                y_rules = []
                for rule in self._rules:
                    if rule['axis'] == ytitle:
                         y_rules.append(rule)
                if len(y_rules):
                    for ix, xval in enumerate(x):
                        data_description = python_copy.deepcopy(description)
                        data_description['xtitle'] = xtitle
                        data_description['x'] = xval
                        data_description['ytitle'] = ytitle
                        data_description['y'] = y
                        if y_interval != None:
                           data_description['y_interval'] = y_interval
                        data = data2d[:, ix]
                        flag = flag2d[:, ix]
                        no_data_flag = no_data_flag2d[:,ix]
                        new_flags = self._flagData(data, flag, no_data_flag,
                         chunks, y_rules, data_description, stageDescription,
                         flag_target_ids)
                        for rule in y_rules:
                            flags[rule['rule']] += new_flags[rule['rule']]

# likewise for rules that apply to sequence in x-axis

                x_rules = []
                for rule in self._rules:
                    if rule['axis'] == xtitle:
                         x_rules.append(rule)
                if len(x_rules):
                    for iy, yval in enumerate(y):
                        data_description = python_copy.deepcopy(description)
                        data_description['xtitle'] = ytitle
                        data_description['x'] = yval
                        data_description['ytitle'] = xtitle
                        data_description['y'] = x
                        data = data2d[iy, :]
                        flag = flag2d[iy, :]
                        no_data_flag = no_data_flag2d[iy,:]
                        new_flags = self._flagData(data, flag, no_data_flag, 
                         [], x_rules, data_description, stageDescription,
                         flag_target_ids)
                        for rule in x_rules:
                            flags[rule['rule']] += new_flags[rule['rule']]

# set any flags raised, stop flagging if none

            n_flags = self._sumFlags(flags)
            print 'number of flagging commands raised %s' % n_flags
            dataView.setFlags(stageDescription, self._rules, flags)
            if n_flags == 0:
                flagging = False
                break

        self._htmlLogger.timing_stop('sequenceFlagger.operate')


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        """

# write out the description

        self._htmlLogger.logHTML('''
         The statistics of each data 'view' were
         calculated and errant pixels flagged according to the
         rules listed below.<ul>''')
        for rule in self._rules:
            description = 'No description available for flagging rule: %s' % (
             rule['rule'])
            if rule['rule'] == 'outlier':
                description = '''
                 The data view was sliced up into 1-d sequences running
                 along the %s axis. For each sequence the median was 
                 calculated and the %s of the contributing points. 
                 Points were flagged if they lay further than %s
                 times the MAD from the median.''' % (rule['axis'],
                 self._htmlLogger.glossaryLink('MAD'), rule['limit'])
            elif rule['rule'] == 'low outlier':
                description = '''
                 The data view was sliced up into 1-d sequences running
                 along the %s axis. For each sequence the median was 
                 calculated and the %s of the contributing points. 
                 Points were flagged if they lay further than %s
                 times the MAD <b>below</b> the median.''' % (rule['axis'],
                 self._htmlLogger.glossaryLink('MAD'), rule['limit'])
            elif rule['rule'] == 'high outlier':
                description = '''
                 The data view was sliced up into 1-d sequences running
                 along the %s axis. For each sequence the median was 
                 calculated and the %s of the contributing points. 
                 Points were flagged if they lay further than %s
                 times the MAD <b>above</b> the median.''' % (rule['axis'],
                 self._htmlLogger.glossaryLink('MAD'), rule['limit'])
            elif rule['rule'] == 'outlier in chunk':
                description = '''
                 The data view was sliced up into 1-d arrays running
                 along the %s axis. These arrays were broken into data
                 sequences running between %s boundaries. For each sequence 
                 the median is calculated and the %s of the contributing points.
                 Points were flagged if they lay further than %s times  
                 the MAD from the median.''' % (rule['axis'],  
                 self._htmlLogger.glossaryLink('chunk'),
                 self._htmlLogger.glossaryLink('MAD'), rule['limit'])
            elif rule['rule'] == 'too many flags':
                description = '''
                 If more than %s of the points along any slice along the %s axis
                 were flagged then all points in the sequence were
                 flagged.''' % (rule['limit'], rule['axis'])

            self._htmlLogger.logHTML('<li> %s' % description)
        self._htmlLogger.logHTML('</ul>')


    def writeDetailedHTMLDescription(self, stageName, parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        parameters -- Dictionary that holds the descriptive information.
        """

        self._htmlLogger.logHTML('<h3>Data Flagging</h3>')
        self._htmlLogger.logHTML('''
         The data view was a 2-d image. The statistics of the image were
         calculated and errant pixels flagged according to the
         rules listed below. Flagged pixels were mapped back to set the
         FLAG_ROW values of the contributing elements in
         the MeasurementSet <ul>''')

        for rule in self._rules:
            description = 'No description available for flagging rule: %s' % (
             rule['rule'])

            if rule['rule'] == 'outlier':
                description = '''
                 The data sequences were slices along the data axis specified
                 by the 'axis' parameter. For each sequence
                 the median was calculated and from it the %s of the 
                 contributing points. Points were flagged if they lay
                 further than 'limit' times the MAD from the median.
                 In this case 'axis' was set to %s and 'limit' to %s.
                 <p>The algorithm looped, recalculating the sequence statistics
                 each time, until no new flags were raised.''' % (
                 self._htmlLogger.glossaryLink('MAD'),
                 rule['axis'], rule['limit'])

            elif rule['rule'] == 'low outlier':
                description = '''
                 The data sequences were slices along the data axis specified
                 by the 'axis' parameter. For each sequence
                 the median was calculated and from it the %s of the 
                 contributing points. Points were flagged if they lay
                 further than 'limit' times the MAD <b>below</b> the median.
                 In this case 'axis' was set to %s and 'limit' to %s.
                 <p>The algorithm looped, recalculating the sequence statistics
                 each time, until no new flags were raised.''' % (
                 self._htmlLogger.glossaryLink('MAD'),
                 rule['axis'], rule['limit'])

            elif rule['rule'] == 'outlier':
                description = '''
                 The data sequences were slices along the data axis specified
                 by the 'axis' parameter. For each sequence
                 the median was calculated and from it the %s of the 
                 contributing points. Points were flagged if they lay
                 further than 'limit' times the MAD <b>above</b> the median.
                 In this case 'axis' was set to %s and 'limit' to %s.
                 <p>The algorithm looped, recalculating the sequence statistics
                 each time, until no new flags were raised.''' % (
                 self._htmlLogger.glossaryLink('MAD'),
                 rule['axis'], rule['limit'])

            elif rule['rule'] == 'outlier in chunk':
                description = '''
                 The data view was sliced up into 1-d arrays running
                 along the axis specified in 'axis'. These arrays were 
                 broken into data
                 sequences running between %s boundaries. For each sequence 
                 the median was calculated and the %s of the 
                 contributing points.
                 Points were flagged if they lay further than 'limit' times  
                 the MAD from the median.
                 In this case 'axis' was set to %s and 'limit' to %s.
                 <p>The algorithm looped, recalculating the sequence statistics
                 each time, until no new flags were raised.''' % (
                 self._htmlLogger.glossaryLink('chunk'),
                 self._htmlLogger.glossaryLink('MAD'),
                 rule['axis'], rule['limit'])
            elif rule['rule'] == 'too many flags':
                description = '''
                 The data view was sliced up into 1-d arrays running
                 along the axis specified in 'axis'. If the proportion of
                 points flagged in a sequence was greater than 'limit'
                 then the entire sequence was flagged.
                 In this case 'axis' was set to %s and 'limit' to %s.
                 <p>The algorithm looped, recalculating the sequence statistics
                 each time, until no new flags were raised.''' % (
                 rule['axis'], rule['limit'])
 
            self._htmlLogger.logHTML('<li> %s' % description)

        self._htmlLogger.logHTML('''</ul>
        <p>The flagging was performed by Python class SequenceFlagger.''')
