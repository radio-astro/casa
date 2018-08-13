from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .editimlist import Editimlist
from . import editimlist

weblog.add_renderer(Editimlist,
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='editimlist.mako',
                                                              description='Editimlist',
                                                              always_rerender=False),
                    group_by=weblog.UNGROUPED,)
