from __future__ import absolute_import

import types

from pipeline.hif.tasks.gaincal import common
from pipeline.hif.tasks.gaincal import gaincalmode
from pipeline.hif.tasks.gaincal import gaincalworker
from pipeline.hif.tasks.gaincal import gtypegaincal

from pipeline.hifa.heuristics.phasespwmap import simple_n2wspwmap
from pipeline.hifa.heuristics.phasespwmap import combine_spwmap
from pipeline.hifa.tasks.gaincalsnr import gaincalsnr

from pipeline.hif.heuristics import caltable as gcaltable

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary

LOG = infrastructure.get_logger(__name__)

class SpwPhaseupInputs(gaincalmode.GaincalModeInputs):

    # Set the spw map determination parameters here.
    intent = basetask.property_with_default('intent', 'BANDPASS')
    hm_spwmapmode = basetask.property_with_default('hm_spwmapmode', 'simple')

    # Auto mode mapping parameters
    phasesnr = basetask.property_with_default('phasesnr', 25.0)

    # Combine mode mapping parameters
    bwedgefrac = basetask.property_with_default('bwedgefrac', 0.03125)
    hm_nantennas = basetask.property_with_default('hm_nantennas', 'all')
    maxfracflagged = basetask.property_with_default('unflagged', 0.90)

    # Simple mode mapping parameter
    maxnarrowbw = basetask.property_with_default('maxnarrowbw', '300MHz')
    minfracmaxbw = basetask.property_with_default('minfracmaxbw', 0.8)
    samebb = basetask.property_with_default('samebb', True)

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, mode=None, caltable=None, intent=None, hm_spwmapmode=None,
        phasesnr=None, bwedgefrac=None, hm_nantennas=None, maxfracflagged=None,
        maxnarrowbw=None, minfracmaxbw=None, samebb=None, **parameters):
        super(SpwPhaseupInputs, self).__init__(context, mode='gtype',
            caltable=caltable, intent=intent, hm_spwmapmode=hm_spwmapmode,
            phasesnr=phasesnr, bwedgefrac=bwedgefrac, hm_nantennas=hm_nantennas,
            maxfracflagged=maxfracflagged,maxnarrowbw=maxnarrowbw, minfracmaxbw=minfracmaxbw,
            samebb=samebb, **parameters)

    @property
    def caltable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary

        if self._caltable is not None:
            return self._caltable

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('caltable')

        return gcaltable.GaincalCaltable()

    @caltable.setter
    def caltable(self, value):
        self._caltable = value


