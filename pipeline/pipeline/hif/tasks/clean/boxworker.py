from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from .resultobjects import BoxResult

from pipeline.hif.heuristics import cleanbox as heuristic

LOG = infrastructure.get_logger(__name__)


class BoxWorkerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir, vis):
        self._init_properties(vars())


class BoxWorker(basetask.StandardTaskTemplate):
    Inputs = BoxWorkerInputs
    
    def __init__(self, inputs):
        super(BoxWorker, self).__init__(inputs)
#        self.cleanboxheuristics = cleanbox.CleanBoxHeuristics()
        self.sidelobe_ratio = None
        self.psf = None
        self.fluxscale = None

        self.iters = []
        self.residuals = []
        self.cleanmasks = []
        self.model_sums = []
        self.residual_maxs = []
        self.residual_mins = []
        self.non_cleaned_rms_list = []
        self.island_peaks_list = []
        self.thresholds = []

        self.result = BoxResult()

    def is_multi_vis_task(self):
        return True

    def iteration_result(self, iter, psf, model, restored, residual,
      fluxscale, cleanmask, threshold):

        if self.sidelobe_ratio is None:
            self.sidelobe_ratio = heuristic.psf_sidelobe_ratio(psf=psf)

        self.psf = psf
        self.fluxscale = fluxscale

        model_sum, clean_rms, non_cleaned_rms, residual_max,\
          residual_min, rms2d, image_max = heuristic.analyse_clean_result(
          model, restored, residual, fluxscale, cleanmask)

        self.iters.append(iter)
        self.residuals.append(residual)
        self.cleanmasks.append(cleanmask)
        self.model_sums.append(model_sum)
        self.residual_maxs.append(residual_max)
        self.residual_mins.append(residual_min)
        self.non_cleaned_rms_list.append(non_cleaned_rms)

    def new_cleanmask(self, new_cleanmask):
        self._new_cleanmask = new_cleanmask

    def prepare(self):
        inputs = self.inputs
#        print 'boxworker prepare',
#        for i in self.iters:
#            print i
#            print self.residuals[i]
#            print self.cleanmasks[i]

        jobs = []

        new_threshold, island_peaks = heuristic.threshold_and_mask(
          residual=self.residuals[-1], old_mask=self.cleanmasks[-1],
          new_mask=self._new_cleanmask, sidelobe_ratio=self.sidelobe_ratio,
          fluxscale=self.fluxscale)

        self.thresholds.append(new_threshold)
        self.island_peaks_list.append(island_peaks)

        self.result.threshold = new_threshold
        self.result.cleanmask = self._new_cleanmask
        self.result.island_peaks = island_peaks

        self.result.iterating = heuristic.clean_more(
          loop=self.iters[-1],
          old_threshold=self.thresholds[-1],
          new_threshold=new_threshold,
          sum=self.model_sums[-1], 
          residual_max=self.residual_maxs[-1],
          residual_min=self.residual_mins[-1], 
          non_cleaned_rms=self.non_cleaned_rms_list[-1],
          island_peaks_list=self.island_peaks_list,
          flux_list=self.model_sums)

        return self.result

    def analyse(self, result):
        return result
