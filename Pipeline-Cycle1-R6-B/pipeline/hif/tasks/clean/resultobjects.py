from __future__ import absolute_import

import collections
import os.path

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.displays as displays

LOG = infrastructure.get_logger(__name__)


class BoxResult(basetask.Results):
    def __init__(self):
        super(BoxResult, self).__init__()        
        self.threshold = None
        self.cleanmask = None
        self.island_peaks = None

    def merge_with_context(self, context):
        pass
    
    def __repr__(self):
        return 'BoxResult <threshold=%s cleanmask=%s>' % (
         self.threshold, self.cleanmask)


class CleanResult(basetask.Results):
    def __init__(self, sourcename=None, intent=None, spw=None, plotdir=None):
        super(CleanResult, self).__init__()
        self.sourcename = sourcename
        self.intent = intent
        self.spw = spw
        self.plotdir = plotdir
        self._psf = None
        self._model = None
        self._flux = None
        self.iterations = collections.defaultdict(dict)

    def empty(self):
        return not(self._psf or self._model or self._flux or 
          self.iterations!={})

    @property
    def flux(self):
        return self._flux

    # this is used to generate a pipeline product, not used by weblog
    @property
    def imageplot(self):
        iters = self.iterations.keys()
        iters.sort()
        image = self.iterations[iters[-1]]['image']
        imageplot = displays.sky.plotfilename(image=image,
          reportdir=self.plotdir)
        return imageplot

    def set_flux(self, image):
        self._flux = image

    @property
    def cleanmask(self, iter, image):
        iters = self.iterations.keys()
        iters.sort()
        if len(iters) > 0:
            return self.iterations[iters[-1]]['cleanmask']
        else:
            return None

    def set_cleanmask(self, iter, image):
        self.iterations[iter]['cleanmask'] = image

    @property
    def image(self):
        iters = self.iterations.keys()
        iters.sort()
        if len(iters) > 0:
            return self.iterations[iters[-1]]['image']
        else:
            return None

    def set_image(self, iter, image):
        self.iterations[iter]['image'] = image

    @property
    def model(self):
        iters = self.iterations.keys()
        iters.sort()
        if len(iters) > 0:
            return self.iterations[iters[-1]]['model']
        else:
            return None

    def set_model(self, iter, image):
        self.iterations[iter]['model'] = image

    @property
    def psf(self):
        return self._psf

    def set_psf(self, image):
        self._psf = image

    @property
    def residual(self):
        iters = self.iterations.keys()
        iters.sort()
        if len(iters) > 0:
            return self.iterations[iters[-1]]['residual']
        else:
            return None

    def set_residual(self, iter, image):
        self.iterations[iter]['residual'] = image

    def __repr__(self):
        repr = 'Clean:\n'
        if self._psf is not None:
            repr += ' psf: %s\n' % os.path.basename(self._psf)
        else:
            repr += ' psf: None'
        if self._flux is not None:
            repr += ' flux: %s\n' % os.path.basename(self._flux)
        else:
            repr += ' flux: None'
    
        for k,v in self.iterations.items():
            repr += ' iteration %s:\n' % k
            repr += '   image    : %s\n' % os.path.basename(v['image'])
            repr += '   residual : %s\n' % os.path.basename(v['residual'])
            repr += '   model    : %s\n' % os.path.basename(v['model'])
            if k > 0:
                repr += '   cleanmask: %s\n' % os.path.basename(v['cleanmask'])

        return repr



