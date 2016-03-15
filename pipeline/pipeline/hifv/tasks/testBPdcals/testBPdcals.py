from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks

from pipeline.hif.tasks import gaincal
from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import applycal
from pipeline.hifv.heuristics import getCalFlaggedSoln, getBCalStatistics
import pipeline.hif.heuristics.findrefant as findrefant
from pipeline.hifv.heuristics import weakbp, do_bandpass

LOG = infrastructure.get_logger(__name__)


class testBPdcalsInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, weakbp=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

        self._weakbp = weakbp

        self.gain_solint1 = 'int'
        self.gain_solint2 = 'int'

    @property
    def weakbp(self):
        return self._weakbp

    @weakbp.setter
    def weakbp(self, value):

        if self._weakbp is None:
            self._weakbp = True

        self._weakbp = value


class testBPdcalsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[], gain_solint1=None, shortsol1=None, vis=None, bpdgain_touse=None):
        super(testBPdcalsResults, self).__init__()

        self.vis = vis
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.gain_solint1 = gain_solint1
        self.shortsol1 = shortsol1
        self.bpdgain_touse = bpdgain_touse
        
    def merge_with_context(self, context):
        m = context.observing_run.get_ms(self.vis)
        context.evla['msinfo'][m.name].gain_solint1 = self.gain_solint1
        context.evla['msinfo'][m.name].shortsol1 = self.shortsol1
    
        
