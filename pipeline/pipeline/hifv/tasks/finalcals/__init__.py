from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog

from .finalcals import Finalcals
from .applycals import Applycals

from . import finalcals
from . import renderer

weblog.add_renderer(Finalcals, renderer.T2_4MDetailsfinalcalsRenderer())