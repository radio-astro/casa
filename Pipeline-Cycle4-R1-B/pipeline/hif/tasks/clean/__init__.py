from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .clean import Clean
from . import resultobjects
from . import renderer
from . import qa

pipelineqa.registry.add_handler(qa.CleanQAHandler())
pipelineqa.registry.add_handler(qa.CleanListQAHandler())
qaadapter.registry.register_to_imaging_topic(resultobjects.BoxResult)
qaadapter.registry.register_to_imaging_topic(resultobjects.CleanResult)

weblog.add_renderer(Clean,
                    renderer.T2_4MDetailsCleanRenderer(),
                    group_by='ungrouped')

