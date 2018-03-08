from __future__ import absolute_import

import copy
import os
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp

from pipeline.h.heuristics import caltable as gcaltable
from pipeline.hif.tasks.common import commoncalinputs

LOG = infrastructure.get_logger(__name__)


class VdpCommonGaincalInputs(commoncalinputs.VdpCommonCalibrationInputs):
    """
    CommonGaincalInputs is the base class for defines inputs that are common
    to all pipeline bandpass calibration tasks.

    CommonGaincalInputs should be considered an abstract class. Refer to the
    specializations that inherit from CommonBandpassInputs for concrete
    implementations.
    """

    calmode = vdp.VisDependentProperty(default='ap')

    @vdp.VisDependentProperty
    def caltable(self):
        namer = gcaltable.GaincalCaltable()
        casa_args = self._get_task_args(ignore=('caltable',))
        return namer.calculate(output_dir=self.output_dir, stage=self.context.stage, **casa_args)

    @vdp.VisDependentProperty
    def intent(self):
        return 'PHASE,AMPLITUDE,BANDPASS'

    @intent.convert
    def intent(self, value):
        if isinstance(value, list):
            value = [str(v).replace('*', '') for v in value]
        if isinstance(value, str):
            value = value.replace('*', '')
        return value

class GaincalResults(basetask.Results):
    """
    GaincalResults is the results class common to all pipeline gaincal
    calibration tasks.
    """
    
    def __init__(self, final=None, pool=None, subtask_results=None):
        """
        Construct and return a new GaincalResults.
        
        GaincalResults can be initialised with an optional list of 
        CalApplications detailing which calibrations, from a pool of candidate
        calibrations (pool), are considered the best to apply (final).
        
        Results from child tasks can be added to the children parameter. They
        will not be added to the context, but their execution may be recorded
        in the weblog.
        
        :param final: the calibrations selected as the best to apply 
        :type final: list of :class:`~pipeline.infrastructure.callibrary.CalApplication`
        :param pool: the pool of all calibrations evaluated by the task 
        :type pool: list of :class:`~pipeline.infrastructure.callibrary.CalApplication`
        :param subtask_results: the Results objects of any subtasks executed by this task
        :type subtask_results: list of :class:`~pipeline.api.Result`
        """
        if final is None:
            final = []
        if pool is None:
            pool = []
        if subtask_results is None:
            subtask_results = []

        super(GaincalResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.subtask_results = subtask_results[:]
        self.error = set()

    def merge_with_context(self, context, to_field=None, to_intent=None):
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:            
            calto = self._get_calto(calapp.calto, to_field, to_intent)
            
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calto, calapp.calfrom))
            context.callibrary.add(calto, calapp.calfrom)

    def _get_calto(self, calto, to_field, to_intent):
        """
        Prepare and return the CalTo to be used for results merging.
        """        
        # Do not modify the CalTo directly, as the original values should be
        # preserved for subsequent applications. The CalLibrary makes a 
        # defensive copy of the CalFrom, so we do not need to protect that 
        # object ourselves.
        calto_copy = copy.deepcopy(calto)

        # When dividing a multi-vis task up into single-vis tasks, the 
        # to_field and to_intent parameters are resolved down to single-vis
        # scope accordingly. Therefore, we can use the to_field and to_intent
        # values directly as they should already be appropriate for the target
        # measurement set specified in this result.
        
        # Give the astronomer a chance to override the destination field and
        # intents, so that the reduction does not need to be repeated just to
        # change how the caltable should be applied.
        if to_field is not None:
            calto_copy.field = to_field
        if to_intent is not None:
            calto_copy.intent = to_intent

        return calto_copy

    def __repr__(self):
        s = 'GaincalResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s
