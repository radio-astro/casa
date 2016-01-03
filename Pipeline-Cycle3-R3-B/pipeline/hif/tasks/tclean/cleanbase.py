from __future__ import absolute_import

import os
import re
import shutil
import types

import casadef
#from recipes import makepb
from . import makepb

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks

from .resultobjects import TcleanResult
from pipeline.hif.heuristics import makeimlist

LOG = infrastructure.get_logger(__name__)

# The basic clean tasks classes. Clean performs a single clean run.


class CleanBaseInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, imagename=None,
                 intent=None, field=None, spw=None, spwsel=None, uvrange=None, specmode=None,
                 gridder=None, deconvolver=None, outframe=None, imsize=None, cell=None,
                 phasecenter=None, nchan=None, start=None, width=None, stokes=None,
                 weighting=None, robust=None, noise=None, npixels=None,
                 restoringbeam=None, iter=None, mask=None, pblimit=None, niter=None,
                 threshold=None, sensitivity=None, result=None, parallel=None):
        self._init_properties(vars())

    deconvolver = basetask.property_with_default('deconvolver', '')
    field = basetask.property_with_default('field', '')
    gridder = basetask.property_with_default('gridder', '')
    imagename = basetask.property_with_default('imagename', '')
    intent = basetask.property_with_default('intent', '')
    iter = basetask.property_with_default('iter', 0)
    mask = basetask.property_with_default('mask', '')
    niter = basetask.property_with_default('niter', 1000)
    noise = basetask.property_with_default('noise', '1.0Jy')
    nchan = basetask.property_with_default('nchan', -1)
    npixels = basetask.property_with_default('npixels', 0)
    outframe = basetask.property_with_default('outframe', 'LSRK')
    parallel = basetask.property_with_default('parallel', 'automatic')
    phasecenter = basetask.property_with_default('phasecenter', '')
    pblimit = basetask.property_with_default('pblimit', 0.2)
    restoringbeam = basetask.property_with_default('restoringbeam', 'common')
    robust = basetask.property_with_default('robust', -999.0)
    sensitivity = basetask.property_with_default('sensitivity', 0.0)
    spwsel = basetask.property_with_default('spwsel', {})
    start = basetask.property_with_default('start', '')
    stokes = basetask.property_with_default('stokes', 'I')
    threshold = basetask.property_with_default('threshold', '0.0mJy')
    uvrange = basetask.property_with_default('uvrange', '')
    weighting = basetask.property_with_default('weighting', 'briggs')
    width = basetask.property_with_default('width', '')

    @property
    def spw(self):
        return self._spw

    @spw.setter
    def spw(self, value):
        if value is None:
            mslist = self.context.observing_run.measurement_sets
            spws = mslist[0].get_spectral_windows()
            spwids = [spw.id for spw in spws]
            value = ','.join(spwids)
        self._spw = value

    @property
    def specmode(self):
        return self._specmode

    @specmode.setter
    def specmode(self, value):
        if value is None:
            if 'TARGET' in self.intent:
                value = 'cube'
            else:
                value = 'mfs'
        self._specmode = value

    @property
    def imsize(self):
        if self._imsize is None:
            return []
        elif type(self._imsize) is types.StringType:
            if self._imsize[0] == '[':
                temp = self._imsize.translate(None, '[]\'')
            temp = temp.split(',')
            self._imsize = map(int, temp)

        return self._imsize

    @imsize.setter
    def imsize(self, value):
        self._imsize = value

    @property
    def cell(self):
        return self._cell

    @cell.setter
    def cell(self, value):
        if value is None:
            value = []
        self._cell = value


