from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import importdata
from . import qa
from . import renderer
from .importdata import ImportData

qaadapter.registry.register_to_dataset_topic(importdata.ImportDataResults)

weblog.add_renderer(ImportData, renderer.T2_4MDetailsImportDataRenderer(), group_by=weblog.UNGROUPED)
