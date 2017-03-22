from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.h.tasks.tsysflag.renderer as super_renderer

from .tsysflag import Tsysflag

weblog.add_renderer(Tsysflag, super_renderer.T2_4MDetailsTsysflagRenderer(), group_by=weblog.UNGROUPED)