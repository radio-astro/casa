'''
Created on 2013/06/23

@author: kana
'''
from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.jobrequest as jobrequest
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from .. import common

from .worker import SDFlagDataWorker

LOG = infrastructure.get_logger(__name__)

class SDFlagDataInputs(common.SingleDishInputs):
    """
    Inputs for single dish flagging
    """
    def __init__(self, context, output_dir=None,
                 iteration=None, edge=None, flag_tsys=None, tsys_thresh=None,
                 flag_weath=None, weath_thresh=None,
                 flag_prfre=None, prfre_thresh=None,
                 flag_pofre=None, pofre_thresh=None,
                 flag_prfr=None, prfr_thresh=None,
                 flag_pofr=None, pofr_thresh=None,
                 flag_prfrm=None, prfrm_thresh=None, prfrm_nmean=None,
                 flag_pofrm=None, pofrm_thresh=None, pofrm_nmean=None,
                 flag_user=None, user_thresh=None,
                 plotflag=None,
                 infiles=None, #field=None,
                 iflist=None, pollist=None, scanlist=None):
        self._init_properties(vars())
        ### Default Flag rule
        #import SDFlagRule
        from . import SDFlagRule
        reload(SDFlagRule)
        self.FlagRuleDictionary = SDFlagRule.SDFlagRule
        # update FlagRuleDictionary
        self.__configureFlagRule()

    @property
    def antennalist(self):
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

    def __configureFlagRule(self):
        d = { 'TsysFlag': (self.flag_tsys, [self.tsys_thresh]),
              'WeatherFlag': (self.flag_weath, [self.weath_thresh]),
              'UserFlag': (self.flag_user, [self.user_thresh]),
              'RmsPreFitFlag': (self.flag_prfr, [self.prfr_thresh]),
              'RmsPostFitFlag': (self.flag_pofr, [self.pofr_thresh]),
              'RmsExpectedPreFitFlag': (self.flag_prfre, [self.prfre_thresh]),
              'RmsExpectedPostFitFlag': (self.flag_pofre, [self.pofre_thresh]),
              'RunMeanPreFitFlag': (self.flag_prfrm, [self.prfrm_thresh, self.prfrm_nmean]),
              'RunMeanPostFitFlag': (self.flag_pofrm, [self.pofrm_thresh, self.pofrm_nmean]) }
        keys = ['Threshold', 'Nmean']
        for (k,v) in d.items():
            (b,p) = v
            if b is None:
                pass
            elif b == True:
                self.activateFlagRule( k )
                for i in xrange(len(p)):
                    self.FlagRuleDictionary[k][p[i]] = p[i]
            elif b == False:
                self.deactivateFlagRule( k )

    def activateFlagRule(self,key):
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['isActive'] = True
        else:
            print 'Error not in predefined Flagging Rules'

    def deactivateFlagRule(self,key):
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['isActive'] = False
        else:
            print 'Error not in predefined Flagging Rules'

        

class SDFlagDataResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDFlagDataResults, self).__init__(task, success, outcome)
        ### WORKAROUND
        #self.outcome = outcome

    def merge_with_context(self, context):
        super(SDFlagDataResults, self).merge_with_context(context)

#    def _outcome_name(self): pass

class SDFlagData(common.SingleDishTaskTemplate):
    Inputs = SDFlagDataInputs

    def prepare(self):
        # self
        inputs = self.inputs
        context = inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        # TODO: make sure the reduction group is defined by hsd_inspect()?
        infiles = inputs.infiles
        #antennalist = self.antennalist
        #field = inputs.field        
        iflist = inputs.iflist
        pollist = inputs.pollist
        scanlist = inputs.scanlist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]
        flag_rule = inputs.FlagRuleDictionary

        # loop over reduction group
        files = set()
        for (group_id,group_desc) in reduction_group.items():
            # selection by infiles
            _file_index = set(file_index) & set([m.antenna for m in group_desc])
            if len(_file_index) < 1:
                Log.debug('Skip reduction group %d'%(group_id))
                continue
            # accumulate files IDs processed
            files = files | _file_index

             # assume all members have same spw and pollist
            first_member = group_desc[0]
            spwid = first_member.spw
            LOG.debug('spwid=%s'%(spwid))
            pols = first_member.pols
            iteration = first_member.iteration[0]
            if pollist is not None:
                pols = list(set(pollist) & set(pols))

            # skip spw not included in iflist
            if iflist is not None and spwid not in iflist:
                LOG.debug('Skip spw %s'%(spwid))
                continue

            # skip polarizations not included in pollist
            if pollist is not None and len(pols)==0:
                LOG.debug('Skip pols %s'%(str(fist_member.pols)))
                continue

            LOG.debug("Group ID = %d" % group_id)
            LOG.debug("- _file_index = %s" % str(_file_index))
            LOG.debug("- file names = %s" % (", ".join([st_names[id] for id in _file_index])))
            LOG.debug("- spw: %d" % spwid)
            LOG.debug("- pols: %s" % str(pols))
            #LOG.debug("- additional selections:")
            #LOG.debug("       scanlist: %s" % str(scanlist))
            #LOG.debug("       field: %s" % field)

            nchan = group_desc.nchan
            worker = SDFlagDataWorker(context)
            parameters = {'datatable': datatable,
                          'iteration': iteration, 
                          'spwid': spwid,
                          'nchan': nchan,
                          'pollist': pols,
                          #'scanlist': scanlist,
                          #'field': field,
                          'file_index': list(_file_index),
                          'flagRule': flag_rule}            
            job = jobrequest.JobRequest(worker.execute, **parameters)
            self._executor.execute(job)
            
            # Validation


        outcome = {}#{'datatable': datatable}
        results = SDFlagDataResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
                
        if self.inputs.context.subtask_counter is 0: 
            results.stage_number = self.inputs.context.task_counter - 1
        else:
            results.stage_number = self.inputs.context.task_counter 
                
        return results
 
 
 
    def analyse(self, result):
        return result
