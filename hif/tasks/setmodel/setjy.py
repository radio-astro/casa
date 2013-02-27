from __future__ import absolute_import
import collections
import datetime
import os
import re
import types

from pipeline.hif.tasks.common import commonfluxresults
import pipeline.infrastructure.casatools as casatools
import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure.jobrequest import casa_tasks
import pipeline.infrastructure.logging as logging

from pipeline.hif.heuristics import standard as standard

LOG = logging.get_logger(__name__)


class SetjyInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None,
                 # standard setjy parameters 
                 vis=None, field=None, spw=None, modimage=None, 
                 scalebychan=None, fluxdensity=None, spix=None,
		 reffreq=None, standard=None,
                 # intent for calculating field name
                 intent=None):
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
        # if flux density was explicitly set, return that value
        if self._fluxdensity is None:
            return -1
        return self._fluxdensity

    @fluxdensity.setter
    def fluxdensity(self, value):
        self._fluxdensity = value

    @property
    def spix(self):
        return self._spix

    @spix.setter
    def spix(self, value):
        if value is None:
            value = 0.0
        self._spix = value

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
    def intent(self):
        if self._intent is not None:
            return self._intent.replace('*', '')
        return None            
    
    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'AMPLITUDE'
        self._intent = value            
    
    @property
    def standard(self):
        # if invoked with multiple mses, return a list of standards
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('standard')
        
        if not callable(self._standard):
            return self._standard
            
        fields = self.field.split(',')
        standards = [self._standard(field) for field in fields]
        return standards[0] if len(standards) is 1 else standards

    @standard.setter
    def standard(self, value):
        if value is None:
            value = standard.Standard()
        self._standard = value
        
    def to_casa_args(self):
        d = super(SetjyInputs, self).to_casa_args()
        if 'intent' in d:
            del d['intent']
        return d


