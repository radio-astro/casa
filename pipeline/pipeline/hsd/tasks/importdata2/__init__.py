from __future__ import absolute_import
import pipeline.hif.tasks.importdata.renderer as super_renderer
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog

from .importdata2 import SDImportData2 as SDImportData2
from . import qa

pipelineqa.registry.add_handler(qa.SDImportData2QAHandler())
pipelineqa.registry.add_handler(qa.SDImportData2ListQAHandler())

# use the standard ImportData renderer to render ALMAImportData results
weblog.add_renderer(SDImportData2, super_renderer.T2_4MDetailsImportDataRenderer())
