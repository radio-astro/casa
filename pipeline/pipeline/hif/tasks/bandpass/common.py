from __future__ import absolute_import

import os
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as bcaltable
from pipeline.hif.tasks.common import commoncalinputs as commoncalinputs

LOG = infrastructure.get_logger(__name__)


class VdpCommonBandpassInputs(commoncalinputs.VdpCommonCalibrationInputs):
    """
    CommonBandpassInputs is the base class for defines inputs that are common
    to all pipeline bandpass calibration tasks.

    CommonBandpassInputs should be considered an abstract class. Refer to the
    specializations that inherit from CommonBandpassInputs for concrete
    implementations.
    """
    combine = vdp.VisDependentProperty(default='scan')
    solint = vdp.VisDependentProperty(default='inf')
    solnorm = vdp.VisDependentProperty(default=True)

    @vdp.VisDependentProperty
    def caltable(self):
        namer = bcaltable.BandpassCaltable()
        casa_args = self._get_task_args(ignore=('caltable',))
        return namer.calculate(output_dir=self.output_dir, stage=self.context.stage, **casa_args)

    @vdp.VisDependentProperty
    def intent(self):
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

    @intent.convert
    def intent(self, value):
        if isinstance(value, list):
            value = [str(v).replace('*', '') for v in value]
        if isinstance(value, str):
            value = value.replace('*', '')
        return value


class BandpassResults(basetask.Results):
    """
    BandpassResults is the results class common to all pipeline bandpass
    calibration tasks.
    """
    
    def __init__(self, final=None, pool=None, preceding=None, applies_adopted=False):
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
        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []

        super(BandpassResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.qa = {}
        self.applies_adopted = applies_adopted

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

    def __repr__(self):
        s = 'BandpassResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s
