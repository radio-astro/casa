from __future__ import absolute_import

import types
from pipeline.hif.tasks.gaincal import common
from pipeline.hif.tasks.gaincal import gaincalworker
from pipeline.hif.tasks.gaincal import gaincalmode
from pipeline.hif.tasks.gaincal import gtypegaincal
from pipeline.hif.heuristics import caltable as gcaltable
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary

LOG = infrastructure.get_logger(__name__)

class TimeGaincalInputs(gaincalmode.GaincalModeInputs):
    @basetask.log_equivalent_CASA_call
#    def __init__(self, context, mode='gtype', calamptable=None,
    def __init__(self, context, mode=None, calamptable=None,
            calphasetable=None, amptable=None, targetphasetable=None,
	    calsolint=None, targetsolint=None, calminsnr=None,
	    targetminsnr=None, **parameters):
#        super(TimeGaincalInputs, self).__init__(context, mode=mode,
        super(TimeGaincalInputs, self).__init__(context, mode='gtype',
            calamptable=calamptable, calphasetable=calphasetable,
	    amptable=amptable, targetphasetable=targetphasetable,
            calsolint=calsolint, targetsolint=targetsolint,
            calminsnr=calminsnr, targetminsnr=targetminsnr,
	    **parameters)

    @property
    def calamptable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary

	if self._calamptable is not None:
            return self._calamptable

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('calamptable')

	return gcaltable.GaincalCaltable()

    @calamptable.setter
    def calamptable(self, value):
        self._calamptable = value

    @property
    def calphasetable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary

	if self._calphasetable is not None:
            return self._calphasetable

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('calphasetable')

	return gcaltable.GaincalCaltable()

    @calphasetable.setter
    def calphasetable(self, value):
        self._calphasetable = value

    @property
    def targetphasetable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary

	if self._targetphasetable is not None:
            return self._targetphasetable

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('targetphasetable')

        return gcaltable.GaincalCaltable()

    @targetphasetable.setter
    def targetphasetable(self, value):
        self._targetphasetable = value

    @property
    def amptable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary

	if self._amptable is not None:
            return self._amptable

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('amptable')

        return gcaltable.GaincalCaltable()

    @amptable.setter
    def amptable(self, value):
        self._amptable = value

    @property
    def calsolint(self):
        return self._calsolint

    @calsolint.setter
    def calsolint(self, value):
        if value is None:
            value = 'int'
        self._calsolint = value

    @property
    def calminsnr(self):
        return self._calminsnr

    @calminsnr.setter
    def calminsnr(self, value):
        if value is None:
            value = 2.0
        self._calminsnr = value

    @property
    def targetsolint(self):
        return self._targetsolint

    @targetsolint.setter
    def targetsolint(self, value):
        if value is None:
            value = 'inf'
        self._targetsolint = value

    @property
    def targetminsnr(self):
        return self._targetminsnr

    @targetminsnr.setter
    def targetminsnr(self, value):
        if value is None:
            value = 3.0
        self._targetminsnr = value

