"""Module allowing pixels in a 2-d array to be flagged according to specified
rules and the image statistics. 
"""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Change profiling.
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
#  2-Jun-2009 jfl line and continuum release.

# standard library modules

import copy as python_copy
import pickle

# package modules

from numpy import * 
import pylab

# alma modules

from baseFlagger import *


class ImageFlagger(BaseFlagger):
    """Class to allow detection and flagging of bad data in images."""

    def _add_flag_description(self, new_flags_set, new_flags, flag,
     cut_pos, p2flag,
     cut_axis_title_field, cut_axis_value_field, cut_axis_interval_field,
     position_axis_title_field, position_axis_value_field, 
     position_axis_interval_field, 
     data_description, stageDescription, rule, flag_target_ids):
        """Utility method to construct a dictionary describing some
        new flags to be set and append it to new_flags.

        Keyword arguments:
        new_flags_set                -- returned True if any flags are 
                                        set.  
        new_flags                    -- returned dictionary. Each rule is a 
                                        key and each value a list of the
                                        flags set by the rule.
        flag                         -- The flag values of the sample. 
        cut_pos                      -- The position of the 'cut' in the image.
        p2flag                       -- Indeces of pixels to flag.
        cut_axis_title_field         -- Name of field holding 'cut' axis name.
        cut_axis_value_field         -- Name of field holding 'cut' axis values.
        cut_axis_interval_field      -- Name of field (if any) holding intervals
                                        (pixel widths) on 'cut' axis.
        position_axis_title_field    -- Name of field holding the name of the
                                        other axis (perp to cut).
        position_axis_value_field    -- Name of field holding values of other
                                        axis.
        position_axis_interval_field -- Name of field (if any) holding intervals
                                        of other axis.
        data_description             -- Dictionary with a description of the 
                                        sample data.
        stageDescription             -- Dictionary with a description of the
                                        calling stage.
        rule                         -- Dictionary with a description of the
                                        rule being applied.
        flag_target_ids              -- List of targets to be flagged in
                                        addition to the sample.
        """
        if len(p2flag) > 0:
            new_flags_set = True
            put(flag, p2flag, ones([len(p2flag)], int))
            flag_description = {}
            if data_description.has_key('ANTENNA1'):
                flag_description['ANTENNA1'] = \
                 [int(data_description['ANTENNA1'])]
            flag_description['DATA_DESC_ID'] = int(
             data_description['DATA_DESC_ID'])
            field_id = int(data_description['FIELD_ID'])
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

            flag_description[data_description[cut_axis_title_field]] = list(
             take(data_description[cut_axis_value_field], p2flag))
            if data_description.has_key(cut_axis_interval_field):
                flag_description['%s_INTERVAL' % 
                 data_description[cut_axis_title_field]] = take(
                 data_description[cut_axis_interval_field], p2flag)

            flag_description[data_description[position_axis_title_field]] = [
             data_description[position_axis_value_field][cut_pos]]
            if data_description.has_key(position_axis_interval_field):
                flag_description['%s_INTERVAL' %
                 data_description[position_axis_title_field]] = [
                 data_description[position_axis_interval_field][cut_pos]]

            new_flags[rule['rule']].append(flag_description)
        return 


    def _flagData(self, data, flag, no_data_flag, data_description, 
     stageDescription, flag_target_ids):
        """Utility method to calculate the statistics of an image
        and flag the data according to the specified 'rules'.

        Keyword arguments:
        data             - the data to be checked.
        flag             - the flags currentl on the data.
        no_data_flag     - flags on the data to signify there is _no_
                           contributing data in the MS; for example,
                           a baseline that was simply not measured.
        data_description - dictionary describing the data.
        stageDescription - dictionary describing the stage calling
                           this method.
        flag_target_ids  - ids of fields to be flagged in the same way
                           as the data.
        """

        new_flags = {}
        for rule in self._rules:
            new_flags[rule['rule']] = []

        rdata = ravel(data)
        rflag = ravel(flag)

        for mad_max_multiplier in [9, 3, 1]:
            new_flags_set = True
            while new_flags_set:
                new_flags_set = False
                valid_data = compress(logical_not(rflag), rdata)
                data_median, data_mad = self.median_mad(valid_data)

