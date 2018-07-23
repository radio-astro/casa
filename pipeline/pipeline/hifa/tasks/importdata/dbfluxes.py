from __future__ import absolute_import

import collections
import datetime
import decimal
import ssl
import urllib
import urllib2
from xml.dom import minidom
from xml.parsers.expat import ExpatError

import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.h.tasks.common.commonfluxresults as commonfluxresults
import pipeline.h.tasks.importdata.fluxes as fluxes
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)

SERVICE_URL = 'https://almascience.eso.org/sc/flux'
ORIGIN_DB = 'DB'


def get_setjy_results(mses):
    """
    Get fluxes values from the database service reverting to the Source
    tables XML for backup values and store the values in the context
    """
    results = []
    for ms in mses:
        result = commonfluxresults.FluxCalibrationResults(ms.name)
        science_spw_ids = {spw.id for spw in ms.get_spectral_windows()}

        for source, measurements in read_fluxes_db(ms).iteritems():
            m = [m for m in measurements if int(m.spw_id) in science_spw_ids]

            # import flux values for all fields and intents so that we can
            # compare them to the fluxscale-derived values later in the run
            #            for field in [f for f in source.fields if 'AMPLITUDE' in f.intents]:
            for field in source.fields:
                result.measurements[field.id].extend(m)

        results.append(result)

    return results


def read_fluxes_db(ms):
    """
    Read fluxes from the data base server, defaulting to the Source XML table
    if no fluxes can be found
    """
    xml_measurements = fluxes.read_fluxes_nodb(ms)

    if not xml_measurements:
        # Source.xml could not be read or parsed. Fall back to catalogue query
        return flux_nosourcexml(ms)

    return add_catalogue_fluxes(xml_measurements, ms)


def flux_nosourcexml(ms):
    """
    Call the flux service and get the frequencies from the ms if no Source.xml is available
    """
    result = collections.defaultdict(list)

    for source in ms.sources:
        for spw in ms.get_spectral_windows(science_windows_only=True):
            m = query_online_catalogue(ms, spw, source)
            if m:
                result[source].append(m)
                log_result(source, spw, 'N/A', m.I, m.spix, m.age)

    return result


def fluxservice(service_url, obs_time, frequency, sourcename):
    """
    Usage of this online service requires:
        - service_url - url for the db service
        - obs_time - for getting the date
        - frequency_text - we will get the frequency out of this in Hz
        - source - we will get source.name from this object
    """
    # Example:
    # https://almascience.eso.org/sc/flux?DATE=10-August-2017&FREQUENCY=232101563000.0&NAME=J1924-2914&WEIGHTED=true&RESULT=1
    date = '{!s}-{!s}-{!s}'.format(obs_time.day, obs_time.strftime('%B'), obs_time.year)
    sourcename = sanitize_string(sourcename)
    urlparams = buildparams(sourcename, date, frequency)

    url = '{!s}?{!s}'.format(service_url, urlparams)
    LOG.debug('Querying {!s}'.format(url))

    # ignore HTTPS certificate
    ssl_context = ssl._create_unverified_context()
    try:
        response = urllib2.urlopen(url, context=ssl_context, timeout=10.0)
    except urllib2.URLError:
        LOG.warn('Error contacting ALMA Source Catalogue Database')
        raise

    try:
        dom = minidom.parse(response)
    except ExpatError:
        LOG.warn('Could not parse source catalogue response')
        raise

    rowdict = {}
    for node in dom.getElementsByTagName('TR'):
        row = node.getElementsByTagName('TD')
        rowdict['sourcename'] = row[0].childNodes[0].nodeValue
        rowdict['dbfrequency'] = row[1].childNodes[0].nodeValue
        rowdict['date'] = row[2].childNodes[0].nodeValue
        rowdict['fluxdensity'] = row[3].childNodes[0].nodeValue
        rowdict['fluxdensityerror'] = row[4].childNodes[0].nodeValue
        rowdict['spectralindex'] = row[5].childNodes[0].nodeValue
        rowdict['spectralindexerror'] = row[6].childNodes[0].nodeValue
        rowdict['error2'] = row[7].childNodes[0].nodeValue
        rowdict['error3'] = row[8].childNodes[0].nodeValue
        rowdict['error4'] = row[9].childNodes[0].nodeValue
        rowdict['warning'] = row[10].childNodes[0].nodeValue
        rowdict['notms'] = row[11].childNodes[0].nodeValue
        rowdict['ageOfNearestMonitorPoint'] = row[12].childNodes[0].nodeValue
        rowdict['url'] = url

    return rowdict


