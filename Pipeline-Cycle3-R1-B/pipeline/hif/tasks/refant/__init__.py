from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .referenceantenna import RefAnt
from . import qa
from . import referenceantenna

pipelineqa.registry.add_handler(qa.RefantQAHandler())
pipelineqa.registry.add_handler(qa.RefantListQAHandler())
qaadapter.registry.register_to_miscellaneous_topic(referenceantenna.RefAntResults)

weblog.add_renderer(RefAnt, basetemplates.T2_4MDetailsDefaultRenderer(uri='refant.mako',
                                                                      description='Select reference antennas'))