class SpwPhaseup(gaincalworker.GaincalWorker):
    Inputs = SpwPhaseupInputs

    def prepare(self, **parameters):

	# Simplify the inputs
	inputs = self.inputs

        # Get a list of all the spws and a list of the science spws
        allspws = inputs.ms.get_spectral_windows(task_arg=inputs.spw,
            science_windows_only=False)
        scispws = inputs.ms.get_spectral_windows(task_arg=inputs.spw,
            science_windows_only=True)

	# Compute the spw map according to the rules defined by each
        # mapping mode. The default map is [] which stands for the
	# default one to one spw mapping.
	LOG.info('The spw mapping mode for %s is %s' % \
            (inputs.ms.basename, inputs.hm_spwmapmode))
        if inputs.hm_spwmapmode == 'auto':
            lowsnr, snrtest_result = self._do_snrtest()
            if lowsnr:
                LOG.info('    Low SNR condition met')
	        combinespwmap = combine_spwmap (allspws, scispws, scispws[0].id)
	        LOG.info('    The combine spw map is %s' % (combinespwmap))
	        phaseupspwmap = []
            elif (len(snrtest_result.snrs) <= 0):
                LOG.info('    Cannot compute PHASE SNR values')
                LOG.info('    Switching to narrow to wide spw mapping algorithm')
	        combinespwmap = []
	        phaseupspwmap = simple_n2wspwmap (allspws, scispws, inputs.maxnarrowbw,
	            inputs.minfracmaxbw, inputs.samebb)
	        LOG.info('    The simple phaseup spw map is %s' % (phaseupspwmap))
            else:
                LOG.info('    Low SNR condition not met')
	        combinespwmap = []
	        phaseupspwmap = []
	        LOG.info('    The phaseup spw map is %s' % (phaseupspwmap))
        elif inputs.hm_spwmapmode == 'combine':
	    combinespwmap = combine_spwmap (allspws, scispws, scispws[0].id)
	    phaseupspwmap = []
	    LOG.info('    The combine spw map is %s' % (combinespwmap))
        elif inputs.hm_spwmapmode == 'simple':
            combinespwmap = []
	    phaseupspwmap = simple_n2wspwmap (allspws, scispws, inputs.maxnarrowbw,
	        inputs.minfracmaxbw, inputs.samebb)
	    LOG.info('    The phaseup spw map is %s' % (phaseupspwmap))
        else:
            phaseupspwmap = []
            combinespwmap = []
	    LOG.info('    No combine or phaseup spw map for %s' % \
                (inputs.ms.basename))

	# Compute the phaseup table and set calwt to False
	LOG.info('Computing spw phaseup table for %s is %s' % \
            (inputs.ms.basename, inputs.hm_spwmapmode))
	phaseupresult = self._do_phaseup()
	self._mod_last_calwt (phaseupresult.pool[0], False)
	#self._mod_last_calwt (phaseupresult.final[0], False)

        # Create the results object.
	result = SpwPhaseupResults(vis=inputs.vis,
            phaseup_result=phaseupresult, combine_spwmap=combinespwmap,
            phaseup_spwmap=phaseupspwmap)

	return result

    def analyse(self, result):

	# The caltable portion of the result is treated as if it
	# were any other calibration result.

        # With no best caltable to find, our task is simply to set the one
        # caltable as the best result

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.phaseup_result.pool
                   if ca.exists() or self._executor._dry_run]
        result.phaseup_result.final[:] = on_disk

        missing = [ca for ca in result.phaseup_result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.phaseup_result.error.clear()
        result.phaseup_result.error.update(missing)


        return result


    def _do_snrtest (self):

	# Simplify inputs.
        inputs = self.inputs

        task_args = {
          'output_dir'       : inputs.output_dir,
          'vis'              : inputs.vis,
          'intent'           : 'PHASE',
          'phasesnr'         : inputs.phasesnr,
          'bwedgefrac'       : inputs.bwedgefrac,
          'hm_nantennas'     : inputs.hm_nantennas,
          'maxfracflagged'   : inputs.maxfracflagged
        }
        task_inputs = gaincalsnr.GaincalSnrInputs(inputs.context,
                                                      **task_args)

        gaincalsnr_task = gaincalsnr.GaincalSnr(task_inputs)
        result = self._executor.execute(gaincalsnr_task)

        lowsnr = False
        for snr in result.snrs:
            if snr < inputs.phasesnr:
                lowsnr = True
                break

        return lowsnr, result 


    def _do_phaseup(self):

	# Simplify inputs.
        inputs = self.inputs

        task_args = {
          'output_dir'  : inputs.output_dir,
          'vis'         : inputs.vis,
          'caltable'    : inputs.caltable,
          'field'       : inputs.field,
          'intent'      : inputs.intent,
          'spw'         : inputs.spw,
          'solint'      : 'inf',
          'gaintype'    : 'G',
          'calmode'     : 'p',
          'minsnr'      : inputs.minsnr,
          'combine'     : inputs.combine,
          'refant'      : inputs.refant,
          'minblperant' : inputs.minblperant,
          'to_intent'   : 'PHASE,CHECK,TARGET,AMPLITUDE,BANDPASS',
          'to_field'    : None
        }
        task_inputs = gtypegaincal.GTypeGaincalInputs(inputs.context,
                                                      **task_args)
        phaseup_task = gtypegaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(phaseup_task)

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



class SpwPhaseupResults(basetask.Results):
    def __init__(self, vis=None, phaseup_result=None, combine_spwmap=[],
        phaseup_spwmap=[]):
        """
        Initialise the phaseup spw mapping results object.
        """
        super(SpwPhaseupResults, self).__init__()
        self.vis=vis
        self.phaseup_result = phaseup_result
        self.combine_spwmap = combine_spwmap
        self.phaseup_spwmap = phaseup_spwmap

    def merge_with_context(self, context):

        if self.vis is None:
            LOG.error ( ' No results to merge ')
            return

        if not self.phaseup_result.final:
            LOG.error ( ' No results to merge ')
            return

        # Merge the spw phaseup offset table
        self.phaseup_result.merge_with_context(context)

        # Merge the phaseup spwmap
        ms = context.observing_run.get_ms( name = self.vis)
        if ms:
            ms.phaseup_spwmap = self.phaseup_spwmap
            ms.combine_spwmap = self.combine_spwmap

    def __repr__(self):
        if self.vis is None or not self.phaseup_result:
            return('SpwPhaseupResults:\n'
            '\tNo spw phaseup table computed')
        else:
            spwmap = 'SpwPhaseupResults:\nCombine spwmap = %s\nNarrow to wide spwmap = %s\n' % \
		(self.combine_spwmap, self.phaseup_spwmap)
            return spwmap



