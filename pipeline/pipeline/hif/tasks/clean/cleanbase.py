from __future__ import absolute_import

import os
import re
import shutil
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks
from .resultobjects import CleanResult
from pipeline.hif.heuristics import makecleanlist

LOG = infrastructure.get_logger(__name__)

# The basic clean tasks classes. Clean performs a single clean run.

class CleanBaseInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, imagename=None,
        intent=None, field=None, spw=None, uvrange=None, mode=None,
        imagermode=None, outframe=None, imsize=None, cell=None,
        phasecenter=None, nchan=None, start=None, width=None, stokes=None,
	weighting=None, robust=None, noise=None, npixels=None, restoringbeam=None, 
        iter=None, mask=None, niter=None, threshold=None, result=None):

        self._init_properties(vars())

    @property
    def imagename(self):
        if self._imagename is None:
            return ''
        return self._imagename

    @imagename.setter
    def imagename(self, value):
         self._imagename = value

    @property
    def intent(self):
        if self._intent is None:
            return ''
        return self._intent

    @intent.setter
    def intent(self, value):
        self._intent = value

    @property
    def field(self):
        if self._field is None:
            return ''
        return self._field

    @field.setter
    def field(self, value):
        self._field = value

    @property
    def spw(self):
        if self._spw is None:
            mslist = self.context.observing_run.measurement_sets
            spws = mslist[0].get_spectral_windows()
            spwids = [spw.id for spw in spws]
            return ','.join(spwids)
        return self._spw

    @spw.setter
    def spw(self, value):
        self._spw = value

    @property
    def uvrange(self):
        if self._uvrange is None:
            return ''
        return self._uvrange

    @uvrange.setter
    def uvrange(self, value):
        self._uvrange = value

    @property
    def mode(self):
        if self._mode is None:
            if 'TARGET' in self.intent:
                return 'frequency'
            else:
                return 'mfs'
        return self._mode

    @mode.setter
    def mode(self, value):
        self._mode = value

    @property
    def imagermode(self):
        if self._imagermode is None:
            return ''
        return self._imagermode

    @imagermode.setter
    def imagermode(self, value):
         self._imagermode = value

    @property
    def outframe(self):
        if self._outframe is None:
            return 'LSRK'
        return self._outframe

    @outframe.setter
    def outframe(self, value):
         self._outframe = value

    @property
    def imsize(self):
        if self._imsize is None:
            return []
        return self._imsize

    @imsize.setter
    def imsize(self, value):
         self._imsize = value

    @property
    def cell(self):
        if self._cell is None:
            return []
        return self._cell

    @cell.setter
    def cell(self, value):
        self._cell = value

    @property
    def phasecenter(self):
        if self._phasecenter is None:
            return ''
        return self._phasecenter

    @phasecenter.setter
    def phasecenter(self, value):
        self._phasecenter = value

    @property
    def nchan(self):
        if self._nchan is None:
            return -1
        return self._nchan

    @nchan.setter
    def nchan(self, value):
         self._nchan = value

    @property
    def start(self):
        if self._start is None:
            return ''
        return self._start

    @start.setter
    def start(self, value):
         self._start = value

    @property
    def width(self):
        if self._width is None:
            return ''
        return self._width

    @width.setter
    def width(self, value):
         self._width = value

    @property
    def stokes(self):
        if self._stokes is None:
            return 'I'
        return self._stokes

    @stokes.setter
    def stokes(self, value):
         self._stokes = value

    @property
    def weighting(self):
        if self._weighting is None:
            return 'natural'
        return self._weighting

    @weighting.setter
    def weighting(self, value):
         self._weighting = value

    @property
    def robust(self):
        if self._robust is None:
            return 0.0
        return self._robust

    @robust.setter
    def robust(self, value):
         self._robust = value

    @property
    def noise(self):
        if self._noise is None:
            return '1.0Jy'
        return self._noise

    @noise.setter
    def noise(self, value):
        self._noise = value

    @property
    def npixels(self):
        if self._npixels is None:
            return 0
        return self._npixels

    @npixels.setter
    def npixels(self, value):
        self._npixels = value

    @property
    def restoringbeam(self):
        if self._restoringbeam is None:
            return ''
        return self._restoringbeam

    @restoringbeam.setter
    def restoringbeam(self, value):
        self._restoringbeam = value

    @property
    def iter(self):
        if self._iter is None:
            return 0
        return self._iter

    @iter.setter
    def iter(self, value):
         self._iter = value

    @property
    def threshold(self):
        if self._threshold is None:
            return '0.0mJy'
        return self._threshold

    @threshold.setter
    def threshold(self, value):
        self._threshold = value

    @property
    def niter(self):
        if self._niter is None:
            return 500
        return self._niter

    @niter.setter
    def niter(self, value):
         self._niter = value

    @property
    def mask(self):
        if self._mask is None:
            return ''
        return self._mask

    @mask.setter
    def mask(self, value):
         self._mask = value

