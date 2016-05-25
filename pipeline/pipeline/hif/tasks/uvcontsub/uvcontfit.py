from __future__ import absolute_import

import os
import types
import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools
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

        # Fix combine
        if 'combine' not in d:
            d['combine'] = ''
        elif d['combine'] is not types.StringType:
            d['combine'] = ''

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
        orig_spw = ','.join([spw.split(':')[0] for spw in inputs.spw.split(',')])

        # every time inputs.caltable is called, the old caltable is deleted.
        # So, calculate the caltable name once and use THIS variable from here
        # on in
        caltable = inputs.caltable

        if not inputs.contfile:
            uvcontfit_args = inputs.to_casa_args(caltable)
            uvcontfit_job = casa_tasks.uvcontfit(**uvcontfit_args)
            self._executor.execute(uvcontfit_job)
        else:
            # Get the continuum ranges
            cranges_spwsel = self._get_ranges_spwsel()

            # Initialize uvcontfit append mode
            append = False
            orig_intent = inputs.intent

            # Loop over the ranges calling uvcontfit once per source
            for sname in cranges_spwsel:
                # Translate to field selection
                sfields, sintents = self._get_source_fields(sname)
                if not sfields:
                    continue

                # Accumulate spw selection string or this source
                source_cranges = cranges_spwsel[sname]
                spw_cranges = ['%s:%s' % (spw_id, source_cranges[spw_id].split()[0])
                               for spw_id in source_cranges
                               if source_cranges[spw_id] not in ['', 'NONE']]
                spwstr = ','.join(spw_cranges)
                if not spwstr:
                    continue

                # Fire off task
                inputs.intent = sintents
                uvcontfit_args = inputs.to_casa_args(caltable, field=sfields, spw=spwstr, append=append)
                uvcontfit_job = casa_tasks.uvcontfit(**uvcontfit_args)
                self._executor.execute(uvcontfit_job)
                
                # Switch to append mode
                append = True
                inputs.intent = orig_intent

        calto = callibrary.CalTo(vis=inputs.vis, spw=orig_spw, intent=inputs.intent)
        # careful now! Calling inputs.caltable mid-task will remove the
        # newly-created caltable, so we must look at the task arguments
        # instead
        calfrom = callibrary.CalFrom(caltable,
                                     caltype='uvcont',
                                     spwmap=[],
                                     interp='',
                                     calwt=False)

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
        contfile_handler = contfilehandler.ContFileHandler(inputs.contfile, warn_nonexist=True)

        # Get all the selected fields
        all_fields = inputs.ms.get_fields(task_arg=inputs.field)

        # Get all the associated sources
        all_sources = [f.source for f in all_fields]
        all_source_names = list(set ([f.source.name for f in all_fields]))

        # Collect the merged ranges
        #    Error checking ?
        cranges_spwsel = {}
        for sname in all_source_names:
            source_fields =  [s.fields for s in all_sources if s.name == sname][0]
            if len(source_fields) > 1:
                rep_field_id, rep_field_name = self._get_rep_field (source_fields)
                if rep_field_id < 0:
                    rep_field_id = source_fields[1].id
                    rep_field_name = source_fields[1].name
            else:
                rep_field_id = source_fields[0].id
                rep_field_name = source_fields[0].name
            LOG.info('Representative field for MS %s source %s is field %s with id %d' % (inputs.ms.basename, sname, rep_field_name, rep_field_id))
            cranges_spwsel[sname] = {}
            for spw_id in [str(spw.id) for spw in inputs.ms.get_spectral_windows(task_arg=inputs.spw)]:
                cranges_spwsel[sname][spw_id] = contfile_handler.get_merged_selection(sname, spw_id)
                if not cranges_spwsel[sname][spw_id]:
                    LOG.warn('No frequency ranges for MS %s source %s and spw %d' % (inputs.ms.basename, sname, int(spw_id)))
                    continue
                else:
                    LOG.info('Input frequency ranges for MS %s and spw %d are %s' % (inputs.ms.basename, int(spw_id), cranges_spwsel[sname][spw_id]))
                try:
                    freq_ranges, chan_ranges = contfile_handler.lsrk_to_topo(cranges_spwsel[sname][spw_id],
                        [inputs.vis], [rep_field_id], int(spw_id))
                    LOG.info('Output frequency range for MS %s and spw %d are %s' % (inputs.ms.basename, int(spw_id),
                        freq_ranges[0]))
                    LOG.info('Output channel ranges for MS %s and spw %d are %s' % (inputs.ms.basename, int(spw_id),
                       chan_ranges[0]))
                    cranges_spwsel[sname][spw_id] = freq_ranges[0]
                except:
                    LOG.info('Frequency ranges for MS %s and spw %d are %s' % (inputs.ms.basename, int(spw_id),
                        cranges_spwsel[sname][spw_id]))

        return cranges_spwsel

    # This code is a duplciate of the imaging heuristics code which
    # estimates the phase center of a mosaic.
    #    This and the imaging code heuristics should be refactored at
    #    some point.
    def _get_rep_field (self, source_fields):

        # Initialize
        rep_field = -1
        rep_field_name = ''

        # Get CASA tools
        cqa = casatools.quanta
        cme = casatools.measures

        # First estimate the phase center

        # Get the individual field directions in  ICRS coordinates
        mdirections = []
        for field in source_fields:
            phase_dir = cme.measure(field.mdirection, 'ICRS')
            mdirections.append(phase_dir)

        # Compute offsets from field 0.
        xsep = []; ysep = []
        for mdirection in mdirections:
            pa = cme.posangle(mdirections[0], mdirection)
            sep = cme.separation(mdirections[0], mdirection)
            xs = cqa.mul(sep, cqa.sin(pa))
            ys = cqa.mul(sep, cqa.cos(pa))
            xs = cqa.convert(xs, 'arcsec')
            ys = cqa.convert(ys, 'arcsec')
            xsep.append(cqa.getvalue(xs))
            ysep.append(cqa.getvalue(ys))
        xsep = np.array(xsep)
        ysep = np.array(ysep)

        # Estimate the x and y center offsets
        xcen = xsep.min() + (xsep.max() - xsep.min()) / 2.0
        ycen = ysep.min() + (ysep.max() - ysep.min()) / 2.0

        # Initialize phase center
        ref =  cme.getref(mdirections[0])
        md = cme.getvalue(mdirections[0])
        m0 = cqa.quantity(md['m0'])
        m1 = cqa.quantity(md['m1'])

        # Get direction of image centre crudely by adding offset
        # of center to ref values of first field.
        m0 = cqa.add(m0, cqa.div('%sarcsec' % xcen, cqa.cos(m1)))
        m1 = cqa.add(m1, '%sarcsec' % ycen)
        if ref=='ICRS' or ref=='J2000' or ref=='B1950':
            m0 = cqa.time(m0, prec=10)[0]
        else:
            m0 = cqa.angle(m0, prec=9)[0]
        m1 = cqa.angle(m1, prec=9)[0]
        phase_center = '%s %s %s' % (ref, m0, m1)
        phase_dir = cme.source(phase_center) 

        # Then find the field with smallest separation from the phase center

        field_ids = [f.id for f in source_fields]
        field_names = [f.name for f in source_fields]
        separations = [cme.separation(phase_dir, f.mdirection)['value'] for f in source_fields]
        index = separations.index(min(separations))
        rep_field = field_ids[index]
        rep_field_name = field_names[index]

        return rep_field, rep_field_name

    def _get_source_fields(self, sname):
        inputs = self.inputs

        # Get fields which match the input selection and
        # filter on source name. Use field names for now
        #    The '''' are a work around
        fields = inputs.ms.get_fields(task_arg=inputs.field)
        unique_field_names = set([f.name for f in fields if (f.name == sname or f.name == '"'+sname+'"')])
        field_ids = set([f.id for f in fields if (f.name == sname or f.name == '"'+sname+'"')])

        # Add proper intent filter
        #    May not be necessary
        field_intents = []
        for f in fields:
            if f.name == sname or f.name == '"'+sname+'"':
                field_intents.extend(list(f.intents))
        field_intents = set(inputs.intent.split(',')).intersection(set(field_intents))

        # Fields with different intents may have the same name. Check for this
        # and return the IDs if necessary
        if len(unique_field_names) is len(field_ids):
            fieldstr = ','.join(unique_field_names)
            intentstr = ','.join(field_intents)
        else:
            fieldstr = ','.join([str(i) for i in field_ids])
            intentstr = ','.join(field_intents)

        return fieldstr, intentstr


class UVcontFitResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None):
        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []

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

