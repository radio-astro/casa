from __future__ import absolute_import
import ast
import re
import types

import numpy

import pipeline.domain as domain
from pipeline.hif.tasks.common import commonfluxresults
from pipeline.hif.heuristics import fieldnames as fieldnames
from pipeline.hif.heuristics import caltable as fcaltable
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.utils as utils
from .. import gaincal

LOG = infrastructure.get_logger(__name__)


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
        fields.update(utils.safe_split(reference_fields))
        
        return ','.join(fields)

    @reference.setter
    def reference(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._reference = value

    @property
    def refintent(self):
        return self._refintent
    
    @refintent.setter
    def refintent(self, value):
        if value is None:
            value = 'AMPLITUDE'
        self._refintent = value

    @property
    def refspwmap(self):
        """
        Get the reference spw map.

        refpwmap is normally found by inspecting the context and returning the
        refspwmap as calculated by a prior 'flux calibrator flagging' task.
        However, if refspwmap has been manually overridden, that
        manually specified value is returned.

        If a flux calibrator flagging task has not been executed and a manual
        override value is not given, None is returned.
        """
        # if refant was overridden, return the manually specified value
        if self._refspwmap is not None:
            return self._refspwmap

        # refspwmap is ms-dependent, so if this inputs is handling multiple
        # measurement sets, return a list of refants instead.
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('refspwmap')

        # we cannot find the context value without the measurement set
        if not self.ms:
            return None

        # get the reference antenna for this measurement set
        refspwmap = self.ms.reference_spwmap

        # otherwise return whatever we found. We assume the calling function
        # knows how to handle an object of this type.
        return refspwmap

    
    @refspwmap.setter
    def refspwmap(self, value):
        def element_to_int(e):
            if type(e) is types.ListType:
                return [element_to_int(i) for i in e]
            return int(e)

        if value not in (None, -1):
            value = [element_to_int(n) for n in ast.literal_eval(str(value))]

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
        if len(fields_with_name) is not len(diff) or len(diff) is not len(transfer_names):
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
        return self._transintent
    
    @transintent.setter
    def transintent(self, value):
        if value is None:
            value = 'PHASE,BANDPASS'
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
        ms = inputs.ms
        result = commonfluxresults.FluxCalibrationResults(inputs.vis)
        
        if inputs.transfer == '' or inputs.reference == '':
            LOG.warning('Fluxscale invoked with no transfer and/or reference '
                        'field. Bypassing fluxscale for %s' % ms.basename)
            return result
        
        # if the user didn't specify a caltable to analyse, generate it now
        if inputs.caltable is None:
            LOG.info('No caltable specified in fluxscale inputs. '
                     'Generating new gaincal table...')
            gaincal_results = self._do_gaincal()
            inputs.caltable = gaincal_results.final[0].gaintable

        fluxscale_job = casa_tasks.fluxscale(**inputs.to_casa_args())
        output = self._executor.execute(fluxscale_job)

        if output is None:
            LOG.warning('No results returned from fluxscale job: missing '
                        'fields in caltable?')
            return result

        no_result = numpy.array([-1.,-1.,-1.,-1.])
        no_result_fn = lambda (spw, flux): not numpy.array_equal(no_result, 
                                                                 flux)

        # fields in the fluxscale output dictionary are identified by a 
        # numeric field ID                  
        for field_id in [key for key in output if re.match('\d+', key)]:
            # flux values themselves are now held at the same dictionary
            # level as field names, spwidx, etc. The only way to identify
            # them is by a numeric key corresponding to the spw.
            flux_for_field = output[field_id]
            flux_for_spws = [(spw, flux_for_field[spw]['fluxd'])
                             for spw in flux_for_field 
                             if spw.isdigit()]

            # filter out the [-1,-1,-1,-1] results
            spw_flux = filter(no_result_fn, flux_for_spws)

            for (spw, [i, q, u, v]) in spw_flux:
                spw = ms.get_spectral_window(spw)
                flux = domain.FluxMeasurement(spw=spw, I=i, Q=q, U=u, V=v)
                result.measurements[field_id].append(flux)

        return result

    def analyse(self, result):
        return result
