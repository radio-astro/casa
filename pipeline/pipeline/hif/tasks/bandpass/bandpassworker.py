from __future__ import absolute_import

import os
import shutil

import pipeline.qa2.bpcal as bpcal
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure as infrastructure
from pipeline.infrastructure import casa_tasks
from . import common

LOG = infrastructure.get_logger(__name__)



class BandpassWorkerInputs(common.CommonBandpassInputs):
    """
    BandpassInputs defines the inputs required by the :class:`Bandpass`
    task.
    """
    
    def __init__(self, context, output_dir=None, run_qa2=None,
                 # 
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # solution parameters
                 solint=None, combine=None, refant=None, minblperant=None,
                 minsnr=None, solnorm=None, bandtype=None, fillgaps=None, 
                 degamp=None, degphase=None, visnorm=None, maskcenter=None,
                 maskedge=None, append=None, scan=None,
                 # preapply calibrations
                 opacity=None, parang=None,
                 # calibration target
                 to_intent=None, to_field=None):
        self._init_properties(vars())


class BandpassWorker(basetask.StandardTaskTemplate):
    """
    BandpassWorker performs a simple bandpass calibration exactly as specified
    by the inputs, with no analysis or parameter refinement.
    
    As this task has no analysis, it is not expected to be used in an
    interactive session. The expected use-case for this task is as a worker
    task for higher-level tasks.
    """
    Inputs = BandpassWorkerInputs
    
    def prepare(self):
        # create a local variable for the inputs associated with this instance
        inputs = self.inputs

        # make a note of the current inputs state before we start fiddling
        # with it. This origin will be attached to the final CalApplication.
        origin = callibrary.CalAppOrigin(task=BandpassWorker, 
                                         inputs=inputs.to_casa_args())
        
        # make fast the caltable name by manually setting it to its current 
        # value. This makes the caltable name permanent, so we can 
        # subsequently append calibrations for each spectral window to the one
        # table
        inputs.caltable = inputs.caltable

        # create a job for each CalTo data selection
        calstate = inputs.context.callibrary.get_calstate(inputs.calto)                

        # make a memo of the original spw input. These are the spws we will
        # apply the resultant caltable to
        orig_spw = inputs.spw
        orig_antenna = inputs.antenna
        
        jobs = []
        for calto, calfroms in calstate.merged().items():
            # arrange a bandpass job for the data selection
            inputs.spw = calto.spw
            inputs.field = calto.field
#            inputs.intent = calto.intent
            inputs.antenna = calto.antenna

            args = inputs.to_casa_args()

            # set the on-the-fly calibration state for the data selection 
            calapp = callibrary.CalApplication(calto, calfroms)
            args['gaintable'] = calapp.gaintable
            args['gainfield'] = calapp.gainfield
            args['spwmap']    = calapp.spwmap
            args['interp']    = calapp.interp
            
            jobs.append(casa_tasks.bandpass(**args))

            # append subsequent bandpass output to the same caltable 
            inputs.append = True

        # execute the jobs
        for job in jobs:
            self._executor.execute(job)

        # create the data selection target defining which data this caltable 
        # should calibrate 
        calto = callibrary.CalTo(vis=inputs.vis,
                                 spw=orig_spw)
                                 #field=inputs.to_field,
                                 #intent=inputs.to_intent,
                                 #antenna=orig_antenna)

        # create the calfrom object describing which data should be selected
        # from this caltable when applied to other data. Just set the table
        # name, leaving spwmap, interp, etc. at their default values.
        calfrom = callibrary.CalFrom(inputs.caltable, caltype='bandpass')

        calapp = callibrary.CalApplication(calto, calfrom, origin)

        result = common.BandpassResults(pool=[calapp])

        return result

    def analyse(self, result):
        # With no best caltable to find, our task is simply to set the one
        # caltable as the best result 

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        result.final[:] = on_disk
        
        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]        
        result.error.clear()
        result.error.update(missing)

        if self.inputs.context.subtask_counter is 0: 
            stage_number = self.inputs.context.task_counter - 1
        else:
            stage_number = self.inputs.context.task_counter   
                         
        LOG.trace('Setting stage number to %s for bandpass' % stage_number)

        qa2_dir = os.path.join(self.inputs.context.report_dir,
                               'stage%s' % stage_number,
                               'qa2')

        # QA2 scores are deemed unreliable for Cycle 1, so turn them off.
#         if self.inputs.run_qa2 is True:
        LOG.todo('Re-enable bandpass QA2 analysis with meaningful scores')
        if False:
            if not os.path.exists(qa2_dir):
                os.makedirs(qa2_dir)
    
            for calapp in on_disk:
                (root, _) = os.path.splitext(os.path.basename(calapp.gaintable))
                qa2_file = os.path.join(qa2_dir, root + '.bpcal.stats')
                if os.path.exists(qa2_file):
#                    LOG.info('QA2 statistics table exists. Assuming QA2 has '
#                             'been done before')
                    LOG.info('Removing existing QA2 statistics table from %s'
                             % qa2_file)
                    shutil.rmtree(qa2_file)

                try:
                    qa2_results = bpcal.bpcal(calapp.gaintable, qa2_dir)
                    result.qa2.update(qa2_results)
                except Exception as e:
                    LOG.error('Problem occurred running QA2 analysis. QA2 '
                              'results will not be available for this task')
                    LOG.exception(e)

        return result
