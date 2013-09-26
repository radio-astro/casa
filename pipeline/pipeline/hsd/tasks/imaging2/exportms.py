from __future__ import absolute_import

import os
import numpy

import pipeline.infrastructure as infrastructure
from pipeline.infrastructure import casa_tasks
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

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return self.outcome['outfiles']


class ExportMS(common.SingleDishTaskTemplate):
    Inputs = ExportMSInputs
    
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
            infile = data.baselined_name
            asdm_name = common.asdm_name(data)
            antenna_name = data.antenna.name
            outfile = os.path.join(output_dir, '.'.join([asdm_name, antenna_name, 'ms']))
            args = {'infile': infile,
                    'outfile': outfile,
                    'outform': 'MS2',
                    'overwrite': True}
            job = casa_tasks.sdsave(**args)
            # job = create_export_job(data, output_dir)
                    
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

    
