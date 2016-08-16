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
        self.sensitivity = None
        self.cleanmask = None
        self.island_peaks = None

    def merge_with_context(self, context):
        pass
    
    def __repr__(self):
        return 'BoxResult <threshold=%s cleanmask=%s>' % (
         self.threshold, self.cleanmask)


class TcleanResult(basetask.Results):
    def __init__(self, sourcename=None, intent=None, spw=None, specmode=None, multiterm=None, plotdir=None):
        super(TcleanResult, self).__init__()
        self.sourcename = sourcename
        self.intent = intent
        self.spw = spw
        self.specmode = specmode
        self.multiterm = multiterm
        self.plotdir = plotdir
        self._psf = None
        self._model = None
        self._flux = None
        self.iterations = collections.defaultdict(dict)
        self._aggregate_bw = 0.0
        self._eff_ch_bw = 0.0
        self._sensitivity = 0.0
        self._min_sensitivity = None
        self._max_sensitivity = None
        self._min_field_id = None
        self._max_field_id = None
        self._threshold = 0.0
        self._image_min = 0.0
        self._image_max = 0.0
        self._image_rms = 0.0
        # Temporarily needed until CAS-8576 is fixed
        self._residual_max = 0.0
        # This should be automatic, but it does not yet work
        self.pipeline_casa_task = 'Tclean'
        # Dummy settings for the weblog renderer
        self.results = [self]
        self.targets = ['']
        self.error = ''

    def empty(self):
        return not(self._psf or self._model or self._flux or 
          self.iterations!={})

    # this is used to generate a pipeline product, not used by weblog
    @property
    def imageplot(self):
        iters = self.iterations.keys()
        iters.sort()
        image = self.iterations[iters[-1]].get('image', None)
        imageplot = displays.sky.plotfilename(image=image,
          reportdir=self.plotdir)
        return imageplot

    @property
    def flux(self):
        return self._flux

    def set_flux(self, image):
        if self._flux is None:
            self._flux = image

    @property
    def cleanmask(self, iter, image):
        iters = self.iterations.keys()
        iters.sort()
        if len(iters) > 0:
            return self.iterations[iters[-1]].get('cleanmask', None)
        else:
            return None

    def set_cleanmask(self, iter, image):
        self.iterations[iter]['cleanmask'] = image

    @property
    def image(self):
        iters = self.iterations.keys()
        iters.sort()
        if len(iters) > 0:
            return self.iterations[iters[-1]].get('image', None)
        else:
            return None

    def set_image(self, iter, image):
        self.iterations[iter]['image'] = image

    @property
    def model(self):
        iters = self.iterations.keys()
        iters.sort()
        if len(iters) > 0:
            return self.iterations[iters[-1]].get('model', None)
        else:
            return None

    def set_model(self, iter, image):
        self.iterations[iter]['model'] = image

    @property
    def mom0_fc(self):
        iters = sorted(self.iterations.keys())
        return self.iterations[iters[-1]].get('mom0_fc')

    def set_mom0_fc(self, iter, image):
        self.iterations[iter]['mom0_fc'] = image

    @property
    def psf(self):
        return self._psf

    def set_psf(self, image):
        if self._psf is None:
            self._psf = image

    @property
    def residual(self):
        iters = self.iterations.keys()
        iters.sort()
        if len(iters) > 0:
            return self.iterations[iters[-1]].get('residual', None)
        else:
            return None

    def set_residual(self, iter, image):
        self.iterations[iter]['residual'] = image

    @property
    def aggregate_bw(self):
        return self._aggregate_bw

    def set_aggregate_bw(self, aggregate_bw):
        self._aggregate_bw = aggregate_bw

    @property
    def eff_ch_bw(self):
        return self._eff_ch_bw

    def set_eff_ch_bw(self, eff_ch_bw):
        self._eff_ch_bw = eff_ch_bw

    @property
    def sensitivity(self):
        return self._sensitivity

    def set_sensitivity(self, sensitivity):
        self._sensitivity = sensitivity

    @property
    def min_sensitivity(self):
        return self._min_sensitivity

    def set_min_sensitivity(self, min_sensitivity):
        self._min_sensitivity = min_sensitivity

    @property
    def max_sensitivity(self):
        return self._max_sensitivity

    def set_max_sensitivity(self, max_sensitivity):
        self._max_sensitivity = max_sensitivity

    @property
    def min_field_id(self):
        return self._min_field_id

    def set_min_field_id(self, min_field_id):
        self._min_field_id = min_field_id

    @property
    def max_field_id(self):
        return self._max_field_id

    def set_max_field_id(self, max_field_id):
        self._max_field_id = max_field_id

    @property
    def threshold(self):
        return self._threshold

    def set_threshold(self, threshold):
        self._threshold = threshold

    @property
    def image_min(self):
        return self._image_min

    def set_image_min(self, image_min):
        self._image_min = image_min

    @property
    def image_max(self):
        return self._image_max

    def set_image_max(self, image_max):
        self._image_max = image_max

    @property
    def image_rms(self):
        return self._image_rms

    def set_image_rms(self, image_rms):
        self._image_rms = image_rms

    def __repr__(self):
        repr = 'Tclean:\n'
        if self._psf is not None:
            repr += ' psf: %s\n' % os.path.basename(self._psf)
        else:
            repr += ' psf: None'
        if self._flux is not None:
            repr += ' flux: %s\n' % os.path.basename(self._flux)
        else:
            repr += ' flux: None'

        items_to_print = ['image','residual','model','cleanmask','mom0_fc']
        str_len = max([len(item) for item in items_to_print])
        for k,v in self.iterations.items():
            repr += ' iteration %s:\n' % k
            for item in items_to_print:
                if item in v:
                    repr += '   %s : %s\n' % (item.ljust(str_len), os.path.basename(v[item]))

        return repr
