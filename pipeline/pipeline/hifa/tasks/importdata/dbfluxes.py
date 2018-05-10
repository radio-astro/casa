from __future__ import absolute_import
import collections
import os
import string
import datetime
import decimal
import urllib
import urllib2
import xml.etree.ElementTree as ElementTree
from xml.dom import minidom

import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

import pipeline.h.tasks.importdata.fluxes as fluxes
import pipeline.h.tasks.common.commonfluxresults as commonfluxresults

LOG = infrastructure.get_logger(__name__)

def get_setjy_results(mses):
    """
    Get fluxes values from the database service reverting to the Source
    tables XML for backup values and store the values in the context
    """
    results = []
    for ms in mses:
        result = commonfluxresults.FluxCalibrationResults(ms.name)
        science_spw_ids = [spw.id for spw in ms.get_spectral_windows()]

        for source, measurements in read_fluxes_db(ms).items():
            m = [m for m in measurements if int(m.spw_id) in science_spw_ids]

            # import flux values for all fields and intents so that we can
            # compare them to the fluxscale-derived values later in the run
            #            for field in [f for f in source.fields if 'AMPLITUDE' in f.intents]:
            for field in source.fields:
                result.measurements[field.id].extend(m)

        results.append(result)

    return result


def read_fluxes_db(ms):
    """
    Read fluxes from the data base server defaulting to the Source XML table
    if not fluxes can be found
    """
    result = collections.defaultdict(list)

    science_spw_ids = [spw.id for spw in ms.get_spectral_windows()]

    source_table = os.path.join(ms.name, 'Source.xml')
    if not os.path.exists(source_table):
        LOG.info('No Source XML found at %s. No flux import performed. '
                 'Attempting database query.', source_table)
        return flux_nosourcexml(ms)

    source_element = ElementTree.parse(source_table)
    if not source_element:
        LOG.info('Could not parse Source XML at %s. No flux import performed. '
                 'Attempting database query.', source_table)
        return flux_nosourcexml(ms)

    # Empty spws that follow non-empty spws can be pruned from MMS data. This
    # set is used to check whether the Source.xml entries refer to spws
    # actually present in the measurement set.
    all_ms_spw_ids = {spw.id for spw in ms.spectral_windows}

    # SCIREQ-852: MS spw IDs != ASDM spw ids
    asdm_to_ms_spw_map = fluxes.get_asdm_to_ms_spw_mapping(ms)

    for row in source_element.findall('row'):
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
            LOG.warning('Could not map ASDM spectral window {!s} to MS '
                        'for {!s}'.format(asdm_spw_id, ms.basename))
            continue
        elif spw_id not in science_spw_ids:
            continue

        source_id = int(source_element)
        if source_id >= len(ms.sources):
            LOG.warning('Source.xml refers to source #{!s}, which was not '
                        'found in {!s}'.format(source_id, ms.basename))
            continue
        source = ms.sources[int(source_id)]

        # all elements must contain data to proceed
        if flux_text is not None and frequency_text is not None:
            msource = fluxes.get_measurement (ms, spw_id, frequency_text, flux_text)
        else:
            msource = None

        source_name = source.name
        asdmmessage = ''
        spw = ms.get_spectral_window(spw_id)
        frequency = str(spw.centre_frequency.value)

        # At this point we take:
        #  - the source name string
        #  - the frequency of the spw_id in Hz
        #  - The observation date
        #  and attempt to call the online flux catalog web service, and use the flux result
        #  and spectral index
        utcnow = datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S UTC")
        try:
            serviceurl = 'https://almascience.eso.org/sc/flux'
            fluxdict = fluxservice(serviceurl, ms, frequency, source_name)
            f = fluxdict['fluxdensity']
            spix = fluxdict['spectralindex']
            ageNMP = fluxdict['ageOfNearestMonitorPoint']
            origin = ('Source.xml','','N/A')

            # Filter for problem values.
            #    If there are problem values revert to the ASDM values

            if float(f) <= 0.0 or str(f) == 'Infinity' or str(spix) == '-1000':
                # Use ASDM values if any
                if msource is not None:
                    if int(spw_id) in science_spw_ids:
                        asdmmessage = "   ASDM Flux: {!s}".format(str(msource.I.value))
                        LOG.info("Source: {!s} spw: {!s} {!s}    Online catalog Flux: {!s} Jy"
                                 "".format(source_name, spw_id, asdmmessage, f))
                        LOG.info("         Online catalog Spectral Index: {!s}".format(str(spix)))
                        LOG.info("         Unusable online catalog information.")
                        LOG.info("---------------------------------------------")
                        origin = ('Source.xml','','N/A')
                    m = msource
                else:
                    if int(spw_id) in science_spw_ids:
                        asdmmessage = "  No ASDM Flux"
                        LOG.info("Source: {!s} spw: {!s} {!s}    Online catalog Flux: {!s} Jy"
                             "".format(source_name, spw_id, asdmmessage, f))
                        LOG.info("         Online catalog Spectral Index: {!s}".format(str(spix)))
                        LOG.info("         Unusable online catalog information.")
                        LOG.info("---------------------------------------------")
                        origin = ('Source.xml','','N/A')
                    continue
            else:
                if msource is None:
                    if int(spw_id) in science_spw_ids:
                        asdmmessage = "  No ASDM Flux"
                        LOG.info("Source: {!s} spw: {!s} {!s}    Online catalog Flux: {!s} Jy"
                            "".format(source_name, spw_id, asdmmessage, f))
                        LOG.info("         Online catalog Spectral Index: {!s}".format(str(spix)))
                        LOG.info("         ageOfNearestMonitorPoint: {!s}".format(str(ageNMP)))
                    iquv_db = (measures.FluxDensity(float(f), measures.FluxDensityUnits.JANSKY),
                        measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY),
                        measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY),
                        measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY))
                    origin = ('DB query ', '{!s}'.format(utcnow), str(ageNMP))
                # Use ASDM polarization values if any
                else:
                    if int(spw_id) in science_spw_ids:
                        asdmmessage = "   ASDM Flux: {!s}".format(str(msource.I.value))
                        LOG.info("Source: {!s} spw: {!s} {!s}    Online catalog Flux: {!s} Jy"
                            "".format(source_name, spw_id, asdmmessage, f))
                        LOG.info("         Online catalog Spectral Index: {!s}".format(str(spix)))
                        LOG.info("         ageOfNearestMonitorPoint: {!s}".format(str(ageNMP)))
                    # iquv_db = (measures.FluxDensity(float(f), measures.FluxDensityUnits.JANSKY),
                    iquv_db = (measures.FluxDensity(float(f), measures.FluxDensityUnits.JANSKY),
                        measures.FluxDensity(float(msource.Q.value), measures.FluxDensityUnits.JANSKY),
                        measures.FluxDensity(float(msource.U.value), measures.FluxDensityUnits.JANSKY),
                        measures.FluxDensity(float(msource.V.value), measures.FluxDensityUnits.JANSKY))
                    origin = ('DB query ', '{!s}'.format(utcnow), str(ageNMP))

                m = domain.FluxMeasurement(spw_id, *iquv_db, spix=decimal.Decimal('%0.3f' % float(spix)),
                                           origin=origin)

        except:

            if None in (source_id, spw_id, flux_text, frequency_text):
                # If both Source.xml AND online flux were a no-go then continue
                continue
            else:
                # Use Source.xml values since nothing was returned from the online database
                m = msource
                if (int(spw_id) in science_spw_ids):
                    LOG.info("Source: " + source_name + " spw: " + str(spw_id) + "    ASDM Flux: " + str(
                        msource.I.value) + "     No online catalog information.")

        result[source].append(m)

    return result

