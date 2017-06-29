from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils

import numpy as np
import math
import scipy as scp
import scipy.optimize as scpo

from pipeline.hifv.heuristics import find_EVLA_band, getCalFlaggedSoln, getBCalStatistics
from pipeline.hifv.tasks.setmodel.vlasetjy import find_standards, standard_sources
import pipeline.hif.heuristics.findrefant as findrefant

LOG = infrastructure.get_logger(__name__)


class Fluxboot2Inputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, caltable=None, refantignore=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        self.spix = 0.0
        self.sources = []
        self.flux_densities = []
        self.spws = []

    @property
    def caltable(self):
        return self._caltable

    @caltable.setter
    def caltable(self, value):
        """
        If a caltable is specified, then the fluxgains stage from the scripted pipeline is skipped
        and we proceed directly to the flux density bootstrapping.
        """
        if value is None:
            value = None
        self._caltable = value

    @property
    def refantignore(self):
        return self._refantignore

    @refantignore.setter
    def refantignore(self, value):
        if value is None:
            value = ''
        self._refantignore = value


class Fluxboot2Results(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None, sources=None,
                 flux_densities=None, spws=None, weblog_results=None, spindex_results=None,
                 vis=None, caltable=None, fluxscale_result=None):

        if sources is None:
            sources = []
        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []
        if flux_densities is None:
            flux_densities = []
        if spws is None:
            spws = []
        if weblog_results is None:
            weblog_results = []
        if spindex_results is None:
            spindex_results = []
        if caltable is None:
            caltable = ''
        if fluxscale_result is None:
            fluxscale_result = {}

        super(Fluxboot2Results, self).__init__()
        self.vis = vis
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.sources = sources
        self.flux_densities = flux_densities
        self.spws = spws
        self.weblog_results = weblog_results
        self.spindex_results = spindex_results
        self.caltable = caltable
        self.fluxscale_result = fluxscale_result

    def merge_with_context(self, context):
        """Add results to context for later use in the final calibration
        """
        m = context.observing_run.measurement_sets[0]
        context.evla['msinfo'][m.name].fluxscale_sources = self.sources
        context.evla['msinfo'][m.name].fluxscale_flux_densities = self.flux_densities
        context.evla['msinfo'][m.name].fluxscale_spws = self.spws
        context.evla['msinfo'][m.name].fluxscale_result = self.fluxscale_result


