from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog

from .semiFinalBPdcals import semiFinalBPdcals
from . import semiFinalBPdcals
from . import renderer

weblog.add_renderer(semiFinalBPdcals, renderer.T2_4MDetailssemifinalBPdcalsRenderer())