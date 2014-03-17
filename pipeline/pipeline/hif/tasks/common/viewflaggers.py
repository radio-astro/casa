from __future__ import absolute_import

import numpy as np

from . import arrayflaggerbase
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class MatrixFlaggerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, datatask=None,
        flagsettertask=None, rules=None, niter=None, extendfields=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())


class MatrixFlagger(basetask.StandardTaskTemplate):
    Inputs = MatrixFlaggerInputs

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
            LOG.info('flagcmds will be extended by removing selection in following fields: %s'
              % inputs.extendfields)

        iter = 1
        flags = []
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
                newflags += self.generate_flags(image, rules)

            # set any flags raised
            LOG.info('Iteration %s raised %s flagging commands' % \
	        (iter, len(newflags)))
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
        result.table = flagsettertask.inputs.table
        result.flagcmdfile = flagsettertask.inputs.inpfile
        result.reason = flagsettertask.inputs.reason

        return result

    def analyse(self, result):
        return result

    @staticmethod
    def make_flag_rules (
      flag_hilo=False, fhl_limit=5.0, fhl_minsample=5, 
      flag_hi=False, fhi_limit=5.0, fhi_minsample=5,
      flag_lo=False, flo_limit=5.0, flo_minsample=5,
      flag_tmf1=False, tmf1_axis='Time', tmf1_limit=0.5,
      flag_tmf2=False, tmf2_axis='Time', tmf2_limit=0.5,
      flag_nmedian=False, fnm_limit=4.0,
      flag_maxabs=False, fmax_limit=0.1,
      flag_minabs=False, fmin_limit=0.0):

        """
        Generate a list of flagging rules from a set of flagging parameters.
        Added detailed docs here.
        """

        # Construct rules from flag properties. If niter is set to curtail
        # the flagging loop then the order that the rules are applied 
        # can be important. For example, 'too many flags' should run after
        # the other rules.
        rules = []
        if flag_maxabs:
            rules.append({'name':'max abs', 'limit':fmax_limit})
        if flag_minabs:
            rules.append({'name':'min abs', 'limit':fmin_limit})
        if flag_nmedian:
            rules.append({'name':'nmedian', 'limit':fnm_limit})
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
              'limit':tmf1_limit})
        if flag_tmf2:
            rules.append({'name':'too many flags',
              'axis':str.upper(tmf2_axis),
              'limit':tmf2_limit})

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

        # Index arrays
        i,j = np.indices(np.shape(data))

        # limit_multiplier was added long ago to prevent bad flags being set
        # because of the view statistics being skewed by extreme outliers.
        # The idea was to flag lightly to begin with, then
        # recalculate the view statistics, then flag more severely
        # until the final 'limit' was reached.
        # Using robust measures such as medians and MAD it is not
        # clear that this is necessary (it may have been added before
        # we were using robust statistics, I forget). So for now
        # the iteration is commented out and we go straight
        # to the final 'limit'.
        # for limit_multiplier in [9, 3, 1]:

        for limit_multiplier in [1]:
            flagging = True
            while flagging:
                flagging = False

                rdata = np.ravel(data)
                rflag = np.ravel(flag)
                valid_data = rdata[np.logical_not(rflag)]
                data_median, data_mad = arrayflaggerbase.median_and_mad(
                  valid_data)

                # flag data according to each rule in turn
                for rule in rules:
                    rulename = rule['name']
                    if rulename == 'outlier':
  
                        # Sample too small
                        minsample = rule['minsample']
                        if len(valid_data) < minsample:
                            continue

                        # Check limits.
                        mad_max = limit_multiplier * rule['limit']
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
                              reason='stage%s' % self.inputs.context.stage,
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=[xtitle, ytitle],
                              flagcoords=flagcoord, pol=pol))
  
                        # Flag the view.
                        flag[i2flag, j2flag] = True

                    elif rulename == 'low outlier':

                        # Sample too small
                        minsample = rule['minsample']
                        if len(valid_data) < minsample:
                            continue

                        # Check limits.
                        mad_max = limit_multiplier * rule['limit']
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
                              reason='stage%s' % self.inputs.context.stage,
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=[xtitle, ytitle],
                              flagcoords=flagcoord, pol=pol))

                        # Flag the view.
                        flag[i2flag, j2flag] = True

                    elif rulename == 'high outlier':

                        # Sample too small
                        minsample = rule['minsample']
                        if len(valid_data) < minsample:
                            continue

                        # Check limits.
                        mad_max = limit_multiplier * rule['limit']
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
                              reason='stage%s' % self.inputs.context.stage,
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=[xtitle, ytitle],
                              flagcoords=flagcoord, pol=pol))

                        # Flag the view.
                        flag[i2flag, j2flag] = True

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
                              reason='stage%s' % self.inputs.context.stage,
                              filename=table, rulename=rulename, spw=spw,
                              axisnames=[xtitle, ytitle],
                              flagcoords=flagcoord, pol=pol))

                        # Flag the view
                        flag[i2flag, j2flag] = True

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
                              reason='stage%s' % self.inputs.context.stage,
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=[xtitle, ytitle],
                              flagcoords=flagcoord, pol=pol))

                        # Flag the view
                        flag[i2flag, j2flag] = True

                    elif rulename == 'too many flags':
 
                        maxfraction = rule['limit']
                        axis = rule['axis']
                        axis = axis.upper().strip()

                        if axis == xtitle.upper().strip():

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
                                if fractionflagged <= maxfraction:
                                    continue

                                i2flag = i[:,iy][np.logical_not(flag[:,iy])]
                                j2flag = j[:,iy][np.logical_not(flag[:,iy])]
  
                                # Add new flag commands to flag data underlying 
                                # the view.
                                flagcoords = zip(xdata[i2flag], ydata[j2flag])
                                for flagcoord in flagcoords:
                                    newflags.append(arrayflaggerbase.FlagCmd(
                                      reason='stage%s' % self.inputs.context.stage,
                                      filename=table, rulename=rulename,
                                      spw=spw, antenna=antenna, 
                                      axisnames=[xtitle,ytitle],
                                      flagcoords=flagcoord, pol=pol))

                                # Flag the view
                                flag[i2flag, j2flag] = True

                        elif axis == ytitle.upper().strip():

                            # look along y axis
                            for ix in np.arange(len(xdata)):
                                if all(flag[ix,:]):
                                    continue

                                # Compute fraction flagged
                                len_data = len(ydata)
                                len_no_data = len(data[ix,:][nodata[ix,:]])
                                len_flagged = len(data[ix,:][flag[ix,:]])
                                fractionflagged = (
                                  float(len_flagged - len_no_data) / 
                                  float(len_data - len_no_data))
                                if fractionflagged <= maxfraction:
                                    continue

                                i2flag = i[ix,:][np.logical_not(flag[ix,:])]
                                j2flag = j[ix,:][np.logical_not(flag[ix,:])]

                                # Add new flag commands to flag data underlying 
                                # the view.
                                flagcoords = zip(xdata[i2flag], ydata[j2flag])
                                for flagcoord in flagcoords:
                                    newflags.append(arrayflaggerbase.FlagCmd(
                                      reason='stage%s' % self.inputs.context.stage,
                                      filename=table, rulename=rulename, spw=spw,
                                      axisnames=[xtitle, ytitle],
                                      flagcoords=flagcoord, pol=pol))

                                # Flag the view.
                                flag[i2flag, j2flag] = True

                    elif rulename == 'nmedian':

                        # Check for valid median
                        if data_median is None:
                            continue

                        # Check limits.
                        limit = rule['limit']
                        if limit < 1:
                            i2flag = i[np.logical_and(data < limit * data_median,
                              np.logical_not(flag))]
                            j2flag = j[np.logical_and(data < limit * data_median,
                              np.logical_not(flag))]
                        else:
                            i2flag = i[np.logical_and(data > limit * data_median,
                              np.logical_not(flag))]
                            j2flag = j[np.logical_and(data > limit * data_median,
                              np.logical_not(flag))]

                        # No flags
                        if len(i2flag) <= 0:
                            continue

                        # Add new flag commands to flag the data underlying
                        # the view.
                        flagcoords=zip(xdata[i2flag], ydata[j2flag])
                        for flagcoord in flagcoords:
                            newflags.append(arrayflaggerbase.FlagCmd(
                              reason='stage%s' % self.inputs.context.stage,
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=[xtitle, ytitle],
                              flagcoords=flagcoord, pol=pol,
                              extendfields=self.inputs.extendfields))

                        # Flag the view.
                        flag[i2flag, j2flag] = True

                    else:           
                        raise NameError, 'bad rule: %s' % rule

        # consolidate flagcmds that specify individual channels into fewer
        # flagcmds that specify ranges
        newflags = arrayflaggerbase.consolidate_flagcmd_channels(newflags)

        return newflags


class VectorFlaggerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, datatask=None,
        flagsettertask=None, rules=None, niter=None, intent=None):

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
            LOG.info('Iteration %s raised %s flagging commands' % \
	        (iter, len(newflags)))
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
        result.flagcmdfile = flagsettertask.inputs.inpfile
        result.reason = flagsettertask.inputs.reason

        return result

    def analyse(self, result):
        return result

    @staticmethod
    def make_flag_rules (flag_edges=False, edge_limit=2.0,
      flag_minabs=False, fmin_limit=0.0,
      flag_nmedian=False, fnm_limit=0.0,
      flag_hilo=False, fhl_limit=5.0, fhl_minsample=5, 
      flag_sharps=False, sharps_limit=0.05,
      flag_sharps2=False, sharps2_limit=0.05,
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
            rules.append({'name':'nmedian', 'limit':fnm_limit})
        if flag_hilo:
            rules.append({'name':'outlier', 'limit':fhl_limit,
              'minsample':fhl_minsample})
        if flag_sharps:
            rules.append({'name':'sharps', 'limit':sharps_limit})
        if flag_sharps2:
            rules.append({'name':'sharps2', 'limit':sharps2_limit})
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

        flagging = True
        while flagging:
            flagging = False

            rdata = np.ravel(data)
            rflag = np.ravel(flag)
            valid_data = rdata[np.logical_not(rflag)]
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
                              reason='stage%s' % self.inputs.context.stage,
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
                              reason='stage%s' % self.inputs.context.stage,
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
                              reason='stage%s' % self.inputs.context.stage,
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
                          reason='stage%s' % self.inputs.context.stage,
                          filename=table, rulename=rulename,
                          spw=spw, pol=pol, antenna=antenna,
                          axisnames=axisnames, flagcoords=flagcoords))

                elif rulename == 'sharps':
                    limit = rule['limit']
                    if len(valid_data):

                        diff = abs(rdata[1:] - rdata[:-1])
                        diff_flag = np.logical_or(rflag[1:], rflag[:-1])

                        newflag = (diff>limit) & np.logical_not(diff_flag)
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
                              reason='stage%s' % self.inputs.context.stage,
                              filename=table, rulename=rulename,
                              spw=spw, axisnames=axisnames,
                              flagcoords=flagcoords))

                elif rulename == 'sharps2':
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
                              reason='stage%s' % self.inputs.context.stage,
                              filename=table,
                              rulename=rulename,
                              spw=spw, axisnames=axisnames,
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
                              reason='stage%s' % self.inputs.context.stage,
                              filename=table, rulename=rulename,
                              spw=spw, pol=pol, antenna=antenna))

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
                                  reason='stage%s' % self.inputs.context.stage,
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

