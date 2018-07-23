from __future__ import absolute_import

import csv
import math
import os

import numpy as np

import pipeline.domain as domain
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.common import commonfluxresults
from pipeline.hifv.heuristics import find_EVLA_band
from pipeline.hifv.heuristics import standard as standard
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

# Paper
# http://iopscience.iop.org/article/10.1088/0067-0049/204/2/19/pdf

LOG = infrastructure.get_logger(__name__)

ORIGIN = 'hifa_vlasetjy'


def find_standards(positions):
    """Function for finding standards from the original scripted EVLA pipeline
    """
    # set the max separation as ~1'
    MAX_SEPARATION = 60*2.0e-5
    position_3C48 = casatools.measures.direction('j2000', '1h37m41.299', '33d9m35.133')
    fields_3C48 = []
    position_3C138 = casatools.measures.direction('j2000', '5h21m9.886', '16d38m22.051')
    fields_3C138 = []
    position_3C147 = casatools.measures.direction('j2000', '5h42m36.138', '49d51m7.234')
    fields_3C147 = []
    position_3C286 = casatools.measures.direction('j2000', '13h31m8.288', '30d30m32.959')
    fields_3C286 = []

    for ii in range(0,len(positions)):
        position = casatools.measures.direction('j2000', str(positions[ii][0])+'rad', str(positions[ii][1])+'rad')
        separation = casatools.measures.separation(position,position_3C48)['value'] * math.pi/180.0
        if (separation < MAX_SEPARATION):
            fields_3C48.append(ii)
        else:
            separation = casatools.measures.separation(position,position_3C138)['value'] * math.pi/180.0
            if (separation < MAX_SEPARATION):
                fields_3C138.append(ii)
            else:
                separation = casatools.measures.separation(position,position_3C147)['value'] * math.pi/180.0
                if (separation < MAX_SEPARATION):
                    fields_3C147.append(ii)
                else:
                    separation = casatools.measures.separation(position,position_3C286)['value'] * math.pi/180.0
                    if (separation < MAX_SEPARATION):
                        fields_3C286.append(ii)

    fields = [fields_3C48, fields_3C138, fields_3C147, fields_3C286]
    
    return fields


def standard_sources(vis):
    """Get standard source names, fields, and positions - convenience function from
       the original EVLA scripted pipeline
    """

    with casatools.TableReader(vis+'/FIELD') as table:
        field_positions = table.getcol('PHASE_DIR')
        
    positions = []

    for ii in range(0,len(field_positions[0][0])):
        positions.append([field_positions[0][0][ii], field_positions[1][0][ii]])

    standard_source_names = ['3C48', '3C138', '3C147', '3C286']
    standard_source_fields = find_standards(positions)
    
    standard_source_found = False
    for standard_source_field in standard_source_fields:
        if standard_source_field:
            standard_source_found = True
    if not standard_source_found:
        standard_source_found = False
        LOG.error("ERROR: No standard flux density calibrator observed, flux density scale will be arbitrary")
        QA_calprep='Fail'

    return standard_source_names, standard_source_fields


