from __future__ import absolute_import
import collections
import string

import pipeline.infrastructure.api as api
import pipeline.domain as domain
import pipeline.infrastructure.basetask as basetask
from . import adapters
from . import common
from . import gaincal
from . import gtypegaincal

from pipeline.h.heuristics import echoheuristic as echoheuristic
from pipeline.hif.heuristics import solint as phaseupsolint
from pipeline.hif.heuristics import bandpass as mediansnr

class PhaseUpGaincalInputs(gaincal.GaincalInputs):
    """
    PhaseUpGaincalInputs is the input class for the PhaseUpGaincal task.

    As the PhaseUpGaincal task calculates the final gaincal caltable using
    the Gaincal task and the phase-up itself requires no additional
    inputs (the phase-up solution intervals coming from a heuristic given to
    PhaseUpGaincal.prepare), PhaseUpGaincalInputs simply inherits from -
    but does not extend - GaincalInputs.

    """
    def __init__(self, context, output_dir=None,
                 #
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, intent=None,
                 # solution parameters
                 gaintype=None, smodel=None, calmode=None, solint=None,
                 combine=None, refant=None, minblperant=None, minsnr=None,
                 solnorm=None, append=None, splinetime=None, npointaver=None,
                 phasewrap=None,
                 # preapply calibrations
                 opacity=None, preavg=None,
                 # ranking heuristic
                 ranker=None):
        super(PhaseUpGaincalInputs, self).__init__(context,
            output_dir=output_dir,
            vis=vis, caltable=caltable,
            # data selection arguments
            field=field, spw=spw, intent=intent,
            # solution parameters
            smodel=smodel, calmode=calmode, combine=combine, refant=refant,
            minblperant=minblperant, minsnr=minsnr, append=append,
            phasewrap=phasewrap,
            # preapply calibrations
            opacity=opacity, preavg=preavg)

        # set heuristics
        self.solint = solint
        self.ranker = ranker

    @property
    def solint(self):
        return self._solint
    
    @solint.setter
    def solint(self, value):
        if value is None:
            value = phaseupsolint.PhaseUpSolInt()
        if not isinstance(value, api.Heuristic):
            value = echoheuristic.EchoHeuristic(value)
        self._solint = value

    @property
    def ranker(self):
        return self._ranker
    
    @ranker.setter
    def ranker(self, value):
        if value is None:
            value = mediansnr.MedianSNR
        self._ranker = value

    def _get_partial_task_args(self):
        partial = super(PhaseUpGaincalInputs, self)._get_partial_task_args()
        partial.update({'solint'  : self._printable(self.solint),
                        'ranker'  : self._printable(self.ranker) })        
        return partial

    def _printable(self, heuristic):
        if isinstance(heuristic, echoheuristic.EchoHeuristic):
            return heuristic()
        return heuristic 


class PhaseUpGaincal(basetask.StandardTaskTemplate):
    Inputs = PhaseUpGaincalInputs

    def _get_phaseup_inputs(self, solint, intent):
        inputs = self.inputs
        return gtypegaincal.GTypeGaincal.Inputs(inputs.context,
            output_dir=inputs.output_dir, vis=inputs.vis, calmode='p',
            solint=str(solint), intent=intent, spw=inputs.spw)

    def prepare(self):
        inputs = self.inputs.clone()
        
        # holds the final jobs to analyse
        jobs = []
        
        for intent in string.split(inputs.intent, ','):
            # Invoke the solution intervals heuristic to determine the appropriate
            # solution intervals for this intent
            solint_fn = adapters.PhaseUpSolIntAdapter(inputs.solint)
            solints = solint_fn(inputs.ms, intent)
    
            for solint in solints:
                # create the inputs for our phase-up with the given solint 
                phaseup_inputs = self._get_phaseup_inputs(solint, intent)
                # now create the task itself with the inputs we just created
                phaseup_task = gtypegaincal.GTypeGaincal(phaseup_inputs)
                # and execute it
                jobs.extend(phaseup_task.prepare())

        return jobs

    def analyse(self, jobs=[]):
        self.results = common.GaincalResults()
        if not jobs:
            return self.results

        ranker = adapters.MedianSNRAdapter(self.inputs.ranker)
        scores = collections.defaultdict(dict)

        for job in jobs:
            caltable = domain.CalibrationTable(job)
            self.results.pool.add(caltable)

            # rank each caltable using the given heuristic, recording the
            # score for this spw in the scores dictionary
            score = ranker(caltable)
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
                best_gaincal = by_score[0][0]
                self.results.final.add(best_gaincal)

        return self.results
