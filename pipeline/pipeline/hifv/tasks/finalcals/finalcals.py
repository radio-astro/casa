from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.hif.heuristics.findrefant as findrefant

import itertools
import numpy as np
import math
import scipy as scp
import scipy.optimize as scpo


from pipeline.hif.tasks import gaincal
from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import applycal
from pipeline.hifv.heuristics import getCalFlaggedSoln, getBCalStatistics
from pipeline.hifv.tasks.setmodel.setmodel import find_standards, standard_sources

from pipeline.hifv.tasks.vlautils import VLAUtils

LOG = infrastructure.get_logger(__name__)

class FinalcalsInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class FinalcalsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(FinalcalsResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

class Finalcals(basetask.StandardTaskTemplate):
    Inputs = FinalcalsInputs
    
    def prepare(self):
        
        gtypecaltable = 'finaldelayinitialgain.g'
        ktypecaltable = 'finaldelay.k'
        bpcaltable = 'finalBPcal.b'
        tablebase = 'finalBPinitialgain'
        table_suffix = ['.g','3.g','10.g']
        soltimes = [1.0,3.0,10.0] 
        m = self.inputs.context.observing_run.measurement_sets[0]
        soltimes = [self.inputs.context.evla['msinfo'][m.name].int_time * x for x in soltimes]
        solints = ['int', '3.0s', '10.0s']
        soltime = soltimes[0]
        solint = solints[0]
        
        context = self.inputs.context

        refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis=self.inputs.vis,field=refantfield,geometry=True,flagging=True, intent='', spw='')
        
        RefAntOutput=refantobj.calculate()
        
        refAnt=str(RefAntOutput[0])+','+str(RefAntOutput[1])+','+str(RefAntOutput[2])+','+str(RefAntOutput[3])

        LOG.info("The pipeline will use antenna(s) "+refAnt+" as the reference")

        gtype_delaycal_result = self._do_gtype_delaycal(caltable=gtypecaltable, context=context, refAnt=refAnt)
        
        ktype_delaycal_result = self._do_ktype_delaycal(caltable=ktypecaltable, addcaltable=gtypecaltable, context=context, refAnt=refAnt)

        #Remove the cal tables from the callibrary
        
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=gtypecaltable, interp='', calwt=False)
        context.callibrary._remove(calto, calfrom, context.callibrary._active)

        
        
        LOG.info("Delay calibration complete")


        # Do initial gaincal on BP calibrator then semi-final BP calibration
        gain_solint1 = context.evla['msinfo'][m.name].gain_solint1
        gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[0], addcaltable=ktypecaltable, solint=gain_solint1, context=context, refAnt=refAnt)
        
        bpdgain_touse = tablebase + table_suffix[0]
        
        #Add appropriate temporary tables to the callibrary
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=bpdgain_touse, interp='', calwt=False)
        context.callibrary.add(calto, calfrom)
        
        LOG.info("Initial BP gain calibration complete")
        

        bandpass_result = self._do_bandpass(bpcaltable, context=context, refAnt=refAnt)
        
        #Force calwt for the bp table to be False
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(bpcaltable, interp='linearperobs,linearflag', calwt=True)
        context.callibrary._remove(calto, calfrom, context.callibrary._active)
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(bpcaltable, interp='', calwt=False)
        context.callibrary.add(calto, calfrom)
        
        LOG.info("Bandpass calibration complete")
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable=bpdgain_touse, interp='', calwt=False)
        context.callibrary._remove(calto, calfrom, context.callibrary._active)

        #Derive an average phase solution for the bandpass calibrator to apply
        #to all data to make QA plots easier to interpret.
        
        avgphase_result = self._do_avgphasegaincal('averagephasegain.g', context, refAnt)
        
        
        #In case any antenna is flagged by this process, unflag all solutions
        #in this gain table (if an antenna does exist or has bad solutions from
        #other steps, it will be flagged by those gain tables).
        
        unflag_result = self._do_unflag('averagephasegain.g')
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable='averagephasegain.g', interp='', calwt=False)
        context.callibrary.add(calto, calfrom)

        applycal_result = self._do_applycal(context=context)
        
        calto = callibrary.CalTo(self.inputs.vis)
        calfrom = callibrary.CalFrom(gaintable='averagephasegain.g', interp='', calwt=False)
        context.callibrary._remove(calto, calfrom, context.callibrary._active)
        
        #---------------------------------------------------
        
        calMs = 'finalcalibrators.ms'
        split_result = self._do_split(calMs)

        

        all_sejy_result = self._doall_setjy(calMs)

        powerfit_results = self._do_powerfit()
        
        powerfit_setjy = self._do_powerfitsetjy(calMs, powerfit_results)
        
        
        m = context.observing_run.measurement_sets[0]
        new_gain_solint1 = context.evla['msinfo'][m.name].new_gain_solint1
        phaseshortgaincal_results = self._do_calibratorgaincal(calMs, 'phaseshortgaincal.g', new_gain_solint1, 3.0, 'p', [''], refAnt)
        
        gain_solint2 = context.evla['msinfo'][m.name].gain_solint2
        finalampgaincal_results = self._do_calibratorgaincal(calMs, 'finalampgaincal.g', gain_solint2, 5.0, 'ap', ['phaseshortgaincal.g'], refAnt)
        
        finalphasegaincal_results = self._do_calibratorgaincal(calMs, 'finalphasegaincal.g', gain_solint2, 3.0, 'p', ['finalampgaincal.g'], refAnt)

        return FinalcalsResults()                        

    def analyse(self, results):
	return results
    
    def _do_gtype_delaycal(self, caltable=None, context=None, refAnt=None):
        
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
            refant   = refAnt.lower(),
            scan = delay_scan_select_string,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '',
            antenna = '')

        delaycal_inputs.refant = delaycal_inputs.refant.lower()

        delaycal_task = gaincal.GTypeGaincal(delaycal_inputs)

        return self._executor.execute(delaycal_task)
    
    def _do_ktype_delaycal(self, caltable=None, addcaltable=None, context=None, refAnt=None):
        
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
            refant   = refAnt.lower(),
            minblperant = minBL_for_cal,
            solnorm = False, 
            combine = 'scan',
            intent = '')

        delaycal_inputs.refant = delaycal_inputs.refant.lower()

        delaycal_task = gaincal.KTypeGaincal(delaycal_inputs)

        return self._executor.execute(delaycal_task)
    
    
    def _do_gtype_bpdgains(self, caltable, addcaltable=None, solint='int', context=None, refAnt=None):

        m = context.observing_run.measurement_sets[0]
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        tst_bpass_spw = context.evla['msinfo'][m.name].tst_bpass_spw
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        
        
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
            calmode  = 'p',
            minsnr   = 3.0,
            refant   = refAnt.lower(),
            scan = bandpass_scan_select_string,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '',
            append=False)

        bpdgains_inputs.refant = bpdgains_inputs.refant.lower()

        bpdgains_task = gaincal.GTypeGaincal(bpdgains_inputs)

        return self._executor.execute(bpdgains_task)
    
    def _do_bandpass(self, caltable, context=None, refAnt=None):
        """Run CASA task bandpass"""

        m = context.observing_run.measurement_sets[0]
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal

        #bandtype = 'B'
        bandpass_inputs = bandpass.ChannelBandpass.Inputs(context,
            vis = self.inputs.vis,
            caltable = caltable,
            field = bandpass_field_select_string,
            spw = '',
            intent = '',
            scan = bandpass_scan_select_string,
            solint = 'inf',
            combine = 'scan',
            refant = refAnt.lower(),
            minblperant = minBL_for_cal,
            minsnr = 5.0,
            solnorm = False)

        bandpass_inputs.refant = bandpass_inputs.refant.lower()

        bandpass_task = bandpass.ChannelBandpass(bandpass_inputs)

        return self._executor.execute(bandpass_task, merge=True)  
      
    def _do_avgphasegaincal(self,caltable, context, refAnt):
        
        m = context.observing_run.measurement_sets[0]
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        
        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
        gaincal_inputs = gaincal.GTypeGaincal.Inputs(context,
            vis = self.inputs.vis,
            caltable = caltable,
            field    = bandpass_field_select_string,
            spw      = '',
            solint   = 'inf',
            calmode  = 'p',
            minsnr   = 1.0,
            refant   = refAnt.lower(),
            scan = bandpass_scan_select_string,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan',
            intent = '',
            antenna = '')

        gaincal_inputs.refant = gaincal_inputs.refant.lower()

        gaincal_task = gaincal.GTypeGaincal(gaincal_inputs)

        return self._executor.execute(gaincal_task)
      
    def _do_unflag(self,gaintable):
        
        task_args = {'vis'          : gaintable,
                     'mode'         : 'unflag',
                     'action'       : 'apply',
                     'display'      : '',
                     'flagbackup'   : False,
                     'savepars'     : False,
                     'async'        : False}
                     
        job = casa_tasks.flagdata(**task_args)
            
        return self._executor.execute(job)
      
    def _do_applycal(self, context=None):
        """Run CASA task applycal"""
        
        m = context.observing_run.measurement_sets[0]
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        
        applycal_inputs = applycal.Applycal.Inputs(context,
            vis = self.inputs.vis,
            field = '',
            spw = '',
            scan = calibrator_scan_select_string,
            intent = '',
            flagbackup = False,
            calwt = False)
        
        applycal_task = applycal.Applycal(applycal_inputs)
        
        return self._executor.execute(applycal_task)
    
    def _do_split(self, calMs):
        
        m = self.inputs.context.observing_run.measurement_sets[0]
        channels = self.inputs.context.evla['msinfo'][m.name].channels
        calibrator_scan_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string
    
        task_args = {'vis'          : m.name,
                     'outputvis'    : calMs,
                     'datacolumn'   : 'corrected',
                     'field'        : '',
                     'spw'          : '',
                     'width'        : int(max(channels)),
                     'antenna'      : '',
                     'timebin'      : '0s',
                     'timerange'    : '',
                     'scan'         : calibrator_scan_select_string,
                     'intent'       : '',
                     'array'        : '',
                     'uvrange'      : '',
                     'correlation'  : '',
                     'observation'  : '',
                     'keepflags'    : False}
        
        job = casa_tasks.split(**task_args)
            
        return self._executor.execute(job)
    
    def _doall_setjy(self, calMs):
        
        context = self.inputs.context
        m = context.observing_run.measurement_sets[0]
        field_spws = context.evla['msinfo'][m.name].field_spws
        spw2band = context.evla['msinfo'][m.name].spw2band
        bands = spw2band.values()
        
        standard_source_names, standard_source_fields = standard_sources(calMs)

        #Look in spectral window domain object as this information already exists!
        with casatools.TableReader(self.inputs.vis+'/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
            originalBBClist = table.getcol('BBC_NO')
            spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
            reference_frequencies = table.getcol('REF_FREQUENCY')
    
        center_frequencies = map(lambda rf, spwbw: rf + spwbw/2, reference_frequencies, spw_bandwidths)

        vlainputs = VLAUtils.Inputs(context)
        vlautils = VLAUtils(vlainputs)
        
        for i, fields in enumerate(standard_source_fields):
            for myfield in fields:
                spws = field_spws[myfield]
                #spws = [1,2,3]
                for myspw in spws:
                    reference_frequency = center_frequencies[myspw]
                    try:
                        EVLA_band = spw2band[myspw]
                    except:
                        LOG.info('Unable to get band from spw id - using reference frequency instead')
                        EVLA_band = vlautils.find_EVLA_band(reference_frequency)
                    
                    LOG.info("Center freq for spw "+str(myspw)+" = "+str(reference_frequency)+", observing band = "+EVLA_band)
                    
                    model_image = standard_source_names[i] + '_' + EVLA_band + '.im'
                    
                    LOG.info("Setting model for field "+str(myfield)+" spw "+str(myspw)+" using "+model_image)

                    #Double check, but the fluxdensity=-1 should not matter since
                    #  the model image take precedence
                    try:
                        setjy_result = self._do_setjy(calMs, str(myfield), str(myspw), model_image, -1)
                        #result.measurements.update(setjy_result.measurements)
                    except Exception, e:
                        # something has gone wrong, return an empty result
                        LOG.error('Unable to complete flux scaling operation')
                        LOG.exception(e)
                        
        
        return True
    
    
    def _do_setjy(self, calMs, field, spw, model_image, fluxdensity):
        
        try:
            task_args = {'vis'            : calMs,
                         'field'          : field,
                         'spw'            : spw,
                         'selectdata'     : False,
                         'model'          : model_image,
                         'listmodels'     : False,
                         'scalebychan'    : True,
                         'fluxdensity'    : -1,
                         'standard'       : 'Perley-Butler 2010',
                         'usescratch'     : True,
                         'async'          : False}
        
            job = casa_tasks.setjy(**task_args)
            
            return self._executor.execute(job)
        except Exception, e:
            print(e)
            return None
    
    def _do_powerfit(self):
        
        context=self.inputs.context
        
        vlainputs = VLAUtils.Inputs(context)
        vlautils = VLAUtils(vlainputs)
        
        m = context.observing_run.measurement_sets[0]
        field_spws = context.evla['msinfo'][m.name].field_spws
        sources = context.evla['msinfo'][m.name].fluxscale_sources
        flux_densities = context.evla['msinfo'][m.name].fluxscale_flux_densities
        spws = context.evla['msinfo'][m.name].fluxscale_spws
        spw2band = context.evla['msinfo'][m.name].spw2band
        bands = spw2band.values()

        #Look in spectral window domain object as this information already exists!
        with casatools.TableReader(self.inputs.vis+'/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
            originalBBClist = table.getcol('BBC_NO')
            spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
            reference_frequencies = table.getcol('REF_FREQUENCY')
    
        center_frequencies = map(lambda rf, spwbw: rf + spwbw/2, reference_frequencies, spw_bandwidths)
        
        fitfunc = lambda p, x: p[0] + p[1] * x
        errfunc = lambda p, x, y, err: (y - fitfunc(p, x)) / err
        
        ##try:
        ##    ff = open(fluxscale_output, 'r')
        ##except IOError as err:
        ##    LOG.error(fluxscale_output+" doesn't exist, error: "+err.filename)
            
        # looking for lines like:
        #2012-03-09 21:30:23     INFO    fluxscale::::    Flux density for J1717-3342 in SpW=3 is: 1.94158 +/- 0.0123058 (SNR = 157.777, N= 34)
        # sometimes they look like:
        #2012-03-09 21:30:23     INFO    fluxscale::::    Flux density for J1717-3342 in SpW=0 is:  INSUFFICIENT DATA
        # so watch for that.
        
        ##sources = []
        ##flux_densities = []
        ##spws = []
        
        ##for line in ff:
        ##    if 'Flux density for' in line:
        ##        fields = line[:-1].split()
        ##        if (fields[11] != 'INSUFFICIENT'):
        ##            sources.append(fields[7])
        ##            flux_densities.append([float(fields[11]), float(fields[13])])
        ##            spws.append(int(fields[9].split('=')[1]))
        
        ii = 0
	unique_sources = list(np.unique(sources))
	results = []
	for source in unique_sources:
	    indices = []
	    for ii in range(len(sources)):
		if (sources[ii] == source):
		    indices.append(ii)
		    
            bands_from_spw = []
            
            if bands == []:
                for ii in range(len(indices)):
                    bands.append(vlautils.find_EVLA_band(center_frequencies[spws[indices[ii]]]))
            else:
                for ii in range(len(indices)):
                    bands_from_spw.append(spw2band[spws[indices[ii]]])
                bands = bands_from_spw
		    

	    unique_bands = list(np.unique(bands))
	    for band in unique_bands:
		lfreqs = []
		lfds = []
		lerrs = []
		uspws = []
		
		#Use spw id to band mappings
		if spw2band.values() != []:
		    for ii in range(len(indices)):
		        if spw2band[spws[indices[ii]]] == band:
			    lfreqs.append(math.log10(center_frequencies[spws[indices[ii]]]))
			    lfds.append(math.log10(flux_densities[indices[ii]][0]))
			    lerrs.append((flux_densities[indices[ii]][1])/(flux_densities[indices[ii]][0])/2.303)
			    uspws.append(spws[indices[ii]])
	        
	        #Use frequencies for band mappings
	        if spw2band.values() == []:
	            for ii in range(len(indices)):
		        if vlautils.find_EVLA_band(center_frequencies[spws[indices[ii]]]) == band:
			    lfreqs.append(math.log10(center_frequencies[spws[indices[ii]]]))
			    lfds.append(math.log10(flux_densities[indices[ii]][0]))
			    lerrs.append((flux_densities[indices[ii]][1])/(flux_densities[indices[ii]][0])/2.303)
			    uspws.append(spws[indices[ii]])
		

		if len(lfds) < 2:
		    pfinal = [lfds[0], 0.0]
		    covar = [0.0,0.0]
		else:
		    alfds = scp.array(lfds)
		    alerrs = scp.array(lerrs)
		    alfreqs = scp.array(lfreqs)
		    pinit = [0.0, 0.0]
		    fit_out = scpo.leastsq(errfunc, pinit, args=(alfreqs, alfds, alerrs), full_output=1)
		    pfinal = fit_out[0]
		    covar = fit_out[1]
		aa = pfinal[0]
		bb = pfinal[1]
		reffreq = 10.0**lfreqs[0]/1.0e9
		fluxdensity = 10.0**(aa + bb*lfreqs[0])
		spix = bb
		results.append([ source, uspws, fluxdensity, spix, reffreq ])
		LOG.info(source + ' ' + band + ' fitted spectral index = ' + str(spix))
		LOG.info("Frequency, data, and fitted data:")
		for ii in range(len(lfreqs)):
		    SS = fluxdensity * (10.0**lfreqs[ii]/reffreq/1.0e9)**spix
		    LOG.info('    '+str(10.0**lfreqs[ii]/1.0e9)+'  '+ str(10.0**lfds[ii])+'  '+str(SS))
        
        return results
                
    
    def _do_powerfitsetjy(self, calMs, results):
        
        LOG.info("Setting power-law fit in the model column")
        
        for result in results:
            for spw_i in result[1]:
                
                try:
                    LOG.info('Running setjy on spw '+str(spw_i))
                    task_args = {'vis'            : calMs,
                                 'field'          : str(result[0]),
                                 'spw'            : str(spw_i),
                                 'selectdata'     : False,
                                 'modimage'       : '',
                                 'listmodels' : False,
                                 'scalebychan'    : True,
                                 'fluxdensity'    : [ result[2], 0, 0, 0 ],
                                 'spix'           : result[3],
                                 'reffreq'        : str(result[4])+'GHz',
                                 'standard'       : 'Perley-Butler 2010',
                                 'usescratch'     : False,
                                 'async'          : False}
        
                    job = casa_tasks.setjy(**task_args)
            
                    self._executor.execute(job)
                except Exception, e:
                    print(e)
                
        return True
            
    
    def _do_calibratorgaincal(self, calMs, caltable, solint, minsnr, calmode, gaintablelist, refAnt):
        
        context = self.inputs.context
        
        m = context.observing_run.measurement_sets[0]
        new_gain_solint1 = context.evla['msinfo'][m.name].new_gain_solint1
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal

        #temp_inputs = gaincal.GTypeGaincal.Inputs(self.inputs.context)
        #refant = temp_inputs.refant.lower()
        
        task_args = {'vis'            : calMs,
                     'caltable'       : caltable,
                     'field'          : '',
                     'spw'            : '',
                     'intent'         : '',
                     'selectdata'     : False,
                     'solint'         : solint,
                     'combine'        : 'scan',
                     'preavg'         : -1.0,
                     'refant'         : refAnt.lower(),
                     'minblperant'    : minBL_for_cal,
                     'minsnr'         : minsnr,
                     'solnorm'        : False,
                     'gaintype'       : 'G',
                     'smodel'         : [],
                     'calmode'        : calmode,
                     'append'         : False,
                     'gaintable'      : gaintablelist,
                     'gainfield'      : [''],
                     'interp'         : [''],
                     'spwmap'         : [],
                     'parang'         : False,
                     'async'          : False}
        
        job = casa_tasks.gaincal(**task_args)
        
        return self._executor.execute(job) 
