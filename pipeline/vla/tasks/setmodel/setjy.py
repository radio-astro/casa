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
from pipeline.hif.heuristics import standard as standard
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


class SetjyInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None,
                 # standard setjy parameters 
                 vis=None, field=None, spw=None, modimage=None, 
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
                    (ms_name, field_id, spw_id, I, Q, U, V) = row
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
                        flux = [1]

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
        d = super(SetjyInputs, self).to_casa_args()
        for ignore in ('intent', 'reffile'):
            if ignore in d:
                del d[ignore]
        return d


class Setjy(basetask.StandardTaskTemplate):
    Inputs = SetjyInputs

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

        #if not inputs.ms.get_fields(inputs.field, intent=inputs.intent):
        #    LOG.warning('Field(s) \'%s\' in %s have no data with intent %s' % 
        #                (inputs.field, inputs.ms.basename, inputs.intent))
        #    return result

        # get the spectral windows for the spw inputs argument
        spws = [spw for spw in inputs.ms.get_spectral_windows(inputs.spw)]

        # Multiple spectral windows spawn multiple setjy jobs. Set a unique
        # marker in the CASA log so that we can identify log entries from this
        # particular task
        start_marker = self._add_marker_to_casa_log()

        # loop over fields so that we can use Setjy for sources with different
        # standards
        for field in utils.safe_split(inputs.field):
            inputs.field = field
            jobs = []
            for spw in spws:
                inputs.spw = spw.id

                #inputs.fluxdensity=-1  #Force to use -1
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
                        flux = domain.FluxMeasurement(spw=spw, I=I, Q=Q, U=U, V=V)
                    except:
                        I = inputs.fluxdensity
                        flux = domain.FluxMeasurement(spw=spw, I=I)
                    result.measurements[field].append(flux)

            # merge identical jobs into one job with a multi-spw argument
            # be careful - that comma after spw is required for ignore to
            # be an iterable of strings!
            jobs = self._merge_jobs(jobs, casa_tasks.setjy, merge=('spw',))
            for job in jobs:
                self._executor.execute(job)
            

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
                        spw = self.inputs.ms.get_spectral_window(spw_id)
 
                        I = log_entry['I']
                        Q = log_entry['Q']
                        U = log_entry['U']
                        V = log_entry['V']                        
                        flux = domain.FluxMeasurement(spw=spw, 
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
                LOG.error('Could not find start of setjy task in CASA log. '
                          'Too many sources, or operating in dry-run mode?')

        return result

    def analyse(self, result):
        return result

    def _add_marker_to_casa_log(self):
        comment = 'Setjy marker: %s' %  datetime.datetime.now()
        comment_job = casa_tasks.comment(comment, False)
        self._executor.execute(comment_job)
        return comment

    def _match_flux_log_entry(self, log_entry):
        for pattern in self._flux_patterns:
            match = pattern.search(log_entry)
            if match:
                return match
        return False
        
