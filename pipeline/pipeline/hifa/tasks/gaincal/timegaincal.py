from __future__ import absolute_import
import types

from pipeline.hif.tasks.gaincal import common
from pipeline.hif.tasks.gaincal import gaincalworker
from pipeline.hif.tasks.gaincal import gaincalmode
from pipeline.hif.tasks.gaincal import gtypegaincal
from pipeline.hif.heuristics import caltable as gcaltable
from pipeline.hifa.heuristics import exptimes as gexptimes
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary

LOG = infrastructure.get_logger(__name__)


class TimeGaincalInputs(gaincalmode.GaincalModeInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, mode=None, calamptable=None,
            calphasetable=None, amptable=None, targetphasetable=None,
	    calsolint=None, targetsolint=None, calminsnr=None,
	    targetminsnr=None, **parameters):
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

	# Simplify
	inputs = self.inputs

        # Create a results object.
        result = common.GaincalResults() 

	# Get the spw mapping mode
        if inputs.ms.combine_spwmap:
            spwidlist = [spw.id for spw in inputs.ms.get_spectral_windows(task_arg=inputs.spw, science_windows_only=True)]
            fieldnamelist = [field.name for field in inputs.ms.get_fields(task_arg=inputs.field, intent='PHASE')]
            exptimes = gexptimes.get_scan_exptimes(inputs.ms, fieldnamelist, 'PHASE',
                spwidlist)
            phase_calsolint = '%0.3fs' % (min ([exptime[1] for exptime in exptimes]) / 4.0)
            phase_gaintype = 'T'
            phase_combine = 'spw'
	    phaseup_spwmap = inputs.ms.combine_spwmap
            phase_interp = 'linearPD,linear'
        else:
            phase_calsolint = inputs.calsolint
            phase_gaintype = 'G'
            phase_combine = inputs.combine
	    phaseup_spwmap = inputs.ms.phaseup_spwmap
            phase_interp = 'linear,linear'
        amp_calsolint = phase_calsolint

        # Produce the diagnostic table for displaying amplitude vs time plots. 
        #     This table is not applied to the data
	#     No special mapping required here.
        calampresult = self._do_caltarget_ampcal(solint=amp_calsolint)
        result.calampresult = calampresult

        # Compute the science target phase solution
        targetphaseresult = self._do_scitarget_phasecal(solint=inputs.targetsolint,
           gaintype=phase_gaintype, combine=phase_combine)

        # Readjust to the true calto.intent
        targetphaseresult.pool[0].calto.intent = 'PHASE,CHECK,TARGET'
        targetphaseresult.final[0].calto.intent = 'PHASE,CHECK,TARGET'        

        # CalFroms are immutable, so we must replace them with a new 
        # object rather than editing them directly
        self._mod_last_calwt(targetphaseresult.pool[0], False)
        self._mod_last_calwt(targetphaseresult.final[0], False)
        if inputs.ms.combine_spwmap:
            self._mod_last_interp(targetphaseresult.pool[0], phase_interp)
            self._mod_last_interp(targetphaseresult.final[0], phase_interp)
	if phaseup_spwmap:
            self._mod_last_spwmap(targetphaseresult.pool[0], phaseup_spwmap)
            self._mod_last_spwmap(targetphaseresult.final[0], phaseup_spwmap)

        # Adopt the target phase result
        result.pool.extend(targetphaseresult.pool)
        result.final.extend(targetphaseresult.final)

        # Compute the calibrator target phase solution
        calphaseresult = self._do_caltarget_phasecal(solint=phase_calsolint,
            gaintype=phase_gaintype, combine=phase_combine)

        # CalFroms are immutable, so we must replace them with a new one
        self._mod_last_calwt(calphaseresult.pool[0], False)
        self._mod_last_calwt(calphaseresult.final[0], False)
        if inputs.ms.combine_spwmap:
            self._mod_last_interp(calphaseresult.pool[0], phase_interp)
            self._mod_last_interp(calphaseresult.final[0], phase_interp)
	if phaseup_spwmap:
            self._mod_last_spwmap(calphaseresult.pool[0], phaseup_spwmap)
            self._mod_last_spwmap(calphaseresult.final[0], phaseup_spwmap)

	# Do a local merge of this result.
	calphaseresult.accept(inputs.context)

        # Readjust to the true calto.intent
        calphaseresult.pool[0].calto.intent = 'AMPLITUDE,BANDPASS'
        calphaseresult.final[0].calto.intent = 'AMPLITUDE,BANDPASS'

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
    def _do_caltarget_phasecal(self, solint=None, gaintype=None, combine=None):
        inputs = self.inputs

        task_args = {
          'output_dir'  : inputs.output_dir,
          'vis'         : inputs.vis,
          'caltable'    : inputs.calphasetable,
          'field'       : inputs.field,
          'intent'      : inputs.intent,
          'spw'         : inputs.spw,
          'solint'      : solint,
          'gaintype'    : gaintype,
          'calmode'     : 'p',
          'minsnr'      : inputs.calminsnr,
          'combine'     : combine,
          'refant'      : inputs.refant,
          'minblperant' : inputs.minblperant,
          'solnorm'     : inputs.solnorm,
          'to_intent'   : 'PHASE,CHECK,TARGET,AMPLITUDE,BANDPASS',
          'to_field'    : None
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
                                                      **task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task)
        
        return result

    # Used for diagnostics not calibration
    def _do_caltarget_ampcal(self, solint):
        inputs = self.inputs

        task_args = {
          'output_dir'  : inputs.output_dir,
          'vis'         : inputs.vis,
          'caltable'    : inputs.calamptable,
          'field'       : inputs.field,
          'intent'      : inputs.intent,
          'spw'         : inputs.spw,
          'solint'      : solint,
          'gaintype'    : 'T',
          'calmode'     : 'a',
          'minsnr'      : inputs.calminsnr,
          'combine'     : inputs.combine,
          'refant'      : inputs.refant,
          'minblperant' : inputs.minblperant,
          'solnorm'     : inputs.solnorm,
          'to_intent'   : 'PHASE,CHECK,TARGET,AMPLITUDE,BANDPASS',
          'to_field'    : None
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
                                                      **task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task)
        
        return result

    def _do_scitarget_phasecal(self, solint=None, gaintype=None, combine=None):
        inputs = self.inputs

        task_args = {
          'output_dir'  : inputs.output_dir,
          'vis'         : inputs.vis,
          'caltable'    : inputs.targetphasetable,
          'field'       : inputs.field,
          'intent'      : inputs.intent,
          'spw'         : inputs.spw,
          'solint'      : solint,
          'gaintype'    : gaintype,
          'calmode'     : 'p',
          'minsnr'      : inputs.targetminsnr,
          'combine'     : combine,
          'refant'      : inputs.refant,
          'minblperant' : inputs.minblperant,
          'solnorm'     : inputs.solnorm,
          'to_intent'   : 'PHASE,CHECK,TARGET',
          'to_field'    : None
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
                                                      **task_args)

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
          'to_intent'   : 'PHASE,CHECK,TARGET,AMPLITUDE,BANDPASS',
          'to_field'    : None
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
                                                      **task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result =  self._executor.execute(gaincal_task)

        return result

    def _mod_last_interp(self, l, interp):
        l.calfrom[-1] = self._copy_with_interp(l.calfrom[-1], interp)

    def _copy_with_interp(self, old_calfrom, interp):
        return callibrary.CalFrom(gaintable=old_calfrom.gaintable,
                                  gainfield=old_calfrom.gainfield,
                                  interp=interp,
                                  spwmap=old_calfrom.spwmap,
                                  caltype=old_calfrom.caltype,
                                  calwt=old_calfrom.calwt)

    def _mod_last_spwmap(self, l, spwmap):
        l.calfrom[-1] = self._copy_with_spwmap(l.calfrom[-1], spwmap)

    def _copy_with_spwmap(self, old_calfrom, spwmap):
        return callibrary.CalFrom(gaintable=old_calfrom.gaintable,
                                  gainfield=old_calfrom.gainfield,
                                  interp=old_calfrom.interp,
                                  spwmap=spwmap,
                                  caltype=old_calfrom.caltype,
                                  calwt=old_calfrom.calwt)

    def _mod_last_calwt(self, l, calwt):
        l.calfrom[-1] = self._copy_with_calwt(l.calfrom[-1], calwt)

    def _copy_with_calwt(self, old_calfrom, calwt):
        return callibrary.CalFrom(gaintable=old_calfrom.gaintable,
                                  gainfield=old_calfrom.gainfield,
                                  interp=old_calfrom.interp,
                                  spwmap=list(old_calfrom.spwmap),
                                  caltype=old_calfrom.caltype,
                                  calwt=calwt)
