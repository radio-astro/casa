from __future__ import absolute_import
import collections
import os
import shutil
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.mpihelpers as mpihelpers

import pipeline.h.tasks.importdata.importdata as importdata
from pipeline.hifv.heuristics.vlascanheuristics import VLAScanHeuristics

LOG = infrastructure.get_logger(__name__)


class VLAImportDataInputs(importdata.ImportDataInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, output_dir=None, asis=None, process_caldevice=None, session=None,
                 overwrite=None, nocopy=None, bdfflags=None, lazy=None, save_flagonline=None, createmms=None,
                 ocorr_mode=None):
        super(VLAImportDataInputs, self).__init__(context=context, vis=vis, output_dir=output_dir, asis=asis,
                                                  process_caldevice=process_caldevice, session=session,
                                                  overwrite=overwrite, nocopy=nocopy, bdfflags=bdfflags, lazy=lazy,
                                                  save_flagonline=save_flagonline, createmms=createmms,
                                                  ocorr_mode=ocorr_mode)

    # Override defaults in ImportDataInputs
    asis = basetask.property_with_default('asis', 'Receiver CalAtmosphere')
    ocorr_mode = basetask.property_with_default('ocorr_mode', 'co')
    bdfflags = basetask.property_with_default('bdfflags', False)
    process_caldevice = basetask.property_with_default('process_caldevice', True)


class VLAImportDataResults(basetask.Results):
    def __init__(self, mses=None, setjy_results=None):
        super(VLAImportDataResults, self).__init__()

        if mses is None:
            mses = []
        self.mses = mses
        self.setjy_results = setjy_results
        self.origin = {}

    def merge_with_context(self, context):
        target = context.observing_run

        for ms in self.mses:
            LOG.info('Adding {0} to context'.format(ms.name))
            target.add_measurement_set(ms)

            if ms.antenna_array.name in ('EVLA', 'VLA', 'JVLA'):
                if not hasattr(context, 'evla'):
                    context.evla = collections.defaultdict(dict)

                msinfos = dict((ms.name, self._do_msinfo_heuristics(ms.name, context)) for ms in self.mses)
                context.evla['msinfo'].update(msinfos)
                context.project_summary.telescope = 'EVLA'
                context.project_summary.observatory = 'Karl G. Jansky Very Large Array'
                # context.evla['msinfo'] = { m.name : msinfo }

        if self.setjy_results:
            for result in self.setjy_results:
                result.merge_with_context(context)




    def _do_msinfo_heuristics(self, ms, context):
        """Gets heuristics for VLA via msinfo script
        """

        msinfo = VLAScanHeuristics(ms)
        msinfo.makescandict()
        msinfo.calibratorIntents()
        msinfo.determine3C84()

        with casatools.TableReader(ms) as table:
            scanNums = sorted(numpy.unique(table.getcol('SCAN_NUMBER')))

        # Check for missing scans
        missingScans = 0
        missingScanStr = ''

        for i in range(max(scanNums)):
            if scanNums.count(i + 1) == 1:
                pass
            else:
                LOG.warn("WARNING: Scan " + str(i + 1) + " is not present")
                missingScans += 1
                missingScanStr = missingScanStr + str(i + 1) + ', '

        if (missingScans > 0):
            LOG.warn("WARNING: There were " + str(missingScans) + " missing scans in this MS")
        else:
            LOG.info("No missing scans found.")

        return msinfo

    def __repr__(self):
        return 'VLAImportDataResults:\n\t{0}'.format('\n\t'.join([ms.name for ms in self.mses]))


class VLAImportData(importdata.ImportData):
    Inputs = VLAImportDataInputs

    def prepare(self, **parameters):
        # get results object by running super.prepare()
        results = super(VLAImportData, self).prepare()

        # create results object
        myresults = VLAImportDataResults(mses=results.mses, setjy_results=results.setjy_results)

        myresults.origin = results.origin

        for ms in myresults.origin:
            if myresults.origin[ms] == 'ASDM':
                myresults.origin[ms] = 'SDM'

        PbandWarning = ''
        for ms in myresults.mses:
            for key, value in ms.get_vla_spw2band().iteritems():
                if 'P' in value:
                    PbandWarning = 'P-band data detected in the raw data. VLA P-band pipeline calibration has not yet been commissioned and may even fail. Please inspect all P-band pipeline products carefully.'

        if PbandWarning:
            LOG.warning(PbandWarning)

        return myresults

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

        with_pointing_correction = getattr(inputs, 'with_pointing_correction', True)

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
                                     process_pointing=True,
                                     createmms=createmms)

        self._executor.execute(task)

        for xml_filename in ['Source.xml', 'SpectralWindow.xml', 'DataDescription.xml']:
            asdm_source = os.path.join(asdm, xml_filename)
            if os.path.exists(asdm_source):
                vis_source = os.path.join(vis, xml_filename)
                LOG.info('Copying %s from ASDM to measurement set', xml_filename)
                LOG.trace('Copying %s: %s to %s', xml_filename, asdm_source, vis_source)
                shutil.copyfile(asdm_source, vis_source)
