from __future__ import absolute_import
import collections

import pipeline.infrastructure.api as api
import pipeline.domain as domain
import pipeline.infrastructure.basetask as basetask
from . import adapters
from . import common
from . import channelbandpass
from .. import gaincal
from pipeline.hif.heuristics import caltable as bcaltable


class PhaseUpBandpassInputs(channelbandpass.ChannelBandpassInputs):
    """
    PhaseUpBandpassInputs is the input class for the PhaseUpBandpass task.

    As the PhaseUpBandpass task calculates the final bandpass caltable using
    the ChannelBandpass task and the phase-up itself requires no additional
    inputs (the phase-up solution intervals coming from a heuristic given to
    PhaseUpBandpass.prepare), PhaseupBandpassInputs simply inherits from -
    but does not extend - ChannelBandpassInputs.

    """
    def __init__(self, context, output_dir=None,
                 #
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # solution parameters
                 solint=None, combine=None, refant=None, minblperant=None,
                 minsnr=None, solnorm=None, fillgaps=None, append=None,
                 # preapply calibrations
                 opacity=None, parang=None):
        super(PhaseUpBandpassInputs, self).__init__(context,
            output_dir=output_dir,
            vis=vis, caltable=caltable,
            # data selection arguments
            field=field, spw=spw, intent=intent,
            # solution parameters
            solint=solint, combine=combine, refant=refant, 
            minblperant=minblperant, minsnr=minsnr, solnorm=solnorm,
            fillgaps=fillgaps, append=append,
            # preapply calibrations
            opacity=opacity, parang=parang)

    def _copy_to_clone(self, clone):
        super(PhaseUpBandpassInputs, self)._copy_to_clone(clone)
        
    def clone(self):
        clone = PhaseUpBandpassInputs(self.context)        
        self._copy_to_clone(clone)
        return clone

    def _get_partial_task_args(self):
        partial = super(PhaseUpBandpassInputs, self)._get_partial_task_args()
        return partial


class PhaseUpBandpass(basetask.StandardTaskTemplate):
    Inputs = PhaseUpBandpassInputs

    def prepare(self, phaseup_solints=None, caltable_ranker=None):
        # adapt the given heuristics so that we can call them with pipeline
        # domain objects
        self._caltable_ranker = adapters.MedianSNRAdapter(caltable_ranker)
        phaseup_solints = adapters.PhaseUpSolIntAdapter(phaseup_solints)

        inputs = self.inputs
        
        # Invoke the solution intervals heuristic to determine the appropriate
        # solution intervals for this intent
        solints = phaseup_solints(inputs.ms, inputs.intent)

        tasks = []
        for solint in solints:
            # create the inputs for our phase-up with the given solint 
            gaincal_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
                output_dir=inputs.output_dir, vis=inputs.vis, calmode='p',
                solint=str(solint), intent=inputs.intent, spw=inputs.spw)
            # now create the task itself with the inputs we just created
            gaincal_task = gaincal.GTypeGaincal(gaincal_inputs)
            # and add the jobs that would be executed to our own
            tasks.extend(gaincal_task)

        for task in tasks:
            self._executor.execute(task, merge)



            # phaseupbandpassinputs directly extends channelbandpass so we can
            # use a clone of our inputs as the bandpass inputs
            bandpass_inputs = inputs.clone()

            # Add the gaincal caltable to the gaintable for our next bandpass.
            # We use gaincal inputs rather than our inputs so we can safely 
            # append to them
            gaintable = gaincal_inputs.gaintable if inputs.spw is not None\
                else gaincal_inputs.gaintable[0]
            gaintable.append(gaincal_inputs.caltable)
            # gaincal was calculated for all SpWs, so add default [] to spwmap
            spwmap = gaincal_inputs.spwmap if inputs.spw is not None\
                else gaincal_inputs.spwmap[0]
            spwmap.append([])

            bandpass_inputs.gaintable = gaintable
            bandpass_inputs.spwmap = spwmap
            
            # need to append solution interval to filename
            bandpass_inputs.caltable = bcaltable.BandpassCaltable()(
                output_dir=inputs.output_dir, 
                method='phaseup{0}'.format(solint), 
                **bandpass_inputs.to_casa_args())
            
            bandpass_task = channelbandpass.ChannelBandpass(bandpass_inputs)
            jobs.extend(bandpass_task.analyse())
            
        return jobs

    def analyse(self, jobs=[]):
        results = common.BandpassResults()
        scores = collections.defaultdict(dict)

        for job in bandpass_jobs:
            caltable = domain.CalibrationTable(job)
            results.caltables.add(caltable)

            # rank each caltable using the given heuristic, recording the
            # score for this spw in the scores dictionary
            score = self._caltable_ranker(caltable)
            spw = job.kw['spw']
            scores[spw][caltable] = score

        # sort the tables by score; this gives a list of (caltable, score)
        # tuples
        for spw in scores:
            by_score = sorted(scores[spw].items(),
                              key=lambda (caltable, score): score,
                              reverse=True)
    
            # the tuple with the highest SNR is the first item in the sorted
            # list, and the caltable itself is the first item of the tuple
            if by_score:
                best_bandpass = by_score[0][0]
                results.best.add(best_bandpass)

        self.results = results

        # TODO: append the best bandpass per spw into one file 
        if not isinstance(self.inputs._caltable, api.Heuristic):
            pass
        
        # no follow-on jobs
        return []
