from __future__ import absolute_import
import copy
import os
import types

from pipeline.hif.tasks.common import commoncalinputs
from pipeline.hif.heuristics import caltable as gcaltable
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class CommonGaincalInputs(commoncalinputs.CommonCalibrationInputs):
    # simple vis-independent properties with default values
    calmode = basetask.property_with_default('calmode', 'ap')

    @property
    def caltable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('caltable')
        return super(CommonGaincalInputs, self).caltable

    @caltable.setter
    def caltable(self, value):
        if value is None:
            value = gcaltable.GaincalCaltable()
        self._caltable = value

    @property
    def intent(self):
        return self._intent

    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'PHASE,AMPLITUDE,BANDPASS'
        self._intent = str(value).replace('*', '')


class GaincalResults(basetask.Results):
    """
    GaincalResults is the results class common to all pipeline gaincal
    calibration tasks.
    """
    
    def __init__(self, final=[], pool=[], subtask_results=[]):
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
