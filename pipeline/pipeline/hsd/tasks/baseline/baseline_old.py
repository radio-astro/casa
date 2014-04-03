from __future__ import absolute_import
import os
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools

from .. import common
from . import maskline
#from .fitting_old import FittingFactory
from . import fitting_old as fitting
from .baseline import SDBaselineInputs
from .baseline import SDBaselineResults

LOG = infrastructure.get_logger(__name__)

class SDBaselineInputsOld(SDBaselineInputs):
    pass

class SDBaseline(common.SingleDishTaskTemplate):
    Inputs = SDBaselineInputsOld

    def prepare(self):
        inputs = self.inputs
        context = inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        infiles = inputs.infiles
        iflist = inputs.iflist
        #antennalist = inputs.antennalist
        pollist = inputs.pollist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]

        window = [] if inputs.linewindow is None else inputs.linewindow
        edge = (0,0) if inputs.edge is None else inputs.edge
        broadline = False if inputs.broadline is None else inputs.broadline
        fitorder = 'automatic' if inputs.fitorder is None or inputs.fitorder < 0 else inputs.fitorder
        fitfunc = 'spline' if inputs.fitfunc is None else inputs.fitfunc
        
        baselined = []

        ifnos = numpy.array(datatable.getcol('IF'))
        polnos = numpy.array(datatable.getcol('POL'))
        srctypes = numpy.array(datatable.getcol('SRCTYPE'))
        antennas = numpy.array(datatable.getcol('ANTENNA'))

        # generate storage for baselined data
        self._generate_storage_for_baselined(context, reduction_group)

        # loop over reduction group
        files = set()
        for (group_id,group_desc) in reduction_group.items():            
            # assume all members have same spw and pollist
            first_member = group_desc[0]
            spwid = first_member.spw
            LOG.debug('spwid=%s'%(spwid))
            pols = first_member.pols
            iteration = first_member.iteration[0]
            if pollist is not None:
                pols = list(set(pollist) & set(pols))

            # skip spw not included in iflist
            if iflist is not None and spwid not in iflist:
                LOG.debug('Skip spw %s'%(spwid))
                continue

            # reference data is first scantable 
            st = context.observing_run[first_member.antenna]

            # skip channel averaged spw
            nchan = group_desc.nchan
            if nchan == 1:
                LOG.info('Skip channel averaged spw %s.'%(spwid))
                continue

                
            srctype = st.calibration_strategy['srctype']
            _file_index = set(file_index) & set([m.antenna for m in group_desc])
            files = files | _file_index
            index_list = numpy.where(numpy.logical_and(ifnos == spwid, srctypes==srctype))[0]
            maskline_inputs = maskline.MaskLine.Inputs(context, list(_file_index), spwid, iteration, 
                                                       index_list, window, edge, broadline)
            maskline_task = maskline.MaskLine(maskline_inputs)
            maskline_result = self._executor.execute(maskline_task, merge=True)
            detected_lines = maskline_result.outcome['detected_lines']
            channelmap_range = maskline_result.outcome['channelmap_range']
            cluster_info = maskline_result.outcome['cluster_info']

            #LOG.info('detected_lines=%s'%(detected_lines))
            #LOG.info('cluster_info=%s'%(cluster_info))

            # filenamer
            namer = filenamer.BaselineSubtractedTable()
            namer.spectral_window(spwid)

            # fit order determination and fitting
            fitter_cls = fitting.FittingFactory.get_fitting_class(fitfunc)

            # loop over file
            for idx in _file_index:
                iteration = group_desc.get_iteration(idx, spwid)
                fitter_inputs = fitter_cls.Inputs(context, idx, spwid, pols, iteration, 
                                                  fitorder, edge)
                fitter = fitter_cls(fitter_inputs)
                fitter_result = self._executor.execute(fitter, merge=True)
                
            name_list = [context.observing_run[f].baselined_name
                         for f in _file_index]
            baselined.append({'name': name_list, 'index': list(_file_index),
                              'spw': spwid, 'pols': pols,
                              'lines': detected_lines,
                              'channelmap_range': channelmap_range,
                              'clusters': cluster_info})
            LOG.debug('cluster_info=%s'%(cluster_info))

        outcome = {'baselined': baselined,
                   'edge': edge}
        results = SDBaselineResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
                
        if self.inputs.context.subtask_counter is 0: 
            results.stage_number = self.inputs.context.task_counter - 1
        else:
            results.stage_number = self.inputs.context.task_counter 
                
        return results

    def analyse(self, result):
        return result

    def _generate_storage_for_baselined(self, context, reduction_group):
        for antenna in xrange(len(context.observing_run)):
            reference = context.observing_run[antenna].name
            storage = context.observing_run[antenna].baselined_name
            if not os.path.exists(storage):
                # generate
                self._generate_storage_from_reference(storage, reference)
            iter_counter_list = []
            for (id, desc) in reduction_group.items():
                for member in desc:
                    if member.antenna == antenna:
                        iter_counter_list.extend(member.iteration)
            LOG.debug('iter_counter_list=%s'%(iter_counter_list))
            if all(numpy.array(iter_counter_list) == 0):
                # generate
                self._generate_storage_from_reference(storage, reference)

    def _generate_storage_from_reference(self, storage, reference):
        LOG.debug('generating %s from %s'%(os.path.basename(storage), os.path.basename(reference)))
        with casatools.TableReader(reference) as tb:
            copied = tb.copy(storage, deep=True, returnobject=True)
            copied.close()
