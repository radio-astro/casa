from __future__ import absolute_import
import copy
import re
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

from . import wvrgcal
from . import resultobjects
from . import wvrgcalflagsetter

from pipeline.hif.tasks.common import calibrationtableaccess
from pipeline.hif.tasks.common import viewflaggers

LOG = infrastructure.get_logger(__name__)


class WvrgcalflagInputs(basetask.StandardInputs):
    
    def __init__(self, context, output_dir=None, vis=None,
      caltable=None, hm_toffset=None, toffset=None, segsource=None, 
      hm_tie=None, tie=None, sourceflag=None, nsol=None,
      disperse=None, wvrflag=None, hm_smooth=None, smooth=None,
      scale=None, flag_intent=None, qa2_intent=None,
      qa2_bandpass_intent=None, flag_hi=None, fhi_limit=None,
      fhi_minsample=None):
	self._init_properties(vars())

    @property
    def hm_toffset(self):
        if self._hm_toffset is None:
            return 'automatic'
        return self._hm_toffset

    @hm_toffset.setter
    def hm_toffset(self, value):
        self._hm_toffset = value

    @property
    def toffset(self):
        if self._toffset is None:
            return 0
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
        return self._wvrflag

    @wvrflag.setter
    def wvrflag(self, value):
        if value is None:
            self._wvrflag = []
        elif type(value) is types.StringType:
            if value == '':
                self._wvrflag = []
            else:
                if value[0] == '[':
                    strvalue=value.replace('[','').replace(']','').replace("'","")
                else:
                    strvalue = value
                self._wvrflag = list(strvalue.split(','))
        else:
            self._wvrflag = value

    @property
    def hm_smooth(self):
        if self._hm_smooth is None:
            return 'automatic'
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
    def flag_intent(self):
        if self._flag_intent is None:
            return '*BANDPASS*'
        return self._flag_intent

    @flag_intent.setter
    def flag_intent(self, value):
        self._flag_intent = value
    @property

    def qa2_intent(self):
        if self._qa2_intent is None:
            value = '*PHASE*,*BANDPASS*'
        else:
            value = self._qa2_intent
        # ensure that qa2_intent includes flag_intent otherwise 
        # the results for flag_intent will not be calculated
        value_set = set(value.split(','))
        value_set.update(self.flag_intent.split(','))
        value = ','.join(value_set)

        return value

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

    # flag high outlier
    @property
    def flag_hi(self):
        if self._flag_hi is None:
            return True
        return self._flag_hi

    @flag_hi.setter
    def flag_hi(self, value):
        self._flag_hi = value

    @property
    def fhi_limit(self):
        if self._fhi_limit is None:
            return 10
        return self._fhi_limit

    @fhi_limit.setter
    def fhi_limit(self, value):
        self._fhi_limit = value
    @property

    def fhi_minsample(self):
        if self._fhi_minsample is None:
            return 5
        return self._fhi_minsample

    @fhi_minsample.setter
    def fhi_minsample(self, value):
        self._fhi_minsample = value


class Wvrgcalflag(basetask.StandardTaskTemplate):
    Inputs = WvrgcalflagInputs
    
    def prepare(self):
        inputs = self.inputs
        jobs = []

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
          scale=inputs.scale, flag_intent=inputs.flag_intent,
          qa2_intent=inputs.qa2_intent,
          qa2_bandpass_intent=inputs.qa2_bandpass_intent)
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

        # Execute it to flag the data view
        result = self._executor.execute(flaggertask)

        return result

    def analyse(self, result):
        return result


class WvrgcalflagWorkerInputs(basetask.StandardInputs):
    
    def __init__(self, context, output_dir=None, vis=None,
      caltable=None, hm_toffset=None, toffset=None, segsource=None, 
      hm_tie=None, tie=None, sourceflag=None, nsol=None,
      disperse=None, wvrflag=None, hm_smooth=None, smooth=None,
      scale=None, flag_intent=None, qa2_intent=None,
      qa2_bandpass_intent=None):
	self._init_properties(vars())


class WvrgcalflagWorker(basetask.StandardTaskTemplate):
    Inputs = WvrgcalflagWorkerInputs

    def __init__(self, inputs):
        super(WvrgcalflagWorker, self).__init__(inputs)
        # we need a persistent result object when flagging as it will contain
        # results at each flagging iteration
        self.result = resultobjects.WvrgcalflagResult()
    
    def prepare(self):
        inputs = self.inputs
        jobs = []

        # calculate the wvrgcal
        wvrgcalinputs = wvrgcal.Wvrgcal.Inputs(context=inputs.context,
          output_dir=inputs.output_dir, vis=inputs.vis,
          hm_toffset=inputs.hm_toffset, toffset=inputs.toffset,
          hm_tie=inputs.hm_tie, tie=inputs.tie,
          sourceflag=inputs.sourceflag, nsol=inputs.nsol,
          disperse=inputs.disperse, wvrflag=inputs.wvrflag,
          hm_smooth=inputs.hm_smooth, smooth=inputs.smooth, scale=inputs.scale,
          qa2_intent=inputs.qa2_intent,
          qa2_bandpass_intent=inputs.qa2_bandpass_intent)
        wvrgcaltask = wvrgcal.Wvrgcal(wvrgcalinputs)
        result = self._executor.execute(wvrgcaltask, merge=True)

        return result

    def analyse(self, result):
        inputs = self.inputs

        # if there is a valid wvrgcal result
        if result.final:
            # copy the views for the flag_intent from the QA2 section of the
            # wvrgcal result
            ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)
            fields = ms.fields
            intent_list = self.inputs.flag_intent.split(',')
            intent_fields = set()
            for intent in intent_list:
                re_intent = intent.replace('*', '.*')
                intent_fields.update(
                  [fld.name for fld in fields if
                  re.search(pattern=re_intent, string=str(fld.intents))])
            intent_fields = list(intent_fields)

            for description in result.qa2.descriptions():
                add = False
                for intent_field in intent_fields:
                    if 'Field:%s' % intent_field in description:
                        add = True
                        break
                
                if add:
                    self.result.addview(description, result.qa2.last(description))
 
        # copy over other info from wvrgcal result
        self.result.final = result.final 
        self.result.vis = result.qa2.vis
        self.result.qa2 = result.qa2
        self.result.wvrflag = result.wvrflag

        return copy.deepcopy(self.result)