class Setjy(basetask.StandardTaskTemplate):
    Inputs = SetjyInputs
    
    def prepare(self):
        inputs = self.inputs

        if inputs.field == '':
            LOG.warning('%s: no field associated with intent %s' % (
              os.path.basename(inputs.vis), inputs.intent))
            return commonfluxresults.FluxCalibrationResults(vis=self.inputs.vis,
              fields={}, fields_setjy={})

        result = commonfluxresults.FluxCalibrationResults(vis=self.inputs.vis,
	  fields={}, fields_setjy={})

        # if fluxdensity is -1 then it will have to be looked up.
        # Look first in the context, if a fluxdensity for the
        # field is found there then use it. Otherwise leave
        # it as -1 to ask CASA setjy to look it up when called.

        # get the spectral windows for the spw inputs argument
        spws = [spw for spw in inputs.ms.get_spectral_windows(inputs.spw)]

        # in order to print flux densities in the same order as the 
        # fields, we need to get the flux density for each field in turn 
        field_flux = collections.defaultdict(dict)
        for name in inputs.field.split(','):

            # field names may resolve to multiple fields. However, 
            # their flux should be the same
            field = inputs.ms.get_fields(task_arg=name, intent=inputs.intent)[0]
            for spw in spws:
                if inputs.fluxdensity == -1:

                    flux = [inputs._get_casa_flux_density(flux)
                            for flux in field.flux_densities
                            if flux.spw == spw]
                    if flux:
                        field_flux[field.name][spw.id] = flux[0]
                    else:
                        field_flux[field.name][spw.id] = -1
                else:
                    field_flux[field.name][spw.id] = inputs.fluxdensity

        # Multiple spectral windows spawn multiple setjy jobs. Set a unique
        # marker in the CASA log so that we can identify log entries from this
        # particular task
        self._start_marker = 'Setjy marker: {0}'.format(datetime.datetime.now())
        comment_job = casa_tasks.comment(self._start_marker, False)
        self._executor.execute(comment_job)

        # following to hold fluxdensity settings written to model data
        # column
        setjy_results = collections.defaultdict(list)

        # loop over fields so that we can use Setjy for sources with different
        # standards
        for field in inputs.field.split(','):
            inputs.field = field
            jobs = []
            for spw in spws:
                inputs.spw = spw.id
                task_args = inputs.to_casa_args()
                fluxdensity = field_flux[field][spw.id]
                if fluxdensity != -1:

                    # store the flux in the setjy result - that is, the
                    # number to which the model data column is actually
                    # set
                    I = measures.FluxDensity(value=fluxdensity[0], 
                      units=measures.FluxDensityUnits.JANSKY)
                    Q = measures.FluxDensity(value=fluxdensity[1], 
                      units=measures.FluxDensityUnits.JANSKY)
                    U = measures.FluxDensity(value=fluxdensity[2], 
                      units=measures.FluxDensityUnits.JANSKY)
                    V = measures.FluxDensity(value=fluxdensity[3], 
                      units=measures.FluxDensityUnits.JANSKY)
                    flux = domain.FluxMeasurement(spw=spw, I=I, Q=Q, U=U, V=V)
                    setjy_results[field].append(flux)

                task_args['fluxdensity'] = fluxdensity
                jobs.append(casa_tasks.setjy(**task_args))

            # merge identical jobs into one job with a multi-spw argument
            # be careful - that comma after spw is required for ignore to
            # be an iterable of strings!
            jobs = self._merge_jobs(jobs, casa_tasks.setjy, merge=('spw',))
            for job in jobs:
                self._executor.execute(job)

        # higher-level tasks may run multiple Setjy tasks before running
        # analyse, so we also tag the end of our jobs so we can identify the
        # values from this task
        self._end_marker = 'Setjy marker: {0}'.format(datetime.datetime.now())
        comment_job = casa_tasks.comment(self._end_marker, False)
        self._executor.execute(comment_job)

        # look at the logger for the last setjy results - this because CASA
        # setjy does not return any information on the fluxes of the sources.

        log_results = collections.defaultdict(list)

        # want to look for latest setjy results but don't want to read
        # through the whole file -> read in last section of file and search
        # that
        logfile = casatools.log.logfile()
        fsize = os.path.getsize(logfile)
        bufsize = min(100000, fsize)

        # construct regexes used to match setjy log sections and one to 
        # terminate reading of the CASA log
        flux_pattern = re.compile('.*setjy\S*\s+'
                                  '(?P<field>\S*): .*'
                                  'spw(?P<spw>\S*)\s*\S*\['
                                  'I=(?P<I>.*),'
                                  'Q=(?P<Q>.*),'
                                  'U=(?P<U>.*),'
                                  'V=(?P<V>.*)\] \+/-.*')

        end_pattern = re.compile('.*{0}.*'.format(self._start_marker))
        start_pattern = re.compile('.*{0}.*'.format(self._end_marker))

        with open(logfile) as flog:
            # read last bufsize section of file
            flog.seek(-bufsize, 2)

            # CASA has a bug whereby setting spw='0,1' logs results for spw #0
            # twice, thus giving us two measurements. We get round this by
            # noting previously recorded spws in this set
            spw_seen = set()
        
            start_tag_found = False
            for l in reversed(flog.readlines()):
                if not start_tag_found and start_pattern.match(l):
                    start_tag_found = True
                    continue
                if start_tag_found:
                    r = flux_pattern.match(l)
                    if r:
                        matches = r.groupdict()
                        field = matches['field']
                        
                        spw_id = int(r.group(2))
                        spw = self.inputs.ms.get_spectral_window(spw_id)
                        
                        I = measures.FluxDensity(value=float(matches['I']), 
                            units=measures.FluxDensityUnits.JANSKY)
                        Q = measures.FluxDensity(value=float(matches['Q']), 
                            units=measures.FluxDensityUnits.JANSKY)
                        U = measures.FluxDensity(value=float(matches['U']), 
                            units=measures.FluxDensityUnits.JANSKY)
                        V = measures.FluxDensity(value=float(matches['V']), 
                            units=measures.FluxDensityUnits.JANSKY)
    
                        flux = domain.FluxMeasurement(spw=spw, I=I, Q=Q, U=U, V=V)

                        # .. and here's that check for previously recorded
                        # spectral windows.
                        if spw_id not in spw_seen:                     
                            log_results[field].append(flux)
                            spw_seen.add(spw_id)
                    if end_pattern.match(l):
                        break
            else:
                LOG.error('Could not find start of setjy task in CASA log. '
                          'Too many sources, or operating in dry-run mode?')

            # store specified and catalog results in setjy results        
            result.set_setjy_result(vis=self.inputs.vis, fields=setjy_results)
            result.set_setjy_result(vis=self.inputs.vis, fields=log_results)

            # store catalog results in flux calibration result
            result.set_result(vis=self.inputs.vis, fields=log_results)

        return result

    def analyse(self, result):
        return result
