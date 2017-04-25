from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

import pipeline.h.tasks.restoredata.restoredata as restoredata
from ..importdata import importdata as importdata
from .. import applycal

LOG = infrastructure.get_logger(__name__)

class SDRestoreDataInputs(restoredata.RestoreDataInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, copytoraw=None, products_dir=None,
                 rawdata_dir=None, output_dir=None, session=None, vis=None,
                 bdfflags=None, lazy=None, asis=None, ocorr_mode=None):
        self._init_properties(vars())

    asis = basetask.property_with_default('asis', 'Antenna Station Receiver Source CalAtmosphere CalWVR')
    ocorr_mode = basetask.property_with_default('ocorr_mode', 'ao')


class SDRestoreData(restoredata.RestoreData):
    Inputs = SDRestoreDataInputs

    def prepare(self):
        
        # run prepare method in the parent class
        results = super(SDRestoreData, self).prepare()
        
        # apply baseline table and produce baseline-subtracted MSs
        
        # apply final flags for baseline-subtracted MSs
        
        return results

    def _do_importasdm(self, sessionlist, vislist):
        inputs = self.inputs
        importdata_inputs = importdata.SDImportData.Inputs(inputs.context,
            vis=vislist, session=sessionlist, save_flagonline=False,
            lazy=inputs.lazy, bdfflags=inputs.bdfflags,
            asis=inputs.asis, ocorr_mode=inputs.ocorr_mode)
        importdata_task = importdata.SDImportData(importdata_inputs)
        return self._executor.execute(importdata_task, merge=True)
    
    def _do_applycal(self):
        inputs = self.inputs
        applycal_inputs = applycal.SDMSApplycal.Inputs(inputs.context)
        applycal_task = applycal.SDMSApplycal(applycal_inputs)
        return self._executor.execute(applycal_task, merge=True)

