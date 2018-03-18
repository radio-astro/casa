from __future__ import absolute_import

import os

import pipeline.h.tasks.restoredata.restoredata as restoredata
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import task_registry
from .. import applycal
from ..importdata import importdata as importdata

LOG = infrastructure.get_logger(__name__)


class SDRestoreDataInputs(restoredata.RestoreDataInputs):
    asis = vdp.VisDependentProperty(default='SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR')
    ocorr_mode = vdp.VisDependentProperty(default='ao')

    def __init__(self, context, copytoraw=None, products_dir=None, rawdata_dir=None, output_dir=None, session=None,
                 vis=None, bdfflags=None, lazy=None, asis=None, ocorr_mode=None):
        super(SDRestoreDataInputs, self).__init__(context, copytoraw=copytoraw, products_dir=products_dir,
                                                  rawdata_dir=rawdata_dir, output_dir=output_dir, session=session,
                                                  vis=vis, bdfflags=bdfflags, lazy=lazy, asis=asis,
                                                  ocorr_mode=ocorr_mode)


class SDRestoreDataResults(restoredata.RestoreDataResults):
    def __init__(self, importdata_results=None, applycal_results=None):
        """
        Initialise the results objects.
        """
        super(SDRestoreDataResults, self).__init__(importdata_results, applycal_results)

    def merge_with_context(self, context):
        super(SDRestoreDataResults, self).merge_with_context(context)

        # set k2jy factor to ms domain objects
        if isinstance(self.applycal_results, basetask.ResultsList):
            for result in self.applycal_results:
                self._merge_k2jycal(context, result)
        else:
            self._merge_k2jycal(context, self.applycal_results)
            
    def _merge_k2jycal(self, context, applycal_results):
        for calapp in applycal_results.applied:
            msobj = context.observing_run.get_ms(name=os.path.basename(calapp.vis))
            if not hasattr(msobj, 'k2jy_factor'):
                for _calfrom in calapp.calfrom:
                    if _calfrom.caltype == 'amp' or _calfrom.caltype == 'gaincal':
                        LOG.debug('Adding k2jy factor to {0}'.format(msobj.basename))
                        # k2jy gaincal table
                        k2jytable = _calfrom.gaintable
                        k2jy_factor = {}
                        with casatools.TableReader(k2jytable) as tb:
                            spws = tb.getcol('SPECTRAL_WINDOW_ID')
                            antennas = tb.getcol('ANTENNA1')
                            params = tb.getcol('CPARAM').real
                            nrow = tb.nrows()
                        for irow in xrange(nrow):
                            spwid = spws[irow]
                            antenna = antennas[irow]
                            param = params[:, 0, irow]
                            npol = param.shape[0]
                            antname = msobj.get_antenna(antenna)[0].name
                            dd = msobj.get_data_description(spw=int(spwid))
                            if dd is None:
                                continue
                            for ipol in xrange(npol):
                                polname = dd.get_polarization_label(ipol)
                                k2jy_factor[(spwid, antname, polname)] = 1.0 / (param[ipol] * param[ipol])
                        msobj.k2jy_factor = k2jy_factor
            LOG.debug('msobj.k2jy_factor = {0}'.format(getattr(msobj, 'k2jy_factor', 'N/A')))


@task_registry.set_equivalent_casa_task('hsd_restoredata')
class SDRestoreData(restoredata.RestoreData):
    Inputs = SDRestoreDataInputs

    def prepare(self):
        # run prepare method in the parent class
        results = super(SDRestoreData, self).prepare()
        
        # apply baseline table and produce baseline-subtracted MSs
        
        # apply final flags for baseline-subtracted MSs
        
        sdresults = SDRestoreDataResults(results.importdata_results,
                                         results.applycal_results)
        
        return sdresults

    def _do_importasdm(self, sessionlist, vislist):
        inputs = self.inputs
        # SDImportDataInputs operate in the scope of a single measurement set.
        # To operate in the scope of multiple MSes we must use an
        # InputsContainer.
        container = vdp.InputsContainer(importdata.SDImportData, inputs.context, vis=vislist, session=sessionlist,
                                        save_flagonline=False, lazy=inputs.lazy, bdfflags=inputs.bdfflags,
                                        asis=inputs.asis, ocorr_mode=inputs.ocorr_mode)
        importdata_task = importdata.SDImportData(container)
        return self._executor.execute(importdata_task, merge=True)
    
    def _do_applycal(self):
        inputs = self.inputs
        # SDApplyCalInputs operates in the scope of a single measurement set.
        # To operate in the scope of multiple MSes we must use an
        # InputsContainer.
        container = vdp.InputsContainer(applycal.SDApplycal, inputs.context)
        applycal_task = applycal.SDApplycal(container)
        return self._executor.execute(applycal_task, merge=True)
