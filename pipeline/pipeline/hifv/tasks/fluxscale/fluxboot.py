from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary

import itertools
import numpy as np
import numpy
import math
import scipy as scp
import scipy.optimize as scpo


from pipeline.hif.tasks import gaincal
from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import applycal
from pipeline.hifv.heuristics import getCalFlaggedSoln, getBCalStatistics
from pipeline.hifv.tasks.setmodel.vlasetjy import find_standards, standard_sources
import pipeline.hif.heuristics.findrefant as findrefant

from pipeline.hifv.tasks.vlautils import VLAUtils

LOG = infrastructure.get_logger(__name__)






class FluxbootInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        self.spix = 0.0
        self.sources = []
        self.flux_densities = []
        self.spws = []


class FluxbootResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[], sources=[], flux_densities=[], spws=[], weblog_results=[],spindex_results=[]):
        super(FluxbootResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.sources = sources
        self.flux_densities = flux_densities
        self.spws = spws
        self.weblog_results = weblog_results
        self.spindex_results = spindex_results

        
    def merge_with_context(self, context):
        """Add results to context for later use in the final calibration
        """
        m = context.observing_run.measurement_sets[0]
        context.evla['msinfo'][m.name].fluxscale_sources = self.sources
        context.evla['msinfo'][m.name].fluxscale_flux_densities = self.flux_densities
        context.evla['msinfo'][m.name].fluxscale_spws = self.spws
        
class Fluxboot(basetask.StandardTaskTemplate):
    Inputs = FluxbootInputs

    def prepare(self):

        #FLUXGAIN stage
        calMs = 'calibrators.ms'

        LOG.info("Setting models for standard primary calibrators")

        standard_source_names, standard_source_fields = standard_sources(calMs)

        context = self.inputs.context
        m = context.observing_run.measurement_sets[0]
        field_spws = context.evla['msinfo'][m.name].field_spws
        new_gain_solint1 = context.evla['msinfo'][m.name].new_gain_solint1
        gain_solint2 = context.evla['msinfo'][m.name].gain_solint2
        spw2band = context.evla['msinfo'][m.name].spw2band
        bands = spw2band.values()

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
                        setjy_result = self._fluxgains_setjy(calMs, str(myfield), str(myspw), model_image, -1)
                        #result.measurements.update(setjy_result.measurements)
                    except Exception, e:
                        # something has gone wrong, return an empty result
                        LOG.error('Unable to complete flux scaling operation for field '+str(myfield)+', spw '+str(myspw))
                        LOG.exception(e)
        
        LOG.info("Making gain tables for flux density bootstrapping")
        LOG.info("Short solint = " + new_gain_solint1)
        LOG.info("Long solint = " + gain_solint2)
        
        refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis='calibrators.ms',field=refantfield,geometry=True,flagging=True, intent='', spw='')
        
        RefAntOutput=refantobj.calculate()
        
        refAnt=str(RefAntOutput[0])+','+str(RefAntOutput[1])+','+str(RefAntOutput[2])+','+str(RefAntOutput[3])
                        
        LOG.info("The pipeline will use antenna(s) "+refAnt+" as the reference")
       
        gaincal_result = self._do_gaincal(context, calMs, 'fluxphaseshortgaincal.g', 'p', [''], solint=new_gain_solint1, minsnr=3.0, refAnt=refAnt)
        
        gaincal_result = self._do_gaincal(context, calMs, 'fluxgaincal.g', 'ap', ['fluxphaseshortgaincal.g'], solint=gain_solint2, minsnr=5.0, refAnt=refAnt)
        
        LOG.info("Gain table fluxgaincal.g is ready for flagging")
        
        
        
        
        
        #---------------------------------------------------------------------
        #Fluxboot stage
        calMs = 'calibrators.ms'
        context = self.inputs.context
        LOG.info("Doing flux density bootstrapping")
        #LOG.info("Flux densities will be written to " + fluxscale_output)
        try:
            fluxscale_result = self._do_fluxscale(context)
            LOG.info("Fitting data with power law")
            powerfit_results, weblog_results, spindex_results = self._do_powerfit(context, fluxscale_result)
            setjy_result = self._do_setjy('calibrators.ms', powerfit_results)
        except Exception as e:
            LOG.warning(e.message)
            LOG.warning("A problem was detected while running fluxscale.  Please review the CASA log.")

        return FluxbootResults(sources=self.inputs.sources, flux_densities=self.inputs.flux_densities, spws=self.inputs.spws, weblog_results=weblog_results, spindex_results=spindex_results)                        





    def analyse(self, results):
        return results
    
 
    def _do_fluxscale(self, context):
        

        m = context.observing_run.measurement_sets[0]
        flux_field_select_string = context.evla['msinfo'][m.name].flux_field_select_string
        fluxcalfields = flux_field_select_string

        task_args = {'vis'          : 'calibrators.ms',
                     'caltable'     : 'fluxgaincal.g',
                     'fluxtable'    : 'fluxgaincalFcal.g',
                     'reference'    : [fluxcalfields],
                     'transfer'     : [''],
                     'append'       : False,
                     'refspwmap'    : [-1],
                     'async'        : False}
                     
        job = casa_tasks.fluxscale(**task_args)
        
        return self._executor.execute(job)

    def _do_powerfit(self, context, fluxscale_result):
        
        vlainputs = VLAUtils.Inputs(context)
        vlautils = VLAUtils(vlainputs)
        
        
        m = context.observing_run.measurement_sets[0]
        field_spws = context.evla['msinfo'][m.name].field_spws
        spw2band = context.evla['msinfo'][m.name].spw2band
        bands = spw2band.values()

        #Look in spectral window domain object as this information already exists!
        with casatools.TableReader(self.inputs.vis+'/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
            originalBBClist = table.getcol('BBC_NO')
            spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
            reference_frequencies = table.getcol('REF_FREQUENCY')
    
        center_frequencies = map(lambda rf, spwbw: rf + spwbw/2, reference_frequencies, spw_bandwidths)
    
        # the variable center_frequencies should already have been filled out
        # with the reference frequencies of the spectral window table
        
        fitfunc = lambda p, x: p[0] + p[1] * x
        errfunc = lambda p, x, y, err: (y - fitfunc(p, x)) / err
        
        #########################################################################
        ##try:
        ##    ff = open(fluxscale_output, 'r')
        ##except IOError as err:
        ##    LOG.fatal(fluxscale_output + " doesn't exist, error: " + err.filename)
        
        # looking for lines like:
        #2012-03-09 21:30:23     INFO    fluxscale::::    Flux density for J1717-3342 in SpW=3 is: 1.94158 +/- 0.0123058 (SNR = 157.777, N= 34)
        # sometimes they look like:
        #2012-03-09 21:30:23     INFO    fluxscale::::    Flux density for J1717-3342 in SpW=0 is:  INSUFFICIENT DATA 
        # so watch for that.
        
        sources = []
        flux_densities = []
        spws = []
        ##for line in ff:
        ##    if 'Flux density for' in line:
        ##        fields = line[:-1].split()
        ##        if (fields[11] != 'INSUFFICIENT'):
        ##            sources.append(fields[7])
        ##            flux_densities.append([float(fields[11]), float(fields[13])])
        ##            spws.append(int(fields[9].split('=')[1]))
        
        #Find the field_ids in the dictionary returned from the CASA task fluxscale
        dictkeys = fluxscale_result.keys()
        keys_to_remove = ['freq', 'spwName', 'spwID']
        dictkeys = [field_id for field_id in dictkeys if field_id not in keys_to_remove]
                
        for field_id in dictkeys:        
            sourcename = fluxscale_result[field_id]['fieldName']
            secondary_keys = fluxscale_result[field_id].keys()
            secondary_keys_to_remove=['fitRefFreq', 'spidxerr', 'spidx', 'fitFluxd', 'fieldName', 'fitFluxdErr']
            spwkeys = [spw_id for spw_id in secondary_keys if spw_id not in secondary_keys_to_remove]
            
            for spw_id in spwkeys:
                flux_d = list(fluxscale_result[field_id][spw_id]['fluxd'])
                flux_d_err = list(fluxscale_result[field_id][spw_id]['fluxdErr'])
                #spwslist  = list(int(spw_id))
                
            
                #flux_d = list(fluxscale_result[field_id]['fluxd'])
                #flux_d_err = list(fluxscale_result[field_id]['fluxdErr'])
                #spwslist  = list(fluxscale_result['spwID'])
        
                for i in range(0,len(flux_d)):
                    if (flux_d[i] != -1.0 and flux_d[i] != 0.0):
                        sources.append(sourcename)
                        flux_densities.append([float(flux_d[i]), float(flux_d_err[i])])
                        spws.append(int(spw_id))
        
        self.inputs.sources = sources
        self.inputs.flux_densities = flux_densities
        self.inputs.spws = spws
        
        ii = 0
        unique_sources = list(np.unique(sources))
        results = []
        weblog_results = []
        spindex_results = []
        
        print 'fluxscale result: ', fluxscale_result
        print 'unique_sources: ', unique_sources
        
        
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
            print unique_bands
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
                # if we didn't care about the errors on the data or the fit coefficients, just:
                #       coefficients = np.polyfit(lfreqs, lfds, 1)
                # or, if we ever get to numpy 1.7.x, for weighted fit, and returning
                # covariance matrix, do:
                #       ...
                #       weights = []
                #       weight_sum = 0.0
                #       for ii in range(len(lfreqs)):
                #           weights.append(1.0 / (lerrs[ii]*lerrs[ii]))
                #           weight_sum += weights[ii]
                #       for ii in range(len(weights)):
                #           weights[ii] /= weight_sum
                #       coefficients = np.polyfit(lfreqs, lfds, 1, w=weights, cov=True)
                # but, for now, use the full scipy.optimize.leastsq route...
                #
                # actually, after a lot of testing, np.polyfit does not return a global
                # minimum solution.  sticking with leastsq (modified as below to get the
                # proper errors), or once we get a modern enough version of scipy, moving
                # to curve_fit, is better.
                #
                
                print lfds
                
                if len(lfds) < 2:
                    aa = lfds[0]
                    bb = 0.0
                    SNR = 0.0
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
        
                    #
                    # the fit is of the form:
                    #     log(S) = a + b * log(f)
                    # with a = pfinal[0] and b = pfinal[1].  the errors on the coefficients are
                    # sqrt(covar[i][i]*residual_variance) with the residual covariance calculated
                    # as below (it's like the reduced chi squared without dividing out the errors).
                    # see the scipy.optimize.leastsq documentation and 
                    # http://stackoverflow.com/questions/14854339/in-scipy-how-and-why-does-curve-fit-calculate-the-covariance-of-the-parameter-es
                    #
                    
                    summed_error = 0.0
                    for ii in range(len(alfds)):
                        model = aa + bb*alfreqs[ii]
                        residual = (model - alfds[ii]) * (model - alfds[ii])
                        summed_error += residual
                    residual_variance = summed_error / (len(alfds) - 2)
                    SNR = math.fabs(bb) / math.sqrt(covar[1][1] * residual_variance)
                    
                #
                # take as the reference frequency the lowest one.  (this shouldn't matter, in principle).
                #    
        
                reffreq = 10.0**lfreqs[0]/1.0e9
                fluxdensity = 10.0**(aa + bb*lfreqs[0])
                spix = bb
                results.append([ source, uspws, fluxdensity, spix, SNR, reffreq ])
                LOG.info(source + ' ' + band + ' fitted spectral index & SNR = ' + str(spix) + ' ' + str(SNR))
                spindex_results.append({'source': source,
                                        'band'  : band,
                                        'spix'  : str(spix),
                                        'SNR'   : str(SNR)})
                LOG.info("Frequency, data, error, and fitted data:")
                for ii in range(len(lfreqs)):
                    SS = fluxdensity * (10.0**lfreqs[ii]/reffreq/1.0e9)**spix
                    fderr = lerrs[ii]*(10**lfds[ii])/math.log10(math.e)
                    LOG.info('    '+str(10.0**lfreqs[ii]/1.0e9)+'  '+ str(10.0**lfds[ii])+'  '+str(fderr)+'  '+str(SS))
                    weblog_results.append({'freq' : str(10.0**lfreqs[ii]/1.0e9),
                                           'data' : str(10.0**lfds[ii]),
                                           'error': str(fderr),
                                           'fitteddata': str(SS)})
        
        self.spix = spix
        
        LOG.info("Setting power-law fit in the model column")
        
        return results, weblog_results, spindex_results
                
    def _do_setjy(self, calMs, results):
        
        for result in results:
            for spw_i in result[1]:
                
                LOG.info('Running setjy on spw '+str(spw_i))
                task_args = {'vis'            : calMs,
                             'field'          : str(result[0]),
                             'spw'            : str(spw_i),
                             'selectdata'     : False,
                             'model'       : '',
                             'listmodels'     : False,
                             'scalebychan'    : True,
                             'fluxdensity'    : [ result[2], 0, 0, 0 ],
                             'spix'           : result[3],
                             'reffreq'        : str(result[5])+'GHz',
                             'standard'       : 'manual',
                             'usescratch'     : True,
                             'async'          : False}
        
                job = casa_tasks.setjy(**task_args)
            
                self._executor.execute(job)
                
                #Run on the ms
                task_args['vis'] = self.inputs.vis
                job = casa_tasks.setjy(**task_args)
                self._executor.execute(job)
                
                if (abs(self.spix) > 5.0):
                    LOG.warn("abs(spix) > 5.0 - Fail")
        
        LOG.info("Flux density bootstrapping finished")
        
        return True
        
    
    def _fluxgains_setjy(self, calMs, field, spw, modimage, fluxdensity):
        
        
        try:
            task_args = {'vis'            : calMs,
                         'field'          : field,
                         'spw'            : spw,
                         'selectdata'     : False,
                         'model'          : modimage,
                         'listmodels'     : False,
                         'scalebychan'    : True,
                         'fluxdensity'    : -1,
                         'standard'       : 'Perley-Butler 2013',
                         'usescratch'     : True,
                         'async'          : False}
        
            job = casa_tasks.setjy(**task_args)
            
            return self._executor.execute(job)
        except Exception, e:
            print(e)
            return None
    
    
    def _do_gaincal(self, context, calMs, caltable, calmode, gaintablelist, solint='int', minsnr=3.0, refAnt=None):
        
        m = context.observing_run.measurement_sets[0]
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        
        #Do this to get the reference antenna string
        #temp_inputs = gaincal.GTypeGaincal.Inputs(context)
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
        
        

