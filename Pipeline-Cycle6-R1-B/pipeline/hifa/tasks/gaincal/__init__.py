from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from .timegaincal import TimeGaincal

#from pipeline.hif.tasks.gaincal import renderer


weblog.add_renderer(TimeGaincal, renderer.T2_4MDetailsGaincalRenderer(), group_by=weblog.UNGROUPED)
