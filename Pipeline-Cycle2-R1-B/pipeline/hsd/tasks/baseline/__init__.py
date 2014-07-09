from __future__ import absolute_import
import pipeline.infrastructure.renderer.qaadapter as qaadapter

from .baseline_old import SDBaseline as SDBaselineOld
from .baseline import SDBaseline
from . import baseline

#qaadapter.registry.register_to_calibration_topic(baseline.SDBaselineResults)
qaadapter.registry.register_to_miscellaneous_topic(baseline.SDBaselineResults)

