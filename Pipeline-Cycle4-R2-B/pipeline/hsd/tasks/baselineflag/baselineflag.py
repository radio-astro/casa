from __future__ import absolute_import

import os
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.utils as utils
from pipeline.hif.heuristics import fieldnames

from .. import common
from ..common import utils as sdutils

from . import worker
from .flagsummary import SDBLFlagSummary

LOG = infrastructure.get_logger(__name__)

class SDBLFlagInputs(basetask.StandardInputs):
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
                 infiles=None, antenna=None, field=None,
                 spw=None, pol=None):
        self._init_properties(vars())
        self.vis = infiles
        self.infiles = self.vis
        # parameters to convert to float type
        for param in ['iteration', 'tsys_thresh', 'weath_thresh',
                          'prfre_thresh', 'pofre_thresh', 'prfr_thresh',
                          'pofr_thresh', 'prfrm_thresh', 'prfrm_nmean',
                          'pofrm_thresh', 'pofrm_nmean', 'user_thresh']:
            setattr(self, param, sdutils.to_numeric(getattr(self, param)))
        # parameters to convert to int type
        for attr in ['iteration', 'prfrm_nmean', 'pofrm_nmean']:
            value = getattr(self, attr)
            if value is not None:
                setattr(self, attr, int(value))
        # parameters to convert to list type
        for param in ['edge']:
            setattr(self, param, sdutils.to_list(getattr(self, param)))
        # parameters to convert to boolean type
        for param in ['flag_tsys', 'flag_weath', 'flag_prfre',
                       'flag_pofre', 'flag_prfr', 'flag_pofr',
                       'flag_prfrm', 'flag_pofrm', 'flag_user',
                       'plotflag']:
            setattr(self, param, sdutils.to_bool(getattr(self, param)))
        if self.iteration is None: self.iteration = 5
        ### Default Flag rule
        from . import SDFlagRule
        reload(SDFlagRule)
        self.FlagRuleDictionary = SDFlagRule.SDFlagRule
        # update FlagRuleDictionary
        self.__configureFlagRule()

    @property
    def msid_list(self):
        """
        Returns MS index in context observing run specified as infiles.
        """
        ms_names = [ms.name for ms in self.context.observing_run.measurement_sets]
        return map(ms_names.index, map(os.path.abspath, self.infiles))

    @property
    def antenna(self):
        if self._antenna is not None:
            return self._antenna
        ##### never come to this place because setter sets ''
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('antenna')

        antennas = self.ms.get_antenna(self._antenna)
        return ','.join([str(a.id) for a in antennas])

    @antenna.setter
    def antenna(self, value):
        if value is None:
            value = ''
        self._antenna = value

    @property
    def field(self):
        if not callable(self._field):
            return self._field

        # filters field with intents in self.intent
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('field')

        # this will give something like '0542+3243,0343+242'
        intent_fields = self._field(self.ms, self.intent)

        # run the answer through a set, just in case there are duplicates
        fields = set()
        fields.update(utils.safe_split(intent_fields))
        
        return ','.join(fields)

    @field.setter
    def field(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._field = value

    @property
    def intent(self):
        return "TARGET"

    @property
    def spw(self):
        if self._spw is not None:
            return self._spw
        
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('spw')
        # filters science spws by default
        science_spws = self.ms.get_spectral_windows(self._spw, with_channels=True)
        return ','.join([str(spw.id) for spw in science_spws])

    @spw.setter
    def spw(self, value):
        self._spw = value

    @property
    def pol(self):
        if self._pol is not None:
            return self._pol
        ##### never come to this place because setter sets ''
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('pol')
        # filters polarization by self.spw
        selected_spwids = [spwobj.id for spwobj in self.ms.get_spectral_windows(self.spw, with_channels=True)]
        pols = set()
        for idx in selected_spwids:
            pols.update(self.ms.get_data_description(spw=idx).corr_axis)
        
        return ','.join(pols)

    @pol.setter
    def pol(self, value):
        if value is None:
            value = ''
        self._pol = value

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

        

class SDBLFlagResults(common.SingleDishResults):
    """
    The results of SDFalgData
    """
    def __init__(self, task=None, success=None, outcome=None):
        super(SDBLFlagResults, self).__init__(task, success, outcome)
        ### WORKAROUND
        #self.outcome = outcome

    def merge_with_context(self, context):
        super(SDBLFlagResults, self).merge_with_context(context)

#         # replace and export datatable to merge updated data with context
#         datatable = self.outcome.pop('datatable')
#         datatable.exportdata(minimal=True)
# 
#         context.observing_run.datatable_instance = datatable
        
    def _outcome_name(self):
        return 'none'

class SDBLFlag(basetask.StandardTaskTemplate):
    """
    Single dish flagging class.
    """
    ##################################################
    # Note
    # The class uses _handle_multiple_vis framework.
    # Method, prepare() is called per MS. Inputs.ms
    # holds "an" MS instance to be processed.
    ##################################################    
    Inputs = SDBLFlagInputs

    def prepare(self):
        """
        Iterates over reduction group and invoke flagdata worker function in each clip_niteration.
        """
        inputs = self.inputs
        context = inputs.context
        # name of MS to process
        cal_name = inputs.ms.name
        bl_name = inputs.ms.work_data
        in_ant = inputs.antenna
        in_spw = inputs.spw
        in_field = inputs.field
        in_pol = '' if inputs.pol in ['', '*'] else inputs.pol.split(',')
        flag_rule = inputs.FlagRuleDictionary
        clip_niteration = inputs.iteration
        reduction_group = context.observing_run.ms_reduction_group

        rowmap = None
        if os.path.abspath(cal_name)==os.path.abspath(bl_name):
            LOG.warn("%s is not yet baselined. Skipping flag by post-fit statistics for the data. MASKLIST will also be cleared up. You may go on flagging but the statistics will contain line emission." % self.inputs.ms.basename)
        else:
            # row map generation is very expensive. Do as few time as possible
            rowmap = sdutils.make_row_map_for_baselined_ms(self.inputs.ms)
        # sumarize flag before execution
        full_intent = utils.to_CASA_intent(self.inputs.ms, self.inputs.intent)
        flagdata_summary_job = casa_tasks.flagdata(vis=bl_name, mode='summary',
                                                   antenna=in_ant, field=in_field,
                                                   spw=in_spw, intent=full_intent,
                                                   spwcorr=True, fieldcnt=True,
                                                   name='before')
        stats_before = self._executor.execute(flagdata_summary_job)

        # loop over reduction group (spw and source combination)
        flagResult = []
        for (group_id,group_desc) in reduction_group.items():
            LOG.debug('Processing Reduction Group %s'%(group_id))
            LOG.debug('Group Summary:')
            for m in group_desc:
                LOG.debug('\t%s: Antenna %d (%s) Spw %d Field %d (%s)' % \
                          (os.path.basename(m.ms.name), m.antenna_id,
                           m.antenna_name, m.spw_id, m.field_id, m.field_name))

            nchan = group_desc.nchan
            if nchan ==1:
                LOG.info('Skipping a group of channel averaged spw')
                continue
 
            # Which group in group_desc list should be processed
            member_list = list(common.get_valid_ms_members(group_desc, [cal_name], in_ant, in_field, in_spw))
            LOG.trace('group %s: member_list=%s'%(group_id, member_list))
            
            # skip this group if valid member list is empty
            if len(member_list) == 0:
                LOG.info('Skip reduction group %d'%(group_id))
                continue
 
            member_list.sort() #list of group_desc IDs to flag
            antenna_list = [group_desc[i].antenna_id for i in member_list]
            spwid_list = [group_desc[i].spw_id for i in member_list]
            ms_list = [group_desc[i].ms for i in member_list]
            fieldid_list = [group_desc[i].field_id for i in member_list]
            temp_dd_list = [ms_list[i].get_data_description(spw=spwid_list[i]) \
                       for i in xrange(len(member_list))]
            pols_list = [[corr for corr in ddobj.corr_axis if (in_pol=='' or corr in in_pol) ] \
                         for ddobj in temp_dd_list]
            del temp_dd_list
             
            LOG.info("*"*60)
            LOG.info('Members to be processed:')
            for i in xrange(len(member_list)):
                LOG.info("\t%s: Antenna %d (%s) Spw %d Field %d (%s) Pol '%s'" % \
                          (os.path.basename(ms_list[i].name),
                           antenna_list[i], group_desc[member_list[i]].antenna_name,
                           spwid_list[i], fieldid_list[i],
                           group_desc[member_list[i]].field_name,
                           ','.join(pols_list[i])))
            LOG.info("*"*60)
            # Calculate flag and update DataTable
            flagging_inputs = worker.SDBLFlagWorkerInputs(context, clip_niteration,
                                            ms_list, antenna_list, fieldid_list,
                                            spwid_list, pols_list, nchan, flag_rule,
                                            rowmap=rowmap)
            flagging_task = worker.SDBLFlagWorker(flagging_inputs)

            flagging_results = self._executor.execute(flagging_task, merge=False)
            thresholds = flagging_results.outcome
            # Summary
            renderer = SDBLFlagSummary(context, ms_list,
                                       antenna_list, fieldid_list, spwid_list,
                                       pols_list, thresholds, flag_rule)
            result = self._executor.execute(renderer, merge=False)
            flagResult += result
            
        # Calculate flag fraction after operation.
        flagdata_summary_job = casa_tasks.flagdata(vis=bl_name, mode='summary',
                                                   antenna=in_ant, field=in_field,
                                                   spw=in_spw, intent=full_intent,
                                                   spwcorr=True, fieldcnt=True,
                                                   name='after')
        stats_after = self._executor.execute(flagdata_summary_job)
 
        outcome = {'flagdata_summary': [stats_before, stats_after],
                    'summary': flagResult,
                    'byfield': True} # temporal flag to tell template 
        results = SDBLFlagResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
        return results
 
 
 
    def analyse(self, result):
        return result
