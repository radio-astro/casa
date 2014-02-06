from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa

from .wvrgcalflag import Wvrgcalflag
from . import qa

pipelineqa.registry.add_handler(qa.WvrgcalflagQAHandler())
pipelineqa.registry.add_handler(qa.WvrgcalflagListQAHandler())
