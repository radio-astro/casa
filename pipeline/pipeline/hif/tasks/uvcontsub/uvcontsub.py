from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.applycal import applycal

LOG = infrastructure.get_logger(__name__)

class UVcontSubInputs(applycal.ApplycalInputs):

    # Property overrides
    intent = basetask.property_with_default('intent', 'TARGET')
    applymode = basetask.property_with_default('applymode','calonly')

    # Would liek to set this to False in future but this causes
    # an issue with the results handling.
    #flagsum = basetask.property_with_default('flagsum', True)
    #flagbackup = basetask.property_with_default('flagbackup', False)

    """
    Input for the UVcontSub task
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

# Register this as an imaging MS(s) preferred task
basetask.ImagingMeasurementSetsPreferred.register(UVcontSubInputs)

class UVcontSub(applycal.Applycal):
    Inputs = UVcontSubInputs


# May need this in the future
#
#
#class UVcontSubResults(basetask.Results):
#    """
#    UVcontSubResults is the results class for the pipeline UVcontSub task.
#    """
#
#    def __init__(self, applied=[]):
#        """
#        Construct and return a new UVContSubResults.
#
#        The resulting object should be initialized with a list of
#        CalibrationTables corresponding to the caltables applied by this task.
#
#        :param applied: caltables applied by this task
#        :type applied: list of :class:`~pipeline.domain.caltable.CalibrationTable`
#        """
#        super(UVcontSubResults, self).__init__()
#        self.applied = set()
#        self.applied.update(applied)
#
#    def merge_with_context(self, context):
#        """
#        Merges these results with the given context by examining the context
#        and marking any applied caltables, so removing them from subsequent
#        on-the-fly calibration calculations.
#
#        See :method:`~pipeline.Results.merge_with_context`
#        """
#        if not self.applied:
#            LOG.error('No results to merge')
#
#        for calapp in self.applied:
#            LOG.trace('Marking %s as applied' % calapp.as_applycal())
#            context.callibrary.mark_as_applied(calapp.calto, calapp.calfrom)
#
#    def __repr__(self):
#        for caltable in self.applied:
#            s = 'UVcontSubResults:\n'
#            if type(caltable.gaintable) is types.ListType:
#                basenames = [os.path.basename(x) for x in caltable.gaintable]
#                s += '\t{name} applied to {vis} spw #{spw}\n'.format(
#                    spw=caltable.spw, vis=os.path.basename(caltable.vis),
#                    name=','.join(basenames))
#            else:
#                s += '\t{name} applied to {vis} spw #{spw}\n'.format(
#                    name=caltable.gaintable, spw=caltable.spw,
#                    vis=os.path.basename(caltable.vis))
#        return s
#
