from __future__ import absolute_import
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .syspower import Syspower
from . import syspower
from . import renderer


#qaadapter.registry.register_to_dataset_topic(syspower.SyspowerResults)

# weblog.add_renderer(Syspower, basetemplates.T2_4MDetailsDefaultRenderer(uri='syspower.mako',
#                          description='Syspower'), group_by=weblog.UNGROUPED)

weblog.add_renderer(Syspower, renderer.T2_4MDetailssyspowerRenderer(), group_by=weblog.UNGROUPED)