class VLASetjyInputs(vdp.StandardInputs):

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

    intent = vdp.VisDependentProperty(default='AMPLITUDE')

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
                    flux = [(reffreq, [I / I, Q / I, U / I, V / I], spix)
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
                        flux = (reffreq, [-1], self.spix)

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

    normfluxes = vdp.VisDependentProperty(default=False)
    reffreq = vdp.VisDependentProperty(default='1GHz')
    fluxdensity = vdp.VisDependentProperty(default=-1)
    spix = vdp.VisDependentProperty(default=0.0)
    scalebychan = vdp.VisDependentProperty(default=True)

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

    def __init__(self, context, output_dir=None, vis=None, field=None, intent=None, spw=None, model=None,
                 scalebychan=None, fluxdensity=None, spix=None, reffreq=None, standard=None,
                 refspectra=None, reffile=None, normfluxes=None):

        super(VLASetjyInputs, self).__init__()

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
        d = super(VLASetjyInputs, self).to_casa_args()

        d['fluxdensity'] = d['refspectra'][1]
        try:
            np.testing.assert_almost_equal(d['refspectra'][2], 0.0)
        except:
            d['reffreq'] = 'TOPO ' + d['refspectra'][0].replace(" ", "")
            d['spix'] = d['refspectra'][2]

        # Filter out reffile. Note that the , is required
        for ignore in ('reffile', 'refspectra', 'normfluxes',):
            if ignore in d:
                del d[ignore]

        # Enable intent selection in CASA.
        d['selectdata'] = True

        # Force usescratch to True for now
        d['usescratch'] = True

        return d


@task_registry.set_equivalent_casa_task('hifv_vlasetjy')
class VLASetjy(basetask.StandardTaskTemplate):
    Inputs = VLASetjyInputs

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

        # get the spectral windows for the spw inputs argument
        # This is done later in the loop for the VLA, just keeping here for history memory
        # print "Type of inputs.spw: ", inputs.spw, type(inputs.spw)
        # spws = [spw for spw in inputs.ms.get_spectral_windows(inputs.spw)]

        # Multiple spectral windows spawn multiple setjy jobs. Set a unique
        # marker in the CASA log so that we can identify log entries from this
        # particular task
        # start_marker = self._add_marker_to_casa_log()

        # loop over fields so that we can use Setjy for sources with different
        # standards

        setjy_dicts = []
        
        standard_source_names, standard_source_fields = standard_sources(self.inputs.vis)
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        field_spws = m.get_vla_field_spws()
        spw2band = m.get_vla_spw2band()
        # bands = spw2band.values()

        # Look in spectral window domain object as this information already exists!
        with casatools.TableReader(self.inputs.vis+'/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
            originalBBClist = table.getcol('BBC_NO')
            spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
            reference_frequencies = table.getcol('REF_FREQUENCY')
    
        center_frequencies = map(lambda rf, spwbw: rf + spwbw/2, reference_frequencies, spw_bandwidths)

        # LOG.info("STANDARD SOURCE FIELDS:")
        # print standard_source_fields

        for i, fields in enumerate(standard_source_fields):
            for myfield in fields:
                domainfield = m.get_fields(myfield)[0]
                if 'AMPLITUDE' in domainfield.intents:
                    inputs.field = myfield
                    # Use the domain object to get the actual field id, ***NOT*** the index
                    # setjyfield = m.get_fields()[myfield].id

                    jobs = []
                    VLAspws = field_spws[myfield]
                    strlistVLAspws = ','.join(str(spw) for spw in VLAspws)
                    spws = [spw for spw in inputs.ms.get_spectral_windows(strlistVLAspws)]

                    for spw in spws:
                        inputs.spw = spw.id
                        reference_frequency = center_frequencies[spw.id]
                        try:
                            EVLA_band = spw2band[spw.id]
                        except:
                            LOG.info('Unable to get band from spw id - using reference frequency instead')
                            EVLA_band = find_EVLA_band(reference_frequency)

                        LOG.info("Center freq for spw " + str(spw.id) + " = "
                                 + str(reference_frequency) + ", observing band = " + EVLA_band)

                        model_image = standard_source_names[i] + '_' + EVLA_band + '.im'

                        LOG.info("Setting model for field " + str(m.get_fields()[myfield].id)
                                 + " spw "+str(spw.id) + " using " + model_image)

                        task_args = {'vis'            : inputs.vis,
                                     'field'          : str(myfield),
                                     'spw'            : str(spw.id),
                                     'selectdata'     : False,
                                     'model'          : model_image,
                                     'intent'         : '',
                                     'listmodels'     : False,
                                     'scalebychan'    : inputs.scalebychan,
                                     'fluxdensity'    : inputs.fluxdensity,
                                     'standard'       : inputs.standard,
                                     'usescratch'     : True}

                        jobs.append(casa_tasks.setjy(**task_args))

                        # Flux densities coming from a non-lookup are added to the
                        # results so that user-provided calibrator fluxes are
                        # committed back to the domain objects

                        if inputs.refspectra[1] is not -1:
                            try:
                                (I,Q,U,V) = inputs.refspectra[1]
                                flux = domain.FluxMeasurement(spw_id=spw.id, I=I, Q=Q, U=U, V=V, origin=ORIGIN)
                            except:
                                I = inputs.refspectra[1][0]
                                flux = domain.FluxMeasurement(spw_id=spw.id, I=I, origin=ORIGIN)
                            result.measurements[str(myfield)].append(flux)

                    # merge identical jobs into one job with a multi-spw argument
                    jobs_and_components = utils.merge_jobs(jobs, casa_tasks.setjy, merge=('spw',))
                    for job, _ in jobs_and_components:
                        try:
                            setjy_dicts.append(self._executor.execute(job))
                        except:
                            LOG.warn("SetJy issue with field id=" + str(job.kw['field'])
                                     + " and spw=" + str(job.kw['spw']))

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
                            flux = domain.FluxMeasurement(spw_id, I, Q=Q, U=U, V=V, origin=ORIGIN)
                    
                            if spw_id not in spw_seen:
                                result.measurements[field.identifier].append(flux)
                                spw_seen.add(spw_id)

        return result

    def analyse(self, result):
        return result

