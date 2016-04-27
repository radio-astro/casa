from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.hif.heuristics import cleanbox as cbheuristic
from .basecleansequence import BaseCleanSequence
from .resultobjects import BoxResult

LOG = infrastructure.get_logger(__name__)

class IterativeSequence2(BaseCleanSequence):

    def __init__(self, maxncleans, sensitivity, channel_rms_factor):
        BaseCleanSequence.__init__(self)

	self.maxncleans = maxncleans
	self.sensitivity = sensitivity
	self.channel_rms_factor = channel_rms_factor

        self.sidelobe_ratio = None
        self.psf = None
        self.flux = None

    def iteration (self, new_cleanmask):

        if self.iters[-1] == 0:
            # first iteration, clean large part of image down to well 
            # above noise 
            nm = casatools.image.newimagefromimage(infile=self.flux,
              outfile=new_cleanmask, overwrite=True)
            nm.calc('1.0')
            nm.close()
            nm.done()

            new_threshold = 10.0 * self.non_cleaned_rms_list[-1]
            new_threshold = '%sJy' % new_threshold
            self.result.threshold = new_threshold
            self.result.cleanmask = new_cleanmask
            self.result.island_peaks = {}
            self.result.niter = 2000
            self.result.iterating = True

        else:

            # island cleanbox/thresholding

            # iter 1 mask covered most of image area
            # iter 2 mask and ones after that should will 
            # accumulate islands
            if self.iters[-1] == 1:
                old_mask = None
            else:
                old_mask = self.cleanmasks[-1]

            new_threshold, island_peaks = cbheuristic.threshold_and_mask(
              residual=self.residuals[-1], old_mask=old_mask,
              new_mask=new_cleanmask, sidelobe_ratio=self.sidelobe_ratio,
              flux=self.flux)

            self.island_peaks_list.append(island_peaks)

            new_threshold = '%sJy' % new_threshold
            self.result.threshold = new_threshold
            self.result.cleanmask = new_cleanmask
            self.result.island_peaks = island_peaks
            self.result.niter = 2000

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
