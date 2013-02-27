from __future__ import absolute_import
import types
import os

import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
from pipeline.infrastructure.jobrequest import casa_tasks

from .. import bandpass
from .. import gaincal
from pipeline.hif.tasks.common import arrayflaggerbase
from . import resultobjects 
from . import wvrg_qa2

from pipeline.hif.heuristics import wvrgcal as heuwvrgcal
from pipeline.hif.heuristics import caltable as wcaltable

LOG = logging.get_logger(__name__)


class WvrgcalInputs(basetask.StandardInputs):
    
    def __init__(self, context, output_dir=None,
      vis=None, caltable=None, toffset=None, segsource=None, 
      hm_tie=None, tie=None, sourceflag=None, nsol=None,
      disperse=None, wvrflag=None, hm_smooth=None, smooth=None,
      scale=None, qa2_intent=None, qa2_bandpass_intent=None):
	self._init_properties(vars())

    @property
    def toffset(self):
        if self._toffset is None:
            return -1
        return self._toffset

    @toffset.setter
    def toffset(self, value):
        self._toffset = value

    @property
    def segsource(self):
        if self._segsource is None:
            return True
        return self._segsource

    @segsource.setter
    def segsource(self, value):
        self._segsource = value

    @property
    def hm_tie(self):
        if self._hm_tie is None:
            return 'automatic'
        return self._hm_tie

    @hm_tie.setter
    def hm_tie(self, value):
        self._hm_tie = value

    @property
    def tie(self):
        if self._tie is None:
            return []
        return self._tie

    @tie.setter
    def tie(self, value):
        self._tie = value

    @property
    def sourceflag(self):
        if self._sourceflag is None:
            return []
        return self._sourceflag

    @sourceflag.setter
    def sourceflag(self, value):
        self._sourceflag = value

    @property
    def nsol(self):
        if self._nsol is None:
            return 1
        return self._nsol

    @nsol.setter
    def nsol(self, value):
        self._nsol = value

    @property
    def disperse(self):
        if self._disperse is None:
            return False
        return self._disperse

    @disperse.setter
    def disperse(self, value):
        self._disperse = value

    @property
    def wvrflag(self):
        if self._wvrflag is None:
            return []
        return self._wvrflag

    @wvrflag.setter
    def wvrflag(self, value):
        self._wvrflag = value

    @property
    def hm_smooth(self):
        if self._hm_smooth is None:
            return 'manual'
        return self._hm_smooth

    @hm_smooth.setter
    def hm_smooth(self, value):
        self._hm_smooth = value

    @property
    def smooth(self):
        if self._smooth is None:
            return 1
        return self._smooth

    @smooth.setter
    def smooth(self, value):
        self._smooth = value

    @property
    def scale(self):
        if self._scale is None:
            return 1.0
        return self._scale

    @scale.setter
    def scale(self, value):
        self._scale = value

    @property
    def qa2_intent(self):
        if self._qa2_intent is None:
            return ''
        return self._qa2_intent

    @qa2_intent.setter
    def qa2_intent(self, value):
        self._qa2_intent = value

    @property
    def qa2_bandpass_intent(self):
        if self._qa2_bandpass_intent is None:
            return '*BANDPASS*'
        return self._qa2_bandpass_intent

    @qa2_bandpass_intent.setter
    def qa2_bandpass_intent(self, value):
        self._qa2_bandpass_intent = value


