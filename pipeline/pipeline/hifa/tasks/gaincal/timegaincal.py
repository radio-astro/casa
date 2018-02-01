from __future__ import absolute_import

import copy
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.vdp as vdp
from pipeline.hif.tasks.gaincal import common
from pipeline.hif.tasks.gaincal import gtypegaincal
from pipeline.hifa.heuristics import exptimes as gexptimes
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)

__all__ = [
    'TimeGaincalInputs',
    'TimeGaincal',
    'TimeGaincalResults'
]


class TimeGaincalInputs(gtypegaincal.GTypeGaincalInputs):

    calamptable = vdp.VisDependentProperty(default=None)
    calphasetable = vdp.VisDependentProperty(default=None)
    targetphasetable = vdp.VisDependentProperty(default=None)
    offsetstable = vdp.VisDependentProperty(default=None)
    amptable = vdp.VisDependentProperty(default=None)
    calsolint = vdp.VisDependentProperty(default='int')
    calminsnr = vdp.VisDependentProperty(default=2.0)
    targetsolint = vdp.VisDependentProperty(default='inf')
    targetminsnr = vdp.VisDependentProperty(default=3.0)

    def __init__(self, context, vis=None, output_dir=None, calamptable=None, calphasetable=None,
        offsetstable=None, amptable=None, targetphasetable=None, calsolint=None,
        targetsolint=None, calminsnr=None, targetminsnr=None, **parameters):
        super(TimeGaincalInputs, self).__init__(context, vis=vis, output_dir=output_dir,  **parameters)
        self.calamptable = calamptable
        self.calphasetable = calphasetable
        self.targetphasetable = targetphasetable
        self.offsetstable = offsetstable
        self.amptable = amptable
        self.calsolint = calsolint
        self.calminsnr = calminsnr
        self.targetsolint = targetsolint
        self.targetminsnr = targetminsnr


@task_registry.set_equivalent_casa_task('hifa_timegaincal')
@task_registry.set_casa_commands_comment('Time dependent gain calibrations are computed.')
class TimeGaincal(gtypegaincal.GTypeGaincal):
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
            exptimes = gexptimes.get_scan_exptimes(inputs.ms, fieldnamelist, 'PHASE', spwidlist)
            phase_calsolint = '%0.3fs' % (min([exptime[1] for exptime in exptimes]) / 4.0)
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
        amp_interp = 'nearest,linear'

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

        # Compute an spw mapping diagnostic table which preapplies the
        # previous table phase table if any spw mapping was done
        # Compute it for maps including the default map
        #     The solution interval is set to 'inf'
        #     The gaintype 'T' or 'G' is set to the gaintype of the parent phase table
        #     Spw combine is off
        #     Unique name generated internally
        phaseoffsetresult = self._do_offsets_phasecal(solint='inf',
            gaintype=phase_gaintype, combine='')
        result.phaseoffsetresult = phaseoffsetresult

        # Readjust to the true calto.intent
        calphaseresult.pool[0].calto.intent = 'AMPLITUDE,BANDPASS'
        calphaseresult.final[0].calto.intent = 'AMPLITUDE,BANDPASS'

        # Accept calphase result as is.
        result.pool.extend(calphaseresult.pool)
        result.final.extend(calphaseresult.final)

        # Compute the amplitude calibration
        #   Make a deep copy of the results
        ampresult = self._do_target_ampcal()

        # The solint for this table is always 'inf'
        #    Use nearest interpolation for the flux, bandpass, and
        #    phase calibraters, the standard linear interpolation
        #    for the check and target sources.
        ampresult2 = copy.deepcopy(ampresult)
        ampresult.pool[0].calto.intent='AMPLITUDE,BANDPASS,PHASE'
        self._mod_last_interp(ampresult.pool[0], amp_interp)
        self._mod_last_interp(ampresult.final[0], amp_interp)
        ampresult2.pool[0].calto.intent='CHECK,TARGET'
        result.pool.extend(ampresult2.pool)
        result.final.extend(ampresult2.final)

        # Accept the amplitude results
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
            'solnorm'     : inputs.solnorm
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
                                                      **task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task)

        return result

    # Used to compute spw mapping diagnostic table
    def _do_offsets_phasecal(self, solint=None, gaintype=None, combine=None):
        inputs = self.inputs

        task_args = {
            'output_dir'  : inputs.output_dir,
            'vis'         : inputs.vis,
            'caltable'    : inputs.offsetstable,
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
            'solnorm'     : inputs.solnorm
        }

        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
                                                      **task_args)
        # Create a unique table nmae
        root, ext = os.path.splitext(task_inputs.caltable)
        task_inputs.caltable = '{!s}.{!s}{!s}'.format(root, 'offsets', ext)

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
            'solnorm'     : inputs.solnorm
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
            'solnorm'     : inputs.solnorm
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
                                                      **task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task)

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
            'solnorm'     : inputs.solnorm
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
                                                      **task_args)

        gaincal_task = gtypegaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task)

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
