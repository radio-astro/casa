from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp

import pipeline.hif.tasks.applycal.ifapplycal as ifapplycal

LOG = infrastructure.get_logger(__name__)


class ApplycalsInputs(ifapplycal.IFApplycalInputs):
    """
    ApplycalInputs defines the inputs for the Applycal pipeline task.
    """
    parang = vdp.VisDependentProperty(default=True)

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, field=None, spw=None, antenna=None, intent=None,
                 opacity=None, parang=None, applymode=None, calwt=None, flagbackup=None, flagsum=None,
                 flagdetailedsum=None):
        super(ApplycalsInputs, self).__init__(context, output_dir=output_dir, vis=vis, field=field, spw=spw,
                                              antenna=antenna, intent=intent, opacity=opacity, parang=parang,
                                              applymode=applymode, calwt=calwt, flagbackup=flagbackup, flagsum=flagsum,
                                              flagdetailedsum=flagdetailedsum)

    def to_casa_args(self):
        d = super(ApplycalsInputs, self).to_casa_args()
        d['intent'] = ''
        d['field'] = ''
        d['spw'] = ''
        return d


class Applycals(ifapplycal.Applycal):
    Inputs = ApplycalsInputs
