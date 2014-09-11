from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .channelbandpass import ChannelBandpass
from .polynomialbandpass import PolynomialBandpass
from .phaseupbandpass import PhaseUpBandpass
from .phcorbandpass import PhcorBandpass
from .bandpassmode import BandpassMode

from . import qa
from . import common
from . import renderer

pipelineqa.registry.add_handler(qa.BandpassQAHandler())
pipelineqa.registry.add_handler(qa.BandpassListQAHandler())
qaadapter.registry.register_to_calibration_topic(common.BandpassResults)

weblog.add_renderer(PhcorBandpass, renderer.T2_4MDetailsBandpassRenderer())
