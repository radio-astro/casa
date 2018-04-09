from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .solint import Solint
from .fluxboot import Fluxboot
from .fluxboot2 import Fluxboot2
from . import solint
from . import fluxboot
from . import renderer
from . import fluxboot2

# from . import qa

#pipelineqa.registry.add_handler(qa.SolintQAHandler())
#pipelineqa.registry.add_handler(qa.SolintListQAHandler())
#qaadapter.registry.register_to_dataset_topic(solint.SolintResults)

#pipelineqa.registry.add_handler(qa.FluxbootQAHandler())
#pipelineqa.registry.add_handler(qa.FluxbootListQAHandler())
#qaadapter.registry.register_to_dataset_topic(fluxboot.FluxbootResults)


weblog.add_renderer(Solint, renderer.T2_4MDetailsSolintRenderer(), group_by=weblog.UNGROUPED)
weblog.add_renderer(Fluxboot, renderer.T2_4MDetailsfluxbootRenderer(), group_by=weblog.UNGROUPED)
weblog.add_renderer(Fluxboot2, renderer.T2_4MDetailsfluxbootRenderer(), group_by=weblog.UNGROUPED)
