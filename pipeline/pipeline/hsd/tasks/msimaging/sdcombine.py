from __future__ import absolute_import

import os, shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks
from .worker import SDImagingWorkerResults

LOG = infrastructure.get_logger(__name__)

class SDImageCombineInputs(basetask.StandardInputs):
    """
    Inputs for image plane combination
    """
    def __init__(self, context, inimages, outfile):
        self._init_properties(vars())

class SDImageCombine(basetask.StandardTaskTemplate):
    Inputs = SDImageCombineInputs
    
    def is_multi_vis_task(self):
        return True

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

            result = SDImagingWorkerResults(task=self.__class__,
                                                   success=True,
                                                   outcome=outfile)
        else:
            # Combination failed due to missing valid data
            result = SDImagingWorkerResults(task=self.__class__,
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
        if os.path.exists(imagename):
            shutil.rmtree(imagename)
        combine_args = dict(imagename=infiles, outfile=imagename,
                            mode='evalexpr', expr=expr)
        LOG.debug('Executing immath task: args=%s'%(combine_args))
        combine_job = casa_tasks.immath(**combine_args)

        # execute job
        self._executor.execute(combine_job)

        return True
