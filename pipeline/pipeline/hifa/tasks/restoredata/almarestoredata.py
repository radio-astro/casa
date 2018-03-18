from __future__ import absolute_import

import pipeline.h.tasks.restoredata.restoredata as restoredata
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import task_registry
from ..importdata import almaimportdata

LOG = infrastructure.get_logger(__name__)


class ALMARestoreDataInputs(restoredata.RestoreDataInputs):
    asis = vdp.VisDependentProperty(default='SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR')

    def __init__(self, context, copytoraw=None, products_dir=None, rawdata_dir=None, output_dir=None, session=None,
                 vis=None, bdfflags=None, lazy=None, asis=None, ocorr_mode=None):
        super(ALMARestoreDataInputs, self).__init__(context, copytoraw=copytoraw, products_dir=products_dir,
                                                    rawdata_dir=rawdata_dir, output_dir=output_dir, session=session,
                                                    vis=vis, bdfflags=bdfflags, lazy=lazy, asis=asis,
                                                    ocorr_mode=ocorr_mode)


@task_registry.set_equivalent_casa_task('hifa_restoredata')
class ALMARestoreData(restoredata.RestoreData):
    Inputs = ALMARestoreDataInputs

    # Override generic method and use an ALMA specific one. Not much difference
    # now but should simplify parameters in future
    def _do_importasdm(self, sessionlist, vislist):
        inputs = self.inputs

        container = vdp.InputsContainer(almaimportdata.ALMAImportData, inputs.context, vis=vislist, session=sessionlist,
                                        save_flagonline=False, lazy=inputs.lazy, bdfflags=inputs.bdfflags,
                                        dbservice=False, asis=inputs.asis, ocorr_mode=inputs.ocorr_mode)
        importdata_task = almaimportdata.ALMAImportData(container)
        return self._executor.execute(importdata_task, merge=True)
