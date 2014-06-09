from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.jobrequest as jobrequest
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from .. import common

from .worker import SDFlagDataWorker
from .flagsummary import SDFlagSummary

LOG = infrastructure.get_logger(__name__)

class SDFlagDataInputs(common.SingleDishInputs):
    """
    Inputs for single dish flagging
    """
    @basetask.log_equivalent_CASA_call
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
                 spw=None, pol=None):#, scanlist=None):
        self._init_properties(vars())
        self._to_numeric(['iteration', 'tsys_thresh', 'weath_thresh',
                          'prfre_thresh', 'pofre_thresh', 'prfr_thresh',
                          'pofr_thresh', 'prfrm_thresh', 'prfrm_nmean',
                          'pofrm_thresh', 'pofrm_nmean', 'user_thresh'])
        for attr in ['iteration', 'prfrm_nmean', 'pofrm_nmean']:
            value = getattr(self, attr)
            if value is not None:
                setattr(self, attr, int(value))
        for key in ['spw', 'pol']:
            val = getattr(self, key)
            if val is None or (val[0] == '[' and val[-1] == ']'):
                self._to_list([key])
        self._to_list(['infiles', 'edge'])
        #self._to_list(['edge', 'infiles', 'iflist', 'pollist'])
        self._to_bool(['flag_tsys', 'flag_weath', 'flag_prfre',
                       'flag_pofre', 'flag_prfr', 'flag_pofr',
                       'flag_prfrm', 'flag_pofrm', 'flag_user',
                       'plotflag'])
        if self.iteration is None: self.iteration = 5
        ### Default Flag rule
        #import SDFlagRule
        from . import SDFlagRule
        reload(SDFlagRule)
        self.FlagRuleDictionary = SDFlagRule.SDFlagRule
        # update FlagRuleDictionary
        self.__configureFlagRule()

    @property
    def antennalist(self):
        """A helper function to convert a list of input files to that of antenna names"""
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

    def __configureFlagRule(self):
        """A private method to convert input parameters to FlagRuleDictionary"""
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
                # Don't touch operation flag but need to update thresholds.
                for i in xrange(len(p)):
                    if p[i] is not None: self.FlagRuleDictionary[k][keys[i]] = p[i] 
            elif b == True:
                self.activateFlagRule( k )
                for i in xrange(len(p)):
                    if p[i] is not None: self.FlagRuleDictionary[k][keys[i]] = p[i]
            elif b == False:
                self.deactivateFlagRule( k )

    def activateFlagRule(self,key):
        """Activates a flag type specified by the input parameter in FlagRuleDictionary"""
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['isActive'] = True
        else:
            print 'Error not in predefined Flagging Rules'

    def deactivateFlagRule(self,key):
        """Deactivates a flag type specified by the input parameter in FlagRuleDictionary"""
        if(key in self.FlagRuleDictionary.keys()):
           self.FlagRuleDictionary[key]['isActive'] = False
        else:
            print 'Error not in predefined Flagging Rules'

        

class SDFlagDataResults(common.SingleDishResults):
    """
    The results of SDFalgData
    """
    def __init__(self, task=None, success=None, outcome=None):
        super(SDFlagDataResults, self).__init__(task, success, outcome)
        ### WORKAROUND
        #self.outcome = outcome

    def merge_with_context(self, context):
        super(SDFlagDataResults, self).merge_with_context(context)

        # replace and export datatable to merge updated data with context
        datatable = self.outcome.pop('datatable')
        datatable.exportdata(minimal=True)

        context.observing_run.datatable_instance = datatable
        
    def _outcome_name(self):
        return 'none'

