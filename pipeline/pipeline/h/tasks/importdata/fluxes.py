from __future__ import absolute_import

import collections
import csv
import decimal
import itertools
import operator
import os
import re
import string
import xml.etree.ElementTree as ElementTree
from datetime import datetime
from functools import reduce

import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
from ..common import commonfluxresults

LOG = infrastructure.get_logger(__name__)

ORIGIN_XML = 'Source.xml'
ORIGIN_ANALYSIS_UTILS = 'analysisUtils'


def get_setjy_results(mses):
    """
    Get the flux results from the ASDM Source.xml file and
    populate the context.
    """
    results = []
    for ms in mses:
        result = commonfluxresults.FluxCalibrationResults(ms.name)
        science_spw_ids = [spw.id for spw in ms.get_spectral_windows()]

        for source, measurements in read_fluxes_nodb(ms).items():
            m = [m for m in measurements if int(m.spw_id) in science_spw_ids]

            # import flux values for all fields and intents so that we can
            # compare them to the fluxscale-derived values later in the run
            #            for field in [f for f in source.fields if 'AMPLITUDE' in f.intents]:
            for field in source.fields:
                result.measurements[field.id].extend(m)

        results.append(result)

    return results


def read_fluxes_nodb(ms):
    """
    Read fluxes from the Source XML table translating from the ASDM
    to MS spw ids as we go.
    """
    result = collections.defaultdict(list)

    source_table = os.path.join(ms.name, 'Source.xml')
    if not os.path.exists(source_table):
        LOG.info('No Source XML found at {}. No flux import performed. '.format(source_table))
        return result

    source_element = ElementTree.parse(source_table)
    if not source_element:
        LOG.info('Could not parse Source XML at {}. No flux import performed.'.format(source_table))
        return result

    # Empty spws that follow non-empty spws can be pruned from MMS data. This
    # set is used to check whether the Source.xml entries refer to spws
    # actually present in the measurement set.
    all_ms_spw_ids = {spw.id for spw in ms.spectral_windows}

    # SCIREQ-852: MS spw IDs != ASDM spw ids
    asdm_to_ms_spw_map = get_asdm_to_ms_spw_mapping(ms)

    for row in source_element.findall('row'):

        # Get the elements
        flux_text = row.findtext('flux')
        frequency_text = row.findtext('frequency')
        source_element = row.findtext('sourceId')
        spw_element = row.findtext('spectralWindowId')
        if spw_element is None or source_element is None:
            continue

        # spws can overlap, so rather than looking up spw by frequency,
        # extract the spw id from the element text. I assume the format uses
        # underscores, eg. 'SpectralWindow_13'
        _, asdm_spw_id = string.split(spw_element, '_')

        # SCIREQ-852: MS spw IDs != ASDM spw ids
        spw_id = asdm_to_ms_spw_map.get(int(asdm_spw_id), None)
        if spw_id not in all_ms_spw_ids:
            LOG.warning('Could not map ASDM spectral window {} to MS for {}'.format(asdm_spw_id, ms.basename))
            continue

        source_id = int(source_element)
        if source_id >= len(ms.sources):
            LOG.warning('Source.xml refers to source #{}, which was not found in {}'.format(source_id, ms.basename))
            continue
        source = ms.sources[int(source_id)]

        # all elements must contain data to proceed
        if flux_text is None or frequency_text is None:
            continue

        # See what elements can be used
        try:
            if spw_id and frequency_text is None:
                spw = ms.get_spectral_windows(spw_id)
                frequency = str(spw[0].centre_frequency.value)
        except:
            continue

        # Get the measurement
        m = get_measurement(ms, spw_id, frequency_text, flux_text)

        result[source].append(m)

    return result


