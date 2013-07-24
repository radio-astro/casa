from __future__ import absolute_import

import os
import math
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools
from .. import common
from ..baseline import baseline
from ..flagdata import flagdata

LOG = infrastructure.get_logger(__name__)

class SDPlotFlagBaselineInputs(common.SingleDishInputs):
    """
    Inputs for flagbaseline plot
    """
    def __init__(self, context, infiles=None, iflist=None, pollist=None,
                 linewindow=None):
        self._init_properties(vars())

    @property
    def antennalist(self):
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

class SDPlotFlagBaselineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDPlotFlagBaselineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDPlotFlagBaselineResults, self).merge_with_context(context)

    def _outcome_name(self):
        return ''

class SDPlotFlagBaseline(common.SingleDishTaskTemplate):
    Inputs = SDPlotFlagBaselineInputs

    def prepare(self):
        inputs = self.inputs
        context = inputs.context
        results = [r.read() for r in context.results]
        infiles = inputs.infiles
        scantable_names = context.observing_run.st_names
        index_list = [scantable_names.index(os.path.basename(infile)) for infile in infiles]
        iflist = inputs.iflist
        antennalist = inputs.antennalist
        pollist = inputs.pollist

        baseline_result_id = -1
        flag_result_id = -1
        for i in xrange(len(results)-1, -1, -1):
            if isinstance(results[i], baseline.SDBaselineResults):
                baseline_result_id = i
            elif isinstance(results[i], flagdata.SDFlagDataResults):
                flag_result_id = i

            if baseline_result_id > 0 and flag_result_id > 0:
                break

        baseline_result = results[baseline_result_id]
        flag_result = results[flag_result_id]

        LOG.info('baseline result: from stage %s'%(baseline_result.stage_number))
        LOG.info('flag result: from stage %s'%(flag_result.stage_number))

        LOG.todo('check that flag is done after baseline')

        flag_processed = [[v['index'], v['spw'], v['pol']] for v in flag_result.outcome['summary']]
        baseline_processed = [[v['index'], v['spw'], v['pols']] for v in baseline_result.outcome['baselined']]
        spwlist_baseline = [entry[1] for entry in baseline_processed]
        net_processed = []
        for entry in flag_processed:
            if iflist is not None and entry[1] not in iflist:
                continue
            try:
                index = spwlist_baseline.index(entry[1])
                baseline_entry = baseline_processed[index]
                if index_list is not None:
                    net_index_list = list(set(index_list) & set(baseline_entry[0]))
                else:
                    net_index_list = baseline_entry[0]
                if pollist is not None:
                    net_pollist = list(set(pollist) & set(baseline_entry[2]))
                else:
                    net_pollist = baseline_entry[2]
                if entry[0] in net_index_list \
                   and entry[2] in net_pollist:
                    net_processed.append(entry)
            except:
                pass


        LOG.info('net_processed=%s'%(net_processed))

        #outcome = {'baseline': baseline_processed,
        #           'flag': flag_processed}
        outcome = {'baselined': [], 'edge': [0,0]}
        spw_list = numpy.unique([entry[1] for entry in net_processed])
        temporary_dict = dict([(spw, {'index': set(), 'pols': set()}) \
                               for spw in spw_list])
        for entry in net_processed:
            temporary_dict[entry[1]]['index'].add(entry[0])
            temporary_dict[entry[1]]['pols'].add(entry[2])
        for (k,v) in temporary_dict.items():
            entry = {'spw': k, 'index': list(v['index']), 'pols': list(v['pols'])}
            entry['name'] = [context.observing_run[index].baselined_name \
                             for index in entry['index']]
            outcome['baselined'].append(entry)
        results = SDPlotFlagBaselineResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
                
        if self.inputs.context.subtask_counter is 0: 
            results.stage_number = self.inputs.context.task_counter - 1
        else:
            results.stage_number = self.inputs.context.task_counter 
                
        return results

        datatable = context.observing_run.datatable_instance
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]
        
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

                
            beam_size = st.beam_size[spwid]
            srctype = st.calibration_strategy['srctype']
            _file_index = set(file_index) & set([m.antenna for m in group_desc])
            files = files | _file_index
            pattern = st.pattern[spwid][pols[0]]
            index_list = numpy.where(numpy.logical_and(ifnos == spwid, srctypes==srctype))[0]

            # loop over file
            for idx in _file_index:
                st = context.observing_run[idx]
                filename_in = st.name
                filename_out = st.baselined_name
                iteration = group_desc.get_iteration(idx, spwid)
                LOG.debug('iteration (antenna %s, spw %s): %s'%(idx,spwid,iteration))
                if iteration == 0:
                    utils.createExportTable(bltable_name)
                for pol in pols:
                    pass
                # output summary of baseline fitting
                BaselineSummary.summary(bltable_name, st.basename, spwid, iteration, edge, datatable, ant_indices, nchan)


            #for f in _file_index:
            #    name = context.observing_run[f].baselined_name
            name_list = [context.observing_run[f].baselined_name
                         for f in _file_index]
            baselined.append({'name': name_list, 'index': list(_file_index),
                              'spw': spwid, 'pols': pols,
                              'lines': detected_lines,
                              'clusters': cluster_info})

        outcome = {'datatable': datatable,
                   'baselined': baselined,
                   'edge': edge}
        results = SDPlotFlagBaselineResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
                
        if self.inputs.context.subtask_counter is 0: 
            results.stage_number = self.inputs.context.task_counter - 1
        else:
            results.stage_number = self.inputs.context.task_counter 
                
        return results

    def analyse(self, result):
        return result


