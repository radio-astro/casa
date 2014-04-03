from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

from .. import common
from ..baseline import baseline
from ..flagdata import flagdata

LOG = infrastructure.get_logger(__name__)

class SDFlagBaselineInputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, infiles=None, iflist=None, pollist=None,
                 linewindow=None, edge=None, broadline=None, fitorder=None,
                 fitfunc=None, output_dir=None,
                 iteration=None, flag_iteration=None, flag_tsys=None, tsys_thresh=None,
                 flag_weath=None, weath_thresh=None,
                 flag_prfre=None, prfre_thresh=None,
                 flag_pofre=None, pofre_thresh=None,
                 flag_prfr=None, prfr_thresh=None,
                 flag_pofr=None, pofr_thresh=None,
                 flag_prfrm=None, prfrm_thresh=None, prfrm_nmean=None,
                 flag_pofrm=None, pofrm_thresh=None, pofrm_nmean=None,
                 flag_user=None, user_thresh=None,
                 plotflag=None):
        self._init_properties(vars())
        self._to_list(['infiles', 'iflist', 'pollist', 'edge', 'linewindow'])
        self._to_bool('broadline')
        self._to_numeric('fitorder')
        if isinstance(self.fitorder, float):
            self.fitorder = int(self.fitorder)
            
    @property
    def iteration(self):
        return self._iteration if self._iteration is not None else 2
    
    @iteration.setter
    def iteration(self, value):
        self._iteration = value
        
    @property
    def flag_iteration(self):
        return self._flag_iteration if self._flag_iteration is not None else 5
    
    @flag_iteration.setter
    def flag_iteration(self, value):
        self._flag_iteration = value

    @property
    def baseline_inputs(self):
        return baseline.SDBaseline.Inputs(self.context, infiles=self.infiles, 
                                          iflist=self.iflist, pollist=self.pollist,
                                          linewindow=self.linewindow, edge=self.edge, 
                                          broadline=self.broadline, fitorder=self.fitorder,
                                          fitfunc=self.fitfunc)
        
    @property
    def flagdata_inputs(self):
        return flagdata.SDFlagData.Inputs(self.context, output_dir=self.output_dir,
                                          iteration=self.flag_iteration, edge=self.edge,
                                          flag_tsys=self.flag_tsys, tsys_thresh=self.tsys_thresh,
                                          flag_weath=self.flag_weath, weath_thresh=self.weath_thresh,
                                          flag_prfre=self.flag_prfre, prfre_thresh=self.prfre_thresh,
                                          flag_pofre=self.flag_pofre, pofre_thresh=self.pofre_thresh,
                                          flag_prfr=self.flag_prfr, prfr_thresh=self.prfr_thresh,
                                          flag_pofr=self.flag_pofr, pofr_thresh=self.pofr_thresh,
                                          flag_prfrm=self.flag_prfrm, prfrm_thresh=self.prfrm_thresh, 
                                          prfrm_nmean=self.prfrm_nmean,
                                          flag_pofrm=self.flag_pofrm, pofrm_thresh=self.pofrm_thresh, 
                                          pofrm_nmean=self.pofrm_nmean,
                                          flag_user=self.flag_user, user_thresh=self.user_thresh,
                                          plotflag=self.plotflag,
                                          infiles=self.infiles, iflist=self.iflist, pollist=self.pollist)

class SDFlagBaselineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDFlagBaselineResults, self).__init__(task, success, outcome)
        
    @property
    def baseline_result(self):
        if self.outcome.has_key('baseline'):
            return self.outcome['baseline']
        else:
            return None
        
    @property
    def flagdata_result(self):
        if self.outcome.has_key('flagdata'):
            return self.outcome['flagdata']
        else:
            return None    

    def merge_with_context(self, context):
        super(SDFlagBaselineResults, self).merge_with_context(context)

    def _outcome_name(self):
        baseline_result = self.baseline_result
        flagdata_result = self.flagdata_result
        baseline_name = ','.join(baseline_result._outcome_name()) if baseline_result is not None else ''
        flagdata_name = flagdata_result._outcome_name() if flagdata_result is not None else ''
        return '\n'.join([baseline_name, flagdata_name])

class SDFlagBaseline(common.SingleDishTaskTemplate):
    Inputs = SDFlagBaselineInputs

    def prepare(self):
        LOG.debug('Start SDFlagBaseline.prepare')
        context = self.context
        iteration = self.inputs.iteration
        inputs = self.inputs
        
        result = basetask.ResultsList()
        
        baseline_inputs = inputs.baseline_inputs
        flagdata_inputs = inputs.flagdata_inputs
        
        for iter in xrange(iteration):
            LOG.debug('iteration %s'%(iter))
            # Run baseline task
            baseline_task = baseline.SDBaseline(baseline_inputs)
            baseline_result = self._executor.execute(baseline_task, merge=True)
            
            # Run flagdata task
            flagdata_task = flagdata.SDFlagData(flagdata_inputs)
            flagdata_result = self._executor.execute(flagdata_task, merge=True)
            
            LOG.todo('maybe we need to generate plots for flagbaseline here')
            
            outcome = {'baseline': baseline_result,
                       'flagdata': flagdata_result}
            r = SDFlagBaselineResults(task=self.__class__,
                                      success=True,
                                      outcome=outcome)

            result.append(r)
                        
        result.task = self.__class__
        
        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 
        
        LOG.todo('logrecords for SDFlagBaseline must be handled properly')
        for r in result:
            r.task = result.task
            r.logrecords = []
            r.stage_number = result.stage_number
            
        return result

    def analyse(self, result):
        return result


