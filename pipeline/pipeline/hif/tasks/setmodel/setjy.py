from __future__ import absolute_import
import ast
import csv
import datetime
import os
import string
import types
import numpy as np
import decimal



import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks

import pipeline.domain as domain
import pipeline.domain.measures as measures
from pipeline.h.tasks.common import commonfluxresults
from pipeline.hif.heuristics import standard as standard

LOG = infrastructure.get_logger(__name__)

class SetjyInputs(vdp.StandardInputs):

    @vdp.VisDependentProperty
    def field(self):
        
        # Get field ids in the current ms that have been observed
        # with the desired intent
        fields = self.ms.get_fields(intent=self.intent)
        unique_field_names = set([f.name for f in fields])
        field_ids = set([f.id for f in fields])

        # Fields with different intents may have the same name. Check for this
        # and return the field ids rather than the names to resolve any 
        # ambiguities.
        if len(unique_field_names) is len(field_ids):
            return ','.join(unique_field_names)
        else:
            return ','.join([str(i) for i in field_ids])

    intent = vdp.VisDependentProperty (default='AMPLITUDE')

    @vdp.VisDependentProperty
    def refspectra(self):

        # If flux density was explicitly set and is not equal to -1, which is 
        # hard-coded as the default value in the task interface return the
        # default tuple which is composed of the reference frequency, the
        # Stokes fluxdensity and the spectral index
        if self.fluxdensity is not -1:
            return (self.reffreq, self.fluxdensity, self.spix)

        # There is no ms object.
        if not self.ms:
            return

        # The fluxdensity parameter  is set ot  -1 which indicates that we must do a
        # flux lookup. The # lookup order is: 
        #     1) from file, unless it's a solar system object
        #     2) from CASA
        
        # TODO: Replace with reading directly from the context 

        # Read the reference flux values from a file
        ref_flux = []
        if os.path.exists(self.reffile):
            with open(self.reffile, 'rt') as f:

                # First row is header row
                reader = csv.reader(f)
                reader.next()
        
                # Loop over rows trying the new CSV format first
                for row in reader:
                    try:
                        try:
                            (ms_name, field_id, spw_id, I, Q, U, V, spix, comment) = row
                        except:
                            (ms_name, field_id, spw_id, I, Q, U, V, comment) = row
                            spix = str(self.spix)
                    except ValueError:
                        LOG.warning('Invalid flux statement in %s: \'%s'
                                    '\'' % (self.reffile, row))
                        continue

                    # Check that the entry is for the correct MS
                    if os.path.basename(ms_name) != self.ms.basename:
                        continue

                    # Add the value
                    spw_id = int(spw_id)
                    ref_flux.append((field_id, spw_id, float(I), float(Q), 
                                     float(U), float(V), float(spix)))
        
                    # TODO sort the flux values in spw order?

        # Issue warning if the reference file was specified but not found.
        if not os.path.exists(self.reffile) and self.reffile not in ('', None):
            LOG.warning('Flux reference file not found: {!s}'.format(self.reffile))

        # Get the spectral window ids for the spws specified by the inputs
        spws = self.ms.get_spectral_windows(self.spw)
        spw_ids = sorted(spw.id for spw in spws)

        # In order to print flux densities in the same order as the fields, we
        # must retrieve the flux density for each field in turn 
        field_flux = []
        for field_arg in utils.safe_split(self.field):

            # Field names may resolve to multiple field IDs
            fields = self.ms.get_fields(task_arg=field_arg, intent=self.intent)
            field_ids = set([str(field.id) for field in fields])
            field_names = set([field.name for field in fields])

            # Find fluxes
            flux_by_spw = [] 
            for spw_id in spw_ids:
                reffreq = str(self.ms.get_spectral_window(spw_id).centre_frequency)
                if self.normfluxes:
                    flux = [(reffreq, [I/I, Q/I, U/I, V/I], spix) 
                        for (ref_field_id, ref_spw_id, I, Q, U, V, spix) in ref_flux
                        if (ref_field_id in field_ids
                            or ref_field_id in field_names)
                        and ref_spw_id == spw_id]
                else:
                    flux = [(reffreq, [I, Q, U, V], spix) 
                        for (ref_field_id, ref_spw_id, I, Q, U, V, spix) in ref_flux
                        if (ref_field_id in field_ids
                            or ref_field_id in field_names)
                        and ref_spw_id == spw_id]
                
                # No flux measurements found for the requested field/spws, so do
                # either a CASA model look-up (-1) or reset the flux to 1.                
                if not flux:
                    if any('AMPLITUDE' in f.intents for f in fields):
                        flux = (reffreq, -1, self.spix)
                    else:
                        flux = (reffreq, [1], self.spix)

                # If this is a solar system calibrator, ignore any catalogue
                # flux and request a CASA model lookup
                if not field_names.isdisjoint(standard.Standard.ephemeris_fields):
                    LOG.debug('Ignoring records from file for solar system '
                              'calibrator')
                    flux = (reffreq, -1, 0.0)

                flux_by_spw.append(flux[0] if len(flux) is 1 else flux)

            field_flux.append(flux_by_spw[0] if len(flux_by_spw) is 1 
                              else flux_by_spw)

        return field_flux[0] if len(field_flux) is 1 else field_flux

    @vdp.VisDependentProperty
    def reffile(self):
        value = os.path.join(self.context.output_dir, 'flux.csv')
        return value
    
    normfluxes = vdp.VisDependentProperty (default = False)
    reffreq = vdp.VisDependentProperty (default = '1GHz')
    fluxdensity = vdp.VisDependentProperty (default = -1)
    spix = vdp.VisDependentProperty (default = 0.0)
    scalebychan = vdp.VisDependentProperty (default = True)

    @vdp.VisDependentProperty
    def standard(self):

        # Get the standard heuriistics function.
        heu_standard = standard.Standard()

        # The field may be an integer, but the standard heuristic operates on
        # strings so determine the corresponding name of the fields 
        field_names = []
        for field in utils.safe_split(self.field):
            if str(field).isdigit():
                matching_fields = self.ms.get_fields(field)
                assert len(matching_fields) is 1
                field_names.append(matching_fields[0].name)
            else:
                field_names.append(field)

        standards = [heu_standard(field) for field in field_names]
        return standards[0] if len(standards) is 1 else standards

    def __init__(self, context, output_dir=None, vis=None,
        field=None, intent=None, spw=None,
        model=None, scalebychan=None, fluxdensity=None,
        spix=None, reffreq=None, standard=None,
        #    tuple containing reffreq, fluxdensity, spix
        refspectra=None,
        reffile=None, normfluxes=None):

        super(SetjyInputs, self).__init__()

        self.context = context
        self.vis = vis
        self.output_dir = output_dir

        self.field = field
        self.intent = intent
        self.spw = spw

        self.model = model
        self.scalebychan = scalebychan
        self.fluxdensity = fluxdensity
        self.spix = spix
        self.reffreq = reffreq
        self.standard = standard
        self.refspectra = refspectra
        self.reffile = reffile
        self.normfluxes = normfluxes

    def to_casa_args(self):
        d = super(SetjyInputs, self).to_casa_args()

        d['fluxdensity'] = d['refspectra'][1]
        try:
            np.testing.assert_almost_equal(d['refspectra'][2], 0.0)
        except:
            d['reffreq'] = 'TOPO ' + d['refspectra'][0].replace(" ", "")
            d['spix'] = d['refspectra'][2]

        # Filter out reffile. Note that the , is required
        for ignore in ('reffile', 'refspectra', 'normfluxes', ):
            if ignore in d:
                del d[ignore]

        # Enable intent selection in CASA.
        d['selectdata'] = True

        # Force usescratch to True for now
        d['usescratch'] = True

        return d


