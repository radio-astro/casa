from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from . import resultobjects
from .bandpassflag import Bandpassflag

__all__ = [
    'Bandpassflag'
]

qaadapter.registry.register_to_calibration_topic(resultobjects.BandpassflagResults)

weblog.add_renderer(Bandpassflag, renderer.T2_4MDetailsBandpassflagRenderer(), group_by=weblog.UNGROUPED)
