"""Module with classes to detect and flag band edges."""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Change profiling, add place holder for PdB edge detection.
# 28-Nov-2007 jfl Recipe release.
# 17-Dec-2007 jfl Add PdB edge detection.
# 13-May-2008 jfl 13 release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Sep-2008 jfl msCalibrater release.
# 10-Oct-2008 jfl complex display release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# standard library modules

import pickle
import copy as python_copy

# package modules

from numpy import * 

# alma modules

from baseFlagger import *

class BandpassEdgeFlagger(BaseFlagger):
    """Class to detect and flag band edges."""

    def _add_flag_description(self, new_flags, flag, y2flag,
     data_description, stageDescription, rule, flag_target_ids):
        """Utility method to add a dictionary description of flags to be set
        to the total list.
        """
        if len(y2flag) > 0:
            flag_description = python_copy.deepcopy(data_description)
            field_id = int(flag_description['FIELD_ID'])
            if flag_target_ids.count(field_id) > 0:
                flag_description['FIELD_ID'] = flag_target_ids
            else:
                flag_description['FIELD_ID'] = [field_id] + flag_target_ids
            flag_description['rule'] = rule['rule']
            flag_description['colour'] = rule['colour']
            flag_description['stageName'] = stageDescription['name']
            flag_description['CHANNELS'] = y2flag

            new_flags[rule['rule']].append(flag_description)


    def _chi_squared(self, data, flag, stddev, profile, 
                     prof_centre_chan, profile_amp):
        """Return chi-squared between data and profile.

        Keyword arguments:
        data -- the data against which the profile is to be checked
        flag -- data flags, set True if datum is to be ignored
        stddev -- the standard deviation of each datum
        profile -- the normalised profile being tested
        prof_centre_chan -- the data index at which the profile is centred
        profile_amp -- the amplitude by which the normalised profile
                       is multiplied before chi-squared is calculated 
        """

# resample profile to match data sampling; data_i is distance from 
# profile centre in data pixels

        fit_profile = array(profile)
        centre = (len(fit_profile)-1)/2
        for data_i in range(1, centre+1):

# interpolate for lower half of profile
           
            profile_i = centre - float(data_i)
            profile_i_below = int(floor(profile_i))
            profile_i_above = profile_i_below + 1
            if profile_i_above <= 0:
                fit_profile[int(centre - data_i)] = profile[0]
            else:
                fit_profile[centre - data_i] = profile[profile_i_below] +\
                 (profile[profile_i_above] - profile[profile_i_below]) *\
                 (float(profile_i - profile_i_below) / 
                 float(profile_i_above - profile_i_below))

# likewise for upper half of profile

            profile_i = centre + float(data_i)
            profile_i_below = int(floor(profile_i))
            profile_i_above = profile_i_below + 1
            if profile_i_below >= (len(fit_profile) - 1):
                fit_profile[centre + data_i] = profile[-1]
            else:
                fit_profile[centre + data_i] = profile[profile_i_below] +\
                 (profile[profile_i_above] - profile[profile_i_below]) *\
                 (float(profile_i - profile_i_below) / 
                 float(profile_i_above - profile_i_below))

