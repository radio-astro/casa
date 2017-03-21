from __future__ import absolute_import
import collections
import contextlib
import csv
import itertools
import operator
import os
import shutil
import string
import tarfile
import types
import decimal
import datetime
import urllib
import urllib2
import xml.etree.ElementTree as ElementTree
from xml.dom import minidom

import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure.tablereader as tablereader
from pipeline.infrastructure import casa_tasks

from ..common import commonfluxresults

LOG = infrastructure.get_logger(__name__)


class ImportDataInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context=None, vis=None, output_dir=None,
                 asis=None, process_caldevice=None,
                 session=None, overwrite=None, save_flagonline=None,
                 bdfflags=None, lazy=None, dbservice=None, createmms=None,
                 ocorr_mode=None, clearcals=None):
        self._init_properties(vars())

    # This are ALMA specific settings. Make them generic at some point.
    # asis = basetask.property_with_default('asis', 'Antenna Station Receiver Source CalAtmosphere CalWVR')
    asis = basetask.property_with_default('asis', '')
    bdfflags = basetask.property_with_default('bdfflags', True)
    createmms = basetask.property_with_default('createmms', 'automatic')
    dbservice = basetask.property_with_default('dbservice', True)
    lazy = basetask.property_with_default('lazy', False)
    overwrite = basetask.property_with_default('overwrite', False)
    process_caldevice = basetask.property_with_default('process_caldevice', False)
    save_flagonline = basetask.property_with_default('save_flagonline', True)
    ocorr_mode = basetask.property_with_default('ocorr_mode', 'ca')
    clearcals = basetask.property_with_default('clearcals', True)

    @property
    def session(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('session')

        # if vis is a scalar but session is a list, return the session for this vis
        if not isinstance(self.vis, list) and isinstance(self._session, list):
            idx = self._my_vislist.index(self.vis)
            return self._session[idx]

        if type(self.vis) is types.StringType and type(self._session) is types.StringType:
            return self._session

        # current default - return all intents
        return 'session_1'

    @session.setter
    def session(self, value):
        self._session = value

    def to_casa_args(self):
        raise NotImplementedError

    # MandatoryPipelineInputs raises an exception if vis has not been
    # registered with the context. For an import task however, the vis is never
    # registered. To avoid the exception, we override the vis getter and
    # setter.
    @property
    def vis(self):
        return self._vis

    @vis.setter
    def vis(self, value):
        vislist = value if type(value) is types.ListType else [value, ]

        # VISLIST_RESET_KEY is present when vis is set by handle_multivis.
        # In this case we do not want to reset my_vislist, as handle_multivis is
        # setting vis to the individual measurement sets
        if not hasattr(self, basetask.VISLIST_RESET_KEY):
            LOG.trace('Setting Inputs._my_vislist to %s' % vislist)
            self._my_vislist = vislist
        else:
            LOG.trace('Leaving Inputs._my_vislist at current value of %s'
                      % self._my_vislist)

        self._vis = value


class ImportDataResults(basetask.Results):
    """
    ImportDataResults holds the results of the ImportData task. It contains
    the resulting MeasurementSet domain objects and optionally the additional
    SetJy results generated from flux entries in Source.xml.
    """

    def __init__(self, mses=None, setjy_results=None):
        super(ImportDataResults, self).__init__()
        self.mses = [] if mses is None else mses
        self.setjy_results = setjy_results
        self.origin = {}

    def merge_with_context(self, context):
        target = context.observing_run
        for ms in self.mses:
            LOG.info('Adding {0} to context'.format(ms.name))
            target.add_measurement_set(ms)

        if self.setjy_results:
            for result in self.setjy_results:
                result.merge_with_context(context)

    def __repr__(self):
        return 'ImportDataResults:\n\t{0}'.format(
            '\n\t'.join([ms.name for ms in self.mses]))


class ImportData(basetask.StandardTaskTemplate):
    Inputs = ImportDataInputs

    def _ms_directories(self, names):
        '''
        Inspect a list of file entries, finding the root directory of any
        measurement sets present via a set of characteristic files and
        directories.
        '''
        identifiers = ('SOURCE', 'FIELD', 'ANTENNA', 'DATA_DESCRIPTION')

        matching = [os.path.dirname(n) for n in names
                    if os.path.basename(n) in identifiers]

        return set([m for m in matching
                    if matching.count(m) == len(identifiers)])

    def _asdm_directories(self, members):
        '''
        Inspect a list of file entries, finding the root directory of any
        ASDMs present via a set of characteristic files and directories.
        '''
        identifiers = ('ASDMBinary', 'Main.xml', 'ASDM.xml', 'Antenna.xml')

        matching = [os.path.dirname(m) for m in members
                    if os.path.basename(m) in identifiers]

        return set([m for m in matching
                    if matching.count(m) == len(identifiers)])

    def prepare(self, **parameters):
        inputs = self.inputs
        vis = inputs.vis

        if vis is None:
            msg = 'Empty input data set list'
            LOG.warning(msg)
            raise ValueError, msg

        if not os.path.exists(vis):
            msg = 'Input data set \'{0}\' not found'.format(vis)
            LOG.error(msg)
            raise IOError, msg

        results = ImportDataResults()
        to_import = set()
        to_convert = set()
        to_clearcal = set()

        # if this is a tar, get the names of the files and directories inside
        # the tar and calculate which can be directly imported (filenames with
        # a measurement set fingerprint) and which must be converted (files
        # with an ASDM fingerprint).
        if os.path.isfile(vis) and tarfile.is_tarfile(vis):
            with contextlib.closing(tarfile.open(vis)) as tar:
                filenames = tar.getnames()

                (to_import, to_convert) = self._analyse_filenames(filenames,
                                                                  vis)

                to_convert = [os.path.join(inputs.output_dir, asdm)
                              for asdm in to_convert]
                to_import = [os.path.join(inputs.output_dir, ms)
                             for ms in to_import]

                if not self._executor._dry_run:
                    LOG.info('Extracting %s to %s' % (vis, inputs.output_dir))
                    tar.extractall(path=inputs.output_dir)

        # Assume that if vis is not a tar, it's a directory ready to be
        # imported, or in the case of an ASDM, converted then imported.
        else:
            # get a list of all the files in the given directory
            filenames = [os.path.join(vis, f) for f in os.listdir(vis)]

            (to_import, to_convert) = self._analyse_filenames(filenames,
                                                              vis)

            if not to_import and not to_convert:
                raise TypeError, '%s is neither a MS nor an ASDM' % vis

            # convert all paths to absolute paths for the next sequence
            to_import = map(os.path.abspath, to_import)

            # if the file is not in the working directory, copy it across,
            # replacing the filename with the relocated filename
            to_copy = set([f for f in to_import
                           if string.find(f, inputs.output_dir) != 0])
            for src in to_copy:
                dst = os.path.join(os.path.abspath(inputs.output_dir),
                                   os.path.basename(src))
                to_import.remove(src)
                to_import.append(dst)

                if os.path.exists(dst):
                    LOG.warning('%s already in %s. Will import existing data.'
                                '' % (os.path.basename(src), inputs.output_dir))
                    if inputs.clearcals:
                        LOG.info('Pipeline clearcal enabled for %s' % (dst))
                        to_clearcal.add(dst)
                    else:
                        LOG.info('Pipeline clearcal disabled for %s' % (dst))
                    continue

                if not self._executor._dry_run:
                    LOG.info('Copying %s to %s' % (src, inputs.output_dir))
                    shutil.copytree(f, dst)

        # clear the calibration of any stale file that exists in the working
        # directory
        for old_file in to_clearcal:
            self._do_clearcal(old_file)

        # launch an import job for each ASDM we need to convert
        for asdm in to_convert:
            self._do_importasdm(asdm)

        # calculate the filenames of the resultant measurement sets
        asdms = [os.path.join(inputs.output_dir, f) for f in to_convert]

        # Now everything is in MS format, create a list of the MSes to import
        converted_asdms = [self._asdm_to_vis_filename(asdm) for asdm in asdms]
        to_import.extend(converted_asdms)

        # get the path to the MS for the converted ASDMs, which we'll later
        # compare to ms.name in order to calculate the origin of each MS
        converted_asdm_abspaths = [os.path.abspath(f) for f in converted_asdms]

        LOG.info('Creating pipeline objects for measurement set(s) {0}'
                 ''.format(', '.join(to_import)))
        if self._executor._dry_run:
            return ImportDataResults()

        ms_reader = tablereader.ObservingRunReader

        to_import = [os.path.abspath(f) for f in to_import]
        observing_run = ms_reader.get_observing_run(to_import)
        for ms in observing_run.measurement_sets:
            LOG.debug('Setting session to %s for %s' % (inputs.session,
                                                        ms.basename))
            ms.session = inputs.session

            ms_origin = 'ASDM' if ms.name in converted_asdm_abspaths else 'MS'
            results.origin[ms.basename] = ms_origin

        # get the flux measurements from Source.xml for each MS
        # print inputs.dbservice
        xml_results = get_setjy_results(observing_run.measurement_sets, dbservice=inputs.dbservice)
        # write/append them to flux.csv

        # Cycle 1 hack for exporting the field intents to the CSV file:
        # export_flux_from_result queries the context, so we pseudo-register
        # the mses with the context by replacing the original observing run
        orig_observing_run = inputs.context.observing_run
        inputs.context.observing_run = observing_run
        try:
            export_flux_from_result(xml_results, inputs.context)
        finally:
            inputs.context.observing_run = orig_observing_run

        # re-read from flux.csv, which will include any user-coded values
        combined_results = import_flux(inputs.context.output_dir, observing_run)

        results.mses.extend(observing_run.measurement_sets)
        results.setjy_results = combined_results

        return results

    def analyse(self, result):
        return result

    def _analyse_filenames(self, filenames, vis):
        to_import = set()
        to_convert = set()

        ms_dirs = self._ms_directories(filenames)
        if ms_dirs:
            LOG.debug('Adding measurement set(s) {0} from {1} to import queue'
                      ''.format(', '.join([os.path.basename(f) for f in ms_dirs]),
                                vis))
            cleaned_paths = map(os.path.normpath, ms_dirs)
            to_import.update(cleaned_paths)

        asdm_dirs = self._asdm_directories(filenames)
        if asdm_dirs:
            LOG.debug('Adding ASDMs {0} from {1} to conversion queue'
                      ''.format(', '.join(asdm_dirs), vis))
            to_convert.update(asdm_dirs)

        return (to_import, to_convert)

    def _asdm_to_vis_filename(self, asdm):
        return '{0}.ms'.format(os.path.join(self.inputs.output_dir,
                                            os.path.basename(asdm)))

    def _do_clearcal(self, vis):
        task = casa_tasks.clearcal(vis=vis, addmodel=False)
        self._executor.execute(task)

    def _do_importasdm(self, asdm):
        inputs = self.inputs
        vis = self._asdm_to_vis_filename(asdm)
        outfile = os.path.join(inputs.output_dir,
                               os.path.basename(asdm) + "_flagonline.txt")

        if inputs.save_flagonline:
            # Create the standard calibration flagging template file
            template_flagsfile = os.path.join(inputs.output_dir,
                                              os.path.basename(asdm) + "_flagtemplate.txt")
            self._make_template_flagfile(asdm, template_flagsfile,
                                         'User flagging commands file for the calibration pipeline')
            # Create the imaging targets file
            template_flagsfile = os.path.join(inputs.output_dir,
                                              os.path.basename(asdm) + "_flagtargetstemplate.txt")
            self._make_template_flagfile(asdm, template_flagsfile,
                                         'User flagging commands file for the imaging pipeline')

        createmms = mpihelpers.parse_mpi_input_parameter(inputs.createmms)

        with_pointing_correction = getattr(inputs, 'with_pointing_correction', False)
        # ocorr_mode = getattr(inputs, 'ocorr_mode', 'ca')

        task = casa_tasks.importasdm(asdm=asdm,
                                     vis=vis,
                                     savecmds=inputs.save_flagonline,
                                     outfile=outfile,
                                     process_caldevice=inputs.process_caldevice,
                                     asis=inputs.asis,
                                     overwrite=inputs.overwrite,
                                     bdfflags=inputs.bdfflags,
                                     lazy=inputs.lazy,
                                     with_pointing_correction=with_pointing_correction,
                                     ocorr_mode=inputs.ocorr_mode,
                                     createmms=createmms)

        self._executor.execute(task)

        for xml_filename in ['Source.xml', 'SpectralWindow.xml', 'DataDescription.xml']:
            asdm_source = os.path.join(asdm, xml_filename)
            if os.path.exists(asdm_source):
                vis_source = os.path.join(vis, xml_filename)
                LOG.info('Copying %s from ASDM to measurement set', xml_filename)
                LOG.trace('Copying %s: %s to %s', xml_filename, asdm_source, vis_source)
                shutil.copyfile(asdm_source, vis_source)

    def _make_template_flagfile(self, asdm, outfile, titlestr):

        # Create a new file if overwrite is true and the file
        # does not already exist.
        inputs = self.inputs
        if inputs.overwrite or not os.path.exists(outfile):
            with open(outfile, 'w') as f:
                f.writelines(['# ' + titlestr + '\n'])
                f.writelines(['#\n'])
                f.writelines(['# Examples\n'])
                f.writelines(['# Note: Do not put spaces inside the reason string !\n'])
                f.writelines(['#\n'])
                f.writelines(["# mode='manual' correlation='YY' antenna='DV01;DV08;DA43;DA48&DV23' spw='21:1920~2880' autocorr=False reason='bad_channels'\n"])
                f.writelines(["# mode='manual' spw='25:0~3;122~127' reason='stage8_2'\n"])
                f.writelines([ "# mode='manual' antenna='DV07' timerange='2013/01/31/08:09:55.248~2013/01/31/08:10:01.296' reason='quack'\n"])
                f.writelines(['#\n'])


def get_setjy_results(mses, dbservice=True):
    results = []
    for ms in mses:
        result = commonfluxresults.FluxCalibrationResults(ms.name)
        science_spw_ids = [spw.id for spw in ms.get_spectral_windows()]

        for source, measurements in read_fluxes(ms, dbservice=dbservice).items():
            m = [m for m in measurements if int(m.spw_id) in science_spw_ids]

            # import flux values for all fields and intents so that we can
            # compare them to the fluxscale-derived values later in the run
            #            for field in [f for f in source.fields if 'AMPLITUDE' in f.intents]:
            for field in source.fields:
                result.measurements[field.id].extend(m)

        results.append(result)

    return results


def read_fluxes(ms, dbservice=True):
    result = collections.defaultdict(list)

    science_spw_ids = [spw.id for spw in ms.get_spectral_windows()]

    source_table = os.path.join(ms.name, 'Source.xml')
    if not os.path.exists(source_table):
        LOG.info('No Source XML found at %s. No flux import performed. '
                 'Attempting database query.', source_table)
        return flux_nosourcexml(ms, dbservice=dbservice)

    source_element = ElementTree.parse(source_table)
    if not source_element:
        LOG.info('Could not parse Source XML at %s. No flux import performed. '
                 'Attempting database query.', source_table)
        return flux_nosourcexml(ms, dbservice=dbservice)

    # Empty spws that follow non-empty spws can be pruned from MMS data. This
    # set is used to check whether the Source.xml entries refer to spws
    # actually present in the measurement set.
    all_ms_spw_ids = {spw.id for spw in ms.spectral_windows}

    # SCIREQ-852: MS spw IDs != ASDM spw ids
    asdm_to_ms_spw_map = get_asdm_to_ms_spw_mapping(ms)

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

        source_id = int(source_element)
        if source_id >= len(ms.sources):
            LOG.warning('Source.xml refers to source #{!s}, which was not '
                        'found in {!s}'.format(source_id, ms.basename))
            continue
        source = ms.sources[int(source_id)]

        # all elements must contain data to proceed
        if flux_text is None or frequency_text is None:
            if not dbservice:
                continue

            # See what elements can be used
            try:
                if spw_id and frequency_text is None:
                    spw = ms.get_spectral_windows(spw_id)
                    frequency = str(spw[0].centre_frequency.value)
            except:
                continue

        else:
            # more than one measurement can be registered against the spectral
            # window. These functions give a lists of frequencies and IQUV
            # 4-tuples
            row_frequencies = to_hertz(frequency_text)
            row_iquvs = to_jansky(flux_text)
            # zip to give a list of (frequency, [I,Q,U,V]) tuples
            zipped = zip(row_frequencies, row_iquvs)

            spw = ms.get_spectral_window(spw_id)

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
                LOG.trace('Averaging {!s} equally close measurements: {!s}'.format(len(joint_closest), joint_closest))

            # calculate the mean of these equally distant  measurements.
            # joint_closest has at least one item, so we don't need to prime
            # the reduce function with an empty accumulator
            mean_iquv = [reduce(lambda x, y: x + y, stokes) / len(joint_closest) for stokes in zip(*joint_closest)]

            LOG.info('Closest flux measurement for {!s} spw {!s} found {!s} '
                     'distant from centre of spw)'.format(ms.basename, spw_id, min_delta))

            # Even if a mean was calculated, any alternative selection should
            # be equally distant and therefore outside the sow range too
            if not spw.min_frequency < closest_frequency < spw.max_frequency:
                # This might become a warning once the PRTSPR-20823 fix is active
                LOG.info('Closest flux measurement for {!s} spw {!s} falls outside'
                         'spw, {!s} distant from spectral window centre'.format(ms.basename, spw_id, min_delta))

            m = domain.FluxMeasurement(spw_id, *mean_iquv)

        # At this point we take:
        #  - the frequency of the spw_id in Hz
        #  - the source name string
        #  - The observation date
        #  and attempt to call the online flux catalog web service, and use the flux result
        #  and spectral index
        if dbservice:
            source_name = source.name
            asdmmessage = ''
            try:
                fluxdict = fluxservice(ms, frequency, source_name)
                f = fluxdict['fluxdensity']
                spix = fluxdict['spectralindex']

                try:
                    fluxvalue = measures.FluxDensity(float(f), measures.FluxDensityUnits.JANSKY)
                    if float(f) == 0.0 or str(f) == 'Infinity' or str(spix) == '-1000':
                        fluxvalue = iquv[0]
                    iquv_db = (fluxvalue,
                               iquv[1], iquv[2], iquv[3])
                    if int(spw_id) in science_spw_ids:
                        asdmmessage = "   ASDM Flux: {!s}".format(str(iquv[0]))
                except:
                    # No flux values from Source.xml
                    iquv_db = (measures.FluxDensity(float(f), measures.FluxDensityUnits.JANSKY),
                               measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY),
                               measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY),
                               measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY))
                    if int(spw_id) in science_spw_ids:
                        asdmmessage = "  No ASDM Flux"

                    # No flux values from Source.xml OR unusable online catalog info
                    if float(f) == 0.0 or str(f) == 'Infinity' or str(spix) == '-1000':
                        LOG.info("Source: {!s} spw: {!s} {!s}    Online catalog Flux: {!s} Jy"
                                 "".format(source_name, spw_id, asdmmessage, f))
                        LOG.info("         Online catalog Spectral Index: {!s}".format(str(spix)))
                        LOG.info("         Unusable online catalog information.")
                        LOG.info("---------------------------------------------")
                        continue

                LOG.info("Source: {!s} spw: {!s} {!s}    Online catalog Flux: {!s} Jy"
                         "".format(source_name, spw_id, asdmmessage, f))
                LOG.info("         Online catalog Spectral Index: {!s}".format(str(spix)))
                if float(f) == 0.0 or str(f) == 'Infinity' or str(spix) == '-1000':
                    LOG.info("         Unusable online catalog information.")
                    LOG.info("---------------------------------------------")

                m = domain.FluxMeasurement(spw_id, *iquv_db)


            except:

                if None in (flux_text, frequency_text, source_id, spw_id):
                    # If Source.xml AND online flux were a no-go then continue
                    continue
                else:
                    # Use Source.xml values since nothing was returned from the online database
                    m = domain.FluxMeasurement(spw_id, *iquv)
                    if (int(spw_id) in science_spw_ids):
                        LOG.info("Source: " + source_name + " spw: " + str(spw_id) + "    ASDM Flux: " + str(
                            iquv[0]) + "     No online catalog information.")
        result[source].append(m)

    return result


