from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog

from .testBPdcals import testBPdcals
from . import testBPdcals
from . import renderer

weblog.add_renderer(testBPdcals, renderer.T2_4MDetailstestBPdcalsRenderer())