# get profile of correct amplitude

        fit_profile *= profile_amp

        ndata = 0
        chisq = 0.0
        for i in range(len(fit_profile)):
            prof_offset = i - ((len(fit_profile)-1)/2)
            data_chan = prof_centre_chan + prof_offset
            if (data_chan >= 0) and (data_chan < len(data)):
                if not(flag[-1][data_chan]):
                    ndata += 1
                    try:
                       chisq += pow ((data[data_chan] - fit_profile[i])/
                                     stddev[data_chan],2)
                    except ZeroDivisionError:
                       print 'caught zero division error'
                       chisq += 1e6                 
        if ndata > 0:
            chisq /= ndata
        return chisq


    def _find_noise_edge(self, mad, flag):
        """Return the index in the mad array where the noise first
        dips below the median value.
                                         
        Keyword arguments:
        mad    -- The noise array to be examined.
        flag   -- Array whose elements are True where mad is invalid.
                                         
        The index of the first point where the noise dips below twice the median
        for the first half of the spectrum. Looking at half the spectrum
        handles the case where the spectrum is a composite of 2 subbands,
        with different noise levels; it's a fudge in that some spectra may
        be composed of more than 2 subbands.
        """

        noise_edge = None

        nchan = len(mad)
        median_mad = median(compress(logical_not(flag[:nchan/4]),
         mad[:nchan/4]))
        for i in range(nchan):
            if not flag[i]:
                if mad[i] < 2.0 * median_mad:
                    noise_edge = i
                    break

        return noise_edge


    def _find_small_diff(self, data, flag):
        """Return the index in the first quarter of the data array where the
        point to point difference first falls below twice the median value.
                                        
        Keyword arguments:
        data -- The data array to be examined.
        flag -- Array whose elements are True where list_data is bad.
                                         
        Returns:
        The index of the first point where the point to point difference 
        first falls below twice the median value.
        """

        result = None

        nchan = len(data)
        good_data = compress(logical_not(flag[:nchan/4]), data[:nchan/4])
        good_data_index = compress(logical_not(flag[:nchan/4]), range(nchan/4))
        diff = abs(good_data[1:] - good_data[:-1])
        median_diff = median(diff)

        for i in good_data_index[:-2]:
            if diff[i] < 2.0 * median_diff:
                result = i
                break

        return result


    def _find_turnover(self, data, flag):
        """Return the index in the data array where the point to point
        difference first changes sign compared to the first difference.
                                        
        Keyword arguments:
        data -- The data array to be examined.
        flag -- Array whose elements are True where list_data is bad.
                                         
        Returns:
        The index of the first point where the difference array changes sign.

        """

        turnover = None
        prev_data = None
        first_diff = None

        for i in range(len(data)):
            if not flag[i]:
                if prev_data == None:
                    prev_data = data[i]
                else:
                    diff = data[i] - prev_data
                    prev_data = data[i]
                    if first_diff == None:
                        first_diff = diff
                        if first_diff == 0:
                            turnover = i
                            break
                    else:                
                        if diff/first_diff < 0:
                            turnover = i
                            break
        return turnover


    def _flagData(self, x, data, data_mad, flag, data_description,
     stageDescription, flag_target_ids):
        """Utility method to apply the flag rules to the data.
        """

        new_flags = {}
        for rule in self._rules:
            new_flags[rule['rule']] = []

# loop through rules

        for rule in self._rules:
            if rule['rule'] == 'VLA edge template':
                if self._telescopeName != 'VLA':
                    continue

                nchannels = len(data)

# construct 'edge' profile according to telescope and number of channels -
# really we need a library of these. For VLA data the edge is just the edge 
# of the filter, so it's just a cos curve.

                if nchannels == 31:
                    edge_width = 5
                    right_edge_profile = zeros([edge_width], float)
                    left_edge_profile = zeros([edge_width], float)
                    for i in range(edge_width):
                        right_edge_profile[i] = (1.0 + 
                         cos(i*pi/(edge_width-1)))/2.0

                    for i in range(edge_width):
                        left_edge_profile[-(i + 1)] = right_edge_profile[i]
                else:
                    edge_width = 11
                    right_edge_profile = zeros([edge_width], float)
                    left_edge_profile = zeros([edge_width], float)
                    for i in range(edge_width):
                        right_edge_profile[i] = (1.0 + 
                         cos(i*pi/(edge_width-1)))/2.0

                    for i in range(edge_width):
                        left_edge_profile[-(i + 1)] = right_edge_profile[i]
    
                left_flag = 0
                right_flag = nchannels

# calculate chi-squared for edge_width channels from right end of spectrum

                valid_data = compress(flag[-1][nchannels/2:]==0, 
                 data[nchannels/2:])
                if len(valid_data):
                    edge_amp = median(valid_data)
                    if mean(data[-2:]) > 0.7 * edge_amp:

# band carries on right to edge, this can confuse the chi-squared method
# so handle it explicitly

                        right_edge_centre = nchannels + (edge_width-1) / 2
                    else:

# look for fit to the band edge template

                        chisq = zeros([3*edge_width], float) + 1.0e6
                        for edge_offset in range(
                         -2*edge_width, ((edge_width-1)/2)):
                            edge_centre = nchannels + edge_offset
                            chisq [edge_offset+2*edge_width] = \
                             self._chi_squared(data, flag, data_mad,
                             right_edge_profile, edge_centre, edge_amp)
                           
