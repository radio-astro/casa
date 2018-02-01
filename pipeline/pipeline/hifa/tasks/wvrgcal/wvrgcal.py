from __future__ import absolute_import

import collections
import os
import shutil

import numpy as np

import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as caltable_heuristic
from pipeline.hif.tasks import gaincal
from pipeline.hifa.heuristics import atm as atm_heuristic
from pipeline.hifa.heuristics import wvrgcal as wvrgcal_heuristic
from pipeline.hifa.tasks import bandpass
from pipeline.infrastructure import casa_tasks, task_registry
from . import resultobjects
from . import wvrg_qa

__all__ = [
    'Wvrgcal',
    'WvrgcalInputs'
]

LOG = infrastructure.get_logger(__name__)

WVRInfo = collections.namedtuple('WVRInfo',
                                 'antenna wvr flag rms disc')


class WvrgcalInputs(vdp.StandardInputs):
    """
    WvrgcalInputs defines the inputs for the Wvrgcal pipeline task.
    """
    accept_threshold = vdp.VisDependentProperty(default=1.0)
    bandpass_result = vdp.VisDependentProperty(default='', hidden=True)
    disperse = vdp.VisDependentProperty(default=False)
    hm_smooth = vdp.VisDependentProperty(default='automatic')
    hm_tie = vdp.VisDependentProperty(default='automatic')
    hm_toffset = vdp.VisDependentProperty(default='automatic')

    @vdp.VisDependentProperty
    def maxdistm(self):
        # Identify fraction of antennas that are 7m.
        nants_7m = len([ant
                        for ant in self.ms.antennas
                        if ant.diameter == 7.0])
        frac = float(nants_7m) / len(self.ms.antennas)

        # If more than 50% of the antennas are 7m, then set maxdistm to a
        # lower value.
        if frac > 0.5:
            return 100.0
        else:
            return 500.0

    minnumants = vdp.VisDependentProperty(default=2)
    mingoodfrac = vdp.VisDependentProperty(default=0.8)
    nowvr_result = vdp.VisDependentProperty(default='', hidden=True)
    nsol = vdp.VisDependentProperty(default=1)
    # Default for offsetstable is blank, which means the wvrgcal task will not
    # apply any offsets.
    offsetstable = vdp.VisDependentProperty(default='')
    refant = vdp.VisDependentProperty(default='')
    # Default for qa_bandpass_intent is blank, which allows the bandpass task
    # to select a sensible default if the dataset lacks data with BANDPASS
    # intent.
    qa_bandpass_intent = vdp.VisDependentProperty(default='')
    qa_intent = vdp.VisDependentProperty(default='')
    qa_spw = vdp.VisDependentProperty(default='', hidden=True)
    scale = vdp.VisDependentProperty(default=1.0)
    segsource = vdp.VisDependentProperty(default=True)
    smooth = vdp.VisDependentProperty(default='')

    @vdp.VisDependentProperty
    def sourceflag(self):
        return []

    @vdp.VisDependentProperty
    def tie(self):
        return []

    toffset = vdp.VisDependentProperty(default=0)

    @vdp.VisDependentProperty
    def wvrflag(self):
        return []

    @wvrflag.convert
    def wvrflag(self, value):
        if isinstance(value, str):
            if value[0] == '[':
                value = value.translate(None, '[]\'')
            return value.split(',')
        else:
            return value

    def __init__(self, context, output_dir=None, vis=None, caltable=None, offsetstable=None, hm_toffset=None,
                 toffset=None, segsource=None, hm_tie=None, tie=None, sourceflag=None, nsol=None, disperse=None,
                 wvrflag=None, hm_smooth=None, smooth=None, scale=None, maxdistm=None, minnumants=None,
                 mingoodfrac=None, refant=None, qa_intent=None, qa_bandpass_intent=None, qa_spw=None,
                 accept_threshold=None, bandpass_result=None, nowvr_result=None):
        super(WvrgcalInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.caltable = caltable
        self.offsetstable = offsetstable

        # solution parameters
        self.hm_toffset = hm_toffset
        self.toffset = toffset
        self.segsource = segsource
        self.hm_tie = hm_tie
        self.tie = tie
        self.sourceflag = sourceflag
        self.nsol = nsol
        self.disperse = disperse
        self.wvrflag = wvrflag
        self.hm_smooth = hm_smooth
        self.smooth = smooth
        self.scale = scale
        self.maxdistm = maxdistm
        self.minnumants = minnumants
        self.mingoodfrac = mingoodfrac
        self.refant = refant
        self.qa_intent = qa_intent
        self.qa_bandpass_intent = qa_bandpass_intent
        self.qa_spw = qa_spw
        self.accept_threshold = accept_threshold
        self.bandpass_result = bandpass_result
        self.nowvr_result = nowvr_result


@task_registry.set_equivalent_casa_task('hifa_wvrgcal')
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
            tie=inputs.tie, hm_smooth=inputs.hm_smooth, smooth=inputs.smooth,
            sourceflag=inputs.sourceflag, nsol=inputs.nsol,
            segsource=inputs.segsource)
        
        # return an empty results object if no WVR data available
        if not wvrheuristics.wvr_available():
            # only 12m antennas are expected to have WVRs fitted
            if any([a for a in inputs.ms.antennas if a.diameter == 12.0]):
                LOG.error('WVR data expected but not found in %s'
                          '' % os.path.basename(inputs.vis))
            return result
        elif len([a for a in inputs.ms.antennas if a.diameter == 12.0]) <= 1:
            # WVR data available for only 1 antenna
            LOG.error('WVR data available for only 1 antenna in %s'
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

        ms = inputs.context.observing_run.get_ms(name=inputs.vis)

        # Set the reference antenna
        refant = inputs.refant
        if refant == '':
            refant = ms.reference_antenna
            if not (refant and refant.strip()):
                msg = ('No reference antenna specified and none found in '
                       'context for %s' % ms.basename)
                LOG.error(msg)
                refant = []
            else:
                refant = refant.split(',')
        
        # smooth may vary with spectral window so need to ensure we calculate
        # results that can cover them all
        science_spws = ms.get_spectral_windows(science_windows_only=True)
        science_spwids = [spw.id for spw in science_spws]
        wvr_spwids = sorted([spw.id for spw in ms.spectral_windows if
                             spw.num_channels == 4 and 'WVR' in spw.intents])

        smooths_done = set()
        callist = []
        caltables = []
        for spw in science_spwids:
            if inputs.hm_smooth == 'automatic':
                # smooth = wvrheuristics.smooth(spw)
                # Force the smooth heuristics to a single value
                # (If integration times vary between spws this may not be
                # the right thing to do). Integration times varying between
                # intents within an spw then it is right to smooth according
                # to the longest.
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
                                          offsetstable=inputs.offsetstable,
                                          toffset=toffset, segsource=segsource,
                                          tie=tie, sourceflag=sourceflag, 
                                          nsol=nsol, disperse=disperse,
                                          wvrflag=wvrflag, smooth=smooth,
                                          scale=scale, maxdistm=maxdistm,
                                          minnumants=minnumants,
                                          mingoodfrac=mingoodfrac,
                                          spw=science_spwids,
                                          wvrspw=[wvr_spwids[0]],
                                          refant=refant)
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
            
            if job_result['success']:
                # extract flags found by CASA job
                job_name = np.array(job_result['Name'])
                job_flag = np.array(job_result['Flag'])
                job_wvrflag = set(job_name[job_flag])
            else:
                LOG.warn('CASA wvrgcal job terminated unexpectedly with '
                         'exit code %s; no flags generated.' % (job_result['rval']))
                job_wvrflag = set([])
            
            input_wvrflag = set(wvrflag)
            generated_wvrflag = job_wvrflag.difference(input_wvrflag)
            if generated_wvrflag:
                LOG.warn('%s wvrgcal has flagged antennas: %s' % (
                  os.path.basename(inputs.vis), list(generated_wvrflag)))
         
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
        
        # get the spw for which qa results are needed
        # If no spw order was specified explicitly, then calculate an order
        # here; otherwise use the specified order (e.g. calculated during
        # previous iteration).
        if inputs.qa_spw == '':
            atmheuristics = atm_heuristic.AtmHeuristics(
                context=inputs.context, vis=inputs.vis)
            # Preferably rank spws by Tsys and bandwidth:
            qa_spw_list = atmheuristics.spwid_rank_by_tsys_and_bandwidth(qa_intent)
            # If ranking by Tsys failed (e.g. no Tsys table, or due to
            # flagging), then fall back to ranking by opacity and bandwidth:
            if qa_spw_list is None:
                LOG.info("qa: ranking spws by bandwidth and Tsys failed for "
                         "{}; will rank by bandwidth and opacity "
                         "instead.".format(os.path.basename(inputs.vis)))
                qa_spw_list = atmheuristics.spwid_rank_by_opacity_and_bandwidth()
        else:
            qa_spw_list = inputs.qa_spw.split(',')
        
        for qa_spw in qa_spw_list:
            LOG.info('qa: %s attempting to calculate wvrgcal QA using spw %s' % (os.path.basename(inputs.vis), qa_spw))
            inputs.qa_spw = qa_spw

            # Do a bandpass calibration
            LOG.info('qa: calculating bandpass calibration')
            bp_result = self._do_qa_bandpass(inputs)
            # Do a gain calibration
            if not bp_result.final:
                LOG.warning('qa: calculating phase calibration without bandpass applied')
            else:
                LOG.info('qa: calculating phase calibration with bandpass applied')
            nowvr_result = self._do_nowvr_gaincal(inputs)
            if not nowvr_result.final:
                continue

            inputs.bandpass_result = bp_result
            inputs.nowvr_result = nowvr_result
            result.qa_wvr.bandpass_result = bp_result
            result.qa_wvr.nowvr_result = nowvr_result
            result.qa_wvr.qa_spw = qa_spw
            LOG.info('qa: wvrgcal QA calculation was successful')
            break

        # accept this result object, thus adding the WVR table to the 
        # callibrary
        LOG.debug('qa: accept WVR results into copy of context')
        result.accept(inputs.context)
        
        # do a phase calibration on the bandpass and phase calibrators, now 
        # with bandpas *and* wvr preapplied.
        if not bp_result.final:
            LOG.warning('qa: calculating phase calibration with wvr applied')
        else:
            LOG.info('qa: calculating phase calibration with bandpass and wvr applied')
        wvr_result = self._do_wvr_gaincal(inputs)            
        
        nowvr_caltable = nowvr_result.inputs['caltable']
        wvr_caltable = wvr_result.inputs['caltable']
        result.qa_wvr.gaintable_wvr = wvr_caltable
        
        LOG.info('qa: calculate ratio no-WVR phase RMS / with-WVR phase rms')
        wvrg_qa.calculate_view(inputs.context, nowvr_caltable,
                               wvr_caltable, result.qa_wvr, qa_intent)
        
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
            return result
    
    @staticmethod
    def _do_user_qa_bandpass(inputs):
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
        # which resolves to all fields. Convert empty/blank string to None,
        # which will tell bandpass to use the default bandpass intents, as
        # expected.
        if inputs.qa_bandpass_intent and inputs.qa_bandpass_intent.strip():
            # qa_bandpass_intent is not None and is not empty/blank string.
            intent = inputs.qa_bandpass_intent
        else:
            intent = None

        args = {'vis': inputs.vis,
                'mode': 'channel',
                'intent': intent,
                'spw': inputs.qa_spw,
                'hm_phaseup': 'manual',
                'hm_bandpass': 'fixed',
                'solint': 'inf,7.8125MHz'}
        
        inputs = bandpass.ALMAPhcorBandpass.Inputs(inputs.context, **args)        
        task = bandpass.ALMAPhcorBandpass(inputs)
        result = self._executor.execute(task, merge=False)
        if not result.final:
            pass
        else:
            result.accept(inputs.context)
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
            nowvr_caltable_namer = self._get_nowvr_caltable_namer()
            result = self._do_qa_gaincal(inputs, nowvr_caltable_namer)            
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
        args = {'vis': inputs.vis,
                'intent': inputs.qa_intent,
                'spw': inputs.qa_spw,
                'solint': 'int',
                'calmode': 'p',
                'minsnr': 0.0}
        
        inputs = gaincal.GTypeGaincal.Inputs(inputs.context, **args)
        
        # give calling code a chance to customise the caltable name via the
        # callback passed as an argument
        inputs.caltable = caltable_namer(inputs.caltable)
        
        task = gaincal.GTypeGaincal(inputs)
        result = self._executor.execute(task, merge=False)
        
        return result

    @staticmethod
    def _get_nowvr_caltable_namer():
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
    
    @staticmethod
    def _get_wvr_caltable_namer(inputs):
        """        
        Returns a function that inserts a ''.flagged_<N>_antennas.wvr' component into a
        filename.
        """
        flags = '.flagged_%d_antennas' % len(inputs.wvrflag) if inputs.wvrflag else ''
        
        def caltable_namer(caltable):
            root, ext = os.path.splitext(caltable)
            new_caltable = '%s%s.wvr%s' % (root, flags, ext)
            LOG.debug('WVR-corrected phase RMS gain table is %s' %
                      os.path.basename(new_caltable))
            return new_caltable
        
        return caltable_namer    

    @staticmethod
    def _get_wvrinfos(result):

        def to_microns(x):
            return measures.Distance(x, measures.DistanceUnits.MICROMETRE)

        # copy result in case we need it unaltered elsewhere, then convert raw
        # values to domain measures    
        copied = dict(result)
        copied['RMS_um'] = [to_microns(v) for v in copied['RMS_um']]
        copied['Disc_um'] = [to_microns(v) for v in copied['Disc_um']]
        
        attrs = ['Name', 'WVR', 'Flag', 'RMS_um', 'Disc_um']
        zipped = zip(*[copied.get(attr) for attr in attrs])
        wvr_infos = [WVRInfo(*row) for row in zipped]
        
        return wvr_infos
