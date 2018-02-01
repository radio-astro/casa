from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from pipeline.hif.tasks.bandpass import common
from . import renderer
from .almaphcorbandpass import ALMAPhcorBandpass, SessionALMAPhcorBandpass

qaadapter.registry.register_to_calibration_topic(common.BandpassResults)

weblog.add_renderer(ALMAPhcorBandpass, renderer.T2_4MDetailsBandpassRenderer(), group_by=weblog.UNGROUPED)
