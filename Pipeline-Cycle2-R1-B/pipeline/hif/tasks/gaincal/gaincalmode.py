from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from . import gsplinegaincal
from . import gtypegaincal
from . import ktypegaincal

LOG = infrastructure.get_logger(__name__)


class GaincalModeInputs(basetask.ModeInputs):
    _modes = {'gtype'   : gtypegaincal.GTypeGaincal,
              'gspline' : gsplinegaincal.GSplineGaincal,
              'ktype'   : ktypegaincal.KTypeGaincal}

    def __init__(self, context, mode='gtype', **parameters):        
        super(GaincalModeInputs, self).__init__(context, mode, **parameters)


class GaincalMode(basetask.ModeTask):
    Inputs = GaincalModeInputs
