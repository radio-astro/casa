from __future__ import absolute_import

import collections
import itertools
import string

import pipeline.domain as domain
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import fieldnames as fieldnames
from pipeline.h.tasks.common import commonfluxresults

LOG = infrastructure.get_logger(__name__)


class NormaliseFluxInputs(vdp.StandardInputs):

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

    def __init__(self, context, output_dir=None, vis=None, reference=None, transfer=None,
                 refintent=None, transintent=None):
        super(NormaliseFluxInputs, self).__init__()
        self.context = context
        self.vis = vis
        self.output_dir = output_dir
        self.reference = reference
        self.transfer = transfer
        self.refintent = refintent
        self.transintent = transintent
        

    @property
    def transfer(self):
        if callable(self._transfer):
            obsrun = self.context.observing_run
            fields = set()
            for ms in obsrun.measurement_sets:
                # first call the heuristic to get the transfer fields as a
                # string
                transfer_fields = self._transfer(ms, self.transintent)
    
                # however, we should remove the reference field should the
                # reference field also have been observed with the transfer 
                # intent
                transfers = set([i for i in utils.safe_split(transfer_fields)])
                references = set([i for i in utils.safe_split(self.reference)])
                
                fields.update(transfers.difference(references))

            return ','.join(set(fields))
             
        return self._transfer

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


class NormaliseFlux(basetask.StandardTaskTemplate):
    Inputs = NormaliseFluxInputs

    is_multi_vis_task = True
    
    def prepare(self):
        result = commonfluxresults.FluxCalibrationResults()
        return result
    
    def analyse(self, result):
        obsrun = self.inputs.context.observing_run

        for field_name in string.split(self.inputs.transfer, ','):
            fields = set(obsrun.get_fields(names=field_name))
            
            # TODO we assume that measurements from spectral windows with the
            # same ID can be averaged, though this will have to become more 
            # intelligent in future
            all_fds = [field.flux_densities for field in fields]
            if not all_fds:
                continue
            
            # flatten the flux_densities lists to a single sequence
            all_fds = list(itertools.chain(*all_fds))

            # find which spws these measurements have been taken in 
            spw_ids = set([fd.spw_id for fd in all_fds])

            averaged = collections.defaultdict(list)
            for spw_id in spw_ids:
                # get the FluxMeasurements for all spectral windows with the
                # current spw ID and average them together
                fds_in_spw = [fd for fd in all_fds 
                  if fd.spw_id == spw_id]
                print('normflux {} {}'.format(spw_id, fds_in_spw))
                if not fds_in_spw:
                    continue

                # ugly hack: get spw from first flux density measurement 
                fd_sum = domain.FluxMeasurement(fds_in_spw[0].spw_id, measures.FluxDensity(0))
                for fd in fds_in_spw:
                    fd_sum = fd_sum + fd
                fd_avg = fd_sum / (len(fds_in_spw))
                print('fd_avg {}'.format(fd_avg))

                averaged[field_name].append(fd_avg)

            # we want to set this averaged flux for any ms containing this
            # field
            ms_names = [ms.name for ms in obsrun.measurement_sets
                        if not set(ms.fields).isdisjoint(fields)]
            for vis in ms_names:
                result.set_result(vis, averaged)
                if len(vis) > 1:
                    LOG.todo('Matched spectral windows by ID when '
                                'normalising flux between measurement sets.')

        self.result = result
        return result
