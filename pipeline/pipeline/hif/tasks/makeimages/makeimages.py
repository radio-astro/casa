from __future__ import absolute_import
import os
import tempfile
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.mpihelpers as mpihelpers

from ..tclean import Tclean
from ..tclean.resultobjects import TcleanResult
from .resultobjects import MakeImagesResult

LOG = infrastructure.get_logger(__name__)


class MakeImagesInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, target_list=None,
                 weighting=None, robust=None, noise=None, npixels=None,
                 hm_masking=None, hm_sidelobethreshold=None, hm_noisethreshold=None,
                 hm_lownoisethreshold=None, hm_negativethreshold=None,
                 hm_minbeamfrac=None, hm_growiterations=None,
                 hm_cleaning=None, tlimit=None,
                 masklimit=None, maxncleans=None, cleancontranges=None, parallel=None):
        self._init_properties(vars())

    @property
    def target_list(self):
        return self._target_list

    @target_list.setter
    def target_list(self, value):
        if not value:
            value = self.context.clean_list_pending
        self._target_list = value

    hm_cleaning = basetask.property_with_default('hm_cleaning', 'rms')
    hm_masking = basetask.property_with_default('hm_masking', 'auto')
    hm_sidelobethreshold = basetask.property_with_default('hm_sidelobethreshold', -999.0)
    hm_noisethreshold = basetask.property_with_default('hm_noisethreshold', -999.0)
    hm_lownoisethreshold = basetask.property_with_default('hm_lownoisethreshold', -999.0)
    hm_negativethreshold = basetask.property_with_default('hm_negativethreshold', -999.0)
    hm_minbeamfrac = basetask.property_with_default('hm_minbeamfrac', -999.0)
    hm_growiterations = basetask.property_with_default('hm_growiterations', -999)
    masklimit = basetask.property_with_default('masklimit', 2.0)
    maxncleans = basetask.property_with_default('maxncleans', 10)
    noise = basetask.property_with_default('noise', '1.0Jy')
    npixels = basetask.property_with_default('npixels', 0)
    parallel = basetask.property_with_default('parallel', 'automatic')
    robust = basetask.property_with_default('robust', -999.0)
    cleancontranges = basetask.property_with_default('cleancontranges', False)
    tlimit = basetask.property_with_default('tlimit', 2.0)
    weighting = basetask.property_with_default('weighting', 'briggs')


# tell the infrastructure to give us mstransformed data when possible by
# registering our preference for imaging measurement sets
api.ImagingMeasurementSetsPreferred.register(MakeImagesInputs)


class MakeImages(basetask.StandardTaskTemplate):
    Inputs = MakeImagesInputs

    is_multi_vis_task = True

    def prepare(self):
        inputs = self.inputs

        # make sure inputs.vis is a list, even it is one that contains a
        # single measurement set
        if type(inputs.vis) is not types.ListType:
            inputs.vis = [inputs.vis]

        result = MakeImagesResult()

        # Carry any message from hif_makeimlist (e.g. for missing PI cube target)
        result.set_info(inputs.context.clean_list_info)

        with CleanTaskFactory(inputs, self._executor) as factory:
            task_queue = [(target, factory.get_task(target))
                          for target in inputs.target_list]

            for (target, task) in task_queue:
                try:
                    worker_result = task.get_result()
                except mpihelpers.PipelineError:
                    result.add_result(TcleanResult(), target, outcome='failure')
                else:
                    result.add_result(worker_result, target, outcome='success')

        # set of descriptions
        if inputs.context.clean_list_info.get('msg', '') != '':
            description = {
                _get_description_map(inputs.context.clean_list_info.get('intent', '')).get(inputs.context.clean_list_info.get('specmode', ''), 'Calculate clean products')  # map specmode to description..
            }
        else:
            description = {
                _get_description_map(target['intent']).get(target['specmode'], 'Calculate clean products')  # map specmode to description..
                for target in inputs.target_list                       # .. for every clean target..
            }

        result.metadata['long description'] = ' / '.join(description)

        return result

    def analyse(self, result):
        return result


