from __future__ import absolute_import

import copy
import math
import os

import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.common import arrayflaggerbase
from pipeline.h.tasks.common import flaggableviewresults

LOG = infrastructure.get_logger(__name__)


class MatrixFlaggerInputs(vdp.StandardInputs):
    prepend = vdp.VisDependentProperty(default='')
    use_antenna_names = vdp.VisDependentProperty(default=True)

    def __init__(self, context, output_dir=None, vis=None, datatask=None, viewtask=None, flagsettertask=None,
                 rules=None, niter=None, extendfields=None, extendbaseband=None, iter_datatask=None,
                 use_antenna_names=None, prepend=None):
        super(MatrixFlaggerInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # solution parameters
        self.datatask = datatask
        self.extendbaseband = extendbaseband
        self.extendfields = extendfields
        self.flagsettertask = flagsettertask
        self.iter_datatask = iter_datatask
        self.niter = niter
        self.prepend = prepend
        self.rules = rules
        self.use_antenna_names = use_antenna_names
        self.viewtask = viewtask


class MatrixFlaggerResults(basetask.Results,
                           flaggableviewresults.FlaggableViewResults):
    def __init__(self, vis=None):
        """
        Construct and return a new MatrixFlaggerResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

        self.vis = vis
        self.dataresult = None
        self.viewresult = None

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'MatrixFlaggerResults'
        return s


class MatrixFlagger(basetask.StandardTaskTemplate):
    Inputs = MatrixFlaggerInputs

    flag_reason_index = {'max abs': 1,
                         'min abs': 2,
                         'nmedian': 3,
                         'outlier': 4,
                         'high outlier': 5,
                         'low outlier': 6,
                         'too many flags': 7,
                         'bad quadrant': 8,
                         'bad antenna': 9,
                         'too many entirely flagged': 10}
    flag_reason_key = {value: key for (key, value) in flag_reason_index.items()}

    # override the inherited __init__ method so that references to the
    # task objects can be kept outside self.inputs. Later on self.inputs
    # will be replaced by a copy which breaks the connection between
    # its references to the tasks and the originals.
    def __init__(self, inputs):
        self.inputs = inputs

    def prepare(self):
        inputs = self.inputs

        # Initialize result.
        result = MatrixFlaggerResults(vis=inputs.vis)

        # Expand flag commands to larger scope, if requested, by removing
        # selection in specified fields
        if inputs.extendfields:
            LOG.info(
                '{0} flagcmds will be extended by removing selection in'
                'following fields: {1}'.format(inputs.prepend,
                                               inputs.extendfields))

        # Expand flag commands to include all spws in a baseband, if requested
        if inputs.extendfields:
            LOG.info(
                '{0} flagcmds will be extended to include all spws within'
                'baseband.'.format(inputs.prepend))

        # Initialize flags, flag_reason, and iteration counter
        flags = []
        flag_reason_plane = {}
        newflags = []
        counter = 1
        include_before = True
        dataresult = None
        viewresult = None

        # Start iterative flagging
        while counter <= inputs.niter:

            # Run the data task if needed
            if counter == 1:
                # Always run data task on first iteration
                dataresult = self._executor.execute(inputs.datatask)
            elif inputs.iter_datatask is True:
                # If requested to re-run datatask on iteration, then
                # run the flag-setting task which modifies the data
                # and then re-run the data task
                
                # If no "before summary" was done, include this in the flag
                # setting task
                if include_before:
                    stats_before, _ = self.set_flags(newflags,
                                                     summarize_before=True)
                    include_before = False
                else:
                    _, _ = self.set_flags(newflags)
                    
                dataresult = self._executor.execute(inputs.datatask)
            else:
                # If not iterating the datatask, the previous 
                # data result will be re-used, but marked here as no 
                # longer new.
                dataresult.new = False

            # Create flagging view                
            viewresult = inputs.viewtask(dataresult)

            # If a view could be created, continue with flagging
            if viewresult.descriptions():
            
                # Import the views from viewtask into the final result
                result.importfrom(viewresult)
    
                # Flag the view
                newflags, newflags_reason = self.flag_view(viewresult,
                                                           inputs.rules)
            
                # Report how many flags were found in this iteration and
                # stop iteration if no new flags were found
                if len(newflags) == 0:
                    # If no new flags are found, report as a log message
                    LOG.info(
                        '{0}{1} iteration {2} raised {3} flagging'
                        ' commands'.format(inputs.prepend,
                                           os.path.basename(inputs.vis),
                                           counter, len(newflags)))
                    break
                else:
                    # Report newly found flags as a warning message
                    LOG.warning(
                        '{0}{1} iteration {2} raised {3} flagging'
                        ' commands'.format(inputs.prepend,
                                           os.path.basename(inputs.vis),
                                           counter, len(newflags)))
    
                # Accumulate new flags and flag reasons
                flags += newflags
                for description in newflags_reason.keys():
                    if description in flag_reason_plane:
                        flag_reason_plane[description][newflags_reason[description] > 0] = \
                            newflags_reason[description][newflags_reason[description] > 0]
                    else:
                        flag_reason_plane[description] = newflags_reason[description]
                
                counter += 1
            else:
                # If no view could be created, exit the iteration
                LOG.warning('No flagging view was created!')
                break

        # Create final set of flags by removing duplicates from our accumulated
        # flags
        flags = list(set(flags))
        
        # If flags were found...
        if len(flags) > 0:

            # If newflags were found on last iteration loop, we need to still
            # set these.
            if len(newflags) > 0:
                
                # If datatask needs to be iterated...
                if inputs.iter_datatask is True:

                    # First set the new flags that were found on the last
                    # iteration. If the "before" summary was not yet created,
                    # then include this here; always include the "after"
                    # summary.
                    if include_before:
                        # Set flags, and include "before" and "after" summary.
                        stats_before, stats_after = self.set_flags(
                            newflags, summarize_before=True,
                            summarize_after=True)
                    else:
                        # Set flags, and include "after" summary
                        _, stats_after = self.set_flags(
                            newflags, summarize_after=True)

                    # After setting the latest flags, re-run the data task.
                    dataresult = self._executor.execute(inputs.datatask)
                                 
                # If the datatask did not need to be iterated, then no flags
                # were set yet and no "before" summary was performed yet, so
                # set all flags and include both "before" and "after" summary.
                else:
                    stats_before, stats_after = self.set_flags(
                        flags, summarize_before=True, summarize_after=True)
                
                # Create final post-flagging view                
                viewresult = inputs.viewtask(dataresult)

                # Import the post-flagging view into the final result
                result.importfrom(viewresult)

            # If flags were found, but no newflags were found on last iteration
            # then the dataresult is already up-to-date, and all that is needed
            # is to ensure the flags are set, and that summaries are created.
            else:
                # If datatask needs to be iterated, then the "before" summary has
                # already been done, and the flags have already been set, so only
                # need to do an "after" summary.
                if inputs.iter_datatask is True:
                    _, stats_after = self.set_flags([], summarize_after=True)
                # If the datatask did not need to be iterated, then no flags
                # were set yet and no "before" summary was performed yet, 
                # so set all flags and include both "before" and "after" summary.
                else:
                    stats_before, stats_after = self.set_flags(
                        flags, summarize_before=True, summarize_after=True)
            
            # Store the final set of flags in the final result
            result.addflags(flags)
            
            # Store the flag reasons in the last (i.e. post-flagging) view in
            # the final result
            result.add_flag_reason_plane(flag_reason_plane,
                                         self.flag_reason_key)
        
        # if no flags were found at all
        else:
            # Run a single flagging summary and use the result as both the "before" 
            # and "after" summary.
            stats_before, _ = self.set_flags(flags, summarize_before=True)
            stats_after = copy.deepcopy(stats_before)
        
        # Store in the final result the name of the measurement set or caltable
        # to which any potentially found flags would need to be applied to.
        result.table = inputs.flagsettertask.inputs.table

        # Store in the final result the final data task result and the final
        # view task result.
        result.dataresult = dataresult
        result.viewresult = viewresult

        # Store the flagging summaries in the final result
        result.summaries = [stats_before, stats_after]

        # Sort the final list of flagging commands.
        result.sort_flagcmds()

        return result

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
            theseflags, this_flag_reason_plane = self.generate_flags(image,
                                                                     rules)

            # update flagging record
            newflags += theseflags
            newflags_reason[description] = this_flag_reason_plane

        return newflags, newflags_reason

    def set_flags(self, flags, summarize_before=False, summarize_after=False):
        # Initialize flag commands.
        allflagcmds = []

        # Add the "before" summary to the flagging commands.
        if summarize_before:
            allflagcmds = ["mode='summary' name='before'"]
        
        # Add the flagging commands.
        allflagcmds.extend(flags)
        
        # Add the "before" summary to the flagging commands.
        if summarize_after:
            allflagcmds.append("mode='summary' name='after'")
        
        # Update flag setting task with all flagging commands.
        self.inputs.flagsettertask.flags_to_set(allflagcmds)
        
        # Run flag setting task
        flagsetterresult = self._executor.execute(self.inputs.flagsettertask)

        # Initialize "before" and/or "after" summaries.
        stats_before = {}
        stats_after = {}

        # If the flagsetter returned results from the CASA flag data task,
        # then proceed to extract "before" and/or "after" flagging summaries.
        if flagsetterresult.results:
            if all(['report' in k for k in flagsetterresult.results[0].keys()]):
                # Go through dictionary of reports.
                for report in flagsetterresult.results[0].keys():
                    if flagsetterresult.results[0][report]['name'] == 'before':
                        stats_before = flagsetterresult.results[0][report]
                    if flagsetterresult.results[0][report]['name'] == 'after':
                        stats_after = flagsetterresult.results[0][report]
            else:
                # Go through single report.
                if flagsetterresult.results[0]['name'] == 'before':
                    stats_before = flagsetterresult.results[0]
                if flagsetterresult.results[0]['name'] == 'after':
                    stats_after = flagsetterresult.results[0]
        # In the alternate case, where no "real" flagsetter results were
        # returned (e.g. by WvrgcalFlagSetter), then there will have been no
        # real flagging summaries created, in which case empty dictionaries
        # are returned as empty summaries.
        else:
            pass

        return stats_before, stats_after

    @staticmethod
    def make_flag_rules(
            flag_hilo=False, fhl_limit=5.0, fhl_minsample=5,
            flag_hi=False, fhi_limit=5.0, fhi_minsample=5,
            flag_lo=False, flo_limit=5.0, flo_minsample=5,
            flag_tmf1=False, tmf1_axis='Time', tmf1_limit=1.0,
            tmf1_excess_limit=10000000,
            flag_tmf2=False, tmf2_axis='Time', tmf2_limit=1.0,
            tmf2_excess_limit=10000000,
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
            rules.append({'name': 'bad quadrant', 'hilo_limit': fbq_hilo_limit,
                          'frac_limit': fbq_antenna_frac_limit,
                          'baseline_frac_limit': fbq_baseline_frac_limit})
        if flag_bad_antenna:
            rules.append({'name': 'bad antenna', 'lo_limit': fba_lo_limit,
                          'frac_limit': fba_frac_limit,
                          'number_limit': fba_number_limit,
                          'minsample': fba_minsample})
        if flag_maxabs:
            rules.append({'name': 'max abs', 'limit': fmax_limit})
        if flag_minabs:
            rules.append({'name': 'min abs', 'limit': fmin_limit})
        if flag_nmedian:
            rules.append({'name': 'nmedian', 'lo_limit': fnm_lo_limit,
                          'hi_limit': fnm_hi_limit})
        if flag_hilo:
            rules.append({'name': 'outlier', 'limit': fhl_limit,
                          'minsample': fhl_minsample})
        if flag_hi:
            rules.append({'name': 'high outlier', 'limit': fhi_limit,
                          'minsample': fhi_minsample})
        if flag_lo:
            rules.append({'name': 'low outlier', 'limit': flo_limit,
                          'minsample': flo_minsample})
        if flag_tmf1:
            rules.append({'name': 'too many flags',
                          'axis': str.upper(tmf1_axis),
                          'limit': tmf1_limit,
                          'excess limit': tmf1_excess_limit})
        if flag_tmf2:
            rules.append({'name': 'too many flags',
                          'axis': str.upper(tmf2_axis),
                          'limit': tmf2_limit,
                          'excess limit': tmf2_excess_limit})
        if flag_tmef1:
            rules.append({'name': 'too many entirely flagged',
                          'axis': str.upper(tmef1_axis),
                          'limit': tmef1_limit})

        return rules

    def generate_flags(self, matrix, rules):
        """
        Calculate the statistics of a matrix and flag the data according
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

        # If there is no valid (non-flagged) data, then return early.
        if np.all(flag):
            return newflags, flag_reason

        # If requested to use antenna names instead of IDs antenna,
        # create an id-to-name translation and check to make sure this
        # would result in unique non-empty names for all IDs, otherwise
        # revert back to flagging by ID
        if self.inputs.use_antenna_names:
            
            # create translation dictionary, reject empty antenna name strings
            antenna_id_to_name = {ant.id: ant.name
                                  for ant in self.inputs.ms.antennas
                                  if ant.name.strip()}

            # Check that each antenna ID is represented by a unique non-empty
            # name, by testing that the unique set of antenna names is same
            # length as list of IDs. If not, then unset the translation
            # dictionary to revert back to flagging by ID
            if len(set(antenna_id_to_name.values())) != len(
                    self.inputs.ms.antennas):
                LOG.info('No unique name available for each antenna ID:'
                         ' flagging by antenna ID instead of by name.')
                antenna_id_to_name = None
        else:
            antenna_id_to_name = None
            
        # If requested, expand current spw to all spws within the same
        # baseband, thus changing spw from an integer to a list of integers
        if self.inputs.extendbaseband:
            ms = self.inputs.context.observing_run.get_ms(self.inputs.vis)
            baseband = ms.get_spectral_window(spw).baseband
            spw = [spw.id for spw in ms.get_spectral_windows()
                   if spw.baseband == baseband]

        # Index arrays
        i, j = np.indices(np.shape(data))

        rdata = np.ravel(data)
        rflag = np.ravel(flag)
        valid_data = rdata[np.logical_not(rflag)]

        # calculate statistics for valid data
        data_median, data_mad = arrayflaggerbase.median_and_mad(valid_data)

        # flag data according to each rule in turn
        for rule in rules:
            rulename = rule['name']

            if rulename == 'outlier':

                # Stop evaluating rule if sample is too small.
                minsample = rule['minsample']
                if len(valid_data) < minsample:
                    continue

                # Check limits.
                mad_max = rule['limit']

                # Create masked array with flagged data masked.
                data_masked = np.ma.array(np.abs(data - data_median), mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                data_masked = np.ma.masked_greater(data_masked, mad_max * data_mad)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))

                # No flagged data.
                if not np.any(ind2flag):
                    continue

                i2flag = i[ind2flag]
                j2flag = j[ind2flag]

                # Add new flag command to flag data underlying the
                # view.
                flagcoords = zip(xdata[i2flag], ydata[j2flag])
                for flagcoord in flagcoords:
                    newflags.append(arrayflaggerbase.FlagCmd(
                        reason='outlier', filename=table, rulename=rulename, spw=spw, antenna=antenna,
                        axisnames=[xtitle, ytitle], flagcoords=flagcoord, pol=pol,
                        antenna_id_to_name=antenna_id_to_name))

                # Flag the view, for any subsequent rules being evaluated.
                flag[i2flag, j2flag] = True
                flag_reason[i2flag, j2flag] = self.flag_reason_index[rulename]

            elif rulename == 'low outlier':

                # Stop evaluating rule if sample is too small.
                minsample = rule['minsample']
                if len(valid_data) < minsample:
                    continue

                # Check limits.
                mad_max = rule['limit']

                # Create masked array with flagged data masked.
                data_masked = np.ma.array(data_median - data, mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                data_masked = np.ma.masked_greater(data_masked, mad_max * data_mad)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))

                # No flagged data.
                if not np.any(ind2flag):
                    continue

                i2flag = i[ind2flag]
                j2flag = j[ind2flag]

                # Add new flag commands to flag data underlying the
                # view.
                flagcoords = zip(xdata[i2flag], ydata[j2flag])
                for flagcoord in flagcoords:
                    newflags.append(arrayflaggerbase.FlagCmd(
                        reason='low_outlier', filename=table, rulename=rulename, spw=spw, axisnames=[xtitle, ytitle],
                        flagcoords=flagcoord, pol=pol, antenna_id_to_name=antenna_id_to_name))

                # Flag the view, for any subsequent rules being evaluated.
                flag[i2flag, j2flag] = True
                flag_reason[i2flag, j2flag] = self.flag_reason_index[rulename]

            elif rulename == 'high outlier':

                # Stop evaluating rule if sample is too small.
                minsample = rule['minsample']
                if len(valid_data) < minsample:
                    continue

                # Check limits.
                mad_max = rule['limit']

                # Create masked array with flagged data masked.
                data_masked = np.ma.array(data - data_median, mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                data_masked = np.ma.masked_greater(data_masked, mad_max * data_mad)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))
                # No flags
                if not np.any(ind2flag):
                    continue

                i2flag = i[ind2flag]
                j2flag = j[ind2flag]

                # Add new flag commands to flag data underlying the
                # view.
                flagcoords = zip(xdata[i2flag], ydata[j2flag])
                for flagcoord in flagcoords:
                    newflags.append(arrayflaggerbase.FlagCmd(
                      reason='high_outlier', filename=table, rulename=rulename, spw=spw, axisnames=[xtitle, ytitle],
                      flagcoords=flagcoord, pol=pol, antenna_id_to_name=antenna_id_to_name))

                # Flag the view, for any subsequent rules being evaluated.
                flag[i2flag, j2flag] = True
                flag_reason[i2flag, j2flag] = self.flag_reason_index[rulename]

            elif rulename == 'min abs':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Check limits.
                limit = rule['limit']

                # Create masked array with flagged data masked.
                data_masked = np.ma.array(np.abs(data), mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                data_masked = np.ma.masked_less(data_masked, limit)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))

                # No flags
                if not np.any(ind2flag):
                    continue

                i2flag = i[ind2flag]
                j2flag = j[ind2flag]

                # Add new flag commands to flag data underlying the
                # view.
                flagcoords = zip(xdata[i2flag], ydata[j2flag])
                for flagcoord in flagcoords:
                    newflags.append(arrayflaggerbase.FlagCmd(
                        reason='min_abs', filename=table, rulename=rulename, spw=spw, axisnames=[xtitle, ytitle],
                        flagcoords=flagcoord, pol=pol, antenna_id_to_name=antenna_id_to_name))

                # Flag the view, for any subsequent rules being evaluated.
                flag[i2flag, j2flag] = True
                flag_reason[i2flag, j2flag] = self.flag_reason_index[rulename]

            elif rulename == 'max abs':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Check limits.
                limit = rule['limit']

                # Create masked array with flagged data masked.
                data_masked = np.ma.array(np.abs(data), mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                data_masked = np.ma.masked_greater(data_masked, limit)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))

                # No flags
                if not np.any(ind2flag):
                    continue

                i2flag = i[ind2flag]
                j2flag = j[ind2flag]

                # Add new flag commands to flag data underlying the
                # view.
                flagcoords = zip(xdata[i2flag], ydata[j2flag])
                for flagcoord in flagcoords:
                    newflags.append(arrayflaggerbase.FlagCmd(
                        reason='max_abs', filename=table, rulename=rulename,  spw=spw, axisnames=[xtitle, ytitle],
                        flagcoords=flagcoord, pol=pol, extendfields=self.inputs.extendfields,
                        antenna_id_to_name=antenna_id_to_name))

                # Flag the view, for any subsequent rules being evaluated.
                flag[i2flag, j2flag] = True
                flag_reason[i2flag, j2flag] = self.flag_reason_index[rulename]

            elif rulename == 'too many flags':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                maxfraction = rule['limit']
                maxexcessflags = rule['excess limit']
                axis = rule['axis']
                axis = axis.upper().strip()

                if axis == xtitle.upper().strip():

                    # Compute median number flagged
                    num_flagged = np.zeros([np.shape(data)[1]], np.int)
                    for iy in np.arange(len(ydata)):
                        num_flagged[iy] = len(data[:, iy][flag[:, iy]])
                    median_num_flagged = np.median(num_flagged)

                    # look along x axis
                    for iy in np.arange(len(ydata)):
                        if all(flag[:, iy]):
                            continue

                        # Compute fraction flagged
                        len_data = len(xdata)
                        len_no_data = len(data[:, iy][nodata[:, iy]])
                        len_flagged = len(data[:, iy][flag[:, iy]])
                        fractionflagged = (
                          float(len_flagged - len_no_data) /
                          float(len_data - len_no_data))
                        if fractionflagged > maxfraction:
                            i2flag = i[:, iy][np.logical_not(flag[:, iy])]
                            j2flag = j[:, iy][np.logical_not(flag[:, iy])]
                        else:
                            i2flag = np.zeros([0], np.int)
                            j2flag = np.zeros([0], np.int)

                        # likewise for maxexcessflags
                        if len_flagged > median_num_flagged + maxexcessflags:
                            i2flag = np.concatenate((i2flag, i[:, iy][np.logical_not(flag[:, iy])]))
                            j2flag = np.concatenate((j2flag, j[:, iy][np.logical_not(flag[:, iy])]))

                        # Add new flag commands to flag data underlying
                        # the view.
                        flagcoords = zip(xdata[i2flag], ydata[j2flag])
                        for flagcoord in flagcoords:
                            newflags.append(arrayflaggerbase.FlagCmd(
                                reason='too_many_flags', filename=table, rulename=rulename, spw=spw, antenna=antenna,
                                axisnames=[xtitle, ytitle], flagcoords=flagcoord, pol=pol,
                                antenna_id_to_name=antenna_id_to_name))

                        # Flag the view, for any subsequent rules being evaluated.
                        flag[i2flag, j2flag] = True
                        flag_reason[i2flag, j2flag] =\
                            self.flag_reason_index[rulename]

                elif axis == ytitle.upper().strip():

                    # Compute median number flagged
                    num_flagged = np.zeros([np.shape(data)[0]], np.int)
                    for ix in np.arange(len(xdata)):
                        num_flagged[ix] = len(data[ix, :][flag[ix, :]])
                    median_num_flagged = np.median(num_flagged)

                    # look along y axis
                    for ix in np.arange(len(xdata)):
                        if all(flag[ix, :]):
                            continue

                        len_data = len(ydata)
                        len_no_data = len(data[ix, :][nodata[ix, :]])
                        len_flagged = len(data[ix, :][flag[ix, :]])
                        fractionflagged = (
                            float(len_flagged - len_no_data) /
                            float(len_data - len_no_data))
                        if fractionflagged > maxfraction:
                            i2flag = i[ix, :][np.logical_not(flag[ix, :])]
                            j2flag = j[ix, :][np.logical_not(flag[ix, :])]
                        else:
                            i2flag = np.zeros([0], np.int)
                            j2flag = np.zeros([0], np.int)

                        len_flagged = len(data[ix, :][flag[ix, :]])
                        if len_flagged > median_num_flagged + maxexcessflags:
                            i2flag = np.concatenate((i2flag, i[ix, :][np.logical_not(flag[ix, :])]))
                            j2flag = np.concatenate((j2flag, j[ix, :][np.logical_not(flag[ix, :])]))

                        # Add new flag commands to flag data underlying
                        # the view.
                        flagcoords = zip(xdata[i2flag], ydata[j2flag])
                        for flagcoord in flagcoords:
                            newflags.append(arrayflaggerbase.FlagCmd(
                                reason='too_many_flags', filename=table, rulename=rulename, spw=spw,
                                axisnames=[xtitle, ytitle], flagcoords=flagcoord, pol=pol,
                                antenna_id_to_name=antenna_id_to_name))

                        # Flag the view, for any subsequent rules being evaluated.
                        flag[i2flag, j2flag] = True
                        flag_reason[i2flag, j2flag] = self.flag_reason_index[rulename]

            elif rulename == 'too many entirely flagged':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

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
                                reason='too_many_flags', filename=table, rulename=rulename,  spw=spw,
                                antenna=antenna, axisnames=[xtitle, ytitle], flagcoords=flagcoord, pol=pol,
                                antenna_id_to_name=antenna_id_to_name))

                        # Flag the view, for any subsequent rules being evaluated.
                        flag[i2flag, j2flag] = True
                        flag_reason[i2flag, j2flag] = self.flag_reason_index[rulename]

            elif rulename == 'nmedian':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Check limits.
                lo_limit = rule['lo_limit']
                hi_limit = rule['hi_limit']

                # Create masked array with flagged data masked.
                data_masked = np.ma.array(data, mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                data_masked = np.ma.masked_greater(data_masked, hi_limit * data_median)
                data_masked = np.ma.masked_less(data_masked, lo_limit * data_median)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))

                # No flags
                if not np.any(ind2flag):
                    continue

                i2flag = i[ind2flag]
                j2flag = j[ind2flag]

                # Add new flag commands to flag the data underlying
                # the view.
                flagcoords = zip(xdata[i2flag], ydata[j2flag])
                for flagcoord in flagcoords:
                    newflags.append(arrayflaggerbase.FlagCmd(
                        reason='nmedian', filename=table, rulename=rulename, spw=spw, axisnames=[xtitle, ytitle],
                        flagcoords=flagcoord, pol=pol, extendfields=self.inputs.extendfields,
                        antenna_id_to_name=antenna_id_to_name))

                # Flag the view, for any subsequent rules being evaluated.
                flag[i2flag, j2flag] = True
                flag_reason[i2flag, j2flag] = self.flag_reason_index[rulename]

            elif rulename == 'bad antenna':
                # this test should be run before the others as it depends on no other
                # flags having been set by other rules before it
                # (because the number of unflagged points on entry are part of the test)

                # Stop evaluating rule if all data is flagged or if the x-axis is not for antenna.
                if np.all(flag) or 'ANTENNA' not in xtitle.upper():
                    continue

                # Check limits.
                mad_max = rule['lo_limit']
                frac_limit = rule['frac_limit']
                number_limit = rule['number_limit']
                minsample = rule['minsample']

                # For every antenna on the x-axis...
                for iant in range(np.shape(flag)[0]):
                    # For current antenna, create references to the
                    # corresponding column in data, flag, and flag_reason.
                    ant_data = data[iant, :]
                    ant_flag = flag[iant, :]
                    ant_flag_reason = flag_reason[iant, :]

                    # Identify valid (non-flagged) data.
                    valid_ant_data = ant_data[np.logical_not(ant_flag)]

                    # If the sample of unflagged datapoints is smaller than
                    # the minimum threshold, skip this antenna.
                    if len(valid_ant_data) < minsample:
                        continue

                    # Create masked array with flagged data masked.
                    ant_data_masked = np.ma.array(data_median - ant_data, mask=ant_flag)

                    # Create new masked array from masked array with outliers
                    # masked. This should avoid performing a comparison with
                    # flagged data that could include NaNs (that would cause a
                    # RuntimeWarning).
                    ant_data_masked = np.ma.masked_greater(ant_data_masked, mad_max * data_mad)

                    # Get indices to flag as the masked elements that were not
                    # already flagged, i.e. the newly masked elements.
                    ind2flag = np.logical_and(np.ma.getmask(ant_data_masked),
                                              np.logical_not(ant_flag))

                    # If no low outliers were found, skip this antenna.
                    if len(ind2flag) <= 0:
                        continue

                    j2flag_lo = j[iant, :][ind2flag]

                    # Determine number of points found to be low outliers that
                    # were not previously flagged.
                    nflags = len(j2flag_lo)

                    # Determine fraction of newly found low outliers over
                    # total number of data points in current antenna data
                    # selection.
                    flagsfrac = float(nflags) / float(np.shape(ant_flag)[0])

                    # If the number of newly found low outliers equals-or-exceeds
                    # a minimum threshold number, and the fraction of newly
                    # found flags exceeds a minimum threshold fraction, then
                    # proceed with actually generating flagging commands.
                    if nflags >= number_limit or flagsfrac > frac_limit:

                        # If we get here, then a sufficient number and
                        # fraction of low outliers were identified for
                        # the current antenna, such that the antenna is
                        # considered "bad" and should be flagged entirely.

                        # In this case, the low outlier data points are
                        # explicitly flagged as "low outlier", while the
                        # remaining non-flagged data points for this antenna
                        # are flagged as "bad antenna".

                        # For current antenna data selection, flag the points
                        # that were identified to be low outliers and not
                        # already flagged, and set corresponding flag reason.
                        ant_flag[j2flag_lo] = True
                        ant_flag_reason[j2flag_lo] =\
                            self.flag_reason_index['low outlier']

                        # Create a flagging command that flags these
                        # low outliers in the data.
                        flagcoords = zip(xdata[[iant]], ydata[j2flag_lo])
                        for flagcoord in flagcoords:
                            newflags.append(arrayflaggerbase.FlagCmd(
                                reason='low outlier', filename=table, rulename='low outlier', spw=spw, pol=pol,
                                axisnames=[xtitle, ytitle], flagcoords=flagcoord,
                                antenna_id_to_name=antenna_id_to_name))

                        # For current antenna data selection, identify the
                        # remaining non-flagged data points.
                        j2flag_bad = j[iant, :][np.logical_not(ant_flag)]

                        # Flag the remaining non-flagged data points as
                        # "bad antenna"; these are references to original view
                        # which is thus updated for any subsequent rules being evaluated.
                        ant_flag[j2flag_bad] = True
                        ant_flag_reason[j2flag_bad] = self.flag_reason_index['bad antenna']

                        # Create a flagging command that flags the remaining
                        # data points as "bad antenna".
                        flagcoords = zip(xdata[[iant]], ydata[j2flag_bad])
                        for flagcoord in flagcoords:
                            newflags.append(arrayflaggerbase.FlagCmd(
                                reason='bad antenna', filename=table, rulename='bad antenna', spw=spw, pol=pol,
                                axisnames=[xtitle, ytitle], flagcoords=flagcoord,
                                antenna_id_to_name=antenna_id_to_name))

            elif rulename == 'bad quadrant':
                # this test should be run before the others as it depends on no other
                # flags having been set by other rules before it
                # (because the number of unflagged points on entry are part of the test)

                # a quadrant is one quarter of the extent of the x-axis

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Check limits.
                hilo_limit = rule['hilo_limit']
                frac_limit = rule['frac_limit']
                baseline_frac_limit = rule['baseline_frac_limit']

                # find outlier flags first
                # Create masked array with flagged data masked.
                data_masked = np.ma.array(np.abs(data - data_median), mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                data_masked = np.ma.masked_greater(data_masked, hilo_limit * data_mad)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))

                # No flagged data.
                if len(ind2flag) <= 0:
                    continue

                i2flag = i[ind2flag]
                j2flag = j[ind2flag]

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
                flag_reason_copy[i2flag, j2flag] = self.flag_reason_index['outlier']

                # look for bad antenna/quadrants in view copy
                data_shape = np.shape(data)
                nchan = data_shape[0]
                nbaseline = data_shape[1]
                nant = int(math.sqrt(nbaseline))

                quadrant = [
                    [0, nchan/4],
                    [nchan/4, nchan/2],
                    [nchan/2, nchan*3/4],
                    [nchan*3/4, nchan],
                ]

                for ant in range(nant):
                    # flag based on outliers in flag_copy, will set new flags
                    # in a further copy so that outlier flags are not corrupted
                    working_copy_flag = np.copy(flag_copy)
                    working_copy_flag_reason = np.copy(flag_reason_copy)

                    # baselines involving this antenna
                    baselines = [baseline
                                 for baseline in range(nbaseline)
                                 if (ant*nant <= baseline < (ant+1)*nant)
                                 or (baseline % nant == ant)]
                    baselines = np.array(baselines)

                    for iquad in range(4):
                        quad_slice = slice(quadrant[iquad][0], quadrant[iquad][1])
                        ninvalid = np.count_nonzero(
                            working_copy_flag[quad_slice, baselines])
                        ninvalid_on_entry = np.count_nonzero(
                            unflagged_flag_copy[quad_slice, baselines])
                        nvalid_on_entry = np.count_nonzero(np.logical_not(
                            unflagged_flag_copy[quad_slice, baselines]))
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
                            for chan in range(
                                    quadrant[iquad][0], quadrant[iquad][1]):
                                flagcoords.append((chan, ant))
                            for flagcoord in flagcoords:
                                newflags.append(arrayflaggerbase.FlagCmd(
                                    reason='bad quadrant', filename=table, rulename=rulename,  spw=spw,
                                    axisnames=[xtitle, ytitle], flagcoords=flagcoord, pol=pol,
                                    extendfields=self.inputs.extendfields,
                                    antenna_id_to_name=antenna_id_to_name))

                            # update working copy view with 'bad quadrant' flags
                            i2flag = i[quad_slice, baselines][
                                np.logical_not(working_copy_flag[quad_slice, baselines])]
                            j2flag = j[quad_slice, baselines][
                                np.logical_not(working_copy_flag[quad_slice, baselines])]

                            if len(i2flag) > 0:
                                working_copy_flag[i2flag, j2flag] = True
                                working_copy_flag_reason[i2flag, j2flag] =\
                                    self.flag_reason_index['bad quadrant']

                            # copy flag state for this antenna
                            # back to original
                            flag[quad_slice, baselines] =\
                                working_copy_flag[quad_slice, baselines]
                            flag_reason[quad_slice, baselines] =\
                                working_copy_flag_reason[quad_slice, baselines]

                            # whole antenna/quadrant flagged, no need to check
                            # individual baselines
                            continue

                        # look for bad quadrant/baseline
                        for baseline in baselines:
                            ninvalid = np.count_nonzero(
                                working_copy_flag[quad_slice, baseline])
                            ninvalid_on_entry = np.count_nonzero(
                                unflagged_flag_copy[quad_slice, baseline])
                            nvalid_on_entry = np.count_nonzero(
                                np.logical_not(unflagged_flag_copy[quad_slice, baseline]))
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
                                for chan in range(
                                        quadrant[iquad][0], quadrant[iquad][1]):
                                    flagcoords.append((chan, ydata[baseline]))

                                for flagcoord in flagcoords:
                                    newflags.append(arrayflaggerbase.FlagCmd(
                                        reason='bad quadrant', filename=table, rulename=rulename, spw=spw,
                                        axisnames=[xtitle, ytitle], flagcoords=flagcoord, pol=pol,
                                        extendfields=self.inputs.extendfields, antenna_id_to_name=antenna_id_to_name))

                                # update working copy view with 'bad quadrant' flags
                                i2flag = i[quad_slice, baseline][
                                    np.logical_not(working_copy_flag[quad_slice, baseline])]
                                j2flag = j[quad_slice, baseline][
                                    np.logical_not(working_copy_flag[quad_slice, baseline])]

                                if len(i2flag) > 0:
                                    working_copy_flag[i2flag, j2flag] = True
                                    working_copy_flag_reason[i2flag, j2flag] =\
                                        self.flag_reason_index['bad quadrant']

                                # copy flag state for this antenna back to original,
                                # for any subsequent rules being evaluated.
                                flag[quad_slice, baseline] = \
                                    working_copy_flag[quad_slice, baseline]
                                flag_reason[quad_slice, baseline] = \
                                    working_copy_flag_reason[quad_slice, baseline]

            else:
                raise NameError('bad rule: %s' % rule)

        # consolidate flagcmds that specify individual channels into fewer
        # flagcmds that specify ranges
        newflags = arrayflaggerbase.consolidate_flagcmd_channels(
            newflags, antenna_id_to_name=antenna_id_to_name)

        return newflags, flag_reason


