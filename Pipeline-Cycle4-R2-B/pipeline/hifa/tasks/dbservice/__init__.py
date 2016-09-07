from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.hif.tasks.importdata.renderer as super_renderer
import pipeline.infrastructure.renderer.basetemplates as basetemplates

from .fluxdb import Fluxdb

weblog.add_renderer(Fluxdb, 
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='fluxdb.mako',
                                                              description='Flux database web service'),
                    group_by='session')
