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
from pipeline.vla.heuristics import getCalFlaggedSoln, getBCalStatistics


LOG = infrastructure.get_logger(__name__)

class testBPdcalsInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class testBPdcalsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(testBPdcalsResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    
        
class testBPdcals(basetask.StandardTaskTemplate):
    Inputs = testBPdcalsInputs

    def prepare(self):
        
        
        gtype_delaycal_result = self._do_gtype_delaycal()
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable='testdelayinitialgain.g', interp='linear,linear', calwt=True)
        self.inputs.context.callibrary._remove(calto, calfrom, self.inputs.context.callibrary._active)

        fracFlaggedSolns = 1.0
        m = self.inputs.context.observing_run.measurement_sets[0]
        critfrac = self.inputs.context.evla['msinfo'][m.name].critfrac

        #Iterate and check the fraciton of Flagged solutions, each time running gaincal in 'K' mode
        while (fracFlaggedSolns > critfrac):
            
                
            ktype_delaycal_result = self._do_ktype_delaycal()
            flaggedSolnResult = getCalFlaggedSoln(ktype_delaycal_result.__dict__['inputs']['caltable'])
            fracFlaggedSolns = self._check_flagSolns(flaggedSolnResult)

            try:
                calto = callibrary.CalTo(self.inputs.vis)
                calfrom = callibrary.CalFrom(gaintable='testdelay.k', interp='linear,linear', calwt=True)
                self.inputs.context.callibrary._remove(calto, calfrom, self.inputs.context.callibrary._active)
            except:
                LOG.info("testdelay.k does not exist in the context, and does not need to be removed.")

        # Do initial amplitude and phase gain solutions on the BPcalibrator and delay
        # calibrator; the amplitudes are used for flagging; only phase
        # calibration is applied in final BP calibration, so that solutions are
        # not normalized per spw and take out the baseband filter shape

        # Try running with solint of int_time, 3*int_time, and 10*int_time.
        # If there is still a large fraction of failed solutions with
        # solint=10*int_time the source may be too weak, and calibration via the 
        # pipeline has failed; will need to implement a mode to cope with weak 
        # calibrators (later)

        tablebase = 'testBPdinitialgain'
        table_suffix = ['.g','3.g','10.g']
        soltimes = [1.0,3.0,10.0] 
        soltimes = [self.inputs.context.evla['msinfo'][m.name].int_time * x for x in soltimes]
        solints = ['int', '3.0s', '10.0s']
        soltime = soltimes[0]
        solint = solints[0]

        bpdgain_touse = 'testBPdinitialgain.g'
        
        gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[0])
        flaggedSolnResult1 = getCalFlaggedSoln(tablebase + table_suffix[0])
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable='testBPdinitialgain.g', interp='linear,linear', calwt=True)
        self.inputs.context.callibrary._remove(calto, calfrom, self.inputs.context.callibrary._active)

        if (flaggedSolnResult1['all']['total'] > 0):
            fracFlaggedSolns1=flaggedSolnResult1['antmedian']['fraction']
        else:
            fracFlaggedSolns1=1.0

        gain_solint1=solint
        shortsol1=soltime

        if (fracFlaggedSolns1 > 0.05):
            soltime = soltimes[1]
            solint = solints[1]

        gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[1])
        flaggedSolnResult3 = getCalFlaggedSoln(tablebase + table_suffix[1])
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable='testBPdinitialgain3.g', interp='linear,linear', calwt=True)
        self.inputs.context.callibrary._remove(calto, calfrom, self.inputs.context.callibrary._active)

        if (flaggedSolnResult3['all']['total'] > 0):
            fracFlaggedSolns3=flaggedSolnResult3['antmedian']['fraction']
        else:
            fracFlaggedSolns3=1.0

        if (fracFlaggedSolns3 < fracFlaggedSolns1):
            gain_solint1 = solint
            shortsol1 = soltime
            
            bpdgain_touse = 'testBPdinitialgain3.g'
            
            if (fracFlaggedSolns3 > 0.05):
                soltime = soltimes[2]
                solint = solints[2]

                gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[2])
                flaggedSolnResult10 = getCalFlaggedSoln(tablebase + table_suffix[2])
                calto = callibrary.CalTo(self.inputs.vis)
                calfrom = callibrary.CalFrom(gaintable='testBPdinitialgain10.g', interp='linear,linear', calwt=True)
                self.inputs.context.callibrary._remove(calto, calfrom, self.inputs.context.callibrary._active)
                

                if (flaggedSolnResult10['all']['total'] > 0):
                    fracFlaggedSolns10 = flaggedSolnResult10['antmedian']['fraction']
                else:
                    fracFlaggedSolns10 = 1.0

                if (fracFlaggedSolns10 < fracFlaggedSolns3):
                    gain_solint1=solint
                    shortsol1=soltime
                    bpdgain_touse = 'testBPdinitialgain10.g'

                    if (fracFlaggedSolns > 0.05):
                        LOG.warn("There is a large fraction of flagged solutions, there might be something wrong with your data.")

        #Add appropriate temporary tables to the callibrary
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable='testdelay.k', interp='linear,linear', calwt=True)
        self.inputs.context.callibrary.add(calto, calfrom)
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=bpdgain_touse, interp='linear,linear', calwt=True)
        self.inputs.context.callibrary.add(calto, calfrom)
        
        
        bandpass_result = self._do_bandpass('testBPcal.b')
        
        applycal_result = self._do_applycal()
        
        
                        
        print self.inputs.context.callibrary.active
                        
        return testBPdcalsResults()                        

    def analyse(self, results):
	return results
    
    def _do_gtype_delaycal(self):
        
        m = self.inputs.context.observing_run.measurement_sets[0]
        delay_field_select_string = self.inputs.context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = self.inputs.context.evla['msinfo'][m.name].tst_delay_spw
        delay_scan_select_string = self.inputs.context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = self.inputs.context.evla['msinfo'][m.name].minBL_for_cal
        
        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
        delaycal_inputs = gaincal.GTypeGaincal.Inputs(self.inputs.context,
            vis = self.inputs.vis,
            caltable = 'testdelayinitialgain.g',
            field    = delay_field_select_string,
            spw      = tst_delay_spw,
            solint   = 'int',
            calmode  = 'p',
            minsnr   = 3.0,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '')

        delaycal_inputs.refant = delaycal_inputs.refant.lower()

        delaycal_task = gaincal.GTypeGaincal(delaycal_inputs)

        return self._executor.execute(delaycal_task)

    def _do_ktype_delaycal(self):
        
        m = self.inputs.context.observing_run.measurement_sets[0]
        delay_field_select_string = self.inputs.context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = self.inputs.context.evla['msinfo'][m.name].tst_delay_spw
        delay_scan_select_string = self.inputs.context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = self.inputs.context.evla['msinfo'][m.name].minBL_for_cal

        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
        delaycal_inputs = gaincal.KTypeGaincal.Inputs(self.inputs.context,
            vis = self.inputs.vis,
            caltable = 'testdelay.k',
            field    = delay_field_select_string,
            spw      = tst_delay_spw,
            solint   = 'inf',
            calmode  = 'p',
            minsnr   = 3.0,
            minblperant = minBL_for_cal,
            solnorm = False, 
            combine = 'scan',
            intent = '')

        delaycal_inputs.refant = delaycal_inputs.refant.lower()

        delaycal_task = gaincal.KTypeGaincal(delaycal_inputs)

        return self._executor.execute(delaycal_task)

    def _check_flagSolns(self, flaggedSolnResult):
        
        if (flaggedSolnResult['all']['total'] > 0):
            fracFlaggedSolns=flaggedSolnResult['antmedian']['fraction']
        else:
            fracFlaggedSolns=1.0

        refant_csvstring = self.inputs.context.observing_run.measurement_sets[0].reference_antenna
        refantlist = [x for x in refant_csvstring.split(',')]

        m = self.inputs.context.observing_run.measurement_sets[0]
        critfrac = self.inputs.context.evla['msinfo'][m.name].critfrac

        if (fracFlaggedSolns > critfrac):
            refantlist.pop(0)
            self.inputs.context.observing_run.measurement_sets[0].reference_antenna = ','.join(refantlist)
            LOG.info("Not enough good solutions, trying a different reference antenna.")
            LOG.info("The pipeline will use antenna "+refantlist[0]+" as the reference.")

        return fracFlaggedSolns

    def _do_gtype_bpdgains(self, caltable):

        m = self.inputs.context.observing_run.measurement_sets[0]
        delay_field_select_string = self.inputs.context.evla['msinfo'][m.name].delay_field_select_string
        tst_bpass_spw = self.inputs.context.evla['msinfo'][m.name].tst_bpass_spw
        delay_scan_select_string = self.inputs.context.evla['msinfo'][m.name].delay_scan_select_string
        bandpass_scan_select_string = self.inputs.context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = self.inputs.context.evla['msinfo'][m.name].minBL_for_cal

        if (delay_scan_select_string == bandpass_scan_select_string):
            testgainscans=bandpass_scan_select_string
        else:
            testgainscans=bandpass_scan_select_string+','+delay_scan_select_string
        
        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
        bpdgains_inputs = gaincal.GTypeGaincal.Inputs(self.inputs.context,
            vis = self.inputs.vis,
            caltable = caltable,
            field    = '',
            spw      = tst_bpass_spw,
            solint   = 'int',
            calmode  = 'ap',
            minsnr   = 5.0,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '')

        bpdgains_inputs.refant = bpdgains_inputs.refant.lower()

        bpdgains_task = gaincal.GTypeGaincal(bpdgains_inputs)

        return self._executor.execute(bpdgains_task)

    def _do_bandpass(self, caltable):
        """Run CASA task bandpass"""

        m = self.inputs.context.observing_run.measurement_sets[0]
        bandpass_field_select_string = self.inputs.context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = self.inputs.context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = self.inputs.context.evla['msinfo'][m.name].minBL_for_cal

        #bandtype = 'B'
        bandpass_inputs = bandpass.ChannelBandpass.Inputs(self.inputs.context,
            vis = self.inputs.vis,
            caltable = caltable,
            field = bandpass_field_select_string,
            spw = '',
            intent = '',
            solint = 'inf',
            combine = 'scan',
            minblperant = minBL_for_cal,
            minsnr = 5.0,
            solnorm = False)

        bandpass_inputs.refant = bandpass_inputs.refant.lower()

        bandpass_task = bandpass.ChannelBandpass(bandpass_inputs)

        return self._executor.execute(bandpass_task, merge=True)

        
    def _do_applycal(self):
        """Run CASA task applycal"""
        
        applycal_inputs = applycal.Applycal.Inputs(self.inputs.context,
            vis = self.inputs.vis,
            field = '',
            spw = '',
            intent = '',
            flagbackup = False,
            calwt = False,
            gaincurve = False)
        
        applycal_task = applycal.Applycal(applycal_inputs)
        
        return self._executor.execute(applycal_task)
            


        
        
        
        
