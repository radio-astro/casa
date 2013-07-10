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

class semiFinalBPdcalsInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class semiFinalBPdcalsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(semiFinalBPdcalsResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

class semiFinalBPdcals(basetask.StandardTaskTemplate):
    Inputs = semiFinalBPdcalsInputs
    
    def prepare(self):
        
        gtypecaltable = 'semiFinaldelayinitialgain.g'
        ktypecaltable = 'delay.k'
        bpcaltable = 'BPcal.b'
        tablebase = 'BPinitialgain'
        table_suffix = ['.g','3.g','10.g']
        soltimes = [1.0,3.0,10.0] 
        soltimes = [self.inputs.context.evla['msinfo'][m.name].int_time * x for x in soltimes]
        solints = ['int', '3.0s', '10.0s']
        soltime = soltimes[0]
        solint = solints[0]
        
        gtype_delaycal_result = self._do_gtype_delaycal(caltable=gtypecaltable)
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=gtypecaltable, interp='linear,linear', calwt=True)
        self.inputs.context.callibrary._remove(calto, calfrom, self.inputs.context.callibrary._active)

        fracFlaggedSolns = 1.0
        m = self.inputs.context.observing_run.measurement_sets[0]
        critfrac = self.inputs.context.evla['msinfo'][m.name].critfrac

        #Iterate and check the fraciton of Flagged solutions, each time running gaincal in 'K' mode
        while (fracFlaggedSolns > critfrac):
            
                
            ktype_delaycal_result = self._do_ktype_delaycal(caltable=ktypecaltable, addcaltable=gtypecaltable)
            flaggedSolnResult = getCalFlaggedSoln(ktype_delaycal_result.__dict__['inputs']['caltable'])
            fracFlaggedSolns = self._check_flagSolns(flaggedSolnResult)

            try:
                calto = callibrary.CalTo(self.inputs.vis)
                calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='linear,linear', calwt=True)
                self.inputs.context.callibrary._remove(calto, calfrom, self.inputs.context.callibrary._active)
            except:
                LOG.info(ktypecaltable + " does not exist in the context callibrary, and does not need to be removed.")

        # Do initial gaincal on BP calibrator then semi-final BP calibration
        gain_solint1 = self.inputs.context.evla['msinfo'][m.name].gain_solint1
        gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[0], addcaltable=ktypecaltable, solint=gain_solint1)
        
        bpdgain_touse = tablebase + table_suffix[0]
        
        #Add appropriate temporary tables to the callibrary
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=ktypecaltable, interp='linear,linear', calwt=True)
        self.inputs.context.callibrary.add(calto, calfrom)
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=bpdgain_touse, interp='linear,linear', calwt=True)
        self.inputs.context.callibrary.add(calto, calfrom)

        bandpass_result = self._do_bandpass(bpcaltable)

        applycal_result = self._do_applycal()

        return semiFinalBPdcalsResults()                        

    def analyse(self, results):
	return results
    
    def _do_gtype_delaycal(self, caltable=None):
        
        m = self.inputs.context.observing_run.measurement_sets[0]
        delay_field_select_string = self.inputs.context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = self.inputs.context.evla['msinfo'][m.name].tst_delay_spw
        delay_scan_select_string = self.inputs.context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = self.inputs.context.evla['msinfo'][m.name].minBL_for_cal
        
        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
        delaycal_inputs = gaincal.GTypeGaincal.Inputs(self.inputs.context,
            vis = self.inputs.vis,
            caltable = caltable,
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
    
    def _do_ktype_delaycal(self, caltable=None, addcaltable=None):
        
        m = self.inputs.context.observing_run.measurement_sets[0]
        delay_field_select_string = self.inputs.context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = self.inputs.context.evla['msinfo'][m.name].tst_delay_spw
        delay_scan_select_string = self.inputs.context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = self.inputs.context.evla['msinfo'][m.name].minBL_for_cal

        #Add appropriate temporary tables to the callibrary
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='linear,linear', calwt=True)
        self.inputs.context.callibrary.add(calto, calfrom)

        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
        delaycal_inputs = gaincal.KTypeGaincal.Inputs(self.inputs.context,
            vis = self.inputs.vis,
            caltable = caltable,
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
    
    def _do_gtype_bpdgains(self, caltable, addcaltable=None, solint='int'):

        m = self.inputs.context.observing_run.measurement_sets[0]
        delay_field_select_string = self.inputs.context.evla['msinfo'][m.name].delay_field_select_string
        tst_bpass_spw = self.inputs.context.evla['msinfo'][m.name].tst_bpass_spw
        delay_scan_select_string = self.inputs.context.evla['msinfo'][m.name].delay_scan_select_string
        bandpass_scan_select_string = self.inputs.context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = self.inputs.context.evla['msinfo'][m.name].minBL_for_cal
        
        
        #Add appropriate temporary tables to the callibrary
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='linear,linear', calwt=True)
        self.inputs.context.callibrary.add(calto, calfrom)
        
        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
        bpdgains_inputs = gaincal.GTypeGaincal.Inputs(self.inputs.context,
            vis = self.inputs.vis,
            caltable = caltable,
            field    = '',
            spw      = tst_bpass_spw,
            solint   = solint,
            calmode  = 'p',
            minsnr   = 3.0,
            preavg   = -1.0,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '',
            append=False)

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