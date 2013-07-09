from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from .resultobjects import BoxResult

from pipeline.hif.heuristics import cleanbox as heuristic

LOG = infrastructure.get_logger(__name__)


class CalibratorBoxWorkerInputs(basetask.StandardInputs):
    """This class implements the clean mask/niter approach suggested
    for calibrators by Eric Villard in July 2013."""

    def __init__(self, context, output_dir, vis):
        self._init_properties(vars())


class CalibratorBoxWorker(basetask.StandardTaskTemplate):
    Inputs = CalibratorBoxWorkerInputs
    
    def __init__(self, inputs):
        super(CalibratorBoxWorker, self).__init__(inputs)
        self.result = BoxResult()

    def is_multi_vis_task(self):
        return True

    def iteration_result(self, iter, psf, model, restored, residual,
      fluxscale, cleanmask, threshold):

        self.psf = psf
        self.residual = residual
        self.iter = iter

        model_sum, clean_rms, non_cleaned_rms, residual_max,\
          residual_min, rms2d, image_max = heuristic.analyse_clean_result(
          model, restored, residual, fluxscale, cleanmask)

    def new_cleanmask(self, new_cleanmask):
        self._new_cleanmask = new_cleanmask

    def prepare(self):
        inputs = self.inputs

        if self.iter==0:
            niter = heuristic.niter_and_mask(psf=self.psf,
              residual=self.residual, new_mask=self._new_cleanmask)

            self.result.threshold = '0.0Jy'
            self.result.cleanmask = self._new_cleanmask
            self.result.niter = niter
            self.result.iterating = True
        else:
            self.result.threshold = '0.0Jy'
            self.result.cleanmask = ''
            self.result.niter = 0
            self.result.iterating = False

        return self.result

    def analyse(self, result):
        return result
