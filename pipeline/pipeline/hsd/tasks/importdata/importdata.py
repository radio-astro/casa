from __future__ import absolute_import

import os

import pipeline.h.tasks.importdata.importdata as importdata
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.domain.datatable import absolute_path
from pipeline.infrastructure import task_registry
from . import inspection

LOG = infrastructure.get_logger(__name__)


class SDImportDataInputs(importdata.ImportDataInputs):
    asis = vdp.VisDependentProperty(default='SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR')
    ocorr_mode = vdp.VisDependentProperty(default='ao')
    with_pointing_correction = vdp.VisDependentProperty(default=True)
    createmms = vdp.VisDependentProperty(default='false')

    def __init__(self, context=None, vis=None, output_dir=None, asis=None, process_caldevice=None, session=None,
                 overwrite=None, nocopy=None, bdfflags=None, save_flagonline=None, lazy=None,
                 with_pointing_correction=None, createmms=None, ocorr_mode=None):
        super(SDImportDataInputs, self).__init__(context, vis=vis, output_dir=output_dir, asis=asis,
                                                 process_caldevice=process_caldevice, session=session,
                                                 overwrite=overwrite, nocopy=nocopy, bdfflags=bdfflags, lazy=lazy,
                                                 save_flagonline=save_flagonline, createmms=createmms,
                                                 ocorr_mode=ocorr_mode, asimaging=False)
        self.with_pointing_correction = with_pointing_correction


class SDImportDataResults(basetask.Results):
    """
    SDImportDataResults is an equivalent class with ImportDataResults. 
    Purpose of SDImportDataResults is to replace QA scoring associated 
    with ImportDataResults with single dish specific QA scoring, which 
    is associated with this class.
    
    ImportDataResults holds the results of the ImportData task. It contains
    the resulting MeasurementSet domain objects and optionally the additional 
    SetJy results generated from flux entries in Source.xml.
    """
    
    def __init__(self, mses=None, reduction_group_list=None, datatable_prefix=None, setjy_results=None):
        super(SDImportDataResults, self).__init__()
        self.mses = [] if mses is None else mses
        self.reduction_group_list = reduction_group_list
        self.datatable_prefix = datatable_prefix
        self.setjy_results = setjy_results
        self.origin = {}
        self.results = importdata.ImportDataResults(mses=mses, setjy_results=setjy_results)
        
    def merge_with_context(self, context):
        self.results.merge_with_context(context)
        self.__merge_reduction_group(context.observing_run, self.reduction_group_list)
        context.observing_run.ms_datatable_name = self.datatable_prefix
        
    def __merge_reduction_group(self, observing_run, reduction_group_list):
        if not hasattr(observing_run, 'ms_reduction_group'):
            LOG.info('Adding ms_reduction_group to observing_run')
            observing_run.ms_reduction_group = {}
            
        # merge reduction group
        for reduction_group in reduction_group_list:
            for (myid, mydesc) in reduction_group.iteritems():
                matched_id = -1
                for (group_id, group_desc) in observing_run.ms_reduction_group.iteritems():
                    if group_desc == mydesc:
                        LOG.info('merge input group %s to group %s' % (myid, group_id))
                        matched_id = group_id
                        LOG.info('number of members before merge: %s' % (len(group_desc)))
                        group_desc.merge(mydesc)
                        LOG.info('number of members after merge: %s' % (len(group_desc)))
                if matched_id == -1:
                    LOG.info('add new group')
                    key = len(observing_run.ms_reduction_group)
                    observing_run.ms_reduction_group[key] = mydesc
           
    def __repr__(self):
        return 'SDImportDataResults:\n\t{0}'.format(
                '\n\t'.join([ms.name for ms in self.mses]))
        

@task_registry.set_equivalent_casa_task('hsd_importdata')
@task_registry.set_casa_commands_comment('If required, ASDMs are converted to MeasurementSets.')
class SDImportData(importdata.ImportData):
    Inputs = SDImportDataInputs 
    
    def prepare(self, **parameters):
        # get results object by running super.prepare()
        results = super(SDImportData, self).prepare()
        
        # per MS inspection
        table_prefix = absolute_path(os.path.join(self.inputs.context.name, 'MSDataTable.tbl'))
        reduction_group_list = []
        for ms in results.mses:
            LOG.debug('Start inspection for %s' % (ms.basename))
            table_name = os.path.join(table_prefix, ms.basename)
            inspector = inspection.SDInspection(table_name, ms=ms)
            reduction_group = self._executor.execute(inspector, merge=False)
            reduction_group_list.append(reduction_group)
            
        # create results object
        myresults = SDImportDataResults(mses=results.mses,
                                        reduction_group_list=reduction_group_list,
                                        datatable_prefix=table_prefix,
                                        setjy_results=results.setjy_results)
        
        myresults.origin = results.origin
        return myresults
