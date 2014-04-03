from __future__ import absolute_import

import os
import shutil
import glob
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools

from .. import common
from . import maskline
#from .fitting import FittingFactory
from . import fitting

LOG = infrastructure.get_logger(__name__)

class SDBaselineInputs(common.SingleDishInputs):
    """
    Inputs for baseline subtraction
    """
    @basetask.log_equivalent_CASA_call
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
        
        dummy_suffix = "_temp"
        # Clear-up old temporary scantables (but they really shouldn't exist)
        self._clearup_dummy()
        
        baselined = []

        ifnos = numpy.array(datatable.getcol('IF'))
        polnos = numpy.array(datatable.getcol('POL'))
        srctypes = numpy.array(datatable.getcol('SRCTYPE'))
        antennas = numpy.array(datatable.getcol('ANTENNA'))

        # generate storage for baselined data
        self._generate_storage_for_baselined(context, reduction_group)

        # loop over reduction group
        files = set()
        files_temp = {}
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

            # fit order determination and fitting
            fitter_cls = fitting.FittingFactory.get_fitting_class(fitfunc)

            # loop over file
            for idx in _file_index:
                _iteration = group_desc.get_iteration(idx, spwid)
                outfile = self._get_dummy_name(context, idx)
                fitter_inputs = fitter_cls.Inputs(context, idx, spwid, pols, _iteration, 
                                                  fitorder, edge, outfile)
                fitter = fitter_cls(fitter_inputs)
                fitter_result = self._executor.execute(fitter, merge=True)
                # store temporal scantable name
                if not files_temp.has_key(idx):
                    files_temp[idx] = fitter_result.outcome.pop('outtable')
                
            name_list = [context.observing_run[f].baselined_name
                         for f in _file_index]
            baselined.append({'group_id': group_id, 'iteration': iteration,
                              'name': name_list, 'index': list(_file_index),
                              'spw': spwid, 'pols': pols,
                              'lines': detected_lines,
                              'channelmap_range': channelmap_range,
                              'clusters': cluster_info})
            LOG.debug('cluster_info=%s'%(cluster_info))

        # replace working scantable with temporal baselined scantable
        for file_idx, temp_name in files_temp.items():
            blname = context.observing_run[file_idx].baselined_name
            if os.path.exists(blname) and os.path.exists(temp_name):
                shutil.rmtree(blname)
            shutil.move(temp_name, blname)

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
#             storage = context.observing_run[antenna].baselined_name
            storage = self._get_dummy_name(context, antenna)
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

    @property
    def _dummy_suffix(self):
        return "_temp"
    
    def _get_dummy_name(self, context, idx):
        """Generate temporal scantable name."""
        return context.observing_run[idx].name + self._dummy_suffix
        
    def _clearup_dummy(self):
        remove_list = glob.glob("*"+self._dummy_suffix)
        for dummy in remove_list:
            LOG.debug("Removing old temprary file '%s'" % dummy)
            shutil.rmtree(dummy)
        del remove_list