class CleanBase(basetask.StandardTaskTemplate):
    Inputs = CleanBaseInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        context = self.inputs.context
        inputs = self.inputs

        # Make sure inputs.vis is a list, even it is one that contains a
        # single measurement set
        if type(inputs.vis) is not types.ListType:
            inputs.vis = [inputs.vis]

        # Instantiate the clean list heuristics class
        clheuristics = makeimlist.MakeImListHeuristics(context=inputs.context,
                                                       vislist=inputs.vis,
                                                       spw=inputs.spw,
                                                       contfile=context.contfile,
                                                       linesfile=context.linesfile)

        # Generate the image name if one is not supplied.
        if inputs.imagename == '':
            inputs.imagename = clheuristics.imagename(intent=inputs.intent,
                                                      field=inputs.field,
                                                      spwspec=inputs.spw)

        # Determine the default gridder
        if inputs.gridder == '':
            inputs.gridder = clheuristics.gridder(inputs.intent, inputs.field)

        # Determine the default deconvolver
        if inputs.deconvolver == '':
            inputs.deconvolver = clheuristics.deconvolver(inputs.intent,
                                                          inputs.field)

        # Determine the phase center.
        if inputs.phasecenter == '':
            field_id = clheuristics.field(inputs.intent, inputs.field)
            inputs.phasecenter = clheuristics.phasecenter(field_id)

        # Get short cut specs of field to be cleaned. Are these really needed?
        intent = inputs.intent
        field = inputs.field
        spw = inputs.spw

        # Adjust the width to get around problems with increasing / decreasing
        # frequency with channel issues.
        if inputs.width == '':
            if inputs.specmode == 'cube':
                width = clheuristics.width(int(spw.split(',')[0]))
                #width = inputs.width
            else:
                width = inputs.width
        else:
            width = inputs.width
        inputs.width = width

        if inputs.stokes == 'Q':
            ncorr = clheuristics.ncorr(int(spw.split(',')[0]))
            if ncorr <= 1:
                LOG.warning('%s/%s/spw%s Q noise estimate invalid ncorrelation'
                            ' is %s' %(field, intent, spw, ncorr))

        # Construct regex for string matching - escape likely problem
        # chars. Simpler way to do this ?
        re_field = field.replace('*', '.*')
        re_field = re_field.replace('[', '\[')
        re_field = re_field.replace(']', '\]')
        re_field = re_field.replace('(', '\(')
        re_field = re_field.replace(')', '\)')
        re_field = re_field.replace('+', '\+')

        # Use scanids to select data with the specified intent
        # Not CASA clean now supports intent selectin but leave
        # this logic in place and use it to eliminate vis that
        # don't contain the requested data.
        scanidlist = []
        vislist = []
        for vis in inputs.vis:
            ms = inputs.context.observing_run.get_ms(name=vis)
            scanids = [scan.id for scan in ms.scans if
                       intent in scan.intents and
                       re.search(pattern=re_field, string=str(scan.fields))]
            if not scanids:
                continue
            scanids = str(scanids)
            scanids = scanids.replace('[', '')
            scanids = scanids.replace(']', '')
            scanidlist.append(scanids)
            vislist.append(vis)
        inputs.vis=vislist

        # If imsize not set then use heuristic code to calculate the
        # centers for each field  / spw
        imsize = inputs.imsize
        cell = inputs.cell
        if imsize == [] or cell == []:

            # The heuristics cell size  is always the same for x and y as
            # the value derives from a single value returned by imager.advise
            cell, beam = clheuristics.cell(field_intent_list=[(field, intent)],
                                           spwspec=spw)
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
            result = TcleanResult(sourcename=inputs.field,
                                  intent=inputs.intent,
                                  spw=inputs.spw,
                                  specmode=inputs.specmode,
                                  multiterm=2 if inputs.deconvolver=='mtmfs' else None,
                                  plotdir=plotdir)
        else:
            result = inputs.result

        try:
            result = self._do_clean_cycle (scanidlist, result, iter=inputs.iter)
        except Exception, e:
            LOG.error('%s/%s/spw%s clean error: %s' % (field, intent, spw, str(e)))

        return result

    def analyse(self, result):
        return result

    def _do_clean_cycle (self, scanidlist=None, result=None, iter=1):
        """
        Compute a clean image.
        """
        if scanidlist is None:
            scanidlist = []

        inputs = self.inputs

        #        LOG.info('Stokes %s' % (inputs.stokes))
        #        LOG.info('Iteration %s threshold %s niter %s' % (iter,
        #          inputs.threshold, inputs.niter))

        # Derive names of clean products for this iteration, remove
        # old clean products with the name name,
        old_model_name = result.model
        model_name = '%s.%s.iter%s.model' % (inputs.imagename, inputs.stokes, iter)
        if (result.multiterm):
            rename_image(old_name=old_model_name, new_name=model_name, extensions=['.tt%d' % (nterm) for nterm in xrange(result.multiterm)])
        else:
            rename_image(old_name=old_model_name, new_name=model_name)
        if (inputs.niter == 0):
            image_name = ''
        else:
            image_name = '%s.%s.iter%s.image' % (
                inputs.imagename, inputs.stokes, iter)
        residual_name = '%s.%s.iter%s.residual' % (
            inputs.imagename, inputs.stokes, iter)
        psf_name = '%s.%s.iter%s.psf' % (
            inputs.imagename, inputs.stokes, iter)
        flux_name = '%s.%s.iter%s.pb' % (
            inputs.imagename, inputs.stokes, iter)

        # delete any old files with this naming root
        try:
            shutil.rmtree('%s.%s.iter%s*' % (inputs.imagename,
                                             inputs.stokes, iter))
        except:
            pass

        spw_param_list = []
        for spwid in inputs.spw.split(','):
            if (inputs.spwsel.has_key('spw%s' % (spwid))):
                spw_param_list.append('%s%s%s' % (spwid, ':'*(1 if inputs.spwsel['spw%s' % (spwid)] != '' else 0), inputs.spwsel['spw%s' % (spwid)]))
            else:
                spw_param_list.append(spwid)
        spw_param = ','.join(spw_param_list)

        parallel = all([mpihelpers.parse_mpi_input_parameter(inputs.parallel),
                        'TARGET' in inputs.intent])

        if (result.multiterm):
            job = casa_tasks.tclean(vis=inputs.vis, imagename='%s.%s.iter%s' %
                  (os.path.basename(inputs.imagename), inputs.stokes, iter),
                  spw=spw_param,
                  intent=utils.to_CASA_intent(inputs.ms[0], inputs.intent),
                  scan=scanidlist, specmode=inputs.specmode if inputs.specmode != 'cont' else 'mfs', gridder=inputs.gridder,
                  pblimit=inputs.pblimit, niter=inputs.niter,
                  threshold=inputs.threshold, deconvolver=inputs.deconvolver,
                  interactive=False, outframe=inputs.outframe, nchan=inputs.nchan,
                  start=inputs.start, width=inputs.width, imsize=inputs.imsize,
                  cell=inputs.cell, phasecenter=inputs.phasecenter,
                  stokes=inputs.stokes,
                  weighting=inputs.weighting, robust=inputs.robust,
                  npixels=inputs.npixels,
                  restoringbeam=inputs.restoringbeam, uvrange=inputs.uvrange,
                  mask=inputs.mask, savemodel='none', nterms=result.multiterm,
                  parallel=parallel)
        else:
            job = casa_tasks.tclean(vis=inputs.vis, imagename='%s.%s.iter%s' %
                  (os.path.basename(inputs.imagename), inputs.stokes, iter),
                  spw=spw_param,
                  intent=utils.to_CASA_intent(inputs.ms[0], inputs.intent),
                  scan=scanidlist, specmode=inputs.specmode if inputs.specmode != 'cont' else 'mfs', gridder=inputs.gridder,
                  pblimit=inputs.pblimit, niter=inputs.niter,
                  threshold=inputs.threshold, deconvolver=inputs.deconvolver,
                  interactive=False, outframe=inputs.outframe, nchan=inputs.nchan,
                  start=inputs.start, width=inputs.width, imsize=inputs.imsize,
                  cell=inputs.cell, phasecenter=inputs.phasecenter,
                  stokes=inputs.stokes,
                  weighting=inputs.weighting, robust=inputs.robust,
                  npixels=inputs.npixels,
                  restoringbeam=inputs.restoringbeam, uvrange=inputs.uvrange,
                  mask=inputs.mask, savemodel='none',
                  parallel=parallel)
        self._executor.execute(job)

        # Create PB for single fields since it is not auto-generated for
        # gridder='standard'.
        if (inputs.gridder == 'standard'):
            # TODO: Change to use list of MSs when makePB supports this.
            if (inputs.specmode == 'cube'):
                mode = 'frequency'
            else:
                mode = 'mfs'
            makepb.makePB(vis=inputs.vis[0],
                          field=inputs.field,
                          intent=utils.to_CASA_intent(inputs.ms[0], inputs.intent),
                          spw=spw_param,
                          scan=scanidlist,
                          mode=mode,
                          imtemplate='%s.%s.iter%s.residual%s' % (os.path.basename(inputs.imagename), inputs.stokes, iter, '.tt0' if result.multiterm else ''),
                          outimage='%s.%s.iter%s.pb' % (os.path.basename(inputs.imagename), inputs.stokes, iter),
                          pblimit = inputs.pblimit)

        # Correct images for primary beam
        pb_corrected = False
        if ((image_name not in (None, '')) and (flux_name not in (None, '')) and (inputs.mask not in (None, ''))):
            if (os.path.exists(flux_name)):
                LOG.info('Applying PB correction')
                pb_corrected = True
                pbcor_image_name = '%s.%s.iter%s.pbcor.image' % (inputs.imagename, inputs.stokes, iter)
                if (result.multiterm):
                    for nterm in xrange(result.multiterm):
                        job = casa_tasks.impbcor(
                                  imagename='%s.tt%d' % (image_name, nterm),
                                  pbimage=flux_name,
                                  outfile='%s.tt%d' % (pbcor_image_name, nterm))
                        self._executor.execute(job)
                else:
                    job = casa_tasks.impbcor(
                              imagename=image_name,
                              pbimage=flux_name,
                              outfile=pbcor_image_name)
                    self._executor.execute(job)

        if ((image_name not in (None, '')) and (not pb_corrected)):
            if (flux_name in (None, '')):
                LOG.warning('Image %s could not be PB corrected due to missing PB !' % (image_name))
            else:
                LOG.warning('Image %s could not be PB corrected !')
 
        # Store the model.
        set_miscinfo(name=model_name, spw=inputs.spw, field=inputs.field,
                     type='model', iter=iter, multiterm=result.multiterm)
        result.set_model(iter=iter, image=model_name)

        # Store the image.
        if (pb_corrected):
            set_miscinfo(name=pbcor_image_name, spw=inputs.spw, field=inputs.field,
                         type='image', iter=iter, multiterm=result.multiterm)
            result.set_image(iter=iter, image=pbcor_image_name)
        else:
            set_miscinfo(name=image_name, spw=inputs.spw, field=inputs.field,
                         type='image', iter=iter, multiterm=result.multiterm)
            result.set_image(iter=iter, image=image_name)

        # Store the residual.
        set_miscinfo(name=residual_name, spw=inputs.spw, field=inputs.field,
                     type='residual', iter=iter, multiterm=result.multiterm)
        result.set_residual(iter=iter, image=residual_name)

        # Store the PSF.
        set_miscinfo(name=psf_name, spw=inputs.spw, field=inputs.field,
                     type='psf', iter=iter, multiterm=result.multiterm)
        result.set_psf(image=psf_name)

        # Store the flux image.
        set_miscinfo(name=flux_name, spw=inputs.spw, field=inputs.field,
                     type='flux', iter=iter)
        result.set_flux(image=flux_name)

        # Make sure mask has path name
        if os.path.exists(inputs.mask):
            set_miscinfo(name=inputs.mask, spw=inputs.spw, field=inputs.field,
                         type='cleanmask', iter=iter)
        result.set_cleanmask(iter=iter, image=inputs.mask)

        # Keep threshold and sensitivity for QA
        result.set_threshold(inputs.threshold)
        result.set_sensitivity(inputs.sensitivity)

        return result

def rename_image(old_name, new_name, extensions=['']):
    """
    Rename an image
    """
    if old_name is not None:
        for extension in extensions:
            with casatools.ImageReader('%s%s' % (old_name, extension)) as image:
                image.rename(name=new_name, overwrite=True)

def set_miscinfo(name, spw=None, field=None, type=None, iter=None, multiterm=None):
    """
    Define miscellaneous image information
    """
    if name != '':
        if (multiterm):
            extension = '.tt0'
        else:
            extension = ''
        with casatools.ImageReader(name+extension) as image:
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
