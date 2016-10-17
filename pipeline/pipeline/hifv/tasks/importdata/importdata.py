from __future__ import absolute_import

import collections
import pipeline.infrastructure.casatools as casatools
import numpy
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hifv.heuristics.vlascanheuristics import VLAScanHeuristics

import pipeline.h.tasks.importdata.importdata as importdata

LOG = infrastructure.get_logger(__name__)


class VLAImportDataInputs(importdata.ImportDataInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, output_dir=None, asis=None,
                 process_caldevice=None, session=None, overwrite=None,
                 bdfflags=None, lazy=None, save_flagonline=None, dbservice=None,
                 createmms=None, ocorr_mode=None, clearcals=None):
        self._init_properties(vars())

    overwrite = basetask.property_with_default('overwrite', False)
    save_flagonline = basetask.property_with_default('save_flagonline', True)
    asis = basetask.property_with_default('asis', 'Receiver CalAtmosphere')
    dbservice = basetask.property_with_default('dbservice', False)
    ocorr_mode = basetask.property_with_default('ocorr_mode', 'co')
    bdfflags = basetask.property_with_default('bdfflags', False)


class VLAImportDataResults(basetask.Results):
    def __init__(self, mses=[], setjy_results=None):
        super(VLAImportDataResults, self).__init__()

        self.mses = mses
        self.setjy_results = setjy_results
        self.origin = {}

    def merge_with_context(self, context):
        target = context.observing_run
        for ms in self.mses:
            LOG.info('Adding {0} to context'.format(ms.name))
            target.add_measurement_set(ms)

        if (ms.antenna_array.name == 'EVLA' or ms.antenna_array.name == 'VLA'):
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

        return myresults