def flux_nosourcexml(ms, dbservice=True):
    """
    Call the flux service and get the frequencies from the ms if no Source.xml is available
    """
    result = collections.defaultdict(list)

    if dbservice:

        spws = ms.get_spectral_windows()

        for source in ms.sources:
            for spw in spws:
                sourcename = source.name
                frequency = str(spw.centre_frequency.value)
                spw_id = spw.id
                LOG.info('freq/sourcename:  ' + str(frequency) + str(sourcename))

                try:

                    fluxdict = fluxservice(ms, frequency, sourcename)
                    f = fluxdict['fluxdensity']
                    iquv_db = (measures.FluxDensity(float(f), measures.FluxDensityUnits.JANSKY),
                               measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY),
                               measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY),
                               measures.FluxDensity(0.0, measures.FluxDensityUnits.JANSKY))
                    m = domain.FluxMeasurement(spw_id, *iquv_db)
                    result[source].append(m)
                except:
                    LOG.debug("    No flux catalog values for source " + str(source.name) + "  spw:" + str(spw_id))

    return result


def fluxservice(ms, frequency, sourcename):
    """
        Usage of this online service requires:
         - ms - for getting the date
         - frequency_text - we will get the frequency out of this in Hz
         - source - we will get source.name from this object
    """
    # serviceurl = 'http://bender.csrg.cl:2121/bfs-0.2/ssap'
    # serviceurl =  'http://asa-test.alma.cl/bfs/'
    serviceurl = 'https://almascience.eso.org/sc/flux'

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
    except:
        LOG.warn('DB flux service timeout/connection problem...')

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
        rowdict['verbose'] = row[12].childNodes[0].nodeValue
        rowdict['url'] = serviceurl + '?%s' % urlparams

    return rowdict


