from __future__ import absolute_import

import os
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks

from pipeline.hif.heuristics import caltable as uvcaltable

LOG = infrastructure.get_logger(__name__)

# Fit the contininuum in the UV plane using the CASA style
# uvcontfit task written by the pipeline.


class UVcontFitInputs(basetask.StandardInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
        caltable=None, contfile=None, field=None, intent=None, spw=None,
        combine=None, solint=None, fitorder=None):

        # Set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def caltable(self):
        """
        Get the caltable argument for these inputs.

        If set to a table-naming heuristic, this should give a sensible name
        considering the current CASA task arguments.
        """
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('caltable')

        if callable(self._caltable):
            casa_args = self._get_task_args(ignore=('caltable',))
            return self._caltable(output_dir=self.output_dir,
                                  stage=self.context.stage,
                                  **casa_args)
        return self._caltable

    @caltable.setter
    def caltable(self, value):
        if value is None:
            value = uvcaltable.UVcontCaltable()
        self._caltable = value

    @property
    def contfile(self):
        return self._contfile

    @contfile.setter
    def contfile(self, value=None):
        if value in (None, ''):
            if self.context.contfile:
                value = self.context.contfile
            elif os.path.exists(os.path.join(self.context.output_dir, 'cont.dat')):
                value = os.path.join(self.context.output_dir, 'cont.dat')
        self._contfile = value

    @property
    def caltype(self):
        return 'uvcont'

    # Find all the fields with TARGET intent
    #     Put in the code for proper field selection but don't pass this
    #     selection to CASA mstransform until the no redindexing capability
    #     is available at the ms transform user level.
    @property
    def field(self):
        # If field was explicitly set, return that value
        if self._field is not None:
            return self._field

        # If invoked with multiple mses, return a list of fields
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('field')

        # Otherwise return each field in the current ms that has been observed
        # with the desired intent
        fields = self.ms.get_fields(intent=self.intent)
        unique_field_names = set([f.name for f in fields])
        field_ids = set([f.id for f in fields])

        # Fields with different intents may have the same name. Check for this
        # and return the IDs if necessary
        if len(unique_field_names) is len(field_ids):
            return ','.join(unique_field_names)
        else:
            return ','.join([str(i) for i in field_ids])

    @field.setter
    def field(self, value):
        self._field = value

    # Select TARGET data by default
    @property
    def intent(self):
        return self._intent

    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'TARGET'
        self._intent = value

    # Find all the the spws TARGET intent. These may be a subset of the
    # science windows which included calibration spws.
    #     Put in the code for proper spw selection but don't pass this
    #     selection to CASA mstransform until the no redindexing capability
    #     is available at the ms transform user level.
    @property
    def spw(self):
        if self._spw is not None:
            return self._spw

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('spw')

        science_target_intents = set (self.intent.split(','))
        science_target_spws = []

        science_spws = [spw for spw in self.ms.get_spectral_windows(self._spw)]
        for spw in science_spws:
            if spw.intents.intersection(science_target_intents):
                science_target_spws.append(spw)
        return ','.join([str(spw.id) for spw in science_target_spws])

    @spw.setter
    def spw(self, value):
        self._spw = value

    @property
    def combine(self):
        return self._combine

    @combine.setter
    def combine(self, value):
        if value is None:
            value = ''
        self._combine = value

    @property
    def solint(self):
        return self._solint

    @solint.setter
    def solint(self, value):
        if value is None:
            value = 'int'
        self._solint = value

    @property
    def fitorder(self):
        return self._fitorder

    @fitorder.setter
    def fitorder(self, value):
        if value is None:
            value = 1
        self._fitorder = value

    def to_casa_args(self, append=False):
        # Override the field and spw selection for the
        # time being.
        d = super(UVcontFitInputs, self).to_casa_args()

        # Filter out field and spw for now and use defaults for
        # now. Note that the trailing , is required
        for ignore in ('caltype', ):
            if ignore in d:
                del d[ignore]

        d['append'] = append
        return d


# tell the infrastructure to give us mstransformed data when possible by
# registering our preference for imaging measurement sets
basetask.ImagingMeasurementSetsPreferred.register(UVcontFitInputs)


class UVcontFit(basetask.StandardTaskTemplate):
    Inputs = UVcontFitInputs

    def prepare(self):
        inputs = self.inputs

        if not inputs.context.contfile:
            uvcontfit_args = inputs.to_casa_args()
            uvcontfit_job = casa_tasks.uvcontfit(**uvcontfit_args)
            self._executor.execute(uvcontfit_job)
        else:
            pass
            # Put continuum file reading loop here

        calto = callibrary.CalTo(vis=inputs.vis)
        # careful now! Calling inputs.caltable mid-task will remove the
        # newly-created caltable, so we must look at the task arguments
        # instead
        calfrom = callibrary.CalFrom(uvcontfit_args['caltable'],
                                     caltype='uvcont',
                                     spwmap=[],
                                     interp='', calwt=False)

        calapp = callibrary.CalApplication(calto, calfrom)


        return UVcontFitResults(pool=[calapp])

    def analyse (self, result):

        # With no best caltable to find, our task is simply to set the one
        # caltable as the best result

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        result.final[:] = on_disk

        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.error.clear()
        result.error.update(missing)

        return result

class UVcontFitResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(UVcontFitResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.warn('No UV continuum results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):
        s = 'UVcontFitResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s

