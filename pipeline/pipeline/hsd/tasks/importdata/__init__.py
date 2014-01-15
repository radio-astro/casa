from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .importdata import SDImportData
from . import qa

pipelineqa.registry.add_handler(qa.SDImportDataQAHandler())
pipelineqa.registry.add_handler(qa.SDImportDataListQAHandler())
