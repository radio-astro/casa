from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .makeimlist import MakeImList
from . import resultobjects

qaadapter.registry.register_to_imaging_topic(resultobjects.MakeImListResult)

weblog.add_renderer(MakeImList, 
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='makeimlist.mako',
                                                              description='Compile a list of cleaned images to be calculated'))
