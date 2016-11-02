from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

import pipeline.h.tasks.restoredata.restoredata as restoredata
from ..importdata import almaimportdata

LOG = infrastructure.get_logger(__name__)

class ALMARestoreDataInputs(restoredata.RestoreDataInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, copytoraw=None, products_dir=None,
                 rawdata_dir=None, output_dir=None, session=None, vis=None,
                 bdfflags=None, lazy=None, asis=None, ocorr_mode=None):
        self._init_properties(vars())

    asis = basetask.property_with_default('asis', 'Antenna Station Receiver Source CalAtmosphere CalWVR')


class ALMARestoreData(restoredata.RestoreData):
    Inputs = ALMARestoreDataInputs

    # Override generic method and use an ALMA specific one. Not much difference
    # now but should simplify parameters in future
    def _do_importasdm(self, sessionlist, vislist):
        inputs = self.inputs
        importdata_inputs = almaimportdata.ALMAImportData.Inputs(inputs.context,
            vis=vislist, session=sessionlist, save_flagonline=False,
            lazy=inputs.lazy, bdfflags=inputs.bdfflags, dbservice=False,
            asis=inputs.asis, ocorr_mode=inputs.ocorr_mode)
        importdata_task = almaimportdata.ALMAImportData(importdata_inputs)
        return self._executor.execute(importdata_task, merge=True)


