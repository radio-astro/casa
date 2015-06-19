from __future__ import absolute_import
import decimal
import itertools
import operator

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

    .. py:attribute:: intents

        the observing intents that have been observed using this spectral
        window
    """
    
    __slots__ = ('id', 'band', 'bandwidth', 'type', 'intents',
                 'ref_frequency', 'name', 'baseband', 'sideband',
                 'mean_frequency', '_min_frequency', '_max_frequency',
                 '_centre_frequency', '_chan_freqs', '_chan_widths',
                 '_channels')

    def __getstate__(self):
        return (self.id, self.band, self.bandwidth, self.type, self.intents,
                self.ref_frequency, self.name, self.baseband, self.sideband, 
                self.mean_frequency, self._min_frequency, 
                self._max_frequency, self._centre_frequency, self._chan_freqs,
                self._chan_widths)

    def __setstate__(self, state):
        (self.id, self.band, self.bandwidth, self.type, self.intents,
                self.ref_frequency, self.name, self.baseband, self.sideband, 
                self.mean_frequency, self._min_frequency, 
                self._max_frequency, self._centre_frequency, self._chan_freqs,
                self._chan_widths) = state
    
    def __init__(self, spw_id, name, spw_type, bandwidth, ref_freq, mean_freq,
                 chan_freqs, chan_widths, sideband, baseband, band='Unknown'):
        self.id = spw_id
        self.bandwidth = measures.Frequency(bandwidth,
                                            measures.FrequencyUnits.HERTZ)
        self.ref_frequency = measures.Frequency(ref_freq, 
                                                measures.FrequencyUnits.HERTZ)
        self.mean_frequency = measures.Frequency(mean_freq, 
                                                 measures.FrequencyUnits.HERTZ)
        self.band = band
        self.type = spw_type
        self.intents = set()
        
        # work around NumPy bug with empty strings
        # http://projects.scipy.org/numpy/ticket/1239
        self.name = str(name)
        self.sideband = str(sideband)
        self.baseband = str(baseband)
    
        self._chan_freqs = chan_freqs
        self._chan_widths = chan_widths
                
        self._min_frequency = min(self.channels, key=lambda r: r.low).low
        self._max_frequency = max(self.channels, key=lambda r: r.high).high
        self._centre_frequency = (self._min_frequency + self._max_frequency) / 2.0

    @property
    def centre_frequency(self):
        return self._centre_frequency

    def channel_range(self, minfreq, maxfreq):
        '''
    	    # More work on this in future 
    	    minfreq -- measures.Frequency object in HERTZ
    	    maxfreq -- measures.Frequency object in HERTZ
    	'''
        freqmin = minfreq
        freqmax = maxfreq
    
        # Check for the no overlap case.
        nchan = self.num_channels
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
    def channels(self):
        if not hasattr(self, '_channels') or self._channels is None:
            channels = []
            for centre, width in zip(self._chan_freqs, self._chan_widths):
                dec_centre = decimal.Decimal(str(centre))
                dec_width= decimal.Decimal(str(width))
                delta = dec_width / decimal.Decimal('2') 
                
                f_lo = measures.Frequency(dec_centre - delta,
                                          measures.FrequencyUnits.HERTZ)
                f_hi = measures.Frequency(dec_centre + delta,
                                          measures.FrequencyUnits.HERTZ)
                channels.append(measures.FrequencyRange(f_lo, f_hi))
            self._channels = channels
        return self._channels

    @property
    def min_frequency(self):
        return self._min_frequency
        
    @property
    def max_frequency(self):
        return self._max_frequency
        
    @property
    def num_channels(self):
        return len(self._chan_freqs)

    def __repr__(self):
        args = map(str, [self.id, self.centre_frequency, self.bandwidth, 
                         self.type])
        return 'SpectralWindow({0})'.format(', '.join(args))

        


class SpectralWindowWithChannelSelection(object):
    """
    SpectralWindowWithChannelSelection decorates a SpectralWindow so that the
    spectral window ID also contains a channel selection.
    """
    def __init__(self, subject, channels):
        self._subject = subject

        channels = sorted(list(channels))

        # prepare a string representation of the number of channels. If all
        # channels are specified for this spw, just specify the spw in the
        # string representation
        if set(channels).issuperset(set(range(0, subject.num_channels))):
            ranges = []
        else:
            ranges = []
            for _, g in itertools.groupby(enumerate(channels),
                                          lambda (i, x):i - x):
                rng = map(operator.itemgetter(1), g)
                if len(rng) is 1:
                    ranges.append('%s' % rng[0])
                else:
                    ranges.append('%s~%s' % (rng[0], rng[-1]))
        self._channels = ';'.join(ranges)

    def __getattr__(self, name):
        return getattr(self._subject, name)

    @property
    def id(self):
        channels = ':%s' % self._channels if self._channels else ''
        return '{spw}{channels}'.format(spw=self._subject.id,
                                        channels=channels)
