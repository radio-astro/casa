from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import task_registry
from . import gsplinegaincal
from . import gtypegaincal
from . import ktypegaincal

LOG = infrastructure.get_logger(__name__)


class GaincalModeInputs(vdp.ModeInputs):
    _modes = {
        'gtype': gtypegaincal.GTypeGaincal,
        'gspline': gsplinegaincal.GSplineGaincal,
        'ktype': ktypegaincal.KTypeGaincal
    }

    def __init__(self, context, mode='gtype', **parameters):        
        super(GaincalModeInputs, self).__init__(context, mode, **parameters)


@task_registry.set_equivalent_casa_task('hif_gaincal')
class GaincalMode(basetask.ModeTask):
    Inputs = GaincalModeInputs
