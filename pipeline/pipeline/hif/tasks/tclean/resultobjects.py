from __future__ import absolute_import

import collections
import os.path
import copy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils

from pipeline.h.tasks.common.displays import sky as sky

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
    def __init__(self, vis=None, sourcename=None, intent=None, spw=None, orig_specmode=None, specmode=None, multiterm=None, plotdir=None,
                 imaging_mode=None):
        super(TcleanResult, self).__init__()
        self.vis = vis
        self.sourcename = sourcename
        self.intent = intent
        self.spw = spw
        self.orig_specmode = orig_specmode
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
        self._dr_corrected_sensitivity = 0.0
        self._threshold = 0.0
        self._dirty_dynamic_range = 0.0
        self._DR_correction_factor = 1.0
        self._maxEDR_used = False
        self._image_min = 0.0
        self._image_max = 0.0
        self._image_rms = 0.0
        self._image_rms_min = 0.0
        self._image_rms_max = 0.0
        # Temporarily needed until CAS-8576 is fixed
        self._residual_max = 0.0
        self._tclean_stopcode = 0
        self._tclean_stopreason = None
        self._tclean_iterdone = 0
        # This should be automatic, but it does not yet work
        self.pipeline_casa_task = 'Tclean'
        # Dummy settings for the weblog renderer
        self.results = [self]
        self.targets = ['']
        self.warning = None
        self.error = None
        # Used to make simple telescope-dependent decisions about weblog output
        self.imaging_mode = imaging_mode
        self.per_spw_cont_sensitivities_all_chan = None
        self.check_source_fit = None

    def merge_with_context(self, context):
        # Calculated sensitivities for later stages
        if self.per_spw_cont_sensitivities_all_chan is not None:
            if 'recalc' in result.per_spw_cont_sensitivities_all_chan:
                context.per_spw_cont_sensitivities_all_chan = copy.deepcopy(self.per_spw_cont_sensitivities_all_chan)
                del context.per_spw_cont_sensitivities_all_chan['recalc']
            else:
                utils.update_sens_dict(context.per_spw_cont_sensitivities_all_chan, self.per_spw_cont_sensitivities_all_chan)

    def empty(self):
        return not(self._psf or self._model or self._flux or 
          self.iterations!={})

    # this is used to generate a pipeline product, not used by weblog
    @property
    def imageplot(self):
        iters = self.iterations.keys()
        iters.sort()
        image = self.iterations[iters[-1]].get('image', None)
        imageplot = sky.plotfilename(image=image,
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
    def mom8_fc(self):
        iters = sorted(self.iterations.keys())
        return self.iterations[iters[-1]].get('mom8_fc')

    def set_mom8_fc(self, iter, image):
        self.iterations[iter]['mom8_fc'] = image

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
    def dr_corrected_sensitivity(self):
        return self._dr_corrected_sensitivity

    def set_dr_corrected_sensitivity(self, dr_corrected_sensitivity):
        self._dr_corrected_sensitivity = dr_corrected_sensitivity

    @property
    def threshold(self):
        return self._threshold

    def set_threshold(self, threshold):
        self._threshold = threshold

    @property
    def dirty_dynamic_range(self):
        return self._dirty_dynamic_range

    def set_dirty_dynamic_range(self, dirty_dynamic_range):
        self._dirty_dynamic_range = dirty_dynamic_range

    @property
    def DR_correction_factor(self):
        return self._DR_correction_factor

    def set_DR_correction_factor(self, DR_correction_factor):
        self._DR_correction_factor = DR_correction_factor

    @property
    def maxEDR_used(self):
        return self._maxEDR_used

    def set_maxEDR_used(self, maxEDR_used):
        self._maxEDR_used = maxEDR_used

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

    @property
    def image_rms_min(self):
        return self._image_rms_min

    def set_image_rms_min(self, image_rms_min):
        self._image_rms_min = image_rms_min

    @property
    def image_rms_max(self):
        return self._image_rms_max

    def set_image_rms_max(self, image_rms_max):
        self._image_rms_max = image_rms_max

    @property
    def tclean_stopcode(self):
        return self._tclean_stopcode

    def set_tclean_stopcode(self, tclean_stopcode):
        self._tclean_stopcode = tclean_stopcode

    @property
    def tclean_stopreason(self):
        return self._tclean_stopreason

    def set_tclean_stopreason(self, tclean_stopcode):
        stopreasons = ['iteration limit', 'threshold', 'force stop',
                       'no change in peak residual across two major cycles',
                       'peak residual increased by more than 5 times from the previous major cycle',
                       'peak residual increased by more than 5 times from the minimum reached', 'zero mask']
        assert 0 < tclean_stopcode <= len(stopreasons),\
            "tclean stop code {} does not index into stop reasons list".format(tclean_stopcode)
        self._tclean_stopreason = stopreasons[tclean_stopcode-1]

    @property
    def tclean_iterdone(self):
        return self._tclean_iterdone

    def set_tclean_iterdone(self, tclean_iterdone):
        self._tclean_iterdone = tclean_iterdone

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
