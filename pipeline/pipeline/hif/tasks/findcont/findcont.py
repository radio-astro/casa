from __future__ import absolute_import
import os
import re
import subprocess
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.contfilehandler as contfilehandler
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.mpihelpers as mpihelpers
from pipeline.infrastructure import casa_tasks
from .resultobjects import FindContResult
from pipeline.hif.heuristics import makeimlist
from pipeline.hif.heuristics import findcont

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
            value = self.context.clean_list_pending
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

        # make sure inputs.vis is a list, even it is one that contains a
        # single measurement set
        if type(inputs.vis) is not types.ListType:
            inputs.vis = [inputs.vis]
        targetmslist = [vis for vis in inputs.vis if context.observing_run.get_ms(name=vis).is_imaging_ms]
        if len(targetmslist) > 0:
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

                if (not result_cont_ranges.has_key(source_name)):
                    result_cont_ranges[source_name] = {}

                cont_ranges_source_spw = []
                if (cont_ranges['fields'].has_key(source_name)):
                    if (cont_ranges['fields'][source_name].has_key(spwid)):
                        cont_ranges_source_spw = cont_ranges['fields'][source_name][spwid]
                else:
                    cont_ranges['fields'][source_name] = {}

                if (cont_ranges_source_spw != []):
                    LOG.info('Using existing selection "%s" for field %s, spw %s' % (cont_ranges_source_spw, source_name, spwid))
                    result_cont_ranges[source_name][spwid] = {'cont_ranges': cont_ranges_source_spw, 'plotfile': 'none', 'status': 'OLD'}
                    num_found += 1
                else:
                    LOG.info('Determining continuum ranges for field %s, spw %s' % (source_name, spwid))

                    findcont_basename = '%s.I.findcont' % (os.path.basename(target['imagename']).replace('spw%s' % (target['spw'].replace(',','_')), 'spw%s' % (spwid)).replace('STAGENUMBER', str(context.stage)))

                    # determine the gridder mode here (temporarily ...)
                    clheuristics = makeimlist.MakeImListHeuristics(context=context,
                                                                   vislist=inputs.vis,
                                                                   spw=spwid,
                                                                   contfile=context.contfile,
                                                                   linesfile=context.linesfile)
                    gridder = clheuristics.gridder(target['intent'], target['field'])

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
                            continue
                        scanids = str(scanids)
                        scanids = scanids.replace('[', '')
                        scanids = scanids.replace(']', '')
                        scanidlist.append(scanids)

                    # Estimate memory usage and adjust chanchunks parameter to avoid
                    # exceeding the available memory.
                    try:
                        mem_bytes = os.sysconf('SC_PAGE_SIZE') * os.sysconf('SC_PHYS_PAGES')
                    except ValueError:
                        # SC_PHYS_PAGES can be missing on OS X
                        mem_bytes = int(subprocess.check_output(['sysctl', '-n', 'hw.memsize']).strip())
                    mem_usable_bytes = 0.8 * mem_bytes
                    ms = context.observing_run.get_ms(name=inputs.vis[0])
                    spw_info = ms.get_spectral_window(spwid)
                    cube_bytes = target['imsize'][0] * target['imsize'][1] * spw_info.num_channels * 4
                    tclean_bytes = 9 * cube_bytes
                    chanchunks = int(tclean_bytes / mem_usable_bytes) + 1

                    parallel = mpihelpers.parse_mpi_input_parameter(inputs.parallel)

                    # Need to make an LSRK cube to get the real ranges in the source
                    # frame. The LSRK ranges will need to be translated to the
                    # individual TOPO ranges for the involved MSs.
                    job = casa_tasks.tclean(vis=inputs.vis, imagename=findcont_basename,
                        datacolumn=datacolumn,
                        spw=spwid, intent=utils.to_CASA_intent(inputs.ms[0], target['intent']),
                        scan=scanidlist, specmode='cube', gridder=gridder, pblimit=0.2,
                        niter=0, threshold='0mJy', deconvolver='hogbom',
                        interactive=False, outframe='LSRK', nchan=-1,
                        width='', imsize=target['imsize'],
                        cell=target['cell'], phasecenter=target['phasecenter'],
                        stokes='I', weighting='briggs', robust=0.5,
                        npixels=0, restoringbeam='common',
                        savemodel='none', chanchunks=chanchunks, parallel=parallel)
                    self._executor.execute(job)

                    # Try detecting continuum frequency ranges
                    cont_ranges['fields'][source_name][spwid], png = findcont_heuristics.find_continuum('%s.residual' % (findcont_basename))

                    result_cont_ranges[source_name][spwid] = {'cont_ranges': cont_ranges['fields'][source_name][spwid], 'plotfile': png, 'status': 'NEW'}

                    if (cont_ranges['fields'][source_name][spwid] not in [['NONE'], ['']]):
                        num_found += 1

                num_total += 1

        result = FindContResult(result_cont_ranges, cont_ranges, num_found, num_total)

        return result

    def analyse(self, result):
        return result
