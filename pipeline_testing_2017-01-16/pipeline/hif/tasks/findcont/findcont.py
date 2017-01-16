from __future__ import absolute_import
import os
import re
import types
import copy

from pipeline.hif.heuristics import findcont
from pipeline.hif.heuristics import imageparams
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.contfilehandler as contfilehandler
import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks
from .resultobjects import FindContResult

LOG = infrastructure.get_logger(__name__)


class FindContInputs(basetask.StandardInputs):
    parallel = basetask.property_with_default('parallel', 'automatic')

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, target_list=None,
                 parallel=None):
        self._init_properties(vars())

    @property
    def target_list(self):
        return self._target_list

    @target_list.setter
    def target_list(self, value):
        if not value:
            value = copy.deepcopy(self.context.clean_list_pending)
        self._target_list = value

# tell the infrastructure to give us mstransformed data when possible by
# registering our preference for imaging measurement sets
basetask.ImagingMeasurementSetsPreferred.register(FindContInputs)


class FindCont(basetask.StandardTaskTemplate):
    Inputs = FindContInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs
        context = self.inputs.context

        qaTool = casatools.quanta

        # make sure inputs.vis is a list, even it is one that contains a
        # single measurement set
        if type(inputs.vis) is not types.ListType:
            inputs.vis = [inputs.vis]
        if any((ms.is_imaging_ms for ms in context.observing_run.get_measurement_sets(inputs.vis))):
            datacolumn = 'data'
        else:
            datacolumn = 'corrected'

        findcont_heuristics = findcont.FindContHeuristics(context)

        contfile_handler = contfilehandler.ContFileHandler(context.contfile)
        cont_ranges = contfile_handler.read()

        result_cont_ranges = {}
        num_found = 0
        num_total = 0
        for i, target in enumerate(inputs.target_list):
            for spwid in target['spw'].split(','):
                source_name = utils.dequote(target['field'])

                # get continuum ranges dict for this source, also setting it if accessed for first time
                source_continuum_ranges = result_cont_ranges.setdefault(source_name, {})

                # get continuum ranges list for this source and spw, also setting them if accessed for first time
                cont_ranges_source_spw = cont_ranges['fields'].setdefault(source_name, {}).setdefault(spwid, [])

                if len(cont_ranges_source_spw) > 0:
                    LOG.info('Using existing selection {!r} for field {!s}, spw {!s}'.format(cont_ranges_source_spw, source_name, spwid))
                    source_continuum_ranges[spwid] = {
                        'cont_ranges': cont_ranges_source_spw,
                        'plotfile': 'none',
                        'status': 'OLD'
                    }
                    num_found += 1

                else:
                    LOG.info('Determining continuum ranges for field %s, spw %s' % (source_name, spwid))

                    findcont_basename = '%s.I.findcont' % (os.path.basename(target['imagename']).replace('spw%s' % (target['spw'].replace(',','_')), 'spw%s' % (spwid)).replace('STAGENUMBER', str(context.stage)))

                    # determine the gridder mode here (temporarily ...)
                    image_heuristics = imageparams.ImageParamsHeuristics(context=context,
                                                                   vislist=inputs.vis,
                                                                   spw=spwid,
                                                                   contfile=context.contfile,
                                                                   linesfile=context.linesfile)
                    gridder = image_heuristics.gridder(target['intent'], target['field'])

                    # need scan id list for multiple target case
                    # TODO: move this to a heuristics to avoid duplicated code (see tclean)

                    # Construct regex for string matching - escape likely problem
                    # chars. Simpler way to do this ?
                    re_field = target['field'].replace('*', '.*')
                    re_field = re_field.replace('[', '\[')
                    re_field = re_field.replace(']', '\]')
                    re_field = re_field.replace('(', '\(')
                    re_field = re_field.replace(')', '\)')
                    re_field = re_field.replace('+', '\+')
                    re_field = utils.dequote(re_field)

                    # Use scanids to select data with the specified intent
                    # Not CASA clean now supports intent selectin but leave
                    # this logic in place and use it to eliminate vis that
                    # don't contain the requested data.
                    scanidlist = []
                    for vis in inputs.vis:
                        ms = inputs.context.observing_run.get_ms(name=vis)
                        scanids = [scan.id for scan in ms.scans if
                                   target['intent'] in scan.intents and
                                   re.search(pattern=re_field, string=str(scan.fields))]
                        if not scanids:
                            LOG.warning('No data for Field %s SPW %s' % (target['field'], spwid))
                            continue
                        scanids = str(scanids)
                        scanids = scanids.replace('[', '')
                        scanids = scanids.replace(']', '')
                        scanidlist.append(scanids)

                    # To avoid noisy edge channels, use only the LSRK frequency
                    # intersection and skip one channel on either end.
                    # Use only the current spw ID here !
                    if0, if1, channel_width = image_heuristics.lsrk_freq_intersection(inputs.vis, target['field'], spwid)
                    if (if0 == -1) or (if1 == -1):
                        LOG.error('No LSRK frequency intersect among selected MSs for Field %s SPW %s' % (target['field'], spwid))
                        cont_ranges['fields'][source_name][spwid] = ['NONE']
                        result_cont_ranges[source_name][spwid] = {'cont_ranges': ['NONE'], 'plotfile': 'none', 'status': 'NEW'}
                        continue

                    # Check for manually supplied values
                    if0_auto = if0
                    if1_auto = if1
                    channel_width_auto = channel_width

                    if target['start'] != '':
                        if0 = qaTool.convert(target['start'], 'Hz')['value']
                        if if0 < if0_auto:
                            LOG.error('Supplied start frequency %s < f_low_native for Field %s SPW %s' % (target['start'], target['field'], target['spw']))
                            continue
                        LOG.info('Using supplied start frequency %s' % (target['start']))

                    if target['width'] != '' and target['nbin'] != -1:
                        LOG.error('Field %s SPW %s: width and nbin are mutually exclusive' % (target['field'], target['spw']))
                        continue

                    if target['width'] != '':
                        channel_width_manual = qaTool.convert(target['width'], 'Hz')['value']
                        if channel_width_manual < channel_width_auto:
                            LOG.error('User supplied channel width smaller than native value of %s GHz for Field %s SPW %s' % (channel_width_auto, target['field'], target['spw']))
                            continue
                        LOG.info('Using supplied width %s' % (target['width']))
                        channel_width = channel_width_manual
                        if channel_width > channel_width_auto:
                            target['nbin'] = int(round(channel_width / channel_width_auto) + 0.5)
                    elif target['nbin'] != -1:
                        LOG.info('Applying binning factor %d' % (target['nbin']))
                        channel_width *= target['nbin']

                    if target['nchan'] != -1:
                        if1 = if0 + channel_width * target['nchan']
                        if if1 > if1_auto:
                            LOG.error('Calculated stop frequency %s GHz > f_high_native for Field %s SPW %s' % (if1, target['field'], target['spw']))
                            continue
                        LOG.info('Using supplied nchan %d' % (target['nchan']))

                    # tclean interprets the start frequency as the center of the
                    # first channel. We have, however, an edge to edge range.
                    # Thus shift by 0.5 channels if no start is supplied.
                    if target['start'] == '':
                        start = '%sGHz' % ((if0 + 1.5 * channel_width) / 1e9)
                    else:
                        start = target['start']

                    width = '%sMHz' % ((channel_width) / 1e6)

                    # Skip edge channels if no nchan is supplied
                    if target['nchan'] == -1:
                        nchan = int(round((if1 - if0 ) / channel_width - 2))
                    else:
                        nchan = target['nchan']

                    # Starting with CASA 4.7.79 tclean can calculate chanchunks automatically.
                    chanchunks = -1

                    parallel = mpihelpers.parse_mpi_input_parameter(inputs.parallel)

                    # Need to make an LSRK cube to get the real ranges in the source
                    # frame. The LSRK ranges will need to be translated to the
                    # individual TOPO ranges for the involved MSs.
                    job = casa_tasks.tclean(vis=inputs.vis, imagename=findcont_basename,
                        datacolumn=datacolumn,
                        spw=spwid, intent=utils.to_CASA_intent(inputs.ms[0], target['intent']),
                        start=start, width=width, nchan=nchan, outframe='LSRK',
                        scan=scanidlist, specmode='cube', gridder=gridder, pblimit=0.2,
                        niter=0, threshold='0mJy', deconvolver='hogbom',
                        interactive=False, imsize=target['imsize'],
                        cell=target['cell'], phasecenter=target['phasecenter'],
                        stokes='I', weighting='briggs', robust=0.5,
                        npixels=0, restoration=False, restoringbeam=[], pbcor=False,
                        savemodel='none', chanchunks=chanchunks, parallel=parallel)
                    self._executor.execute(job)

                    # Try detecting continuum frequency ranges
                    cont_ranges['fields'][source_name][spwid], png = findcont_heuristics.find_continuum('%s.residual' % (findcont_basename))

                    source_continuum_ranges[spwid] = {
                        'cont_ranges': cont_ranges['fields'][source_name][spwid],
                        'plotfile': png,
                        'status': 'NEW'
                    }

                    if cont_ranges['fields'][source_name][spwid] not in [['NONE'], ['']]:
                        num_found += 1

                num_total += 1

        result = FindContResult(result_cont_ranges, cont_ranges, num_found, num_total)

        return result

    def analyse(self, result):
        return result
