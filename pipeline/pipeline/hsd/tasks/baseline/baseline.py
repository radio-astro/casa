from __future__ import absolute_import

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer

from .. import common
from . import maskline
#from .fitting import FittingFactory
from . import fitting

LOG = infrastructure.get_logger(__name__)

class SDBaselineInputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
    def __init__(self, context, infiles=None, iflist=None, pollist=None,
                 linewindow=None, edge=None, broadline=None, fitorder=None,
                 fitfunc=None):
        self._init_properties(vars())
        self._to_list(['infiles', 'iflist', 'pollist', 'edge', 'linewindow'])
        self._to_bool('broadline')
        self._to_numeric('fitorder')
        if isinstance(self.fitorder, float):
            self.fitorder = int(self.fitorder)

    @property
    def antennalist(self):
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

class SDBaselineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDBaselineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDBaselineResults, self).merge_with_context(context)

        # manually sync with datatable on disk
        context.observing_run.datatable_instance.sync()

        # increment iteration counter
        # register detected lines to reduction group member
        reduction_group = context.observing_run.reduction_group
        for b in self.outcome['baselined']:
            spw = b['spw']
            antenna = b['index']
            pols = b['pols']
            lines = b['lines']
            channelmap_range = b['channelmap_range']
            for _ant in antenna:
                group_id = -1
                for (idx,desc) in reduction_group.items():
                    if desc[0].spw == spw:
                        group_id = idx
                        break
                if group_id >= 0:
                    reduction_group[group_id].iter_countup(_ant, spw, pols)
                    reduction_group[group_id].add_linelist(lines, _ant, spw, pols, 
                                                           channelmap_range=channelmap_range)
                st = context.observing_run[_ant]
                st.work_data = st.baselined_name

    def _outcome_name(self):
        return ['%s: %s (spw=%s, pol=%s)'%(idx, name, b['spw'], b['pols'])
                for b in self.outcome['baselined']
                for (idx,name) in zip(b['index'], b['name'])]

class SDBaseline(common.SingleDishTaskTemplate):
    Inputs = SDBaselineInputs

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
                              'channelmap_range': detected_lines,
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


