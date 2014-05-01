from __future__ import absolute_import
import collections
import numpy as np
import os
import shutil
import types

import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks
from pipeline.hif.heuristics import caltable as caltable_heuristic

from pipeline.hifa.heuristics import wvrgcal as wvrgcal_heuristic

from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import gaincal
from . import resultobjects 
from . import wvrg_qa

LOG = infrastructure.get_logger(__name__)

WVRInfo = collections.namedtuple('WVRInfo',
                                 'antenna wvr flag rms disc')


class WvrgcalInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call        
    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
                 hm_toffset=None, toffset=None, segsource=None, hm_tie=None,
                 tie=None, sourceflag=None, nsol=None, disperse=None, 
                 wvrflag=None, hm_smooth=None, smooth=None, scale=None, 
                 maxdistm=None, minnumants=None, mingoodfrac=None,
		 qa_intent=None, qa_bandpass_intent=None,
		 accept_threshold=None, bandpass_result=None,
		 nowvr_result=None):
        self._init_properties(vars())

    @property
    def accept_threshold(self):
        return self._accept_threshold

    @accept_threshold.setter
    def accept_threshold(self, value):
        if value is None:
            value = 1.0
        self._accept_threshold = value
        
    @property
    def disperse(self):
        return self._disperse

    @disperse.setter
    def disperse(self, value):
        if value is None:
            value = False
        self._disperse = value

    @property
    def hm_smooth(self):
        return self._hm_smooth

    @hm_smooth.setter
    def hm_smooth(self, value):
        if value is None:
            value = 'automatic'
        self._hm_smooth = value

    @property
    def hm_tie(self):
        return self._hm_tie

    @hm_tie.setter
    def hm_tie(self, value):
        if value is None:
            value = 'automatic'
        self._hm_tie = value

    @property
    def hm_toffset(self):
        return self._hm_toffset

    @hm_toffset.setter
    def hm_toffset(self, value):
        if value is None:
            value = 'automatic'
        self._hm_toffset = value

    @property
    def maxdistm(self):
        return self._maxdistm

    @maxdistm.setter
    def maxdistm(self, value):
        if value is None:
            value = 500.0
        self._maxdistm = value

    @property
    def minnumants(self):
        return self._minnumants

    @minnumants.setter
    def minnumants(self, value):
        if value is None:
            value = 2
        self._minnumants = value

    @property
    def mingoodfrac(self):
        return self._mingoodfrac

    @mingoodfrac.setter
    def mingoodfrac(self, value):
        if value is None:
            value = 0.8
        self._mingoodfrac = value

    @property
    def nsol(self):
        return self._nsol

    @nsol.setter
    def nsol(self, value):
        if value is None:
            value = 1
        self._nsol = value

    @property
    def qa_bandpass_intent(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('qa_bandpass_intent')

        if self._qa_bandpass_intent is None:
            # default is blank, which allows the bandpass task
            # to select a sensible default if the dataset lacks
            # data with BANDPASS intent
            return ''

        return self._qa_bandpass_intent

    @qa_bandpass_intent.setter
    def qa_bandpass_intent(self, value):
        self._qa_bandpass_intent = value

    @property
    def qa_intent(self):
        return self._qa_intent

    @qa_intent.setter
    def qa_intent(self, value):
        if value is None:
            value = ''
        self._qa_intent = value

    @property
    def scale(self):
        return self._scale

    @scale.setter
    def scale(self, value):
        if value is None:
            value = 1.0
        self._scale = value

    @property
    def segsource(self):
        return self._segsource

    @segsource.setter
    def segsource(self, value):
        if value is None:
            value = True
        self._segsource = value

    @property
    def smooth(self):
        return self._smooth

    @smooth.setter
    def smooth(self, value):
        if value is None:
            value = '1s'
        self._smooth = value

    @property
    def sourceflag(self):
        return self._sourceflag

    @sourceflag.setter
    def sourceflag(self, value):
        if value is None:
            value = []
        self._sourceflag = value

    @property
    def tie(self):
        return self._tie

    @tie.setter
    def tie(self, value):
        if value is None:
            value = []
        self._tie = value

    @property
    def toffset(self):
        return self._toffset

    @toffset.setter
    def toffset(self, value):
        if value is None:
            value = 0
        self._toffset = value

    @property
    def wvrflag(self):
        return self._wvrflag

    @wvrflag.setter
    def wvrflag(self, value):
        if value in (None, ''):
            value = []
        elif type(value) is types.StringType:
            if value[0] == '[':
                value = value.translate(None, '[]\'')
            value = value.split(',')

        self._wvrflag = value


class Wvrgcal(basetask.StandardTaskTemplate):
    Inputs = WvrgcalInputs

    def __init__(self, inputs):
        super(Wvrgcal, self).__init__(inputs)
    
    def prepare(self):
        inputs = self.inputs
        result = resultobjects.WvrgcalResult(vis=inputs.vis)        
        jobs = []

        # get parameters that can be set from outside or which will be derived
        # from heuristics otherwise
        wvrheuristics = wvrgcal_heuristic.WvrgcalHeuristics(
          context=inputs.context, vis=inputs.vis, hm_tie=inputs.hm_tie, 
          tie=inputs.tie, hm_smooth = inputs.hm_smooth, smooth=inputs.smooth,
          sourceflag=inputs.sourceflag, nsol=inputs.nsol,
          segsource=inputs.segsource)

        # return an empty results object if no WVR data available
        if not wvrheuristics.wvr_available():
            # only 12m antennas are expected to have WVRs fitted
            if any([a for a in inputs.ms.antennas if a.diameter == 12.0]):
                LOG.error('WVR data expected but not found in %s'
                          '' % os.path.basename(inputs.vis))
            return result

        if inputs.hm_toffset == 'automatic':
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
        # add tie to results object for display in the weblog. Tie is not spw
        # dependent, so we can add it early.
        result.tie = tie

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
        maxdistm = inputs.maxdistm
        minnumants = inputs.minnumants
        mingoodfrac = inputs.mingoodfrac

        # smooth may vary with spectral window so need to ensure we calculate
        # results that can cover them all
        ms = inputs.context.observing_run.get_ms(name=inputs.vis)
        science_spws = ms.get_spectral_windows(science_windows_only=True)
        science_spwids = [spw.id for spw in science_spws]

        smooths_done = set()
        callist = []
        caltables = []
        for spw in science_spwids:
            if inputs.hm_smooth == 'automatic':
                #smooth = wvrheuristics.smooth(spw)
		# Force the smooth heuristics to a single value
                smooth = wvrheuristics.smoothall(science_spwids)
            else:
                smooth = inputs.smooth

            # prepare to run the wvrgcal task if necessary
            caltable = caltable_heuristic.WvrgCaltable()
            caltable = caltable(output_dir=inputs.output_dir,
                                stage=inputs.context.stage,
                                vis=inputs.vis, smooth=smooth)
            if smooth not in smooths_done:
                # different caltable for each smoothing, remove old versions
                shutil.rmtree(caltable, ignore_errors=True)

                task = casa_tasks.wvrgcal(vis=inputs.vis, caltable=caltable,
                                          toffset=toffset, segsource=segsource,
                                          tie=tie, sourceflag=sourceflag, 
                                          nsol=nsol,disperse=disperse, 
                                          wvrflag=wvrflag, smooth=smooth,
                                          scale=scale, maxdistm=maxdistm,
                                          minnumants=minnumants,
					                      mingoodfrac=mingoodfrac)
                jobs.append(task)

                smooths_done.add(smooth)

            # add this wvrg table to the callibrary for this spw
            calto = callibrary.CalTo(vis=inputs.vis, spw=spw)
            calfrom = callibrary.CalFrom(caltable, caltype='wvr', spwmap=[],
                                         interp='nearest', calwt=False)
            calapp = callibrary.CalApplication(calto, calfrom)
            callist.append(calapp)
            caltables.append(caltable)

        # execute the jobs
        for job in jobs:
            job_result = self._executor.execute(job)

            # update results.wvrflag if necessary
            job_name = np.array(job_result['Name'])
            job_flag = np.array(job_result['Flag'])
            job_wvrflag = set(job_name[job_flag])
            input_wvrflag = set(wvrflag)
            generated_wvrflag = job_wvrflag.difference(input_wvrflag)
            if generated_wvrflag:
                LOG.warn('wvrgcal has flagged antennas: %s' % 
                  list(generated_wvrflag))
         
            wvrflag_set = set(result.wvrflag)
            wvrflag_set.update(job_wvrflag)
            result.wvrflag = list(wvrflag_set)

            result.wvr_infos = self._get_wvrinfos(job_result)
 
        LOG.info('wvrgcal complete')

        # removed any 'unsmoothed' wvrcal tables generated by the wvrgcal jobs
        for caltable in caltables:
            shutil.rmtree('%s_unsmoothed' % caltable, ignore_errors=True)

        result.pool[:] = callist

        return result

    def analyse(self, result):
        inputs = self.inputs

        # check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.error.clear()
        result.error.update(missing)

        # wvrcal files to be applied
        result.final[:] = on_disk

        qa_intent = inputs.qa_intent.strip()
        if not qa_intent:
            return result
        
        # calculate the qa results if required
        if not result.final:
            return result

        # do a bandpass calibration
        result.bandpass_result = self._do_qa_bandpass(inputs)
        
        # do a phase calibration on the bandpass and phase
        # calibrators with B preapplied
        LOG.info('qa: calculating phase calibration with B applied')
        nowvr_result = self._do_nowvr_gaincal(inputs)
        result.nowvr_result = nowvr_result

        # accept this result object, thus adding the WVR table to the 
        # callibrary
        LOG.debug('qa: accept WVR results into copy of context')
        result.accept(inputs.context)

        # do a phase calibration on the bandpass and phase calibrators, now 
        # with B *and* WVR preapplied.
        LOG.info('qa: calculating phase calibration with B and WVR applied')
        wvr_result = self._do_wvr_gaincal(inputs)            

        nowvr_caltable = nowvr_result.inputs['caltable']
        wvr_caltable = wvr_result.inputs['caltable']
        result.qa_wvr.gaintable_wvr = wvr_caltable

        LOG.info('qa: calculate ratio no-WVR phase RMS / with-WVR phase rms')
        wvrg_qa.calculate_view(inputs.context, nowvr_caltable,
                                wvr_caltable, result.qa_wvr)

        wvrg_qa.calculate_qa_numbers(result.qa_wvr)

        # if the qa score indicates that applying the wvrg file will
        # make things worse then remove it from the results so that
        # it cannot be accepted into the context.
        if result.qa_wvr.overall_score < inputs.accept_threshold:
            LOG.warning('wvrgcal has qa score (%s) below accept_threshold '
                        '(%s) and will not be applied' %
                        (result.qa_wvr.overall_score, inputs.accept_threshold))
            result.final[:] = []

        return result

    def _do_qa_bandpass(self, inputs):
        """
        Create a bandpass caltable for QA analysis, returning the result of
        the worker bandpass task.
        
        If a suitable bandpass caltable already exists, it will be reused. 
        """
        if inputs.bandpass_result:
            # table already exists use it
            LOG.info('Reusing B calibration result:\n%s' % 
                     inputs.bandpass_result)
            return self._do_user_qa_bandpass(inputs)
        else:
            LOG.info('Calculating new bandpass for QA analysis')
            result = self._do_new_qa_bandpass(inputs)
            inputs.bandpass_result = result
            return result
        
    def _do_user_qa_bandpass(self, inputs):
        """
        Accept and return the bandpass result affixed to the inputs.
        
        This code path is used as an optimisation, so identical caltables
        need not be recalculated.
        """
        bp_result = inputs.bandpass_result
        bp_result.accept(inputs.context)
        return bp_result
    
    def _do_new_qa_bandpass(self, inputs):
        """
        Create a new bandpass caltable by spawning a bandpass worker task, 
        merging the results with the context.
        """
        # passing an empty string as intent tells bandpass to use all intents,
        # which resolves to all fields. Convert '' to None, which will tell
        # bandpass to use the default bandpass intents, as expected. 
        intent = None if inputs.qa_bandpass_intent is '' else inputs.qa_bandpass_intent
        
        args = {'vis'         : inputs.vis,
                'mode'        : 'channel',
                'intent'      : intent,
                'solint'      : 'inf,7.8125MHz',
                'maxchannels' : 0,
                'qa_intent'  : '',
                'run_qa'     : False}

        inputs = bandpass.PhcorBandpass.Inputs(inputs.context, **args)        
        task = bandpass.PhcorBandpass(inputs)
        result = self._executor.execute(task, merge=True)

        return result
    
    def _do_nowvr_gaincal(self, inputs):
        # do a phase calibration on the bandpass and phase
        # calibrators with B preapplied
        LOG.info('Calculating phase calibration with B applied')

        if inputs.nowvr_result:
            # if table already exists use it
            LOG.debug('Reusing WVR-uncorrected gain result for RMS:\n %s' %
                      inputs.nowvr_result)
            return inputs.nowvr_result
        else:
            LOG.debug('Calculating new gaincal with B but no WVR')
            # get namer that will add '.wvr' to caltable filename 
            nowvr_caltable_namer = self._get_nowvr_caltable_namer(inputs)            
            result = self._do_qa_gaincal(inputs, nowvr_caltable_namer)            
            # cache the no WVR result on the inputs to save us having to
            # recalculate it in future runs
            inputs.nowvr_result = result
            return result
        
    def _do_wvr_gaincal(self, inputs):
        # get namer that will add '.flags_1_2.wvr' to caltable filename 
        wvr_caltable_namer = self._get_wvr_caltable_namer(inputs)            
        return self._do_qa_gaincal(inputs, wvr_caltable_namer)                    

    def _do_qa_gaincal(self, inputs, caltable_namer):
        """
        Generate a new gain caltable via a call to a child pipeline task.
        
        Analysing the improvement gained by applying the WVR requires that
        exactly the same gaincal job is called with and without the WVR 
        preapply. Coding the gaincal as a separate function with minimal
        outside interaction helps enforce that requirement.  
        """
        args = {'vis' : inputs.vis,
                'intent' : inputs.qa_intent,
                'solint' : 'int',
                'calmode' : 'p',
                'minsnr' : 0.0}

        inputs = gaincal.GTypeGaincal.Inputs(inputs.context, **args)

        # give calling code a chance to customise the caltable name via the
        # callback passed as an argument
        inputs.caltable = caltable_namer(inputs.caltable)
        
        task = gaincal.GTypeGaincal(inputs)
        result = self._executor.execute(task, merge=False)

        return result

    def _get_nowvr_caltable_namer(self, inputs):
        """        
        Returns a function that inserts a '.nowvr' component into a filename.
        """
        def caltable_namer(caltable):
            root, ext = os.path.splitext(caltable)
            new_caltable_name = '%s.nowvr%s' % (root, ext)
            LOG.debug('WVR uncorrected phase RMS gain table is %s' %
                      new_caltable_name)
            return new_caltable_name
        
        return caltable_namer
        
    def _get_wvr_caltable_namer(self, inputs):
        """        
        Returns a function that inserts a ''.flags_X_X_X.wvr' component into a
        filename.
        """
        flags = '.flag%s' % '_'.join(inputs.wvrflag) if inputs.wvrflag else ''
        
        def caltable_namer(caltable):
            root, ext = os.path.splitext(caltable)
            new_caltable = '%s%s.wvr%s' % (root, flags, ext)
            LOG.debug('WVR-corrected phase RMS gain table is %s' %
                      os.path.basename(new_caltable))
            return new_caltable
        
        return caltable_namer    

    def _get_wvrinfos(self, result):
        to_microns = lambda x : measures.Distance(x, 
                                                  measures.DistanceUnits.MICROMETRE)
    
        # copy result in case we need it unaltered elsewhere, then convert raw
        # values to domain measures    
        copied = dict(result)
        copied['RMS_um'] = [to_microns(v) for v in copied['RMS_um']]
        copied['Disc_um'] = [to_microns(v) for v in copied['Disc_um']]
    
        attrs = ['Name', 'WVR', 'Flag', 'RMS_um', 'Disc_um']
        zipped = zip(*[copied.get(attr) for attr in attrs])
        wvr_infos = [WVRInfo(*row) for row in zipped]
        
        return wvr_infos
    
