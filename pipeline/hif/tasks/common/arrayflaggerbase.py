from __future__ import absolute_import

import os.path
import numpy as np

def median_and_mad(data):
    """
    Return the median and MAD of the numpy data array.
    """
    data_median = None
    data_mad = None
    if len(data) > 0:
        data_median = np.median(data)
        data_mad = np.median(np.abs(data - data_median))
    return data_median, data_mad

 
class FlagOp(object):
    """
    Create a flagging operation object.
        Added detailed docs here.
    """
    def __init__(self, filename, rulename, spw, axisnames, flagcoords, 
      cell_index=None, ruleaxis=None, flagchannels=None):
        self.filename = filename
        self.rulename = rulename
        self.spw = spw
        self.axisnames = axisnames
        self.flagcoords = flagcoords
        self.cell_index = cell_index
        self.ruleaxis = ruleaxis
        self.flagchannels = flagchannels

    def __repr__(self):
        # Format the FlagOp for the terminal.
        if self.filename is not None:
            basename = os.path.basename(self.filename)
        else:
            basename = None
        s = 'FlagOp: filename-%s rule-%s axis-%s spw-%s axisnames-%s flagcoords-%s cell_index-%s flagchannels-%s' % (
          basename, self.rulename, self.ruleaxis, self.spw, self.axisnames,
          self.flagcoords, self.cell_index, self.flagchannels) 

        return s