# flag data according to each rule in turn

                for rule in self._rules:
                    if rule['rule'] == 'outlier':
                        mad_max = mad_max_multiplier * rule['limit']

                        data_indeces = arange(shape(data)[0])
                        for ix in range(shape(data)[1]):
                            y2flag = compress(
                             logical_and(logical_not(flag[:,ix]),
                             abs(data[:,ix] - data_median) > 
                             mad_max * data_mad), data_indeces)

                            self._add_flag_description(new_flags_set,
                             new_flags, flag[:,ix], ix, y2flag,
                             'ytitle', 'y', 'y_interval',
                             'xtitle', 'x', 'x_interval',
                             data_description, stageDescription, rule,
                             flag_target_ids)

                    elif rule['rule'] == 'low outlier':
                        mad_max = mad_max_multiplier * rule['limit']

                        data_indeces = arange(shape(data)[0])
                        for ix in range(shape(data)[1]):
                            y2flag = compress(
                             logical_and(logical_not(flag[:,ix]),
                             (data_median - data[:,ix]) > 
                             mad_max * data_mad), data_indeces)

                            self._add_flag_description(new_flags_set,
                             new_flags, flag[:,ix], ix, y2flag,
                             'ytitle', 'y', 'y_interval',
                             'xtitle', 'x', 'x_interval',
                             data_description, stageDescription, rule,
                             flag_target_ids)

                    elif rule['rule'] == 'high outlier':
                        mad_max = mad_max_multiplier * rule['limit']

                        data_indeces = arange(shape(data)[0])
                        for ix in range(shape(data)[1]):
                            y2flag = compress(
                             logical_and(logical_not(flag[:,ix]),
                             (data[:,ix] - data_median) > 
                             mad_max * data_mad), data_indeces)

                            self._add_flag_description(new_flags_set,
                             new_flags, flag[:,ix], ix, y2flag,
                             'ytitle', 'y', 'y_interval',
                             'xtitle', 'x', 'x_interval',
                             data_description, stageDescription, rule,
                             flag_target_ids)

                    elif rule['rule'] == 'min abs':
                        mad_max = rule['limit']

                        data_indeces = arange(shape(data)[0])
                        for ix in range(shape(data)[1]):
                            y2flag = compress(
                             logical_and(logical_not(flag[:,ix]),
                             abs(data[:,ix]) < mad_max),
                             data_indeces)

                            self._add_flag_description(new_flags_set,
                             new_flags, flag[:,ix], ix, y2flag,
                             'ytitle', 'y', 'y_interval',
                             'xtitle', 'x', 'x_interval',
                             data_description, stageDescription, rule,
                             flag_target_ids)

                    elif rule['rule'] == 'max abs':
                        mad_max = rule['limit']

                        data_indeces = arange(shape(data)[0])
                        for ix in range(shape(data)[1]):
                            y2flag = compress(
                             logical_and(logical_not(flag[:,ix]),
                             abs(data[:,ix]) > mad_max),
                             data_indeces)

                            self._add_flag_description(new_flags_set,
                             new_flags, flag[:,ix], ix, y2flag,
                             'ytitle', 'y', 'y_interval',
                             'xtitle', 'x', 'x_interval',
                             data_description, stageDescription, rule,
                             flag_target_ids)

                    elif rule['rule'] == 'too many flags':
                        max_factor = rule['limit']
                        axis = rule['axis']

