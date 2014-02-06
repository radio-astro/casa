from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .channelbandpass import ChannelBandpass
from .polynomialbandpass import PolynomialBandpass
from .phaseupbandpass import PhaseUpBandpass
from .phcorbandpass import PhcorBandpass
from .bandpassmode import BandpassMode

from . import qa

pipelineqa.registry.add_handler(qa.BandpassQAHandler())
pipelineqa.registry.add_handler(qa.BandpassListQAHandler())
