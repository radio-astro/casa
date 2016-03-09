from __future__ import absolute_import

import os

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure
from .basecleansequence import BaseCleanSequence
from .resultobjects import BoxResult

LOG = infrastructure.get_logger(__name__)


class ImageCentreThresholdSequence(BaseCleanSequence):

    def __init__(self, gridder, threshold='0.0mJy', sensitivity=0.0, niter=1000):
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

    def iteration(self, new_cleanmask, pblimit_image=0.2, pblimit_cleanmask=0.3):

        if (self.multiterm):
            extension = '.tt0'
        else:
            extension = ''

        if self.iter is None:
            raise Exception, 'no data for iteration'

        elif self.iter == 0:
            # next iteration, 1, should have mask covering central area:
            #   flux > 0.3 (or adjusted for image size) when flux available
            #   centre quarter otherwise
            if self.flux not in (None, ''):
                cm = casatools.image.newimagefromimage(infile=self.flux,
                  outfile=new_cleanmask, overwrite=True)
                # verbose = False to suppress warning message
                cm.calc('1', verbose=False)
                try:
                    cm.replacemaskedpixels('0')
                except Exception as e:
                    # We get here for the case of calmaxpixel limits and
                    # no masked pixel for the default pblimit values in iter0.
                    pass
                cm.calc('replace("%s"["%s" > %f], 0)' % (os.path.basename(new_cleanmask), self.flux, pblimit_cleanmask), verbose=False)
                cm.calcmask('"%s" > %s' % (self.flux, str(pblimit_image)))
                cm.done()
            else:
                cm = casatools.image.newimagefromimage(infile=self.residuals[0]+extension,
                  outfile=new_cleanmask, overwrite=True)
                cm.set(pixels='0')
                shape = cm.shape()
                rg = casatools.regionmanager
                region = rg.box([shape[0]/4, shape[1]/4],
                  [shape[0]-shape[0]/4, shape[1]-shape[1]/4])
                cm.set(pixels='1', region=region)
                rg.done()
                cm.done()
          
            self.result.cleanmask = new_cleanmask
            self.result.threshold = self.threshold
            self.result.sensitivity = self.sensitivity
            self.result.niter =  self.niter
            self.result.iterating = True
        else:
            self.result.cleanmask = ''
            self.result.threshold = '0.0mJy'
            self.result.sensitivity = 0.0
            self.result.niter =  0
            self.result.iterating = False

        return self.result