class CleanBase(basetask.StandardTaskTemplate):

    Inputs = CleanBaseInputs

    def is_multi_vis_task(self):
        return True
    
    def prepare(self):
        inputs = self.inputs

        # Make sure inputs.vis is a list, even it is one that contains a
        # single measurement set
        if type(inputs.vis) is not types.ListType:
            inputs.vis = [inputs.vis]

        # Instantiate the clean list heuristics class
        clheuristics = makecleanlist.MakeCleanListHeuristics(
            context=inputs.context, vislist=inputs.vis, spw=inputs.spw)

        # Generate the image name if one is not supplied.
        if inputs.imagename == '':
            inputs.imagename = clheuristics.imagename(intent=inputs.intent,
                field=inputs.field, spwspec=inputs.spw)

        # Determine the default imagermode
        if inputs.imagermode == '':
            inputs.imagermode = clheuristics.imagermode (inputs.intent,
                inputs.field)

        # Determine the phase center.
        if inputs.phasecenter == '':
            field_id = clheuristics.field (inputs.intent, inputs.field)
            inputs.phasecenter = clheuristics.phasecenter(field_id)

        # Get short cut specs of field to be cleaned. Are these really needed?
        intent = inputs.intent
        field = inputs.field
        spw = inputs.spw

        # Adjust the width to get around problems with increasing / decreasing
	# frequency with channel issues.
        if inputs.width == '':
            if inputs.mode != 'mfs':
                width = clheuristics.width(int(spw.split(',')[0]))
                #width = inputs.width
            else:
                width = inputs.width
        else:
            width = inputs.width
	inputs.width = width

	if inputs.stokes == 'Q':
	    ncorr = clheuristics.ncorr(int(spw.split(',')[0]))
	    if ncorr <= 1 :
	        LOG.warning('Q noise estimate invalid ncorrelation is %s' % ncorr )

        # Construct regex for string matching - escape likely problem
        # chars. Simpler way to do this ?
        re_field = field.replace('*', '.*')
        re_field = re_field.replace('[', '\[')
        re_field = re_field.replace(']', '\]')
        re_field = re_field.replace('(', '\(')
        re_field = re_field.replace(')', '\)')
        re_field = re_field.replace('+', '\+')

        # Use scanids to select data with the specified intent
	#    We should not have to do this.
        scanidlist = []
        for vis in inputs.vis:
            ms = inputs.context.observing_run.get_ms(name=vis)
            scanids = [scan.id for scan in ms.scans if
                intent in scan.intents and
                re.search(pattern=re_field, string=str(scan.fields))]
            scanids = str(scanids)
            scanids = scanids.replace('[', '')
            scanids = scanids.replace(']', '')
            scanidlist.append(scanids)

        # If imsize not set then use heuristic code to calculate the
        # centers for each field  / spw
        imsize = inputs.imsize
        cell = inputs.cell
        if imsize == [] or cell ==[]:

            # The heuristics cell size  is always the same for x and y as
            # the value derives from a single value returned by imager.advise
            cell, beam = clheuristics.cell(
                field_intent_list=[(field, intent)], spwspec=spw)
            if inputs.cell == []:
                inputs.cell = cell
                LOG.info('Heuristic cell: %s' % cell)

            field_ids = clheuristics.field(intent, field)
            imsize = clheuristics.imsize(fields=field_ids,
              cell=inputs.cell, beam=beam)
            if inputs.imsize == []:
                inputs.imsize = imsize
                LOG.info('Heuristic imsize: %s', imsize)

        # Initialize imaging results structure
	if not inputs.result: 
            plotdir = os.path.join(inputs.context.report_dir,
                'stage%s' % inputs.context.stage.split('_')[0])
            result = CleanResult(sourcename=inputs.field,
                intent=inputs.intent, spw=inputs.spw, plotdir=plotdir)
	else:
	    result = inputs.result

	try:
	    # Compute dirty image only
            if inputs.iter == 0:
	        result = self._do_dirty_cycle(scanidlist, result)
	    # Compute nth clean image
	    else:
	        result = self._do_clean_cycle (scanidlist, result, iter=inputs.iter)
	except Exception, e:
            LOG.error('Clean error: %s' % (str(e)))
	finally:
	    pass

	return result

    def analyse(self, result):
        return result

    # Compute the clean image. Iter > 0
    def _do_clean_cycle (self, scanidlist=[], result=None, iter=1):

	inputs = self.inputs

        LOG.info('Iteration %s threshold %s' % (iter, inputs.threshold))
        LOG.info('Iteration %s niter %s' % (iter, inputs.niter))

        # Derive names of clean products for this iteration, remove
        # old clean products with the name name,
        #old_model_name = model_name
        old_model_name = result.model
        model_name = purge('%s.iter%s.model' % (inputs.imagename, iter))
        rename_image(old_name=old_model_name, new_name=model_name)
        image_name = purge(name='%s.iter%s.image' % ( inputs.imagename, iter))
        residual_name = purge(name='%s.iter%s.residual' % (
             inputs.imagename, iter))
        psf_name = purge('%s.iter%s.psf' % (inputs.imagename, iter))
        flux_name = purge('%s.iter%s.flux' % (inputs.imagename, iter))

	# Call CASA clean. Clean this up at some point.
        job = casa_tasks.clean(vis=inputs.vis, imagename='%s.iter%s' %
	    (inputs.imagename, iter), field=inputs.field, spw=inputs.spw,
	    selectdata=True, scan=scanidlist, mode=inputs.mode, niter=inputs.niter,
            threshold=inputs.threshold, imagermode=inputs.imagermode,
	    interactive=False, outframe=inputs.outframe, nchan=inputs.nchan,
            start=inputs.start, width=inputs.width, imsize=inputs.imsize,
	    cell=inputs.cell, phasecenter=inputs.phasecenter,
	    stokes=inputs.stokes,
            weighting=inputs.weighting, robust=inputs.robust,
            noise=inputs.noise, npixels=inputs.npixels,
            restoringbeam=inputs.restoringbeam, uvrange=inputs.uvrange,
            mask=inputs.mask, usescratch=True)
        self._executor.execute(job)

        # Store the  model image.
        set_miscinfo(name=model_name, spw=inputs.spw, field=inputs.field,
	    type='model', iter=iter)
        result.set_model(iter=iter, image=model_name)

	# Store the image as model. 
	# Is this necessary.
        set_miscinfo(name=image_name, spw=inputs.spw, field=inputs.field,
	    type='model', iter=iter)
        result.set_model(iter=iter, image=model_name)

	# Store the image as image.
        set_miscinfo(name=image_name, spw=inputs.spw, field=inputs.field,
	    type='image', iter=iter)
        result.set_image(iter=iter, image=image_name)

	# Store the residual.
        set_miscinfo(name=residual_name, spw=inputs.spw, field=inputs.field,
	    type='residual', iter=iter)
        result.set_residual(iter=iter, image=residual_name)

	# Make sure mask has path name
        if os.path.exists(inputs.mask):
            set_miscinfo(name=inputs.mask, spw=inputs.spw, field=inputs.field,
	        type='cleanmask', iter=iter)
        result.set_cleanmask(iter=iter, image=inputs.mask)

        return result

    # Compute the dirty image. Iter is always 0
    def _do_dirty_cycle (self, scanidlist, result, iter=0):

        inputs = self.inputs

        # Remove previous images if any.
        model_name = purge(name='%s.iter%s.model' % (inputs.imagename, iter))
        image_name = purge(name='%s.iter%s.image' % (inputs.imagename, iter))
        residual_name = purge(name='%s.iter%s.residual' %
	     (inputs.imagename, iter))
        psf_name = purge('%s.iter%s.psf' % (inputs.imagename, iter))
        flux_name = purge('%s.iter%s.flux' % (inputs.imagename, iter))

        # Occasionally an old file confuses the clean so that clean starts
	# in a corrupted state. This is not understood. For now, delete all
	# <imagename>.iter* files.
        #     Replace with os independent code at some point.
        #os.system('rm -rf %s.iter*' % inputs.imagename)
	try:
	    shutil.rmtree('%s.iter*' * inputs.imagenmae)
	except:
	    pass

	# Set up dirty image job.
        job = casa_tasks.clean(vis=inputs.vis,
            imagename='%s.iter%s' % (inputs.imagename, iter),
            field=inputs.field, spw=inputs.spw, selectdata=True, scan=scanidlist,
            mode=inputs.mode, niter=0, threshold='0.0mJy',
            imagermode=inputs.imagermode, interactive=False,
            outframe=inputs.outframe, nchan=inputs.nchan, start=inputs.start,
            width=inputs.width, imsize=inputs.imsize, cell=inputs.cell,
            phasecenter=inputs.phasecenter, weighting=inputs.weighting,
	    stokes=inputs.stokes,
            robust=inputs.robust, noise=inputs.noise,
            npixels=inputs.npixels, restoringbeam=inputs.restoringbeam,
            uvrange=inputs.uvrange, mask=None, usescratch=True)

	# Execute
        self._executor.execute(job)

        # Store the model image information.
        set_miscinfo(name=model_name, spw=inputs.spw, field=inputs.field,
            type='model', iter=iter)
        result.set_model(iter=iter, image=model_name)

	# Store the dirty image.
        set_miscinfo(name=image_name, spw=inputs.spw, field=inputs.field,
            type='image', iter=iter)
        result.set_image(iter=iter, image=image_name)

	# Store the residual image. Why do we need to keep this ?
        set_miscinfo(name=residual_name, spw=inputs.spw, field=inputs.field,
            type='residual', iter=iter)
        result.set_residual(iter=iter, image=residual_name)

	# Store the PSF image.
        set_miscinfo(name=psf_name, spw=inputs.spw, field=inputs.field,
            type='psf', iter=iter)
        result.set_psf(image=psf_name)

	# Store the flux image.
        set_miscinfo(name=flux_name, spw=inputs.spw, field=inputs.field,
            type='flux', iter=iter)
        result.set_flux(image=flux_name)

	return result

# Some utility routines.

# Remove a directory 
def purge(name):
    #os.system('rm -fr %s' % name)
    try:
        shutil.rmtree(name)
    except:
        pass
    return name

# Rename an image
def rename_image(old_name, new_name):
    with casatools.ImageReader(old_name) as image:
        image.rename(name=new_name, overwrite=True)

# Define miscellaneous image information
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