class testBPdcals(basetask.StandardTaskTemplate):
    Inputs = testBPdcalsInputs

    def prepare(self):


        
        gtypecaltable = 'testdelayinitialgain.g'
        ktypecaltable = 'testdelay.k'
        bpcaltable = 'testBPcal.b'
        tablebase = 'testBPdinitialgain'
        table_suffix = ['.g','3.g','10.g']
        soltimes = [1.0,3.0,10.0] 
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        soltimes = [m.get_vla_max_integration_time() * x for x in soltimes]
        # soltimes = [self.inputs.context.evla['msinfo'][m.name].int_time * x for x in soltimes]
        solints = ['int', str(soltimes[1])+'s', str(soltimes[2])+'s']
        soltime = soltimes[0]
        solint = solints[0]

        context = self.inputs.context
        refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis=self.inputs.vis,field=refantfield,
                                                geometry=True,flagging=True, intent='', spw='')
        
        RefAntOutput = refantobj.calculate()
        
        print "RefAntOutput: ", RefAntOutput
        
        gtype_delaycal_result = self._do_gtype_delaycal(caltable=gtypecaltable,
                                                        context=context, RefAntOutput=RefAntOutput)
        
        LOG.info("Initial phase calibration on delay calibrator complete")

        fracFlaggedSolns = 1.0
        
        # critfrac = context.evla['msinfo'][m.name].critfrac
        critfrac = m.get_vla_critfrac()

        '''
        #Notes from original code...
        ktype_delaycal_result = self._do_ktype_delaycal(caltable=ktypecaltable, addcaltable=gtypecaltable, context=context)
        flaggedSolnResult = getCalFlaggedSoln(ktype_delaycal_result.__dict__['inputs']['caltable'])
        LOG.info("Fraction of flagged solutions = "+str(flaggedSolnResult['all']['fraction']))
        LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult['antmedian']['fraction']))

        if (flaggedSolnResult['all']['total'] > 0):
            fracFlaggedSolns=flaggedSolnResult['antmedian']['fraction']
        else:
            fracFlaggedSolns=1.0
            
        # NB: in case the reference antenna has a bad baseband/IF, check
        # a couple of reference antennas if there is a high fraction of 
        # flagged solutions

        if (fracFlaggedSolns > critfrac):
            fracFlaggedSolns = self._check_flagSolns(flaggedSolnResult)
        '''

        # Iterate and check the fraction of Flagged solutions, each time running gaincal in 'K' mode
        flagcount=0
        while (fracFlaggedSolns > critfrac and flagcount < 4):

            # Don't remove the CalFrom before its first addition! The gaincal task above
            # was not executed with 'merge=True', so it was not added to the CalLibrary.
            # This removal marks the CalFrom as globally removed, before it has chance to
            # be added in the do_ktype_delaycal call; the add call in do_kytype_delaycal
            # then just reactivates the CalFrom - but it never existed in any entry so the
            # 'reactivated' caltable never appears in the subsequent calls.
            #
            # LOG.info("TESTBPDCALS DEBUG:  START CALLIBRARY")
            # calfrom = callibrary.CalFrom(gaintable=gtypecaltable, interp='', calwt=False)
            # context.callibrary._remove(context.callibrary._active, calfrom)
            # LOG.info("TESTBPDCALS DEBUG:  END CALLIBRARY")

            ktype_delaycal_result = self._do_ktype_delaycal(caltable=ktypecaltable, addcaltable=gtypecaltable, context=context, RefAntOutput=RefAntOutput)
            #flaggedSolnResult = getCalFlaggedSoln(ktype_delaycal_result.__dict__['inputs']['caltable'])
            flaggedSolnResult = getCalFlaggedSoln(ktypecaltable)
            (fracFlaggedSolns,RefAntOutput)  = self._check_flagSolns(flaggedSolnResult, RefAntOutput)
            LOG.info("Fraction of flagged solutions = "+str(flaggedSolnResult['all']['fraction']))
            LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult['antmedian']['fraction']))
            flagcount += 1

            # Removal before first addition again. See comment above.
            # try:
            #     calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
            #     context.callibrary._remove(context.callibrary._active, calfrom)
            # except:
            #     LOG.info(ktypecaltable + " does not exist in the context callibrary, and does not need to be removed.")

        # Do initial amplitude and phase gain solutions on the BPcalibrator and delay
        # calibrator; the amplitudes are used for flagging; only phase
        # calibration is applied in final BP calibration, so that solutions are
        # not normalized per spw and take out the baseband filter shape

        # Try running with solint of int_time, 3*int_time, and 10*int_time.
        # If there is still a large fraction of failed solutions with
        # solint=10*int_time the source may be too weak, and calibration via the 
        # pipeline has failed; will need to implement a mode to cope with weak 
        # calibrators (later)
        ##calfrom = callibrary.CalFrom(gaintable=gtypecaltable, interp='', calwt=False)
        ##context.callibrary._remove(context.callibrary._active, calfrom)

        context = self.inputs.context
        
        bpdgain_touse = tablebase + table_suffix[0]
        
        gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[0], addcaltable=ktypecaltable,
                                                       solint=solint, context=context, RefAntOutput=RefAntOutput)

        flaggedSolnResult1 = getCalFlaggedSoln(tablebase + table_suffix[0])
        LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult1['all']['fraction']))
        LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult1['antmedian']['fraction']))

        # caltable generated above was never added, so this removal disables it from
        # ever being active in any subsequent addition. See comment at line 107.
        # calfrom = callibrary.CalFrom(gaintable=tablebase + table_suffix[0], interp='', calwt=False)
        # context.callibrary._remove(context.callibrary._active, calfrom)
        
        ##calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
        ##context.callibrary._remove(context.callibrary._active, calfrom)

        if (flaggedSolnResult1['all']['total'] > 0):
            fracFlaggedSolns1=flaggedSolnResult1['antmedian']['fraction']
        else:
            fracFlaggedSolns1=1.0

        gain_solint1=solint
        shortsol1=soltime

        if (fracFlaggedSolns1 > 0.05):
            soltime = soltimes[1]
            solint = solints[1]

            context = self.inputs.context
            
            gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[1], addcaltable=ktypecaltable,
                                                           solint=solint, context=context, RefAntOutput=RefAntOutput)

            flaggedSolnResult3 = getCalFlaggedSoln(tablebase + table_suffix[1])
            LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult3['all']['fraction']))
            LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult3['antmedian']['fraction']))
            ##calfrom = callibrary.CalFrom(gaintable=tablebase + table_suffix[1], interp='', calwt=False)
            ##context.callibrary._remove(context.callibrary._active, calfrom)
        
            ##calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
            ##context.callibrary._remove(context.callibrary._active, calfrom)

            if (flaggedSolnResult3['all']['total'] > 0):
                fracFlaggedSolns3=flaggedSolnResult3['antmedian']['fraction']
            else:
                fracFlaggedSolns3=1.0

            if (fracFlaggedSolns3 < fracFlaggedSolns1):
                gain_solint1 = solint
                shortsol1 = soltime
            
                bpdgain_touse = tablebase + table_suffix[1]
            
                if (fracFlaggedSolns3 > 0.05):
                    soltime = soltimes[2]
                    solint = solints[2]

                    context = self.inputs.context
                
                    gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[2],
                                                                   addcaltable=ktypecaltable, solint=solint,
                                                                   context=context, RefAntOutput=RefAntOutput)
                    flaggedSolnResult10 = getCalFlaggedSoln(tablebase + table_suffix[2])
                    LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult10['all']['fraction']))
                    LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult10['antmedian']['fraction']))
                    ##calfrom = callibrary.CalFrom(gaintable=tablebase + table_suffix[2], interp='', calwt=False)
                    ##context.callibrary._remove(context.callibrary._active, calfrom)
                
                    ##calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
                    ##context.callibrary._remove(context.callibrary._active, calfrom)
                

                    if (flaggedSolnResult10['all']['total'] > 0):
                        fracFlaggedSolns10 = flaggedSolnResult10['antmedian']['fraction']
                    else:
                        fracFlaggedSolns10 = 1.0

                    if (fracFlaggedSolns10 < fracFlaggedSolns3):
                        gain_solint1=solint
                        shortsol1=soltime
                        bpdgain_touse = tablebase + table_suffix[2]

                        if (fracFlaggedSolns10 > 0.05):
                            LOG.warn("There is a large fraction of flagged solutions, there might be something wrong with your data.  The fraction of flagged solutions is " + str(fracFlaggedSolns10))

        LOG.info("Test amp and phase calibration on delay and bandpass calibrators complete")
        LOG.info("Using short solint = "+gain_solint1)
        #context = self.inputs.context
        
        # Add appropriate temporary tables to the callibrary
        ##calto = callibrary.CalTo(self.inputs.vis)
        ##calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
        ##context.callibrary.add(calto, calfrom)
        
        ##calto = callibrary.CalTo(self.inputs.vis)
        ##calfrom = callibrary.CalFrom(gaintable=bpdgain_touse, interp='', calwt=False)
        ##context.callibrary.add(calto, calfrom)
        
        # print context.callibrary.active

        LOG.info("Doing test bandpass calibration")

        '''
        flagdata_task_args = {'vis'         :self.inputs.vis, 'spw' : '0:35~40'}
        job = casa_tasks.flagdata(**flagdata_task_args)
        self._executor.execute(job)

        flagdata_task_args = {'vis'         :self.inputs.vis, 'spw' : '3:35~40'}
        job = casa_tasks.flagdata(**flagdata_task_args)
        self._executor.execute(job)
        '''

        print "WEAKBP: ", self.inputs.weakbp

        if (self.inputs.weakbp == True):
            print "USING WEAKBP HEURISTICS"
            interp = weakbp(self.inputs.vis, bpcaltable, context=context, RefAntOutput=RefAntOutput[0],
                                            ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse, solint='inf', append=False)
        else:
            print "Using REGULAR heuristics"
            interp = ''
            bandpass_job = do_bandpass(self.inputs.vis, bpcaltable, context=context, RefAntOutput=RefAntOutput[0], spw='',
                                            ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse, solint='inf', append=False)
            self._executor.execute(bandpass_job)

        LOG.info("Test bandpass calibration complete")

        # print context.callibrary.active
        
        # Force calwt for the bp table to be False
        ##calfrom = callibrary.CalFrom(bpcaltable, interp='linearperobs,linearflag', calwt=True)
        ##context.callibrary._remove(context.callibrary._active, calfrom)
        
        ##calto = callibrary.CalTo(self.inputs.vis)
        ##calfrom = callibrary.CalFrom(bpcaltable, interp='', calwt=False)
        ##context.callibrary.add(calto, calfrom)
        
        # print context.callibrary.active
        
        LOG.info("Fraction of flagged solutions = "+str(flaggedSolnResult['all']['fraction']))
        LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult['antmedian']['fraction']))
        
        LOG.info("Applying test calibrations to BP and delay calibrators")
        
        applycal_result = self._do_applycal(context=context, ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse,
                                            bpcaltable=bpcaltable, interp=interp)

        return testBPdcalsResults(gain_solint1=gain_solint1, shortsol1=shortsol1, vis=self.inputs.vis, bpdgain_touse=bpdgain_touse)

    def analyse(self, results):
        return results
    
    def _do_gtype_delaycal(self, caltable=None, context=None, RefAntOutput=None):
        
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        # tst_delay_spw = context.evla['msinfo'][m.name].tst_delay_spw
        tst_delay_spw = m.get_vla_tst_delay_spw()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        # minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        minBL_for_cal = max(3,int(len(m.antennas)/2.0))
        
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
            scan     = delay_scan_select_string,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '')

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
                                'gaintable'   : list(self.inputs.context.callibrary.active.get_caltable()),
                                'gainfield'   : [''],
                                'interp'      : [''],
                                'spwmap'      : [],
                                'parang'      : False}

        delaycal_inputs.refant = delaycal_inputs.refant.lower()


        delaycal_task = gaincal.GTypeGaincal(delaycal_inputs)

        job = casa_tasks.gaincal(**delaycal_task_args)

        return self._executor.execute(job)

    def _do_ktype_delaycal(self, caltable=None, addcaltable=None, context=None, RefAntOutput=None):
        
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        # tst_delay_spw = context.evla['msinfo'][m.name].tst_delay_spw
        tst_delay_spw = m.get_vla_tst_delay_spw()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        # minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        minBL_for_cal = max(3,int(len(m.antennas)/2.0))

        # Add appropriate temporary tables to the callibrary
        ##calto = callibrary.CalTo(self.inputs.vis)
        ##calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='', calwt=False)
        ##context.callibrary.add(calto, calfrom)

        # need to add scan?
        # ref antenna string needs to be lower case for gaincal
        '''
        delaycal_inputs = gaincal.KTypeGaincal.Inputs(context,
            vis = self.inputs.vis,
            caltable = caltable,
            field    = delay_field_select_string,
            spw      = '',
            solint   = 'inf',
            calmode  = 'p',
            minsnr   = 3.0,
            refant   = RefAntOutput[0].lower(),
            scan = delay_scan_select_string,
            minblperant = minBL_for_cal,
            solnorm = False, 
            combine = 'scan',
            intent = '')
        '''

        GainTables = list(self.inputs.context.callibrary.active.get_caltable())
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
                              'parang'      :False}


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
        # refantlist = RefAntOutput

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        # critfrac = self.inputs.context.evla['msinfo'][m.name].critfrac
        critfrac = m.get_vla_critfrac()

        if (fracFlaggedSolns > critfrac):
            print RefAntOutput
            # RefAntOutput.pop(0)
            RefAntOutput = np.delete(RefAntOutput,0)
            self.inputs.context.observing_run.measurement_sets[0].reference_antenna = ','.join(RefAntOutput)
            LOG.info("Not enough good solutions, trying a different reference antenna.")
            LOG.info("The pipeline will use antenna "+RefAntOutput[0].lower()+" as the reference.")

        return (fracFlaggedSolns, RefAntOutput)

    def _do_gtype_bpdgains(self, caltable, addcaltable=None, solint='int', context=None, RefAntOutput=None):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        # tst_bpass_spw = context.evla['msinfo'][m.name].tst_bpass_spw
        tst_bpass_spw = m.get_vla_tst_bpass_spw()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        # minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        minBL_for_cal = max(3,int(len(m.antennas)/2.0))

        if (delay_scan_select_string == bandpass_scan_select_string):
            testgainscans=bandpass_scan_select_string
        else:
            testgainscans=bandpass_scan_select_string+','+delay_scan_select_string
        
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
            calmode  = 'ap',
            minsnr   = 5.0,
            scan     = testgainscans,
            refant   = RefAntOutput[0].lower(),
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '')

        GainTables = list(self.inputs.context.callibrary.active.get_caltable())
        GainTables.append(addcaltable)

        bpdgains_task_args = {'vis'         :self.inputs.vis,
                              'caltable'    :caltable,
                              'field'       :'',
                              'spw'         :tst_bpass_spw,
                              'intent'      :'',
                              'selectdata'  :True,
                              'uvrange'     :'',
                              'scan'        :testgainscans,
                              'solint'      :solint,
                              'combine'     :'scan',
                              'preavg'      :-1.0,
                              'refant'      :RefAntOutput[0].lower(),
                              'minblperant' :minBL_for_cal,
                              'minsnr'      :5.0,
                              'solnorm'     :False,
                              'gaintype'    :'G',
                              'smodel'      :[],
                              'calmode'     :'ap',
                              'append'      :False,
                              'docallib'    :False,
                              'gaintable'   :GainTables,
                              'gainfield'   :[''],
                              'interp'      :[''],
                              'spwmap'      :[],
                              'parang'      :False}

        #bpdgains_inputs.refant = bpdgains_inputs.refant.lower()

        #bpdgains_task = gaincal.GTypeGaincal(bpdgains_inputs)

        job = casa_tasks.gaincal(**bpdgains_task_args)

        return self._executor.execute(job)

    def _do_applycal(self, context=None, ktypecaltable=None, bpdgain_touse=None, bpcaltable=None, interp=None):
        """Run CASA task applycal"""
        
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        testgainscans = context.evla['msinfo'][m.name].testgainscans
        
        applycal_inputs = applycal.Applycal.Inputs(context,
            vis = self.inputs.vis,
            field = '',
            spw = '',
            intent = '',
            scan = testgainscans,
            flagbackup = False,
            calwt = False,
            flagsum = False)

        AllCalTables = list(self.inputs.context.callibrary.active.get_caltable())
        AllCalTables.append(ktypecaltable)
        AllCalTables.append(bpdgain_touse)
        AllCalTables.append(bpcaltable)

        ntables=len(AllCalTables)

        applycal_task_args = {'vis'        :self.inputs.vis,
                              'field'      :'',
                              'spw'        :'',
                              'intent'     :'',
                              'selectdata' :True,
                              'scan'       :testgainscans,
                              'docallib'   :False,
                              'gaintable'  :AllCalTables,
                              'gainfield'  :[''],
                              'interp'     :[interp],
                              'spwmap'     :[],
                              'calwt'      :[False]*ntables,
                              'parang'     :False,
                              'applymode'  :'calflagstrict',
                              'flagbackup' :False}

        
        #applycal_task = applycal.Applycal(applycal_inputs)
        
        job = casa_tasks.applycal(**applycal_task_args)

        return self._executor.execute(job)