def get_measurement(ms, spw, frequency_text, flux_text):
    """
    Construct the measurement
    """

    # more than one measurement can be registered against the spectral
    # window. These functions give a lists of frequencies and IQUV
    # 4-tuples
    row_frequencies = to_hertz(frequency_text)
    row_iquvs = to_jansky(flux_text)
    # zip to give a list of (frequency, [I,Q,U,V]) tuples
    zipped = zip(row_frequencies, row_iquvs)

    spw = ms.get_spectral_window(spw)

    # Task: select flux measurement closest to spectral window centre
    # frequency, taking the mean when measurements are equally distant

    # first, sort the measurements by distance to spw centre
    # frequency, annotating each tuple with the delta frequency
    by_delta = sorted([(abs(spw.centre_frequency - f), f, iquv) for f, iquv in zipped])

    # identify all measurements equally as close as this provisionally
    # 'closest' measurement
    min_delta, closest_frequency, _ = by_delta[0]
    joint_closest = [iquv for delta_f, _, iquv in by_delta if delta_f == min_delta]

    if len(joint_closest) > 1:
        LOG.trace('Averaging {} equally close measurements: {}'.format(len(joint_closest), joint_closest))

    # calculate the mean of these equally distant  measurements.
    # joint_closest has at least one item, so we don't need to prime
    # the reduce function with an empty accumulator
    mean_iquv = [reduce(lambda x, y: x + y, stokes) / len(joint_closest) for stokes in zip(*joint_closest)]

    LOG.info('Closest flux measurement for {} spw {} found {} distant from centre of spw)'
             ''.format(ms.basename, spw, min_delta))

    # Even if a mean was calculated, any alternative selection should
    # be equally distant and therefore outside the sow range too
    if not spw.min_frequency < closest_frequency < spw.max_frequency:
        # This might become a warning once the PRTSPR-20823 fix is active
        LOG.info('Closest flux measurement for {} spw {} falls outside spw, {} distant from spectral window centre'
                 ''.format(ms.basename, spw, min_delta))

    m = domain.FluxMeasurement(spw.id, *mean_iquv, origin=ORIGIN_XML)

    return m


def to_jansky(flux_text):
    """
    Convert a string extracted from an ASDM XML element to FluxDensity domain
    objects.
    """
    flux_fn = lambda f: measures.FluxDensity(float(f), measures.FluxDensityUnits.JANSKY)
    return get_atoms(flux_text, flux_fn)


def to_hertz(freq_text):
    """
    Convert a string extracted from an ASDM XML element to Frequency domain
    objects.
    """
    freq_fn = lambda f: measures.Frequency(float(f), measures.FrequencyUnits.HERTZ)
    return get_atoms(freq_text, freq_fn)


def get_atoms(text, conv_fn=lambda x: x):
    """
    Get the individual measurements from an ASDM element.

    This function converts a CASA record from a linear space-separated string
    into a multidimensional list, using the dimension headers given at the
    start of the CASA record to determine the number and size of each
    dimension.

    text - text from an ASDM element, with space-separated values
    fn - optional function converting a string to a user-defined type
    """
    values = string.split(text)
    # syntax is <num dimensions> <size dimension 1> <size dimension 2> etc.
    num_dimensions = int(values[0])
    dimension_sizes = map(int, values[1:num_dimensions + 1])

    # find how may values are needed to form one complete 'entity'
    step_size = reduce(operator.mul, dimension_sizes)
    # idx holds the index of the first value for each entity
    idx = len(dimension_sizes) + 1

    results = []
    while idx < len(values):
        # get our complete entity as a linear list of strings, ready to be
        # parcelled up into dimensions
        data = values[idx:idx + step_size]
        # convert the values using the given function, eg. from string to Jy
        data = map(conv_fn, data)
        # group the values into dimensions using the sizes in the header
        for s in dimension_sizes[-1:0:-1]:
            data = list(grouper(s, data))
        results.extend(data)
        idx = idx + step_size

    return results


def grouper(n, iterable, fillvalue=None):
    """
    grouper(3, 'ABCDEFG', 'x') --> ABC DEF Gxx
    """
    args = [iter(iterable)] * n
    return itertools.izip_longest(fillvalue=fillvalue, *args)


