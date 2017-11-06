from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
import pipeline.hif.tasks.antpos.antpos as antpos

__all__ = [
    'ALMAAntpos',
    'ALMAAntposInputs'
]

LOG = infrastructure.get_logger(__name__)


class ALMAAntposInputs(antpos.AntposInputs):
    """
    ALMAAntposInputs defines the inputs for the ALMAAntpos pipeline task.
    """
    # These are ALMA specific settings and override the defaults in
    # the base class.

    # Force the offset input to be from a file
    hm_antpos = vdp.VisDependentProperty(default='file')

    def __init__(self, context, output_dir=None, vis=None, caltable=None, hm_antpos=None, antposfile=None, antenna=None,
                 offsets=None):
        super(ALMAAntposInputs, self).__init__(
            context, output_dir=output_dir, vis=vis, caltable=caltable, hm_antpos=hm_antpos, antposfile=antposfile,
            antenna=antenna, offsets=offsets)


class ALMAAntpos(antpos.Antpos):
    Inputs = ALMAAntposInputs
