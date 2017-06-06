from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

import pipeline.h.tasks.importdata.fluxes as fluxes
import pipeline.h.tasks.importdata.importdata as importdata
from . import dbfluxes

__all__ = [
    'ALMAImportData',
    'ALMAImportDataInputs'
]

LOG = infrastructure.get_logger(__name__)


class ALMAImportDataInputs(importdata.ImportDataInputs):
    asis = basetask.property_with_default('asis',
                                          'Antenna CalAtmosphere CalWVR ExecBlock Receiver SBSummary Source Station')
    dbservice = basetask.property_with_default('dbservice', True)

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, output_dir=None, asis=None, process_caldevice=None, session=None,
                 overwrite=None, nocopy=None, bdfflags=None, lazy=None, save_flagonline=None, dbservice=None,
                 createmms=None, ocorr_mode=None):
        super(ALMAImportDataInputs, self).__init__(context, vis=vis, output_dir=output_dir, asis=asis,
                                                   process_caldevice=process_caldevice, session=session,
                                                   overwrite=overwrite, nocopy=nocopy, bdfflags=bdfflags, lazy=lazy,
                                                   save_flagonline=save_flagonline, createmms=createmms,
                                                   ocorr_mode=ocorr_mode)
        self.dbservice = dbservice

    def to_casa_args(self):
        raise NotImplementedError


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
