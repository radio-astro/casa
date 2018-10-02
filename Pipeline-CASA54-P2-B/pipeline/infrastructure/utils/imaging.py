"""
The imaging module contains utility functions used by the imaging tasks.

TODO These utility functions should migrate to hif.tasks.common
"""
import re

from .. import casatools
from .. import logging

LOG = logging.get_logger(__name__)

__all__ = ['chan_selection_to_frequencies', 'freq_selection_to_channels', 'spw_intersect', 'update_sens_dict',
           'update_beams_dict', 'set_nested_dict', 'intersect_ranges', 'merge_ranges', 'equal_to_n_digits']


def _get_cube_freq_axis(img):
    """
    Get image cube frequency axis.

    :param img:
    :return:
    """
    iaTool = casatools.image

    # Get frequency axis
    iaTool.open(img)
    imInfo = iaTool.summary()
    iaTool.close()

    fIndex = imInfo['axisnames'].tolist().index('Frequency')
    refFreq = imInfo['refval'][fIndex]
    deltaFreq = imInfo['incr'][fIndex]
    freqUnit = imInfo['axisunits'][fIndex]
    refPix = imInfo['refpix'][fIndex]
    numPix = imInfo['shape'][fIndex]

    return refFreq, deltaFreq, freqUnit, refPix, numPix


def chan_selection_to_frequencies(img, selection, unit='GHz'):
    """
    Convert channel selection to frequency tuples.

    :param img:
    :param selection:
    :param unit:
    :return:
    """
    frequencies = []
    if selection != '':
        qaTool = casatools.quanta

        # Get frequency axis
        try:
            refFreq, deltaFreq, freqUnit, refPix, numPix = _get_cube_freq_axis(img)
        except:
            LOG.error('No frequency axis found in %s.' % (img))
            return ['NONE']

        for crange in selection.split(';'):
            c0, c1 = map(float, crange.split('~'))
            # Make sure c0 is the lower channel so that the +/-0.5 channel
            # adjustments below go in the right direction.
            if (c1 < c0):
                c0, c1 = c1, c0

            # Convert the channel range (c0-c1) to the corresponding frequency range
            # that spans between the outer edges of this channel range. I.e., from
            # the lower frequency edge of c0 to the upper frequency edge of c1.
            f0 = qaTool.convert({'value': refFreq + (c0 - 0.5 - refPix) * deltaFreq, 'unit': freqUnit}, unit)
            f1 = qaTool.convert({'value': refFreq + (c1 + 0.5 - refPix) * deltaFreq, 'unit': freqUnit}, unit)
            if qaTool.lt(f0, f1):
                frequencies.append((f0['value'], f1['value']))
            else:
                frequencies.append((f1['value'], f0['value']))
    else:
        frequencies = ['NONE']

    return frequencies


def freq_selection_to_channels(img, selection):
    """
    Convert frequency selection to channel tuples.

    :param img:
    :param selection:
    :return:
    """
    channels = []
    if selection != '':
        qaTool = casatools.quanta

        # Get frequency axis
        try:
            refFreq, deltaFreq, freqUnit, refPix, numPix = _get_cube_freq_axis(img)
        except:
            LOG.error('No frequency axis found in %s.' % (img))
            return ['NONE']

        p = re.compile('([\d.]*)(~)([\d.]*)(\D*)')
        for frange in p.findall(selection.replace(';', '')):
            f0 = qaTool.convert('%s%s' % (frange[0], frange[3]), freqUnit)['value']
            f1 = qaTool.convert('%s%s' % (frange[2], frange[3]), freqUnit)['value']
            # It is assumed here that the frequency ranges are given from
            # the lower edge of the lowest frequency channel to the upper
            # edge of the highest frequency channel, while the reference frequency
            # is specified at the center of the reference pixel (channel). To calculate
            # the corresponding channel range, we need to add 0.5 to the lower channel,
            # and subtract 0.5 from the upper channel.
            c0 = (f0 - refFreq) / deltaFreq
            c1 = (f1 - refFreq) / deltaFreq

            # Avoid stepping outside possible channel range
            c0 = max(c0, 0)
            c0 = min(c0, numPix - 1)
            c0 = int(round(c0 + 0.5))
            c0 = max(c0, 0)
            c0 = min(c0, numPix - 1)

            c1 = max(c1, 0)
            c1 = min(c1, numPix - 1)
            c1 = int(round(c1 - 0.5))
            c1 = max(c1, 0)
            c1 = min(c1, numPix - 1)

            if c0 < c1:
                channels.append((c0, c1))
            else:
                channels.append((c1, c0))
    else:
        channels = ['NONE']

    return channels


