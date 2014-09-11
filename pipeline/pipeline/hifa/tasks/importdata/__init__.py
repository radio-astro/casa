from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.hif.tasks.importdata.renderer as super_renderer

from .almaimportdata import ALMAImportData
from . import qa

pipelineqa.registry.add_handler(qa.ALMAImportDataQAHandler())

# use the standard ImportData renderer to render ALMAImportData results
weblog.add_renderer(ALMAImportData, super_renderer.T2_4MDetailsImportDataRenderer())
