from __future__ import absolute_import

import os.path
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from ..clean import Clean
from ..clean.resultobjects import CleanResult
from .resultobjects import CleanListResult

LOG = infrastructure.get_logger(__name__)


class CleanListInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, target_list=None,
      weighting=None, robust=None, noise=None, npixels=None,
      hm_cleanboxing=None, mask=None, threshold=None, maxthreshiter=None):
        self._init_properties(vars())

    @property
    def target_list(self):
        if self._target_list is None:
            return {}
        return self._target_list

    @target_list.setter
    def target_list(self, value):
        self._target_list = value

    @property
    def weighting(self):
        if self._weighting is None:
            return 'natural'
        return self._weighting

    @weighting.setter
    def weighting(self, value):
        self._weighting = value

    @property
    def robust(self):
        if self._robust is None:
            return 0.0
        return self._robust

    @robust.setter
    def robust(self, value):
        self._robust = value

    @property
    def noise(self):
        if self._noise is None:
            return '1.0Jy'
        return self._noise

    @noise.setter
    def noise(self, value):
        self._noise = value

    @property
    def npixels(self):
        if self._npixels is None:
            return 0
        return self._npixels

    @npixels.setter
    def npixels(self, value):
        self._npixels = value

    @property
    def hm_cleanboxing(self):
        if self._hm_cleanboxing is None:
            return 'automatic'
        return self._hm_cleanboxing

    @hm_cleanboxing.setter
    def hm_cleanboxing(self, value):
        self._hm_cleanboxing = value

    @property
    def mask(self):
        if self._mask is None:
            return ''
        return self._mask

    @mask.setter
    def mask(self, value):
        self._mask = value

    @property
    def threshold(self):
        if self._target_list is None:
            return 0.0
        return self._threshold

    @threshold.setter
    def threshold(self, value):
        self._threshold = value

    @property
    def maxthreshiter(self):
        if self._maxthreshiter is None:
            return 10
        return self._maxthreshiter

    @maxthreshiter.setter
    def maxthreshiter(self, value):
        self._maxthreshiter = value


class CleanList(basetask.StandardTaskTemplate):
    Inputs = CleanListInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs

        # make sure inputs.vis is a list, even it is one that contains a
        # single measurement set
        if type(inputs.vis) is not types.ListType:
            inputs.vis = [inputs.vis]

        # get the target list; if none specified get from context
        target_list = inputs.target_list
        if target_list == {}:
            target_list = inputs.context.clean_list_pending

        result = CleanListResult()

        for image_target in target_list:
            # format a report of the clean to be run next and output it
            target_str = ''
            for k,v in image_target.iteritems():
                target_str += '%s=%s ' % (k,v)
            LOG.debug('Cleaning %s' % target_str)

            # add inputs values to image_target string to complete the
            # parameter list to the clean task
            full_image_target = dict(image_target)
            full_image_target['output_dir'] = inputs.output_dir
            full_image_target['vis'] = inputs.vis

            full_image_target['weighting'] = inputs.weighting
            if inputs.weighting == 'briggs':
                full_image_target['robust'] = inputs.robust
                full_image_target['npixels'] = inputs.npixels
            elif inputs.weighting == 'briggsabs':
                full_image_target['robust'] = inputs.robust
                full_image_target['noise'] = inputs.noise
            elif inputs.weighting == 'superuniform':
                full_image_target['npixels'] = inputs.npixels

            if inputs.hm_cleanboxing == 'automatic':
                if full_image_target['intent'] == 'TARGET':
                    full_image_target['hm_cleanboxing'] = 'iterative'
                else:
                    full_image_target['hm_cleanboxing'] = 'calibrator'
            else:
                full_image_target['hm_cleanboxing'] = inputs.hm_cleanboxing

            if inputs.hm_cleanboxing == 'iterative':
                full_image_target['maxthreshiter'] = inputs.maxthreshiter
            elif inputs.hm_cleanboxing == 'manual':
                full_image_target['mask'] = inputs.mask
                full_image_target['threshold'] = inputs.threshold

            # build the task to do the cleaning and execute it
            datainputs = Clean.Inputs(context=inputs.context,
              **full_image_target)
            datatask = Clean(datainputs)

            try:
                clean_result = self._executor.execute(datatask)
                result.add_result(clean_result, image_target,
                  outcome='success')
            except Exception, e:
                LOG.error('clean failed')
                LOG.exception(e)
                result.add_result(CleanResult(), image_target,
                  outcome='failure')

        return result

    def analyse(self, result):
        return result

