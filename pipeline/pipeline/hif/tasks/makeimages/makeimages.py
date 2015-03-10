from __future__ import absolute_import
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

from ..tclean import Tclean
from ..tclean.resultobjects import TcleanResult
from .resultobjects import MakeImagesResult

from pipeline.hif.heuristics import makeimlist

LOG = infrastructure.get_logger(__name__)


class MakeImagesInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, target_list=None,
      weighting=None, robust=None, noise=None, npixels=None,
      hm_masking=None, hm_cleaning=None, tlimit=None, masklimit=None,
      maxncleans=None):
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
    def hm_masking(self):
        if self._hm_masking is None:
            return ''
        return self._hm_masking

    @hm_masking.setter
    def hm_masking(self, value):
        self._hm_masking = value

    @property
    def hm_cleaning(self):
        if self._hm_cleaning is None:
            return ''
        return self._hm_cleaning

    @hm_cleaning.setter
    def hm_cleaning(self, value):
        self._hm_cleaning = value

    @property
    def tlimit(self):
        if self._tlimit is None:
            return 2.0
        return self._tlimit

    @tlimit.setter
    def tlimit(self, value):
         self._tlimit = value

    @property
    def masklimit(self):
        if self._masklimit is None:
            return 2.0
        return self._masklimit

    @masklimit.setter
    def masklimit(self, value):
         self._masklimit = value

    @property
    def maxncleans(self):
        if self._maxncleans is None:
            return 10
        return self._maxncleans

    @maxncleans.setter
    def maxncleans(self, value):
         self._maxncleans = value

class MakeImages(basetask.StandardTaskTemplate):
    Inputs = MakeImagesInputs

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

        result = MakeImagesResult()

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

	    # set the imager mode here (temporarily ...)
            clheuristics = makeimlist.MakeImListHeuristics(
                context=inputs.context, vislist=full_image_target['vis'], \
                spw=full_image_target['spw'])
            full_image_target['gridmode'] = clheuristics.gridmode ( \
	        full_image_target['intent'], full_image_target['field'])
            full_image_target['deconvolver'] = clheuristics.deconvolver ( \
	        full_image_target['intent'], full_image_target['field'])

	    # set the weighting values.
            full_image_target['weighting'] = inputs.weighting
            full_image_target['robust'] = inputs.robust
            full_image_target['noise'] = inputs.noise
            full_image_target['npixels'] = inputs.npixels

	    if inputs.hm_masking == '':
	        if 'TARGET' in full_image_target['intent']:
                    # For the time being the target imaging uses the
                    # inner quarter. Other methods will be made available
                    # later.
	            #full_image_target['hm_masking'] = 'psfiter'
	            full_image_target['hm_masking'] = 'centralquarter'
		else:
	            full_image_target['hm_masking'] = 'centralquarter'
	    else:
	        full_image_target['hm_masking'] = inputs.hm_masking
	    if inputs.hm_cleaning == '':
                full_image_target['hm_cleaning'] = 'rms'
	    else:
                full_image_target['hm_cleaning'] = inputs.hm_cleaning
            full_image_target['tlimit'] = inputs.tlimit
            full_image_target['masklimit'] = inputs.masklimit
	    if full_image_target['hm_masking'] == 'psfiter':
                full_image_target['maxncleans'] = inputs.maxncleans
	    else:
                full_image_target['maxncleans'] = 1

            # build the task to do the cleaning and execute it
            datainputs = Tclean.Inputs(context=inputs.context,
              **full_image_target)
#            datainputs = Tclean.Inputs(context=inputs.context,
#	        output_dir=full_image_target['output_dir'],
#	        vis=full_image_target['vis'],
#	        imagename=full_image_target['imagename'],
#	        intent=full_image_target['intent'],
#	        field=full_image_target['field'],
#	        spw=full_image_target['spw'],
#	        uvrange=full_image_target['uvrange'],
#	        mode=full_image_target['mode'],
#	        imsize=full_image_target['imsize'],
#	        cell=full_image_target['cell'],
#	        phasecenter=full_image_target['phasecenter'],
#	        nchan=full_image_target['nchan'],
#	        start=full_image_target['start'],
#	        width=full_image_target['width'],
#	        weighting=full_image_target['weighting'],
#	        robust=full_image_target['robust'],
#	        noise=full_image_target['noise'],
#	        npixels=full_image_target['npixels'],
#	        hm_masking=full_image_target['hm_masking'],
#	        hm_cleaning=full_image_target['hm_cleaning'],
#	        tlimit=full_image_target['tlimit'],
#	        masklimit=full_image_target['masklimit'],
#	        maxncleans=full_image_target['maxncleans'],
#		gridmode=None,
#		deconvolver=None,
#		outframe=None,
#		restoringbeam=None,
#		iter=None, 
#		mask=None,
#		niter=None,
#		threshold=None)
            datatask = Tclean(datainputs)

	    clean_result = None
            try:
                clean_result = self._executor.execute(datatask)
                result.add_result(clean_result, image_target,
                  outcome='success')
            except Exception, e:
                LOG.error('Tclean failed error: %s' % (str(e)))
		if not clean_result:
                    result.add_result(TcleanResult(), image_target,
                        outcome='failure')
		else:
                    result.add_result(clean_result, image_target,
                        outcome='failure')

        return result

    def analyse(self, result):
        return result

