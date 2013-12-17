from __future__ import absolute_import
import os
import types

from pipeline.hif.tasks.common import commoncalinputs as commoncalinputs
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
from pipeline.hif.heuristics import caltable as bcaltable

LOG = infrastructure.get_logger(__name__)



class CommonBandpassInputs(commoncalinputs.CommonCalibrationInputs):
    """
    CommonBandpassInputs is the base class for defines inputs that are common
    to all pipeline bandpass calibration tasks.
    
    CommonBandpassInputs should be considered an abstract class. Refer to the
    specializations that inherit from CommonBandpassInputs for concrete
    implementations.
    """
    
    combine = basetask.property_with_default('combine', 'scan')
    # Force to False for the time being.
    run_qa2 = basetask.property_with_default('run_qa2', False, 'Perform QA2 analysis')
    solint  = basetask.property_with_default('solint', 'inf')
    solnorm = basetask.property_with_default('solnorm', True)

    @property
    def caltable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary 
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('caltable')
        return super(CommonBandpassInputs, self).caltable
        
    @caltable.setter
    def caltable(self, value):
        if value is None:
            value = bcaltable.BandpassCaltable()
        self._caltable = value

    @property
    def intent(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('intent')
        
        if not isinstance(self.vis, list) and isinstance(self._intent, list):
            idx = self._my_vislist.index(self.vis)
            #return self._intent[idx]
            return self._intent

        if type(self.vis) is types.StringType and type(self._intent) is types.StringType:
            return self._intent        
        
        # if the spw was set, look to see which intents were observed in that
        # spectral window and return the intent based on our order of
        # preference: BANDPASS, AMPLITUDE, PHASE
        preferred_intents = ('BANDPASS', 'PHASE', 'AMPLITUDE')
        if self.spw:
            for spw in self.ms.get_spectral_windows(self.spw):
                for intent in preferred_intents:
                    if intent in spw.intents:
                        if intent != preferred_intents[0]:
                            LOG.warning('%s spw %s: %s not present, %s used instead' %
                              (os.path.basename(self.vis), spw.id, 
                              preferred_intents[0], intent))
                        return intent
            
        # spw was not set, so look through the spectral windows
        for intent in preferred_intents:
            for spw in self.ms.spectral_windows:
                if intent in spw.intents:
                    if intent != preferred_intents[0]:
                        LOG.warning('%s %s: %s not present, %s used instead' %
                          (os.path.basename(self.vis), spw.id, preferred_intents[0],
                          intent))
                    return intent
                
        # current fallback - return an empty intent
        return ''
    
    @intent.setter
    def intent(self, value):
        if isinstance(value, list):
            value = [str(v).replace('*', '') for v in value]
        if type(value) is types.StringType:    
            value = str(value).replace('*', '')
        self._intent = value

        
class BandpassResults(basetask.Results):
    """
    BandpassResults is the results class common to all pipeline bandpass
    calibration tasks.
    """
    
    def __init__(self, final=[], pool=[], preceding=[]):
        """
        Construct and return a new BandpassResults.
        
        The resulting object can optionally be initialized with lists of
        CalibrationTables referring to all caltables created by this task
        (caltables), those caltables deemed the best results for this task
        (best), and those caltables which this task analysed in order to 
        find the best parameters (basis).

        :param best: the best caltables
        :type best: list of :class:`~pipeline.domain.caltable.CalibrationTable`
        :param candidates: all caltables considered by this task
        :type candidates: list of :class:`~pipeline.domain.caltable.CalibrationTable`
        :param tasks: the caltables on which the parameters were determined
        :type tasks: list of :class:`~pipeline.domain.caltable.CalibrationTable`
        """
        super(BandpassResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.qa2 = {}

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    @property
    def caltable_names(self):
        return set([t.basename for t in self.candidates])

    def __repr__(self):
        s = 'BandpassResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s