def flux_nosourcexml(ms):
    """
    Call the flux service and get the frequencies from the ms if no Source.xml is available
    """
    result = collections.defaultdict(list)

    spws = ms.get_spectral_windows()

    for source in ms.sources:
        for spw in spws:
            sourcename = source.name
            frequency = str(spw.centre_frequency.value)
            spw_id = spw.id
            LOG.info('freq/sourcename:  ' + str(frequency) + str(sourcename))

            try:

                serviceurl = 'https://almascience.eso.org/sc/flux'
                fluxdict = fluxservice(serviceurl, ms, frequency, sourcename)

                f = fluxdict['fluxdensity']
                spix = fluxdict['spectralindex']
                ageNMP = fluxdict['ageOfNearestMonitorPoint']

                iquv_db = (measures.FluxDensity(float(f), measures.FluxDensityUnits.JANSKY),
                       measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY),
                       measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY),
                       measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY))
                utcnow = datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S UTC")
                m = domain.FluxMeasurement(spw_id, *iquv_db,
                                           spix=decimal.Decimal('%0.3f' % float(spix)),
                                           origin=('DB query ','{!s}'.format(utcnow), str(ageNMP)))
                result[source].append(m)
            except:
                LOG.debug("    No flux catalog values for source " + str(source.name) + "  spw:" + str(spw_id))

    return result


def fluxservice(serviceurl, ms, frequency, sourcename):
    """
    Usage of this online service requires:
        - serviceurl - url for the db service
        - ms - for getting the date
        - frequency_text - we will get the frequency out of this in Hz
        - source - we will get source.name from this object
    """
    # serviceurl = 'http://bender.csrg.cl:2121/bfs-0.2/ssap'
    # serviceurl =  'http://asa-test.alma.cl/bfs/'
    # serviceurl = 'https://almascience.eso.org/sc/flux'
    # Example:
    # https://almascience.eso.org/sc/flux?DATE=10-August-2017&FREQUENCY=232101563000.0&NAME=J1924-2914&WEIGHTED=true&RESULT=1

    qt = casatools.quanta
    mt = casatools.measures
    s = qt.time(mt.getvalue(ms.start_time)['m0'], form=['fits'])
    dt = datetime.datetime.strptime(s[0], '%Y-%m-%dT%H:%M:%S')
    year = dt.year
    month = dt.strftime("%B")
    day = dt.day
    date = str(day) + '-' + month + '-' + str(year)

    sourcename = sanitize_string(sourcename)

    urlparams = buildparams(sourcename, date, frequency)
    try:
        dom = minidom.parse(urllib2.urlopen(serviceurl + '?%s' % urlparams, timeout=10.0))
    except Exception as ex:
        LOG.warn('ALMA Source Catalog Database is unreachable...')
        LOG.debug(str(ex))
    LOG.debug('url: ' + serviceurl + '?%s' % urlparams)

    domtable = dom.getElementsByTagName('TR')
    rowdict = {}
    for node in domtable:
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
        # rowdict['verbose'] = row[12].childNodes[0].nodeValue
        rowdict['url'] = serviceurl + '?%s' % urlparams

    return rowdict


def buildparams(sourcename, date, frequency):
    """
    Inputs are all strings with the format:
    NAME=3c279&DATE=04-Apr-2014&FREQUENCY=231.435E9&WEIGHTED=true&RESULT=1
    """

    params = {'NAME'      : sourcename,
              'DATE'      : date,
              'FREQUENCY' : frequency,
              'WEIGHTED'  : 'true',
              'RESULT'    : 0}

    urlparams = urllib.urlencode(params)

    return urlparams


def sanitize_string(name):
    """
    Sanitize source name if needed
    """

    namereturn = name.split(';')

    return namereturn[0]