class BaselineSummary(object):
    @staticmethod
    def summary(tablename, stname, spw, iteration, edge, datatable, index_list, nchan):
        header = 'Summary of cspline_baseline for %s (spw%s, iter%s)'%(stname, spw, iteration)
        separator = '=' * len(header)
        LOG.info(separator)
        LOG.info(header)
        LOG.info(separator)

        # edge channels dropped
        LOG.info('1) Number of edge channels dropped')
        LOG.info('')
        LOG.info('\t left edge: %s channels'%(edge[0]))
        LOG.info('\tright edge: %s channels'%(edge[1]))
        LOG.info('')

        # line masks
        LOG.info('2) Masked fraction on each channel')
        LOG.info('')
        histogram = numpy.zeros(nchan, dtype=float)
        nrow = len(index_list)
        for idx in index_list:
            masklist = datatable.getcell('MASKLIST', idx)
            for mask in masklist:
                start = mask[0]
                end = mask[1] + 1
                for ichan in xrange(start, end):
                    histogram[ichan] += 1.0
        nonzero_channels = histogram.nonzero()[0]
        if len(nonzero_channels) > 0:
            dnz = nonzero_channels[1:] - nonzero_channels[:-1]
            mask_edges = numpy.where(dnz > 1)[0]
            start_chan = nonzero_channels.take([0]+(mask_edges+1).tolist())
            end_chan = nonzero_channels.take(mask_edges.tolist()+[-1])
            merged_start_chan = [start_chan[0]]
            merged_end_chan = []
            for i in xrange(1, len(start_chan)):
                if start_chan[i] - end_chan[i-1] > 4:
                    merged_start_chan.append(start_chan[i])
                    merged_end_chan.append(end_chan[i-1])
            merged_end_chan.append(end_chan[-1])
            LOG.info('channel|fraction')
            LOG.info('-------|---------')
            if merged_start_chan[0] > 0:
                LOG.info('%7d|%9.1f%%'%(0, 0))
                LOG.info('       ~')
                LOG.info('       ~')
            #for ichan in xrange(len(histogram)):
            for i in xrange(len(merged_start_chan)):
                for j in xrange(max(0,merged_start_chan[i]-1), min(nchan,merged_end_chan[i]+2)):
                    LOG.info('%7d|%9.1f%%'%(j, histogram[j]/nrow*100.0))
                if merged_end_chan[i] < nchan-2:
                    LOG.info('       ~')
                    LOG.info('       ~')
            if merged_end_chan[-1] < nchan-2:
                LOG.info('%7d|%9.1f%%'%(nchan-1, 0))
        else:
            LOG.info('\tNo line mask')
        LOG.info('')
            
        BaselineSummary.cspline_summary(tablename)

        footer = separator
        LOG.info(footer)

    @staticmethod
    def cspline_summary(tablename):
        # number of segments for cspline_baseline
        with casatools.TableReader(tablename) as tb:
            nrow = tb.nrows()
            num_segments = [tb.getcell('Sections', irow).shape[0] \
                            for irow in xrange(nrow)]
        unique_values = numpy.unique(num_segments)
        max_segments = max(unique_values) + 2
        LOG.info('3) Frequency distribution for number of segments')
        LOG.info('')
        LOG.info('# of segments|frequency')
        LOG.info('-------------|---------')
        #for val in unique_values:
        for val in xrange(1, max_segments):
            count = num_segments.count(val)
            LOG.info('%13d|%9d'%(val, count))
        LOG.info('')

