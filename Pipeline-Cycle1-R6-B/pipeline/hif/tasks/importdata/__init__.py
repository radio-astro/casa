from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .importdata import ImportData
from . import qa

pipelineqa.registry.add_handler(qa.ImportDataQAHandler())
pipelineqa.registry.add_handler(qa.ImportDataListQAHandler())
