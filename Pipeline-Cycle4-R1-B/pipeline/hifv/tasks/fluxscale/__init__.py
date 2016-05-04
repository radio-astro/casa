from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog

from .solint import Solint
from .testgains import Testgains
from .fluxboot import Fluxboot
from . import solint
from . import testgains
from . import fluxboot
from . import renderer

weblog.add_renderer(Solint, renderer.T2_4MDetailsSolintRenderer(), group_by='ungrouped')
weblog.add_renderer(Fluxboot, renderer.T2_4MDetailsfluxbootRenderer(), group_by='ungrouped')