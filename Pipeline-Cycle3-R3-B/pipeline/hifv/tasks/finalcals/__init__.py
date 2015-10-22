from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog
#import pipeline.hif.tasks.applycal.renderer as applycal_renderer

from .finalcals import Finalcals
from .applycals import Applycals

from . import finalcals
from . import renderer

weblog.add_renderer(Finalcals, renderer.T2_4MDetailsfinalcalsRenderer(), group_by='ungrouped')
#weblog.add_renderer(Applycals, applycal_renderer.T2_4MDetailsApplycalRenderer(uri='applycals.mako'), group_by='ungrouped')      
weblog.add_renderer(Applycals, renderer.T2_4MDetailsVLAApplycalRenderer(always_rerender=False, uri='applycals.mako'), group_by='ungrouped') 