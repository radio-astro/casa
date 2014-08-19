from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
from .resultobjects import BoxResult
from pipeline.hif.heuristics import cleanbox as cbheuristic

LOG = infrastructure.get_logger(__name__)

class IterBoxWorker:

    def __init__(self,  maxncleans):

	self.maxncleans = maxncleans

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

    def iteration_result(self, iter, psf, model, restored, residual,
      fluxscale, cleanmask, threshold):

        if self.sidelobe_ratio is None:
            self.sidelobe_ratio = cbheuristic.psf_sidelobe_ratio(psf=psf)

        self.psf = psf
        self.fluxscale = fluxscale

        model_sum, clean_rms, non_cleaned_rms, residual_max,\
          residual_min, rms2d, image_max = cbheuristic.analyse_clean_result(
          model, restored, residual, fluxscale, cleanmask)

	# Append the statistics.
        self.iters.append(iter)
        self.residuals.append(residual)
        self.cleanmasks.append(cleanmask)
        self.thresholds.append(threshold)
        self.model_sums.append(model_sum)
        self.residual_maxs.append(residual_max)
        self.residual_mins.append(residual_min)
        self.non_cleaned_rms_list.append(non_cleaned_rms)

        return model_sum, clean_rms, non_cleaned_rms, residual_max, \
          residual_min, rms2d, image_max

    def new_cleanmask(self, new_cleanmask):
        self._new_cleanmask = new_cleanmask

    def iterating (self):
        return self.result.iterating

    def iteration (self):

        new_threshold, island_peaks = cbheuristic.threshold_and_mask(
            residual=self.residuals[-1], old_mask=self.cleanmasks[-1],
            new_mask=self._new_cleanmask, sidelobe_ratio=self.sidelobe_ratio,
            fluxscale=self.fluxscale)

        self.island_peaks_list.append(island_peaks)

        new_threshold = '%sJy' % new_threshold
        self.result.threshold = new_threshold
        self.result.cleanmask = self._new_cleanmask
        self.result.island_peaks = island_peaks
        self.result.niter = 1000

        self.result.iterating = cbheuristic.clean_more(
            loop=self.iters[-1],
            threshold_list=self.thresholds,
            new_threshold=new_threshold,
            sum=self.model_sums[-1], 
            residual_max=self.residual_maxs[-1],
            residual_min=self.residual_mins[-1], 
            non_cleaned_rms=self.non_cleaned_rms_list[-1],
            island_peaks_list=self.island_peaks_list,
            flux_list=self.model_sums)

        # Are we exceeding max number of iterations ?
        if self.result.iterating and self.iters[-1] > self.maxncleans:
            LOG.warning('terminate clean; threshiter (%s) >= %s' % (
                self.iters[-1], self.maxncleans))
            self.result.iterating = False

        return self.result