# careful here. Dims of data array are [y,x]. Confusing should be changed.

                        if axis == data_description['xtitle']:
                            data_indeces = arange(shape(flag)[1])
                            len_data = shape(flag)[1]

                            for i in arange(shape(data)[0]):
                                if all(flag[i,:]):
                                    continue

                                len_no_data = float(len(compress(
                                 no_data_flag[i,:], data[i,:])))
                                len_flagged = float(len(compress(flag[i,:],
                                 data[i,:])))
                                factor = (len_flagged - len_no_data) /\
                                 (len_data - len_no_data)

                                if factor > max_factor:
                                    i2flag = compress(logical_not(flag[i,:]),
                                     data_indeces)

                                    self._add_flag_description(new_flags_set,
                                     new_flags, flag[i,:], i, i2flag,
                                     'xtitle', 'x', 'x_interval',
                                     'ytitle', 'y', 'y_interval',
                                     data_description, stageDescription, rule,
                                     flag_target_ids)

                        elif axis == data_description['ytitle']:
                            data_indeces = arange(shape(flag)[0])
                            len_data = shape(flag)[0]

                            for i in arange(shape(data)[1]):
                                if all(flag[:,i]):
                                    continue

                                len_no_data = float(len(compress(
                                 no_data_flag[:,i], data[:,i])))
                                len_flagged = float(len(compress(flag[:,i],
                                 data[:,i])))
                                factor = (len_flagged - len_no_data) /\
                                 (len_data - len_no_data)

                                if factor > max_factor:
                                    i2flag = compress(logical_not(flag[:,i]),
                                     data_indeces)

                                    self._add_flag_description(new_flags_set,
                                     new_flags, flag[:,i], i, i2flag,
                                     'ytitle', 'y', 'y_interval',
                                     'xtitle', 'x', 'x_interval',
                                     data_description, stageDescription, rule,
                                     flag_target_ids)

                    else:           
                        raise NameError, 'bad rule: %s' % rule

        return new_flags


    def operate(self, stageDescription, dataView):
        """Public method to apply the flag rules to the data 'view'.

        Keyword arguments:
        stageDescription -- Dictionary with a description of the calling
                            reduction stage.
        dataView         -- Object providing the data 'view'.
        """
#
#        print 'ImageFlagger.operate called'
        self._htmlLogger.timing_start('imageFlagger.operate')

# get list of field ids to which flags are to be applied

        flag_target_ids = []
        for flag_target in self._flag_targets:
            target_ids = dataView.getFieldsOfType(flag_target)
            for target_id in target_ids:
                if not(flag_target_ids.count(target_id)):
                    flag_target_ids.append(target_id)

# add these to list of fields potentially flagged

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
            for key,results in collection.iteritems():
                description = pickle.loads(key)

# add field to those potentially being flagged

                newField = int(description['FIELD_ID'])
                if self._potentially_flagged_target_ids.count(newField) == 0:
                    self._potentially_flagged_target_ids.append(newField)

                data = results[-1]['data']

# get current flags on data

                flagVersions = results[-1]['flagVersions']
                if flagVersions.count('Current') == 0:
                    raise NameError, 'no Current flag version'
                else:
                    i_current = flagVersions.index('Current')
                    flag = results[-1]['flag'][i_current]

# and get flags signalling 'no data'

                if flagVersions.count('NoData') == 0:
                    raise NameError, 'no NoData flag version'
                else:
                    i_nodata = flagVersions.index('NoData')
                    no_data_flag = results[-1]['flag'][i_nodata]

# run set of rules 

                data_description = python_copy.deepcopy(description)
                data_description['xtitle'] = results[-1]['xtitle']
                data_description['x'] = results[-1]['x']
                data_description['ytitle'] = results[-1]['ytitle']
                data_description['y'] = results[-1]['y']
                data_description['y_interval'] = results[-1]['y_interval']
                new_flags = self._flagData(data, flag, no_data_flag, 
                 data_description, stageDescription, flag_target_ids)
                for rule in self._rules:
                    flags[rule['rule']] += new_flags[rule['rule']]

