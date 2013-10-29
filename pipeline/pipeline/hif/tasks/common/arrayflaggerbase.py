from __future__ import absolute_import

import os.path
import numpy as np

import pipeline.infrastructure.casatools as casatools

def channel_ranges(channels):
    """
    Given a list of channels will return a list of 
    contiguous ranges that describe them.
    """
    channels.sort()

    range = [channels[0], channels[0]]

    for i,chan in enumerate(channels):
        if chan <= range[1] + 1:
            range[1] = chan
        else:
            return [range] + channel_ranges(channels[i:])

    # get here if last channel reached
    return [range]        

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
    def __init__(self, filename, rulename, spw, antenna=None, axisnames=None,
      flagcoords=None, intent=None, pol=None, ruleaxis=None,
      flagchannels=None, channel_axis=None, reason=None,
      extendfields=None):
#        print 'FlagCmd intent %s spw%s antenna%s axisnames%s flagcoords%s pol%s flagchannels%s reason%s' % (
#          intent, spw, antenna, axisnames, flagcoords, pol, flagchannels,
#          reason)

        self.filename = filename
        self.rulename = rulename
        self.intent = intent
        self.spw = spw
        self.pol = pol
        self.ruleaxis = ruleaxis
        self.flagchannels = flagchannels
        self.axisnames = axisnames
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
        else:
            self.antenna = antenna

        # construct the corresponding flag command
        flagcmd = ""

        if intent is not None:
            flagcmd += " intent='%s'" % intent

        if spw is not None:
            flagcmd += " spw='%s'" % spw

        if flagchannels is not None:
            ranges = channel_ranges(flagchannels)

            if channel_axis is None:
                # just set the ranges of channels directly
                rangestrs = []
                for trange in ranges:
                    rangestrs.append('%s~%s' % (trange[0], trange[1]))
            else:
                # convert the channel ranges to use the axis values
                # and units
                rangestrs = []
                channel_width = channel_axis.channel_width
                for trange in ranges:
                    axrange = [
                      channel_axis.data[trange[0]]-channel_width/2,
                      channel_axis.data[trange[1]]+channel_width/2]
                    rangestrs.append('%s~%s%s' % (axrange[0],
                      axrange[1], channel_axis.units))

            flagcmd = flagcmd[:-1] + ":%s'" % ';'.join(rangestrs)

        if pol is not None:
            flagcmd += " correlation='%s'" % pol

        flagcmd += " reason='%s'" % reason

        if self.antenna is not None:
            flagcmd += " antenna='%s'" % (self.antenna)

        if self.flag_time is not None:
            start = casatools.quanta.quantity(self.flag_time - 0.5, 's')
            start = casatools.quanta.time(start, form=['ymd'])
            end = casatools.quanta.quantity(self.flag_time + 0.5, 's')
            end = casatools.quanta.time(end, form=['ymd'])
            flagcmd += " timerange='%s~%s'" % (start[0], end[0])

        flagcmd = flagcmd.strip()

        # lastly, remove any 'extend' fields requested, done to extend the effect
        # of the flagcmd beyond the detected data
        if extendfields:
            specifiers = flagcmd.split(' ')
            for specifier in specifiers:
                for extendfield in extendfields:
                    if extendfield in specifier:
                        specifiers.remove(specifier)
                        break
            flagcmd = ' '. join(specifiers)

        self.flagcmd = flagcmd
#        print 'flagcmd', flagcmd

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
        if self.pol is not None:
            match = match and (self.pol == spectrum.pol)
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
        if self.pol is not None:
            match = match and (self.pol == image.pol)
        return match

    def __repr__(self):
        # Format the FlagCmd for the terminal.
        if self.filename is not None:
            basename = os.path.basename(self.filename)
        else:
            basename = None
        s = 'FlagCmd: filename-%s flagcmd-%s' % (basename, self.flagcmd)

        return s

