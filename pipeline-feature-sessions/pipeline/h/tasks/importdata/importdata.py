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
from . import fluxes

LOG = infrastructure.get_logger(__name__)


class ImportDataInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context=None, vis=None, output_dir=None,
                 asis=None, process_caldevice=None,
                 session=None, overwrite=None, save_flagonline=None,
                 bdfflags=None, lazy=None, createmms=None,
                 ocorr_mode=None, clearcals=None):
        self._init_properties(vars())

    # This are ALMA specific settings. Make them generic at some point.
    # asis = basetask.property_with_default('asis', 'Antenna Station Receiver Source CalAtmosphere CalWVR')
    asis = basetask.property_with_default('asis', '')
    bdfflags = basetask.property_with_default('bdfflags', True)
    createmms = basetask.property_with_default('createmms', 'automatic')
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

        combined_results = self._get_fluxes(inputs.context, observing_run)

        results.mses.extend(observing_run.measurement_sets)
        results.setjy_results = combined_results

        return results

    def analyse(self, result):
        return result

    def _get_fluxes (self, context, observing_run):

        # get the flux measurements from Source.xml for each MS
        xml_results = fluxes.get_setjy_results(observing_run.measurement_sets)
        # write/append them to flux.csv

        # Cycle 1 hack for exporting the field intents to the CSV file:
        # export_flux_from_result queries the context, so we pseudo-register
        # the mses with the context by replacing the original observing run
        orig_observing_run = context.observing_run
        context.observing_run = observing_run
        try:
            fluxes.export_flux_from_result(xml_results, context)
        finally:
            context.observing_run = orig_observing_run

        # re-read from flux.csv, which will include any user-coded values
        combined_results = fluxes.import_flux(context.output_dir, observing_run)

        return combined_results


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

