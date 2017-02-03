from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure

from pipeline.hif.tasks import applycal


LOG = infrastructure.get_logger(__name__)


class ApplycalsInputs(applycal.IFApplycalInputs):
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

    def to_casa_args(self):
        d = super(ApplycalsInputs, self).to_casa_args()
        d['intent'] = ''
        d['field'] = ''
        d['spw'] = ''

        return d


class Applycals(applycal.IFApplycal):
    Inputs = ApplycalsInputs
    
    def prepare(self):
        
        # Run applycal
        applycal_results = self._do_applycal()

        return applycal_results
    
    def analyse(self, results):
        return results
    
    def _do_applycal(self):

        result = self.applycal_run()

        return result
