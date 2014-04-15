from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .tsysflagspectra import Tsysflagspectra
from pipeline.hifa.tasks.tsysflag import qa
from pipeline.hifa.tasks.tsysflag import resultobjects

pipelineqa.registry.add_handler(qa.TsysflagQAHandler())
pipelineqa.registry.add_handler(qa.TsysflagListQAHandler())