class TimeGaincal(gaincalworker.GaincalWorker):
    Inputs = TimeGaincalInputs

    def prepare(self, **parameters):

        # Create a results object.
        result = common.GaincalResults() 

	# Produce the diagnostic table for displaying
	# amplitude vs time plots. 
        calampresult = self._do_caltarget_ampcal()

        # Compute the science target phase solution
        targetphaseresult = self._do_scitarget_phasecal()
        # Readjust to the true calto.intent
        targetphaseresult.pool[0].calto.intent = 'PHASE,TARGET'
        targetphaseresult.final[0].calto.intent = 'PHASE,TARGET'        
        # CalFroms are immutable, so we must replace them with a new 
        # object rather than editing them directly
        self._mod_last_calwt(targetphaseresult.pool[0], False)
        self._mod_last_calwt(targetphaseresult.final[0], False)

        # Adopt the target phase result
        result.pool.extend(targetphaseresult.pool)
        result.final.extend(targetphaseresult.final)

        # Compute the calibrator target phase solution
        # A local merge to context is done here.
        calphaseresult = self._do_caltarget_phasecal()
        # Readjust to the true calto.intent
        calphaseresult.pool[0].calto.intent = 'AMPLITUDE,BANDPASS'
        calphaseresult.final[0].calto.intent = 'AMPLITUDE,BANDPASS'
        self._mod_last_calwt(calphaseresult.pool[0], False)
        self._mod_last_calwt(calphaseresult.final[0], False)

        # Accept calphase result as is.
        result.pool.extend(calphaseresult.pool)
        result.final.extend(calphaseresult.final)

        # Compute the amplitude calibration
        ampresult = self._do_target_ampcal()

        # Accept the amplitude result as is.
        result.pool.extend(ampresult.pool)
        result.final.extend(ampresult.final)


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
        result.error.clear()
        result.error.update(missing)

        return result
    
    # Used to calibrated "selfcaled" targets
    def _do_caltarget_phasecal(self):
        inputs = self.inputs

        task_args = {
          'output_dir'  : inputs.output_dir,
          'vis'         : inputs.vis,
          'caltable'    : inputs.calphasetable,
          'field'       : inputs.field,
          'intent'      : inputs.intent,
          'spw'         : inputs.spw,
          'solint'      : inputs.calsolint,
          'gaintype'    : 'G',
          'calmode'     : 'p',
          'minsnr'      : inputs.calminsnr,
          'combine'     : inputs.combine,
          'refant'      : inputs.refant,
          'minblperant' : inputs.minblperant,
          'solnorm'     : inputs.solnorm,
          'to_intent'   : 'PHASE,TARGET,AMPLITUDE,BANDPASS',
          'to_field'    : None
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
            **task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task, merge=True)
        
        return result

    # Used for diagnostics not calibration
    def _do_caltarget_ampcal(self):
        inputs = self.inputs

        task_args = {
          'output_dir'  : inputs.output_dir,
          'vis'         : inputs.vis,
          'caltable'    : inputs.calamptable,
          'field'       : inputs.field,
          'intent'      : inputs.intent,
          'spw'         : inputs.spw,
          'solint'      : inputs.calsolint,
          'gaintype'    : 'T',
          'calmode'     : 'a',
          'minsnr'      : inputs.calminsnr,
          'combine'     : inputs.combine,
          'refant'      : inputs.refant,
          'minblperant' : inputs.minblperant,
          'solnorm'     : inputs.solnorm,
          'to_intent'   : 'PHASE,TARGET,AMPLITUDE,BANDPASS',
          'to_field'    : None
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
            **task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task)
        
        return result

    def _do_scitarget_phasecal(self):
        inputs = self.inputs

        task_args = {
          'output_dir'  : inputs.output_dir,
          'vis'         : inputs.vis,
          'caltable'    : inputs.targetphasetable,
          'field'       : inputs.field,
          'intent'      : inputs.intent,
          'spw'         : inputs.spw,
          'solint'      : inputs.targetsolint,
          'gaintype'    : 'G',
          'calmode'     : 'p',
          'minsnr'      : inputs.targetminsnr,
          'combine'     : inputs.combine,
          'refant'      : inputs.refant,
          'minblperant' : inputs.minblperant,
          'solnorm'     : inputs.solnorm,
          'to_intent'   : 'PHASE,TARGET',
          'to_field'    : None
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
            ** task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result =  self._executor.execute(gaincal_task)
        
        return result

    def _do_target_ampcal(self):
        inputs = self.inputs

        task_args = {
          'output_dir'  : inputs.output_dir,
          'vis'         : inputs.vis,
          'caltable'    : inputs.amptable,
          'field'       : inputs.field,
          'intent'      : inputs.intent,
          'spw'         : inputs.spw,
          'solint'      : 'inf',
          'gaintype'    : 'T',
          'calmode'     : 'a',
          'minsnr'      : inputs.targetminsnr,
          'combine'     : inputs.combine,
          'refant'      : inputs.refant,
          'minblperant' : inputs.minblperant,
          'solnorm'     : inputs.solnorm,
          'to_intent'   : 'PHASE,TARGET,AMPLITUDE,BANDPASS',
          'to_field'    : None
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
            ** task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result =  self._executor.execute(gaincal_task)

        return result

    def _mod_last_calwt(self, l, calwt):
        l.calfrom[-1] = self._copy_with_calwt(l.calfrom[-1], calwt)

    def _copy_with_calwt(self, old_calfrom, calwt):
        return callibrary.CalFrom(gaintable=old_calfrom.gaintable,
                                  gainfield=old_calfrom.gainfield,
                                  interp=old_calfrom.interp,
                                  spwmap=list(old_calfrom.spwmap),
                                  caltype=old_calfrom.caltype,
                                  calwt=calwt)
