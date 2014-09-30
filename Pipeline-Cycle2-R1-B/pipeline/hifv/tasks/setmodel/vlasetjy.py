from __future__ import absolute_import
import ast
import csv
import datetime
import os
import re
import string
import types
import math

from pipeline.hif.tasks.common import commonfluxresults
import pipeline.infrastructure.casatools as casatools
import pipeline.domain as domain
import pipeline.domain.measures as measures
from pipeline.hifv.heuristics import standard as standard
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils

from pipeline.hifv.tasks.vlautils import VLAUtils
import numpy

import itertools
import pipeline.extern.asizeof as asizeof


LOG = infrastructure.get_logger(__name__)

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
    position_3C286 = casatools.measures.direction('j2000', '13h31m8.288', '30d30m23.959')
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

    fields = [ fields_3C48, fields_3C138, fields_3C147, fields_3C286 ]
    
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

    standard_source_names = [ '3C48', '3C138', '3C147', '3C286' ]
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



class VLASetjyInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None,
                 # standard setjy parameters 
                 vis=None, field=None, spw=None, model=None, 
                 scalebychan=None, fluxdensity=None, spix=None,
                 reffreq=None, standard=None,
                 # intent for calculating field name
                 intent=None,
                 # reference flux file
                 reffile=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @staticmethod
    def _get_casa_flux_density(flux):
        iquv = [flux.I.to_units(measures.FluxDensityUnits.JANSKY),
                flux.Q.to_units(measures.FluxDensityUnits.JANSKY),
                flux.U.to_units(measures.FluxDensityUnits.JANSKY),
                flux.V.to_units(measures.FluxDensityUnits.JANSKY) ]
        return map(float, iquv)

    @property
    def field(self):
        # if field was explicitly set, return that value 
        if self._field is not None:
            return self._field
        
        # if invoked with multiple mses, return a list of fields
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('field')
        
        # otherwise return each field in the current ms that has been observed
        # with the desired intent
        fields = self.ms.get_fields(intent=self.intent)
        field_names = set([f.name for f in fields])
        return ','.join(field_names)
    
        # fields with different intents may have the same name. Check for this
        # and return the IDs if necessary
        if len(unique_field_names) is len(field_ids):
            return ','.join(unique_field_names)
        else:
            return ','.join([str(i) for i in field_ids])

    @field.setter
    def field(self, value):
        self._field = value

    @property
    def fluxdensity(self):
        # if flux density was explicitly set and is *not* -1 (which is 
        # hard-coded as the default value in the task interface), return that
        # value. 
        if self._fluxdensity is not -1:
            return self._fluxdensity

        # if invoked with multiple mses, return a list of flux densities
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('fluxdensity')

        if not self.ms:
            return

        # so _fluxdensity is -1, indicating we should do a flux lookup. The
        # lookup order is: 
        # 1) from file, unless it's a solar system object
        # 2) from CASA
        # We now read the reference flux value from a file
        ref_flux = []
        if os.path.exists(self.reffile):
            with open(self.reffile, 'rt') as f:
                reader = csv.reader(f)

                # first row is header row
                reader.next()
        
                for row in reader:
                    try:
                        (ms_name, field_id, spw_id, I, Q, U, V, comment) = row
                    except ValueError:
                        LOG.warning('Invalid flux statement in %s: \'%s'
                                    '\'' % (self.reffile, row))
                        continue

                    if os.path.basename(ms_name) != self.ms.basename:
                        continue

                    spw_id = int(spw_id)
                    ref_flux.append((field_id, spw_id, float(I), float(Q), 
                                     float(U), float(V)))
        
                    # TODO sort the flux values in spw order?

        if not os.path.exists(self.reffile) and self.reffile not in ('', None):
            LOG.warning('Flux reference file \'%s\' not found')

        # get the spectral window IDs for the spws specified by the inputs
        spws = self.ms.get_spectral_windows(self.spw)
        spw_ids = sorted(spw.id for spw in spws)

        # in order to print flux densities in the same order as the fields, we
        # need to get the flux density for each field in turn 
        field_flux = []
        for field_arg in utils.safe_split(self.field):
            # field names may resolve to multiple field IDs
            fields = self.ms.get_fields(task_arg=field_arg, intent=self.intent)
            field_ids = set([str(field.id) for field in fields])
            field_names = set([field.name for field in fields])

            flux_by_spw = [] 
            for spw_id in spw_ids:
                flux = [[I, Q, U, V] 
                        for (ref_field_id, ref_spw_id, I, Q, U, V) in ref_flux
                        if (ref_field_id in field_ids
                            or ref_field_id in field_names)
                        and ref_spw_id == spw_id]
                
                # no flux measurements found for the requested field/spws, so do
                # either a CASA model look-up (-1) or reset the flux to 1.                
                if not flux:
                    if any('AMPLITUDE' in f.intents for f in fields):
                        flux = [-1]
                    else:
                        flux = [-1]  #originally set to [1]

                # if this is a solar system calibrator, ignore any catalogue
                # flux and request a CASA model lookup
                if not field_names.isdisjoint(standard.Standard.ephemeris_fields):
                    LOG.debug('Ignoring records from file for solar system '
                              'calibrator')
                    flux = [-1]

                flux_by_spw.append(flux[0] if len(flux) is 1 else flux)

            field_flux.append(flux_by_spw[0] if len(flux_by_spw) is 1 
                              else flux_by_spw)

        return field_flux[0] if len(field_flux) is 1 else field_flux

    @fluxdensity.setter
    def fluxdensity(self, value):
        # Multiple flux density values in the form of nested lists are
        # required when multiple spws and fields are to be processed. This
        # function recursively converts those nested list elements to floats.
        def element_to_float(e):
            if type(e) is types.ListType:
                return [element_to_float(i) for i in e]
            return float(e)

        # default value is -1
        if value is None:
            value = -1
        elif value is not -1:
            value = [element_to_float(n) for n in ast.literal_eval(str(value))]        
        
        self._fluxdensity = value

    @property
    def intent(self):
        if self._intent is not None:
            return self._intent.replace('*', '')
        return None            
    
    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'AMPLITUDE'
        self._intent = string.replace(value, '*', '')

    @property
    def reffile(self):
        return self._reffile
    
    @reffile.setter
    def reffile(self, value=None):
        if value in (None, ''):
            value = os.path.join(self.context.output_dir, 'flux.csv')
        self._reffile = value

    @property
    def reffreq(self):
        return self._reffreq

    @reffreq.setter
    def reffreq(self, value):
        if value is None:
            value = '1GHz'
        self._reffreq = value

    @property
    def scalebychan(self):
        return self._scalebychan

    @scalebychan.setter
    def scalebychan(self, value):
        if value is None:
            value = True
        self._scalebychan = value

    @property
    def spix(self):
        return self._spix

    @spix.setter
    def spix(self, value):
        if value is None:
            value = 0.0
        self._spix = value

    @property
    def standard(self):
        # if invoked with multiple mses, return a list of standards
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('standard')
        
        if not callable(self._standard):
            return self._standard
            
        fields = utils.safe_split(self.field)
        standards = [self._standard(field) for field in fields]
        return standards[0] if len(standards) is 1 else standards

    @standard.setter
    def standard(self, value):
        if value is None:
            value = standard.Standard()
        self._standard = value
        
    def to_casa_args(self):
        d = super(VLASetjyInputs, self).to_casa_args()
        # Filter out reffile. Note that the , is required
	for ignore in ('reffile',):
	    if ignore in d:
                del d[ignore]

        # Enable intent selection in CASA. Convert to CASA intent if
	# necessary. Not required here.
	# d['intent'] = utils.to_CASA_intent (self.ms, d['intent'])
	d['selectdata'] = True
	
	# Force usescratch to True for now
	d['usescratch'] = True

        return d

