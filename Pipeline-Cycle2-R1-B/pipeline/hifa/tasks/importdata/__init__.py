from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .almaimportdata import ALMAImportData
from . import qa

pipelineqa.registry.add_handler(qa.ALMAImportDataQAHandler())
