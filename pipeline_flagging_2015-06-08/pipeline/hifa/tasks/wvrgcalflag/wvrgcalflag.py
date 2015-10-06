from __future__ import absolute_import
import copy
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging

from pipeline.hifa.tasks import bandpass
from ..wvrgcal import wvrgcal
from . import resultobjects
from . import wvrgcalflagsetter

from pipeline.hif.tasks.common import commonhelpermethods
from pipeline.hif.tasks.common import viewflaggers

LOG = infrastructure.get_logger(__name__)


class WvrgcalflagInputs(wvrgcal.WvrgcalInputs):
    
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
      caltable=None, hm_toffset=None, toffset=None, segsource=None, 
      hm_tie=None, tie=None, sourceflag=None, nsol=None,
      disperse=None, wvrflag=None, hm_smooth=None, smooth=None,
      scale=None, maxdistm=None, minnumants=None, mingoodfrac=None,
      flag_intent=None, qa_intent=None, qa_bandpass_intent=None,
      accept_threshold=None, flag_hi=None, fhi_limit=None, fhi_minsample=None):
        self._init_properties(vars())

    # qa_intent setter/getter overrides version in WvrgcalInputs with a 
    # different default 
    @property
    def qa_intent(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('qa_intent')

        if self._qa_intent is None:
            value = 'PHASE'
        else:
            value = self._qa_intent

        # ensure that qa_intent includes flag_intent otherwise 
        # the results for flag_intent will not be calculated
        value_set = set(value.split(','))
        temp = self.flag_intent
        value_set.update(temp.split(','))
        value = ','.join(value_set)
        return value

    @qa_intent.setter
    def qa_intent(self, value):
        self._qa_intent = value

    @property
    def flag_intent(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('flag_intent')

        # This is a somewhat complicated way to get the default
        # intent.
        if self._flag_intent is None:
            # default to the intent that would be used for bandpass
            # calibration
            bp_inputs = bandpass.ALMAPhcorBandpass.Inputs(
              context=self.context,
              vis=self.vis,
              intent=None)
            value = bp_inputs.intent
            return value

        return self._flag_intent

    @flag_intent.setter
    def flag_intent(self, value):
        self._flag_intent = value
 
    # flag high outlier
    @property
    def flag_hi(self):
        return self._flag_hi

    @flag_hi.setter
    def flag_hi(self, value):
        if value is None:
            value = True
        self._flag_hi = value

    @property
    def fhi_limit(self):
        return self._fhi_limit

    @fhi_limit.setter
    def fhi_limit(self, value):
        if value is None:
            value = 10
        self._fhi_limit = value

    @property
    def fhi_minsample(self):
        return self._fhi_minsample

    @fhi_minsample.setter
    def fhi_minsample(self, value):
        if value is None:
            value = 5
        self._fhi_minsample = value


class Wvrgcalflag(basetask.StandardTaskTemplate):
    Inputs = WvrgcalflagInputs
    
    def prepare(self):
        inputs = self.inputs

        # scratch area where hif_wvrgcal can store its B calibration 
        # result
        inputs.context.scratch = {}

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = WvrgcalflagWorker.Inputs(context=inputs.context,
          output_dir=inputs.output_dir, vis=inputs.vis,
          caltable=inputs.caltable, hm_toffset=inputs.hm_toffset,
          toffset=inputs.toffset,
          segsource=inputs.segsource, hm_tie=inputs.hm_tie,
          tie=inputs.tie, sourceflag=inputs.sourceflag, nsol=inputs.nsol,
          disperse=inputs.disperse, wvrflag=inputs.wvrflag, 
          hm_smooth=inputs.hm_smooth, smooth=inputs.smooth,
          scale=inputs.scale, maxdistm=inputs.maxdistm,
          minnumants=inputs.minnumants, mingoodfrac=inputs.mingoodfrac,
          flag_intent=inputs.flag_intent, qa_intent=inputs.qa_intent,
          qa_bandpass_intent=inputs.qa_bandpass_intent)
        datatask = WvrgcalflagWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = wvrgcalflagsetter.WvrgcalFlagSetterInputs(
          context=inputs.context, vis=inputs.vis, wvrgcaltask=datatask)
        flagsettertask = wvrgcalflagsetter.WvrgcalFlagSetter(flagsetterinputs)

        # Translate the input flagging parameters to a more compact
        # list of rules.
        rules = viewflaggers.MatrixFlagger.make_flag_rules(
          flag_hi=inputs.flag_hi,
          fhi_limit=inputs.fhi_limit, 
          fhi_minsample=inputs.fhi_minsample)

        # Construct the flagger task around the data view task  and the
        # flagger task. When executed this will:
        #   loop:
        #     execute datatask to obtain view from underlying data
        #     examine view, raise flags
        #     execute flagsetter task to set flags in underlying data        
        #     exit loop if no flags raised or if # iterations > niter 
        matrixflaggerinputs = viewflaggers.MatrixFlaggerInputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, flagsettertask=flagsettertask,
          rules=rules, niter=1)
        flaggertask = viewflaggers.MatrixFlagger(matrixflaggerinputs)

        # Wrap the child task in a SuspendCapturingLogger so that warnings
        # emitted by the child task do not make it to the web log page.
        # See CAS-7795.
        with logging.SuspendCapturingLogger():
            # Execute it to flag the data view
            result = self._executor.execute(flaggertask)

        return result

    def analyse(self, result):
        inputs = self.inputs

        # the result should now contain a wvrg file that has been
        # 'flagged' as necessary. If the associated qa score indicates 
        # that applying it will make things worse then remove the file from
        # the result so that it cannot be accepted into the context.
        if result.qa_wvr.overall_score is not None and \
          result.qa_wvr.overall_score < inputs.accept_threshold:
            LOG.warning(
              'wvrgcal file has qa score (%s) below accept_threshold (%s) and will not be applied' %
              (result.qa_wvr.overall_score, inputs.accept_threshold))
            result.final = []

        return result


class WvrgcalflagWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None,
      caltable=None, hm_toffset=None, toffset=None, segsource=None, 
      hm_tie=None, tie=None, sourceflag=None, nsol=None,
      disperse=None, wvrflag=None, hm_smooth=None, smooth=None,
      scale=None, maxdistm=None, minnumants=None, mingoodfrac=None,
      flag_intent=None, qa_intent=None, qa_bandpass_intent=None):
        self._init_properties(vars())


