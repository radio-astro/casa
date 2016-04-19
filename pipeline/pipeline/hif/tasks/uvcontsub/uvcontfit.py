from __future__ import absolute_import

import os
import types
import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks

from pipeline.hif.heuristics import caltable as uvcaltable
import pipeline.infrastructure.contfilehandler as contfilehandler

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
    def contfile(self, value):
        if value in (None, ''):
            if self.context.contfile:
                value = self.context.contfile
            elif os.path.exists(os.path.join(self.context.output_dir, 'cont.dat')):
                value = os.path.join(self.context.output_dir, 'cont.dat')
        self._contfile = value

    @property
    def caltype(self):
        return 'uvcont'

    # Find all the fields with the specified intent
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

        #Fields with different intents may have the same name. Check for this
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

    # Find all the the spws with the specified intent. These may be a subset of the
    # science windows which included calibration spws.

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

    def to_casa_args(self, caltable, field='', spw='', append=False):
        d = super(UVcontFitInputs, self).to_casa_args()

        # Note that the trailing , is required
        for ignore in ('caltype', 'contfile', ):
            if ignore in d:
                del d[ignore]

        # Fix caltable
        d['caltable'] = caltable

        # If field and spw are not defined use the default
        if field:
            d['field'] = field
        if spw:
            d['spw'] = spw

        d['append'] = append

        return d


# tell the infrastructure to give us mstransformed data when possible by
# registering our preference for imaging measurement sets
basetask.ImagingMeasurementSetsPreferred.register(UVcontFitInputs)


class UVcontFit(basetask.StandardTaskTemplate):
    Inputs = UVcontFitInputs

    def prepare(self):
        inputs = self.inputs

        inputs.caltable = inputs.caltable
        if not inputs.contfile:
            uvcontfit_args = inputs.to_casa_args(caltable=inputs.caltable)
            uvcontfit_job = casa_tasks.uvcontfit(**uvcontfit_args)
            self._executor.execute(uvcontfit_job)
        else:
            # Get the continuum ranges
            cranges_spwsel = self._get_ranges_spwsel()

            # Initialize uvcontfit append mode
            append = False

            # Loop over the ranges calling uvcontfit once per source
            for sname in cranges_spwsel.iterkeys():
                # Translate to field selection
                sfields = self._get_source_fields (sname)
                if not sfields:
                    continue
                spwstr = ''
                for spw_id in cranges_spwsel[sname].iterkeys():
                    # Skip empty entry
                    if cranges_spwsel[sname][spw_id] == 'NONE':
                        continue
                    # Accumulate spw selection string or this source
                    if not spwstr:
                        spwstr =  spwstr + '%s:%s' % (spw_id, cranges_spwsel[sname][spw_id].split()[0])
                    else:
                        spwstr =  spwstr + ',%s:%s' % (spw_id, cranges_spwsel[sname][spw_id].split()[0])

                # Fire off task
                uvcontfit_args = inputs.to_casa_args(caltable=inputs.caltable, field=sfields, spw=spwstr, append=append)
                uvcontfit_job = casa_tasks.uvcontfit(**uvcontfit_args)
                self._executor.execute(uvcontfit_job)
                
                # Switch to append mode
                append = True

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

    def _get_ranges_spwsel(self):
        inputs = self.inputs

        # Read continuum file
        contfile_handler = contfilehandler.ContFileHandler(inputs.contfile)
        all_fields = inputs.ms.get_fields(task_arg=inputs.field)
        if len(all_fields) > 1:
            rep_field = all_fields[1]
        else:
            rep_field = all_fields[0]

        # Collect the merged ranges
        #    Error checking ?
        cranges_spwsel = {}
        for sname in [field.source.name for field in all_fields]:
            cranges_spwsel[sname] = {}
            for spw_id in [str(spw.id) for spw in inputs.ms.get_spectral_windows(task_arg=inputs.spw)]:
                cranges_spwsel[sname][spw_id] = contfile_handler.get_merged_selection(sname, spw_id)
                LOG.info('Input frequency ranges for MS %s and spw %d are %s' % (inputs.ms.basename, int(spw_id),
                    cranges_spwsel[sname][spw_id]))
                freq_ranges, chan_ranges = contfile_handler.lsrk_to_topo(cranges_spwsel[sname][spw_id],
                    [inputs.vis], [rep_field.id], int(spw_id))
                LOG.info('Output frequency range for MS %s and spw %d are %s' % (inputs.ms.basename, int(spw_id),
                    freq_ranges))
                LOG.info('Output channel ranges for MS %s and spw %d are %s' % (inputs.ms.basename, int(spw_id),
                    chan_ranges))
                cranges_spwsel[sname][spw_id] = freq_ranges[0]

        return cranges_spwsel

    def _get_source_fields(self, sname):
        inputs = self.inputs

        # Get fields which match the input selection and
        # filter on source name
        fields = inputs.ms.get_fields(task_arg=inputs.field)
        unique_field_names = set([f.name for f in fields if f.name == sname])
        field_ids = set([f.id for f in fields if f.name == sname])

        # Fields with different intents may have the same name. Check for this
        # and return the IDs if necessary
        if len(unique_field_names) is len(field_ids):
            return ','.join(unique_field_names)
        else:
            return ','.join([str(i) for i in field_ids])


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