def buildparams(name, date, frequency):
    """
    Inputs are all strings with the format:
    NAME=3c279&DATE=04-Apr-2014&FREQUENCY=231.435E9&WEIGHTED=true&RESULT=1
    """
    params = dict(NAME=name, DATE=date, FREQUENCY=frequency, WEIGHTED='true', RESULT=0)
    return urllib.urlencode(params)


def sanitize_string(name):
    """
    Sanitize source name if needed, taking first alias.
    """
    return name.split(';')[0]


def query_online_catalogue(ms, spw, source):
    # At this point we take:
    #  - the source name string
    #  - the frequency of the spw_id in Hz
    #  - The observation date
    #  and attempt to call the online flux catalog web service, and use the flux result
    #  and spectral index
    source_name = source.name
    freq_hz = str(spw.centre_frequency.to_units(measures.FrequencyUnits.HERTZ))
    obs_time = utils.get_epoch_as_datetime(ms.start_time)

    utcnow = datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S UTC")
    try:
        fluxdict = fluxservice(SERVICE_URL, obs_time, freq_hz, source_name)
    except urllib2.URLError:
        # error contacting service
        return None
    except ExpatError:
        # error parsing VOtable
        return None

    try:
        cat_fd = float(fluxdict['fluxdensity'])
        cat_spix = float(fluxdict['spectralindex'])
    except ValueError:
        # could not convert 'null' to number. Bad catalogue value.
        return None

    valid_catalogue_val = cat_fd > 0.0 and cat_spix != -1000
    if not valid_catalogue_val:
        return None

    final_I = measures.FluxDensity(cat_fd, measures.FluxDensityUnits.JANSKY)
    final_spix = decimal.Decimal('%0.3f' % cat_spix)
    age_n_m_p = fluxdict['ageOfNearestMonitorPoint']

    return domain.FluxMeasurement(spw.id, final_I, spix=final_spix, origin=ORIGIN_DB, queried_at=utcnow, age=age_n_m_p)


def add_catalogue_fluxes(measurements, ms):
    results = collections.defaultdict(list)

    science_windows = ms.get_spectral_windows(science_windows_only=True)

    for source, xml_measurements in measurements.iteritems():
        for xml_measurement in xml_measurements:
            spw = ms.get_spectral_window(xml_measurement.spw_id)

            # only query database for science windows
            if spw not in science_windows:
                continue

            catalogue_measurement = query_online_catalogue(ms, spw, source)

            if catalogue_measurement:
                # Catalogue doesn't return Q,U,V so adopt Q,U,V from XML
                catalogue_measurement.Q = xml_measurement.Q
                catalogue_measurement.U = xml_measurement.U
                catalogue_measurement.V = xml_measurement.V

                results[source].append(catalogue_measurement)

                # set text for logging statements
                catalogue_I = catalogue_measurement.I
                spix = catalogue_measurement.spix
                age = catalogue_measurement.age

            else:
                # No/invalid catalogue entry, so use Source.XML measurement
                results[source].append(xml_measurement)

                # set text for logging statements
                catalogue_I = 'N/A'
                spix = 'N/A'
                age = 'N/A'

            log_result(source, spw, xml_measurement.I, catalogue_I, spix, age)

    return results


def log_result(source, spw, asdm_I, catalogue_I, spix, age):
    LOG.info('Source: {!s} spw: {!s}    ASDM flux: {!s}    Catalogue flux: {!s} Jy'.format(source.name, spw.id,
                                                                                           asdm_I, catalogue_I))
    LOG.info('         Online catalog Spectral Index: {!s}'.format(spix))
    LOG.info('         ageOfNearestMonitorPoint: {!s}'.format(age))
    LOG.info("---------------------------------------------")
