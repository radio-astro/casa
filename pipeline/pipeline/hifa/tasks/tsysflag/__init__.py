from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .tsysflag import Tsysflag
from .tsysflagchans import Tsysflagchans
from . import qa

pipelineqa.registry.add_handler(qa.TsysflagQAHandler())
pipelineqa.registry.add_handler(qa.TsysflagListQAHandler())