# find minimum in chi-squared
                           
                        right_edge_centre =  nchannels + argmin(chisq) - \
                             (2 * edge_width)

# flag channels within half the edge_width of the edge centre.

                        right_flag = right_edge_centre - (edge_width-1) / 2.0
                        if right_flag >= 0:
                            flag[-1][right_flag:] = 1

# same for left edge profile
                               
                valid_data = compress(flag[-1][:nchannels/2]==0,
                 data[:nchannels/2])
                if len(valid_data):
                    edge_amp = median(valid_data)

                    if mean(data[:1]) > 0.7 * edge_amp:
                        left_edge_centre = -(edge_width - 1) / 2.0
                    else:
                        chisq = zeros([3*edge_width], float) + 1.0e6
                        for edge_offset in range(
                         -((edge_width-1)/2),2*edge_width):
                            chisq [edge_offset + ((edge_width-1)/2)] =\
                             self._chi_squared(data, flag, data_mad,
                             left_edge_profile, edge_offset, edge_amp)
                        left_edge_centre = argmin(chisq) - \
                        (edge_width - 1) / 2.0

                        left_flag = left_edge_centre + (edge_width-1) / 2.0
                        if (left_flag+1) > 0:
                            flag[-1][:left_flag+1] = 1

# get indeces of channels flagged

                channels_flagged = arange(nchannels)
                channels_flagged = compress(logical_or(
                 channels_flagged < left_flag, channels_flagged > right_flag),
                 channels_flagged)

                if len(channels_flagged) > 0:
                    self._add_flag_description(new_flags, flag,
                     channels_flagged, data_description,
                     stageDescription, rule, flag_target_ids)

            elif rule['rule'] == 'PdB edge template':
                if self._telescopeName != 'IRAM_PDB':
                    continue

                valid_data = compress(flag[-1]==0, data)
                if len(valid_data):

# find left edge

                    left_amp = self._find_small_diff(data, flag[-1])
                    left_amp_stddev = self._find_noise_edge(data_mad, flag[-1])
                    left_edge = max(0, left_amp, left_amp_stddev)
#                    print 'left', left_amp, left_amp_stddev, left_edge

# and right edge

                    reverse_data = data[-1::-1]
                    reverse_flag = flag[-1][-1::-1]
                    reverse_mad = data_mad[-1::-1]
                    right_amp = self._find_small_diff(reverse_data,
                     reverse_flag)
                    right_amp_stddev = self._find_noise_edge(reverse_mad,
                     reverse_flag)
                    right_edge = max(0, right_amp, right_amp_stddev)
#                    print 'right', right_amp, right_amp_stddev, right_edge

# flag the Python data

                    flag[-1][:left_edge] = 1
                    flag[-1][-right_edge:] = 1

# now compose a description of the flagging required on the MS

                    nchannels = len(data)
                    channels = arange(nchannels)
                    channels_flagged = compress(logical_or(
                     channels < left_edge,
                     channels > (nchannels-1-right_edge)),
                     channels)

                    if len(channels_flagged) > 0:
                        self._add_flag_description(new_flags, flag,
                         channels_flagged, data_description,
                         stageDescription, rule, flag_target_ids)

            else:
                raise NameError, 'bad rule: %s' % rule['rule']

        return new_flags


    def operate(self, stageDescription, dataView):
        """Public method to apply the flag rules to the data 'view'.

        Keyword arguments:
        stageDescription -- Dictionary with a description of the calling
                            reduction stage.
        dataView         -- Object providing the data 'view'.
        """

#        print 'BandpassEdgeFlagger.operate called'
        self._htmlLogger.timing_start('bandpassEdgeFlagger.operate')

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

# get data

        collection = dataView.getData()
        self._telescopeName = collection['summary']['telescope_name']
        flags = {}
        for key, results in collection['data'].iteritems():
            description = pickle.loads(key)

# add field to those potentially being flagged

            newField = int(description['FIELD_ID'])
            if self._potentially_flagged_target_ids.count(newField) == 0:
                self._potentially_flagged_target_ids.append(newField)

            x = results[-1]['x']
            xtitle = results[-1]['xtitle']
            data = results[-1]['data']
            data_mad = results[-1]['mad_floor']
            flag = results[-1]['flag']

