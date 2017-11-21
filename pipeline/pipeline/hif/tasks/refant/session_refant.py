import os
import string

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from ...heuristics import findrefant

LOG = infrastructure.get_logger(__name__)


class RefAntInputs(vdp.StandardInputs):
    field = vdp.VisDependentProperty(default='')
    flagging = vdp.VisDependentProperty(default=True)
    geometry = vdp.VisDependentProperty(default=True)
    intent = vdp.VisDependentProperty(default='')
    refant = vdp.VisDependentProperty(default='')
    spw = vdp.VisDependentProperty(default='')

    hm_refant = vdp.VisDependentProperty(default='automatic')

    @hm_refant.convert
    def hm_refant(self, value):
        return 'manual' if value == 'manual' else 'automatic'

    def to_casa_args(self):
        # refant does not use CASA tasks
        raise NotImplementedError

    def __init__(self, context, vis=None, output_dir=None, field=None, spw=None, intent=None, hm_refant=None,
                 refant=None, geometry=None, flagging=None):
        self.context = context
        self.vis = vis
        self.output_dir = output_dir

        # data selection parameters
        self.field = field
        self.spw = spw
        self.intent = intent

        # override and heuristic parameters
        self.hm_refant = hm_refant
        self.refant = refant
        self.geometry = geometry
        self.flagging = flagging


class RefAntResults(basetask.Results):
    def __init__(self, vis, refant):
        super(RefAntResults, self).__init__()
        self._vis = vis
        self._refant = ','.join([str(ant) for ant in refant])

    def merge_with_context(self, context):
        if self._vis is None or self._refant is None:
            LOG.error('No results to merge')
            return

        # Do we also need to locate the antenna in the measurement set?
        # This might become necessary when using different sessions
        ms = context.observing_run.get_ms(name=self._vis)

        if ms:
            LOG.debug('Setting refant for {!s} to {!r}'.format(ms.basename, self._refant))
            ms.reference_antenna = self._refant

    def __str__(self):
        if self._vis is None or self._refant is None:
            return ('Reference antenna results:\n'
                    '\tNo reference antenna selected')
        else:
            return ('Reference antenna results:\n'
                    '{!s}: refant={!r}'.format(os.path.basename(self._vis), self._refant))

    def __repr__(self):
        return 'RefAntResults({!r}, {!r})'.format(self._vis, self._refant)


class RefAnt(basetask.StandardTaskTemplate):
    Inputs = RefAntInputs

    def prepare(self, **parameters):
        inputs = self.inputs

        # Get the reference antenna list
        if inputs.hm_refant == 'manual':
            refant = string.split(inputs.refant, ',')
        elif inputs.hm_refant == 'automatic':
            heuristics = findrefant.RefAntHeuristics(vis=inputs.vis, field=inputs.field, spw=inputs.spw,
                                                     intent=inputs.intent, geometry=inputs.geometry,
                                                     flagging=inputs.flagging)
            refant = heuristics.calculate()
        else:
            raise NotImplemented('Unhandled hm_refant value: {!r}'.format(inputs.hm_refant))

        return RefAntResults(inputs.vis, refant)

    def analyse(self, results):
        return results