class VectorFlaggerInputs(vdp.StandardInputs):
    prepend = vdp.VisDependentProperty(default='')
    use_antenna_names = vdp.VisDependentProperty(default=True)

    def __init__(self, context, output_dir=None, vis=None, datatask=None, viewtask=None, flagsettertask=None,
                 rules=None, niter=None, iter_datatask=None, use_antenna_names=None, prepend=None):
        super(VectorFlaggerInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # solution parameters
        self.datatask = datatask
        self.flagsettertask = flagsettertask
        self.iter_datatask = iter_datatask
        self.niter = niter
        self.prepend = prepend
        self.rules = rules
        self.use_antenna_names = use_antenna_names
        self.viewtask = viewtask


class VectorFlaggerResults(basetask.Results,
                           flaggableviewresults.FlaggableViewResults):
    def __init__(self, vis=None):
        """
        Construct and return a new VectorFlaggerResults.
        """
        basetask.Results.__init__(self)
        flaggableviewresults.FlaggableViewResults.__init__(self)

        self.vis = vis
        self.dataresult = None
        self.viewresult = None

    def merge_with_context(self, context):
        pass

    def __repr__(self):
        s = 'VectorFlaggerResults'
        return s


class VectorFlagger(basetask.StandardTaskTemplate):
    Inputs = VectorFlaggerInputs

    # override the inherited __init__ method so that references to the
    # task objects can be kept outside self.inputs. Later on self.inputs
    # will be replaced by a copy which breaks the connection between
    # its references to the tasks and the originals.
    def __init__(self, inputs):
        self.inputs = inputs

    def prepare(self):
        inputs = self.inputs

        # Initialize result.
        result = VectorFlaggerResults()

        # Initialize flags and iteration counter
        flags = []
        newflags = []
        counter = 1
        include_before = True
        dataresult = None
        viewresult = None

        # Start iterative flagging
        while counter <= inputs.niter:

            # Run the data task if needed
            if counter == 1:
                # Always run data task on first iteration
                dataresult = self._executor.execute(self.inputs.datatask)
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
                    
                dataresult = self._executor.execute(inputs.datatask)
            else:
                # If not iterating the datatask, the previous 
                # data result will be re-used, but marked here as no 
                # longer new.
                dataresult.new = False

            # Create flagging view                
            viewresult = inputs.viewtask(dataresult)

            # If a view could be created, continue with flagging
            if viewresult.descriptions():
            
                # Import the views from viewtask into the final result
                result.importfrom(viewresult)
    
                # Flag the view
                newflags = self.flag_view(viewresult)
            
                # Report how many flags were found in this iteration and
                # stop iteration if no new flags were found
                if len(newflags) == 0:
                    # If no new flags are found, report as a log message
                    LOG.info(
                        '{0}{1} iteration {2} raised {3} flagging'
                        ' commands'.format(inputs.prepend,
                                           os.path.basename(inputs.vis),
                                           counter, len(newflags)))
                    break
                else:
                    # Report newly found flags as a warning message
                    LOG.warning(
                        '{0}{1} iteration {2} raised {3} flagging'
                        ' commands'.format(inputs.prepend,
                                           os.path.basename(inputs.vis),
                                           counter, len(newflags)))

                # Accumulate new flags
                flags += newflags
                
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
                
                # If datatask needs to be iterated...
                if inputs.iter_datatask is True:

                    # First set the new flags that were found on the last
                    # iteration. If the "before" summary was not yet created,
                    # then include this here; always include the "after"
                    # summary.
                    if include_before:
                        # Set flags, and include "before" and "after" summary.
                        stats_before, stats_after = self.set_flags(
                            newflags, summarize_before=True,
                            summarize_after=True)
                    else:
                        # Set flags, and include "after" summary
                        _, stats_after = self.set_flags(
                            newflags, summarize_after=True)

                    # After setting the latest flags, re-run the data task.
                    dataresult = self._executor.execute(inputs.datatask)

                # If the datatask did not need to be iterated, then no flags
                # were set yet and no "before" summary was performed yet, so
                # set all flags and include both "before" and "after" summary.
                else:
                    stats_before, stats_after = self.set_flags(
                        flags, summarize_before=True, summarize_after=True)
                
                # Create final post-flagging view                
                viewresult = inputs.viewtask(dataresult)

                # Import the post-flagging view into the final result
                result.importfrom(viewresult)
                
            # If flags were found, but no newflags were found on last iteration
            # then the dataresult is already up-to-date, and all that is needed
            # is to ensure the flags are set, and that summaries are created.
            else:
                
                # If datatask needs to be iterated, then the "before" summary has
                # already been done, and the flags have already been set, so only
                # need to do an "after" summary.
                if inputs.iter_datatask is True:
                    _, stats_after = self.set_flags([], summarize_after=True)
                # If the datatask did not need to be iterated, then no flags
                # were set yet and no "before" summary was performed yet, 
                # so set all flags and include both "before" and "after" summary.
                else:
                    stats_before, stats_after = self.set_flags(
                        flags, summarize_before=True, summarize_after=True)
            
            # Store the final set of flags in the final result
            result.addflags(flags)
            
        # if no flags were found at all
        else:
            # Run a single flagging summary and use the result as both the "before" 
            # and "after" summary.
            stats_before, _ = self.set_flags(flags, summarize_before=True)
            stats_after = copy.deepcopy(stats_before)
        
        # Store in the final result the name of the measurement set or caltable
        # to which any potentially found flags would need to be applied to.
        result.table = inputs.flagsettertask.inputs.table

        # Store in the final result the final data task result and the final
        # view task result.
        result.dataresult = dataresult
        result.viewresult = viewresult

        # Store the flagging summaries in the final result
        result.summaries = [stats_before, stats_after]

        # Sort the final list of flagging commands.
        result.sort_flagcmds()

        return result

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

        # Initialize "before" and/or "after" summaries. If "real" flagsetter
        # results are returned (e.g. by WvrgcalFlagSetter), then there will
        # have been no real flagging summaries created, in which case empty
        # dictionaries will be returned as empty flagging summaries.
        stats_before = {}
        stats_after = {}

        # If the flagsetter returned results from the CASA flag data task,
        # then proceed to extract "before" and/or "after" flagging summaries.
        if flagsetterresult.results:
            if all(['report' in k for k in flagsetterresult.results[0].keys()]):
                # Go through dictionary of reports...
                for report in flagsetterresult.results[0].keys():
                    if flagsetterresult.results[0][report]['name'] == 'before':
                        stats_before = flagsetterresult.results[0][report]
                    if flagsetterresult.results[0][report]['name'] == 'after':
                        stats_after = flagsetterresult.results[0][report]
            else:
                # Go through single report.
                if flagsetterresult.results[0]['name'] == 'before':
                    stats_before = flagsetterresult.results[0]
                if flagsetterresult.results[0]['name'] == 'after':
                    stats_after = flagsetterresult.results[0]

        return stats_before, stats_after

    @staticmethod
    def make_flag_rules(
            flag_edges=False, edge_limit=2.0,
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
            rules.append({'name': 'edges', 'limit': edge_limit})
        if flag_minabs:
            rules.append({'name': 'min abs', 'limit': fmin_limit})
        if flag_nmedian:
            rules.append({'name': 'nmedian', 'lo_limit': fnm_lo_limit,
                          'hi_limit': fnm_hi_limit})
        if flag_hilo:
            rules.append({'name': 'outlier', 'limit': fhl_limit,
                          'minsample': fhl_minsample})
        if flag_sharps:
            rules.append({'name': 'sharps', 'limit': sharps_limit})
        if flag_diffmad:
            rules.append({'name': 'diffmad', 'limit': diffmad_limit,
                          'nchan_limit': diffmad_nchan_limit})
        if flag_tmf:
            rules.append({'name': 'tmf', 'frac_limit': tmf_frac_limit,
                          'nchan_limit': tmf_nchan_limit})

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

        # Initialize flags
        newflags = []

        # If there is no valid (non-flagged) data, then return early.
        if np.all(flag):
            return newflags

        # If requested to use antenna names instead of IDs antenna,
        # create an id-to-name translation and check to make sure this
        # would result in unique non-empty names for all IDs, otherwise
        # revert back to flagging by ID
        if self.inputs.use_antenna_names:
            
            # create translation dictionary, reject empty antenna name strings
            antenna_id_to_name = {ant.id: ant.name
                                  for ant in self.inputs.ms.antennas
                                  if ant.name.strip()}

            # Check that each antenna ID is represented by a unique non-empty
            # name, by testing that the unique set of antenna names is same 
            # length as list of IDs. If not, then unset the translation 
            # dictionary to revert back to flagging by ID.
            if len(set(antenna_id_to_name.values())) != len(
                    self.inputs.ms.antennas):
                LOG.info('No unique name available for each antenna ID: '
                         'flagging by antenna ID instead of by name.')
                antenna_id_to_name = None
        else:
            antenna_id_to_name = None

        # any flags found will apply to this subset of the data
        axisnames = []
        flagcoords = []
        if antenna is not None:
            axisnames.append('ANTENNA1')
            flagcoords.append(antenna)
        axisnames = ['channels']

        # Create flattened 1D views of data and flags, with corresponding
        # channels array. TODO: this should be unnecessary, as VectorFlagger
        # expects 1D arrays as input.
        rdata = np.ravel(data)
        rflag = np.ravel(flag)
        valid_data = rdata[np.logical_not(rflag)]

        nchannels = len(rdata)
        channels = np.arange(nchannels)

        # calculate statistics for valid data
        data_median, data_mad = arrayflaggerbase.median_and_mad(valid_data)

        # flag data according to each rule in turn
        for rule in self.inputs.rules:

            rulename = rule['name']

            if rulename == 'edges':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Get limits.
                limit = rule['limit']

                # find left edge
                left_edge = VectorFlagger._find_small_diff(
                    rdata, rflag, limit, vector.description)

                # and right edge
                reverse_data = rdata[-1::-1]
                reverse_flag = rflag[-1::-1]
                right_edge = VectorFlagger._find_small_diff(
                    reverse_data, reverse_flag, limit, vector.description)

                # flag the 'view', for any subsequent rules being evaluated.
                rflag[:left_edge] = True
                if right_edge > 0:
                    rflag[-right_edge:] = True

                # now compose a description of the flagging required on
                # the MS
                channels_flagged = channels[np.logical_or(
                    channels < left_edge,
                    channels > (nchannels-1-right_edge))]
                flagcoords = [list(channels_flagged)]

                if len(channels_flagged) > 0:
                    # Add new flag command to flag data underlying the
                    # view.
                    newflags.append(arrayflaggerbase.FlagCmd(
                        reason='edges', filename=table, rulename=rulename, spw=spw, axisnames=axisnames,
                        flagcoords=flagcoords, antenna_id_to_name=antenna_id_to_name))

            elif rulename == 'min abs':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Get limits.
                limit = rule['limit']

                # Create masked array with flagged data masked.
                data_masked = np.ma.array(np.abs(data), mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                data_masked = np.ma.masked_less(data_masked, limit)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))

                # No flags
                if not np.any(ind2flag):
                    continue

                # flag the 'view', for any subsequent rules being evaluated.
                rflag[ind2flag] = True

                # now compose a description of the flagging required on
                # the MS
                channels_flagged = channels[ind2flag]
                flagcoords = [list(channels_flagged)]

                # Add new flag command to flag data underlying the
                # view.
                newflags.append(arrayflaggerbase.FlagCmd(
                    reason='min_abs', filename=table, rulename=rulename, spw=spw, axisnames=axisnames,
                    flagcoords=flagcoords, antenna_id_to_name=antenna_id_to_name))

            elif rulename == 'nmedian':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Get limits.
                lo_limit = rule['lo_limit']
                hi_limit = rule['hi_limit']

                # Create masked array with flagged data masked.
                data_masked = np.ma.array(data, mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                outlier_high_threshold = hi_limit * data_median
                outlier_low_threshold = lo_limit * data_median
                data_masked = np.ma.masked_greater(data_masked, outlier_high_threshold)
                data_masked = np.ma.masked_less(data_masked, outlier_low_threshold)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))

                # No flags
                if not np.any(ind2flag):
                    continue

                # flag the 'view', for any subsequent rules being evaluated.
                rflag[ind2flag] = True

                # now compose a description of the flagging required on
                # the MS
                channels_flagged = channels[ind2flag]
                flagcoords = [list(channels_flagged)]

                # Add new flag command to flag data underlying the
                # view.
                newflags.append(arrayflaggerbase.FlagCmd(
                    reason='nmedian', filename=table, rulename=rulename, spw=spw, axisnames=axisnames,
                    flagcoords=flagcoords, antenna_id_to_name=antenna_id_to_name))

            elif rulename == 'outlier':

                minsample = rule['minsample']

                # Stop evaluating rule if sample is too small.
                if len(valid_data) < minsample:
                    continue

                # Get limits.
                limit = rule['limit']

                # Create masked array with flagged data masked.
                data_masked = np.ma.array(np.abs(data - data_median), mask=flag)

                # Create new masked array from masked array with outliers
                # masked. This should avoid performing a comparison with
                # flagged data that could include NaNs (that would cause a
                # RuntimeWarning).
                outlier_threshold = limit * data_mad
                data_masked = np.ma.masked_greater(data_masked, outlier_threshold)

                # Get indices to flag as the masked elements that were not
                # already flagged, i.e. the newly masked elements.
                ind2flag = np.logical_and(np.ma.getmask(data_masked),
                                          np.logical_not(flag))

                # No flags
                if not np.any(ind2flag):
                    continue

                # flag the 'view', for any subsequent rules being evaluated.
                rflag[ind2flag] = True

                # now compose a description of the flagging required on
                # the MS
                channels_flagged = channels[ind2flag]
                flagcoords = [list(channels_flagged)]

                # Add new flag command to flag data underlying the
                # view.
                newflags.append(arrayflaggerbase.FlagCmd(
                    reason='outlier', filename=table, rulename=rulename, spw=spw, pol=pol, antenna=antenna,
                    axisnames=axisnames, flagcoords=flagcoords, antenna_id_to_name=antenna_id_to_name))

            elif rulename == 'sharps':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Get limits.
                limit = rule['limit']

                # Compute channel-to-channel difference, and corresponding flag array.
                diff = abs(rdata[1:] - rdata[:-1])
                diff_flag = (rflag[1:] | rflag[:-1])

                # flag channels whose slope is greater than the
                # limit for a 'sharp feature'
                newflag = (diff > limit) & np.logical_not(diff_flag)

                # now broaden the flags until the diff falls below
                # 2 times the median diff, to catch the wings of
                # sharp features
                if np.any([np.logical_not(diff_flag | newflag)]):
                    median_diff = np.median(
                        diff[np.logical_not(diff_flag | newflag)])
                    median_flag = ((diff > 2 * median_diff)
                                   & np.logical_not(diff_flag))
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

                # flag the 'view', for any subsequent rules being evaluated.
                rflag[flag_chan] = True

                # now compose a description of the flagging required on
                # the MS
                channels_flagged = channels[flag_chan]
                flagcoords = [list(channels_flagged)]

                if len(channels_flagged) > 0:
                    # Add new flag command to flag data underlying the
                    # view.
                    newflags.append(arrayflaggerbase.FlagCmd(
                      reason='sharps', filename=table, rulename=rulename, spw=spw, antenna=antenna, axisnames=axisnames,
                      flagcoords=flagcoords, antenna_id_to_name=antenna_id_to_name))

            elif rulename == 'diffmad':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Get limits.
                limit = rule['limit']
                nchan_limit = rule['nchan_limit']

                # Compute channel-to-channel difference (and associated median
                # and MAD), and corresponding flag array.
                diff = rdata[1:] - rdata[:-1]
                diff_flag = np.logical_or(rflag[1:], rflag[:-1])
                median_diff = np.median(diff[diff_flag == 0])
                mad = np.median(np.abs(diff[diff_flag == 0] - median_diff))

                # first, flag channels further from the median than
                # limit * MAD
                newflag = (
                    (abs(diff-median_diff) > limit*mad)
                    & (diff_flag == 0))

                # second, flag all channels if more than nchan_limit
                # were flagged by the first stage
                if np.count_nonzero(newflag) >= nchan_limit:
                    newflag = np.ones(diff.shape, np.bool)

                # set channels flagged
                flag_chan = np.zeros([len(newflag)+1], np.bool)
                flag_chan[:-1] = newflag
                flag_chan[1:] = np.logical_or(flag_chan[1:], newflag)

                # flag the 'view', for any subsequent rules being evaluated.
                rflag[flag_chan] = True

                # now compose a description of the flagging required on
                # the MS
                channels_flagged = channels[flag_chan]
                flagcoords = [list(channels_flagged)]

                if len(channels_flagged) > 0:
                    # Add new flag command to flag data underlying the
                    # view.
                    newflags.append(arrayflaggerbase.FlagCmd(
                        reason='diffmad', filename=table, rulename=rulename, spw=spw, pol=pol, antenna=antenna,
                        axisnames=axisnames, flagcoords=flagcoords, antenna_id_to_name=antenna_id_to_name))

            elif rulename == 'tmf':

                # Stop evaluating rule if all data is flagged.
                if np.all(flag):
                    continue

                # Get limits.
                frac_limit = rule['frac_limit']
                nchan_limit = rule['nchan_limit']

                # flag all channels if fraction already flagged
                # is greater than tmf_limit of total
                if (float(np.count_nonzero(rflag)) / len(rdata) >= frac_limit or
                        np.count_nonzero(rflag) >= nchan_limit):

                    newflag = np.logical_not(rflag)

                    # flag the 'view', for any subsequent rules being evaluated.
                    rflag[newflag] = True

                    # now compose a description of the flagging required on
                    # the MS
                    channels_flagged = channels[newflag]
                    flagcoords = [list(channels_flagged)]

                    if len(channels_flagged) > 0:
                        # Add new flag command to flag data underlying the
                        # view.
                        newflags.append(arrayflaggerbase.FlagCmd(
                            reason='tmf', filename=table, rulename=rulename, spw=spw, pol=pol, antenna=antenna,
                            axisnames=axisnames, flagcoords=flagcoords, antenna_id_to_name=antenna_id_to_name))

            else:
                raise NameError('bad rule: %s' % rule)

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
            if not flag[i] and mad[i] < 2.0 * median_mad:
                noise_edge = i
                break

        return noise_edge

    @staticmethod
    def _find_small_diff(data, flag, limit=2.0, description='unknown'):
        """Return the index in the first quarter of the data array where the
        point to point difference first falls below a threshold, where 
        the threshold is defined as the "limit" * the median point-to-point
        difference.

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
        good_data_diff = abs(good_data[1:] - good_data[:-1])
        median_diff = np.median(good_data_diff)

        for i, diff in enumerate(good_data_diff):
            if diff < limit * median_diff:
                result = good_data_index[i]
                break

        if result is None:
            LOG.warning('edge finder failed for:%s' % description)
            # flag one edge channel - sole purpose of this is to ensure
            # that a plot is made in the weblog so that the problem
            # can be understood
            result = 1

        return result
