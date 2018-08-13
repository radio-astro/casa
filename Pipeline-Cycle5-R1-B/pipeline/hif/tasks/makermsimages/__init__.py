from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog

from .makermsimages import Makermsimages
from . import makermsimages
from . import renderer

weblog.add_renderer(Makermsimages,
                    renderer.T2_4MDetailsMakermsimagesRenderer(uri='makermsimages.mako',
                                                               description='Makermsimages'),
                    group_by=weblog.UNGROUPED)
