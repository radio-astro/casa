from __future__ import absolute_import
import collections
import os
import re
import string
import types

from pipeline.hif.tasks.common import commonfluxresults
import pipeline.infrastructure.casatools as casatools
import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure.jobrequest import casa_tasks
import pipeline.infrastructure.logging as logging
from .. import gaincal

from pipeline.hif.heuristics import fieldnames as fieldnames
from pipeline.hif.heuristics import caltable as fcaltable

LOG = logging.get_logger(__name__)


class FluxscaleInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None,
                 vis=None, caltable=None, fluxtable=None,
                 reference=None, transfer=None, refspwmap=None,
                 refintent=None, transintent=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    def to_casa_args(self):
        """Express these inputs as a dictionary of CASA arguments. The values
        in the dictionary are in a format suitable for CASA and can be 
        directly passed to the CASA task.
        
        :rtype: a dictionary of string/??? kw/val pairs
        """
        args = self._get_task_args(ignore=('refintent','transintent'))

        # Due to BANDPASS.ON_SOURCE etc, we need *BANDPASS*, not BANDPASS
        if 'intent' in args:
            if args['intent'] in ('', None):
                args['intent'] = None
            else:
                intents = []
                for i in args['intent'].split(','):
                    no_asterisks = i.replace('*','')
                    if no_asterisks is not '':
                        intents.append('*{0}*'.format(no_asterisks))
                args['intent'] = ','.join(intents)
            
        # spw needs to be a string and not a number
        if 'spw' in args:
            args['spw'] = str(args['spw'])
            
        for k,v in args.items():
            if v is None:
                del args[k]        
        return args

    @property
    def fluxtable(self):
        """
        Get the fluxtable argument for these inputs.
        
        If set to a table-naming heuristic, this gives a standard calibration
        table filename based on current CASA task arguments.
        """ 
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('fluxtable')

        if callable(self._fluxtable):
            casa_args = self._get_task_args(ignore=('fluxtable',))
            return self._fluxtable(output_dir=self.output_dir,
                                   stage=self.context.stage,
                                   **casa_args)
        return self._caltable
        
    @fluxtable.setter
    def fluxtable(self, value):
        if value is None:
            value = fcaltable.FluxCaltable()
        self._fluxtable = value

    @property
    def reference(self):
        if not callable(self._reference):
            return self._reference

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('reference')

        # this will give something like '0542+3243,0343+242'
        reference_fields = self._reference(self.ms, self.refintent)

        # run the answer through a set, just in case there are duplicates
        fields = set()
        fields.update(reference_fields.split(','))
        
        return ','.join(fields)

    @reference.setter
    def reference(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._reference = value

    @property
    def refintent(self):
        if self._refintent is None:
            return 'AMPLITUDE'
        return self._refintent
    
    @refintent.setter
    def refintent(self, value):
        self._refintent = value

    @property
    def refspwmap(self):
        return self._refspwmap
    
    @refspwmap.setter
    def refspwmap(self, value):
        if value is None:
            value = [-1,]
        self._refspwmap = value

    @property
    def transfer(self):
        if not callable(self._transfer):
            return self._transfer

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('transfer')

        # call the heuristic to get the transfer fields as a string
        transfer_fields = self._transfer(self.ms, self.transintent)

        # remove the reference field should it also have been observed with
        # the transfer intent
        transfers = set(self.ms.get_fields(task_arg=transfer_fields))
        references = set(self.ms.get_fields(task_arg=self.reference))            
        diff = transfers.difference(references)
        
        transfer_names = set([f.name for f in diff])
        fields_with_name = self.ms.get_fields(name=transfer_names)
        if len(fields_with_name) != len(diff):
            return ','.join([str(f.id) for f in diff])
        else:
            return ','.join(transfer_names)
        
    @transfer.setter
    def transfer(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._transfer = value

    @property
    def transintent(self):
        if self._transintent is None:
            return 'PHASE'
        return self._transintent
    
    @transintent.setter
    def transintent(self, value):
        self._transintent = value
        
        
class Fluxscale(basetask.StandardTaskTemplate):
    Inputs = FluxscaleInputs
    
    def _do_gaincal(self):
        inputs = self.inputs
        gaincal_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
            vis   = inputs.vis,
            field = ','.join((inputs.reference, inputs.transfer)))

        gaincal_task = gaincal.GTypeGaincal(gaincal_inputs)
        return self._executor.execute(gaincal_task, True)

    def prepare(self):
        inputs = self.inputs
        
        # if the user didn't specify a caltable to analyse, generate it now
        if inputs.caltable is None:
            LOG.info('Input caltable for fluxscale not set. '
                     'Generating new gaincal table...')
            gaincal_results = self._do_gaincal()
            inputs.caltable = gaincal_results.final[0].gaintable

        fluxscale_job = casa_tasks.fluxscale(**inputs.to_casa_args())
        self._executor.execute(fluxscale_job)
        
        # look at the logger for the last fluxscale results - this because
        # CASA fluxscale does not return any information on the fluxes of the
        # sources.

        # want to look for latest fluxscale results but don't want to read
        # through the whole file -> read in last section of file and search
        # that
        logfile = casatools.log.logfile()
        fsize = os.path.getsize(logfile)
        bufsize = min(100000, fsize)
        
        # construct regexes used to match log sections that read flux, check
        # for a bad fluxscale, and finally one to terminate reading of the
        # CASA log
        readpattern = re.compile(
            '.*Flux density for (.*) in SpW=(.*) is: (.*) \+/- (.*) \(.*')
        checkpattern = re.compile(
            '.*Flux density for (.*) in (.*) is:(?!.*INSUFFICIENT DATA.*)')
        endpattern = re.compile('.*Beginning fluxscale--.*')

        results = collections.defaultdict(list)        
        with open(logfile) as flog:
            # read last bufsize section of file
            flog.seek(-bufsize, 2)

            for line in reversed(flog.readlines()):
                if checkpattern.match(line):
                    r = readpattern.match(line)

                    field = r.group(1)
                    if field != field.translate(string.maketrans(
                      '() ;', '____')):
                        field = '"{0}"'.format(field)
                    
                    spw_id = int(r.group(2))
                    spw = self.inputs.ms.get_spectral_window(spw_id)

                    flux_density = measures.FluxDensity(
                      value=float(r.group(3)), 
                      units=measures.FluxDensityUnits.JANSKY)

                    flux = domain.FluxMeasurement(spw=spw, I=flux_density)
                    results[field].append(flux)
                if endpattern.match(line):
                    break
            else:
                LOG.error('Could not find start of fluxscale task in CASA log. '
                          'Too many sources, or operating in dry-run mode?')

        # we don't use the output caltable, but if we needed it we could pass
        # it to the results like so:
        # caltable = domain.CalibrationTable(fluxscale_jobs[0])
        return commonfluxresults.FluxCalibrationResults(vis=self.inputs.vis,
          fields=results)

    def analyse(self, result):
        return result