class Wvrgcal(basetask.StandardTaskTemplate):
    Inputs = WvrgcalInputs

    def __init__(self, inputs):
        super(Wvrgcal, self).__init__(inputs)
    
    def prepare(self):
        inputs = self.inputs
        jobs = []

        # get parameters that can be set from outside or which will be derived
        # from heuristics otherwise
        wvrheuristics = heuwvrgcal.WvrgcalHeuristics(
          context=inputs.context, vis=inputs.vis, hm_tie=inputs.hm_tie, 
          tie=inputs.tie, hm_smooth = inputs.hm_smooth, smooth=inputs.smooth,
          sourceflag=inputs.sourceflag, nsol=inputs.nsol,
          segsource=inputs.segsource)

        if inputs.toffset is None:
            toffset = wvrheuristics.toffset()
        else:
            toffset = inputs.toffset

        if inputs.segsource is None:
            segsource = wvrheuristics.segsource()
        else:
            segsource = inputs.segsource

        if inputs.hm_tie == 'automatic':
            tie = wvrheuristics.tie()
        else:
            tie = inputs.tie

        if inputs.sourceflag is None:
            sourceflag = wvrheuristics.sourceflag()
        else:
            sourceflag = inputs.sourceflag

        if inputs.nsol is None:
            nsol = wvrheuristics.nsol()
        else:
            nsol = inputs.nsol

        # get parameters that must be set from outside
        disperse = inputs.disperse
        wvrflag = inputs.wvrflag
        scale = inputs.scale

        # smooth may vary with spectral window so need to ensure we calculate
        # results that can cover them all
        ms = inputs.context.observing_run.get_ms(name=inputs.vis)
        spws = ms.spectral_windows
        science_spwids = [spw.id for spw in spws
          if spw.channels not in (1,4) and not spw.intents.isdisjoint(
          ['BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET'])]

        smooths_done = set()
        callist = []
        for spw in science_spwids:
            if inputs.hm_smooth == 'automatic':
                smooth = wvrheuristics.smooth(spw)
            else:
                smooth = inputs.smooth

            # prepare to run the wvrgcal task if necessary
            if smooth not in smooths_done:
                # different caltable for each smoothing, remove old versions
                caltable = wcaltable.WvrgCaltable()
                caltable = caltable(output_dir=inputs.output_dir,
                  stage=inputs.context.stage,
                  vis=inputs.vis, smooth=smooth)
                os.system('rm -fr %s' % caltable)

                jobs.append(casa_tasks.wvrgcal(vis=inputs.vis,
                  caltable=caltable, toffset=toffset, segsource=segsource,
                  tie=tie, sourceflag=sourceflag, nsol=nsol,
                  disperse=disperse, wvrflag=wvrflag,
                  smooth=smooth))

                smooths_done.add(smooth)

            # add this wvrg table to the callibrary for this spw
            calto = callibrary.CalTo(vis=inputs.vis, spw=spw)
            calfrom = callibrary.CalFrom(caltable, caltype='wvr', spwmap=[],
              interp='nearest')
            calapp = callibrary.CalApplication(calto, calfrom)
            callist.append(calapp)

        # execute the jobs
        for job in jobs:
            self._executor.execute(job)

        return resultobjects.WvrgcalflagResult(pool=callist,
          wvrflag=inputs.wvrflag)

    def analyse(self, result):
        inputs = self.inputs

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        result.final[:] = on_disk

        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.error.clear()
        result.error.update(missing)

        # calculate the qa2 results if required
        if inputs.qa2_intent.replace(' ','') != '':

            LOG.info('qa2: calculate B and accept into copy of context')
            bandpassinputs = bandpass.PhcorBandpass.Inputs(
              context=inputs.context, output_dir=inputs.output_dir,
              vis=inputs.vis, mode='channel', intent=inputs.qa2_bandpass_intent,
              qa2_intent='')
            bandpasstask = bandpass.PhcorBandpass(bandpassinputs)
            self.bp_results = self._executor.execute(bandpasstask, merge=True)

            # do a phase calibration on the bandpass and phase calibrators 
            # with B preapplied
            LOG.info('qa2: calculating phase calibration with B applied')
            gaincalinputs = gaincal.GTypeGaincal.Inputs(context=inputs.context,
              output_dir=inputs.output_dir, vis=inputs.vis,
              intent=inputs.qa2_intent, solint='int', minsnr=0.0)
            gaincaltask = gaincal.GTypeGaincal(gaincalinputs)
            self.gresults_nowvr = self._executor.execute(gaincaltask, merge=False)

            LOG.info('qa2: calculate phase rms')
            calapplication = self.gresults_nowvr.final[0]
            wvrg_qa2.calculate_view(inputs.context, calapplication.gaintable,
              result.qa2)

            LOG.info('qa2: accept wvr results into copy of context')
            result.accept(inputs.context)

            # do a phase calibration on the bandpass and phase calibrators 
            # with B and wvr preapplied. This will write into the same
            # caltable as the no-wvr version which is why the qa2 nowvr view has
            # to be calculated before this. 
            LOG.info('qa2: calculating phase calibration with B and wvr applied')
            gaincalinputs = gaincal.GTypeGaincal.Inputs(context=inputs.context,
              output_dir=inputs.output_dir, vis=inputs.vis,
              intent=inputs.qa2_intent, solint='int', minsnr=0.0)
            gaincaltask = gaincal.GTypeGaincal(gaincalinputs)
            self.gresults_wvr = self._executor.execute(gaincaltask, merge=False)

            LOG.info('qa2: calculate phase rms')
            calapplication = self.gresults_wvr.final[0]
            wvrg_qa2.calculate_view(inputs.context, calapplication.gaintable,
              result.qa2)

            wvrg_qa2.calculate_qa2_numbers(result.qa2)

            # put a dummy FlagOp in the result structure to fool
            # displays/image.py into plotting the before and after view
            result.qa2.flagging = [arrayflaggerbase.FlagOp(filename=None,
              rulename='ignore', spw=None, axisnames=None, flagcoords=None,
              cell_index=None)]

            # if the qa2 score indicates that applying the wvrg file will
            # make things worse then remove it from the results so that
            # it cannot be accepted into the context.
            if result.qa2.overall_score < 1:
                LOG.warning(
                  'wvrgcal has low qa2 score (%s) and will not be applied' %
                  result.qa2.overall_score)
                result.final = []

        return result

