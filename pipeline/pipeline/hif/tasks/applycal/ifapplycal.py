from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

from pipeline.h.tasks.applycal import applycal

LOG = infrastructure.get_logger(__name__)


class IFApplycalInputs(applycal.ApplycalInputs, basetask.StandardInputs,
                         basetask.OnTheFlyCalibrationMixin):
    """
    ApplycalInputs defines the inputs for the Applycal pipeline task.
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None,
                 #
                 vis=None,
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # preapply calibrations
                 opacity=None, parang=None, applymode=None, calwt=None,
                 flagbackup=None, flagsum=None, flagdetailedsum=None):
        self._init_properties(vars())

    flagdetailedsum = basetask.property_with_default('flagdetailedsum', True)


class IFApplycal(applycal.Applycal):
    Inputs = IFApplycalInputs
