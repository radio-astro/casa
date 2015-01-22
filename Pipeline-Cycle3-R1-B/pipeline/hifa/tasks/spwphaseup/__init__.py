from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .spwphaseup import SpwPhaseup
from . import qa
from . import spwphaseup

pipelineqa.registry.add_handler(qa.SpwPhaseupQAHandler())
pipelineqa.registry.add_handler(qa.SpwPhaseupListQAHandler())
qaadapter.registry.register_to_calibration_topic(spwphaseup.SpwPhaseupResults)

weblog.add_renderer(SpwPhaseup, basetemplates.T2_4MDetailsDefaultRenderer(uri='spwphaseup.mako',
      description='Map narrow to wide spectral windows'))

