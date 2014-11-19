from __future__ import absolute_import

import os
import numpy

import pipeline.infrastructure as infrastructure
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import casatools
from .. import common

LOG = infrastructure.get_logger(__name__)

class ExportMSInputs(common.SingleDishInputs):
    """
    Inputs for exporting data to MS 
    """
    def __init__(self, context, infiles=None, iflist=None, pollist=None):
        self._init_properties(vars())
        self._to_list(['infiles', 'iflist', 'pollist'])

class ExportMSResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(ExportMSResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(ExportMSResults, self).merge_with_context(context)
        observing_run = context.observing_run
        outfiles = self.outcome
        keys = outfiles.keys()
        for i in xrange(len(observing_run)):
            if i in keys:
                observing_run[i].exported_ms = outfiles[i]
            else:
                observing_run[i].exported_ms = None

    def _outcome_name(self):
        return self.outcome


class ExportMS(common.SingleDishTaskTemplate):
    Inputs = ExportMSInputs
    
    @common.datatable_setter
    def prepare(self):
        # for each data
        context = self.inputs.context
        reduction_group = context.observing_run.reduction_group
        output_dir = context.output_dir
        infiles = self.inputs.infiles

        outfiles = {}
        index_list = []
        for (group_id, group_desc) in reduction_group.items():
            index_list.extend([member.antenna for member in group_desc])
                    
        unique_index_list = numpy.unique(index_list)
        index_for_infiles = [context.observing_run.st_names.index(v) 
                             for v in infiles]
        final_index_list = list(set(unique_index_list) & set(index_for_infiles))

        for index in final_index_list:
            data = context.observing_run[index]
            # create job
            infile = data.work_data
#             infile = data.baselined_name
#             if not os.path.exists(infile):
#                 infile = data.name
            self._modify_molecules(infile)
            asdm_name = common.asdm_name(data)
            antenna_name = data.antenna.name
            outfile = os.path.join(output_dir, '.'.join([asdm_name, antenna_name, 'ms']))
            args = {'infile': infile,
                    'outfile': outfile,
                    'outform': 'MS2',
                    'overwrite': True}
            job = casa_tasks.sdsave(**args)
                    
            # execute job
            self._executor.execute(job)

            outfiles[index] = outfile
            
        result = ExportMSResults(task=self.__class__,
                                 success=True,
                                 outcome=outfiles)
        result.task = self.__class__

        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 

        return result
    
    def analyse(self, result):
        return result

    def _modify_molecules(self, infile):
        molecules_table = os.path.join(infile, 'MOLECULES')
        with casatools.TableReader(molecules_table, nomodify=False) as tb:
            nrow = tb.nrows()
            rest_freqs = [tb.getcell('RESTFREQUENCY',irow) for irow in xrange(nrow)]
            max_length = max(map(len, rest_freqs))
            for irow in xrange(nrow):
                if len(rest_freqs[irow]) < max_length:
                    zero_restfreqs = numpy.zeros(max_length, dtype=rest_freqs[irow].dtype)
                    nonames = ['none'] * max_length
                    tb.putcell('RESTFREQUENCY', irow, zero_restfreqs) 
                    tb.putcell('NAME', irow, nonames)
                    tb.putcell('FORMATTEDNAME', irow, nonames)
