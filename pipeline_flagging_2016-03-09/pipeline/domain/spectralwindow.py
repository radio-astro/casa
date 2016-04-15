from __future__ import absolute_import
import collections
import decimal
import itertools
import operator

import numpy

from . import measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


ArithmeticProgression = collections.namedtuple('ArithmeticProgression', 'start delta num_terms')

def expand_ap(ap):
    """
    Expand an ArithmeticProgression back to its original sequence.
    """
    g = itertools.count(ap.start, ap.delta)
    return itertools.islice(g, 0, ap.num_terms)


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
                 '_channels', '_num_channels', '_ref_frequency_frame')

    def __getstate__(self):
        return (self.id, self.band, self.bandwidth, self.type, self.intents,
                self.ref_frequency, self.name, self.baseband, self.sideband,
                self.mean_frequency, self._min_frequency,
                self._max_frequency, self._centre_frequency, self._chan_freqs,
                self._chan_widths, self._num_channels,
                self._ref_frequency_frame)

    def __setstate__(self, state):
        (self.id, self.band, self.bandwidth, self.type, self.intents,
                self.ref_frequency, self.name, self.baseband, self.sideband,
                self.mean_frequency, self._min_frequency,
                self._max_frequency, self._centre_frequency, self._chan_freqs,
                self._chan_widths, self._num_channels,
                self._ref_frequency_frame) = state

    def __repr__(self):
        chan_freqs = self._chan_freqs
        if isinstance(chan_freqs, ArithmeticProgression):
            chan_freqs = numpy.array(list(expand_ap(chan_freqs)))

        chan_widths = self._chan_widths
        if isinstance(chan_widths, ArithmeticProgression):
            chan_widths = numpy.array(list(expand_ap(chan_widths)))

        return 'SpectralWindow({0!r}, {1!r}, {2!r}, {3!r}, {4!r}, {5!r}, {6}, {7}, {8!r}, {9!r}, {10!r})'.format(
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
            self.sideband,
            self.baseband,
            self.band
        )

    def __init__(self, spw_id, name, spw_type, bandwidth, ref_freq, mean_freq,
                 chan_freqs, chan_widths, sideband, baseband, band='Unknown'):
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

        self._chan_freqs = self.__create_arithmetic_progression(chan_freqs)
        self._chan_widths = self.__create_arithmetic_progression(chan_widths)

        channels = self.channels
        self._num_channels = len(chan_freqs)

        self._min_frequency = min(channels, key=lambda r: r.low).low
        self._max_frequency = max(channels, key=lambda r: r.high).high
        self._centre_frequency = (self._min_frequency + self._max_frequency) / 2.0

    def __create_arithmetic_progression(self, values):
        """
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
    def channels(self):
        if not hasattr(self, '_channels') or self._channels is None:
            channels = []

            chan_freqs = self._chan_freqs
            if isinstance(chan_freqs, ArithmeticProgression):
                chan_freqs = expand_ap(chan_freqs)

            chan_widths = self._chan_widths
            if isinstance(chan_widths, ArithmeticProgression):
                chan_widths = expand_ap(chan_widths)

            for centre, width in zip(chan_freqs, chan_widths):
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
        return self._num_channels

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
