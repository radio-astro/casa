from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .importdata2 import SDImportData2 as SDImportData2
from . import qa

pipelineqa.registry.add_handler(qa.SDImportData2QAHandler())
pipelineqa.registry.add_handler(qa.SDImportData2ListQAHandler())
