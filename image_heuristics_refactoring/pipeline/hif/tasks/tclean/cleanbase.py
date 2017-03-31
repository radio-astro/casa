from __future__ import absolute_import

import os
import re
import shutil
import types
from math import sqrt

import casadef
#from recipes import makepb
from . import makepb

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks
import pipeline.domain.measures as measures

from .resultobjects import TcleanResult

LOG = infrastructure.get_logger(__name__)

# The basic clean tasks classes. Clean performs a single clean run.


class CleanBaseInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, imagename=None, datacolumn=None,
                 intent=None, field=None, spw=None, spwsel=None, uvrange=None, specmode=None,
                 gridder=None, deconvolver=None, uvtaper=None,
                 nterms=None, cycleniter=None, cyclefactor=None, scales=None,
                 outframe=None, imsize=None, cell=None,
                 phasecenter=None, nchan=None, start=None, width=None, stokes=None,
                 weighting=None,
                 robust=None, noise=None, npixels=None,
                 restoringbeam=None, iter=None, mask=None, hm_masking=None, hm_autotest=None, pblimit=None, niter=None,
                 threshold=None, sensitivity=None, result=None, parallel=None,
                 heuristics=None):
        self._init_properties(vars())
        self.heuristics = heuristics

    deconvolver = basetask.property_with_default('deconvolver', '')
    uvtaper = basetask.property_with_default('uvtaper', None)
    nterms = basetask.property_with_default('nterms', None)
    cycleniter = basetask.property_with_default('cycleniter', None)
    cyclefactor = basetask.property_with_default('cyclefactor', None)
    scales = basetask.property_with_default('scales', None)
    field = basetask.property_with_default('field', '')
    gridder = basetask.property_with_default('gridder', '')
    imagename = basetask.property_with_default('imagename', '')
    intent = basetask.property_with_default('intent', '')
    iter = basetask.property_with_default('iter', 0)
    mask = basetask.property_with_default('mask', '')
    hm_masking = basetask.property_with_default('hm_masking', 'centralregion')
    hm_autotest = basetask.property_with_default('hm_autotest', '')
    niter = basetask.property_with_default('niter', 5000)
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
    spwsel = basetask.property_with_default('spwsel', [])
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

    @property
    def datacolumn(self):
        return self._datacolumn

    @datacolumn.setter
    def datacolumn(self, value):
        if value is None:
            value = ''
        self._datacolumn = value


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

        # Set the data column
        targetmslist = [vis for vis in inputs.vis if context.observing_run.get_ms(name=vis).is_imaging_ms]
        if not inputs.datacolumn:
            if len(targetmslist) > 0:
                if inputs.specmode == 'cube':
                    inputs.datacolumn = 'corrected'
                else:
                    inputs.datacolumn = 'data'
            else:
                inputs.datacolumn = 'corrected'

        # Remove MSs that do not contain data for the given field(s)
        scanidlist, visindexlist = inputs.heuristics.get_scanidlist(inputs.vis, inputs.field, inputs.intent)
        inputs.vis = [inputs.vis[i] for i in visindexlist]
        inputs.spwsel = [inputs.spwsel[i] for i in visindexlist]

        # Initialize imaging results structure
        if not inputs.result:
            plotdir = os.path.join(inputs.context.report_dir,
                                   'stage%s' % inputs.context.stage.split('_')[0])
            result = TcleanResult(sourcename=inputs.field,
                                  intent=inputs.intent,
                                  spw=inputs.spw,
                                  specmode=inputs.specmode,
                                  multiterm=inputs.nterms if inputs.deconvolver=='mtmfs' else None,
                                  plotdir=plotdir)
        else:
            result = inputs.result

        try:
            result = self._do_clean_cycle (scanidlist, result, iter=inputs.iter)
        except Exception, e:
            LOG.error('%s/%s/spw%s clean error: %s' % (inputs.field, inputs.intent, inputs.spw, str(e)))
            result.error = '%s/%s/spw%s clean error: %s' % (inputs.field, inputs.intent, inputs.spw, str(e))

        return result

    def analyse(self, result):
        return result

    def _do_clean_cycle (self, scanidlist=None, result=None, iter=1):
        """
        Compute a clean image.
        """
        if scanidlist is None:
            scanidlist = []

        context = self.inputs.context
        inputs = self.inputs

        # Derive names of clean products for this iteration
        old_model_name = result.model
        model_name = '%s.%s.iter%s.model' % (inputs.imagename, inputs.stokes, iter)
        if old_model_name is not None:
            if os.path.exists(old_model_name):
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

        #if (inputs.specmode == 'cube'):
            # Estimate memory usage and adjust chanchunks parameter to avoid
            # exceeding the available memory.
        #    mem_bytes = os.sysconf('SC_PAGE_SIZE') * os.sysconf('SC_PHYS_PAGES')
        #    mem_usable_bytes = 0.8 * mem_bytes
        #    if (inputs.nchan != -1):
        #        cube_bytes = inputs.imsize[0] * inputs.imsize[1] * inputs.nchan * 4
        #    else:
        #        ms = context.observing_run.get_ms(name=inputs.vis[0])
        #        spw_info = ms.get_spectral_window(spwid)
        #        cube_bytes = inputs.imsize[0] * inputs.imsize[1] * spw_info.num_channels * 4
        #    tclean_bytes = 9 * cube_bytes
        #    chanchunks = int(tclean_bytes / mem_usable_bytes) + 1
        #else:
        #    chanchunks = 1

        # Starting with CASA 4.7.79 tclean can calculate chanchunks automatically.
        chanchunks = -1

        parallel = all([mpihelpers.parse_mpi_input_parameter(inputs.parallel),
                        'TARGET' in inputs.intent])

        # Common tclean parameters
        tclean_job_parameters = {
            'vis':           inputs.vis,
            'imagename':     '%s.%s.iter%s' % (os.path.basename(inputs.imagename), inputs.stokes, iter),
            'datacolumn':    inputs.datacolumn,
            'field':         inputs.field,
            'spw':           inputs.spwsel,
            'intent':        utils.to_CASA_intent(inputs.ms[0], inputs.intent),
            'specmode':      inputs.specmode if inputs.specmode != 'cont' else 'mfs',
            'gridder':       inputs.gridder,
            'pblimit':       inputs.pblimit,
            'niter':         inputs.niter,
            'threshold':     inputs.threshold,
            'deconvolver':   inputs.deconvolver,
            'interactive':   0,
            'outframe':      inputs.outframe,
            'nchan':         inputs.nchan,
            'start':         inputs.start,
            'width':         inputs.width,
            'imsize':        inputs.imsize,
            'cell':          inputs.cell,
            'phasecenter':   inputs.phasecenter,
            'stokes':        inputs.stokes,
            'weighting':     inputs.weighting,
            'robust':        inputs.robust,
            'npixels':       inputs.npixels,
            'restoringbeam': inputs.restoringbeam,
            'uvrange':       inputs.uvrange,
            'savemodel':     'none',
            'chanchunks':    chanchunks,
            'parallel':      parallel
            }

        if scanidlist not in [[], None]:
            tclean_job_parameters['scan'] = scanidlist

        # Set up masking parameters
        if inputs.hm_masking == 'auto':
            tclean_job_parameters['usemask'] = 'auto-multithresh'
            # Defaults for ALMA 7m / 12m
            if 'ALMA' in context.project_summary.observatory:
                min_diameter = 1.e9
                for msname in inputs.vis:
                    min_diameter = min(min_diameter, min([antenna.diameter for antenna in context.observing_run.get_ms(msname).antennas]))
                if min_diameter == 7.0:
                    tclean_job_parameters['sidelobethreshold'] = 2.0
                    tclean_job_parameters['noisethreshold'] = 3.0
                    tclean_job_parameters['lownoisethreshold'] = 2.5
                    tclean_job_parameters['minbeamfrac'] = 0.2
                elif min_diameter == 12.0:
                    tclean_job_parameters['sidelobethreshold'] = 3.0
                    tclean_job_parameters['noisethreshold'] = 5.0
                    tclean_job_parameters['lownoisethreshold'] = 1.5
                    tclean_job_parameters['minbeamfrac'] = 0.3
            if inputs.hm_autotest != '':
                hm_autotest_params = dict((key.strip(), float(value)) for key, value in [kvpair.split(':') for kvpair in inputs.hm_autotest.split(',')])
                for key in hm_autotest_params.iterkeys():
                    if key in ['sidelobethreshold', 'noisethreshold', 'lownoisethreshold', 'minbeamfrac']:
                        tclean_job_parameters[key] = hm_autotest_params[key]
        else:
            tclean_job_parameters['usemask'] = 'user'
            tclean_job_parameters['mask'] = inputs.mask

        # Show nterms parameter only if it is used.
        if (result.multiterm):
            tclean_job_parameters['nterms'] = result.multiterm

        # Select whether to restore image
        if (inputs.niter == 0):
            tclean_job_parameters['restoration'] = False
            tclean_job_parameters['pbcor'] = False
        else:
            tclean_job_parameters['restoration'] = True
            tclean_job_parameters['pbcor'] = True

        # Re-use products from previous iteration.
        if (iter > 0):
            tclean_job_parameters['restart'] = True
            tclean_job_parameters['calcpsf'] = False
            tclean_job_parameters['calcres'] = False

        # Additional heuristics or task parameters
        if inputs.cyclefactor:
            tclean_job_parameters['cyclefactor'] = inputs.cyclefactor
        else:
            # Call first and assign to variable to avoid calling slow methods twice
            cyclefactor = inputs.heuristics.cyclefactor()
            if cyclefactor:
                tclean_job_parameters['cyclefactor'] = cyclefactor

        if inputs.cycleniter:
            tclean_job_parameters['cycleniter'] = inputs.cycleniter
        else:
            cycleniter = inputs.heuristics.cycleniter()
            if cycleniter:
                tclean_job_parameters['cycleniter'] = cycleniter

        if inputs.scales:
            tclean_job_parameters['scales'] = inputs.scales
        else:
            scales = inputs.heuristics.scales()
            if scales:
                tclean_job_parameters['scales'] = scales

        if inputs.uvrange:
            tclean_job_parameters['uvrange'] = inputs.uvrange
        else:
            uvrange = inputs.heuristics.uvrange()
            if uvrange:
                tclean_job_parameters['uvrange'] = uvrange

        if inputs.uvtaper:
            tclean_job_parameters['uvtaper'] = inputs.uvtaper
        else:
            uvtaper = inputs.heuristics.uvtaper()
            if uvtaper:
                tclean_job_parameters['uvtaper'] = uvtaper

        job = casa_tasks.tclean(**tclean_job_parameters)
        tclean_result = self._executor.execute(job)

        pbcor_image_name = '%s.%s.iter%s.image.pbcor' % (inputs.imagename, inputs.stokes, iter)

        if (inputs.niter > 0):
            if tclean_result.has_key('stopcode'):
                # Serial tclean result
                tclean_stopcode = tclean_result['stopcode']
                tclean_iterdone = tclean_result['iterdone']
                tclean_niter = tclean_result['niter']
            else:
                # Parallel tclean result structure is currently (2017-03) different
                tclean_stopcode = max([tclean_result[key][int(key.replace('node',''))]['stopcode'] for key in tclean_result.keys()])
                tclean_iterdone = sum([tclean_result[key][int(key.replace('node',''))]['iterdone'] for key in tclean_result.keys()])
                tclean_niter = max([tclean_result[key][int(key.replace('node',''))]['niter'] for key in tclean_result.keys()])

            LOG.info('tclean used %d iterations' % (tclean_iterdone))
            if ((tclean_stopcode == 1) and (tclean_iterdone >= tclean_niter)):
                result.error = CleanBaseError('tclean reached niter limit. Field: %s SPW: %s' % (inputs.field, inputs.spw), 'Reached niter limit')
                LOG.warning('tclean reached niter limit of %d for %s / spw%s !' % (tclean_niter, utils.dequote(inputs.field), inputs.spw))

            if (tclean_stopcode == 5):
                result.error = CleanBaseError('tclean stopped to prevent divergence. Field: %s SPW: %s' % (inputs.field, inputs.spw), 'tclean stopped to prevent divergence.')
                LOG.warning('tclean stopped to prevent divergence. Field: %s SPW: %s' % (inputs.field, inputs.spw))

        if (iter > 0):
            # Store the model.
            set_miscinfo(name=model_name, spw=inputs.spw, field=inputs.field,
                         type='model', iter=iter, multiterm=result.multiterm)
            result.set_model(iter=iter, image=model_name)

            # Always set info on the uncorrected image for plotting
            set_miscinfo(name=image_name, spw=inputs.spw, field=inputs.field,
                         type='image', iter=iter, multiterm=result.multiterm)

            # Store the PB corrected image.
            set_miscinfo(name=pbcor_image_name, spw=inputs.spw, field=inputs.field,
                         type='pbcorimage', iter=iter, multiterm=result.multiterm)
            result.set_image(iter=iter, image=pbcor_image_name)

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
                     type='flux', iter=iter, multiterm=result.multiterm)
        result.set_flux(image=flux_name)

        # Make sure mask has path name
        if os.path.exists(inputs.mask):
            set_miscinfo(name=inputs.mask, spw=inputs.spw, field=inputs.field,
                         type='cleanmask', iter=iter)
        result.set_cleanmask(iter=iter, image=inputs.mask)

        # Keep threshold and sensitivity for QA and weblog
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
            if (name.find('.image.pbcor') != -1):
                imagename = name.replace('.image.pbcor', '.image.tt0.pbcor')
            else:
                imagename = '%s.tt0' % (name)
        else:
            imagename = name
        with casatools.ImageReader(imagename) as image:
            info = image.miscinfo()
            if spw:
                info['spw'] = spw
            if field:
                # TODO: Find common key calculation. Long VLASS lists cause trouble downstream.
                #       Truncated list may cause duplicates.
                info['field'] = field.split(',')[0]
            if type:
                info['type'] = type
            if iter is not None:
                info['iter'] = iter
            image.setmiscinfo(info)


class CleanBaseError(object):

    '''Clean Base Error Class to transfer detailed messages for weblog
       reporting.'''

    def __init__(self, longmsg='', shortmsg=''):
        self.longmsg = longmsg
        self.shortmsg= shortmsg
