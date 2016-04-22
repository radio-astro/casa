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
import pipeline.infrastructure.contfilehandler as contfilehandler
from pipeline.infrastructure import casa_tasks
import pipeline.domain.measures as measures

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

        # Construct regex for string matching - escape likely problem
        # chars. Simpler way to do this ?
        re_field = inputs.field.replace('*', '.*')
        re_field = re_field.replace('[', '\[')
        re_field = re_field.replace(']', '\]')
        re_field = re_field.replace('(', '\(')
        re_field = re_field.replace(')', '\)')
        re_field = re_field.replace('+', '\+')
        re_field = utils.dequote(re_field)

        # Use scanids to select data with the specified intent
        # Note CASA clean now supports intent selectin but leave
        # this logic in place and use it to eliminate vis that
        # don't contain the requested data.
        scanidlist = []
        vislist = []
        for vis in inputs.vis:
            ms = inputs.context.observing_run.get_ms(name=vis)
            scanids = [scan.id for scan in ms.scans if
                       inputs.intent in scan.intents and
                       re.search(pattern=re_field, string=str(scan.fields))]
            if not scanids:
                continue
            scanids = str(scanids)
            scanids = scanids.replace('[', '')
            scanids = scanids.replace(']', '')
            scanidlist.append(scanids)
            vislist.append(vis)
        inputs.vis=vislist

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
            LOG.error('%s/%s/spw%s clean error: %s' % (inputs.field, inputs.intent, inputs.spw, str(e)))

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

        spw_freq_param_lists = []
        spw_chan_param_lists = []
        p = re.compile('([\d.]*)(~)([\d.]*)(\D*)')
        freq_ranges = []
        num_channels = []

        # get spw info from first vis set, assume spws uniform
        # across datasets
        ms = context.observing_run.get_ms(name=inputs.vis[0])

        # Get ID of field closest to the phase center
        meTool = casatools.measures
        ref_field_ids = []

        # Phase center coordinates
        pc_direc = meTool.source(inputs.phasecenter)

        for msname in inputs.vis:
            ms_obj = context.observing_run.get_ms(msname)
            field_ids = [f.id for f in ms_obj.fields if inputs.intent in f.intents]
            separations = [meTool.separation(pc_direc, ms_obj.fields[i].mdirection)['value'] for i in field_ids]
            ref_field_ids.append(field_ids[separations.index(min(separations))])

        # Get a cont file handler for the conversion to TOPO
        contfile_handler = contfilehandler.ContFileHandler(context.contfile)

        for spwid in inputs.spw.split(','):
            spw_info = ms.get_spectral_window(spwid)
            num_channels.append(spw_info.num_channels)
            if (inputs.spwsel.has_key('spw%s' % (spwid))):
                if (inputs.spwsel['spw%s' % (spwid)] != ''):
                    freq_selection, refer = inputs.spwsel['spw%s' % (spwid)].split()
                    if (refer == 'LSRK'):
                        # Convert to TOPO
                        topo_freq_selections, topo_chan_selections = contfile_handler.lsrk_to_topo(inputs.spwsel['spw%s' % (spwid)], inputs.vis, ref_field_ids, spwid)
                        spw_freq_param_lists.append(['%s:%s' % (spwid, topo_freq_selection.split()[0]) for topo_freq_selection in topo_freq_selections])
                        spw_chan_param_lists.append(['%s:%s' % (spwid, topo_chan_selection.split()[0]) for topo_chan_selection in topo_chan_selections])
                        # Count only one selection !
                        for freq_range in topo_freq_selections[0].split(';'):
                            f1, sep, f2, unit = p.findall(freq_range)[0]
                            freq_ranges.append((float(f1), float(f2)))
                    else:
                        LOG.warning('Cannot convert frequency selection properly to TOPO. Using plain ranges for all MSs.')
                        spw_freq_param_lists.append(['%s:%s' % (spwid, freq_selection)] * len(inputs.vis))
                        for freq_range in freq_selection.split(';'):
                            f1, sep, f2, unit = p.findall(freq_range)[0]
                            freq_ranges.append((float(f1), float(f2)))
                else:
                    spw_freq_param_lists.append([spwid] * len(inputs.vis))
                    min_frequency = float(spw_info.min_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                    max_frequency = float(spw_info.max_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                    freq_ranges.append((min_frequency, max_frequency))
            else:
                spw_freq_param_lists.append([spwid] * len(inputs.vis))
                min_frequency = float(spw_info.min_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                max_frequency = float(spw_info.max_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                freq_ranges.append((min_frequency, max_frequency))

        spw_freq_param = [','.join(spwsel_per_ms) for spwsel_per_ms in [[spw_freq_param_list_per_ms[i] for spw_freq_param_list_per_ms in spw_freq_param_lists] for i in xrange(len(inputs.vis))]]
        aggregate_bw = '0.0GHz'
        for freq_range in utils.merge_ranges(freq_ranges):
            aggregate_bw = casatools.quanta.add(aggregate_bw, casatools.quanta.sub('%sGHz' % (freq_range[1]), '%sGHz' % (freq_range[0])))

        # TODO: Adjust sensitivity to selection bandwidth here ?
        #       Save channel selection in result for weblog.

        if (inputs.specmode == 'cube'):
            # Estimate memory usage and adjust chanchunks parameter to avoid
            # exceeding the available memory.
            mem_bytes = os.sysconf('SC_PAGE_SIZE') * os.sysconf('SC_PHYS_PAGES')
            mem_usable_bytes = 0.8 * mem_bytes
            if (inputs.nchan != -1):
                cube_bytes = inputs.imsize[0] * inputs.imsize[1] * inputs.nchan * 4
            else:
                cube_bytes = inputs.imsize[0] * inputs.imsize[1] * max(num_channels) * 4
            tclean_bytes = 9 * cube_bytes
            chanchunks = int(tclean_bytes / mem_usable_bytes) + 1
        else:
            chanchunks = 1

        parallel = all([mpihelpers.parse_mpi_input_parameter(inputs.parallel),
                        'TARGET' in inputs.intent])

        if (result.multiterm):
            job = casa_tasks.tclean(vis=inputs.vis, imagename='%s.%s.iter%s' %
                  (os.path.basename(inputs.imagename), inputs.stokes, iter),
                  spw=spw_freq_param,
                  intent=utils.to_CASA_intent(inputs.ms[0], inputs.intent),
                  scan=scanidlist, specmode=inputs.specmode if inputs.specmode != 'cont' else 'mfs', gridder=inputs.gridder,
                  pblimit=inputs.pblimit, niter=inputs.niter,
                  threshold=inputs.threshold, deconvolver=inputs.deconvolver,
                  interactive=0, outframe=inputs.outframe, nchan=inputs.nchan,
                  start=inputs.start, width=inputs.width, imsize=inputs.imsize,
                  cell=inputs.cell, phasecenter=inputs.phasecenter,
                  stokes=inputs.stokes,
                  weighting=inputs.weighting, robust=inputs.robust,
                  npixels=inputs.npixels,
                  restoringbeam=inputs.restoringbeam, uvrange=inputs.uvrange,
                  mask=inputs.mask, usemask='user', savemodel='none',
                  nterms=result.multiterm,
                  makeimages='choose', restoremodel=True,
                  chanchunks=chanchunks, parallel=parallel)
        else:
            job = casa_tasks.tclean(vis=inputs.vis, imagename='%s.%s.iter%s' %
                  (os.path.basename(inputs.imagename), inputs.stokes, iter),
                  spw=spw_freq_param,
                  intent=utils.to_CASA_intent(inputs.ms[0], inputs.intent),
                  scan=scanidlist, specmode=inputs.specmode if inputs.specmode != 'cont' else 'mfs', gridder=inputs.gridder,
                  pblimit=inputs.pblimit, niter=inputs.niter,
                  threshold=inputs.threshold, deconvolver=inputs.deconvolver,
                  interactive=0, outframe=inputs.outframe, nchan=inputs.nchan,
                  start=inputs.start, width=inputs.width, imsize=inputs.imsize,
                  cell=inputs.cell, phasecenter=inputs.phasecenter,
                  stokes=inputs.stokes,
                  weighting=inputs.weighting, robust=inputs.robust,
                  npixels=inputs.npixels,
                  restoringbeam=inputs.restoringbeam, uvrange=inputs.uvrange,
                  mask=inputs.mask, usemask='user', savemodel='none',
                  makeimages='choose', restoremodel=True,
                  chanchunks=chanchunks, parallel=parallel)
        tclean_result = self._executor.execute(job)

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
                          spw=spw_freq_param,
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

        # Keep threshold, sensitivity and aggregate bandwidth for QA and weblog
        result.set_threshold(inputs.threshold)
        result.set_sensitivity(inputs.sensitivity)
        result.set_aggregate_bw(aggregate_bw)

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