def buildparams(sourcename, date, frequency):
    """
       Inputs are all strings, in the format:
       NAME=3c279&DATE=04-Apr-2014&FREQUENCY=231.435E9
    """
    params = {'NAME': sourcename,
              'DATE': date,
              'FREQUENCY': frequency}

    urlparams = urllib.urlencode(params)

    return urlparams


def sanitize_string(name):
    """
        sanitize source name if needed
    """

    namereturn = name.split(';')

    return namereturn[0]


def get_flux_density(frequency_text, flux_text):
    frequencies = to_hertz(frequency_text)
    fluxes = to_jansky(flux_text)

    for freq, flux in zip(frequencies, fluxes):
        yield (freq, flux)


def to_jansky(flux_text):
    """
    Convert a string extracted from an ASDM XML element to FluxDensity domain
    objects.
    """
    flux_fn = lambda f: measures.FluxDensity(float(f),
                                             measures.FluxDensityUnits.JANSKY)
    return get_atoms(flux_text, flux_fn)


def to_hertz(freq_text):
    """
    Convert a string extracted from an ASDM XML element to Frequency domain
    objects.
    """
    freq_fn = lambda f: measures.Frequency(float(f),
                                           measures.FrequencyUnits.HERTZ)
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


def export_flux_from_context(context, filename=None):
    """
    Export flux densities stored in the given context to a CSV file.
    """
    if not filename:
        filename = os.path.join(context.output_dir, 'flux.csv')

    with open(filename, 'wt') as f:
        writer = csv.writer(f)
        writer.writerow(('ms', 'field', 'spw', 'I', 'Q', 'U', 'V', 'spix', 'comment'))

        counter = 0
        for ms in context.observing_run.measurement_sets:
            for field in ms.fields:
                for flux in field.flux_densities:
                    (I, Q, U, V) = flux.casa_flux_density
                    comment = 'intent=' + ','.join(sorted(field.intents))
                    writer.writerow((ms.basename, field.id, flux.spw_id,
                                     I, Q, U, V, float(flux.spix), comment))
                    counter += 1

        LOG.info('Exported %s flux measurements to %s' % (counter, filename))