class VLASetjy(basetask.StandardTaskTemplate):
    Inputs = VLASetjyInputs

    # SetJy outputs different log messages depending on the type of amplitude
    # calibrator. These two patterns match the formats we've seen so far
    _flux_patterns = (
        # 2013-02-07 15:27:51 INFO setjy        0542+498 (fld ind 2) spw 0  [I=21.881, Q=0, U=0, V=0] Jy, (Perley-Butler 2010)
        re.compile('\(fld ind (?P<field>\d+)\)'
                   '\s+spw\s+(?P<spw>\d+)'
                   '\s+\[I=(?P<I>[\d\.]+)'
                   ', Q=(?P<Q>[\d\.]+)'
                   ', U=(?P<U>[\d\.]+)'
                   ', V=(?P<V>[\d\.]+)]'),
    
        # 2013-02-07 16:28:38 INFO setjy     Callisto: spw9 Flux:[I=20.043,Q=0.0,U=0.0,V=0.0] +/- [I=0.0,Q=0.0,U=0.0,V=0.0] Jy
        re.compile('(?P<field>\S+): '
                   'spw(?P<spw>\d+)\s+'
                   'Flux:\[I=(?P<I>[\d\.]+),'
                   'Q=(?P<Q>[\d\.]+),'
                   'U=(?P<U>[\d\.]+),'
                   'V=(?P<V>[\d\.]+)\]'))
    
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
        # This is done later in the loop for the VLA
        print "Type of inputs.spw: ", inputs.spw, type(inputs.spw)
        spws = [spw for spw in inputs.ms.get_spectral_windows(inputs.spw)]

        # Multiple spectral windows spawn multiple setjy jobs. Set a unique
        # marker in the CASA log so that we can identify log entries from this
        # particular task
        start_marker = self._add_marker_to_casa_log()

        # loop over fields so that we can use Setjy for sources with different
        # standards
        
        
        
        
        
        standard_source_names, standard_source_fields = standard_sources(self.inputs.vis)
        context = self.inputs.context
        m = context.observing_run.measurement_sets[0]
        field_spws = context.evla['msinfo'][m.name].field_spws
        spw2band = context.evla['msinfo'][m.name].spw2band
        bands = spw2band.values()

        #Look in spectral window domain object as this information already exists!
        with casatools.TableReader(self.inputs.vis+'/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
            originalBBClist = table.getcol('BBC_NO')
            spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
            reference_frequencies = table.getcol('REF_FREQUENCY')
    
        center_frequencies = map(lambda rf, spwbw: rf + spwbw/2, reference_frequencies, spw_bandwidths)

        vlainputs = VLAUtils.Inputs(context)
        vlautils = VLAUtils(vlainputs)
        
        for i, fields in enumerate(standard_source_fields):
            for myfield in fields:
                inputs.field = myfield
                jobs = []
                VLAspws = field_spws[myfield]
                print 'VLAspws: ', VLAspws, type(VLAspws)
                strlistVLAspws = ','.join(str(spw) for spw in VLAspws)
                spws = [spw for spw in inputs.ms.get_spectral_windows(strlistVLAspws)]
                
                for spw in spws:
                    inputs.spw = spw.id
                    reference_frequency = center_frequencies[spw.id]
                    try:
                        EVLA_band = spw2band[spw.id]
                    except:
                        LOG.info('Unable to get band from spw id - using reference frequency instead')
                        EVLA_band = vlautils.find_EVLA_band(reference_frequency)
                    
                    LOG.info("Center freq for spw "+str(spw.id)+" = "+str(reference_frequency)+", observing band = "+EVLA_band)
                    
                    model_image = standard_source_names[i] + '_' + EVLA_band + '.im'
        
                    LOG.info("Setting model for field "+str(myfield)+" spw "+str(spw.id)+" using "+model_image)
        
                    task_args = {'vis'            : inputs.vis,
                                 'field'          : str(myfield),
                                 'spw'            : str(spw.id),
                                 'selectdata'     : False,
                                 'model'          : model_image,
                                 'intent'         : '',
                                 'listmodels'     : False,
                                 'scalebychan'    : True,
                                 'fluxdensity'    : -1,
                                 'standard'       : 'Perley-Butler 2013',
                                 'usescratch'     : True,
                                 'async'          : False}
                    
                    jobs.append(casa_tasks.setjy(**task_args))
                    
                    # Flux densities coming from a non-lookup are added to the
                    # results so that user-provided calibrator fluxes are
                    # committed back to the domain objects
                    
                    if inputs.fluxdensity is not -1:
                        try:
                            (I,Q,U,V) = inputs.fluxdensity
                            flux = domain.FluxMeasurement(spw_id=spw.id, I=I, Q=Q, U=U, V=V)
                        except:
                            I = inputs.fluxdensity
                            flux = domain.FluxMeasurement(spw_id=spw.id, I=I)
                        result.measurements[field].append(flux)
                
                # merge identical jobs into one job with a multi-spw argument
                # be careful - that comma after spw is required for ignore to
                # be an iterable of strings!
                jobs = self._merge_jobs(jobs, casa_tasks.setjy, merge=('spw',))
                for job in jobs:
                    self._executor.execute(job)
        
        '''
        for field in utils.safe_split(inputs.field):
            print "setjy field: ", field
            inputs.field = field
            jobs = []
            for spw in spws:
                print "setjy spw: ", spw, spw.id, type(spw.id)
                inputs.spw = spw.id
                task_args = inputs.to_casa_args()
                jobs.append(casa_tasks.setjy(**task_args))
                
                # Flux densities coming from a non-lookup are added to the
                # results so that user-provided calibrator fluxes are
                # committed back to the domain objects
                if inputs.fluxdensity is not -1:
#                    l = inputs.field.split(',')
#                     field_idx = l.index(field)                                    
                    try:
                        (I,Q,U,V) = inputs.fluxdensity
                        flux = domain.FluxMeasurement(spw_id=spw.id, I=I, Q=Q, U=U, V=V)
                    except:
                        I = inputs.fluxdensity
                        flux = domain.FluxMeasurement(spw_id=spw.id, I=I)
                    result.measurements[field].append(flux)

            # merge identical jobs into one job with a multi-spw argument
            # be careful - that comma after spw is required for ignore to
            # be an iterable of strings!
            jobs = self._merge_jobs(jobs, casa_tasks.setjy, merge=('spw',))
            for job in jobs:
                self._executor.execute(job)
        '''

        # higher-level tasks may run multiple Setjy tasks before running
        # analyse, so we also tag the end of our jobs so we can identify the
        # values from this task
        end_marker = self._add_marker_to_casa_log()

        # We read the log in reverse order, so the end timestamp should
        # trigger processing and the start timestamps should terminate it.
        end_pattern = re.compile('.*%s.*' % start_marker)
        start_pattern = re.compile('.*%s.*' % end_marker)

        with commonfluxresults.File(casatools.log.logfile()) as casa_log:
            # CASA has a bug whereby setting spw='0,1' logs results for spw #0
            # twice, thus giving us two measurements. We get round this by
            # noting previously recorded spws in this set
            spw_seen = set()
        
            start_tag_found = False
            for l in casa_log.backward():
                if not start_tag_found and start_pattern.match(l):
                    start_tag_found = True
                    continue

                if start_tag_found:
                    flux_match = self._match_flux_log_entry(l)
                    if flux_match:
                        log_entry = flux_match.groupdict()
                        
                        field_id = log_entry['field']
                        field = self.inputs.ms.get_fields(field_id)[0]
                        
                        spw_id = log_entry['spw']
 
                        I = log_entry['I']
                        Q = log_entry['Q']
                        U = log_entry['U']
                        V = log_entry['V']                        
                        flux = domain.FluxMeasurement(spw_id=spw_id, 
                                                      I=I, Q=Q, U=U, V=V)
                       
                        # .. and here's that check for previously recorded
                        # spectral windows.
                        if spw_id not in spw_seen:
                            result.measurements[field.identifier].append(flux)
                            spw_seen.add(spw_id)
                    if end_pattern.match(l):
                        break

            # Yes, this else belongs to the for loop! This will execute when
            # no break occurs 
            else:
                LOG.error('Could not find start of vlasetjy task in CASA log. '
                          'Too many sources, or operating in dry-run mode?')

        return result

    def analyse(self, result):
        return result

    def _add_marker_to_casa_log(self):
        comment = 'VLASetjy marker: %s' %  datetime.datetime.now()
        comment_job = casa_tasks.comment(comment, False)
        self._executor.execute(comment_job)
        return comment

    def _match_flux_log_entry(self, log_entry):
        for pattern in self._flux_patterns:
            match = pattern.search(log_entry)
            if match:
                return match
        return False
        
