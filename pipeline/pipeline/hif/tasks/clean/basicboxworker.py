from __future__ import absolute_import

#import os.path

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from .resultobjects import BoxResult
from pipeline.hif.heuristics import cleanbox as cbheuristic

LOG = infrastructure.get_logger(__name__)


class BasicBoxWorker():

    # We don't need to pass in mask, threshold, niter but keep the pattern
    # for now and cleanup later. Iter is initialized to -1. Result
    # is initialized.

    def __init__(self, iter=-1, mask='', threshold='0.0mJy', niter=1000): 
        self.iter = iter
	self.mask = mask
	self.threshold = threshold
	self.niter=niter
        self.result = BoxResult()

    # This method sets the iteration counter and returns statistics for
    # that iteration.
    def iteration_result(self, iter, psf, model, restored, residual,
      fluxscale, cleanmask, threshold=None):

        self.iter = iter

        #if cleanmask is not None and os.path.exists(cleanmask):
        model_sum, clean_rms, non_cleaned_rms, residual_max,\
            residual_min, rms2d, image_max = cbheuristic.analyse_clean_result(
            model, restored, residual, fluxscale, cleanmask)
	return model_sum, clean_rms, non_cleaned_rms, residual_max, \
	    residual_min, rms2d, image_max

    # This method insures that only a single iteration is performed.
    #     Keep this pattern until disposition of iterator code is finalized.
    def iterating (self):
	if (self.iter == -1):
	    return None

	if  self.iter == 0:
             self.result.cleanmask = self.mask
             self.result.threshold = self.threshold
             self.result.niter =  self.niter
	     self.result.iterating = True
	else:
             self.result.cleanmask = ''
             self.result.threshold = '0.0mJy'
             self.result.niter =  0
	     self.result.iterating = False

	return self.result.iterating


