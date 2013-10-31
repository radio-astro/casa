from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks
from . import common

LOG = infrastructure.get_logger(__name__)



class GaincalWorkerInputs(common.CommonGaincalInputs):
    def __init__(self, context, output_dir=None,
                 # 
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 selectdata=None,
                 # solution parameters
                 gaintype=None, smodel=None, calmode=None, solint=None,
                 combine=None, refant=None, minblperant=None, minsnr=None,
                 solnorm=None, append=None, scan=None, splinetime=None, npointaver=None,
                 phasewrap=None,
                 # preapply calibrations
                 opacity=None, preavg=None,
                 # calibration target
                 to_intent=None, to_field=None):
        self._init_properties(vars())


class GaincalWorker(basetask.StandardTaskTemplate):
    """
    GaincalWorker performs a simple gain calibration exactly as specified by
    the inputs, with no analysis or parameter refinement.
    
    As this task has no analysis, it is not expected to be used in an
    interactive session. The use-case for this task is as a worker task for 
    higher-level tasks.
    """
    Inputs = GaincalWorkerInputs
    
    def prepare(self):
        # create a local variable for the inputs associated with this instance
        inputs = self.inputs

        # make a note of the current inputs state before we start fiddling
        # with it. This origin will be attached to the final CalApplication.
        origin = callibrary.CalAppOrigin(task=GaincalWorker, 
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
            # arrange a gaincal job for the data selection
            if ':' in inputs.spw or '~' in inputs.spw:
                inputs.spw = orig_spw
            else:
                inputs.spw = calto.spw
            inputs.field = calto.field
            # kludge to handle more esoteric 'antenna syntax' 
            LOG.todo('handle & and * in antenna selection')
            if '&' in inputs.antenna or '*' in inputs.antenna:
                inputs.antenna = orig_antenna
            else:
                inputs.antenna = calto.antenna

            args = inputs.to_casa_args()

            # set the on-the-fly calibration state for the data selection 
            calapp = callibrary.CalApplication(calto, calfroms)
            args['gaintable'] = calapp.gaintable
            args['gainfield'] = calapp.gainfield
            args['spwmap']    = calapp.spwmap
            args['interp']    = calapp.interp

            jobs.append(casa_tasks.gaincal(**args))

            # append subsequent output to the same caltable 
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
        # from this caltable when applied to other data. Set the table name
        # name (mandatory) and gainfield (to conform to suggested script 
        # standard), leaving spwmap, interp, etc. at their default values.
        calfrom = callibrary.CalFrom(inputs.caltable, 
                                     caltype='gaincal',
                                     gainfield='nearest')

        calapp = callibrary.CalApplication(calto, calfrom, origin)

        result = common.GaincalResults(pool=[calapp])

        return result

    def analyse(self, result):
        # With no best caltable to find, our task is simply to set the one
        # caltable as the best result 

        # double-check that the caltable was actually generated
        on_disk = [table for table in result.pool
                   if table.exists() or self._executor._dry_run]
        result.final[:] = on_disk
        
        missing = [table for table in result.pool
                   if table not in on_disk and not self._executor._dry_run]        
        if missing:
            l = len(missing)
            msg = ('Gaincal output caltable%s %s %s missing' % 
                   ('' if l is 1 else 's',
                    utils.commafy([os.path.basename(ca.gaintable) for ca in missing], False),
                    'is' if l is 1 else 'are'))
            LOG.error(msg)
            raise IOError(msg)

        return result
