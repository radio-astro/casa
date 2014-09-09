from __future__ import absolute_import

from pipeline.hif.tasks.gaincal import common
from pipeline.hif.tasks.gaincal import gaincalmode
from pipeline.hif.tasks.gaincal import gaincalworker
from pipeline.hif.tasks.gaincal import gtypegaincal

from pipeline.hifa.heuristics.phasespwmap import simple_w2nspwmap

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)

class SpwPhaseupInputs(gaincalmode.GaincalModeInputs):

    # Set the spw map determination parameters here.
    intent = basetask.property_with_default('intent', 'BANDPASS')
    maxnarrowbw = basetask.property_with_default('maxnarrowbw', '300MHz')
    minfracmaxbw = basetask.property_with_default('minfracmaxbw', 0.8)
    samebb = basetask.property_with_default('samebb', True)

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, mode=None, intent=None, maxnarrowbw=None,
        minfracmaxbw=None, samebb=None, **parameters):
        super(SpwPhaseupInputs, self).__init__(context, mode='gtype',
            intent=intent,
            maxnarrowbw=maxnarrowbw, minfracmaxbw=minfracmaxbw, samebb=samebb,
            **parameters)

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

	# Compute the spw map. The default map is [] which stands for the
	# default one to one mapping.
	phaseupspwmap = simple_w2nspwmap (allspws, scispws, inputs.maxnarrowbw,
	    inputs.minfracmaxbw, inputs.samebb)
	LOG.info('The phaseup spw map for %s is %s' % \
            (inputs.ms.basename, phaseupspwmap))

	# Compute the phaseup table
	phaseupresult = self._do_phaseup()

        # Create the results object.
	result = SpwPhaseupResults(vis=inputs.vis,
            phaseup_result=phaseupresult, phaseup_spwmap=phaseupspwmap)

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

class SpwPhaseupResults(basetask.Results):
    def __init__(self, vis=None, phaseup_result=None, phaseup_spwmap=[]):
        """
        Initialise the phaseup spw mapping results object.
        """
        super(SpwPhaseupResults, self).__init__()
        self.vis=vis
        self.phaseup_result = phaseup_result
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

    def __repr__(self):
        if self.vis is None or not self.phaseup_result:
            return('SpwPhaseupResults:\n'
            '\tNo spw phaseup table computed')
        else:
            spwmap = 'SpwPhaseupResults:\n\tSpwmap = %s\n' % \
                self.phaseup_spwmap
            return spwmap