def CYCLE7_export_flux_from_result(results, context, filename='flux.csv'):
    """
    Export flux densities from a set of results to a CSV file.

    This function was reverted because it came too late to the C6 deadline
    for analysisUtils to match the new format. It should be committed during
    C7 development.
    """
    if not isinstance(results, list):
        results = [results, ]
    abspath = os.path.join(context.output_dir, filename)

    columns = ['ms', 'field', 'spw', 'I', 'Q', 'U', 'V', 'spix', 'origin', 'query_date', 'age', 'comment']
    existing = []

    # if the file exists, read it in
    if os.path.exists(abspath):
        with open(abspath, 'r') as f:
            # slurp in all but the header rows
            existing.extend([l for l in f.readlines() if not l.startswith(','.join(columns))])

    # so we can write it back out again, with our measurements appended
    with open(abspath, 'wt') as f:
        writer = csv.writer(f)
        writer.writerow(columns)
        f.writelines(existing)

        counter = 0
        for setjy_result in results:
            ms_name = setjy_result.vis
            ms_basename = os.path.basename(ms_name)
            for field_id, measurements in setjy_result.measurements.items():
                for m in measurements:

                    prefix = '%s,%s,%s' % (ms_basename, field_id, m.spw_id)
                    for row in existing:
                        if row.startswith(prefix):
                            LOG.info('Not overwriting flux data for %s field %s '
                                     'spw %s in %s' % (ms_basename, field_id,
                                                       m.spw_id,
                                                       os.path.basename(abspath)))
                            break

                    else:
                        (I, Q, U, V) = m.casa_flux_density

                        ms = context.observing_run.get_ms(ms_basename)
                        field = ms.get_fields(field_id)[0]
                        comment = "\'" + utils.dequote(field.name) + "\'" + ' ' + 'intent=' + ','.join(
                            sorted(field.intents))

                        writer.writerow((ms_basename, field_id, m.spw_id, I, Q, U, V, float(m.spix), m.origin,
                                         m.queried_at, m.age, comment))
                        counter += 1

        LOG.info('Exported %s flux measurements to %s' % (counter, abspath))


def export_flux_from_result(results, context, filename='flux.csv'):
    """
    Export flux densities from a set of results to a CSV file.
    """
    if not isinstance(results, list):
        results = [results, ]
    abspath = os.path.join(context.output_dir, filename)

    columns = ['ms', 'field', 'spw', 'I', 'Q', 'U', 'V', 'spix', 'comment']
    existing = []

    # if the file exists, read it in
    if os.path.exists(abspath):
        with open(abspath, 'r') as f:
            # slurp in all but the header rows
            existing.extend([l for l in f.readlines() if not l.startswith(','.join(columns))])

    # so we can write it back out again, with our measurements appended
    comment_template = '# field={field} intents={intents} origin={origin} age={age} queried_at={queried_at}'
    with open(abspath, 'wt') as f:
        writer = csv.writer(f)
        writer.writerow(columns)
        f.writelines(existing)

        counter = 0
        for setjy_result in results:
            ms_name = setjy_result.vis
            ms_basename = os.path.basename(ms_name)
            for field_id, measurements in setjy_result.measurements.items():
                for m in measurements:

                    prefix = '%s,%s,%s' % (ms_basename, field_id, m.spw_id)
                    for row in existing:
                        if row.startswith(prefix):
                            LOG.info('Not overwriting flux data for {} field {} spw {}'
                                     ''.format(ms_basename, field_id, m.spw_id))
                            break

                    else:
                        (I, Q, U, V) = m.casa_flux_density

                        ms = context.observing_run.get_ms(ms_basename)
                        field = ms.get_fields(field_id)[0]

                        origin = m.origin if m.origin else 'N/A'
                        age = m.age if m.age else 'N/A'
                        queried_at = m.queried_at if m.queried_at else 'N/A'

                        comment = comment_template.format(field=field.clean_name,
                                                          intents=','.join(sorted(field.intents)), origin=origin,
                                                          age=age, queried_at=queried_at)

                        writer.writerow([ms_basename, field_id, m.spw_id, I, Q, U, V, float(m.spix), comment])
                        counter += 1

        LOG.info('Exported %s flux measurements to %s' % (counter, abspath))


