from __future__ import absolute_import

import os.path
import numpy as np

import pipeline.infrastructure.casatools as casatools

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

 
class FlagCmd(object):
    """
    Create a flagcmd.
        Added detailed docs here.
    """
    def __init__(self, filename, rulename, spw, axisnames,
      flagcoords, cell_index=None, ruleaxis=None, flagchannels=None,
      reason=None):
#        print 'FlagCmd spw%s axisnames%s flagcoords%s cell_index%s flagchannels%s reason%s' % (
#          spw, axisnames, flagcoords, cell_index, flagchannels, reason)

        self.filename = filename
        self.rulename = rulename
        self.spw = spw
        self.cell_index = cell_index
        self.ruleaxis = ruleaxis
        self.flagchannels = flagchannels
        self.axisnames = axisnames
#        self.axisnames = str(axisnames).upper()
        self.flagcoords = flagcoords
        self.reason = reason

        # decode the flagcoords
        self.antenna = None
        self.flag_time = None
        if axisnames is not None:
            self.axisnames = []
            for k,name in enumerate(axisnames):
                self.axisnames.append(name.upper())
                if name.upper()=='ANTENNA1':
                    self.antenna = flagcoords[k]
                elif name.upper()=='TIME':
                    self.flag_time = flagcoords[k]

        # construct the corresponding flag command
        flagcmd = ""

        if spw is not None:
            flagcmd += " spw='%s'" % spw

        if flagchannels is not None:
            flagcmd = flagcmd[:-1] + ":%s'" % ';'.join(map(str,
              flagchannels))

        if cell_index == 0:
            flagcmd += " correlation='XX'"
        elif cell_index == 1:
            flagcmd += " correlation='YY'"

        flagcmd += " reason='%s'" % reason

        if self.antenna is not None:
            flagcmd += " antenna='%s'" % (self.antenna)

        if self.flag_time is not None:
            start = casatools.quanta.quantity(self.flag_time - 0.5, 's')
            start = casatools.quanta.time(start, form=['ymd'])
            end = casatools.quanta.quantity(self.flag_time + 0.5, 's')
            end = casatools.quanta.time(end, form=['ymd'])
            flagcmd += " timerange='%s~%s'" % (start[0], end[0])

        self.flagcmd = flagcmd
#        print 'flagcmd', flagcmd

    def __repr__(self):
        # Format the FlagCmd for the terminal.
        if self.filename is not None:
            basename = os.path.basename(self.filename)
        else:
            basename = None
        s = 'FlagCmd: filename-%s flagcmd-%s' % (basename, self.flagcmd)

        return s

    def match(self, spectrum):
        """Return True if the FlagCmd operates on this SpectrumResult.
        """
        match = True
        match = match and (self.filename == spectrum.filename)
        if self.spw is not None:
            match = match and (self.spw == spectrum.spw)
        if self.antenna is not None:
            match = match and (self.antenna == spectrum.ant[0])
        if self.flag_time is not None:
            match = match and (self.flag_time > spectrum.time-0.5 and
              self.flag_time < spectrum.time + 0.5)
        return match

    def match_image(self, image):
        """Return True if the FlagCmd operates on this ImageResult.
        """
        match = True
        match = match and (self.filename == image.filename)
        if self.spw is not None:
            match = match and (self.spw == image.spw)
        if self.antenna is not None:
            match = match and ('ANTENNA' in str(self.axisnames))
        if self.flag_time is not None:
            match = match and ('TIME' in self.axisnames)
        if self.cell_index is not None:
            match = match and (self.cell_index == image.cell_index)
        return match
