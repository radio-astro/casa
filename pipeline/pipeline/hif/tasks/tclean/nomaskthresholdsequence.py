from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
from .basecleansequence import BaseCleanSequence
from .resultobjects import BoxResult
from pipeline.hif.heuristics import cleanbox as cleanbox

LOG = infrastructure.get_logger(__name__)


class NoMaskThresholdSequence(BaseCleanSequence):

    def __init__(self, gridder, threshold='0.0mJy', sensitivity=0.0, niter=0):
        """Constructor.
        """
        BaseCleanSequence.__init__(self)
        self.gridder = gridder
        self.threshold = threshold
        self.sensitivity = sensitivity
        self.niter = niter
        self.iter = None
        self.result = BoxResult()
        self.sidelobe_ratio = -1

    def iteration_result(self, iter, multiterm, psf, model, restored, residual,
                         flux, cleanmask, threshold=None, pblimit_image=0.2, pblimit_cleanmask=0.3):
        """This method sets the iteration counter and returns statistics for
        that iteration.
        """
        self.iter = iter
        self.multiterm = multiterm

        if self.sidelobe_ratio is None:
            self.sidelobe_ratio = cleanbox.psf_sidelobe_ratio(psf=psf, multiterm=multiterm)
        self.psf = psf
        self.flux = flux

        #if cleanmask is not None and os.path.exists(cleanmask):
        model_sum, residual_cleanmask_rms, residual_non_cleanmask_rms, residual_max, \
        residual_min, nonpbcor_image_non_cleanmask_rms, pbcor_image_min, pbcor_image_max, \
        residual_robust_rms = cleanbox.analyse_clean_result(multiterm, model, restored,
                                                                 residual, flux, cleanmask,
                                                                 pblimit_image, pblimit_cleanmask)

        LOG.info('Residual robust rms: %s', residual_robust_rms)
        peak_over_rms = residual_max/residual_robust_rms
        LOG.info('Residual peak/rms: %s', peak_over_rms)
        # TODO make threshold_nsigma a parameter that we can pass in via hif_editimlist()
        threshold_nsigma = 4.0
        self.threshold = residual_robust_rms * threshold_nsigma

        # Append the statistics.
        self.iters.append(iter)
        self.residuals.append(residual)
        self.cleanmasks.append(cleanmask)
        self.thresholds.append(threshold)
        self.model_sums.append(model_sum)
        self.residual_maxs.append(residual_max)
        self.residual_mins.append(residual_min)
        self.residual_non_cleanmask_rms_list.append(residual_non_cleanmask_rms)
        self.image_non_cleanmask_rms_list.append(nonpbcor_image_non_cleanmask_rms)

        return model_sum, residual_cleanmask_rms, residual_non_cleanmask_rms, residual_max, \
               residual_min, nonpbcor_image_non_cleanmask_rms, pbcor_image_min, pbcor_image_max

    def iteration(self, new_cleanmask='', pblimit_image=0.2, pblimit_cleanmask=0.3, spw=None, frequency_selection=None):

        if self.iter is None:
            raise Exception, 'no data for iteration'
        elif self.iter == 0:
            self.result.cleanmask = ''
            self.result.threshold = self.threshold
            self.result.sensitivity = self.sensitivity
            self.result.niter = self.niter
            self.result.iterating = True
        else:
            self.result.cleanmask = ''
            self.result.threshold = '0.0mJy'
            self.result.sensitivity = 0.0
            self.result.niter = 0
            self.result.iterating = False

        return self.result