class WvrgcalflagWorker(basetask.StandardTaskTemplate):
    Inputs = WvrgcalflagWorkerInputs

    def __init__(self, inputs):
        super(WvrgcalflagWorker, self).__init__(inputs)
        # we need a persistent result object when flagging as it will contain
        # results at each flagging iteration
        self.result = resultobjects.WvrgcalflagResult(
          vis=inputs.vis)
        self.result.bandpass_result = None
        self.result.nowvr_result = None
        self.result.qa_spw = ''
    
    def prepare(self):
        inputs = self.inputs

        # calculate the wvrgcal with low value for accept_threshold to
        # ensure that the wvrgcal is always accepted into the context.
        wvrgcalinputs = wvrgcal.Wvrgcal.Inputs(context=inputs.context,
          output_dir=inputs.output_dir, vis=inputs.vis,
          hm_toffset=inputs.hm_toffset, toffset=inputs.toffset,
          hm_tie=inputs.hm_tie, tie=inputs.tie,
          sourceflag=inputs.sourceflag, nsol=inputs.nsol,
          disperse=inputs.disperse, wvrflag=inputs.wvrflag,
          hm_smooth=inputs.hm_smooth, smooth=inputs.smooth, scale=inputs.scale,
          maxdistm=inputs.maxdistm, minnumants=inputs.minnumants,
          mingoodfrac=inputs.mingoodfrac, qa_intent=inputs.qa_intent,
          qa_bandpass_intent=inputs.qa_bandpass_intent,
          accept_threshold=0.0,
          bandpass_result=self.result.bandpass_result,
          nowvr_result=self.result.nowvr_result,
          qa_spw=self.result.qa_spw)
        wvrgcaltask = wvrgcal.Wvrgcal(wvrgcalinputs)
        result = self._executor.execute(wvrgcaltask, merge=True)

        # cache bandpass and nowvr results for next call to wvrgcal
        self.result.bandpass_result = result.qa_wvr.bandpass_result
        self.result.nowvr_result = result.qa_wvr.nowvr_result
        self.result.qa_spw = result.qa_wvr.qa_spw

        return result

    def analyse(self, result):
        inputs = self.inputs

        # take note of any antennas flagged by wvrgcal itself
        self.result.wvrflag = result.wvrflag

        # get the intents from the MS
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)
        ms_intent_list = ms.intents

        # test if the flagging intents are present in the MS
        flag_intent_list = self.inputs.flag_intent.split(',')
        for flag_intent in flag_intent_list:
            intent_available = [intent for intent in ms_intent_list if flag_intent in intent]
            if intent_available:
                LOG.info('flagging views will use %s data' % flag_intent)
            else:
                LOG.warning('no data for intent %s' % flag_intent)
        if not intent_available:
            LOG.warning('no data fits flag_intent %s, no flagging will be done' %
              self.inputs.flag_intent)

        # from the QA section of the wvrgcal result, copy those views for which 
        # the intent matches the flag_intent 
        for description in result.qa_wvr.descriptions():
            for flag_intent in flag_intent_list:
                if ('Intent:%s' % flag_intent in description):
                    # set antennas specified by the wvrflag parameter in the 
                    # flagging image to show that these data are already 'flagged'
                    # (i.e. interpolated)
                    image = result.qa_wvr.last(description)
                    ant_names, ant_ids = commonhelpermethods.get_antenna_names(ms)
                    wvrflagids = []
                    for ant_name in self.result.wvrflag:
                        ant_id = [id for id in ant_names if 
                                  ant_names[id]==ant_name]
                        wvrflagids += ant_id
                    image.setflags(axisname='Antenna', indices=wvrflagids)
                    self.result.addview(description, image)
 
        # populate other parts of result
        self.result.pool[:] = result.pool[:]
        self.result.final = result.final 
        self.result.vis = inputs.vis
        self.result.qa_wvr = result.qa_wvr
        self.result.wvrflag = result.wvrflag
        self.result.wvr_infos = getattr(result, 'wvr_infos', [])

        return copy.deepcopy(self.result)