def export_flux_from_result(results, context, filename='flux.csv'):
    """
    Export flux densities from a set of results to a CSV file.
    """
    if type(results) is not types.ListType:
        results = [results, ]
    abspath = os.path.join(context.output_dir, filename)

    columns = ['ms', 'field', 'spw', 'I', 'Q', 'U', 'V', 'spix', 'comment']
    existing = []

    # if the file exists, read it in
    if os.path.exists(abspath):
        with open(abspath, 'r') as f:
            # slurp in all but the header rows
            existing.extend([l for l in f.readlines()
                             if not l.startswith(','.join(columns))])

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
                    exists = False
                    for row in existing:
                        if row.startswith(prefix):
                            LOG.info('Not overwriting flux data for %s field %s '
                                     'spw %s in %s' % (ms_basename, field_id,
                                                       m.spw_id,
                                                       os.path.basename(abspath)))
                            exists = True

                    if not exists:
                        (I, Q, U, V) = m.casa_flux_density

                        ms = context.observing_run.get_ms(ms_basename)
                        field = ms.get_fields(field_id)[0]
                        comment = field.name + ' ' + 'intent=' + ','.join(sorted(field.intents))

                        writer.writerow((ms_basename, field_id, m.spw_id,
                                         I, Q, U, V, float(m.spix), comment))
                        counter += 1

        LOG.info('Exported %s flux measurements to %s' % (counter, abspath))


