from __future__ import absolute_import

import os
import math
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
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
        self._to_list(['infiles', 'iflist', 'pollist'])

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
        asdm_names = [common.asdm_name(st) for st in context.observing_run]
        antenna_names = [st.antenna.name for st in context.observing_run]
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
            elif isinstance(results[i], basetask.ResultsList):
                if isinstance(results[i][0], baseline.SDBaselineResults):
                    baseline_result_id = i
                elif isinstance(results[i][0], flagdata.SDFlagDataResults):
                    flag_result_id = i

            if baseline_result_id > 0 and flag_result_id > 0:
                break


        LOG.debug('baseline_result_id=%s'%(baseline_result_id))
        LOG.debug('flag_result_id=%s'%(flag_result_id))
        
        if baseline_result_id < 0:
            msg = 'No baseline results exist. You have to execute baseline subtraction task first!'
            LOG.error(msg)
            raise RuntimeError(msg)

        baseline_result = results[baseline_result_id]
        if isinstance(baseline_result, basetask.ResultsList):
            baseline_result = baseline_result[0]

        baseline_processed = [[v['index'], v['spw'], v['pols']] for v in baseline_result.outcome['baselined']]
        spwlist_baseline = [entry[1] for entry in baseline_processed]
        net_processed = []

        if flag_result_id < 0:
            LOG.warn('No flag results available. Flag status may not be correct.')
            
            net_processed = []
            for (antennas, spw, pols) in baseline_processed:
                LOG.debug('spw=%s'%(spw))
                if iflist is not None and spw not in iflist:
                    continue
                if index_list is not None:
                    net_index_list = list(set(antennas) & set(index_list))
                else:
                    net_index_list = antennas
                LOG.debug('net_index_list=%s'%(net_index_list))
                if pollist is not None:
                    net_pollist = list(set(pols) & set(pollist))
                else:
                    net_pollist = pols
                LOG.debug('net_pollist=%s'%(net_pollist))
                for index in net_index_list:
                    for pol in net_pollist:
                        LOG.debug('append [%s, %s, %s]'%(index, spw, pol))
                        net_processed.append([index, spw, pol])

        else:
            flag_result = results[flag_result_id]
            if isinstance(flag_result, basetask.ResultsList):
                flag_result = flag_result[0]

            LOG.info('baseline result: from stage %s'%(baseline_result.stage_number))
            LOG.info('flag result: from stage %s'%(flag_result.stage_number))

            if flag_result.stage_number < baseline_result.stage_number:
                LOG.warn('Flagging has not been done after baseline subtraction. Flag results may not be correct.')

            flag_processed = [[v['antenna'], v['spw'], v['pol'], v['name']] for v in flag_result.outcome['summary']]
            for entry in flag_processed:
                antenna_name = entry[0]
                spw = entry[1]
                pol = entry[2]
                asdm = entry[3]
                st_list = [i for i in xrange(len(asdm_names))
                           if asdm_names[i] == asdm]
                antenna_list = [i for i in st_list
                                if antenna_names[i] == antenna_name]
                LOG.debug('antenna_list=%s'%(antenna_list))
                if len(antenna_list) == 0:
                    continue
                antenna = antenna_list[0]
                if iflist is not None and spw not in iflist:
                    continue
                try:
                    index = spwlist_baseline.index(spw)
                    baseline_entry = baseline_processed[index]
                    if index_list is not None:
                        net_index_list = list(set(index_list) & set(baseline_entry[0]))
                    else:
                        net_index_list = baseline_entry[0]
                    LOG.debug('net_index_list=%s'%(net_index_list))
                    if pollist is not None:
                        net_pollist = list(set(pollist) & set(baseline_entry[2]))
                    else:
                        net_pollist = baseline_entry[2]
                    LOG.debug('net_pollist=%s'%(net_pollist))
                    if antenna in net_index_list \
                       and pol in net_pollist:
                        #net_processed.append(entry)
                        net_processed.append([antenna, spw, pol])
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

