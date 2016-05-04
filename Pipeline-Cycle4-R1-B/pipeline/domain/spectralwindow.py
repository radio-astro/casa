from __future__ import absolute_import
import decimal
import itertools
import operator

import numpy

from . import measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


class ArithmeticProgression(object):
    """
    A representation of an arithmetic progression that can generate sequence
    elements on demand.
    """
    __slots__ = ('start', 'delta', 'num_terms')

    def __getstate__(self):
        return self.start, self.delta, self.num_terms

    def __setstate__(self, state):
        self.start, self.delta, self.num_terms = state

    def __init__(self, start, delta, num_terms):
        self.start = start
        self.delta = delta
        self.num_terms = num_terms

    def __iter__(self):
        g = itertools.count(self.start, self.delta)
        return itertools.islice(g, 0, self.num_terms)

    def __len__(self):
        return self.num_terms

    def __getitem__(self, index):
        if abs(index) >= self.num_terms:
            raise IndexError
        if index < 0:
            index += self.num_terms
        return self.start + index * self.delta


def compress(values):
    """
    Compress (if possible) a sequence of values.

    If the numbers in the given list constitute an arithmetic progression,
    return an ArithmeticProgression object that summarises it as such. If
    the list cannot be summarised as a simple arithmetic progression,
    return the list as given.
    """
    deltas = set(numpy.diff(values))
    if len(deltas) is 1:
        delta = deltas.pop()
        return ArithmeticProgression(values[0], delta, len(values))
    else:
        return values


class ChannelList(object):
    """
    A container/generator for Channel objects.

    A spectral window can contain thousands of channels. Rather than store all of
    these objects, a ChannelList generates and returns them lazily, on-demand.
    """
    def __init__(self, chan_freqs, chan_widths, effbw):
        assert len(chan_freqs) == len(chan_widths) == len(effbw)
        self.chan_freqs = chan_freqs
        self.chan_widths = chan_widths
        self.chan_effbws = effbw

    def __iter__(self):
        raw_channel_data = zip(self.chan_freqs, self.chan_widths, self.chan_effbws)
        for chan_centre, chan_width, chan_effective_bw in raw_channel_data:
            yield self.__create_channel(chan_centre, chan_width, chan_effective_bw)

    def __len__(self):
        return len(self.chan_freqs)

    def __getitem__(self, index):
        return self.__create_channel(self.chan_freqs[index],
                                     self.chan_widths[index],
                                     self.chan_effbws[index])

    def __create_channel(self, centre, width, effective_bw):
        dec_centre = decimal.Decimal(str(centre))
        dec_width = decimal.Decimal(str(width))
        delta = dec_width / decimal.Decimal('2')

        f_lo = measures.Frequency(dec_centre - delta,
                                  measures.FrequencyUnits.HERTZ)
        f_hi = measures.Frequency(dec_centre + delta,
                                  measures.FrequencyUnits.HERTZ)
        f_bw = measures.Frequency(effective_bw,
                                  measures.FrequencyUnits.HERTZ)
        return Channel(f_lo, f_hi, f_bw)


