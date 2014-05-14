from __future__ import absolute_import

import collections
import os.path
import numpy as np
import types

import pipeline.infrastructure.casatools as casatools

def channel_ranges(channels):
    """
    Given a list of channels will return a list of 
    ranges that describe them.
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

def consolidate_flagcmd_channels(flagcmds):
    """Method to consolidate multiple flagcmds that specify a single
       channel into fewer flagcmds with channel ranges."""

    consolidated_flagcmds = []
    flagcmd_dict = collections.defaultdict(set)
    flagcmd_coords_dict = {}

    # find FlagCmd objects that can/cannot be consolidated
    for flagcmd in flagcmds:
        flagchannels = None
        if flagcmd.axisnames is not None:
            for k,name in enumerate(flagcmd.axisnames):
                if name.upper()=='CHANNELS':
                    flagchannels = [flagcmd.flagcoords[k]]
                    # need remaining flagcoords in dict key
                    truncated_flagcoords = list(flagcmd.flagcoords)
                    truncated_flagcoords[k] = None

        if flagchannels is not None:
            ftuple = (
              flagcmd.filename,
              flagcmd.rulename,
              flagcmd.ruleaxis,
              flagcmd.spw,
              flagcmd.antenna,
              flagcmd.intent,
              flagcmd.pol,
              tuple(flagcmd.axisnames),
              tuple(truncated_flagcoords),
              flagcmd.channel_axis,
              flagcmd.reason,
              flagcmd.extendfields)
            flagcmd_dict[ftuple].update(flagchannels)
            flagcmd_coords_dict[ftuple] = list(flagcmd.flagcoords)
        else:
            consolidated_flagcmds.append(flagcmd)

    # add consolidated flagcmds
    for ftuple in flagcmd_dict.keys():
        flagchannels = list(flagcmd_dict[ftuple])
        flagchannels.sort()

        flagcoords = flagcmd_coords_dict[ftuple]
        for k,name in enumerate(list(ftuple[7])):
            if name.upper()=='CHANNELS':
                flagcoords[k] = flagchannels

        consolidated_flagcmds.append(FlagCmd(
          filename=ftuple[0],
          rulename=ftuple[1],
          ruleaxis=ftuple[2],
          spw=ftuple[3],
          antenna=ftuple[4],
          intent=ftuple[5],
          pol=ftuple[6],
          axisnames=list(ftuple[7]),
          flagcoords=flagcoords,
          channel_axis=ftuple[9],
          reason=ftuple[10],
          extendfields=ftuple[11]))

    return consolidated_flagcmds

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

    def __init__(self, filename=None, rulename=None, ruleaxis=None, spw=None,
      antenna=None, intent=None, pol=None, axisnames=None,
      flagcoords=None, channel_axis=None, reason=None,
      extendfields=None):
#        print 'FlagCmd intent%s spw%s antenna%s axisnames%s flagcoords%s pol%s reason%s' % (
#          intent, spw, antenna, axisnames, flagcoords, pol, reason)

        self.filename = filename
        self.rulename = rulename
        self.ruleaxis = ruleaxis
        self.spw = spw
        self.antenna = antenna
        self.intent = intent
        self.pol = pol
        self.axisnames = axisnames
        self.flagcoords = flagcoords
        self.channel_axis = channel_axis
        self.reason = reason
        self.extendfields = extendfields

        # construct the corresponding flag command
        flagcmd = ""

        if intent is not None:
            flagcmd += " intent='%s'" % intent

        if pol is not None:
            flagcmd += " correlation='%s'" % pol

        flagcmd += " reason='%s'" % reason

        if self.antenna is not None:
            flagcmd += " antenna='%s'" % (self.antenna)

        if spw is not None:
            flagcmd += " spw='%s'" % spw

        # decode axisnames/flagcoords
        if axisnames is not None:
            for k,name in enumerate(axisnames):
                if name.upper()=='CHANNELS':
                    flagchannels = flagcoords[k]
                    if not isinstance(flagchannels, types.ListType):
                        flagchannels = [flagchannels]

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

            # antenna/baseline flags
            ax_antenna = None
            for k,name in enumerate(axisnames):
                if 'ANTENNA' in name.upper():
                    if antenna is None or antenna==flagcoords[k]:
                        ax_antenna = flagcoords[k]
                    else:
                        ax_antenna = '%s&%s' % (antenna, flagcoords[k])
                elif name.upper()=='BASELINE':
                    ax_antenna = flagcoords[k]
            if ax_antenna is not None:
                flagcmd += " antenna='%s'" % (ax_antenna)
                self.antenna = ax_antenna

            flag_time = None
            for k,name in enumerate(axisnames):
                if name.upper()=='TIME':
                    flag_time = flagcoords[k]

            self.start_time = None
            self.end_time = None
            if flag_time is not None:
                self.start_time = flag_time - 0.5
                start = casatools.quanta.quantity(self.start_time, 's')
                start = casatools.quanta.time(start, form=['ymd'])
                self.end_time = flag_time + 0.5
                end = casatools.quanta.quantity(self.end_time + 0.5, 's')
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

#        print 'flagcmd', flagcmd
        self.flagcmd = flagcmd

    @property
    def flagchannels(self):
        """Return list of channels flagged.
        """
        result = np.array([], np.int)
        # decode axisnames/flagcoords
        if self.axisnames is not None:
            for k,name in enumerate(self.axisnames):
                if name.upper()=='CHANNELS':
                    flagchannels = self.flagcoords[k]
                    if not isinstance(flagchannels, types.ListType):
                        flagchannels = [flagchannels]
                    result = np.array(flagchannels)
                    break
        return result

    def match(self, spectrum):
        """Return True if the FlagCmd operates on this SpectrumResult.
        """
        match = True
        match = match and (self.filename == spectrum.filename)

        # does spw match?
        match = match and (
          ("spw=" not in self.__repr__()) or
          (("spw='%s" % spectrum.spw) in self.__repr__()))

        # does antenna match?
        try:
            match = match and (
              ("antenna=" not in self.__repr__()) or
              (("antenna='%s'" % spectrum.ant[0]) in self.__repr__()))
        except:
            match = False

        # does time match?
        if spectrum.time is not None:
            try:
                match = match and (spectrum.time > self.start_time and
                  spectrum.time < self.end_time)
            except:
                match = False        

        # does correlation/pol match?
        match = match and (
          ("correlation=" not in self.__repr__()) or
          (("correlation='%s'" % spectrum.pol) in self.__repr__()))

        return match

    def match_image(self, image):
        """Return True if the FlagCmd operates on this ImageResult.
        """
        match = True
        match = match and (self.filename == image.filename)

        # does spw match?
        match = match and (
          ("spw=" not in self.__repr__()) or
          (("spw='%s" % image.spw) in self.__repr__()))

        # does antenna match?
        if self.antenna is not None:
            match = match and (('ANTENNA' in str(self.axisnames).upper()) or 
              ('BASELINE' in str(self.axisnames).upper()))

#        if self.flag_time is not None:
#            match = match and ('TIME' in self.axisnames)

        # does correlation/pol match?
        match = match and (
          ("correlation=" not in self.__repr__()) or
          (("correlation='%s'" % image.pol) in self.__repr__()))

        return match

    def __repr__(self):
        # Format the FlagCmd for the terminal.
        if self.filename is not None:
            basename = os.path.basename(self.filename)
        else:
            basename = None
        s = 'FlagCmd: filename-%s flagcmd-%s' % (basename, self.flagcmd)

        return s
