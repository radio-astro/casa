from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .priorcals import Priorcals
from . import priorcals
from . import renderer

weblog.add_renderer(Priorcals, renderer.T2_4MDetailspriorcalsRenderer())