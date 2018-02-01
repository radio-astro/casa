from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import renderer
from .gaincalsnr import GaincalSnr
from .gaincalsnr import GaincalSnrResults

qaadapter.registry.register_to_dataset_topic(GaincalSnrResults)

#weblog.add_renderer(GaincalSnr, basetemplates.T2_4MDetailsDefaultRenderer( \
      #description='Estimate gain calibration SNR'))
weblog.add_renderer(GaincalSnr, renderer.T2_4MDetailsGaincalSnrRenderer(), group_by=weblog.UNGROUPED)