# get new flags, and accumulate these in a dictionary with entries for each
# 'rule'
 
            new_flags = self._flagData(x, data[0], data_mad[0], flag[0],
             description, stageDescription, flag_target_ids)
            for k,v in new_flags.iteritems():
                if not flags.has_key(k):
                    flags[k] = []
                flags[k] += new_flags[k]

# store the flags in the MS but don't actually apply them

        dataView.setFlags(stageDescription, self._rules, flags, apply=False)
        self._htmlLogger.timing_stop('bandpassEdgeFlagger.operate')


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        """

# write out the description

        self._htmlLogger.logHTML('''
         <p>Each spectrum was examined
         and the edge channels flagged acording to the rules listed below.
         <ul>''')

        for rule in self._rules:
            description = 'No description available for flagging rule: %s' % (
             rule['rule'])
            if rule['rule'] == 'VLA edge template':
                description = '''
                 Templates designed to resemble the shape of the data amplitude
                 at the band edges were fitted to each end of the bandpass
                 spectrum. The edge channels were those covered by the
                 template placed at the best fit position.'''

            elif rule['rule'] == 'PdB edge template':
                description = '''
                 The data have been preprocessed in such a way that the band
                 profile is not directly visible. Instead the band edges 
                 appear as regions of lower S/N. Two methods were used to 
                 estimate the edge of the noisy region at each end of the band
                 and the edge channels were set to cover the maximum 
                 estimate.'''

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
         <p>The data 'view' is a list of spectra, one for each spectral window.
         Each was examined
         and the edge channels flagged according to the rules listed below.
         <ul>''')

        for rule in self._rules:
            description = 'No description available for flagging rule: %s' % (
             rule['rule'])
            if rule['rule'] == 'VLA edge template':
                description = '''
                 Templates designed to resemble the shape of the data amplitude
                 at the band edges are placed at different positions in the data
                 and the chi-squared between data and template calculated at
                 each. The position of the band edges are those with the 
                 minimum values for chi-squared.
        
                 The VLA band profile is something like this:
        
                 <pre>      
            15                **************************************
                            *                                        *
            10             *                                          *
                          *                                            *
            5            *                                              *
                       *                                                  *
            0*******                                                          *****

                 </pre>

                 <p>The algorithm constructs a normalised template for each
                 end of the band. The template for the left hand end in this 
                 case would look like this:
                 <pre>

            1              **
                         *
                        *
            0.5        *
                      *
                    *
            0  **
                 </pre>

                 <p>Next the normalised template is multiplied by the median of 
                 the values across the bandpass (about 15 for the first diagram 
                 above) to give a shape that should resemble the left hand end 
                 of the actual data:
                 <pre>
            15             **
                         *
            10          *
                       *
            5         *
                    *
            0  **
                 </pre>

                 Next the algorithm slides the above template along a channel 
                 range at the left end of the data, calculating a chi-squared 
                 value at each position:
                 <pre>

        chisq = sum_over_channel ((data - template)/data_sigma)^2
                -------------------------------------------------
                                 nchannels
                 </pre>

                 where nchannels is the number of channels in the overlap 
                 between the template and the data at each template position.

                 The algorithm takes the minimum in the array of chisq as 
                 being the position of the left end of the bandpass. It then 
                 flags all data channels in the specified source types that
                 are covered by the template when it is at that position.'''

            elif rule['rule'] == 'PdB edge template':
                description = '''
                 The data have been preprocessed in such a way that the band
                 profile is not directly visible. Instead the band edges appear 
                 as regions of lower S/N. Two methods are used to 
                 estimate the edge of the noisy region at each end of the band:
                 <ul>
                  <li>The channel closest to the band end where the standard 
                      deviation on the amplitude first dips below twice the
                      median s.d. for the first quarter of the band.
                  <li>The channel closest to the band end where the difference
                      between adjacent channels first falls below the median of 
                      such differences over the first quarter of the band. 
                 </ul>
                 The band edge is set to the maximum of the values returned by
                 the 2 methods.'''

#                  <li>The channel closest to the band end where the amplitude 'curve'
#                      viewed across the channels has its first inflection.

            self._htmlLogger.logHTML('<li> %s' % description)

#        self._htmlLogger.logHTML('''</ul>
#        <p>The flagging was performed by Python class BandpassEdgeFlagger.''')
