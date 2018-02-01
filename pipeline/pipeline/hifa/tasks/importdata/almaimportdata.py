from __future__ import absolute_import

import pipeline.h.tasks.importdata.fluxes as fluxes
import pipeline.h.tasks.importdata.importdata as importdata
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import task_registry
from . import dbfluxes

__all__ = [
    'ALMAImportData',
    'ALMAImportDataInputs'
]

LOG = infrastructure.get_logger(__name__)


class ALMAImportDataInputs(importdata.ImportDataInputs):
    asis = vdp.VisDependentProperty(default='Antenna CalAtmosphere CalWVR ExecBlock Receiver SBSummary Source Station')
    dbservice = vdp.VisDependentProperty(default=True)

    def __init__(self, context, vis=None, output_dir=None, asis=None, process_caldevice=None, session=None,
                 overwrite=None, nocopy=None, bdfflags=None, lazy=None, save_flagonline=None, dbservice=None,
                 createmms=None, ocorr_mode=None, asimaging=None):
        super(ALMAImportDataInputs, self).__init__(context, vis=vis, output_dir=output_dir, asis=asis,
                                                   process_caldevice=process_caldevice, session=session,
                                                   overwrite=overwrite, nocopy=nocopy, bdfflags=bdfflags, lazy=lazy,
                                                   save_flagonline=save_flagonline, createmms=createmms,
                                                   ocorr_mode=ocorr_mode, asimaging=asimaging)
        self.dbservice = dbservice


@task_registry.set_equivalent_casa_task('hifa_importdata')
@task_registry.set_casa_commands_comment('If required, ASDMs are converted to MeasurementSets.')
class ALMAImportData(importdata.ImportData):
    Inputs = ALMAImportDataInputs

    def _get_fluxes(self, context, observing_run):
        # get the flux measurements from Source.xml for each MS
        if self.inputs.dbservice:
            xml_results = dbfluxes.get_setjy_results(observing_run.measurement_sets)
        else:
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
