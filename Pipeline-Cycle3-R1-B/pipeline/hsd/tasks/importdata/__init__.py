from __future__ import absolute_import
import pipeline.hif.tasks.importdata.renderer as super_renderer
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog

from .importdataold import SDImportDataOld
from .importdata import SDImportData as SDImportData
from . import qa
from . import importdataold
from . import renderer

pipelineqa.registry.add_handler(qa.SDImportDataQAHandler())
pipelineqa.registry.add_handler(qa.SDImportDataListQAHandler())

pipelineqa.registry.add_handler(qa.SDImportDataOldQAHandler())
pipelineqa.registry.add_handler(qa.SDImportDataOldListQAHandler())
qaadapter.registry.register_to_dataset_topic(importdataold.SDImportDataOldResults)

weblog.add_renderer(SDImportDataOld, renderer.T2_4MDetailsSingleDishImportDataOldRenderer())

# use the standard ImportData renderer to render ALMAImportData results
weblog.add_renderer(SDImportData, super_renderer.T2_4MDetailsImportDataRenderer())

