from __future__ import absolute_import
import os
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.domain as domain
from pipeline.domain.datatable import absolute_path
from . import inspection

import pipeline.hif.tasks.importdata.importdata as importdata
import pipeline.hifa.tasks.importdata.almaimportdata as almaimportdata

LOG = infrastructure.get_logger(__name__)

class SDImportDataInputs(almaimportdata.ALMAImportDataInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context=None, vis=None, output_dir=None,
                 asis=None, process_caldevice=None, session=None, overwrite=None, 
                 bdfflags=None, save_flagonline=None, lazy=None, dbservice=None,
                 with_pointing_correction=None, createmms=None):
        self._init_properties(vars())

    asis = basetask.property_with_default('asis', 'Antenna Station Receiver CalAtmosphere CalWVR')
    with_pointing_correction = basetask.property_with_default('with_pointing_correction', True)

class SDImportDataResults(basetask.Results):
    '''
    SDImportDataResults is an equivalent class with ImportDataResults. 
    Purpose of SDImportDataResults is to replace QA scoring associated 
    with ImportDataResults with single dish specific QA scoring, which 
    is associated with this class.
    
    ImportDataResults holds the results of the ImportData task. It contains
    the resulting MeasurementSet domain objects and optionally the additional 
    SetJy results generated from flux entries in Source.xml.
    '''
    
    def __init__(self, mses=None, reduction_group_list=None, setjy_results=None):
        super(SDImportDataResults, self).__init__()
        self.mses = [] if mses is None else mses
        self.reduction_group_list = reduction_group_list
        self.setjy_results = setjy_results
        self.origin = {}
        self.results = importdata.ImportDataResults(mses=mses, setjy_results=setjy_results)
        
    def merge_with_context(self, context):
        if not isinstance(context.observing_run, domain.ScantableList):
            context.observing_run = domain.ScantableList()
        self.results.merge_with_context(context)
        self.__merge_reduction_group(context.observing_run, self.reduction_group_list)
        
    def __merge_reduction_group(self, observing_run, reduction_group_list):
        if not hasattr(observing_run, 'ms_reduction_group'):
            LOG.info('Adding ms_reduction_group to observing_run')
            observing_run.ms_reduction_group = {}
            
        # merge reduction group
        for reduction_group in reduction_group_list:
            for (myid, mydesc) in reduction_group.items():
                matched_id = -1
                for (group_id, group_desc) in observing_run.ms_reduction_group.items():
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
        return 'SDImportDataResults:\n\t{0}'.format(
                '\n\t'.join([ms.name for ms in self.mses]))
        


class SDImportData(importdata.ImportData):
    Inputs = SDImportDataInputs 
    
    def prepare(self, **parameters):
        # get results object by running super.prepare()
        results = super(SDImportData, self).prepare()
        
        # per MS inspection
        table_name = absolute_path(os.path.join(self.inputs.context.name,'MSDataTable.tbl'))

        reduction_group_list = []
        for ms in results.mses:
            LOG.debug('Start inspection for %s'%(ms.basename))
            inspector = inspection.SDMSInspection(table_name, ms=ms)
            reduction_group = self._executor.execute(inspector, merge=False)
            reduction_group_list.append(reduction_group)
            
        # create results object
        myresults = SDImportDataResults(mses=results.mses,
                                        reduction_group_list=reduction_group_list, 
                                        setjy_results=results.setjy_results)
        
        myresults.origin = results.origin
        return myresults
    
