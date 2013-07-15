from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks
#from .boxworker import BoxWorker
from .resultobjects import CleanResult
#import pipeline.infrastructure.renderer as renderer

from pipeline.hif.heuristics import cleanbox

LOG = infrastructure.get_logger(__name__)


def purge(name):
    os.system('rm -fr %s' % name)
    return name

def copy_image(old_name, new_name):
    casatools.image.fromimage(infile=old_name, outfile=new_name)

def rename_image(old_name, new_name):
    with casatools.ImageReader(old_name) as image:
        image.rename(name=new_name, overwrite=True)

def set_miscinfo(name, spw=None, field=None, type=None, iter=None):
    with casatools.ImageReader(name) as image:
        info = image.miscinfo()
        if spw:
            info['spw'] = spw
        if field:
            info['field'] = field
        if type:
            info['type'] = type
        if iter is not None:
            info['iter'] = iter
        image.setmiscinfo(info)


class CleanWorkerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir, vis, mode, imagermode, imagename,
      intent, field_id, field, scan, spw, phasecenter, cell, imsize, outframe,
      nchan, start, width, weighting, robust, noise, npixels,
      restoringbeam, uvrange, cleanboxtask):

        self._init_properties(vars())


class CleanWorker(basetask.StandardTaskTemplate):
    Inputs = CleanWorkerInputs

    def is_multi_vis_task(self):
        return True
    
    def __init__(self, inputs):
        self.inputs = inputs
        # weblog renderer will plot in the root stage directory, i.e.
        # stage7 not stage7_2 
        plot_dir = os.path.join(inputs.context.report_dir,
          'stage%s' % inputs.context.stage.split('_')[0])
        self.result = CleanResult(sourcename=inputs.field,
          intent=inputs.intent, spw=inputs.spw, plotdir=plot_dir)

    def prepare(self):
        inputs = self.inputs

        iter = 0

        # derive names of clean products, remove old files as these screw up
        # the clean task
        model_name = purge(name='%s.iter%s.model' % (inputs.imagename, iter))
        image_name = purge(name='%s.iter%s.image' % (inputs.imagename, iter))
        residual_name = purge(name='%s.iter%s.residual' % (inputs.imagename,
          iter))
        psf_name = purge('%s.iter%s.psf' % (inputs.imagename, iter))
        flux_name = purge('%s.iter%s.flux' % (inputs.imagename, iter))

        # occasionally an old file (don't know which) confuses the clean
        # so that it starts corrupt. For now, delete all <imagename>.iter*
        # files
        os.system('rm -rf %s.iter*' % inputs.imagename)

        # iteration 0, the dirty image, no cleanmask
        flux_list = []
        job = casa_tasks.clean(vis=inputs.vis,
          imagename='%s.iter%s' % (inputs.imagename, iter), spw=inputs.spw, 
          selectdata=True, scan=inputs.scan, mode=inputs.mode,
          niter=0, threshold='0Jy', imagermode=inputs.imagermode,
          interactive=False, outframe=inputs.outframe, 
          nchan=inputs.nchan,
          start=inputs.start, width=inputs.width, imsize=inputs.imsize,
          cell=inputs.cell, phasecenter=inputs.phasecenter,
          weighting=inputs.weighting, robust=inputs.robust,
          noise=inputs.noise, npixels=inputs.npixels, 
          restoringbeam=inputs.restoringbeam, uvrange=inputs.uvrange,
          mask=None)

        self._executor.execute(job)

        # store spw, pol, field and type with each image and store in the
        # result
        set_miscinfo(name=model_name, spw=inputs.spw, field=inputs.field,
          type='model', iter=iter)
        self.result.set_model(iter=iter, image=model_name)

        set_miscinfo(name=image_name, spw=inputs.spw, field=inputs.field,
          type='image', iter=iter)
        self.result.set_image(iter=iter, image=image_name)

        set_miscinfo(name=residual_name, spw=inputs.spw, field=inputs.field,
          type='residual', iter=iter)
        self.result.set_residual(iter=iter, image=residual_name)

        set_miscinfo(name=psf_name, spw=inputs.spw, field=inputs.field,
          type='psf', iter=iter)
        self.result.set_psf(image=psf_name)

        set_miscinfo(name=flux_name, spw=inputs.spw, field=inputs.field,
          type='flux', iter=iter)
        self.result.set_flux(image=flux_name)

        # give the boxing task the dirty image result to start its record
        # of image statistics
        inputs.cleanboxtask.iteration_result(iter=iter, psf=self.result.psf,
          model=self.result.model, restored=self.result.image,
          residual=self.result.residual, fluxscale=self.result.flux,
          cleanmask=None, threshold=None)

        iterating = True
        while iterating:
            new_cleanmask = purge('%s.iter%s.cleanmask' % (inputs.imagename,
              iter+1))
            inputs.cleanboxtask.new_cleanmask(new_cleanmask)

            # determine cleanboxes, threshold, niter for next iteration
            box_result = self._executor.execute(inputs.cleanboxtask)

            iterating = box_result.iterating

            if iterating:
                iter += 1
                LOG.info('iter %s threshold %s' % (iter, box_result.threshold))
                LOG.info('iter %s niter %s' % (iter, box_result.niter))

                # derive names of clean products for this iteration, remove
                # old files
                old_model_name = model_name
                model_name = purge('%s.iter%s.model' % (inputs.imagename,
                  iter))
                rename_image(old_name=old_model_name, new_name=model_name)

                image_name = purge(name='%s.iter%s.image' % (
                  inputs.imagename, iter))
                residual_name = purge(name='%s.iter%s.residual' % (
                  inputs.imagename, iter))
                psf_name = purge('%s.iter%s.psf' % (inputs.imagename, iter))
                flux_name = purge('%s.iter%s.flux' % (inputs.imagename, iter))

                job = casa_tasks.clean(vis=inputs.vis, 
                  imagename='%s.iter%s' % (inputs.imagename, iter), spw=inputs.spw,
                  selectdata=True, scan=inputs.scan, mode=inputs.mode,
                  niter=box_result.niter,
                  threshold=box_result.threshold,
                  imagermode=inputs.imagermode, interactive=False,
                  outframe=inputs.outframe, nchan=inputs.nchan,
                  start=inputs.start, width=inputs.width,
                  imsize=inputs.imsize, cell=inputs.cell,
                  phasecenter=inputs.phasecenter,
                  weighting=inputs.weighting, robust=inputs.robust,
                  noise=inputs.noise, npixels=inputs.npixels,
                  restoringbeam=inputs.restoringbeam, uvrange=inputs.uvrange,
                  mask=box_result.cleanmask)

                self._executor.execute(job)

                # store the results for this iteration
                set_miscinfo(name=model_name, spw=inputs.spw,
                  field=inputs.field, type='model', iter=iter)
                self.result.set_model(iter=iter, image=model_name)

                set_miscinfo(name=image_name, spw=inputs.spw,
                  field=inputs.field, type='model', iter=iter)
                self.result.set_model(iter=iter, image=model_name)

                set_miscinfo(name=image_name, spw=inputs.spw,
                  field=inputs.field, type='image', iter=iter)
                self.result.set_image(iter=iter, image=image_name)

                set_miscinfo(name=residual_name, spw=inputs.spw,
                  field=inputs.field, type='residual', iter=iter)
                self.result.set_residual(iter=iter, image=residual_name)

                if os.path.exists(box_result.cleanmask):
                    set_miscinfo(name=box_result.cleanmask, spw=inputs.spw,
                      field=inputs.field, type='cleanmask', iter=iter)
                self.result.set_cleanmask(iter=iter,
                  image=box_result.cleanmask)

                # give the results of this iteration to the cleanboxtask
                inputs.cleanboxtask.iteration_result(iter=iter,
                  psf=self.result.psf,
                  model=self.result.model, restored=self.result.image,
                  residual=self.result.residual, fluxscale=self.result.flux,
                  cleanmask=box_result.cleanmask,
                  threshold=box_result.threshold)

        return self.result

    def analyse(self, result):
        return result
