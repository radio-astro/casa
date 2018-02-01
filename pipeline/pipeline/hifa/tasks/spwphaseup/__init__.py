from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from . import spwphaseup
from .spwphaseup import SpwPhaseup

qaadapter.registry.register_to_calibration_topic(spwphaseup.SpwPhaseupResults)

weblog.add_renderer(SpwPhaseup, renderer.T2_4MDetailsSpwPhaseupRenderer(),
    group_by=weblog.UNGROUPED)

