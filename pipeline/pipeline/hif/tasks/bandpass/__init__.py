from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import common
from . import qa
from . import renderer
from .bandpassmode import BandpassMode
from .channelbandpass import ChannelBandpass
from .phaseupbandpass import PhaseUpBandpass
from .phcorbandpass import PhcorBandpass
from .polynomialbandpass import PolynomialBandpass

qaadapter.registry.register_to_calibration_topic(common.BandpassResults)

weblog.add_renderer(PhcorBandpass, 
                    renderer.T2_4MDetailsBandpassRenderer(),
                    group_by='session')
