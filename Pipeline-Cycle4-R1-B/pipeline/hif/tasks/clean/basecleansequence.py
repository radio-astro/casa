from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
from pipeline.hif.heuristics import cleanbox as cbheuristic
from .resultobjects import BoxResult

LOG = infrastructure.get_logger(__name__)


class BaseCleanSequence:

    def __init__(self): 
        """Constructor.
        """
        self.iter = None
        self.result = BoxResult()

        self.sidelobe_ratio = None
        self.psf = None
        self.flux = None

        self.iters = []
        self.residuals = []
        self.cleanmasks = []
        self.model_sums = []
        self.residual_maxs = []
        self.residual_mins = []
        self.non_cleaned_rms_list = []
        self.island_peaks_list = []
        self.thresholds = []

    def iteration_result(self, iter, psf, model, restored, residual,
      flux, cleanmask, threshold=None):
        """This method sets the iteration counter and returns statistics for
        that iteration.
        """
        self.iter = iter

        if self.sidelobe_ratio is None:
            self.sidelobe_ratio = cbheuristic.psf_sidelobe_ratio(psf=psf)
        self.psf = psf
        self.flux = flux

        #if cleanmask is not None and os.path.exists(cleanmask):
        model_sum, clean_rms, non_cleaned_rms, residual_max,\
          residual_min, rms2d, image_max = cbheuristic.analyse_clean_result(
          model, restored, residual, flux, cleanmask)

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

    def iteration (self, new_cleanmask):
        """The base boxworker allows only one iteration.
        """

	if self.iter is None:
	    raise Exception, 'no data for iteration'

	else:
            self.result.threshold = '0.0Jy'
            self.result.cleanmask = ''
            self.result.niter = 0
            self.result.iterating = False

	return self.result


