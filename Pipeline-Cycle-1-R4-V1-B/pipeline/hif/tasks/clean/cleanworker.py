from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks
from .boxworker import BoxWorker
from .resultobjects import CleanResult
import pipeline.infrastructure.renderer as renderer

#import pipeline.hif.heuristics as heuristics
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
      restfreq, nchan, start, width, weighting, robust, noise):

        self._init_properties(vars())


class CleanWorker(basetask.StandardTaskTemplate):
    Inputs = CleanWorkerInputs

    def is_multi_vis_task(self):
        return True
    
    def __init__(self, inputs):
        self.inputs = inputs
        self.result = CleanResult(sourcename=inputs.field,
          intent=inputs.intent, spw=inputs.spw,
          plotdir=renderer.htmlrenderer.get_plot_dir(context=inputs.context,
          stage_number=inputs.context.task_counter))

        self.heuristics = cleanbox.CleanBoxHeuristics()

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
        cleanmask = None
        job = casa_tasks.clean(vis=inputs.vis,
          imagename='%s.iter%s' % (inputs.imagename, iter),
          field=inputs.field, spw=inputs.spw, 
          selectdata=True, scan=inputs.scan, mode=inputs.mode,
          niter=0, threshold='0Jy', imagermode=inputs.imagermode,
          interactive=False, outframe=inputs.outframe, 
          restfreq=inputs.restfreq, nchan=inputs.nchan,
          start=inputs.start, width=inputs.width, imsize=inputs.imsize,
          cell=inputs.cell, phasecenter=inputs.phasecenter,
          weighting=inputs.weighting, robust=inputs.robust,
          noise=inputs.noise, mask=cleanmask)

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

        iterating = True
        cleanmask = None
        while iterating:
            # get some information on the residual
            model_sum, clean_rms, non_clean_rms, residual_max, residual_min,\
              rms2d, image_max = cleanbox.analyse_clean_result(
              self.result.model, self.result.image, self.result.residual,
              self.result.flux, cleanmask)

            old_cleanmask = cleanmask
            cleanmask = purge('%s.iter%s.cleanmask' % (inputs.imagename,
              iter+1))

            # determine cleanboxes and threshold for next iteration
            boxinputs = BoxWorker.Inputs(context=inputs._context, 
              output_dir=inputs.output_dir, vis=None, psf=self.result.psf,
              model=self.result.model, restored=self.result.image, 
              residual=self.result.residual, fluxscale=self.result.flux,
              old_cleanmask=old_cleanmask, new_cleanmask=cleanmask)
            boxtask = BoxWorker(boxinputs)
            box_result = self._executor.execute(boxtask)

            # decide whether to clean further
            iterating = self.heuristics.clean_more(loop=iter,
              new_threshold=box_result.threshold,
              sum=model_sum, residual_max=residual_max,
              residual_min=residual_min, non_cleaned_rms=non_clean_rms,
              island_peaks=box_result.island_peaks)

            if iterating:
                iter += 1
                LOG.info('iter %s threshold %sJy' % (iter,
                  box_result.threshold))

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
                  imagename='%s.iter%s' % (inputs.imagename, iter),
                  field=inputs.field, spw=inputs.spw,
                  selectdata=True, scan=inputs.scan, mode=inputs.mode,
                  niter=1000, threshold='%sJy' % box_result.threshold,
                  imagermode=inputs.imagermode, interactive=False,
                  outframe=inputs.outframe,
                  restfreq=inputs.restfreq, nchan=inputs.nchan,
                  start=inputs.start, width=inputs.width,
                  imsize=inputs.imsize, cell=inputs.cell,
                  phasecenter=inputs.phasecenter,
                  weighting=inputs.weighting, robust=inputs.robust,
                  noise=inputs.noise, mask=box_result.cleanmask)

                self._executor.execute(job)

                # store the results for this iteration
                set_miscinfo(name=image_name, spw=inputs.spw,
                  field=inputs.field, type='model', iter=iter)
                self.result.set_model(iter=iter, image=model_name)

                set_miscinfo(name=image_name, spw=inputs.spw,
                  field=inputs.field, type='image', iter=iter)
                self.result.set_image(iter=iter, image=image_name)

                set_miscinfo(name=residual_name, spw=inputs.spw,
                  field=inputs.field, type='residual', iter=iter)
                self.result.set_residual(iter=iter, image=residual_name)

                set_miscinfo(name=cleanmask, spw=inputs.spw,
                  field=inputs.field, type='cleanmask', iter=iter)
                self.result.set_cleanmask(iter=iter, image=cleanmask)

        return self.result

    def analyse(self, result):
        return result
