from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from . import gsplinegaincal
from . import gtypegaincal

LOG = logging.get_logger(__name__)


class GaincalModeInputs(basetask.ModeInputs):
    _modes = {'gtype'   : gtypegaincal.GTypeGaincal,
              'gspline' : gsplinegaincal.GSplineGaincal}

    def __init__(self, context, mode='gtype', **parameters):        
        super(GaincalModeInputs, self).__init__(context, mode, **parameters)


class GaincalMode(basetask.ModeTask):
    Inputs = GaincalModeInputs
