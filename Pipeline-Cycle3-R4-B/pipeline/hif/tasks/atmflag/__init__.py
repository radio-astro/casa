from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .atmflag import Atmflag
from . import resultobjects
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates

qaadapter.registry.register_to_flagging_topic(resultobjects.AtmflagResults)

weblog.add_renderer(Atmflag, 
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='atmflag.mako',
                                                              description='Flag atmospheric lines'),
                    group_by='session')
