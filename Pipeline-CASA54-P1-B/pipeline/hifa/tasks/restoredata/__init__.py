from __future__ import absolute_import

import pipeline.infrastructure.renderer.weblog as weblog

from .almarestoredata import ALMARestoreData
from . import renderer

weblog.add_renderer(ALMARestoreData, renderer.T2_4MDetailsRestoreDataRenderer(), group_by=weblog.UNGROUPED)
