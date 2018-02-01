from __future__ import absolute_import

import contextlib
import os
import shutil
import string
import tarfile

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure.tablereader as tablereader
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry
from . import fluxes

__all__ = [
    'ImportData',
    'ImportDataInputs',
    'ImportDataResults'
]

LOG = infrastructure.get_logger(__name__)


class ImportDataInputs(vdp.StandardInputs):
    asimaging = vdp.VisDependentProperty(default=False)
    asis = vdp.VisDependentProperty(default='')
    bdfflags = vdp.VisDependentProperty(default=True)
    createmms = vdp.VisDependentProperty(default='automatic')
    lazy = vdp.VisDependentProperty(default=False)
    nocopy = vdp.VisDependentProperty(default=False)
    ocorr_mode = vdp.VisDependentProperty(default='ca')
    overwrite = vdp.VisDependentProperty(default=False)
    process_caldevice = vdp.VisDependentProperty(default=False)
    save_flagonline = vdp.VisDependentProperty(default=True)
    session = vdp.VisDependentProperty(default='session_1')

    def __init__(self, context, vis=None, output_dir=None, asis=None, process_caldevice=None, session=None,
                 overwrite=None, nocopy=None, save_flagonline=None, bdfflags=None, lazy=None, createmms=None,
                 ocorr_mode=None, asimaging=None):
        super(ImportDataInputs, self).__init__()

        self.context = context
        self.vis = vis
        self.output_dir = output_dir

        self.asimaging = asimaging
        self.asis = asis
        self.bdfflags = bdfflags
        self.createmms = createmms
        self.lazy = lazy
        self.nocopy = nocopy
        self.ocorr_mode = ocorr_mode
        self.overwrite = overwrite
        self.process_caldevice = process_caldevice
        self.save_flagonline = save_flagonline
        self.session = session

    def to_casa_args(self):
        raise NotImplementedError


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


@task_registry.set_equivalent_casa_task('h_importdata')
@task_registry.set_casa_commands_comment('If required, ASDMs are converted to MeasurementSets.')
class ImportData(basetask.StandardTaskTemplate):
    Inputs = ImportDataInputs

    def _ms_directories(self, names):
        """
        Inspect a list of file entries, finding the root directory of any
        measurement sets present via a set of characteristic files and
        directories.
        """
        identifiers = ('SOURCE', 'FIELD', 'ANTENNA', 'DATA_DESCRIPTION')

        matching = [os.path.dirname(n) for n in names
                    if os.path.basename(n) in identifiers]

        return set([m for m in matching
                    if matching.count(m) == len(identifiers)])

    def _asdm_directories(self, members):
        """
        Inspect a list of file entries, finding the root directory of any
        ASDMs present via a set of characteristic files and directories.
        """
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
            raise ValueError(msg)

        if not os.path.exists(vis):
            msg = 'Input data set \'{0}\' not found'.format(vis)
            LOG.error(msg)
            raise IOError(msg)

        results = ImportDataResults()

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
                raise TypeError('{!s} is of unhandled type'.format(vis))

            # convert all paths to absolute paths for the next sequence
            to_import = map(os.path.abspath, to_import)

            # if the file is not in the working directory, copy it across,
            # replacing the filename with the relocated filename
            to_copy = {f for f in to_import
                       if string.find(f, inputs.output_dir) != 0
                       and inputs.nocopy is False}
            for src in to_copy:
                dst = os.path.join(os.path.abspath(inputs.output_dir),
                                   os.path.basename(src))
                to_import.remove(src)
                to_import.append(dst)

                if os.path.exists(dst):
                    LOG.warning('%s already in %s. Will import existing data.'
                                '' % (os.path.basename(src), inputs.output_dir))
                    continue

                if not self._executor._dry_run:
                    LOG.info('Copying %s to %s' % (src, inputs.output_dir))
                    shutil.copytree(src, dst)

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
            if inputs.asimaging:
               LOG.info('Importing %s as an imaging measurement set' % (ms.basename))
               ms.is_imaging_ms = True

            ms.session = inputs.session

            ms_origin = 'ASDM' if ms.name in converted_asdm_abspaths else 'MS'
            results.origin[ms.basename] = ms_origin

        combined_results = self._get_fluxes(inputs.context, observing_run)

        results.mses.extend(observing_run.measurement_sets)
        results.setjy_results = combined_results

        return results

    def analyse(self, result):
        return result

    def _get_fluxes(self, context, observing_run):

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

        return to_import, to_convert

    def _asdm_to_vis_filename(self, asdm):
        return '{0}.ms'.format(os.path.join(self.inputs.output_dir,
                                            os.path.basename(asdm)))

    def _do_importasdm(self, asdm):
        inputs = self.inputs
        vis = self._asdm_to_vis_filename(asdm)
        outfile = os.path.join(inputs.output_dir,
                               os.path.basename(asdm) + '.flagonline.txt')

        if inputs.save_flagonline:
            # Create the standard calibration flagging template file
            template_flagsfile = os.path.join(inputs.output_dir,
                                              os.path.basename(asdm) + '.flagtemplate.txt')
            self._make_template_flagfile(asdm, template_flagsfile,
                                         'User flagging commands file for the calibration pipeline')
            # Create the imaging targets file
            template_flagsfile = os.path.join(inputs.output_dir,
                                              os.path.basename(asdm) + '.flagtargetstemplate.txt')
            self._make_template_flagfile(asdm, template_flagsfile,
                                         'User flagging commands file for the imaging pipeline')

        createmms = mpihelpers.parse_mpi_input_parameter(inputs.createmms)

        with_pointing_correction = getattr(inputs, 'with_pointing_correction', False)

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
            template_text = FLAGGING_TEMPLATE_HEADER.replace('___TITLESTR___', titlestr)
            with open(outfile, 'w') as f:
                f.writelines(template_text)


FLAGGING_TEMPLATE_HEADER = '''#
# ___TITLESTR___
#
# Examples
# Note: Do not put spaces inside the reason string !
#
# mode='manual' correlation='YY' antenna='DV01;DV08;DA43;DA48&DV23' spw='21:1920~2880' autocorr=False reason='bad_channels'
# mode='manual' spw='25:0~3;122~127' reason='stage8_2'
# mode='manual' antenna='DV07' timerange='2013/01/31/08:09:55.248~2013/01/31/08:10:01.296' reason='quack'
#
'''
