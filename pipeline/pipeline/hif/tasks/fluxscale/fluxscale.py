from __future__ import absolute_import

import ast
import re

import numpy

import pipeline.domain as domain
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as fcaltable
from pipeline.h.heuristics import fieldnames as fieldnames
from pipeline.h.tasks.common import commonfluxresults
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry
from .. import gaincal

LOG = infrastructure.get_logger(__name__)


class FluxscaleInputs(vdp.StandardInputs):
    @vdp.VisDependentProperty
    def fluxtable(self):
        namer = fcaltable.FluxCaltable()
        casa_args = self._get_task_args(ignore=('fluxtable',))
        return namer.calculate(output_dir=self.output_dir, stage=self.context.stage, **casa_args)

    @vdp.VisDependentProperty
    def reference(self):
        # this will give something like '0542+3243,0343+242'
        field_fn = fieldnames.IntentFieldnames()
        reference_fields = field_fn.calculate(self.ms, self.refintent)
        # run the answer through a set, just in case there are duplicates
        fields = {s for s in utils.safe_split(reference_fields)}

        return ','.join(fields)

    refintent = vdp.VisDependentProperty(default='AMPLITUDE')

    @vdp.VisDependentProperty
    def refspwmap(self):
        return self.ms.reference_spwmap

    @refspwmap.convert
    def refspwmap(self, value):
        def element_to_int(e):
            if isinstance(e, (list, tuple)):
                return [element_to_int(i) for i in e]
            return int(e)

        if value not in (None, -1):
            value = [element_to_int(n) for n in ast.literal_eval(str(value))]

        return value

    @vdp.VisDependentProperty
    def transfer(self):
        transfer_fn = fieldnames.IntentFieldnames()
        # call the heuristic to get the transfer fields as a string
        transfer_fields = transfer_fn.calculate(self.ms, self.transintent)

        # remove the reference field should it also have been observed with
        # the transfer intent
        transfers = set(self.ms.get_fields(task_arg=transfer_fields))
        references = set(self.ms.get_fields(task_arg=self.reference))
        diff = transfers.difference(references)

        transfer_names = {f.name for f in diff}
        fields_with_name = self.ms.get_fields(name=transfer_names)
        if len(fields_with_name) is not len(diff) or len(diff) is not len(transfer_names):
            return ','.join([str(f.id) for f in diff])
        else:
            return ','.join(transfer_names)

    transintent = vdp.VisDependentProperty(default='PHASE,BANDPASS,CHECK')

    def to_casa_args(self):
        casa_args = super(FluxscaleInputs, self).to_casa_args()
        # delete pipeline-only properties that shouldn't be passed to
        # the CASA task
        del casa_args['refintent']
        del casa_args['transintent']
        return casa_args

    def __init__(self, context, output_dir=None, vis=None, caltable=None, fluxtable=None, reference=None, transfer=None,
                 refspwmap=None, refintent=None, transintent=None):
        super(FluxscaleInputs, self).__init__()
        self.context = context
        self.vis = vis
        self.output_dir = output_dir
        self.caltable = caltable
        self.fluxtable = fluxtable
        self.reference = reference
        self.transfer = transfer
        self.refspwmap = refspwmap
        self.refintent = refintent
        self.transintent = transintent


@task_registry.set_equivalent_casa_task('hif_fluxscale')
class Fluxscale(basetask.StandardTaskTemplate):
    Inputs = FluxscaleInputs

    def _do_gaincal(self):
        inputs = self.inputs
        gaincal_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
            vis = inputs.vis,
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

        no_result = numpy.array([-1., -1., -1., -1.])
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

            for (spw_id, [i, q, u, v]) in spw_flux:
                flux = domain.FluxMeasurement(spw_id=spw_id, I=i, Q=q, U=u, V=v)

                uI, uQ, uU, uV = flux_for_field[spw_id]['fluxdErr']
                unc = domain.FluxMeasurement(spw_id=spw_id, I=uI, Q=uQ, U=uU, V=uV)
                flux.uncertainty = unc
                
                result.measurements[field_id].append(flux)

        return result

    def analyse(self, result):
        return result