def import_flux(output_dir, observing_run, filename=None):
    """
    Read flux densities from a CSV file and import them into the context.
    """
    if not filename:
        filename = os.path.join(output_dir, 'flux.csv')

    with open(filename, 'rt') as f:
        reader = csv.reader(f)

        # first row is header row
        reader.next()

        counter = 0
        for row in reader:
            try:
                try:
                    (ms_name, field_id, spw_id, I, Q, U, V, spix, _) = row
                    spix = decimal.Decimal(spix)
                except:
                    (ms_name, field_id, spw_id, I, Q, U, V, __) = row
                    spix = decimal.Decimal('0.0')
                spw_id = int(spw_id)
                try:
                    ms = observing_run.get_ms(ms_name)
                except KeyError:
                    # No MS registered by that name. This could be caused by a
                    # flux.csv from a previous run
                    LOG.info('%s refers to unregistered measurement set \'%s\'. '
                             'If this is a multi-ASDM run this to be expected.'
                             '' % (filename, ms_name))
                    continue

                fields = ms.get_fields(field_id)
                measurement = domain.FluxMeasurement(spw_id, I, Q, U, V, spix)

                # A single field identifier could map to multiple field objects,
                # but the flux should be the same for all, so we iterate..
                for field in fields:
                    # .. removing any existing measurements in these spws from
                    # these fields..
                    map(field.flux_densities.remove,
                        [m for m in field.flux_densities if m.spw_id is spw_id])

                    # .. and then updating with our new values
                    LOG.trace('Adding %s to spw %s' % (measurement, spw_id))
                    field.flux_densities.add(measurement)
                    counter += 1
            except:
                LOG.warning('Problem importing \'%s\' as a flux statement' % row)

        LOG.info('Imported %s flux measurements from %s' % (counter, filename))

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
                    result.measurements[field.name].append(flux)
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