class CleanTaskFactory(object):
    def __init__(self, inputs, executor):
        self.__inputs = inputs
        self.__context = inputs.context
        self.__executor = executor
        self.__context_path = None

    def __enter__(self):
        # If there's a possibility that we'll submit MPI jobs, save the context
        # to disk ready for import by the MPI servers.
        if mpihelpers.mpiclient:
            # Use the tempfile module to generate a unique temporary filename,
            # which we use as the output path for our pickled context
            tmpfile = tempfile.NamedTemporaryFile(suffix='.context',
                                                  dir=self.__context.output_dir,
                                                  delete=True)
            self.__context_path = tmpfile.name
            tmpfile.close()

            self.__context.save(self.__context_path)

        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.__context_path and os.path.exists(self.__context_path):
            os.unlink(self.__context_path)

    def get_task(self, target):
        """
        Create and return a SyncTask or AsyncTask for the clean job required
        to produce the clean target.

        The current algorithm generates Tier 0 clean jobs for calibrator
        images (=AsyncTask) and Tier 1 clean jobs for target images
        (=SyncTask).

        :param target: a clean job definition generated by MakeImList
        :return: a SyncTask or AsyncTask
        """
        task_args = self.__get_task_args(target)

        is_mpi_ready = mpihelpers.is_mpi_ready()
        is_cal_image = 'TARGET' not in target['intent']

        is_tier0_job = is_mpi_ready and is_cal_image
        parallel_wanted = mpihelpers.parse_mpi_input_parameter(self.__inputs.parallel)

        if is_tier0_job and parallel_wanted:
            executable = mpihelpers.Tier0PipelineTask(Tclean,
                                                      task_args,
                                                      self.__context_path)
            return mpihelpers.AsyncTask(executable)
        else:
            inputs = Tclean.Inputs(self.__context, **task_args)
            task = Tclean(inputs)
            return mpihelpers.SyncTask(task, self.__executor)

    def __get_task_args(self, target):
        inputs = self.__inputs

        parallel_wanted = mpihelpers.parse_mpi_input_parameter(inputs.parallel)

        # request Tier 1 tclean parallelisation if the user requested it, this
        # is science target imaging, and we are running as an MPI client.
        parallel = all([parallel_wanted,
                        'TARGET' in target['intent'],
                        mpihelpers.is_mpi_ready()])

        task_args = dict(target)
        task_args.update({
            'output_dir': inputs.output_dir,
            'vis': inputs.vis,
            # set the weighting values.
            'weighting': inputs.weighting,
            'noise': inputs.noise,
            'npixels': inputs.npixels,
            # other vals
            'tlimit': inputs.tlimit,
            'masklimit': inputs.masklimit,
            'cleancontranges': inputs.cleancontranges,
            'parallel': parallel,
        })

        if target['robust']:
            task_args['robust'] = target['robust']
        else:
            task_args['robust'] = inputs.robust

        # set the imager mode here (temporarily ...)
        image_heuristics = target['heuristics']
        if target['gridder'] is not None:
            task_args['gridder'] = target['gridder']
        else:
            task_args['gridder'] = image_heuristics.gridder(
                    task_args['intent'], task_args['field'])

        if inputs.hm_masking == '':
            if 'TARGET' in task_args['intent']:
                # For the time being the target imaging uses the
                # inner quarter. Other methods will be made available
                # later.
                #datatask_args['hm_masking'] = 'psfiter'
                task_args['hm_masking'] = 'auto'
            else:
                task_args['hm_masking'] = 'auto'
        else:
            task_args['hm_masking'] = inputs.hm_masking

        if inputs.hm_masking == 'auto':
            task_args['hm_sidelobethreshold'] = inputs.hm_sidelobethreshold
            task_args['hm_noisethreshold'] = inputs.hm_noisethreshold
            task_args['hm_lownoisethreshold'] = inputs.hm_lownoisethreshold
            task_args['hm_negativethreshold'] = inputs.hm_negativethreshold
            task_args['hm_minbeamfrac'] = inputs.hm_minbeamfrac
            task_args['hm_growiterations'] = inputs.hm_growiterations

        if inputs.hm_cleaning == '':
            task_args['hm_cleaning'] = 'rms'
        else:
            task_args['hm_cleaning'] = inputs.hm_cleaning

        if task_args['hm_masking'] == 'psfiter':
            task_args['maxncleans'] = inputs.maxncleans
        else:
            task_args['maxncleans'] = 1

        return task_args


def _get_description_map(intent):
    if intent in ('PHASE', 'BANDPASS', 'CHECK'):
        return {
            'mfs': 'Make calibrator images'
        }
    elif intent == 'TARGET':
        return {
            'mfs': 'Make target per-spw continuum images',
            'cont': 'Make target aggregate continuum images',
            'cube': 'Make target cubes',
            'repBW': 'Make representative bandwidth target cube'

        }
    else:
        return {}