class Channel(object):
    """
    Representation of a channel within a spectral window.

    This object can be considered as a FrequencyRange object plus an effective
    bandwidth property. It provides the same interface as a FrequencyRange.
    """
    __slots__ = ('frequency_range', 'effective_bw')

    def __getstate__(self):
        return self.frequency_range, self.effective_bw

    def __setstate__(self, state):
        self.frequency_range, self.effective_bw = state

    def __init__(self, start, end, effective_bw):
        self.frequency_range = measures.FrequencyRange(start, end)
        self.effective_bw = effective_bw

    def __eq__(self, other):
        if not isinstance(other, Channel):
            return False

        if other.frequency_range != self.frequency_range:
            return False
        if other.effective_bw != self.effective_bw:
            return False

        return True

    def __ne__(self, other):
        return not self.__eq__(other)

    def __repr__(self):
        return 'Channel(%s, %s, %s)' % (self.frequency_range.low,
                                        self.frequency_range.high,
                                        self.effective_bw)

    @property
    def low(self):
        return self.frequency_range.low

    @property
    def high(self):
        return self.frequency_range.high

    def contains(self, frequency):
        return self.frequency_range.contains(frequency)

    def convert_to(self, newUnits=measures.FrequencyUnits.GIGAHERTZ):
        return self.frequency_range.convert_to(newUnits)

    def getCentreFrequency(self):
        return self.frequency_range.getCentreFrequency()

    def getOverlapWith(self, other):
        return self.frequency_range.getOverlapWith(other)

    def getGapBetween(self, other):
        return self.frequency_range.getGapBetween(other)

    def getWidth(self):
        return self.frequency_range.getWidth()

    def overlaps(self, other):
        return self.frequency_range.overlaps(other)

    def set(self, frequency1, frequency2):
        self.frequency_range.set(frequency1, frequency2)


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
                 '_centre_frequency', 'channels', '_ref_frequency_frame')

    def __getstate__(self):
        return (self.id, self.band, self.bandwidth, self.type, self.intents,
                self.ref_frequency, self.name, self.baseband, self.sideband,
                self.mean_frequency, self._min_frequency, self._max_frequency,
                self._centre_frequency, self.channels,
                self._ref_frequency_frame)

    def __setstate__(self, state):
        (self.id, self.band, self.bandwidth, self.type, self.intents,
                self.ref_frequency, self.name, self.baseband, self.sideband,
                self.mean_frequency, self._min_frequency, self._max_frequency,
                self._centre_frequency, self.channels,
                self._ref_frequency_frame) = state

    def __repr__(self):
        chan_freqs = self.channels.chan_freqs
        if isinstance(chan_freqs, ArithmeticProgression):
            chan_freqs = numpy.array(list(chan_freqs))

        chan_widths = self.channels.chan_widths
        if isinstance(chan_widths, ArithmeticProgression):
            chan_widths = numpy.array(list(chan_widths))

        chan_effective_bws = self.channels.chan_effbws
        if isinstance(chan_effective_bws, ArithmeticProgression):
            chan_effective_bws = numpy.array(list(chan_effective_bws))

        return 'SpectralWindow({0!r}, {1!r}, {2!r}, {3!r}, {4!r}, {5!r}, {6}, {7}, {8}, {9!r}, {10!r}, {11!r})'.format(
            self.id,
            self.name,
            self.type,
            float(self.bandwidth.to_units(measures.FrequencyUnits.HERTZ)),
            dict(m0={'unit': 'Hz',
                     'value': float(self.ref_frequency.to_units(measures.FrequencyUnits.HERTZ))},
                 refer=self._ref_frequency_frame,
                 type='frequency'),
            float(self.mean_frequency.to_units(measures.FrequencyUnits.HERTZ)),
            'numpy.array(%r)' % chan_freqs.tolist(),
            'numpy.array(%r)' % chan_widths.tolist(),
            'numpy.array(%r)' % chan_effective_bws.tolist(),
            self.sideband,
            self.baseband,
            self.band
        )

    def __init__(self, spw_id, name, spw_type, bandwidth, ref_freq, mean_freq,
                 chan_freqs, chan_widths, chan_effective_bws, sideband,
                 baseband, band='Unknown'):
        self.id = spw_id
        self.bandwidth = measures.Frequency(bandwidth,
                                            measures.FrequencyUnits.HERTZ)

        ref_freq_hz = casatools.quanta.convertfreq(ref_freq['m0'], 'Hz')
        ref_freq_val = casatools.quanta.getvalue(ref_freq_hz)[0]
        self.ref_frequency = measures.Frequency(ref_freq_val,
                                                measures.FrequencyUnits.HERTZ)
        self._ref_frequency_frame = ref_freq['refer']

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

        chan_freqs = compress(chan_freqs)
        chan_widths = compress(chan_widths)
        chan_effective_bws = compress(chan_effective_bws)
        self.channels = ChannelList(chan_freqs, chan_widths, chan_effective_bws)

        self._min_frequency = min(self.channels, key=lambda r: r.low).low
        self._max_frequency = max(self.channels, key=lambda r: r.high).high
        self._centre_frequency = (self._min_frequency + self._max_frequency) / 2.0

    @property
    def centre_frequency(self):
        return self._centre_frequency

    def channel_range(self, minfreq, maxfreq):
        """
        # More work on this in future
        minfreq -- measures.Frequency object in HERTZ
        maxfreq -- measures.Frequency object in HERTZ
        """
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
    def frame(self):
        return self._ref_frequency_frame

    @property
    def min_frequency(self):
        return self._min_frequency

    @property
    def max_frequency(self):
        return self._max_frequency

    @property
    def num_channels(self):
        return len(self.channels)

    def __str__(self):
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
