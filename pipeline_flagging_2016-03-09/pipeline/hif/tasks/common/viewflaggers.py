from __future__ import absolute_import

import math
import os
import numpy as np

from . import arrayflaggerbase
from . import flaggableviewresults
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask


LOG = infrastructure.get_logger(__name__)


class MatrixFlaggerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, datatask=None,
        flagsettertask=None, rules=None, niter=None, extendfields=None,
        prepend=''):

        # set the properties to the values given as input arguments
        self._init_properties(vars())


class MatrixFlagger(basetask.StandardTaskTemplate):
    Inputs = MatrixFlaggerInputs

    flag_reason_index = {'max abs':1,
                         'min abs':2,
                         'nmedian':3,
                         'outlier':4,
                         'high outlier':5,
                         'low outlier':6,
                         'too many flags':7,
                         'bad quadrant':8,
                         'bad antenna':9}
    flag_reason_key = {1:'max abs',
                       2:'min abs',
                       3:'nmedian',
                       4:'outlier',
                       5:'high outlier',
                       6:'low outlier',
                       7:'too many flags',
                       8:'bad quadrant',
                       9:'bad antenna'}

    # override the inherited __init__ method so that references to the
    # task objects can be kept outside self.inputs. Later on self.inputs
    # will be replaced by a copy which breaks the connection between
    # its references to the tasks and the originals.
    def __init__(self, inputs):
        self.inputs = inputs
        self.datatask = inputs.datatask
        self.flagsettertask = inputs.flagsettertask

    def prepare(self):
        inputs = self.inputs

        datatask = self.datatask
        rules = inputs.rules
        flagsettertask = self.flagsettertask
        niter = inputs.niter

        if inputs.extendfields:
            LOG.info('%s flagcmds will be extended by removing selection in following fields: %s'
              % (inputs.prepend, inputs.extendfields))

        iter = 1
        flags = []
        flag_reason_plane = {}

        while iter <= niter:

            # Get latest data
            result = self._executor.execute(datatask)
            if not result.descriptions():
                return result

            descriptionlist = result.descriptions()
            descriptionlist.sort()
            newflags = []
            for description in descriptionlist:
                image = result.last(description)
                # get flags for this view according to the rules
                theseflags, this_flag_reason_plane = self.generate_flags(image, rules)

                # update flagging record
                newflags += theseflags
                if flag_reason_plane.has_key(description):
                    # perhaps should add check here that flags are not being set
                    # twice, which would imply that something inconsistent is 
                    # happening
                    flag_reason_plane[description][this_flag_reason_plane > 0] = \
                      this_flag_reason_plane[this_flag_reason_plane > 0]
                else:
                    flag_reason_plane[description] = this_flag_reason_plane

            # set any flags raised
            if newflags:
                LOG.warning('%s%s iteration %s raised %s flagging commands' % \
                  (inputs.prepend, os.path.basename(inputs.vis), iter, len(newflags)))
            else:
                LOG.info('%s%s iteration %s raised %s flagging commands' % \
                  (inputs.prepend, os.path.basename(inputs.vis), iter, len(newflags)))
            flagsettertask.flags_to_set(newflags)
            ignore = self._executor.execute(flagsettertask)

            # accumulate all flags set
            flags += newflags

            if len(newflags) == 0:
                break

            iter += 1

        if len(newflags) > 0:
            # exited loop when flags still being found, recalculate
            # result with latest flags incorporated.
            result = self._executor.execute(datatask)

        # remove duplicate flags, add a record of flags set to the 
        # results object
        flags = list(set(flags))
        result.addflags(flags)
        result.add_flag_reason_plane(flag_reason_plane, self.flag_reason_key)
        result.table = flagsettertask.inputs.table

        return result

    def analyse(self, result):
        return result

    @staticmethod
    def make_flag_rules (
      flag_hilo=False, fhl_limit=5.0, fhl_minsample=5, 
      flag_hi=False, fhi_limit=5.0, fhi_minsample=5,
      flag_lo=False, flo_limit=5.0, flo_minsample=5,
      flag_tmf1=False, tmf1_axis='Time', tmf1_limit=1.0, tmf1_excess_limit=10000000,
      flag_tmf2=False, tmf2_axis='Time', tmf2_limit=1.0, tmf2_excess_limit=10000000,
      flag_nmedian=False, fnm_lo_limit=0.7, fnm_hi_limit=1.3,
      flag_maxabs=False, fmax_limit=0.1,
      flag_minabs=False, fmin_limit=0.0,
      flag_bad_quadrant=False, fbq_hilo_limit=7.0,
      fbq_antenna_frac_limit=0.5, fbq_baseline_frac_limit=0.5,
      flag_bad_antenna=False, fba_lo_limit=7.0,
      fba_frac_limit=0.05, fba_number_limit=3, fba_minsample=5):

        """
        Generate a list of flagging rules from a set of flagging parameters.
        Added detailed docs here.
        """

        # Construct rules from flag properties. If niter is set to curtail
        # the flagging loop then the order that the rules are applied 
        # can be important. For example, 'too many flags' should run after
        # the other rules, 'bad quadrant' or 'bad antenna' should be run 
        # before the others.
        rules = []
        if flag_bad_quadrant:
            rules.append({'name':'bad quadrant', 'hilo_limit':fbq_hilo_limit,
              'frac_limit':fbq_antenna_frac_limit,
              'baseline_frac_limit':fbq_baseline_frac_limit})
        if flag_bad_antenna:
            rules.append({'name':'bad antenna', 'lo_limit':fba_lo_limit,
              'frac_limit':fba_frac_limit, 'number_limit':fba_number_limit,
              'minsample':fba_minsample})
        if flag_maxabs:
            rules.append({'name':'max abs', 'limit':fmax_limit})
        if flag_minabs:
            rules.append({'name':'min abs', 'limit':fmin_limit})
        if flag_nmedian:
            rules.append({'name':'nmedian', 'lo_limit':fnm_lo_limit,
              'hi_limit':fnm_hi_limit})
        if flag_hilo:
            rules.append({'name':'outlier', 'limit':fhl_limit,
              'minsample':fhl_minsample})
        if flag_hi:
            rules.append({'name':'high outlier', 'limit':fhi_limit,
              'minsample':fhi_minsample}) 
        if flag_lo:
            rules.append({'name':'low outlier', 'limit':flo_limit,
              'minsample':flo_minsample})
        if flag_tmf1:
            rules.append({'name':'too many flags',
              'axis':str.upper(tmf1_axis),
              'limit':tmf1_limit,
              'excess limit':tmf1_excess_limit})
        if flag_tmf2:
            rules.append({'name':'too many flags',
              'axis':str.upper(tmf2_axis),
              'limit':tmf2_limit,
              'excess limit':tmf2_excess_limit})

        return rules

    def generate_flags(self, matrix, rules):
        """
        Calculate the statistics of a  matrix and flag the data according
        to a list of specified rules.

        Keyword arguments:
        matrix - ImageResult object containing data to be flagged.
        rules - Rules to be applied.
        """

        # Get the attributes - ensure all arrays are numpy arrays
        # as some subsequent processing depends on numpy array indexing
        data = np.array(matrix.data)
        flag = np.array(matrix.flag)
        nodata = np.array(matrix.nodata)
        xtitle = matrix.axes[0].name
        xdata = np.array(matrix.axes[0].data)
        ytitle = matrix.axes[1].name
        ydata = np.array(matrix.axes[1].data) 
        spw = matrix.spw
        table = matrix.filename
        pol = matrix.pol
        antenna = matrix.ant
        if antenna is not None:
            # deal with antenna id not name
            antenna = antenna[0]

        # Initialize flags
        newflags = []
        flag_reason = np.zeros(np.shape(flag), np.int)

        # Index arrays
        i,j = np.indices(np.shape(data))

        rdata = np.ravel(data)
        rflag = np.ravel(flag)
        valid_data = rdata[np.logical_not(rflag)]

        # If there is valid data (non-flagged), then proceed with flagging
        if len(valid_data) > 0:

            # calculate statistics for valid data
            data_median, data_mad = arrayflaggerbase.median_and_mad(valid_data)
            
            # flag data according to each rule in turn
            for rule in rules:
                rulename = rule['name']

                if rulename == 'outlier':
  
                    # Sample too small
                    minsample = rule['minsample']
                    if len(valid_data) < minsample:
                        continue

                    # Check limits.
                    mad_max = rule['limit']
                    i2flag = i[np.logical_and(np.abs(data - data_median) >\
                      mad_max * data_mad, np.logical_not(flag))]
                    j2flag = j[np.logical_and(np.abs(data - data_median) >\
                      mad_max * data_mad, np.logical_not(flag))]

                    # No flagged data.
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag command to flag data underlying the
                    # view.
                    flagcoords = zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='outlier',
                          filename=table, rulename=rulename,
                          spw=spw, antenna=antenna,
                          axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol))
  
                    # Flag the view.
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'low outlier':

                    # Sample too small
                    minsample = rule['minsample']
                    if len(valid_data) < minsample:
                        continue

                    # Check limits.
                    mad_max = rule['limit']
                    i2flag = i[np.logical_and(data_median - data > \
                      mad_max * data_mad, np.logical_not(flag))]
                    j2flag = j[np.logical_and(data_median - data > \
                      mad_max * data_mad, np.logical_not(flag))]

                    # No flagged data.
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag data underlying the
                    # view.
                    flagcoords = zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='low_outlier',
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol))

                    # Flag the view.
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'high outlier':

                    # Sample too small
                    minsample = rule['minsample']
                    if len(valid_data) < minsample:
                        continue

                    # Check limits.
                    mad_max = rule['limit']
                    i2flag = i[np.logical_and(data - data_median > \
                      mad_max * data_mad, np.logical_not(flag))]
                    j2flag = j[np.logical_and(data - data_median > \
                      mad_max * data_mad, np.logical_not(flag))]

                    # No flags
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag data underlying the
                    # view.
                    flagcoords = zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='high_outlier',
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol))

                    # Flag the view.
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'min abs':

                    # Check limits.
                    limit = rule['limit']
                    i2flag = i[np.logical_and(np.abs(data) < limit,
                      np.logical_not(flag))]
                    j2flag = j[np.logical_and(np.abs(data) < limit,
                      np.logical_not(flag))]

                    # No flags
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag data underlying the
                    # view.
                    flagcoords = zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='min_abs',
                          filename=table, rulename=rulename, spw=spw,
                          axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol))

                    # Flag the view
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'max abs':

                    # Check limits.
                    limit = rule['limit']
                    i2flag = i[np.logical_and(np.abs(data) > limit,
                      np.logical_not(flag))]
                    j2flag = j[np.logical_and(np.abs(data) > limit,
                      np.logical_not(flag))]

                    # No flags
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag data underlying the
                    # view.
                    flagcoords=zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='max_abs',
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol))

                    # Flag the view
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'too many flags':
 
                    maxfraction = rule['limit']
                    maxexcessflags = rule['excess limit']
                    axis = rule['axis']
                    axis = axis.upper().strip()

                    if axis == xtitle.upper().strip():

                        # Compute median number flagged
                        num_flagged = np.zeros([np.shape(data)[1]], np.int)
                        for iy in np.arange(len(ydata)):
                            num_flagged[iy] = len(data[:,iy][flag[:,iy]])
                        median_num_flagged = np.median(num_flagged)

                        # look along x axis
                        for iy in np.arange(len(ydata)):
                            if all(flag[:,iy]):
                                continue

                            # Compute fraction flagged
                            len_data = len(xdata)
                            len_no_data = len(data[:,iy][nodata[:,iy]])
                            len_flagged = len(data[:,iy][flag[:,iy]])
                            fractionflagged = (
                              float(len_flagged - len_no_data) /
                              float(len_data - len_no_data))
                            if fractionflagged > maxfraction:
                                i2flag = i[:,iy][np.logical_not(flag[:,iy])]
                                j2flag = j[:,iy][np.logical_not(flag[:,iy])]
                            else:
                                i2flag = np.zeros([0], np.int)
                                j2flag = np.zeros([0], np.int)

                            # likewise for maxexcessflags
                            if len_flagged > median_num_flagged + maxexcessflags:
                                i2flag = np.concatenate((i2flag, i[:,iy][np.logical_not(flag[:,iy])]))
                                j2flag = np.concatenate((j2flag, j[:,iy][np.logical_not(flag[:,iy])]))

                            # Add new flag commands to flag data underlying 
                            # the view.
                            flagcoords = zip(xdata[i2flag], ydata[j2flag])
                            for flagcoord in flagcoords:
                                newflags.append(arrayflaggerbase.FlagCmd(
                                  reason='too_many_flags',
                                  filename=table, rulename=rulename,
                                  spw=spw, antenna=antenna, 
                                  axisnames=[xtitle,ytitle],
                                  flagcoords=flagcoord, pol=pol))

                            # Flag the view
                            flag[i2flag, j2flag] = True
                            flag_reason[i2flag, j2flag] =\
                              self.flag_reason_index[rulename]

                    elif axis == ytitle.upper().strip():

                        # Compute median number flagged
                        num_flagged = np.zeros([np.shape(data)[0]], np.int)
                        for ix in np.arange(len(xdata)):
                            num_flagged[ix] = len(data[ix,:][flag[ix,:]])
                        median_num_flagged = np.median(num_flagged)

                        # look along y axis
                        for ix in np.arange(len(xdata)):
                            if all(flag[ix,:]):
                                continue

                            len_data = len(ydata)
                            len_no_data = len(data[ix,:][nodata[ix,:]])
                            len_flagged = len(data[ix,:][flag[ix,:]])
                            fractionflagged = (
                              float(len_flagged - len_no_data) / 
                              float(len_data - len_no_data))
                            if fractionflagged > maxfraction:
                                i2flag = i[ix,:][np.logical_not(flag[ix,:])]
                                j2flag = j[ix,:][np.logical_not(flag[ix,:])]
                            else:
                                i2flag = np.zeros([0], np.int)
                                j2flag = np.zeros([0], np.int)

                            len_flagged = len(data[ix,:][flag[ix,:]])
                            if len_flagged > median_num_flagged + maxexcessflags:
                                i2flag = np.concatenate((i2flag, i[ix,:][np.logical_not(flag[ix,:])]))
                                j2flag = np.concatenate((j2flag, j[ix,:][np.logical_not(flag[ix,:])]))

                            # Add new flag commands to flag data underlying 
                            # the view.
                            flagcoords = zip(xdata[i2flag], ydata[j2flag])
                            for flagcoord in flagcoords:
                                newflags.append(arrayflaggerbase.FlagCmd(
                                  reason='too_many_flags',
                                  filename=table, rulename=rulename, spw=spw,
                                  axisnames=[xtitle, ytitle],
                                  flagcoords=flagcoord, pol=pol))

                            # Flag the view.
                            flag[i2flag, j2flag] = True
                            flag_reason[i2flag, j2flag] =\
                              self.flag_reason_index[rulename]

                elif rulename == 'nmedian':

                    # Check for valid median
                    if data_median is None:
                        continue

                    # Check limits.
                    lo_limit = rule['lo_limit']
                    hi_limit = rule['hi_limit']
                    i2flag = i[np.logical_and(
                      np.logical_or(data < lo_limit * data_median,
                      data > hi_limit * data_median),
                      np.logical_not(flag))]
                    j2flag = j[np.logical_and(
                      np.logical_or(data < lo_limit * data_median,
                      data > hi_limit * data_median),
                      np.logical_not(flag))]

                    # No flags
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag the data underlying
                    # the view.
                    flagcoords=zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='nmedian',
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol,
                          extendfields=self.inputs.extendfields))

                    # Flag the view.
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'bad antenna':
                    # this test should be run before the others 
                    # as it depends on no other
                    # flags having been set by other rules before it
                    # (because the number of unflagged points
                    # on entry are part of the test)

                    # Check limits.
                    mad_max = rule['lo_limit']
                    frac_limit = rule['frac_limit']
                    number_limit = rule['number_limit']
                    minsample = rule['minsample']

                    if 'ANTENNA' in xtitle.upper():

                        i,jant = np.indices(np.shape(data))

                        for ant in range(np.shape(flag)[0]):
                            ant_data = data[ant,:]
                            ant_flag = flag[ant,:]
                            valid_ant_data = ant_data[np.logical_not(ant_flag)]

                            # Sample too small?
                            if len(valid_ant_data) < minsample:
                                continue

                            ant_data_median, ant_data_mad = \
                              arrayflaggerbase.median_and_mad(valid_ant_data)

                            # find low outlier flags first
                            j_ant = np.arange(np.shape(ant_flag)[0])
                            j2flag_lo = j_ant[np.logical_and(data_median - ant_data > \
                              mad_max * data_mad, np.logical_not(ant_flag))]
 
                            # No flagged data?
                            if len(j2flag_lo) <= 0:
                                continue

                            # is the antenna bad?
                            nflags = len(j2flag_lo)
                            flagsfrac = float(nflags) / float(np.shape(ant_flag)[0])

                            if nflags >= number_limit or \
                               flagsfrac > frac_limit:

                                # Only once we get here do we actually flag
                                # the view and the data

                                # first flag the view, specifying the reason
                                # for each flagged point
                                i2flag = np.zeros(np.shape(j2flag_lo), np.int)
                                i2flag += ant

                                ant_flag[j2flag_lo] = True
                                flag_reason[i2flag, j2flag_lo] = \
                                  self.flag_reason_index['low outlier']

                                j2flag_bad = j_ant[np.logical_not(ant_flag)]
                                i2flag_bad = np.zeros(np.shape(j2flag_bad), np.int)
                                i2flag_bad += ant

                                ant_flag[j2flag_bad] = True
                                flag_reason[i2flag_bad, j2flag_bad] = \
                                  self.flag_reason_index['bad antenna']

                                # copy flags back into view
                                data[ant,:] = ant_data
                                flag[ant,:] = ant_flag

                                # Just flag the entire antenna in the data
                                newflags.append(arrayflaggerbase.FlagCmd(
                                  reason='bad antenna', filename=table,
                                  rulename=rulename, spw=spw, pol=pol,
                                  antenna=ant))

                elif rulename == 'bad quadrant':
                    # this test should be run before the others 
                    # as it depends on no other
                    # flags having been set by other rules before it
                    # (because the number of unflagged points
                    # on entry are part of the test)

                    # a quadrant is one quarter of the extent of the x-axis

                    # Check limits.
                    hilo_limit = rule['hilo_limit']
                    frac_limit = rule['frac_limit']
                    baseline_frac_limit = rule['baseline_frac_limit']

                    # find outlier flags first                 
                    i2flag = i[np.logical_and(np.abs(data - data_median) >\
                      hilo_limit * data_mad, np.logical_not(flag))]
                    j2flag = j[np.logical_and(np.abs(data - data_median) >\
                      hilo_limit * data_mad, np.logical_not(flag))]

                    # No flagged data.
                    if len(i2flag) <= 0:
                        continue

                    # have to be careful here not to corrupt the data view
                    # as we go through it testing for bad quadrant/antenna.
                    # Make a copy of the view flags and go though these
                    # one antenna at a time testing for bad quadrant.
                    # If bad, copy 'outlier' and 'bad quadrant' flags to
                    # original view. 'unflagged_flag_copy' is a copy
                    # of the flags made before the 'outlier' flags are
                    # applied - it is used to estimate how many 
                    # _additional_ points have been flagged.
                    unflagged_flag_copy = np.copy(flag)
                    flag_copy = np.copy(flag)
                    flag_reason_copy = np.copy(flag_reason)
                    flag_copy[i2flag, j2flag] = True
                    flag_reason_copy[i2flag, j2flag] = \
                      self.flag_reason_index['outlier']

                    # look for bad antenna/quadrants in view copy
                    data_shape = np.shape(data)
                    nchan = data_shape[0]
                    nbaseline = data_shape[1]
                    nant = int(math.sqrt(nbaseline))

                    quadrant = [[0,nchan/4-1],
                      [nchan/4,nchan/2-1],
                      [nchan/2,nchan*3/4-1],
                      [nchan*3/4,nchan-1]]
                        
                    quadrant_len = nchan/4

                    for ant in range(nant):
                        # flag based on outliers in flag_copy, will set new flags
                        # in a further copy so that outlier flags are not corrupted
                        working_copy_flag = np.copy(flag_copy)
                        working_copy_flag_reason = np.copy(flag_reason_copy)

                        # baselines involving this antenna
                        baselines = [baseline for baseline in range(nbaseline) if 
                          (baseline >= ant*nant and baseline < (ant+1)*nant) or
                          (baseline%nant == ant)]
                        baselines = np.array(baselines)

                        for iquad in range(4):
                            quad_slice = slice(quadrant[iquad][0], quadrant[iquad][1])
                            ninvalid = np.count_nonzero(
                              working_copy_flag[quad_slice,baselines])
                            ninvalid_on_entry = np.count_nonzero(
                              unflagged_flag_copy[quad_slice,baselines])
                            nvalid_on_entry = np.count_nonzero(np.logical_not(
                              unflagged_flag_copy[quad_slice,baselines]))
                            if nvalid_on_entry:
                                frac = float(ninvalid - ninvalid_on_entry) /\
                                  float(nvalid_on_entry)
                            else:
                                frac = 0.0

                            if frac > frac_limit:
                                # Add new flag commands to flag the data 
                                # underlying the view. These will flag the entire 
                                # quadrant/antenna. If the quadrant is not
                                # bad then any 'outlier' points found
                                # earlier will not be flagged.
                                flagcoords = []
                                for chan in range(quadrant[iquad][0], 
                                  quadrant[iquad][1]):
                                    flagcoords.append((chan,ant))
                                for flagcoord in flagcoords:
                                    newflags.append(arrayflaggerbase.FlagCmd(
                                      reason='bad quadrant',
                                      filename=table, rulename=rulename,
                                      spw=spw, axisnames=[xtitle, ytitle],
                                      flagcoords=flagcoord, pol=pol,
                                      extendfields=self.inputs.extendfields))

                                # update working copy view with 'bad quadrant' flags 
                                i2flag = i[quad_slice,baselines]\
                                  [np.logical_not(working_copy_flag[quad_slice,baselines])]
                                j2flag = j[quad_slice,baselines]\
                                  [np.logical_not(working_copy_flag[quad_slice,baselines])]

                                if len(i2flag) > 0:
                                    working_copy_flag[i2flag, j2flag] = True
                                    working_copy_flag_reason[i2flag, j2flag] =\
                                      self.flag_reason_index['bad quadrant']
 
                                # copy flag state for this antenna 
                                # back to original
                                flag[quad_slice,baselines] = \
                                  working_copy_flag[quad_slice,baselines]
                                flag_reason[quad_slice,baselines] = \
                                  working_copy_flag_reason[quad_slice,baselines]

                                # whole antenna/quadrant flagged, no need to check 
                                # individual baselines
                                continue

                            # look for bad quadrant/baseline
                            for baseline in baselines:
                                ninvalid = np.count_nonzero(
                                  working_copy_flag[quad_slice,baseline])
                                ninvalid_on_entry = np.count_nonzero(
                                  unflagged_flag_copy[quad_slice,baseline])
                                nvalid_on_entry = np.count_nonzero(
                                  np.logical_not(unflagged_flag_copy[
                                  quad_slice,baseline]))
                                if nvalid_on_entry:
                                    frac = float(ninvalid - ninvalid_on_entry) /\
                                      float(nvalid_on_entry)
                                else:
                                    frac = 0.0

                                if frac > baseline_frac_limit:

                                    # Add new flag commands to flag the data 
                                    # underlying the view. These will flag the entire 
                                    # quadrant/baseline. If the quadrant is not
                                    # bad then any 'outlier' points found
                                    # earlier will not be flagged.
                                    flagcoords = []
                                    for chan in range(quadrant[iquad][0], 
                                      quadrant[iquad][1]):
                                        flagcoords.append((chan,ydata[baseline]))
                                    for flagcoord in flagcoords:
                                        newflags.append(arrayflaggerbase.FlagCmd(
                                          reason='bad quadrant',
                                          filename=table, rulename=rulename,
                                          spw=spw, axisnames=[xtitle, ytitle],
                                          flagcoords=flagcoord, pol=pol,
                                          extendfields=self.inputs.extendfields))

                                    # update working copy view with 'bad quadrant' flags 
                                    i2flag = i[quad_slice,baseline]\
                                      [np.logical_not(working_copy_flag[quad_slice,baseline])]
                                    j2flag = j[quad_slice,baseline]\
                                      [np.logical_not(working_copy_flag[quad_slice,baseline])]

                                    if len(i2flag) > 0:
                                        working_copy_flag[i2flag, j2flag] = True
                                        working_copy_flag_reason[i2flag, j2flag] =\
                                          self.flag_reason_index['bad quadrant']

                                    # copy flag state for this antenna 
                                    # back to original
                                    flag[quad_slice,baseline] = \
                                      working_copy_flag[quad_slice,baseline]
                                    flag_reason[quad_slice,baseline] = \
                                      working_copy_flag_reason[quad_slice,baseline]

                else:           
                    raise NameError, 'bad rule: %s' % rule

        # consolidate flagcmds that specify individual channels into fewer
        # flagcmds that specify ranges
        newflags = arrayflaggerbase.consolidate_flagcmd_channels(newflags)

        return newflags, flag_reason


class NewMatrixFlaggerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, datatask=None,
        viewtask=None, flagsettertask=None, rules=None, niter=None, 
        extendfields=None, extendbaseband=None, iter_datatask=None, 
        use_antenna_names=None, prepend=''):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def use_antenna_names(self):
        return self._use_antenna_names

    @use_antenna_names.setter
    def use_antenna_names(self, value):
        if value is None:
            value = True
        self._use_antenna_names = value


class NewMatrixFlaggerResults(basetask.Results,
  flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new NewMatrixFlaggerResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'NewMatrixFlaggerResults'
        return s


class NewMatrixFlagger(basetask.StandardTaskTemplate):
    Inputs = NewMatrixFlaggerInputs

    flag_reason_index = {'max abs':1,
                         'min abs':2,
                         'nmedian':3,
                         'outlier':4,
                         'high outlier':5,
                         'low outlier':6,
                         'too many flags':7,
                         'bad quadrant':8,
                         'bad antenna':9,
                         'too many entirely flagged':10}
    flag_reason_key = {value: key for (key, value) in flag_reason_index.items()}

    # override the inherited __init__ method so that references to the
    # task objects can be kept outside self.inputs. Later on self.inputs
    # will be replaced by a copy which breaks the connection between
    # its references to the tasks and the originals.
    def __init__(self, inputs):
        self.inputs = inputs
        self.result = NewMatrixFlaggerResults()

    def prepare(self):

        inputs = self.inputs

        # Expand flag commands to larger scope, if requested, by removing selection in 
        # specified fields
        if inputs.extendfields:
            LOG.info('%s flagcmds will be extended by removing selection in following fields: %s'
              % (inputs.prepend, inputs.extendfields))

        # Expand flag commands to include all spws in a baseband, if requested
        if inputs.extendfields:
            LOG.info('%s flagcmds will be extended to include all spws within baseband.'
              % (inputs.prepend))

        # Initialize flags, flag_reason, and iteration counter
        flags = []
        flag_reason_plane = {}
        newflags = []
        counter = 1
        include_before = True

        # Start iterative flagging
        while counter <= inputs.niter:

            # Run the data task if needed
            if counter == 1:
                # Always run data task on first iteration
                data = self._executor.execute(inputs.datatask)
            elif inputs.iter_datatask is True:
                # If requested to re-run datatask on iteration, then
                # run the flag-setting task which modifies the data
                # and then re-run the data task
                
                # If no "before summary" was done, include this in the flag setting task
                if include_before:
                    stats_before, _ = self.set_flags(newflags, summarize_before=True)
                    include_before = False
                else:
                    _, _ = self.set_flags(newflags)
                    
                data = self._executor.execute(inputs.datatask)
            else:
                # TODO: implement flagging mask for the case where iter_datatask = False
                raise(NotImplementedError)

            # Create flagging view                
            viewresult = inputs.viewtask(data)

            # If a view could be created, continue with flagging
            if viewresult.descriptions():
            
                # Import the views from viewtask into the final result
                self.result.importfrom(viewresult)
    
                # Flag the view
                newflags, newflags_reason = self.flag_view(viewresult, inputs.rules)
            
                # Report how many flags were found in this iteration and
                # stop iteration if no new flags were found
                if len(newflags) == 0:
                    # If no new flags are found, report as a log message
                    LOG.info('%s%s iteration %s raised %s flagging commands' % \
                             (inputs.prepend, os.path.basename(inputs.vis), counter, len(newflags)))
                    break
                else:
                    # Report newly found flags as a warning message
                    LOG.warning('%s%s iteration %s raised %s flagging commands' % \
                                (self.inputs.prepend, os.path.basename(inputs.vis), counter, len(newflags)))
    
                # Accumulate new flags and flag reasons
                flags += newflags
                for description in newflags_reason.keys():
                    if flag_reason_plane.has_key(description):
                        flag_reason_plane[description][newflags_reason[description] > 0] = \
                            newflags_reason[newflags_reason[description] > 0]
                    else:
                        flag_reason_plane[description] = newflags_reason[description]
                
                counter += 1
            else:
                # If no view could be created, exit the iteration
                LOG.warning('No flagging view was created!')
                break

        # Create final set of flags by removing duplicates from our accumulated flags
        flags = list(set(flags))
        
        # If flags were found...
        if len(flags) > 0:
            
            # If newflags were found on last iteration loop, we need to still set 
            # these.
            if len(newflags) > 0:
                
                # If datatask needs to be iterated, and max number of iterations is
                # not 1, then the "before" summary has already been done, so only 
                # need to set flags and do "after" summary
                if inputs.iter_datatask is True and inputs.niter != 1:
                    
                    # Set flags, and include "after" summary
                    _ , stats_after = self.set_flags(newflags,
                                                     summarize_after=True)   

                    # With new flags set, re-run the data task
                    data = self._executor.execute(inputs.datatask)
                                 
                # Otherwise, set flags, and include both "before" and "after" summary
                else:
                    stats_before, stats_after = self.set_flags(newflags, 
                                                               summarize_before=True, 
                                                               summarize_after=True)
                
                # Create final post-flagging view                
                viewresult = inputs.viewtask(data)

                # Import the post-flagging view into the final result
                self.result.importfrom(viewresult)
                
            # If no newflags were found on last iteration loop
            else:
                
                # If datatask needs to be iterated, then the "before" summary has
                # already been done, and the flags have already been set, so only
                # need to do an "after" summary
                if inputs.iter_datatask is True:
                    summary_job = casa_tasks.flagdata(vis=inputs.flagsettertask.inputs.table, mode='summary')
                    stats_after = self._executor.execute(summary_job)
                # Otherwise, set flags, and include both "before" and "after" summary
                else:
                    stats_before, stats_after = self.set_flags(newflags, 
                                                               summarize_before=True, 
                                                               summarize_after=True)
            
            # Store the final set of flags in the final result
            self.result.addflags(flags)
            
            # Store the flag reasons in the last (i.e. post-flagging) view in the final result
            self.result.add_flag_reason_plane(flag_reason_plane, self.flag_reason_key)
        
        # if no flags were found at all
        else:
            # Run a single flagging summary and use the result as both the "before" 
            # and "after" summary.
            summary_job = casa_tasks.flagdata(vis=inputs.flagsettertask.inputs.table, mode='summary')
            stats_before = self._executor.execute(summary_job)
            stats_after = stats_before        
        
        # Store in the final result the name of the measurement set or caltable to 
        # which any potentially found flags would need to be applied to
        self.result.table = self.inputs.flagsettertask.inputs.table
        
        # Store the flagging summaries in the final result
        self.result.summaries = [stats_before, stats_after]

        return self.result

    def analyse(self, result):
        return result
        
    def flag_view(self, view, rules):
        newflags = []
        newflags_reason = {}
        descriptionlist = view.descriptions()
        descriptionlist.sort() 
        for description in descriptionlist:
            image = view.last(description)
            # get flags for this view according to the rules
            theseflags, this_flag_reason_plane = self.generate_flags(image, rules)

            # update flagging record
            newflags += theseflags
            newflags_reason[description] = this_flag_reason_plane

        return newflags, newflags_reason

    def set_flags(self, flags, summarize_before=False, summarize_after=False):
        # Initialize flagging summaries
        stats_before = []
        stats_after = []
        allflagcmds = []

        # Add the "before" summary to the flagging commands
        if summarize_before:
            allflagcmds = ["mode='summary' name='before'"]
        
        # Add the flagging commands
        allflagcmds.extend(flags)
        
        # Add the "before" summary to the flagging commands
        if summarize_after:
            allflagcmds.append("mode='summary' name='after'")
        
        # Update flag setting task with all flagging commands
        self.inputs.flagsettertask.flags_to_set(allflagcmds)
        
        # Run flag setting task
        flagsetterresult = self._executor.execute(self.inputs.flagsettertask)

        # Extract "before" summary if necessary
        if summarize_before:
            stats_before = {}
            for report in flagsetterresult.results[0].keys():
                if flagsetterresult.results[0][report]['name'] == 'before':
                    stats_before = flagsetterresult.results[0][report]
        if summarize_after:
            stats_after = {}
            for report in flagsetterresult.results[0].keys():
                if flagsetterresult.results[0][report]['name'] == 'after':
                    stats_after = flagsetterresult.results[0][report]
        
        return stats_before, stats_after
        

    @staticmethod
    def make_flag_rules (
      flag_hilo=False, fhl_limit=5.0, fhl_minsample=5, 
      flag_hi=False, fhi_limit=5.0, fhi_minsample=5,
      flag_lo=False, flo_limit=5.0, flo_minsample=5,
      flag_tmf1=False, tmf1_axis='Time', tmf1_limit=1.0, tmf1_excess_limit=10000000,
      flag_tmf2=False, tmf2_axis='Time', tmf2_limit=1.0, tmf2_excess_limit=10000000,
      flag_tmef1=False, tmef1_axis='Antenna1', tmef1_limit=1.0,
      flag_nmedian=False, fnm_lo_limit=0.7, fnm_hi_limit=1.3,
      flag_maxabs=False, fmax_limit=0.1,
      flag_minabs=False, fmin_limit=0.0,
      flag_bad_quadrant=False, fbq_hilo_limit=7.0,
      fbq_antenna_frac_limit=0.5, fbq_baseline_frac_limit=0.5,
      flag_bad_antenna=False, fba_lo_limit=7.0,
      fba_frac_limit=0.05, fba_number_limit=3, fba_minsample=5):

        """
        Generate a list of flagging rules from a set of flagging parameters.
        Added detailed docs here.
        """

        # Construct rules from flag properties. If niter is set to curtail
        # the flagging loop then the order that the rules are applied 
        # can be important. For example, 'too many flags' should run after
        # the other rules, 'bad quadrant' or 'bad antenna' should be run 
        # before the others.
        rules = []
        if flag_bad_quadrant:
            rules.append({'name':'bad quadrant', 'hilo_limit':fbq_hilo_limit,
              'frac_limit':fbq_antenna_frac_limit,
              'baseline_frac_limit':fbq_baseline_frac_limit})
        if flag_bad_antenna:
            rules.append({'name':'bad antenna', 'lo_limit':fba_lo_limit,
              'frac_limit':fba_frac_limit, 'number_limit':fba_number_limit,
              'minsample':fba_minsample})
        if flag_maxabs:
            rules.append({'name':'max abs', 'limit':fmax_limit})
        if flag_minabs:
            rules.append({'name':'min abs', 'limit':fmin_limit})
        if flag_nmedian:
            rules.append({'name':'nmedian', 'lo_limit':fnm_lo_limit,
              'hi_limit':fnm_hi_limit})
        if flag_hilo:
            rules.append({'name':'outlier', 'limit':fhl_limit,
              'minsample':fhl_minsample})
        if flag_hi:
            rules.append({'name':'high outlier', 'limit':fhi_limit,
              'minsample':fhi_minsample}) 
        if flag_lo:
            rules.append({'name':'low outlier', 'limit':flo_limit,
              'minsample':flo_minsample})
        if flag_tmf1:
            rules.append({'name':'too many flags',
              'axis':str.upper(tmf1_axis),
              'limit':tmf1_limit,
              'excess limit':tmf1_excess_limit})
        if flag_tmf2:
            rules.append({'name':'too many flags',
              'axis':str.upper(tmf2_axis),
              'limit':tmf2_limit,
              'excess limit':tmf2_excess_limit})
        if flag_tmef1:
            rules.append({'name':'too many entirely flagged',
              'axis':str.upper(tmef1_axis),
              'limit':tmef1_limit})

        return rules

    def generate_flags(self, matrix, rules):
        """
        Calculate the statistics of a  matrix and flag the data according
        to a list of specified rules.

        Keyword arguments:
        matrix - ImageResult object containing data to be flagged.
        rules - Rules to be applied.
        """

        # Get the attributes - ensure all arrays are numpy arrays
        # as some subsequent processing depends on numpy array indexing
        data = np.array(matrix.data)
        flag = np.array(matrix.flag)
        nodata = np.array(matrix.nodata)
        xtitle = matrix.axes[0].name
        xdata = np.array(matrix.axes[0].data)
        ytitle = matrix.axes[1].name
        ydata = np.array(matrix.axes[1].data) 
        spw = matrix.spw
        table = matrix.filename
        pol = matrix.pol
        antenna = matrix.ant
        if antenna is not None:
            # deal with antenna id not name
            antenna = antenna[0]

        # If requested to use antenna names instead of IDs antenna,
        # create an id-to-name translation and check to make sure this
        # would result in unique non-empty names for all IDs, otherwise
        # revert back to flagging by ID
        if self.inputs.use_antenna_names:
            
            # create translation dictionary, reject empty antenna name strings
            antenna_id_to_name = {ant.id: ant.name for ant in self.inputs.ms.antennas if ant.name.strip()}

            # Check that each antenna ID is represented by a unique non-empty name, by testing that the 
            # unique set of antenna names is same length as list of IDs. If not, then unset the 
            # translation dictionary to revert back to flagging by ID
            if len(set(antenna_id_to_name.values())) != len(self.inputs.ms.antennas):
                LOG.info('No unique name available for each antenna ID: flagging by antenna ID instead of by name.')
                antenna_id_to_name = None
        else:
            antenna_id_to_name = None
            
        # If requested, expand current spw to all spws within the same baseband, thus
        # changing spw from an integer to a list of integers
        if self.inputs.extendbaseband:
            ms = self.inputs.context.observing_run.get_ms(self.inputs.vis)
            baseband = ms.get_spectral_window(spw).baseband
            spw = [spw.id for spw in ms.get_spectral_windows() if spw.baseband == baseband]

        # Initialize flags
        newflags = []
        flag_reason = np.zeros(np.shape(flag), np.int)

        # Index arrays
        i,j = np.indices(np.shape(data))

        rdata = np.ravel(data)
        rflag = np.ravel(flag)
        valid_data = rdata[np.logical_not(rflag)]

        # If there is valid data (non-flagged), then proceed with flagging
        if len(valid_data) > 0:

            # calculate statistics for valid data
            data_median, data_mad = arrayflaggerbase.median_and_mad(valid_data)
            
            # flag data according to each rule in turn
            for rule in rules:
                rulename = rule['name']

                if rulename == 'outlier':
  
                    # Sample too small
                    minsample = rule['minsample']
                    if len(valid_data) < minsample:
                        continue

                    # Check limits.
                    mad_max = rule['limit']
                    i2flag = i[np.logical_and(np.abs(data - data_median) >\
                      mad_max * data_mad, np.logical_not(flag))]
                    j2flag = j[np.logical_and(np.abs(data - data_median) >\
                      mad_max * data_mad, np.logical_not(flag))]

                    # No flagged data.
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag command to flag data underlying the
                    # view.
                    flagcoords = zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='outlier',
                          filename=table, rulename=rulename,
                          spw=spw, antenna=antenna,
                          axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol,
                          antenna_id_to_name=antenna_id_to_name))
  
                    # Flag the view.
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'low outlier':

                    # Sample too small
                    minsample = rule['minsample']
                    if len(valid_data) < minsample:
                        continue

                    # Check limits.
                    mad_max = rule['limit']
                    i2flag = i[np.logical_and(data_median - data > \
                      mad_max * data_mad, np.logical_not(flag))]
                    j2flag = j[np.logical_and(data_median - data > \
                      mad_max * data_mad, np.logical_not(flag))]

                    # No flagged data.
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag data underlying the
                    # view.
                    flagcoords = zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='low_outlier',
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol,
                          antenna_id_to_name=antenna_id_to_name))

                    # Flag the view.
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'high outlier':

                    # Sample too small
                    minsample = rule['minsample']
                    if len(valid_data) < minsample:
                        continue

                    # Check limits.
                    mad_max = rule['limit']
                    i2flag = i[np.logical_and(data - data_median > \
                      mad_max * data_mad, np.logical_not(flag))]
                    j2flag = j[np.logical_and(data - data_median > \
                      mad_max * data_mad, np.logical_not(flag))]

                    # No flags
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag data underlying the
                    # view.
                    flagcoords = zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='high_outlier',
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol,
                          antenna_id_to_name=antenna_id_to_name))

                    # Flag the view.
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'min abs':

                    # Check limits.
                    limit = rule['limit']
                    i2flag = i[np.logical_and(np.abs(data) < limit,
                      np.logical_not(flag))]
                    j2flag = j[np.logical_and(np.abs(data) < limit,
                      np.logical_not(flag))]

                    # No flags
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag data underlying the
                    # view.
                    flagcoords = zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='min_abs',
                          filename=table, rulename=rulename, spw=spw,
                          axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol,
                          antenna_id_to_name=antenna_id_to_name))

                    # Flag the view
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'max abs':

                    # Check limits.
                    limit = rule['limit']
                    i2flag = i[np.logical_and(np.abs(data) > limit,
                      np.logical_not(flag))]
                    j2flag = j[np.logical_and(np.abs(data) > limit,
                      np.logical_not(flag))]

                    # No flags
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag data underlying the
                    # view.
                    flagcoords=zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='max_abs',
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol,
                          extendfields=self.inputs.extendfields,
                          antenna_id_to_name=antenna_id_to_name))

                    # Flag the view
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'too many flags':
 
                    maxfraction = rule['limit']
                    maxexcessflags = rule['excess limit']
                    axis = rule['axis']
                    axis = axis.upper().strip()

                    if axis == xtitle.upper().strip():

                        # Compute median number flagged
                        num_flagged = np.zeros([np.shape(data)[1]], np.int)
                        for iy in np.arange(len(ydata)):
                            num_flagged[iy] = len(data[:,iy][flag[:,iy]])
                        median_num_flagged = np.median(num_flagged)

                        # look along x axis
                        for iy in np.arange(len(ydata)):
                            if all(flag[:,iy]):
                                continue

                            # Compute fraction flagged
                            len_data = len(xdata)
                            len_no_data = len(data[:,iy][nodata[:,iy]])
                            len_flagged = len(data[:,iy][flag[:,iy]])
                            fractionflagged = (
                              float(len_flagged - len_no_data) /
                              float(len_data - len_no_data))
                            if fractionflagged > maxfraction:
                                i2flag = i[:,iy][np.logical_not(flag[:,iy])]
                                j2flag = j[:,iy][np.logical_not(flag[:,iy])]
                            else:
                                i2flag = np.zeros([0], np.int)
                                j2flag = np.zeros([0], np.int)

                            # likewise for maxexcessflags
                            if len_flagged > median_num_flagged + maxexcessflags:
                                i2flag = np.concatenate((i2flag, i[:,iy][np.logical_not(flag[:,iy])]))
                                j2flag = np.concatenate((j2flag, j[:,iy][np.logical_not(flag[:,iy])]))

                            # Add new flag commands to flag data underlying 
                            # the view.
                            flagcoords = zip(xdata[i2flag], ydata[j2flag])
                            for flagcoord in flagcoords:
                                newflags.append(arrayflaggerbase.FlagCmd(
                                  reason='too_many_flags',
                                  filename=table, rulename=rulename,
                                  spw=spw, antenna=antenna, 
                                  axisnames=[xtitle,ytitle],
                                  flagcoords=flagcoord, pol=pol,
                                  antenna_id_to_name=antenna_id_to_name))

                            # Flag the view
                            flag[i2flag, j2flag] = True
                            flag_reason[i2flag, j2flag] =\
                              self.flag_reason_index[rulename]

                    elif axis == ytitle.upper().strip():

                        # Compute median number flagged
                        num_flagged = np.zeros([np.shape(data)[0]], np.int)
                        for ix in np.arange(len(xdata)):
                            num_flagged[ix] = len(data[ix,:][flag[ix,:]])
                        median_num_flagged = np.median(num_flagged)

                        # look along y axis
                        for ix in np.arange(len(xdata)):
                            if all(flag[ix,:]):
                                continue

                            len_data = len(ydata)
                            len_no_data = len(data[ix,:][nodata[ix,:]])
                            len_flagged = len(data[ix,:][flag[ix,:]])
                            fractionflagged = (
                              float(len_flagged - len_no_data) / 
                              float(len_data - len_no_data))
                            if fractionflagged > maxfraction:
                                i2flag = i[ix,:][np.logical_not(flag[ix,:])]
                                j2flag = j[ix,:][np.logical_not(flag[ix,:])]
                            else:
                                i2flag = np.zeros([0], np.int)
                                j2flag = np.zeros([0], np.int)

                            len_flagged = len(data[ix,:][flag[ix,:]])
                            if len_flagged > median_num_flagged + maxexcessflags:
                                i2flag = np.concatenate((i2flag, i[ix,:][np.logical_not(flag[ix,:])]))
                                j2flag = np.concatenate((j2flag, j[ix,:][np.logical_not(flag[ix,:])]))

                            # Add new flag commands to flag data underlying 
                            # the view.
                            flagcoords = zip(xdata[i2flag], ydata[j2flag])
                            for flagcoord in flagcoords:
                                newflags.append(arrayflaggerbase.FlagCmd(
                                  reason='too_many_flags',
                                  filename=table, rulename=rulename, spw=spw,
                                  axisnames=[xtitle, ytitle],
                                  flagcoords=flagcoord, pol=pol,
                                  antenna_id_to_name=antenna_id_to_name))

                            # Flag the view.
                            flag[i2flag, j2flag] = True
                            flag_reason[i2flag, j2flag] =\
                              self.flag_reason_index[rulename]

                elif rulename == 'too many entirely flagged':
 
                    maxfraction = rule['limit']
                    axis = rule['axis']
                    axis = axis.upper().strip()

                    # if flagging for each element on x-axis (i.e. evaluate column by column)
                    if axis == xtitle.upper().strip():
                        
                        # Determine fraction of columns that are entirely flagged
                        frac_ef = np.count_nonzero(np.all(flag, axis=1)) / float(flag.shape[0])

                        # If the fraction of "entirely flagged" columns exceeds the limit, then
                        # all non-flagged data will need to be flagged.
                        if frac_ef > maxfraction:
                            
                            # Indices to flag are all those that are currently not flagged
                            i2flag = i[np.logical_not(flag)]
                            j2flag = j[np.logical_not(flag)]
                            
                            # Add new flag commands to flag data underlying 
                            # the view.
                            flagcoords = zip(xdata[i2flag], ydata[j2flag])
                            for flagcoord in flagcoords:
                                newflags.append(arrayflaggerbase.FlagCmd(
                                  reason='too_many_flags',
                                  filename=table, rulename=rulename,
                                  spw=spw, antenna=antenna, 
                                  axisnames=[xtitle,ytitle],
                                  flagcoords=flagcoord, pol=pol,
                                  antenna_id_to_name=antenna_id_to_name))

                            # Flag the view
                            flag[i2flag, j2flag] = True
                            flag_reason[i2flag, j2flag] =\
                              self.flag_reason_index[rulename]
                
                elif rulename == 'nmedian':

                    # Check for valid median
                    if data_median is None:
                        continue

                    # Check limits.
                    lo_limit = rule['lo_limit']
                    hi_limit = rule['hi_limit']
                    i2flag = i[np.logical_and(
                      np.logical_or(data < lo_limit * data_median,
                      data > hi_limit * data_median),
                      np.logical_not(flag))]
                    j2flag = j[np.logical_and(
                      np.logical_or(data < lo_limit * data_median,
                      data > hi_limit * data_median),
                      np.logical_not(flag))]

                    # No flags
                    if len(i2flag) <= 0:
                        continue

                    # Add new flag commands to flag the data underlying
                    # the view.
                    flagcoords=zip(xdata[i2flag], ydata[j2flag])
                    for flagcoord in flagcoords:
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='nmedian',
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=flagcoord, pol=pol,
                          extendfields=self.inputs.extendfields,
                          antenna_id_to_name=antenna_id_to_name))

                    # Flag the view.
                    flag[i2flag, j2flag] = True
                    flag_reason[i2flag, j2flag] =\
                      self.flag_reason_index[rulename]

                elif rulename == 'bad antenna':
                    # this test should be run before the others 
                    # as it depends on no other
                    # flags having been set by other rules before it
                    # (because the number of unflagged points
                    # on entry are part of the test)

                    # Check limits.
                    mad_max = rule['lo_limit']
                    frac_limit = rule['frac_limit']
                    number_limit = rule['number_limit']
                    minsample = rule['minsample']

                    if 'ANTENNA' in xtitle.upper():

                        i, _ = np.indices(np.shape(data))

                        for ant in range(np.shape(flag)[0]):
                            ant_data = data[ant,:]
                            ant_flag = flag[ant,:]
                            valid_ant_data = ant_data[np.logical_not(ant_flag)]

                            # Sample too small?
                            if len(valid_ant_data) < minsample:
                                continue

                            # find low outlier flags first
                            j_ant = np.arange(np.shape(ant_flag)[0])
                            j2flag_lo = j_ant[np.logical_and(data_median - ant_data > \
                              mad_max * data_mad, np.logical_not(ant_flag))]
 
                            # No flagged data?
                            if len(j2flag_lo) <= 0:
                                continue

                            # is the antenna bad?
                            nflags = len(j2flag_lo)
                            flagsfrac = float(nflags) / float(np.shape(ant_flag)[0])

                            if nflags >= number_limit or \
                               flagsfrac > frac_limit:

                                # Only once we get here do we actually flag
                                # the view and the data

                                # first flag the view, specifying the reason
                                # for each flagged point
                                i2flag = np.zeros(np.shape(j2flag_lo), np.int)
                                i2flag += ant

                                ant_flag[j2flag_lo] = True
                                flag_reason[i2flag, j2flag_lo] = \
                                  self.flag_reason_index['low outlier']

                                j2flag_bad = j_ant[np.logical_not(ant_flag)]
                                i2flag_bad = np.zeros(np.shape(j2flag_bad), np.int)
                                i2flag_bad += ant

                                ant_flag[j2flag_bad] = True
                                flag_reason[i2flag_bad, j2flag_bad] = \
                                  self.flag_reason_index['bad antenna']

                                # copy flags back into view
                                data[ant,:] = ant_data
                                flag[ant,:] = ant_flag

                                # Just flag the entire antenna in the data
                                newflags.append(arrayflaggerbase.FlagCmd(
                                  reason='bad antenna', filename=table,
                                  rulename=rulename, spw=spw, pol=pol,
                                  antenna=ant,
                                  antenna_id_to_name=antenna_id_to_name))

                elif rulename == 'bad quadrant':
                    # this test should be run before the others 
                    # as it depends on no other
                    # flags having been set by other rules before it
                    # (because the number of unflagged points
                    # on entry are part of the test)

                    # a quadrant is one quarter of the extent of the x-axis

                    # Check limits.
                    hilo_limit = rule['hilo_limit']
                    frac_limit = rule['frac_limit']
                    baseline_frac_limit = rule['baseline_frac_limit']

                    # find outlier flags first                 
                    i2flag = i[np.logical_and(np.abs(data - data_median) >\
                      hilo_limit * data_mad, np.logical_not(flag))]
                    j2flag = j[np.logical_and(np.abs(data - data_median) >\
                      hilo_limit * data_mad, np.logical_not(flag))]

                    # No flagged data.
                    if len(i2flag) <= 0:
                        continue

                    # have to be careful here not to corrupt the data view
                    # as we go through it testing for bad quadrant/antenna.
                    # Make a copy of the view flags and go though these
                    # one antenna at a time testing for bad quadrant.
                    # If bad, copy 'outlier' and 'bad quadrant' flags to
                    # original view. 'unflagged_flag_copy' is a copy
                    # of the flags made before the 'outlier' flags are
                    # applied - it is used to estimate how many 
                    # _additional_ points have been flagged.
                    unflagged_flag_copy = np.copy(flag)
                    flag_copy = np.copy(flag)
                    flag_reason_copy = np.copy(flag_reason)
                    flag_copy[i2flag, j2flag] = True
                    flag_reason_copy[i2flag, j2flag] = \
                      self.flag_reason_index['outlier']

                    # look for bad antenna/quadrants in view copy
                    data_shape = np.shape(data)
                    nchan = data_shape[0]
                    nbaseline = data_shape[1]
                    nant = int(math.sqrt(nbaseline))

                    quadrant = [[0,nchan/4-1],
                      [nchan/4,nchan/2-1],
                      [nchan/2,nchan*3/4-1],
                      [nchan*3/4,nchan-1]]
                    
                    for ant in range(nant):
                        # flag based on outliers in flag_copy, will set new flags
                        # in a further copy so that outlier flags are not corrupted
                        working_copy_flag = np.copy(flag_copy)
                        working_copy_flag_reason = np.copy(flag_reason_copy)

                        # baselines involving this antenna
                        baselines = [baseline for baseline in range(nbaseline) if 
                          (baseline >= ant*nant and baseline < (ant+1)*nant) or
                          (baseline%nant == ant)]
                        baselines = np.array(baselines)

                        for iquad in range(4):
                            quad_slice = slice(quadrant[iquad][0], quadrant[iquad][1])
                            ninvalid = np.count_nonzero(
                              working_copy_flag[quad_slice,baselines])
                            ninvalid_on_entry = np.count_nonzero(
                              unflagged_flag_copy[quad_slice,baselines])
                            nvalid_on_entry = np.count_nonzero(np.logical_not(
                              unflagged_flag_copy[quad_slice,baselines]))
                            if nvalid_on_entry:
                                frac = float(ninvalid - ninvalid_on_entry) /\
                                  float(nvalid_on_entry)
                            else:
                                frac = 0.0

                            if frac > frac_limit:
                                # Add new flag commands to flag the data 
                                # underlying the view. These will flag the entire 
                                # quadrant/antenna. If the quadrant is not
                                # bad then any 'outlier' points found
                                # earlier will not be flagged.
                                flagcoords = []
                                for chan in range(quadrant[iquad][0], 
                                  quadrant[iquad][1]):
                                    flagcoords.append((chan,ant))
                                for flagcoord in flagcoords:
                                    newflags.append(arrayflaggerbase.FlagCmd(
                                      reason='bad quadrant',
                                      filename=table, rulename=rulename,
                                      spw=spw, axisnames=[xtitle, ytitle],
                                      flagcoords=flagcoord, pol=pol,
                                      extendfields=self.inputs.extendfields,
                                      antenna_id_to_name=antenna_id_to_name))

                                # update working copy view with 'bad quadrant' flags 
                                i2flag = i[quad_slice,baselines]\
                                  [np.logical_not(working_copy_flag[quad_slice,baselines])]
                                j2flag = j[quad_slice,baselines]\
                                  [np.logical_not(working_copy_flag[quad_slice,baselines])]

                                if len(i2flag) > 0:
                                    working_copy_flag[i2flag, j2flag] = True
                                    working_copy_flag_reason[i2flag, j2flag] =\
                                      self.flag_reason_index['bad quadrant']
 
                                # copy flag state for this antenna 
                                # back to original
                                flag[quad_slice,baselines] = \
                                  working_copy_flag[quad_slice,baselines]
                                flag_reason[quad_slice,baselines] = \
                                  working_copy_flag_reason[quad_slice,baselines]

                                # whole antenna/quadrant flagged, no need to check 
                                # individual baselines
                                continue

                            # look for bad quadrant/baseline
                            for baseline in baselines:
                                ninvalid = np.count_nonzero(
                                  working_copy_flag[quad_slice,baseline])
                                ninvalid_on_entry = np.count_nonzero(
                                  unflagged_flag_copy[quad_slice,baseline])
                                nvalid_on_entry = np.count_nonzero(
                                  np.logical_not(unflagged_flag_copy[
                                  quad_slice,baseline]))
                                if nvalid_on_entry:
                                    frac = float(ninvalid - ninvalid_on_entry) /\
                                      float(nvalid_on_entry)
                                else:
                                    frac = 0.0

                                if frac > baseline_frac_limit:

                                    # Add new flag commands to flag the data 
                                    # underlying the view. These will flag the entire 
                                    # quadrant/baseline. If the quadrant is not
                                    # bad then any 'outlier' points found
                                    # earlier will not be flagged.
                                    flagcoords = []
                                    for chan in range(quadrant[iquad][0], 
                                      quadrant[iquad][1]):
                                        flagcoords.append((chan,ydata[baseline]))
                                    for flagcoord in flagcoords:
                                        newflags.append(arrayflaggerbase.FlagCmd(
                                          reason='bad quadrant',
                                          filename=table, rulename=rulename,
                                          spw=spw, axisnames=[xtitle, ytitle],
                                          flagcoords=flagcoord, pol=pol,
                                          extendfields=self.inputs.extendfields,
                                          antenna_id_to_name=antenna_id_to_name))

                                    # update working copy view with 'bad quadrant' flags 
                                    i2flag = i[quad_slice,baseline]\
                                      [np.logical_not(working_copy_flag[quad_slice,baseline])]
                                    j2flag = j[quad_slice,baseline]\
                                      [np.logical_not(working_copy_flag[quad_slice,baseline])]

                                    if len(i2flag) > 0:
                                        working_copy_flag[i2flag, j2flag] = True
                                        working_copy_flag_reason[i2flag, j2flag] =\
                                          self.flag_reason_index['bad quadrant']

                                    # copy flag state for this antenna 
                                    # back to original
                                    flag[quad_slice,baseline] = \
                                      working_copy_flag[quad_slice,baseline]
                                    flag_reason[quad_slice,baseline] = \
                                      working_copy_flag_reason[quad_slice,baseline]

                else:           
                    raise NameError, 'bad rule: %s' % rule

        # consolidate flagcmds that specify individual channels into fewer
        # flagcmds that specify ranges
        newflags = arrayflaggerbase.consolidate_flagcmd_channels(newflags)

        return newflags, flag_reason


class VectorFlaggerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, datatask=None,
        flagsettertask=None, rules=None, niter=None, intent=None, prepend=''):

        # set the properties to the values given as input arguments
        self._init_properties(vars())


class VectorFlagger(basetask.StandardTaskTemplate):
    Inputs = VectorFlaggerInputs

    # override the inherited __init__ method so that references to the
    # task objects can be kept outside self.inputs. Later on self.inputs
    # will be replaced by a copy which breaks the connection between
    # its references to the tasks and the originals.
    def __init__(self, inputs):
        self.inputs = inputs
        self.datatask = inputs.datatask
        self.flagsettertask = inputs.flagsettertask

    def prepare(self):
        inputs = self.inputs

        datatask = self.datatask
        rules = inputs.rules
        flagsettertask = self.flagsettertask
        niter = inputs.niter
        intent = inputs.intent

        iter = 1
        flags = []
        while iter <= niter:

            # Get latest data
            result = self._executor.execute(datatask)
            if not result:
                return []

            descriptionlist = result.descriptions()
            descriptionlist.sort()
            newflags = []
            for description in descriptionlist:
                image = result.last(description)
                # get flags for this view according to the rules
                newflags += self.generate_flags(image, rules, intent)

            # set any flags raised
            if newflags:
                LOG.warning('%s%s iteration %s raised %s flagging commands' % \
                  (inputs.prepend, os.path.basename(inputs.vis), iter, len(newflags)))
            else:
                LOG.info('%s%s iteration %s raised %s flagging commands' % \
                  (inputs.prepend, os.path.basename(inputs.vis), iter, len(newflags)))
            flagsettertask.flags_to_set(newflags)
            ignore = self._executor.execute(flagsettertask)

            # accumulate all flags set
            flags += newflags

            if len(newflags) == 0:
                break

            iter += 1

        if len(newflags) != 0:
            # exited loop when flags still being found, recalculate
            # result with latest flags incorporated.
            result = self._executor.execute(datatask)

        # add a record of flags set to the results object
        result.addflags(flags)
        result.table = flagsettertask.inputs.table

        return result

    def analyse(self, result):
        return result

    @staticmethod
    def make_flag_rules (flag_edges=False, edge_limit=2.0,
      flag_minabs=False, fmin_limit=0.0,
      flag_nmedian=False, fnm_lo_limit=0.7, fnm_hi_limit=1.3,
      flag_hilo=False, fhl_limit=5.0, fhl_minsample=5, 
      flag_sharps=False, sharps_limit=0.05,
      flag_diffmad=False, diffmad_limit=10, diffmad_nchan_limit=4,
      flag_tmf=None, tmf_frac_limit=0.1, tmf_nchan_limit=4):

        """
        Generate a list of flagging rules from a set of flagging parameters.
        Added detailed docs here.
        """

        # Construct rules from flag properties
        rules = []
        if flag_edges:
            rules.append({'name':'edges', 'limit':edge_limit})
        if flag_minabs:
            rules.append({'name':'min abs', 'limit':fmin_limit})
        if flag_nmedian:
            rules.append({'name':'nmedian', 'lo_limit':fnm_lo_limit,
              'hi_limit':fnm_hi_limit})
        if flag_hilo:
            rules.append({'name':'outlier', 'limit':fhl_limit,
              'minsample':fhl_minsample})
        if flag_sharps:
            rules.append({'name':'sharps', 'limit':sharps_limit})
        if flag_diffmad:
            rules.append({'name':'diffmad', 'limit':diffmad_limit,
              'nchan_limit':diffmad_nchan_limit})
        if flag_tmf:
            rules.append({'name':'tmf', 'frac_limit':tmf_frac_limit,
              'nchan_limit':tmf_nchan_limit})

        return rules

    def generate_flags(self, vector, rules, intent):
        """
        Calculate the statistics of a vector and flag the data according
        to a list of specified rules.

        Keyword arguments:
        vector - SpectrumResult object containing data to be flagged.
        rules - Rules to be applied.
        """

        # Get the attributes - ensure all arrays are numpy arrays
        # as some subsequent processing depends on numpy array indexing
        data = np.array(vector.data)
        flag = np.array(vector.flag)
        nodata = np.array(vector.nodata)
        xtitle = vector.axis.name
        xdata = np.array(vector.axis.data)
        spw = vector.spw
        pol = vector.pol
        antenna = vector.ant
        if antenna is not None:
            # deal with antenna id not name
            antenna = antenna[0]
        table = vector.filename

        # any flags found will apply to this subset of the data
        axisnames = []
        flagcoords = []
        if antenna is not None:
            axisnames.append('ANTENNA1')
            flagcoords.append(antenna)

        # Initialize flags
        newflags = []

        # Index arrays
        i = np.indices(np.shape(data))

        rdata = np.ravel(data)
        rflag = np.ravel(flag)
        valid_data = rdata[np.logical_not(rflag)]

        # If there is valid data (non-flagged), then proceed with flagging
        if len(valid_data) > 0:

            # calculate statistics for valid data
            data_median, data_mad = arrayflaggerbase.median_and_mad(valid_data)

            # flag data according to each rule in turn
            for rule in rules:
                rulename = rule['name']
                if rulename == 'edges':
                    limit = rule['limit']
                    if len(valid_data):

                        # find left edge
                        left_edge = VectorFlagger._find_small_diff(rdata,
                          rflag, limit, vector.description)

                        # and right edge
                        reverse_data = rdata[-1::-1]
                        reverse_flag = rflag[-1::-1]
                        right_edge = VectorFlagger._find_small_diff(
                          reverse_data, reverse_flag, limit, vector.description)

                        # flag the 'view'
                        rflag[:left_edge] = True
                        if right_edge > 0:
                            rflag[-right_edge:] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[np.logical_or(
                          channels < left_edge,
                          channels > (nchannels-1-right_edge))]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]
                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='edges',
                              filename=table,
                              rulename=rulename, spw=spw, axisnames=axisnames,
                              flagcoords=flagcoords))

                elif rulename == 'min abs':
                    limit = rule['limit']
                    if len(valid_data):

                        flag_chan = (np.abs(data)<limit) & np.logical_not(flag)

                        # flag the 'view'
                        rflag[flag_chan] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[flag_chan]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]

                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='min_abs',
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=axisnames,
                              flagcoords=flagcoords,
                              channel_axis=vector.axis,
                              intent=intent))

                elif rulename == 'nmedian':
                    limit = rule['limit']
                    if len(valid_data):

                        if limit < 1:
                            flag_chan = (data < np.median(data)*limit) & np.logical_not(flag)
                        else:
                            flag_chan = (data > np.median(data)*limit) & np.logical_not(flag)

                        # flag the 'view'
                        rflag[flag_chan] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[flag_chan]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]

                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='nmedian',
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=axisnames,
                              flagcoords=flagcoords,
                              channel_axis=vector.axis,
                              intent=intent))

                elif rulename == 'outlier':
                    minsample = rule['minsample']
                    limit = rule['limit']

                    # enough data for valid statistics?
                    if len(valid_data) < minsample:
                        continue
  
                    # get channels to flag
                    flag_chan = (np.abs(data-data_median) > data_mad*limit) & np.logical_not(flag)

                    # flag the 'view'
                    rflag[flag_chan] = True

                    # now compose a description of the flagging required on
                    # the MS
                    nchannels = len(rdata)
                    channels = np.arange(nchannels)
                    channels_flagged = channels[flag_chan]

                    axisnames = ['channels']
                    flagcoords = [list(channels_flagged)]

                    if len(channels_flagged) > 0:
                        # Add new flag command to flag data underlying the
                        # view.
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='outlier',
                          filename=table, rulename=rulename,
                          spw=spw, pol=pol, antenna=antenna,
                          axisnames=axisnames, flagcoords=flagcoords))

                elif rulename == 'sharps':
                    limit = rule['limit']
                    if len(valid_data):
                        diff = abs(rdata[1:] - rdata[:-1])
                        diff_flag = (rflag[1:] | rflag[:-1])

                        # flag channels whose slope is greater than the 
                        # limit for a 'sharp feature'
                        newflag = (diff>limit) & np.logical_not(diff_flag)

                        # now broaden the flags until the diff falls below
                        # 2 times the median diff, to catch the wings of
                        # sharp features
                        if np.any([np.logical_not(diff_flag | newflag)]):
                            median_diff = np.median(diff[np.logical_not(
                              diff_flag | newflag)])
                            median_flag = ((diff > 2 * median_diff) &
                              np.logical_not(diff_flag)) 
                        else:
                            median_flag = newflag

                        start = None
                        for i in np.arange(len(median_flag)):
                            if median_flag[i]:
                                end = i
                                if start is None:
                                    start = i
                            else:
                                if start is not None:
                                    # have found start and end of a block
                                    # of contiguous True flags. Does the
                                    # block contain a sharp feature? If 
                                    # so broaden the sharp feature flags 
                                    # to include the whole block
                                    if np.any(newflag[start:end]):
                                        newflag[start:end] = True
                                    start = None

                        flag_chan = np.zeros([len(newflag)+1], np.bool) 
                        flag_chan[:-1] = newflag
                        flag_chan[1:] = (flag_chan[1:] | newflag)

                        # flag the 'view'
                        rflag[flag_chan] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[flag_chan]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]

                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='sharps',
                              filename=table,
                              rulename=rulename,
                              spw=spw, antenna=antenna, axisnames=axisnames,
                              flagcoords=flagcoords))

                elif rulename == 'diffmad':
                    limit = rule['limit']
                    nchan_limit = rule['nchan_limit']
                    if len(valid_data):
                        diff = rdata[1:] - rdata[:-1]
                        diff_flag = np.logical_or(rflag[1:], rflag[:-1])
                        median_diff = np.median(diff[diff_flag==0])
                        mad = np.median(
                          np.abs(diff[diff_flag==0] - median_diff))

                        # first, flag channels further from the median than
                        # limit * MAD
                        newflag = (abs(diff-median_diff) > limit*mad) & \
                          (diff_flag==0)

                        # second, flag all channels if more than nchan_limit 
                        # were flagged by the first stage
                        if len(newflag[newflag==True]) >= nchan_limit:
                            newflag[newflag==False] = True         

                        # set channels flagged 
                        flag_chan = np.zeros([len(newflag)+1], np.bool) 
                        flag_chan[:-1] = newflag
                        flag_chan[1:] = np.logical_or(flag_chan[1:], newflag)

                        # flag the 'view'
                        rflag[flag_chan] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[flag_chan]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]

                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='diffmad',
                              filename=table, rulename=rulename,
                              spw=spw, pol=pol, antenna=antenna,
                              axisnames=axisnames, flagcoords=flagcoords))

                elif rulename == 'tmf':
                    frac_limit = rule['frac_limit']
                    nchan_limit = rule['nchan_limit']
                    if len(valid_data):
                        # flag all channels if fraction already flagged 
                        # is greater than tmf_limit of total
                        if (float(len(rdata[rflag==True])) / len(rdata) >= frac_limit) or \
                          (len(rdata[rflag==True]) >= nchan_limit):
                            newflag = (rflag==False)

                            # flag the 'view'
                            rflag[newflag] = True

                            # now compose a description of the flagging required on
                            # the MS
                            nchannels = len(rdata)
                            channels = np.arange(nchannels)
                            channels_flagged = channels[newflag]

                            axisnames = ['channels']
                            flagcoords = [list(channels_flagged)]

                            if len(channels_flagged) > 0:
                                # Add new flag command to flag data underlying the
                                # view.
                                newflags.append(arrayflaggerbase.FlagCmd(
                                  reason='tmf',
                                  filename=table, rulename=rulename,
                                  spw=spw, pol=pol, antenna=antenna,
                                  axisnames=axisnames, flagcoords=flagcoords))

                else:           
                    raise NameError, 'bad rule: %s' % rule

        return newflags

    @staticmethod
    def _find_noise_edge(mad, flag):
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
        median_mad = np.median(mad[:nchan/4][np.logical_not(flag[:nchan/4])])
        for i in range(nchan):
            if not flag[i]:
                if mad[i] < 2.0 * median_mad:
                    noise_edge = i
                    break

        return noise_edge

    @staticmethod
    def _find_small_diff(data, flag, limit=2.0, description='unknown'):
        """Return the index in the first quarter of the data array where the
        point to point difference first falls below twice the median value.

        Keyword arguments:
        data -- The data array to be examined.
        flag -- Array whose elements are True where list_data is bad.
        limit -- Multiple of median value where the 'edge' will be set.

        Returns:
        The index of the first point where the point to point difference
        first falls below 'limit' times the median value.
        """
        result = None

        nchan = len(data)
        good_data = data[:nchan/4][np.logical_not(flag[:nchan/4])]
        good_data_index = np.arange(nchan/4)[np.logical_not(flag[:nchan/4])]
        diff = abs(good_data[1:] - good_data[:-1])
        median_diff = np.median(diff)

        for i in good_data_index[:-2]:
            if diff[i] < limit * median_diff:
                result = i
                break

        if result is None:
            LOG.warning('edge finder failed for:%s' % description)
            # flag one edge channel - sole purpose of this is to ensure
            # that a plot is made in the weblog so that the problem
            # can be understood
            result = 1

        return result


class NewVectorFlaggerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, datatask=None,
        viewtask=None, flagsettertask=None, rules=None, niter=None, 
        iter_datatask=None, prepend=''):

        # set the properties to the values given as input arguments
        self._init_properties(vars())


class NewVectorFlaggerResults(basetask.Results,
  flaggableviewresults.FlaggableViewResults):
    def __init__(self):
        """
        Construct and return a new NewVectorFlaggerResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'NewVectorFlaggerResults'
        return s


class NewVectorFlagger(basetask.StandardTaskTemplate):
    Inputs = NewVectorFlaggerInputs

    # override the inherited __init__ method so that references to the
    # task objects can be kept outside self.inputs. Later on self.inputs
    # will be replaced by a copy which breaks the connection between
    # its references to the tasks and the originals.
    def __init__(self, inputs):
        self.inputs = inputs
        self.result = NewVectorFlaggerResults()

    def prepare(self):
        
        inputs = self.inputs

        # Initialize flags and iteration counter
        flags = []
        newflags = []
        counter = 1
        include_before = True

        # Start iterative flagging
        while counter <= inputs.niter:

            # Run the data task if needed
            if counter == 1:
                # Always run data task on first iteration
                data = self._executor.execute(self.inputs.datatask)
            elif inputs.iter_datatask is True:
                # If requested to re-run datatask on iteration, then
                # run the flag-setting task which modifies the data
                # and then re-run the data task
                
                # If no "before summary" was done, include this in the flag setting task
                if include_before:
                    stats_before, _ = self.set_flags(newflags, summarize_before=True)
                    include_before = False
                else:
                    _, _ = self.set_flags(newflags)
                    
                data = self._executor.execute(inputs.datatask)
            else:
                # TODO: implement flagging mask for the case where iter_datatask = False
                raise(NotImplementedError)

            # Create flagging view                
            viewresult = inputs.viewtask(data)

            # If a view could be created, continue with flagging
            if viewresult.descriptions():
            
                # Import the views from viewtask into the final result
                self.result.importfrom(viewresult)
    
                # Flag the view
                newflags = self.flag_view(viewresult)
            
                # Report how many flags were found in this iteration and
                # stop iteration if no new flags were found
                if len(newflags) == 0:
                    # If no new flags are found, report as a log message
                    LOG.info('%s%s iteration %s raised %s flagging commands' % \
                             (inputs.prepend, os.path.basename(inputs.vis), counter, len(newflags)))
                    break
                else:
                    # Report newly found flags as a warning message
                    LOG.warning('%s%s iteration %s raised %s flagging commands' % \
                                (inputs.prepend, os.path.basename(inputs.vis), counter, len(newflags)))
    
                # Accumulate new flags
                flags += newflags
                
                counter += 1
            else:
                # If no view could be created, exit the iteration
                break

        # Create final set of flags by removing duplicates from our accumulated flags
        flags = list(set(flags))
        
        # If flags were found...
        if len(flags) > 0:
            
            # If newflags were found on last iteration loop, we need to still set 
            # these.
            if len(newflags) > 0:
                
                # If datatask needs to be iterated, and max number of iterations is
                # not 1, then the "before" summary has already been done, so only 
                # need to set flags and do "after" summary
                if inputs.iter_datatask is True and inputs.niter != 1:
                    
                    # Set flags, and include "after" summary
                    _ , stats_after = self.set_flags(newflags,
                                                     summarize_after=True)   

                    # With new flags set, re-run the data task
                    data = self._executor.execute(inputs.datatask)
                                 
                # Otherwise, set flags, and include both "before" and "after" summary
                else:
                    stats_before, stats_after = self.set_flags(newflags, 
                                                               summarize_before=True, 
                                                               summarize_after=True)
                
                # Create final post-flagging view                
                viewresult = inputs.viewtask(data)

                # Import the post-flagging view into the final result
                self.result.importfrom(viewresult)
                
            # If no newflags were found on last iteration loop
            else:
                
                # If datatask needs to be iterated, then the "before" summary has
                # already been done, and the flags have already been set, so only
                # need to do an "after" summary
                if inputs.iter_datatask is True:
                    summary_job = casa_tasks.flagdata(vis=inputs.flagsettertask.inputs.table, mode='summary')
                    stats_after = self._executor.execute(summary_job)
                # Otherwise, set flags, and include both "before" and "after" summary
                else:
                    stats_before, stats_after = self.set_flags(newflags, 
                                                               summarize_before=True, 
                                                               summarize_after=True)
            
            # Store the final set of flags in the final result
            self.result.addflags(flags)
            
        # if no flags were found at all
        else:
            # Run a single flagging summary and use the result as both the "before" 
            # and "after" summary.
            summary_job = casa_tasks.flagdata(vis=inputs.flagsettertask.inputs.table, mode='summary')
            stats_before = self._executor.execute(summary_job)
            stats_after = stats_before        
        
        # Store in the final result the name of the measurement set or caltable to 
        # which any potentially found flags would need to be applied to
        self.result.table = inputs.flagsettertask.inputs.table
        
        # Store the flagging summaries in the final result
        self.result.summaries = [stats_before, stats_after]

        return self.result

    def analyse(self, result):
        return result

    def flag_view(self, view):
        newflags = []
        descriptionlist = view.descriptions()
        descriptionlist.sort() 
        for description in descriptionlist:
            image = view.last(description)
            # get flags for this view according to the rules
            newflags += self.generate_flags(image)

        return newflags
    
    def set_flags(self, flags, summarize_before=False, summarize_after=False):
        # Initialize flagging summaries
        stats_before = []
        stats_after = []
        allflagcmds = []

        # Add the "before" summary to the flagging commands
        if summarize_before:
            allflagcmds = ["mode='summary' name='before'"]
        
        # Add the flagging commands
        allflagcmds.extend(flags)
        
        # Add the "before" summary to the flagging commands
        if summarize_after:
            allflagcmds.append("mode='summary' name='after'")
        
        # Update flag setting task with all flagging commands
        self.inputs.flagsettertask.flags_to_set(allflagcmds)
        
        # Run flag setting task
        flagsetterresult = self._executor.execute(self.inputs.flagsettertask)

        # Extract "before" summary if necessary
        if summarize_before:
            stats_before = {}
            for report in flagsetterresult.results[0].keys():
                if flagsetterresult.results[0][report]['name'] == 'before':
                    stats_before = flagsetterresult.results[0][report]
        if summarize_after:
            stats_after = {}
            for report in flagsetterresult.results[0].keys():
                if flagsetterresult.results[0][report]['name'] == 'after':
                    stats_after = flagsetterresult.results[0][report]
        
        return stats_before, stats_after

    @staticmethod
    def make_flag_rules (flag_edges=False, edge_limit=2.0,
      flag_minabs=False, fmin_limit=0.0,
      flag_nmedian=False, fnm_lo_limit=0.7, fnm_hi_limit=1.3,
      flag_hilo=False, fhl_limit=5.0, fhl_minsample=5, 
      flag_sharps=False, sharps_limit=0.05,
      flag_diffmad=False, diffmad_limit=10, diffmad_nchan_limit=4,
      flag_tmf=None, tmf_frac_limit=0.1, tmf_nchan_limit=4):

        """
        Generate a list of flagging rules from a set of flagging parameters.
        Added detailed docs here.
        """

        # Construct rules from flag properties
        rules = []
        if flag_edges:
            rules.append({'name':'edges', 'limit':edge_limit})
        if flag_minabs:
            rules.append({'name':'min abs', 'limit':fmin_limit})
        if flag_nmedian:
            rules.append({'name':'nmedian', 'lo_limit':fnm_lo_limit,
              'hi_limit':fnm_hi_limit})
        if flag_hilo:
            rules.append({'name':'outlier', 'limit':fhl_limit,
              'minsample':fhl_minsample})
        if flag_sharps:
            rules.append({'name':'sharps', 'limit':sharps_limit})
        if flag_diffmad:
            rules.append({'name':'diffmad', 'limit':diffmad_limit,
              'nchan_limit':diffmad_nchan_limit})
        if flag_tmf:
            rules.append({'name':'tmf', 'frac_limit':tmf_frac_limit,
              'nchan_limit':tmf_nchan_limit})

        return rules

    def generate_flags(self, vector):
        """
        Calculate the statistics of a vector and flag the data according
        to a list of specified rules.

        Keyword arguments:
        vector - SpectrumResult object containing data to be flagged.
        rules - Rules to be applied.
        """

        # Get the attributes - ensure all arrays are numpy arrays
        # as some subsequent processing depends on numpy array indexing
        data = np.array(vector.data)
        flag = np.array(vector.flag)
        spw = vector.spw
        pol = vector.pol
        antenna = vector.ant
        if antenna is not None:
            # deal with antenna id not name
            antenna = antenna[0]
        table = vector.filename

        # any flags found will apply to this subset of the data
        axisnames = []
        flagcoords = []
        if antenna is not None:
            axisnames.append('ANTENNA1')
            flagcoords.append(antenna)

        # Initialize flags
        newflags = []

        # Index arrays
        i = np.indices(np.shape(data))

        rdata = np.ravel(data)
        rflag = np.ravel(flag)
        valid_data = rdata[np.logical_not(rflag)]

        # If there is valid data (non-flagged), then proceed with flagging
        if len(valid_data) > 0:

            # calculate statistics for valid data
            data_median, data_mad = arrayflaggerbase.median_and_mad(valid_data)

            # flag data according to each rule in turn
            for rule in self.inputs.rules:
                rulename = rule['name']
                if rulename == 'edges':
                    limit = rule['limit']
                    if len(valid_data):

                        # find left edge
                        left_edge = VectorFlagger._find_small_diff(rdata,
                          rflag, limit, vector.description)

                        # and right edge
                        reverse_data = rdata[-1::-1]
                        reverse_flag = rflag[-1::-1]
                        right_edge = VectorFlagger._find_small_diff(
                          reverse_data, reverse_flag, limit, vector.description)

                        # flag the 'view'
                        rflag[:left_edge] = True
                        if right_edge > 0:
                            rflag[-right_edge:] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[np.logical_or(
                          channels < left_edge,
                          channels > (nchannels-1-right_edge))]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]
                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='edges',
                              filename=table,
                              rulename=rulename, spw=spw, axisnames=axisnames,
                              flagcoords=flagcoords))

                elif rulename == 'min abs':
                    limit = rule['limit']
                    if len(valid_data):

                        flag_chan = (np.abs(data)<limit) & np.logical_not(flag)

                        # flag the 'view'
                        rflag[flag_chan] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[flag_chan]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]

                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='min_abs',
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=axisnames,
                              flagcoords=flagcoords,
                              channel_axis=vector.axis))

                elif rulename == 'nmedian':
                    limit = rule['limit']
                    if len(valid_data):

                        if limit < 1:
                            flag_chan = (data < np.median(data)*limit) & np.logical_not(flag)
                        else:
                            flag_chan = (data > np.median(data)*limit) & np.logical_not(flag)

                        # flag the 'view'
                        rflag[flag_chan] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[flag_chan]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]

                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='nmedian',
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=axisnames,
                              flagcoords=flagcoords,
                              channel_axis=vector.axis))

                elif rulename == 'outlier':
                    minsample = rule['minsample']
                    limit = rule['limit']

                    # enough data for valid statistics?
                    if len(valid_data) < minsample:
                        continue
  
                    # get channels to flag
                    flag_chan = (np.abs(data-data_median) > data_mad*limit) & np.logical_not(flag)

                    # flag the 'view'
                    rflag[flag_chan] = True

                    # now compose a description of the flagging required on
                    # the MS
                    nchannels = len(rdata)
                    channels = np.arange(nchannels)
                    channels_flagged = channels[flag_chan]

                    axisnames = ['channels']
                    flagcoords = [list(channels_flagged)]

                    if len(channels_flagged) > 0:
                        # Add new flag command to flag data underlying the
                        # view.
                        newflags.append(arrayflaggerbase.FlagCmd(
                          reason='outlier',
                          filename=table, rulename=rulename,
                          spw=spw, pol=pol, antenna=antenna,
                          axisnames=axisnames, flagcoords=flagcoords))

                elif rulename == 'sharps':
                    limit = rule['limit']
                    if len(valid_data):
                        diff = abs(rdata[1:] - rdata[:-1])
                        diff_flag = (rflag[1:] | rflag[:-1])

                        # flag channels whose slope is greater than the 
                        # limit for a 'sharp feature'
                        newflag = (diff>limit) & np.logical_not(diff_flag)

                        # now broaden the flags until the diff falls below
                        # 2 times the median diff, to catch the wings of
                        # sharp features
                        if np.any([np.logical_not(diff_flag | newflag)]):
                            median_diff = np.median(diff[np.logical_not(
                              diff_flag | newflag)])
                            median_flag = ((diff > 2 * median_diff) &
                              np.logical_not(diff_flag)) 
                        else:
                            median_flag = newflag

                        start = None
                        for i in np.arange(len(median_flag)):
                            if median_flag[i]:
                                end = i
                                if start is None:
                                    start = i
                            else:
                                if start is not None:
                                    # have found start and end of a block
                                    # of contiguous True flags. Does the
                                    # block contain a sharp feature? If 
                                    # so broaden the sharp feature flags 
                                    # to include the whole block
                                    if np.any(newflag[start:end]):
                                        newflag[start:end] = True
                                    start = None

                        flag_chan = np.zeros([len(newflag)+1], np.bool) 
                        flag_chan[:-1] = newflag
                        flag_chan[1:] = (flag_chan[1:] | newflag)

                        # flag the 'view'
                        rflag[flag_chan] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[flag_chan]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]

                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='sharps',
                              filename=table,
                              rulename=rulename,
                              spw=spw, antenna=antenna, axisnames=axisnames,
                              flagcoords=flagcoords))

                elif rulename == 'diffmad':
                    limit = rule['limit']
                    nchan_limit = rule['nchan_limit']
                    if len(valid_data):
                        diff = rdata[1:] - rdata[:-1]
                        diff_flag = np.logical_or(rflag[1:], rflag[:-1])
                        median_diff = np.median(diff[diff_flag==0])
                        mad = np.median(
                          np.abs(diff[diff_flag==0] - median_diff))

                        # first, flag channels further from the median than
                        # limit * MAD
                        newflag = (abs(diff-median_diff) > limit*mad) & \
                          (diff_flag==0)

                        # second, flag all channels if more than nchan_limit 
                        # were flagged by the first stage
                        if len(newflag[newflag==True]) >= nchan_limit:
                            newflag[newflag==False] = True         

                        # set channels flagged 
                        flag_chan = np.zeros([len(newflag)+1], np.bool) 
                        flag_chan[:-1] = newflag
                        flag_chan[1:] = np.logical_or(flag_chan[1:], newflag)

                        # flag the 'view'
                        rflag[flag_chan] = True

                        # now compose a description of the flagging required on
                        # the MS
                        nchannels = len(rdata)
                        channels = np.arange(nchannels)
                        channels_flagged = channels[flag_chan]

                        axisnames = ['channels']
                        flagcoords = [list(channels_flagged)]

                        if len(channels_flagged) > 0:
                            # Add new flag command to flag data underlying the
                            # view.
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='diffmad',
                              filename=table, rulename=rulename,
                              spw=spw, pol=pol, antenna=antenna,
                              axisnames=axisnames, flagcoords=flagcoords))

                elif rulename == 'tmf':
                    frac_limit = rule['frac_limit']
                    nchan_limit = rule['nchan_limit']
                    if len(valid_data):
                        # flag all channels if fraction already flagged 
                        # is greater than tmf_limit of total
                        if (float(len(rdata[rflag==True])) / len(rdata) >= frac_limit) or \
                          (len(rdata[rflag==True]) >= nchan_limit):
                            newflag = (rflag==False)

                            # flag the 'view'
                            rflag[newflag] = True

                            # now compose a description of the flagging required on
                            # the MS
                            nchannels = len(rdata)
                            channels = np.arange(nchannels)
                            channels_flagged = channels[newflag]

                            axisnames = ['channels']
                            flagcoords = [list(channels_flagged)]

                            if len(channels_flagged) > 0:
                                # Add new flag command to flag data underlying the
                                # view.
                                newflags.append(arrayflaggerbase.FlagCmd(
                                  reason='tmf',
                                  filename=table, rulename=rulename,
                                  spw=spw, pol=pol, antenna=antenna,
                                  axisnames=axisnames, flagcoords=flagcoords))

                else:           
                    raise NameError, 'bad rule: %s' % rule

        return newflags

    @staticmethod
    def _find_noise_edge(mad, flag):
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
        median_mad = np.median(mad[:nchan/4][np.logical_not(flag[:nchan/4])])
        for i in range(nchan):
            if not flag[i]:
                if mad[i] < 2.0 * median_mad:
                    noise_edge = i
                    break

        return noise_edge

    @staticmethod
    def _find_small_diff(data, flag, limit=2.0, description='unknown'):
        """Return the index in the first quarter of the data array where the
        point to point difference first falls below twice the median value.

        Keyword arguments:
        data -- The data array to be examined.
        flag -- Array whose elements are True where list_data is bad.
        limit -- Multiple of median value where the 'edge' will be set.

        Returns:
        The index of the first point where the point to point difference
        first falls below 'limit' times the median value.
        """
        result = None

        nchan = len(data)
        good_data = data[:nchan/4][np.logical_not(flag[:nchan/4])]
        good_data_index = np.arange(nchan/4)[np.logical_not(flag[:nchan/4])]
        diff = abs(good_data[1:] - good_data[:-1])
        median_diff = np.median(diff)

        for i in good_data_index[:-2]:
            if diff[i] < limit * median_diff:
                result = i
                break

        if result is None:
            LOG.warning('edge finder failed for:%s' % description)
            # flag one edge channel - sole purpose of this is to ensure
            # that a plot is made in the weblog so that the problem
            # can be understood
            result = 1

        return result

