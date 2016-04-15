from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks
from .. import common
from . import worker

LOG = infrastructure.get_logger(__name__)

class SDImageCombineInputs(common.SingleDishInputs):
    """
    Inputs for image plane combination
    """
    def __init__(self, context, inimages, outfile):
        self._init_properties(vars())

# class SDImageCombineResults(common.SingleDishResults):
#     def __init__(self, task=None, success=None, outcome=None):
#         super(SDImageCombineResults, self).__init__(task, success, outcome)

#     def merge_with_context(self, context):
#         super(SDImageCombineResults, self).merge_with_context(context)

#     def _outcome_name(self):
#         # return [image.imagename for image in self.outcome]
#         return self.outcome


class SDImageCombine(common.SingleDishTaskTemplate):
    Inputs = SDImageCombineInputs
    
    @common.datatable_setter
    def prepare(self):
        infiles = self.inputs.inimages
        outfile = self.inputs.outfile
        inweights = [name+".weight" for name in infiles]
        outweight = outfile + ".weight"
        num_in = len(infiles)

        # combine weight images
        LOG.info("Generating combined weight image.")
        expr = [ ("IM%d" % idx) for idx in range(num_in) ]
        status = self._do_combine(inweights, outweight, str("+").join(expr))
        if status is True:
            # combine images with weight
            LOG.info("Generating combined image.")
            in_images = list(infiles) + list(inweights) + [outweight]
            expr = [ "IM%d*IM%d" % (idx, idx+num_in) for idx in range(num_in) ]
            expr = "(%s)/IM%d" % (str("+").join(expr), len(in_images)-1)
            status = self._do_combine(in_images, outfile, expr)

        if status is True:
            # Need to replace NaNs in masked pixels
            with casatools.ImageReader(outfile) as ia:
                stat = ia.statistics()
                shape = ia.shape()
                # replacemaskedpixels fails if all pixels are valid
                if shape.prod() > stat['npts'][0]:
                    ia.replacemaskedpixels(0.0, update=False)

            result = worker.SDImagingWorkerResults(task=self.__class__,
                                                   success=True,
                                                   outcome=outfile)
        else:
            # Combination failed due to missing valid data
            result = worker.SDImagingWorkerResults(task=self.__class__,
                                                   success=False,
                                                   outcome=None)

        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 

        return result

    def analyse(self, result):
        return result

    def _do_combine(self, infiles, imagename, expr):
        combine_args = dict(imagename=infiles, outfile=imagename,
                            mode='evalexpr', expr=expr)
        LOG.debug('Executing immath task: args=%s'%(combine_args))
        combine_job = casa_tasks.immath(**combine_args)

        # execute job
        self._executor.execute(combine_job)

        return True
