from __future__ import absolute_import
import os
import tempfile
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.contfilehandler as contfilehandler
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.mpihelpers as mpihelpers
from pipeline.infrastructure import casa_tasks
from .resultobjects import FindContResult
from pipeline.hif.heuristics import findcont

LOG = infrastructure.get_logger(__name__)


class FindContInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, target_list=None,
                 parallel=None):
        self._init_properties(vars())

    @property
    def parallel(self):
        return self._parallel

    @parallel.setter
    def parallel(self, value):
        if value in ('true', 'True', 'TRUE', True, 1):
            value = True
        elif value in ('false', 'False', 'FALSE', False, 0):
            value = False
        else:
            value = mpihelpers.is_mpi_ready()
        self._parallel = value

    @property
    def target_list(self):
        return self._target_list

    @target_list.setter
    def target_list(self, value):
        if not value:
            value = self.context.clean_list_pending
        self._target_list = value


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

        findcont_heuristics = findcont.FindContHeuristics(context)

        contfile_handler = contfilehandler.ContFileHandler(context.contfile)
        cont_ranges = contfile_handler.read()

        result_cont_ranges = {}
        num_found = 0
        num_total = 0
        for i, target in enumerate(inputs.target_list):
            for spwid in target['spw'].split(','):
                source_name = target['field']

                if (not result_cont_ranges.has_key(source_name)):
                    result_cont_ranges[source_name] = {}

                cont_ranges_source_spw = []
                if (cont_ranges.has_key(source_name)):
                    if (cont_ranges[source_name].has_key(spwid)):
                        cont_ranges_source_spw = cont_ranges[source_name][spwid]
                else:
                    cont_ranges[source_name] = {}

                if (cont_ranges_source_spw != []):
                    LOG.info('Using existing selection "%s" for field %s, spw %s' % (cont_ranges_source_spw, source_name, spwid))
                    result_cont_ranges[source_name][spwid] = {'cont_ranges': cont_ranges_source_spw, 'plotfile': 'none', 'status': 'OLD'}
                    num_found += 1
                else:
                    LOG.info('Determining continuum ranges for field %s, spw %s' % (source_name, spwid))

                    findcont_basename = '%s.I.findcont' % (os.path.basename(target['imagename']))
                    job = casa_tasks.tclean(vis=inputs.vis, imagename=findcont_basename,
                        spw=spwid, intent=utils.to_CASA_intent(inputs.ms[0], target['intent']),
                        specmode='cube', gridder='mosaic', pblimit=0.2,
                        niter=0, threshold='0mJy', deconvolver='hogbom',
                        interactive=False, outframe='LSRK', nchan=-1,
                        width='', imsize=target['imsize'],
                        cell=target['cell'], phasecenter=target['phasecenter'],
                        stokes='I', weighting='briggs', robust=0.5,
                        npixels=0, savemodel='none', parallel=inputs.parallel)
                    self._executor.execute(job)

                    # Try detecting continuum frequency ranges
                    cont_ranges[source_name][spwid], png = findcont_heuristics.find_continuum('%s.residual' % (findcont_basename))

                    result_cont_ranges[source_name][spwid] = {'cont_ranges': cont_ranges[source_name][spwid], 'plotfile': png, 'status': 'NEW'}

                    if (cont_ranges[source_name][spwid] != ''):
                        num_found += 1

                num_total += 1

        result = FindContResult(result_cont_ranges, cont_ranges, num_found, num_total)

        return result

    def analyse(self, result):
        return result
