from __future__ import absolute_import
from . import measures
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


class SpectralWindow(object):
    """
    SpectralWindow is a logical representation of a spectral window. 
    
    .. py:attribute:: id
    
        the numerical identifier of this spectral window within the
        SPECTRAL_WINDOW subtable of the measurement set

    .. py:attribute:: channels
    
        the number of channels
        
    .. py:attribute:: bandwidth
    
        the total bandwidth
        
    .. py:attribute:: ref_frequency

        the reference frequency

    .. py:attribute:: chan_width

        the channel width

    .. py:attribute:: group
    
        the calibration group to which this spectral window belongs
        
    .. py:attribute:: intents

        the observing intents that have been observed using this spectral
        window
    """
    
    __slots__ = ('id', 'band', 'bandwidth', 'channels', 'group', 'intents',
                 'ref_frequency', 'name', 'baseband', 'sideband')

    def __getstate__(self):
        return (self.id, self.band, self.bandwidth, self.channels, self.group,
            self.intents, self.ref_frequency, self.name, self.baseband, 
            self.sideband)

    def __setstate__(self, state):
        (self.id, self.band, self.bandwidth, self.channels, self.group,
            self.intents, self.ref_frequency, self.name, self.baseband, 
            self.sideband) = state
    
    def __init__(self, spw_id, bandwidth, ref_frequency, chan_widths, 
                 chan_freqs, name, sideband, baseband, group=None, 
                 band='Unknown'):
        self.id = spw_id
        self.bandwidth = measures.Frequency(bandwidth,
                                            measures.FrequencyUnits.HERTZ)
        self.ref_frequency = measures.Frequency(ref_frequency, 
                                                measures.FrequencyUnits.HERTZ)
        self.group = group
        self.intents = set()
        self.band = band

        # work around NumPy bug with empty strings
        # http://projects.scipy.org/numpy/ticket/1239
        self.name = str(name)
        self.sideband = str(sideband)
        self.baseband = str(baseband)
        
        channels = []
        for (centre, width) in zip(chan_freqs, chan_widths):
            f_lo = measures.Frequency(centre[0] - (width[0] / 2.0),
                                      measures.FrequencyUnits.HERTZ)
            f_hi = measures.Frequency(centre[0] + (width[0] / 2.0),
                                      measures.FrequencyUnits.HERTZ)
            channels.append(measures.FrequencyRange(f_lo, f_hi))
        self.channels = channels

    @property
    def centre_frequency(self):
        return (self.min_frequency + self.max_frequency) / 2.0

    def channel_range(self, minfreq, maxfreq):
        '''
    	    # More work on this in future 
    	    minfreq -- measures.Frequency object in HERTZ
    	    maxfreq -- measures.Frequency object in HERTZ
    	'''
        freqmin = minfreq
        freqmax = maxfreq
    
        # Check for the no overlap case.
        nchan = len(self.channels)
        if freqmax < self.min_frequency:
            return (None, None)
        if freqmin > self.max_frequency:
            return (None, None)
    
        # Find the minimum channel
        chanmin = 0
        if self.channels[0].low < self.channels[nchan-1].low:
            for i in range(nchan):
                if self.channels[i].low > freqmin:
                    break
                chanmin = i
        else:
            for i in range(nchan):
                if self.channels[i].high < freqmax:
                    break
                chanmin = i
    
        # Find the maximum channel
        chanmax = nchan - 1
        if self.channels[0].low < self.channels[nchan-1].low:
            for i in range(nchan-1, -1, -1):
                if self.channels[i].high < freqmax:
                    break
                chanmax = i
        else:
            for i in range(nchan-1, -1, -1):
                if self.channels[i].low > freqmin:
                    break
                chanmax = i
    
        return (chanmin, chanmax)

    @property
    def min_frequency(self):
        return min(self.channels, key=lambda r: r.low).low
        
    @property
    def max_frequency(self):
        return max(self.channels, key=lambda r: r.high).high
        
    @property
    def num_channels(self):
        return len(self.channels)

    def __repr__(self):
        args = map(str, [self.id, self.centre_frequency, self.bandwidth, 
                         self.group.name])
        return 'SpectralWindow({0})'.format(', '.join(args))

        


class SpectralWindowWithChannelSelection(object):
    """
    SpectralWindowWithChannelSelection decorates a SpectralWindow so that the
    spectral window ID also contains a channel selection.
    """
    def __init__( self, subject, channels):
        self._subject = subject
        self._channels = [str(channel) for channel in sorted(list(channels))]

    def __getattr__( self, name ):
        return getattr( self._subject, name )

    @property
    def id(self):
        channels = ';'.join(self._channels)
        channels = ':' + channels if channels else ''
        return '{spw}{channels}'.format(spw=self._subject.id,
                                        channels=channels)
