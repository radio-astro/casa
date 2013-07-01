from __future__ import absolute_import

import os.path

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from .resultobjects import BoxResult

from pipeline.hif.heuristics import cleanbox as heuristic

LOG = infrastructure.get_logger(__name__)


class ManualBoxWorkerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir, vis, mask, threshold):
        self._init_properties(vars())

    @property
    def mask(self):
        if self._mask is None:
            return ''
        return self._mask

    @mask.setter
    def mask(self, value):
        self._mask = value


class ManualBoxWorker(basetask.StandardTaskTemplate):
    Inputs = ManualBoxWorkerInputs
    
    def __init__(self, inputs):
        super(ManualBoxWorker, self).__init__(inputs)
        self.inputs = inputs

        self.iters = []
        self.result = BoxResult()

    def is_multi_vis_task(self):
        return True

    def iteration_result(self, iter, psf, model, restored, residual,
      fluxscale, cleanmask, threshold):

        if cleanmask is not None and os.path.exists(cleanmask):
           model_sum, clean_rms, non_cleaned_rms, residual_max,\
             residual_min, rms2d, image_max = heuristic.analyse_clean_result(
             model, restored, residual, fluxscale, cleanmask)

        self.iters.append(iter)

    def new_cleanmask(self, new_cleanmask):
        pass

    def prepare(self):
        inputs = self.inputs

        jobs = []

        self.result.threshold = inputs.threshold
        self.result.cleanmask = inputs.mask
        self.result.iterating = (self.iters[-1]==0)
 
        return self.result

    def analyse(self, result):
        return result