# set any flags raised, stop flagging if none

            n_flags = self._sumFlags(flags)
            print 'number of flagging commands raised %s' % n_flags
            dataView.setFlags(stageDescription, self._rules, flags)
            if n_flags == 0:
                flagging = False
                break

        self._htmlLogger.timing_stop('imageFlagger.operate')


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        """

# write out the description

        self._htmlLogger.logHTML('''
         Errant pixels were detected and flagged according to the
         rules listed below. The flagging was done in a loop; in each
         iteration the data 'view' would be calculated, the rule(s) would 
         be applied, the data would be flagged. This cycle continued until 
         no new flags were raised. If more than one rule was used then 
         for each flagging pass the rules were applied in no particular 
         order.<ul>''')
        for rule in self._rules:
            description = 'No description available for flagging rule: %s' % (
             rule['rule'])
            if rule['rule'] == 'outlier':
                description = '''
                 The median of the image was calculated and the %s of the 
                 pixels. Points were flagged if they lay further than %s 
                 times the MAD from the median.''' % (
                 self._htmlLogger.glossaryLink('MAD'), rule['limit'])
            elif rule['rule'] == 'low outlier':
                description = '''
                 The median of the image was calculated and the %s of the 
                 pixels. Points were flagged if they lay further than %s 
                 times the MAD <b>below</b> the median.''' % (
                 self._htmlLogger.glossaryLink('MAD'), rule['limit'])
            elif rule['rule'] == 'high outlier':
                description = '''
                 The median of the image was calculated and the %s of the 
                 pixels. Points were flagged if they lay further than %s 
                 times the MAD <b>above</b> the median.''' % (
                 self._htmlLogger.glossaryLink('MAD'), rule['limit'])
            elif rule['rule'] == 'min abs':
                description = '''
                 Points were flagged if their absolute value was less than 
                 than %s.''' % rule['limit']
            elif rule['rule'] == 'max abs':
                description = '''
                 Points were flagged if their absolute value was greater than 
                 than %s.''' % rule['limit']
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
         The data view is a 2-d image. 
         Errant pixels were detected and flagged according to the
         rules listed below. The flagging was done in a loop; in each
         iteration the data 'view' would be calculated, the rule(s) would 
         be applied, the data would be flagged. This cycle continued until 
         no new flags were raised. If more than one rule was used then 
         for each flagging pass the rules were applied in no particular 
         order. Flagged pixels were mapped back to set the 
         FLAG_ROW values of the contributing elements in 
         the MeasurementSet <ul>''')

        for rule in self._rules:
            description = 'No description available for flagging rule: %s' % (
             rule['rule'])
            if rule['rule'] == 'outlier':
                description = '''For each image the median was calculated 
                 and from it the %s of the
                 contributing points. Points were flagged if they lie
                 further than 'limit' times the %s from the median.
                 The algorithm looped, recalculating the view and image 
                 statistics each time, until no new flags were raised. In this
                 case 'limit' was set to %s''' % (
                  self._htmlLogger.glossaryLink('MAD'),
                  self._htmlLogger.glossaryLink('MAD'), rule['limit'])

            elif rule['rule'] == 'low outlier':
                description = '''For each image the median was calculated 
                 and from it the %s of the
                 contributing points. Points were flagged if they lay
                 further than 'limit' times the %s <b>below</b> the median.
                 The algorithm looped, recalculating the view and image 
                 statistics each time, until no new flags were raised. In this
                 case 'limit' was set to %s''' % (
                  self._htmlLogger.glossaryLink('MAD'),
                  self._htmlLogger.glossaryLink('MAD'), rule['limit'])

            elif rule['rule'] == 'high outlier':
                description = '''For each image the median was calculated 
                 and from it the %s of the
                 contributing points. Points were flagged if they lay
                 further than 'limit' times the %s <b>above</b> the median.
                 The algorithm looped, recalculating the view and image 
                 statistics each time, until no new flags were raised. In this
                 case 'limit' was set to %s''' % (
                  self._htmlLogger.glossaryLink('MAD'),
                  self._htmlLogger.glossaryLink('MAD'), rule['limit'])

            elif rule['rule'] == 'min abs':
                description = '''Points were flagged if their absolute value 
                 was less than 'limit'.
                 The algorithm looped, recalculating the view
                 each time, until no new flags were raised. In this case
                 'limit' was set to %s''' % rule['limit']

            elif rule['rule'] == 'max abs':
                description = '''Points were flagged if their absolute value 
                 was greater than 'limit'.
                 The algorithm looped, recalculating the view
                 each time, until no new flags were raised. In this case
                 'limit' was set to %s''' % rule['limit']

            elif rule['rule'] == 'too many flags':
                description = '''Looking at data slices along the axis 
                 specified in the 'axis' parameter, if the proportion
                 of flagged points relative to the total number of points 
                 exceeded the value in 'limit' then all points in the sequence
                 were flagged. In this case the 'axis' was %s and the 'limit'
                 %s''' % (rule['axis'], rule['limit'])
 
            self._htmlLogger.logHTML('<li> %s' % description)

        self._htmlLogger.logHTML('''</ul>''')
#        self._htmlLogger.logHTML('''</ul>
#        <p>The flagging was performed by Python class ImageFlagger.''')

