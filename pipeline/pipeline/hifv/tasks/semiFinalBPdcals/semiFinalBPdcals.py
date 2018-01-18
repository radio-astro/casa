from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure as infrastructure
from pipeline.infrastructure import casa_tasks

import numpy as np

from pipeline.hif.tasks import gaincal
from pipeline.hifv.heuristics import getCalFlaggedSoln
import pipeline.hif.heuristics.findrefant as findrefant
from pipeline.hifv.heuristics import weakbp, do_bandpass

LOG = infrastructure.get_logger(__name__)


class semiFinalBPdcalsInputs(vdp.StandardInputs):
    weakbp = vdp.VisDependentProperty(default=False)
    refantignore = vdp.VisDependentProperty(default='')

    def __init__(self, context, vis=None, weakbp=None, refantignore=None):
        super(semiFinalBPdcalsInputs, self).__init__()
        self.context = context
        self.vis = vis
        self._weakbp = weakbp
        self.refantignore = refantignore


class semiFinalBPdcalsResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None, bpdgain_touse=None):

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []

        super(semiFinalBPdcalsResults, self).__init__()

        # self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.bpdgain_touse = bpdgain_touse


class semiFinalBPdcals(basetask.StandardTaskTemplate):
    Inputs = semiFinalBPdcalsInputs
    
    def prepare(self):

        self.parang = True
        
        gtypecaltable = 'semiFinaldelayinitialgain.g'
        ktypecaltable = 'delay.k'
        bpcaltable = 'BPcal.b'
        tablebase = 'BPinitialgain'
        table_suffix = ['.g','3.g','10.g']
        soltimes = [1.0,3.0,10.0] 
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        #soltimes = [self.inputs.context.evla['msinfo'][m.name].int_time * x for x in soltimes]
        soltimes = [m.get_vla_max_integration_time() * x for x in soltimes]
        solints = ['int', '3.0s', '10.0s']
        soltime = soltimes[0]
        solint = solints[0]
        
        context = self.inputs.context
        refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis=self.inputs.vis, field=refantfield,
                                                geometry=True, flagging=True, intent='',
                                                spw='', refantignore=self.inputs.refantignore)
        
        RefAntOutput=refantobj.calculate()
        
        
        gtype_delaycal_result = self._do_gtype_delaycal(caltable=gtypecaltable, context=context, RefAntOutput=RefAntOutput)
        
        

        fracFlaggedSolns = 1.0
        
        # critfrac = context.evla['msinfo'][m.name].critfrac
        critfrac = m.get_vla_critfrac()

        # Iterate and check the fraciton of Flagged solutions, each time running gaincal in 'K' mode
        flagcount=0
        while (fracFlaggedSolns > critfrac and flagcount < 4):
            
            #context = self.inputs.context
            
            #calto = callibrary.CalTo(self.inputs.vis)
            #calfrom = callibrary.CalFrom(gaintable=gtypecaltable, interp='', calwt=False)
            #context.callibrary._remove(context.callibrary._active, calfrom)
                
            ktype_delaycal_result = self._do_ktype_delaycal(caltable=ktypecaltable, addcaltable=gtypecaltable, context=context, RefAntOutput=RefAntOutput)
            flaggedSolnResult = getCalFlaggedSoln(ktypecaltable)
            fracFlaggedSolns = self._check_flagSolns(flaggedSolnResult, RefAntOutput)
            
            LOG.info("Fraction of flagged solutions = "+str(flaggedSolnResult['all']['fraction']))
            LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult['antmedian']['fraction']))
            flagcount += 1

            # try:
            #     calto = callibrary.CalTo(self.inputs.vis)
            #     calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
            #     context.callibrary._remove(context.callibrary._active, calfrom)
            # except:
            #     LOG.info(ktypecaltable + " does not exist in the context callibrary, and does not need to be removed.")

        LOG.info("Delay calibration complete")

        #calto = callibrary.CalTo(self.inputs.vis)
        #calfrom = callibrary.CalFrom(gaintable=gtypecaltable, interp='', calwt=False)
        #context.callibrary._remove(context.callibrary._active, calfrom)
        
        #context = self.inputs.context

        # Do initial gaincal on BP calibrator then semi-final BP calibration
        gain_solint1 = context.evla['msinfo'][m.name].gain_solint1
        gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[0], addcaltable=ktypecaltable,
                                                       solint=gain_solint1, context=context, RefAntOutput=RefAntOutput)
        
        # Remove temporary caltables
        #calto = callibrary.CalTo(self.inputs.vis)
        #calfrom = callibrary.CalFrom(gaintable=tablebase + table_suffix[0], interp='', calwt=False)
        #context.callibrary._remove(context.callibrary._active, calfrom)
        
        #calto = callibrary.CalTo(self.inputs.vis)
        #calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
        #context.callibrary._remove(context.callibrary._active, calfrom)
        
        #context = self.inputs.context
        bpdgain_touse = tablebase + table_suffix[0]
        
        # Add appropriate temporary tables to the callibrary
        #calto = callibrary.CalTo(self.inputs.vis)
        #calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
        #context.callibrary.add(calto, calfrom)
        
        #calto = callibrary.CalTo(self.inputs.vis)
        #calfrom = callibrary.CalFrom(gaintable=bpdgain_touse, interp='', calwt=False)
        #context.callibrary.add(calto, calfrom)

        #LOG.info("WEAKBP: "+str(self.inputs.weakbp))

        if (self.inputs.weakbp == True):
            #LOG.info("USING WEAKBP HEURISTICS")
            interp = weakbp(self.inputs.vis, bpcaltable, context=context, RefAntOutput=RefAntOutput[0],
                                            ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse, solint='inf', append=False)
        else:
            #LOG.info("Using REGULAR heuristics")
            interp = ''
            bandpass_job = do_bandpass(self.inputs.vis, bpcaltable, context=context, RefAntOutput=RefAntOutput[0], spw='',
                                            ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse, solint='inf', append=False)
            self._executor.execute(bandpass_job)

            AllCalTables = sorted(self.inputs.context.callibrary.active.get_caltable())
            AllCalTables.append(ktypecaltable)
            # AllCalTables.append(bpdgain_touse)
            AllCalTables.append(bpcaltable)
            ntables = len(AllCalTables)
            interp = [''] * ntables
            LOG.info("Using 'linear,nearestflag' for bandpass table")
            interp[-1] = 'linear,nearestflag'


        #self._executor.execute(bandpass_job)
        
        # Force calwt for the bp table to be False
        #calto = callibrary.CalTo(self.inputs.vis)
        #calfrom = callibrary.CalFrom(bpcaltable, interp='linearperobs,linearflag', calwt=True)
        #context.callibrary._remove(context.callibrary._active, calfrom)
        
        #calto = callibrary.CalTo(self.inputs.vis)
        #calfrom = callibrary.CalFrom(bpcaltable, interp='', calwt=False)
        #context.callibrary.add(calto, calfrom)

        #calto = callibrary.CalTo(self.inputs.vis)
        #calfrom = callibrary.CalFrom(gaintable=bpdgain_touse, interp='', calwt=False)
        #context.callibrary._remove(context.callibrary._active, calfrom)

        # context = self.inputs.context
        
        # calto = callibrary.CalTo(self.inputs.vis)
        # calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
        # context.callibrary.add(calto, calfrom)
        
        # calto = callibrary.CalTo(self.inputs.vis)
        # calfrom = callibrary.CalFrom(gaintable=bpcaltable, interp='', calwt=False)
        # context.callibrary.add(calto, calfrom)

        applycal_result = self._do_applycal(context=context, ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse,
                                            bpcaltable=bpcaltable, interp=interp)

        return semiFinalBPdcalsResults(bpdgain_touse=bpdgain_touse)

    def analyse(self, results):
        return results
    
    def _do_gtype_delaycal(self, caltable=None, context=None, RefAntOutput=None):
        
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = m.get_vla_tst_delay_spw()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = m.vla_minbaselineforcal()
        
        # need to add scan?
        # ref antenna string needs to be lower case for gaincal
        delaycal_inputs = gaincal.GTypeGaincal.Inputs(context,
            vis = self.inputs.vis,
            caltable = caltable,
            field    = delay_field_select_string,
            spw      = tst_delay_spw,
            solint   = 'int',
            calmode  = 'p',
            minsnr   = 3.0,
            refant   = RefAntOutput[0].lower(),
            scan = delay_scan_select_string,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '',
            antenna = '')

        delaycal_task_args = {'vis'         : self.inputs.vis,
                                'caltable'    : caltable,
                                'field'       : delay_field_select_string,
                                'spw'         : tst_delay_spw,
                                'intent'      : '',
                                'selectdata'  : True,
                                'uvrange'     : '',
                                'scan'        : delay_scan_select_string,
                                'solint'      : 'int',
                                'combine'     : 'scan',
                                'preavg'      : -1.0,
                                'refant'      : RefAntOutput[0].lower(),
                                'minblperant' : minBL_for_cal,
                                'minsnr'      : 3.0,
                                'solnorm'     : False,
                                'gaintype'    : 'G',
                                'smodel'      : [],
                                'calmode'     : 'p',
                                'append'      : False,
                                'docallib'    : False,
                                'gaintable'   : sorted(self.inputs.context.callibrary.active.get_caltable()),
                                'gainfield'   : [''],
                                'interp'      : [''],
                                'spwmap'      : [],
                                'parang'      : self.parang}

        delaycal_inputs.refant = delaycal_inputs.refant.lower()

        delaycal_task = gaincal.GTypeGaincal(delaycal_inputs)

        job = casa_tasks.gaincal(**delaycal_task_args)

        return self._executor.execute(job)
    
    def _do_ktype_delaycal(self, caltable=None, addcaltable=None, context=None, RefAntOutput=None):
        
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = m.vla_minbaselineforcal()

        # Add appropriate temporary tables to the callibrary
        ##calto = callibrary.CalTo(self.inputs.vis)
        ##calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='', calwt=False)
        ##context.callibrary.add(calto, calfrom)

        # need to add scan?
        # ref antenna string needs to be lower case for gaincal
        delaycal_inputs = gaincal.KTypeGaincal.Inputs(context,
            vis = self.inputs.vis,
            caltable = caltable,
            field    = delay_field_select_string,
            spw      = '',
            solint   = 'inf',
            calmode  = 'p',
            minsnr   = 3.0,
            refant   = RefAntOutput[0].lower(),
            scan     = delay_scan_select_string,
            minblperant = minBL_for_cal,
            solnorm = False, 
            combine = 'scan',
            intent = '')

        GainTables = sorted(self.inputs.context.callibrary.active.get_caltable())
        GainTables.append(addcaltable)

        delaycal_task_args = {'vis'         :self.inputs.vis,
                              'caltable'    :caltable,
                              'field'       : delay_field_select_string,
                              'spw'         :'',
                              'intent'      :'',
                              'selectdata'  :True,
                              'uvrange'     :'',
                              'scan'        :delay_scan_select_string,
                              'solint'      :'inf',
                              'combine'     :'scan',
                              'preavg'      :-1.0,
                              'refant'      :RefAntOutput[0].lower(),
                              'minblperant' :minBL_for_cal,
                              'minsnr'      :3.0,
                              'solnorm'     :False,
                              'gaintype'    :'K',
                              'smodel'      :[],
                              'calmode'     :'p',
                              'append'      :False,
                              'docallib'    :False,
                              'gaintable'   :GainTables,
                              'gainfield'   :[''],
                              'interp'      :[''],
                              'spwmap'      :[],
                              'parang'      :self.parang}

        #delaycal_inputs.refant = delaycal_inputs.refant.lower()

        #delaycal_task = gaincal.KTypeGaincal(delaycal_inputs)

        job = casa_tasks.gaincal(**delaycal_task_args)

        return self._executor.execute(job)
    
    def _check_flagSolns(self, flaggedSolnResult, RefAntOutput):
        
        if (flaggedSolnResult['all']['total'] > 0):
            fracFlaggedSolns=flaggedSolnResult['antmedian']['fraction']
        else:
            fracFlaggedSolns=1.0

        # refant_csvstring = self.inputs.context.observing_run.measurement_sets[0].reference_antenna
        # refantlist = [x for x in refant_csvstring.split(',')]

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        # critfrac = self.inputs.context.evla['msinfo'][m.name].critfrac
        critfrac = m.get_vla_critfrac()

        if (fracFlaggedSolns > critfrac):
            # RefAntOutput.pop(0)
            RefAntOutput = np.delete(RefAntOutput,0)
            self.inputs.context.observing_run.measurement_sets[0].reference_antenna = ','.join(RefAntOutput)
            LOG.info("Not enough good solutions, trying a different reference antenna.")
            LOG.info("The pipeline will use antenna "+RefAntOutput[0]+" as the reference.")

        return fracFlaggedSolns
    
    def _do_gtype_bpdgains(self, caltable, addcaltable=None, solint='int', context=None, RefAntOutput=None):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        tst_bpass_spw = m.get_vla_tst_bpass_spw()
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = m.vla_minbaselineforcal()

        # Add appropriate temporary tables to the callibrary
        ##calto = callibrary.CalTo(self.inputs.vis)
        ##calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='', calwt=False)
        ##context.callibrary.add(calto, calfrom)
        
        # need to add scan?
        # ref antenna string needs to be lower case for gaincal
        bpdgains_inputs = gaincal.GTypeGaincal.Inputs(context,
            vis = self.inputs.vis,
            caltable = caltable,
            field    = '',
            spw      = tst_bpass_spw,
            solint   = solint,
            calmode  = 'p',
            minsnr   = 3.0,
            refant   = RefAntOutput[0].lower(),
            scan     = bandpass_scan_select_string,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '',
            append=False)

        GainTables = sorted(self.inputs.context.callibrary.active.get_caltable())
        GainTables.append(addcaltable)

        bpdgains_task_args = {'vis'         :self.inputs.vis,
                              'caltable'    :caltable,
                              'field'       :'',
                              'spw'         :tst_bpass_spw,
                              'intent'      :'',
                              'selectdata'  :True,
                              'uvrange'     :'',
                              'scan'        :bandpass_scan_select_string,
                              'solint'      :solint,
                              'combine'     :'scan',
                              'preavg'      :-1.0,
                              'refant'      :RefAntOutput[0].lower(),
                              'minblperant' :minBL_for_cal,
                              'minsnr'      :5.0,
                              'solnorm'     :False,
                              'gaintype'    :'G',
                              'smodel'      :[],
                              'calmode'     :'p',
                              'append'      :False,
                              'docallib'    :False,
                              'gaintable'   :GainTables,
                              'gainfield'   :[''],
                              'interp'      :[''],
                              'spwmap'      :[],
                              'parang'      :self.parang}

        #bpdgains_inputs.refant = bpdgains_inputs.refant.lower()

        #bpdgains_task = gaincal.GTypeGaincal(bpdgains_inputs)

        job = casa_tasks.gaincal(**bpdgains_task_args)

        return self._executor.execute(job)
      
    def _do_applycal(self, context=None, ktypecaltable=None, bpdgain_touse=None, bpcaltable=None, interp=None):
        """Run CASA task applycal"""
        
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        
        LOG.info("Applying semi-final delay and BP calibrations to all calibrators")

        AllCalTables = sorted(self.inputs.context.callibrary.active.get_caltable())
        AllCalTables.append(ktypecaltable)
        #AllCalTables.append(bpdgain_touse)
        AllCalTables.append(bpcaltable)

        ntables=len(AllCalTables)

        applycal_task_args = {'vis'        :self.inputs.vis,
                              'field'      :'',
                              'spw'        :'',
                              'intent'     :'',
                              'selectdata' :True,
                              'scan'       :calibrator_scan_select_string,
                              'docallib'   :False,
                              'gaintable'  :AllCalTables,
                              'gainfield'  :[''],
                              'interp'     :interp,
                              'spwmap'     :[],
                              'calwt'      :[False]*ntables,
                              'parang'     :self.parang,
                              'applymode'  :'calflagstrict',
                              'flagbackup' :False}

        job = casa_tasks.applycal(**applycal_task_args)

        return self._executor.execute(job)
