from __future__ import absolute_import

import numpy as np

from . import arrayflaggerbase
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)


class MatrixFlaggerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, datatask=None,
        flagsettertask=None, rules=None, niter=None):

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

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs

        datatask = self.datatask
        rules = inputs.rules
        flagsettertask = self.flagsettertask
        niter = inputs.niter

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

        if len(newflags) != 0:
            # exited loop when flags still being found, recalculate
            # result with latest flags incorporated.
            result = self._executor.execute(datatask)

        # add a record of flags set to the results object
        result.addflags(flags)

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
      flag_nmedian=False, fnm_limit=4.0):

        """
        Generate a list of flagging rules from a set of flagging parameters.
        Added detailed docs here.
        """

        # Construct rules from flag properties
        rules = []
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
        if flag_nmedian:
            rules.append({'name':'nmedian', 'limit':fnm_limit})
        return rules

    @staticmethod
    def generate_flags(matrix, rules):
        """
        Calculate the statistics of a  matrix and flag the data according
        to a list of specified rules.

        Keyword arguments:
        matrix - ImageResult object containing data to be flagged.
        rules - Rules to be applied.
        """

        # Get the attributes
        data = matrix.data
        flag = matrix.flag
        nodata = matrix.nodata
        xtitle = matrix.axes[0].name
        xdata = matrix.axes[0].data
        ytitle = matrix.axes[1].name
        ydata = matrix.axes[1].data 
        spw = matrix.spw
        table = matrix.filename
        cell_index = matrix.cell_index

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
                        newflags.append(arrayflaggerbase.FlagOp(
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                        flagcoords=zip(xdata[i2flag], ydata[j2flag]),
                        cell_index=cell_index))
  
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

                        # Add new flag command to flag data underlying the
                        # view.
                        newflags.append(arrayflaggerbase.FlagOp(
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=zip(xdata[i2flag], ydata[j2flag]),
                          cell_index=cell_index))

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

                        # Add new flag command to flag data underlying the
                        # view.
                        newflags.append(arrayflaggerbase.FlagOp(
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=zip(xdata[i2flag], ydata[j2flag]),
                          cell_index=cell_index))

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

                        # Add new flag command to flag data underlying the
                        # view.
                        newflags.append(arrayflaggerbase.FlagOp(
                          filename=table, rulename=rulename, spw=spw,
                          axisnames=[xtitle, ytitle],
                          flagcoords=zip(xdata[i2flag], ydata[j2flag]),
                          cell_index=cell_index))

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

                        # Add new flag command to flag data underlying the
                        # view.
                        newflags.append(arrayflaggerbase.FlagOp(
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=zip(xdata[i2flag], ydata[j2flag]),
                          cell_index=cell_index))

                        # Flag the view
                        flag[i2flag, j2flag] = True

                    elif rulename == 'too many flags':
 
                        maxfraction = rule['limit']
                        axis = rule['axis']
                        axis = axis.upper()

                        if axis == xtitle.upper():

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
  
                                # Add new flag command to flag data underlying 
                                # the view.
                                newflags.append(arrayflaggerbase.FlagOp(
                                  filename=table,
                                  rulename=rulename, spw=spw, axisnames=[xtitle,
                                  ytitle], flagcoords=zip(xdata[i2flag],
                                  ydata[j2flag]), cell_index=cell_index))

                                # Flag the view
                                flag[i2flag, j2flag] = True

                        elif axis == ytitle.upper():

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

                                # Add new flag command to flag data underlying 
                                # the view.
                                newflags.append(arrayflaggerbase.FlagOp(
                                  filename=table, rulename=rulename, spw=spw,
                                  axisnames=[xtitle, ytitle],
                                  flagcoords=zip(xdata[i2flag], ydata[j2flag]),
                                  cell_index=cell_index))

                                # Flag the view.
                                flag[i2flag, j2flag] = True

                    elif rulename == 'nmedian':

                        # Check limits.
                        limit = rule['limit']
                        i2flag = i[np.logical_and(data > limit * data_median,
                          np.logical_not(flag))]
                        j2flag = j[np.logical_and(data > limit * data_median,
                          np.logical_not(flag))]

                        # No flags
                        if len(i2flag) <= 0:
                            continue

                        # Add new flag command to flag the data underlying
                        # the view.
                        newflags.append(arrayflaggerbase.FlagOp(
                          filename=table, rulename=rulename,
                          spw=spw, axisnames=[xtitle, ytitle],
                          flagcoords=zip(xdata[i2flag], ydata[j2flag]),
                          cell_index=cell_index))

                        # Flag the view.
                        flag[i2flag, j2flag] = True

                    else:           
                        raise NameError, 'bad rule: %s' % rule

        return newflags
