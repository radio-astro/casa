'''
Created on Dec 4, 2017

@author: kana
'''
from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.domain.datatable import absolute_path

import pipeline.h.tasks.importdata.importdata as importdata
import pipeline.hsd.tasks.importdata.inspection as inspection

LOG = infrastructure.get_logger(__name__)


class NROImportDataInputs(importdata.ImportDataInputs):
    def __init__(self, context=None, vis=None, output_dir=None, asis=None, process_caldevice=None, session=None,
                 overwrite=None, nocopy=None, bdfflags=None, save_flagonline=None, lazy=None,
                 with_pointing_correction=None, createmms=None, ocorr_mode=None):
        super(NROImportDataInputs, self).__init__(context=context, vis=vis, output_dir=output_dir, asis=asis,
                                                 process_caldevice=process_caldevice, session=session,
                                                 overwrite=overwrite, nocopy=nocopy, bdfflags=bdfflags, lazy=lazy,
                                                 save_flagonline=save_flagonline, createmms=createmms,
                                                 ocorr_mode=ocorr_mode)
        self.with_pointing_correction = with_pointing_correction

    asis = basetask.property_with_default('asis', 'SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR')
    ocorr_mode = basetask.property_with_default('ocorr_mode', 'ao')
    with_pointing_correction = basetask.property_with_default('with_pointing_correction', True)


class NROImportDataResults(basetask.Results):
    """
    NROImportDataResults is an equivalent class with ImportDataResults. 
    Purpose of NROImportDataResults is to replace QA scoring associated 
    with ImportDataResults with single dish specific QA scoring, which 
    is associated with this class.
    
    ImportDataResults holds the results of the ImportData task. It contains
    the resulting MeasurementSet domain objects and optionally the additional 
    SetJy results generated from flux entries in Source.xml.
    """
    
    def __init__(self, mses=None, reduction_group_list=None, datatable_name=None, setjy_results=None):
        super(NROImportDataResults, self).__init__()
        self.mses = [] if mses is None else mses
        self.reduction_group_list = reduction_group_list
        self.datatable_name = datatable_name
        self.setjy_results = setjy_results
        self.origin = {}
        self.results = importdata.ImportDataResults(mses=mses, setjy_results=setjy_results)
        
    def merge_with_context(self, context):
        self.results.merge_with_context(context)
        for ms in self.mses:
            if ms.antenna_array.name in ('NRO'):
                context.project_summary.telescope = 'NRO'
                context.project_summary.observatory = 'Nobeyama'

        self.__merge_reduction_group(context.observing_run, self.reduction_group_list)
        context.observing_run.ms_datatable_name = self.datatable_name
        
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
                        LOG.info('merge input group %s to group %s'%(myid, group_id))
                        matched_id = group_id
                        LOG.info('number of members before merge: %s'%(len(group_desc)))
                        group_desc.merge(mydesc)
                        LOG.info('number of members after merge: %s'%(len(group_desc)))
                if matched_id == -1:
                    LOG.info('add new group')
                    key = len(observing_run.ms_reduction_group)
                    observing_run.ms_reduction_group[key] = mydesc
           
    def __repr__(self):
        return 'NROImportDataResults:\n\t{0}'.format(
                '\n\t'.join([ms.name for ms in self.mses]))
        


class NROImportData(importdata.ImportData):
    Inputs = NROImportDataInputs 
    
    def prepare(self, **parameters):
        # get results object by running super.prepare()
        results = super(NROImportData, self).prepare()
        
        # per MS inspection
        table_name = absolute_path(os.path.join(self.inputs.context.name,'MSDataTable.tbl'))

        reduction_group_list = []
        for ms in results.mses:
            LOG.debug('Start inspection for %s'%(ms.basename))
            inspector = inspection.SDInspection(table_name, ms=ms)
            reduction_group = self._executor.execute(inspector, merge=False)
            reduction_group_list.append(reduction_group)
            
        # create results object
        myresults = NROImportDataResults(mses=results.mses,
                                        reduction_group_list=reduction_group_list,
                                        datatable_name=table_name,
                                        setjy_results=results.setjy_results)
        
        myresults.origin = results.origin
        return myresults
    