def import_flux(output_dir, observing_run, filename=None):
    """
    Read flux densities from a CSV file and import them into the context.
    """
    # regular expressions to match values from comment template
    origin_re = re.compile('(?:origin=)(?P<origin>\S+)')
    age_re = re.compile('(?:age=)(?P<age>\S+)')
    query_re = re.compile('(?:queried_at=)(?P<timestamp>\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2} \w{3})')

    if not filename:
        filename = os.path.join(output_dir, 'flux.csv')

    with open(filename, 'rt') as f:
        reader = csv.DictReader(f, restkey='others', restval=None)

        counter = 0
        for row in reader:
            ms_name = row['ms']
            try:
                ms = observing_run.get_ms(ms_name)
            except KeyError:
                # No MS registered by that name. This could be caused by a
                # flux.csv from a previous run
                LOG.info('{} refers to unregistered ASDM \'{}\'. If this is a multi-ASDM run this to be expected.'
                         ''.format(filename, ms_name))
                continue

            field_id = int(row['field'])
            spw_id = int(row['spw'])
            I = row['I']
            Q = row['Q']
            U = row['U']
            V = row['V']

            try:
                spix = decimal.Decimal(row['spix'])
            except decimal.InvalidOperation:
                spix = decimal.Decimal('0.0')

            comment = row['comment']

            match = origin_re.search(comment)
            origin = match.group('origin') if match else None
            if origin == 'N/A':
                origin = None
            if 'Jy, freq=' in comment:
                origin = ORIGIN_ANALYSIS_UTILS

            match = age_re.search(comment)
            age = match.group('age') if match else None
            if age:
                try:
                    age = int(age)
                except ValueError:
                    age = None

            match = query_re.search(comment)
            query_date = match.group('timestamp') if match else None
            if query_date:
                try:
                    query_date = datetime.strptime(query_date, '%Y-%m-%d %H:%M:%S %Z')
                except TypeError:
                    query_date = None

            try:
                fields = ms.get_fields(field_id)
                measurement = domain.FluxMeasurement(spw_id, I, Q, U, V, spix, origin=origin, age=age, queried_at=query_date)

                # A single field identifier could map to multiple field objects,
                # but the flux should be the same for all, so we iterate..
                for field in fields:
                    # .. removing any existing measurements in these spws from
                    # these fields..
                    map(field.flux_densities.remove,
                        [m for m in field.flux_densities if m.spw_id is spw_id])

                    # .. and then updating with our new values
                    LOG.trace('Adding {} to spw {}'.format(measurement, spw_id))
                    field.flux_densities.add(measurement)
                    counter += 1
            except Exception as e:
                LOG.debug(e)
                LOG.warning('Problem importing \'{}\' as a flux statement'.format(row))

        LOG.info('Imported {} flux measurements from {}'.format(counter, filename))

        # Convert into a set of results for the web log
        results = []
        for ms in observing_run.measurement_sets:
            science_spw_ids = [spw.id for spw in ms.get_spectral_windows(science_windows_only=True)]
            result = commonfluxresults.FluxCalibrationResults(ms.name)
            for field in ms.get_fields():
                if field.flux_densities is None:
                    continue
                for flux in field.flux_densities:
                    if flux.spw_id not in science_spw_ids:
                        continue
                    # Important! The field ID is used rather than the field
                    # name as some data describe independent fields using the
                    # by the same name, e.g., J1733-1304 is both a science
                    # target and phase calibrator in
                    # uid://A002/Xc2ae09/X27f.ms.
                    #
                    # More info:
                    # https://open-jira.nrao.edu/browse/CAS-10792?focusedCommentId=123387&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-123387
                    result.measurements[field.id].append(flux)
            results.append(result)
        return results


def parse_spectral_window_ids_from_xml(xml_path):
    """
    Extract the spectral window ID element from each row of an XML file.

    :param xml_path: path for XML file
    :return: list of integer spectral window IDs
    """
    root_element = ElementTree.parse(xml_path)

    ids = []
    for row in root_element.findall('row'):
        element = row.findtext('spectralWindowId')
        _, str_id = string.split(element, '_')
        ids.append(int(str_id))

    return ids


def get_data_description_spw_ids(ms):
    """
    Extract a list of spectral window IDs from the DataDescription XML for an
    ASDM.

    This function assumes the XML has been copied across to the measurement
    set directory.

    :param ms: measurement set to inspect
    :return: list of integers corresponding to ASDM spectral window IDs
    """
    xml_path = os.path.join(ms.name, 'DataDescription.xml')
    return parse_spectral_window_ids_from_xml(xml_path)


def get_spectral_window_spw_ids(ms):
    """
    Extract a list of spectral window IDs from the SpectralWindow XML for an
    ASDM.

    This function assumes the XML has been copied across to the measurement
    set directory.

    :param ms: measurement set to inspect
    :return: list of integers corresponding to ASDM spectral window IDs
    """
    xml_path = os.path.join(ms.name, 'SpectralWindow.xml')
    return parse_spectral_window_ids_from_xml(xml_path)


def get_asdm_to_ms_spw_mapping(ms):
    """
    Get the mapping of ASDM spectral window ID to Measurement Set spectral
    window ID.

    This function requires the SpectralWindow and DataDescription ASDM XML
    files to have been copied across to the measurement set directory.

    :param ms: measurement set to inspect
    :return: dict of ASDM spw: MS spw
    """
    dd_spws = get_data_description_spw_ids(ms)
    spw_spws = get_spectral_window_spw_ids(ms)
    asdm_ids = [i for i in spw_spws if i in dd_spws] + [i for i in spw_spws if i not in dd_spws]
    return {k: v for k, v in zip(asdm_ids, spw_spws)}