class Setjy(basetask.StandardTaskTemplate):
    Inputs = SetjyInputs

    def prepare(self):
        inputs = self.inputs
        result = commonfluxresults.FluxCalibrationResults(vis=inputs.vis) 

        # Return early if the field has no data of the required intent. This
        # could be the case when given multiple MSes, one of which could be
        # without an amplitude calibrator for instance.
        if not inputs.ms.get_fields(inputs.field, intent=inputs.intent):
            LOG.warning('Field(s) \'%s\' in %s have no data with intent %s' % 
                        (inputs.field, inputs.ms.basename, inputs.intent))
            return result

        # Get the spectral windows for the spw inputs argument
        spws = [spw for spw in inputs.ms.get_spectral_windows(inputs.spw)]

        # loop over fields so that we can use Setjy for sources with different
        # standards
        
        setjy_dicts = []
        
        for field_name in utils.safe_split(inputs.field):
            jobs = []

            # Intent is now passed through to setjy, where the intents are 
            # AND'ed to form the data selection. This causes problems when a 
            # field name resolves to two field IDs with disjoint intents: 
            # no data is selected. So, create our own OR data selection by 
            # looping over the individual fields, specifying just those 
            # intents present in the field.
            fields = inputs.ms.get_fields(field_name)
            if field_name.isdigit():
                field_is_unique = False
            else: 
                field_is_unique = True if len(fields) is 1 else False
                        
            for field in fields:

		# Determine the valid spws for that field
		valid_spwids = [spw.id for spw in field.valid_spws]

                field_identifier = field.name if field_is_unique else str(field.id)
                # We're specifying field PLUS intent, so we're unlikely to
                # have duplicate data selections. We ensure no duplicate
                # selections by using field ID at the expense of losing some 
                # readability in the log. Also, this helps if the amplitude
                # is time dependent.
                inputs.field = field_identifier

                for spw in spws:  

		    # Skip invalid spws 
		    if spw.id not in valid_spwids:
		        continue
                    inputs.spw = spw.id
    
                    orig_intent = inputs.intent
                    try:                
                        # The field may not have all intents, which leads to its
                        # deselection in the setjy data selection. Only list
                        # the target intents that are present in the field.
                        input_intents = set(inputs.intent.split(',')) 
                        targeted_intents = field.intents.intersection(input_intents)
                        if not targeted_intents:
                            continue 
                        inputs.intent = ','.join(targeted_intents)
    
                        task_args = inputs.to_casa_args()
                        jobs.append(casa_tasks.setjy(**task_args))
                    finally:
                        inputs.intent = orig_intent
                    
                    # Flux densities coming from a non-lookup are added to the
                    # results so that user-provided calibrator fluxes are
                    # committed back to the domain objects
                    if inputs.refspectra[1] is not -1:
                        try:
                            (I,Q,U,V) = inputs.refspectra[1]
                            spix = decimal.Decimal(str(inputs.refspectra[2]))
                            flux = domain.FluxMeasurement(spw_id=spw.id, I=I, Q=Q, U=U, V=V, spix=spix)
                        except:
                            I = inputs.refspectra[1][0]
                            spix = decimal.Decimal(str(inputs.refspectra[2]))
                            flux = domain.FluxMeasurement(spw_id=spw.id, I=I, spix=spix)
                        result.measurements[field_identifier].append(flux)

            # Merge identical jobs into one job with a multi-spw argument
            jobs_and_components = utils.merge_jobs(jobs, casa_tasks.setjy, merge=('spw',))
            for job, _ in jobs_and_components:
                setjy_dicts.append(self._executor.execute(job))


        # Process the setjy results.
        #    There can be ambiguity in the field names and ids
        spw_seen = set()
        for setjy_dict in setjy_dicts:
            setjy_dict.pop('format')
            for field_id in setjy_dict.keys():
                setjy_dict[field_id].pop('fieldName')
                spwkeys = setjy_dict[field_id].keys()
                field = self.inputs.ms.get_fields(field_id)[0]

                if field_id not in result.measurements.keys():
                    if field.name not in result.measurements.keys():
                        for spw_id in spwkeys:
                            I = setjy_dict[field_id][spw_id]['fluxd'][0]
                            Q = setjy_dict[field_id][spw_id]['fluxd'][1]
                            U = setjy_dict[field_id][spw_id]['fluxd'][2]
                            V = setjy_dict[field_id][spw_id]['fluxd'][3]
                            flux = domain.FluxMeasurement(spw_id=spw_id, I=I, Q=Q, U=U, V=V)
                    
                            if spw_id not in spw_seen:
                                result.measurements[str(field_id)].append(flux)
                                spw_seen.add(spw_id)
            
        return result

    def analyse(self, result):
        return result

