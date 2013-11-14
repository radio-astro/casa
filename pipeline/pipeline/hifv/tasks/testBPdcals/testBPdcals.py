from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary

import itertools

from pipeline.hif.tasks import gaincal
from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import applycal
from pipeline.hifv.heuristics import getCalFlaggedSoln, getBCalStatistics
import pipeline.hif.heuristics.findrefant as findrefant

LOG = infrastructure.get_logger(__name__)

class testBPdcalsInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        
        self.gain_solint1 = 'int'
        self.gain_solint2 = 'int'

class testBPdcalsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[], gain_solint1=None, shortsol1=None):
        super(testBPdcalsResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.gain_solint1 = gain_solint1
        self.shortsol1 = shortsol1
        
    def merge_with_context(self, context):    
        m = context.observing_run.measurement_sets[0]
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
        m = self.inputs.context.observing_run.measurement_sets[0]
        soltimes = [self.inputs.context.evla['msinfo'][m.name].int_time * x for x in soltimes]
        solints = ['int', str(soltimes[1])+'s', str(soltimes[2])+'s']
        soltime = soltimes[0]
        solint = solints[0]

        context = self.inputs.context
        refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis=self.inputs.vis,field=refantfield,geometry=True,flagging=True, intent='', spw='')
        
        RefAntOutput=refantobj.calculate()
        
        print "RefAntOutput: ", RefAntOutput
        
        gtype_delaycal_result = self._do_gtype_delaycal(caltable=gtypecaltable, context=context, RefAntOutput=RefAntOutput)
        
        LOG.info("Initial phase calibration on delay calibrator complete")

        fracFlaggedSolns = 1.0
        
        critfrac = context.evla['msinfo'][m.name].critfrac

        

        '''
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

        #Iterate and check the fraciton of Flagged solutions, each time running gaincal in 'K' mode
        flagcount=0
        while (fracFlaggedSolns > critfrac and flagcount < 4):
            
            context = self.inputs.context
            
            
            calto = callibrary.CalTo(self.inputs.vis)
            calfrom = callibrary.CalFrom(gaintable=gtypecaltable, interp='', calwt=False)
            context.callibrary._remove(calto, calfrom, context.callibrary._active)
            
            ktype_delaycal_result = self._do_ktype_delaycal(caltable=ktypecaltable, addcaltable=gtypecaltable, context=context, RefAntOutput=RefAntOutput)
            flaggedSolnResult = getCalFlaggedSoln(ktype_delaycal_result.__dict__['inputs']['caltable'])
            (fracFlaggedSolns,RefAntOutput)  = self._check_flagSolns(flaggedSolnResult, RefAntOutput)
            LOG.info("Fraction of flagged solutions = "+str(flaggedSolnResult['all']['fraction']))
            LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult['antmedian']['fraction']))
            flagcount += 1

            try:
                calto = callibrary.CalTo(self.inputs.vis)
                calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
                context.callibrary._remove(calto, calfrom, context.callibrary._active)
            except:
                LOG.info(ktypecaltable + " does not exist in the context callibrary, and does not need to be removed.")

        # Do initial amplitude and phase gain solutions on the BPcalibrator and delay
        # calibrator; the amplitudes are used for flagging; only phase
        # calibration is applied in final BP calibration, so that solutions are
        # not normalized per spw and take out the baseband filter shape

        # Try running with solint of int_time, 3*int_time, and 10*int_time.
        # If there is still a large fraction of failed solutions with
        # solint=10*int_time the source may be too weak, and calibration via the 
        # pipeline has failed; will need to implement a mode to cope with weak 
        # calibrators (later)
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=gtypecaltable, interp='', calwt=False)
        context.callibrary._remove(calto, calfrom, context.callibrary._active)

        context = self.inputs.context
        
        bpdgain_touse = tablebase + table_suffix[0]
        
        gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[0], addcaltable=ktypecaltable, solint=solint, context=context, RefAntOutput=RefAntOutput)
        
        
        
        flaggedSolnResult1 = getCalFlaggedSoln(tablebase + table_suffix[0])
        LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult1['all']['fraction']))
        LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult1['antmedian']['fraction']))
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=tablebase + table_suffix[0], interp='', calwt=False)
        context.callibrary._remove(calto, calfrom, context.callibrary._active)
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
        context.callibrary._remove(calto, calfrom, context.callibrary._active)

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
            
            gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[1], addcaltable=ktypecaltable, solint=solint, context=context, RefAntOutput=RefAntOutput)
            flaggedSolnResult3 = getCalFlaggedSoln(tablebase + table_suffix[1])
            LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult1['all']['fraction']))
            LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult1['antmedian']['fraction']))
            calto = callibrary.CalTo(self.inputs.vis)
            calfrom = callibrary.CalFrom(gaintable=tablebase + table_suffix[1], interp='', calwt=False)
            context.callibrary._remove(calto, calfrom, context.callibrary._active)
        
            calto = callibrary.CalTo(self.inputs.vis)
            calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
            context.callibrary._remove(calto, calfrom, context.callibrary._active)

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
                
                    gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[2], addcaltable=ktypecaltable, solint=solint, context=context, RefAntOutput=RefAntOutput)
                    flaggedSolnResult10 = getCalFlaggedSoln(tablebase + table_suffix[2])
                    LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult1['all']['fraction']))
                    LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult1['antmedian']['fraction']))
                    calto = callibrary.CalTo(self.inputs.vis)
                    calfrom = callibrary.CalFrom(gaintable=tablebase + table_suffix[2], interp='', calwt=False)
                    context.callibrary._remove(calto, calfrom, context.callibrary._active)
                
                    calto = callibrary.CalTo(self.inputs.vis)
                    calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
                    context.callibrary._remove(calto, calfrom, context.callibrary._active)
                

                    if (flaggedSolnResult10['all']['total'] > 0):
                        fracFlaggedSolns10 = flaggedSolnResult10['antmedian']['fraction']
                    else:
                        fracFlaggedSolns10 = 1.0

                    if (fracFlaggedSolns10 < fracFlaggedSolns3):
                        gain_solint1=solint
                        shortsol1=soltime
                        bpdgain_touse = tablebase + table_suffix[2]

                        if (fracFlaggedSolns > 0.05):
                            LOG.warn("There is a large fraction of flagged solutions, there might be something wrong with your data.")


        LOG.info("Test amp and phase calibration on delay and bandpass calibrators complete")
        LOG.info("Using short solint = "+gain_solint1)
        context = self.inputs.context
        
        #Add appropriate temporary tables to the callibrary
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='', calwt=False)
        context.callibrary.add(calto, calfrom)
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=bpdgain_touse, interp='', calwt=False)
        context.callibrary.add(calto, calfrom)
        
        #print context.callibrary.active
        
        bandpass_result = self._do_bandpass(bpcaltable, context=context, RefAntOutput=RefAntOutput)
        
        #print context.callibrary.active
        
        #Force calwt for the bp table to be False
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(bpcaltable, interp='linear,linear', calwt=True)
        context.callibrary._remove(calto, calfrom, context.callibrary._active)
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(bpcaltable, interp='', calwt=False)
        context.callibrary.add(calto, calfrom)
        
        #print context.callibrary.active
        
        LOG.info("Fraction of flagged solutions = "+str(flaggedSolnResult['all']['fraction']))
        LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult['antmedian']['fraction']))
        
        LOG.info("Applying test calibrations to BP and delay calibrators")
        
        applycal_result = self._do_applycal(context=context)

                        
        return testBPdcalsResults(gain_solint1=gain_solint1, shortsol1=shortsol1)                        

    def analyse(self, results):
	return results
    
    def _do_gtype_delaycal(self, caltable=None, context=None, RefAntOutput=None):
        
        m = context.observing_run.measurement_sets[0]
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = context.evla['msinfo'][m.name].tst_delay_spw
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        
        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
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

        delaycal_inputs.refant = delaycal_inputs.refant.lower()

        delaycal_task = gaincal.GTypeGaincal(delaycal_inputs)

        return self._executor.execute(delaycal_task)

    def _do_ktype_delaycal(self, caltable=None, addcaltable=None, context=None, RefAntOutput=None):
        
        m = context.observing_run.measurement_sets[0]
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = context.evla['msinfo'][m.name].tst_delay_spw
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal

        #Add appropriate temporary tables to the callibrary
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='', calwt=False)
        context.callibrary.add(calto, calfrom)

        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
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

        delaycal_inputs.refant = delaycal_inputs.refant.lower()

        delaycal_task = gaincal.KTypeGaincal(delaycal_inputs)

        return self._executor.execute(delaycal_task)

    def _check_flagSolns(self, flaggedSolnResult, RefAntOutput):
        
        if (flaggedSolnResult['all']['total'] > 0):
            fracFlaggedSolns=flaggedSolnResult['antmedian']['fraction']
        else:
            fracFlaggedSolns=1.0

        #refant_csvstring = self.inputs.context.observing_run.measurement_sets[0].reference_antenna
        #refantlist = [x for x in refant_csvstring.split(',')]
        #refantlist = RefAntOutput

        m = self.inputs.context.observing_run.measurement_sets[0]
        critfrac = self.inputs.context.evla['msinfo'][m.name].critfrac

        if (fracFlaggedSolns > critfrac):
            RefAntOutput.pop(0)
            self.inputs.context.observing_run.measurement_sets[0].reference_antenna = ','.join(RefAntOutput)
            LOG.info("Not enough good solutions, trying a different reference antenna.")
            LOG.info("The pipeline will use antenna "+RefAntOutput[0].lower()+" as the reference.")

        return (fracFlaggedSolns, RefAntOutput)

    def _do_gtype_bpdgains(self, caltable, addcaltable=None, solint='int', context=None, RefAntOutput=None):

        m = context.observing_run.measurement_sets[0]
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        tst_bpass_spw = context.evla['msinfo'][m.name].tst_bpass_spw
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal

        if (delay_scan_select_string == bandpass_scan_select_string):
            testgainscans=bandpass_scan_select_string
        else:
            testgainscans=bandpass_scan_select_string+','+delay_scan_select_string
        
        #Add appropriate temporary tables to the callibrary
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='', calwt=False)
        context.callibrary.add(calto, calfrom)
        
        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
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

        bpdgains_inputs.refant = bpdgains_inputs.refant.lower()

        bpdgains_task = gaincal.GTypeGaincal(bpdgains_inputs)

        return self._executor.execute(bpdgains_task)

    def _do_bandpass(self, caltable, context=None, RefAntOutput=None):
        """Run CASA task bandpass"""

        m = context.observing_run.measurement_sets[0]
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal

        LOG.info("Doing test bandpass calibration")

        #bandtype = 'B'
        bandpass_inputs = bandpass.ChannelBandpass.Inputs(context,
            vis = self.inputs.vis,
            caltable = caltable,
            field = bandpass_field_select_string,
            spw = '',
            intent = '',
            solint = 'inf',
            combine = 'scan',
            refant = RefAntOutput[0].lower(),
            scan = bandpass_scan_select_string,
            minblperant = minBL_for_cal,
            minsnr = 5.0,
            solnorm = False)

        bandpass_inputs.refant = bandpass_inputs.refant.lower()

        bandpass_task = bandpass.ChannelBandpass(bandpass_inputs)
        
        LOG.info("Test bandpass calibration complete")

        return self._executor.execute(bandpass_task, merge=True)

        
    def _do_applycal(self, context=None):
        """Run CASA task applycal"""
        
        m = context.observing_run.measurement_sets[0]
        testgainscans = context.evla['msinfo'][m.name].testgainscans
        
        applycal_inputs = applycal.Applycal.Inputs(context,
            vis = self.inputs.vis,
            field = '',
            spw = '',
            intent = '',
            scan = testgainscans,
            flagbackup = False,
            calwt = False)
        
        applycal_task = applycal.Applycal(applycal_inputs)
        
        return self._executor.execute(applycal_task)
            


        
        
        
        