def spw_intersect(spw_range, line_regions):
    """
    Compute intersect between SPW frequency range and line frequency
    ranges to be excluded.

    :param spw_range:
    :param line_regions:
    :return:
    """
    spw_sel_intervals = []
    for line_region in line_regions:
        if (line_region[0] <= spw_range[0]) and (line_region[1] >= spw_range[1]):
            spw_sel_intervals = []
            spw_range = []
            break
        elif (line_region[0] <= spw_range[0]) and (line_region[1] >= spw_range[0]):
            spw_range = [line_region[1], spw_range[1]]
        elif (line_region[0] >= spw_range[0]) and (line_region[1] < spw_range[1]):
            spw_sel_intervals.append([spw_range[0], line_region[0]])
            spw_range = [line_region[1], spw_range[1]]
        elif line_region[0] >= spw_range[1]:
            spw_sel_intervals.append(spw_range)
            spw_range = []
            break
        elif (line_region[0] >= spw_range[0]) and (line_region[1] >= spw_range[1]):
            spw_sel_intervals.append([spw_range[0], line_region[0]])
            spw_range = []
            break
    if spw_range != []:
        spw_sel_intervals.append(spw_range)

    return spw_sel_intervals


def update_sens_dict(dct, udct):
    """
    Update a sensitivity dictionary. All generic solutions
    tried so far did not do the job. So this method assumes
    an explicit dictionary structure of
    ['<MS name>']['<field name']['<intent>'][<spw>]: {<sensitivity result>}.

    :param dct:
    :param udct:
    :return:
    """
    for msname in udct:
        # Exclude special primary keys that are not MS names
        if msname not in ['recalc', 'robust', 'uvtaper']:
            if msname not in dct:
                dct[msname] = {}
            for field in udct[msname]:
                if field not in dct[msname]:
                    dct[msname][field] = {}
                for intent in udct[msname][field]:
                    if intent not in dct[msname][field]:
                        dct[msname][field][intent] = {}
                    for spw in udct[msname][field][intent]:
                        if spw not in dct[msname][field][intent]:
                            dct[msname][field][intent][spw] = {}
                        dct[msname][field][intent][spw] = udct[msname][field][intent][spw]


def update_beams_dict(dct, udct):
    """
    Update a beams dictionary. All generic solutions
    tried so far did not do the job. So this method assumes
    an explicit dictionary structure of
    ['<field name']['<intent>'][<spwids>]: {<beam>}.

    :param dct:
    :param udct:
    :return:
    """
    for field in udct:
        # Exclude special primary keys that are not MS names
        if field not in ['recalc', 'robust', 'uvtaper']:
            if field not in dct:
                dct[field] = {}
            for intent in udct[field]:
                if intent not in dct[field]:
                    dct[field][intent] = {}
                for spwids in udct[field][intent]:
                    if spwids not in dct[field][intent]:
                        dct[field][intent][spwids] = {}
                    dct[field][intent][spwids] = udct[field][intent][spwids]


def set_nested_dict(dct, keys, value):
    """

    :param dct:
    :param keys:
    :param value:
    :return:
    """
    for key in keys[:-1]:
        dct = dct.setdefault(key, {})
    dct[keys[-1]] = value


def intersect_ranges(ranges):
    """
    Compute intersection of ranges.

    :param ranges:
    :return:
    """
    if len(ranges) == 0:
        return ()
    elif len(ranges) == 1:
        return ranges[0]
    else:
        ref_range = ranges[0]
        for range in ranges[1:]:
            i0 = max(ref_range[0], range[0])
            i1 = min(ref_range[1], range[1])
            if i0 <= i1:
                ref_range = (i0, i1)
            else:
                return ()

        return ref_range


def merge_ranges(ranges):
    """
    Merge overlapping and adjacent ranges and yield the merged ranges
    in order. The argument must be an iterable of pairs (start, stop).

    >>> list(merge_ranges([(5,7), (3,5), (-1,3)]))
    [(-1, 7)]
    >>> list(merge_ranges([(5,6), (3,4), (1,2)]))
    [(1, 2), (3, 4), (5, 6)]
    >>> list(merge_ranges([]))
    []

    (c) Gareth Rees 02/2013

    """
    ranges = iter(sorted(ranges))
    current_start, current_stop = next(ranges)
    for start, stop in ranges:
        if start > current_stop:
            # Gap between segments: output current segment and start a new one.
            yield current_start, current_stop
            current_start, current_stop = start, stop
        else:
            # Segments adjacent or overlapping: merge.
            current_stop = max(current_stop, stop)
    yield current_start, current_stop


def equal_to_n_digits(x, y, numdigits=7):
    """
    Approximate equality check up to a given number of digits.

    :param x:
    :param y:
    :param numdigits:
    :return:
    """
    try:
        numpy.testing.assert_approx_equal(x, y, numdigits)
        return True
    except:
        return False

