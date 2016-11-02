from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.h.tasks.tsyscal.renderer as super_renderer

from .tsyscal import Tsyscal

weblog.add_renderer(Tsyscal, super_renderer.T2_4MDetailsTsyscalRenderer(), group_by=weblog.UNGROUPED)
