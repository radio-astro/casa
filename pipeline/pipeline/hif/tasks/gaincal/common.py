from __future__ import absolute_import
import os
import types

import pipeline.infrastructure.api as api
from pipeline.hif.tasks.common import commoncalinputs
import pipeline.infrastructure.logging as logging

from pipeline.hif.heuristics import caltable as gcaltable

LOG = logging.get_logger(__name__)


class CommonGaincalInputs(commoncalinputs.CommonCalibrationInputs):
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

    #@property
    #def intent(self):
        #return self._intent

    @property
    def intent(self):
        if self._intent is not None:
            return self._intent

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('intent')

        return 'PHASE'

    @intent.setter
    def intent(self, value):
        if value is None:
            self._intent = None
        else:
            self._intent = str(value).replace('*', '')

    @property
    def calmode(self):
        if self._calmode is not None:
            return self._calmode
        return 'ap'

    @calmode.setter
    def calmode(self, value):
        self._calmode = value


class GaincalResults(api.Results):
    """
    GaincalResults is the results class common to all pipeline bandpass
    calibration tasks.
    """
    
    def __init__(self, final=[], pool=[], preceding=[]):
        """
        Construct and return a new GaincalResults.
        
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
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context, replace=False):
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
        s = 'GaincalResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s