class Fluxboot2(basetask.StandardTaskTemplate):
    Inputs = Fluxboot2Inputs

    def prepare(self):

        calMs = 'calibrators.ms'
        context = self.inputs.context

        if (self.inputs.caltable is None):
            # FLUXGAIN stage

            caltable = 'fluxgaincal.g'

            LOG.info("Setting models for standard primary calibrators")

            standard_source_names, standard_source_fields = standard_sources(calMs)

            m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
            field_spws = m.get_vla_field_spws()
            new_gain_solint1 = context.evla['msinfo'][m.name].new_gain_solint1
            gain_solint2 = context.evla['msinfo'][m.name].gain_solint2
            spw2band = m.get_vla_spw2band()
            bands = spw2band.values()

            # Look in spectral window domain object as this information already exists!
            with casatools.TableReader(self.inputs.vis + '/SPECTRAL_WINDOW') as table:
                channels = table.getcol('NUM_CHAN')
                originalBBClist = table.getcol('BBC_NO')
                spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
                reference_frequencies = table.getcol('REF_FREQUENCY')

            center_frequencies = map(lambda rf, spwbw: rf + spwbw / 2, reference_frequencies, spw_bandwidths)

            for i, fields in enumerate(standard_source_fields):
                for myfield in fields:
                    domainfield = m.get_fields(myfield)[0]
                    if 'AMPLITUDE' in domainfield.intents:
                        spws = field_spws[myfield]
                        # spws = [1,2,3]
                        jobs = []
                        for myspw in spws:
                            reference_frequency = center_frequencies[myspw]
                            try:
                                EVLA_band = spw2band[myspw]
                            except:
                                LOG.info('Unable to get band from spw id - using reference frequency instead')
                                EVLA_band = find_EVLA_band(reference_frequency)

                            LOG.info("Center freq for spw " + str(myspw) + " = " + str(
                                reference_frequency) + ", observing band = " + EVLA_band)

                            model_image = standard_source_names[i] + '_' + EVLA_band + '.im'

                            LOG.info("Setting model for field " + str(m.get_fields()[myfield].id) + " spw " + str(
                                myspw) + " using " + model_image)

                            # Double check, but the fluxdensity=-1 should not matter since
                            #  the model image take precedence
                            try:
                                job = self._fluxgains_setjy(calMs, str(m.get_fields()[myfield].id), str(myspw),
                                                            model_image, -1)
                                jobs.append(job)

                                # result.measurements.update(setjy_result.measurements)
                            except Exception, e:
                                # something has gone wrong, return an empty result
                                LOG.error('Unable merge setjy jobs for flux scaling operation for field ' + str(
                                    myfield) + ', spw ' + str(myspw))
                                LOG.exception(e)

                        LOG.info("Merging flux scaling operation for setjy jobs for " + self.inputs.vis)
                        jobs_and_components = utils.merge_jobs(jobs, casa_tasks.setjy, merge=('spw',))
                        for job, _ in jobs_and_components:
                            try:
                                self._executor.execute(job)
                            except Exception, e:
                                LOG.warn("SetJy issue with field id=" + str(job.kw['field']) + " and spw=" + str(
                                    job.kw['spw']))
                                # LOG.exception(e)

            LOG.info("Making gain tables for flux density bootstrapping")
            LOG.info("Short solint = " + new_gain_solint1)
            LOG.info("Long solint = " + gain_solint2)

            refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
            refantobj = findrefant.RefAntHeuristics(vis=calMs, field=refantfield,
                                                    geometry=True, flagging=True, intent='',
                                                    spw='', refantignore=self.inputs.refantignore)

            RefAntOutput = refantobj.calculate()

            refAnt = ','.join([str(i) for i in RefAntOutput[0:4]])

            LOG.info("The pipeline will use antenna(s) " + refAnt + " as the reference")

            gaincal_result = self._do_gaincal(context, calMs, 'fluxphaseshortgaincal.g', 'p', [''],
                                              solint=new_gain_solint1, minsnr=3.0, refAnt=refAnt)

            # ----------------------------------------------------------------------------
            # New Heuristics, CAS-9186
            field_objects = m.get_fields(intent=['AMPLITUDE', 'BANDPASS', 'PHASE'])

            # run gaincal with solnorm=True per calibrator field, pre-applying
            # short-solint phase solution and setting append=True for all fields
            # after the first, to obtain (temporary) scan-averaged, normalized
            # amps for flagging, fluxflag.g
            fluxflagtable = 'fluxflag.g'
            for field in field_objects:
                gaincal_result = self._do_gaincal(context, calMs, fluxflagtable, 'ap', ['fluxphaseshortgaincal.g'],
                                                  solint=gain_solint2, minsnr=5.0, refAnt=refAnt, field=field.name)

            # use flagdata to clip fluxflag.g outside the range 0.9-1.1
            flagjob = casa_tasks.flagdata(vis=fluxflagtable, mode='clip', correlation='ABS_ALL',
                                          datacolumn='CPARAM', clipminmax=[0.9,1.1], clipoutside=True,
                                          action='apply', flagbackup=False, savepars=False)
            self._executor.execute(flagjob)

            # use applycal to apply fluxflag.g to calibrators.ms, applymode='flagonlystrict
            applycaljob = casa_tasks.applycal(vis=calMs, field="", spw="", intent="",
                                              selectdata=False, docallib=False, gaintable=[fluxflagtable],
                                              gainfield=[''], interp=[''], spwmap=[], calwt=[False], parang=False,
                                              applymode='flagonlystrict', flagbackup=True)

            # -------------------------------------------------------------------------------


            gaincal_result = self._do_gaincal(context, calMs, caltable, 'ap', ['fluxphaseshortgaincal.g'],
                                              solint=gain_solint2, minsnr=5.0, refAnt=refAnt)

            LOG.info("Gain table " + caltable + " is ready for flagging.")
        else:
            caltable = self.inputs.caltable
            LOG.warn("Caltable " + caltable + " has been flagged and will be used in the flux density bootstrapping.")

        # ---------------------------------------------------------------------
        # Fluxboot stage

        LOG.info("Doing flux density bootstrapping using caltable " + caltable)
        # LOG.info("Flux densities will be written to " + fluxscale_output)
        try:
            fluxscale_result = self._do_fluxscale(context, calMs, caltable)
            LOG.info("Fitting data with power law")
            powerfit_results, weblog_results, spindex_results = self._do_powerfit(context, fluxscale_result)
            setjy_result = self._do_setjy(calMs, powerfit_results)
        except Exception as e:
            LOG.warning(e.message)
            LOG.warning("A problem was detected while running fluxscale.  Please review the CASA log.")
            powerfit_results = []
            weblog_results = []
            spindex_results = []

        return Fluxboot2Results(sources=self.inputs.sources, flux_densities=self.inputs.flux_densities,
                               spws=self.inputs.spws, weblog_results=weblog_results,
                               spindex_results=spindex_results, vis=self.inputs.vis, caltable=caltable,
                               fluxscale_result=fluxscale_result)

    def analyse(self, results):
        return results

    def _do_fluxscale(self, context, calMs, caltable):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        flux_field_select_string = context.evla['msinfo'][m.name].flux_field_select_string
        fluxcalfields = flux_field_select_string

        task_args = {'vis': calMs,
                     'caltable': caltable,
                     'fluxtable': 'fluxgaincalFcal.g',
                     'reference': [fluxcalfields],
                     'transfer': [''],
                     'append': False,
                     'refspwmap': [-1]}

        job = casa_tasks.fluxscale(**task_args)

        return self._executor.execute(job)

    def _do_powerfit(self, context, fluxscale_result):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        field_spws = m.get_vla_field_spws()
        spw2band = m.get_vla_spw2band()
        bands = spw2band.values()

        # Look in spectral window domain object as this information already exists!
        with casatools.TableReader(self.inputs.vis + '/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
            originalBBClist = table.getcol('BBC_NO')
            spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
            reference_frequencies = table.getcol('REF_FREQUENCY')

        center_frequencies = map(lambda rf, spwbw: rf + spwbw / 2, reference_frequencies, spw_bandwidths)

        # the variable center_frequencies should already have been filled out
        # with the reference frequencies of the spectral window table

        fitfunc = lambda p, x: p[0] + p[1] * x
        errfunc = lambda p, x, y, err: (y - fitfunc(p, x)) / err

        #########################################################################
        # Old method of parsing fluxscale results from the CASA log
        ##try:
        ##    ff = open(fluxscale_output, 'r')
        ##except IOError as err:
        ##    LOG.fatal(fluxscale_output + " doesn't exist, error: " + err.filename)

        # looking for lines like:
        # 2012-03-09 21:30:23     INFO    fluxscale::::    Flux density for J1717-3342 in SpW=3 is: 1.94158 +/- 0.0123058 (SNR = 157.777, N= 34)
        # sometimes they look like:
        # 2012-03-09 21:30:23     INFO    fluxscale::::    Flux density for J1717-3342 in SpW=0 is:  INSUFFICIENT DATA
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

        # Find the field_ids in the dictionary returned from the CASA task fluxscale
        dictkeys = fluxscale_result.keys()
        keys_to_remove = ['freq', 'spwName', 'spwID']
        dictkeys = [field_id for field_id in dictkeys if field_id not in keys_to_remove]

        for field_id in dictkeys:
            sourcename = fluxscale_result[field_id]['fieldName']
            secondary_keys = fluxscale_result[field_id].keys()
            secondary_keys_to_remove = ['fitRefFreq', 'spidxerr', 'spidx', 'fitFluxd', 'fieldName', 'fitFluxdErr']
            spwkeys = [spw_id for spw_id in secondary_keys if spw_id not in secondary_keys_to_remove]

            for spw_id in spwkeys:
                flux_d = list(fluxscale_result[field_id][spw_id]['fluxd'])
                flux_d_err = list(fluxscale_result[field_id][spw_id]['fluxdErr'])
                # spwslist  = list(int(spw_id))


                # flux_d = list(fluxscale_result[field_id]['fluxd'])
                # flux_d_err = list(fluxscale_result[field_id]['fluxdErr'])
                # spwslist  = list(fluxscale_result['spwID'])

                for i in range(0, len(flux_d)):
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

        # print 'fluxscale result: ', fluxscale_result
        # print 'unique_sources: ', unique_sources

        for source in unique_sources:
            indices = []
            for ii in range(len(sources)):
                if (sources[ii] == source):
                    indices.append(ii)

            bands_from_spw = []

            if bands == []:
                for ii in range(len(indices)):
                    bands.append(find_EVLA_band(center_frequencies[spws[indices[ii]]]))
            else:
                for ii in range(len(indices)):
                    bands_from_spw.append(spw2band[spws[indices[ii]]])
                bands = bands_from_spw

            unique_bands = list(np.unique(bands))
            print unique_bands

            fieldobject = m.get_fields(source)
            fieldid = str(fieldobject[0].id)

            for band in unique_bands:
                lfreqs = []
                lfds = []
                lerrs = []
                uspws = []

                # Use spw id to band mappings
                if spw2band.values() != []:
                    for ii in range(len(indices)):
                        if spw2band[spws[indices[ii]]] == band:
                            lfreqs.append(math.log10(center_frequencies[spws[indices[ii]]]))
                            lfds.append(math.log10(flux_densities[indices[ii]][0]))
                            lerrs.append((flux_densities[indices[ii]][1]) / (flux_densities[indices[ii]][0]) / 2.303)
                            uspws.append(spws[indices[ii]])

                # Use frequencies for band mappings
                if spw2band.values() == []:
                    for ii in range(len(indices)):
                        if find_EVLA_band(center_frequencies[spws[indices[ii]]]) == band:
                            lfreqs.append(math.log10(center_frequencies[spws[indices[ii]]]))
                            lfds.append(math.log10(flux_densities[indices[ii]][0]))
                            lerrs.append((flux_densities[indices[ii]][1]) / (flux_densities[indices[ii]][0]) / 2.303)
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
                    bberr = 0.0
                else:
                    alfds = scp.array(lfds)
                    alerrs = scp.array(lerrs)
                    alfreqs = scp.array(lfreqs)
                    pinit = [0.0, 0.0]
                    ## fit_out = scpo.leastsq(errfunc, pinit, args=(alfreqs, alfds, alerrs), full_output=1)
                    ## pfinal = fit_out[0]
                    ## covar = fit_out[1]

                    # aa = pfinal[0]
                    # bb = pfinal[1]



                    # Use result from fluxscale, not from fitting function
                    aa = fluxscale_result[fieldid]['spidx'][0]
                    bb = fluxscale_result[fieldid]['spidx'][1]
                    bberr = fluxscale_result[fieldid]['spidxerr'][1]

                    #
                    # the fit is of the form:
                    #     log(S) = a + b * log(f)
                    # with a = pfinal[0] and b = pfinal[1].  the errors on the coefficients are
                    # sqrt(covar[i][i]*residual_variance) with the residual covariance calculated
                    # as below (it's like the reduced chi squared without dividing out the errors).
                    # see the scipy.optimize.leastsq documentation and
                    # http://stackoverflow.com/questions/14854339/in-scipy-how-and-why-does-curve-fit-calculate-the-covariance-of-the-parameter-es
                    #

                    '''
                    summed_error = 0.0
                    for ii in range(len(alfds)):
                        model = aa + bb * alfreqs[ii]
                        residual = (model - alfds[ii]) * (model - alfds[ii])
                        summed_error += residual
                    residual_variance = summed_error / (len(alfds) - 2)
                    SNR = math.fabs(bb) / math.sqrt(covar[1][1] * residual_variance)
                    '''
                    SNR = 0.0

                #
                # take as the reference frequency the lowest one.  (this shouldn't matter, in principle).
                #

                freqs = fluxscale_result['freq']
                fitflx = fluxscale_result[fieldid]['fitFluxd']
                fitreff = fluxscale_result[fieldid]['fitRefFreq']
                spidx = fluxscale_result[fieldid]['spidx']
                # fittedfluxd = []

                freqs = freqs[spws]
                freqs.sort()

                fittedfluxd = map(
                    lambda x: 10.0 ** (
                    spidx[0] + spidx[1] * math.log10(x / fitreff) + spidx[2] * (math.log10(x / fitreff)) ** 2),
                    freqs)

                reffreq = fitreff/1.e9
                fluxdensity = fitflx
                spix = bb
                spixerr = bberr
                results.append([source, uspws, fluxdensity, spix, SNR, reffreq])
                LOG.info(source + ' ' + band + ' fluxscale fitted spectral index = ' + str(spix) + '+/-' + str(spixerr))
                spindex_results.append({'source': source,
                                        'band': band,
                                        'spix': str(spix),
                                        'spixerr': str(spixerr),
                                        'SNR': str(SNR)})
                LOG.info("Frequency, data, error, and fitted data:")
                # Sort arrays based on frequency
                lfreqs_orig = lfreqs
                lfreqs, lfds = zip(*sorted(zip(lfreqs, lfds)))
                lfreqs_orig, lerrs = zip(*sorted(zip(lfreqs_orig, lerrs)))

                for ii in range(len(freqs)):
                    SS = fittedfluxd[ii]
                    freq = freqs[ii]/1.e9
                    data = 10.0 ** lfds[ii]

                    # fderr = lerrs[ii] * (10 ** lfds[ii]) / math.log10(math.e)
                    # fitFluxd = 10**a0  #(or 10**spidx[0] in my previous example)
                    # fitFluxdErr = ln(10)*fitFluxd*err_a0
                    fderr = math.log(10) * SS * fluxscale_result[fieldid]['spidxerr'][0]

                    LOG.info('    ' + str(freq) + '  ' + str(data) + '  ' + str(
                        fderr) + '  ' + str(SS))
                    weblog_results.append({'source': source,
                                           'freq': str(freq),
                                           'data': str(data),
                                           'error': str(fderr),
                                           'fitteddata': str(SS)})
                '''
                for ii in range(len(lfreqs)):
                    SS = fluxdensity * (10.0 ** lfreqs[ii] / reffreq / 1.0e9) ** spix

                    import pdb
                    pdb.set_trace()

                    fderr = lerrs[ii] * (10 ** lfds[ii]) / math.log10(math.e)
                    LOG.info('    ' + str(10.0 ** lfreqs[ii] / 1.0e9) + '  ' + str(10.0 ** lfds[ii]) + '  ' + str(
                        fderr) + '  ' + str(SS))
                    weblog_results.append({'source': source,
                                           'freq': str(10.0 ** lfreqs[ii] / 1.0e9),
                                           'data': str(10.0 ** lfds[ii]),
                                           'error': str(fderr),
                                           'fitteddata': str(SS)})
                '''

        self.spix = spix

        LOG.info("Setting fluxscale power-law fit in the model column")

        # Sort weblog results by frequency
        weblog_results = sorted(weblog_results, key=lambda k: (k['source'], k['freq']))

        return results, weblog_results, spindex_results

    def _do_setjy(self, calMs, results):

        for result in results:

            jobs_calMs = []
            jobs_vis = []

            for spw_i in result[1]:

                LOG.info('Running setjy on spw ' + str(spw_i))
                task_args = {'vis': calMs,
                             'field': str(result[0]),
                             'spw': str(spw_i),
                             'selectdata': False,
                             'model': '',
                             'listmodels': False,
                             'scalebychan': True,
                             'fluxdensity': [result[2], 0, 0, 0],
                             'spix': result[3],
                             'reffreq': str(result[5]) + 'GHz',
                             'standard': 'manual',
                             'usescratch': True}

                # job = casa_tasks.setjy(**task_args)
                jobs_calMs.append(casa_tasks.setjy(**task_args))

                # self._executor.execute(job)

                # Run on the ms
                task_args['vis'] = self.inputs.vis
                jobs_vis.append(casa_tasks.setjy(**task_args))
                # job = casa_tasks.setjy(**task_args)
                # self._executor.execute(job)

                if (abs(self.spix) > 5.0):
                    LOG.warn("abs(spix) > 5.0 - Fail")

            # merge identical jobs into one job with a multi-spw argument
            LOG.info("Merging setjy jobs for " + calMs)
            jobs_and_components_calMs = utils.merge_jobs(jobs_calMs, casa_tasks.setjy, merge=('spw',))
            for job, _ in jobs_and_components_calMs:
                self._executor.execute(job)

            LOG.info("Merging setjy jobs for " + self.inputs.vis)
            jobs_and_components_vis = utils.merge_jobs(jobs_vis, casa_tasks.setjy, merge=('spw',))
            for job, _ in jobs_and_components_vis:
                self._executor.execute(job)

        LOG.info("Flux density bootstrapping finished")

        return True

    def _fluxgains_setjy(self, calMs, field, spw, modimage, fluxdensity):

        try:
            task_args = {'vis': calMs,
                         'field': field,
                         'spw': spw,
                         'selectdata': False,
                         'model': modimage,
                         'listmodels': False,
                         'scalebychan': True,
                         'fluxdensity': -1,
                         'standard': 'Perley-Butler 2013',
                         'usescratch': True}

            job = casa_tasks.setjy(**task_args)

            return job
        except Exception, e:
            print(e)
            return None

    def _do_gaincal(self, context, calMs, caltable, calmode, gaintablelist,
                    solint='int', minsnr=3.0, refAnt=None, field=''):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        # minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        minBL_for_cal = m.vla_minbaselineforcal()

        # Do this to get the reference antenna string
        # temp_inputs = gaincal.GTypeGaincal.Inputs(context)
        # refant = temp_inputs.refant.lower()

        task_args = {'vis': calMs,
                     'caltable': caltable,
                     'field': field,
                     'spw': '',
                     'intent': '',
                     'selectdata': False,
                     'solint': solint,
                     'combine': 'scan',
                     'preavg': -1.0,
                     'refant': refAnt.lower(),
                     'minblperant': minBL_for_cal,
                     'minsnr': minsnr,
                     'solnorm': False,
                     'gaintype': 'G',
                     'smodel': [],
                     'calmode': calmode,
                     'append': False,
                     'gaintable': gaintablelist,
                     'gainfield': [''],
                     'interp': [''],
                     'spwmap': [],
                     'parang': True}

        job = casa_tasks.gaincal(**task_args)

        return self._executor.execute(job)
