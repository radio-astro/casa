from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from pipeline.hif.tasks.antpos import antpos
from . import almaantpos
from . import qa
from . import renderer
from .almaantpos import ALMAAntpos

qaadapter.registry.register_to_calibration_topic(antpos.AntposResults)

# Simple web log
weblog.add_renderer(ALMAAntpos,
                    renderer.T2_4MDetailsALMAAntposRenderer(),
                    group_by=weblog.UNGROUPED)