class SDFlagData(common.SingleDishTaskTemplate):
    """
    Single dish flagging class
    """
    Inputs = SDFlagDataInputs

    def prepare(self):
        """
        Iterates over reduction group and invoke flagdata worker function in each iteration.
        """
        # self
        inputs = self.inputs
        context = inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        # TODO: make sure the reduction group is defined by hsd_inspect()?
        infiles = inputs.infiles
        #antennalist = self.antennalist
        #field = inputs.field        
        #iflist = inputs.iflist
        #pollist = inputs.pollist
        args = inputs.to_casa_args()
        #scanlist = inputs.scanlist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]
        flag_rule = inputs.FlagRuleDictionary
        iteration = inputs.iteration

        # loop over reduction group
        files = set()
        flagResult = []
        for (group_id,group_desc) in reduction_group.items():
            LOG.debug('Processing Reduction Group %s'%(group_id))
            LOG.debug('Group Summary:')
            for m in group_desc:
                LOG.debug('\tAntenna %s Spw %s Pol %s'%(m.antenna, m.spw, m.pols))

            # assume all members have same spw and pollist
            first_member = group_desc[0]
            #spwid = first_member.spw
            #LOG.debug('spwid = %s'%(spwid))
            #pols = first_member.pols
            ###iteration = first_member.iteration[0]
            #if pollist is not None:
            #    pols = list(set(pollist) & set(pols))

            nchan = group_desc.nchan
            if nchan ==1:
                LOG.info('Skip channel averaged spw %s' % (spwid))
                continue

            pols_list = list(common.pol_filter(group_desc, inputs.get_pollist))
            LOG.debug('pols_list=%s'%(pols_list))


            member_list = list(common.get_valid_members(group_desc, file_index, args['spw']))
            # skip this group if valid member list is empty
            if len(member_list) == 0:
                LOG.info('Skip reduction group %d'%(group_id))
                continue

            member_list.sort()
            _file_index = [group_desc[i].antenna for i in member_list]
            spwid_list = [group_desc[i].spw for i in member_list]
            pols_list = [pols_list[i] for i in member_list]
            
            # selection by infiles
            if len(_file_index) < 1:
                LOG.debug('Skip reduction group %d'%(group_id))
                continue

            # accumulate files IDs processed
            files = files | set(_file_index)
            


            LOG.debug('Members to be processed:')
            for i in xrange(len(member_list)):
                LOG.debug('\tAntenna %s Spw %s Pol %s'%(_file_index[i], spwid_list[i], pols_list[i]))

            # skip spw not included in iflist
            #if len(spwid_list) == 0:
            #    LOG.info('Skip spw %d (not in iflist)'%(spwid_list))
            #    continue

            # skip polarizations not included in pollist
            #if pollist is not None and len(pols)==0:
            #    LOG.info('Skip pols %s (not in pollist)'%(str(first_member.pols)))
            #    continue

            LOG.info("*"*60)
            LOG.info("Start processing reduction group %d" % (group_id))
            LOG.debug("- file indices = %s" % str(_file_index))
            LOG.info("- scantable names: %s" % (", ".join([st_names[id] for id in _file_index])))
            LOG.info("- spw: %s" % spwid_list)
            LOG.info("- pols: %s" % str(pols_list))
            #LOG.debug("- additional selections:")
            #LOG.debug("       scanlist: %s" % str(scanlist))
            #LOG.debug("       field: %s" % field)
            LOG.info("*"*60)

            worker = SDFlagDataWorker(context, datatable, iteration, spwid_list, nchan, pols_list, _file_index, flag_rule)
            thresholds = self._executor.execute(worker, merge=False)
            # Summary
            renderer = SDFlagSummary(context, datatable, spwid_list, pols_list, _file_index, thresholds, flag_rule)
            result = self._executor.execute(renderer, merge=False)
            flagResult += result
            
            # Validation


        outcome = {'datatable': datatable,
                   'summary': flagResult}
        results = SDFlagDataResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
#         #stage_number is taken care of by basetask.result_finaliser
#         if self.inputs.context.subtask_counter is 0: 
#             results.stage_number = self.inputs.context.task_counter - 1
#         else:
#             results.stage_number = self.inputs.context.task_counter 
                
        return results
 
 
 
    def analyse(self, result):
        return result
