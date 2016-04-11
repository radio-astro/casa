from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils

from pipeline.hif.tasks.applycal.applycal import ApplycalInputs, Applycal

LOG = infrastructure.get_logger(__name__)


class SDMSApplycalInputs(ApplycalInputs,basetask.StandardInputs,
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
                 flagbackup=None, scan=None, flagsum=None):
        self._init_properties(vars())

    @property
    def intent(self):
        return self._intent

    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'TARGET'
        self._intent = value.replace('*', '')

    @property
    def applymode(self):
        return self._applymode

    @applymode.setter
    def applymode(self, value):
        if value is None:
            value = 'calflagstrict'
        elif value == '':
            value = 'calflagstrict'
        self._applymode = value


class SDMSApplycal(Applycal,basetask.StandardTaskTemplate):
    """
    Applycal executes CASA applycal tasks for the current context state,
    applying calibrations registered with the pipeline context to the target
    measurement set.
    
    Applying the results from this task to the context marks the referred
    tables as applied. As a result, they will not be included in future
    on-the-fly calibration arguments.
    """
    Inputs = SDMSApplycalInputs
    ### Note this is a temporary workaround ###
    antenna_to_apply = '*&&&'
    
