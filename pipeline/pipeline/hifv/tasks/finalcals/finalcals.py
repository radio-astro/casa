from __future__ import absolute_import

import math
import os

import numpy as np
import scipy as scp

import pipeline.hif.heuristics.findrefant as findrefant
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from pipeline.hifv.heuristics import find_EVLA_band
from pipeline.hifv.heuristics import standard as standard
from pipeline.hifv.heuristics import weakbp, do_bandpass
from pipeline.hifv.tasks.setmodel.vlasetjy import standard_sources
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class FinalcalsInputs(vdp.StandardInputs):
    weakbp = vdp.VisDependentProperty(default=False)
    refantignore = vdp.VisDependentProperty(default='')

    def __init__(self, context, vis=None, weakbp=None, refantignore=None):
        super(FinalcalsInputs, self).__init__()
        self.context = context
        self.vis = vis
        self._weakbp = weakbp
        self.refantignore = refantignore


class FinalcalsResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None, vis=None, bpdgain_touse=None,
                 gtypecaltable=None, ktypecaltable=None, bpcaltable=None,
                 phaseshortgaincaltable=None, finalampgaincaltable=None,
                 finalphasegaincaltable=None):

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []

        super(FinalcalsResults, self).__init__()

        self.vis = vis
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.bpdgain_touse = bpdgain_touse
        self.gtypecaltable = gtypecaltable
        self.ktypecaltable = ktypecaltable
        self.bpcaltable = bpcaltable
        self.phaseshortgaincaltable = phaseshortgaincaltable
        self.finalampgaincaltable = finalampgaincaltable
        self.finalphasegaincaltable = finalphasegaincaltable

    def merge_with_context(self, context):
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)


@task_registry.set_equivalent_casa_task('hifv_finalcals')
class Finalcals(basetask.StandardTaskTemplate):
    Inputs = FinalcalsInputs

    def prepare(self):

        self.parang = True

        try:
            stage_number = self.inputs.context.results[-1].read()[0].stage_number + 1
        except Exception as e:
            stage_number = self.inputs.context.results[-1].read().stage_number + 1

        tableprefix = os.path.basename(self.inputs.vis) + '.' + 'hifv_finalcals.s'

        gtypecaltable = tableprefix + str(stage_number) + '_1.' + 'finaldelayinitialgain.tbl'
        ktypecaltable = tableprefix + str(stage_number) + '_2.' + 'finaldelay.tbl'
        bpcaltable    = tableprefix + str(stage_number) + '_4.' + 'finalBPcal.tbl'
        tablebase     = tableprefix + str(stage_number) + '_3.' + 'finalBPinitialgain'
        table_suffix = ['.tbl', '3.tbl', '10.tbl']
        soltimes = [1.0, 3.0, 10.0]
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        soltimes = [m.get_vla_max_integration_time() * x for x in soltimes]
        solints = ['int', '3.0s', '10.0s']
        soltime = soltimes[0]
        solint = solints[0]

        context = self.inputs.context
        refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis=self.inputs.vis, field=refantfield,
                                                geometry=True, flagging=True, intent='',
                                                spw='', refantignore=self.inputs.refantignore)

        RefAntOutput = refantobj.calculate()

        refAnt = ','.join(RefAntOutput)

        LOG.info("The pipeline will use antenna(s) " + refAnt + " as the reference")

        gtype_delaycal_result = self._do_gtype_delaycal(caltable=gtypecaltable, context=context, refAnt=refAnt)

        ktype_delaycal_result = self._do_ktype_delaycal(caltable=ktypecaltable,
                                                        addcaltable=gtypecaltable, context=context, refAnt=refAnt)

        LOG.info("Delay calibration complete")

        # Do initial gaincal on BP calibrator then semi-final BP calibration
        gain_solint1 = context.evla['msinfo'][m.name].gain_solint1
        gtype_gaincal_result = self._do_gtype_bpdgains(tablebase + table_suffix[0], addcaltable=ktypecaltable,
                                                       solint=gain_solint1, context=context, refAnt=refAnt)

        bpdgain_touse = tablebase + table_suffix[0]
        LOG.info("Initial BP gain calibration complete")

        if self.inputs.weakbp:
            # LOG.info("USING WEAKBP HEURISTICS")
            interp = weakbp(self.inputs.vis, bpcaltable, context=context, RefAntOutput=RefAntOutput,
                            ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse, solint='inf', append=False)
        else:
            # LOG.info("Using REGULAR heuristics")
            interp = ''
            bandpass_job = do_bandpass(self.inputs.vis, bpcaltable, context=context, RefAntOutput=RefAntOutput,
                                       spw='',
                                       ktypecaltable=ktypecaltable, bpdgain_touse=bpdgain_touse, solint='inf',
                                       append=False)
            self._executor.execute(bandpass_job)

        LOG.info("Bandpass calibration complete")

        # Derive an average phase solution for the bandpass calibrator to apply
        # to all data to make QA plots easier to interpret.

        refantmode = 'flex'
        intents = list(m.intents)
        if [intent for intent in intents if 'POL' in intent]:
            # set to strict
            refantmode = 'strict'

        avgpgain = tableprefix + str(stage_number) + '_5.' + 'averagephasegain.tbl'

        avgphase_result = self._do_avgphasegaincal(avgpgain, context, refAnt,
                                                   ktypecaltable=ktypecaltable, bpcaltable=bpcaltable,
                                                   refantmode=refantmode)

        # In case any antenna is flagged by this process, unflag all solutions
        # in this gain table (if an antenna does exist or has bad solutions from
        # other steps, it will be flagged by those gain tables).

        unflag_result = self._do_unflag(avgpgain)

        applycal_result = self._do_applycal(context=context, ktypecaltable=ktypecaltable,
                                            bpcaltable=bpcaltable, avgphasegaincaltable=avgpgain, interp=interp)

        # ---------------------------------------------------

        calMs = 'finalcalibrators.ms'
        split_result = self._do_split(calMs)

        field_spws = m.get_vla_field_spws()

        all_sejy_result = self._doall_setjy(calMs, field_spws)

        if self.inputs.context.evla['msinfo'][m.name].fbversion == 'fb1':
            LOG.info("Using power-law fit results from original hifv_fluxboot task.")
            powerfit_results = self._do_powerfit(field_spws)
            powerfit_setjy = self._do_powerfitsetjy1(calMs, powerfit_results)
            LOG.info("Using power-law fits results from fluxscale and hifv_fluxboot2 task.")
        if self.inputs.context.evla['msinfo'][m.name].fbversion == 'fb2':
            powerfit_setjy = self._do_powerfitsetjy2(calMs)

        new_gain_solint1 = context.evla['msinfo'][m.name].new_gain_solint1
        phaseshortgaincaltable = tableprefix + str(stage_number) + '_6.' + 'phaseshortgaincal.tbl'
        phaseshortgaincal_results = self._do_calibratorgaincal(calMs, phaseshortgaincaltable,
                                                               new_gain_solint1, 3.0, 'p', [''], refAnt,
                                                               refantmode=refantmode)

        gain_solint2 = context.evla['msinfo'][m.name].gain_solint2
        finalampgaincaltable = tableprefix + str(stage_number) + '_7.' +'finalampgaincal.tbl'
        finalampgaincal_results = self._do_calibratorgaincal(calMs, finalampgaincaltable, gain_solint2, 5.0,
                                                             'ap', [phaseshortgaincaltable], refAnt,
                                                             refantmode=refantmode)



        finalphasegaincaltable = tableprefix + str(stage_number) + '_8.' + 'finalphasegaincal.tbl'
        finalphasegaincal_results = self._do_calibratorgaincal(calMs, finalphasegaincaltable, gain_solint2,
                                                               3.0, 'p', [finalampgaincaltable], refAnt,
                                                               refantmode=refantmode)

        tablesToAdd = [(ktypecaltable, '', ''), (bpcaltable, 'linear,linearflag', ''),
                       (avgpgain, '', ''), (finalampgaincaltable, '', ''),
                       (finalphasegaincaltable, '', '')]
        # tablesToAdd = [(table, interp, gainfield) for table, interp, gainfield in tablesToAdd]

        callist = []
        for addcaltable, interp, gainfield in tablesToAdd:
            LOG.info("Finalcals stage:  Adding " + addcaltable + " to callibrary.")
            calto = callibrary.CalTo(self.inputs.vis)
            calfrom = callibrary.CalFrom(gaintable=addcaltable, interp=interp, calwt=False,
                                         caltype='finalcal', gainfield=gainfield)
            calapp = callibrary.CalApplication(calto, calfrom)
            callist.append(calapp)

        return FinalcalsResults(vis=self.inputs.vis, pool=callist, final=callist,
                                bpdgain_touse=bpdgain_touse, gtypecaltable=gtypecaltable,
                                ktypecaltable=ktypecaltable, bpcaltable=bpcaltable,
                                phaseshortgaincaltable=phaseshortgaincaltable,
                                finalampgaincaltable=finalampgaincaltable,
                                finalphasegaincaltable=finalphasegaincaltable)

    def analyse(self, results):
        return results

    def _do_gtype_delaycal(self, caltable=None, context=None, refAnt=None):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = m.get_vla_tst_delay_spw()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = m.vla_minbaselineforcal()

        delaycal_task_args = {'vis': self.inputs.vis,
                              'caltable': caltable,
                              'field': delay_field_select_string,
                              'spw': tst_delay_spw,
                              'intent': '',
                              'selectdata': True,
                              'uvrange': '',
                              'scan': delay_scan_select_string,
                              'solint': 'int',
                              'combine': 'scan',
                              'preavg': -1.0,
                              'refant': refAnt.lower(),
                              'minblperant': minBL_for_cal,
                              'minsnr': 3.0,
                              'solnorm': False,
                              'gaintype': 'G',
                              'smodel': [],
                              'calmode': 'p',
                              'append': False,
                              'docallib': False,
                              'gaintable': sorted(self.inputs.context.callibrary.active.get_caltable()),
                              'gainfield': [''],
                              'interp': [''],
                              'spwmap': [],
                              'parang': self.parang}

        job = casa_tasks.gaincal(**delaycal_task_args)

        return self._executor.execute(job)

    def _do_ktype_delaycal(self, caltable=None, addcaltable=None, context=None, refAnt=None):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = m.get_vla_tst_delay_spw()
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = m.vla_minbaselineforcal()

        GainTables = sorted(self.inputs.context.callibrary.active.get_caltable())
        GainTables.append(addcaltable)

        delaycal_task_args = {'vis': self.inputs.vis,
                              'caltable': caltable,
                              'field': delay_field_select_string,
                              'spw': '',
                              'intent': '',
                              'selectdata': True,
                              'uvrange': '',
                              'scan': delay_scan_select_string,
                              'solint': 'inf',
                              'combine': 'scan',
                              'preavg': -1.0,
                              'refant': refAnt.lower(),
                              'minblperant': minBL_for_cal,
                              'minsnr': 3.0,
                              'solnorm': False,
                              'gaintype': 'K',
                              'smodel': [],
                              'calmode': 'p',
                              'append': False,
                              'docallib': False,
                              'gaintable': GainTables,
                              'gainfield': [''],
                              'interp': [''],
                              'spwmap': [],
                              'parang': self.parang}

        job = casa_tasks.gaincal(**delaycal_task_args)

        return self._executor.execute(job)

    def _do_gtype_bpdgains(self, caltable, addcaltable=None, solint='int', context=None, refAnt=None):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        tst_bpass_spw = m.get_vla_tst_bpass_spw()
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = m.vla_minbaselineforcal()

        GainTables = sorted(self.inputs.context.callibrary.active.get_caltable())
        GainTables.append(addcaltable)

        bpdgains_task_args = {'vis': self.inputs.vis,
                              'caltable': caltable,
                              'field': '',
                              'spw': tst_bpass_spw,
                              'intent': '',
                              'selectdata': True,
                              'uvrange': '',
                              'scan': bandpass_scan_select_string,
                              'solint': solint,
                              'combine': 'scan',
                              'preavg': -1.0,
                              'refant': refAnt.lower(),
                              'minblperant': minBL_for_cal,
                              'minsnr': 3.0,
                              'solnorm': False,
                              'gaintype': 'G',
                              'smodel': [],
                              'calmode': 'p',
                              'append': False,
                              'docallib': False,
                              'gaintable': GainTables,
                              'gainfield': [''],
                              'interp': [''],
                              'spwmap': [],
                              'parang': self.parang}

        job = casa_tasks.gaincal(**bpdgains_task_args)

        return self._executor.execute(job)

    def _do_avgphasegaincal(self, caltable, context, refAnt, ktypecaltable=None, bpcaltable=None,
                            refantmode='flex'):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
        bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
        minBL_for_cal = m.vla_minbaselineforcal()

        AllCalTables = sorted(self.inputs.context.callibrary.active.get_caltable())
        AllCalTables.append(ktypecaltable)
        AllCalTables.append(bpcaltable)

        avgphasegaincal_task_args = {'vis': self.inputs.vis,
                                     'caltable': caltable,
                                     'field': bandpass_field_select_string,
                                     'spw': '',
                                     'selectdata': True,
                                     'uvrange': '',
                                     'scan': bandpass_scan_select_string,
                                     'solint': 'inf',
                                     'combine': 'scan',
                                     'preavg': -1.0,
                                     'refant': refAnt.lower(),
                                     'minblperant': minBL_for_cal,
                                     'minsnr': 1.0,
                                     'solnorm': False,
                                     'gaintype': 'G',
                                     'smodel': [],
                                     'calmode': 'p',
                                     'append': False,
                                     'docallib': False,
                                     'gaintable': AllCalTables,
                                     'gainfield': [''],
                                     'interp': [''],
                                     'spwmap': [],
                                     'parang': self.parang,
                                     'refantmode': refantmode}

        job = casa_tasks.gaincal(**avgphasegaincal_task_args)

        return self._executor.execute(job)

    def _do_unflag(self, gaintable):

        task_args = {'vis': gaintable,
                     'mode': 'unflag',
                     'action': 'apply',
                     'display': '',
                     'flagbackup': False,
                     'savepars': False}

        job = casa_tasks.flagdata(**task_args)

        return self._executor.execute(job)

    def _do_applycal(self, context=None, ktypecaltable=None, bpcaltable=None, avgphasegaincaltable=None, interp=None):
        """Run CASA task applycal"""

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string

        AllCalTables = sorted(self.inputs.context.callibrary.active.get_caltable())
        AllCalTables.append(ktypecaltable)
        AllCalTables.append(bpcaltable)
        AllCalTables.append(avgphasegaincaltable)

        ntables = len(AllCalTables)

        applycal_task_args = {'vis': self.inputs.vis,
                              'field': '',
                              'spw': '',
                              'intent': '',
                              'selectdata': True,
                              'scan': calibrator_scan_select_string,
                              'docallib': False,
                              'gaintable': AllCalTables,
                              'gainfield': [''],
                              'interp': [interp],
                              'spwmap': [],
                              'calwt': [False] * ntables,
                              'parang': self.parang,
                              'applymode': 'calflagstrict',
                              'flagbackup': True}

        job = casa_tasks.applycal(**applycal_task_args)

        return self._executor.execute(job)

    def _do_split(self, calMs):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        channels = m.get_vla_numchan()
        calibrator_scan_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string

        task_args = {'vis': m.name,
                     'outputvis': calMs,
                     'datacolumn': 'corrected',
                     'keepmms': True,
                     'field': '',
                     'spw': '',
                     # 'width': int(max(channels)),
                     'width': 1,
                     'antenna': '',
                     'timebin': '0s',
                     'timerange': '',
                     'scan': calibrator_scan_select_string,
                     'intent': '',
                     'array': '',
                     'uvrange': '',
                     'correlation': '',
                     'observation': '',
                     'keepflags': False}

        job = casa_tasks.split(**task_args)

        return self._executor.execute(job)

    def _doall_setjy(self, calMs, field_spws):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        spw2band = m.get_vla_spw2band()
        bands = spw2band.values()

        standard_source_names, standard_source_fields = standard_sources(calMs)

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

                        LOG.info(
                            "Setting model for field " + str(myfield) + " spw " + str(myspw) + " using " + model_image)

                        # Double check, but the fluxdensity=-1 should not matter since
                        #  the model image take precedence
                        try:
                            job = self._do_setjy(calMs, str(myfield), str(myspw), model_image, -1)
                            jobs.append(job)
                            # result.measurements.update(setjy_result.measurements)
                        except Exception, e:
                            # something has gone wrong, return an empty result
                            LOG.warn(
                                "SetJy issue with field id=" + str(job.kw['field']) + " and spw=" + str(job.kw['spw']))
                            # LOG.exception(e)

                    LOG.info("Merging flux scaling operation for setjy jobs for " + self.inputs.vis)
                    jobs_and_components = utils.merge_jobs(jobs, casa_tasks.setjy, merge=('spw',))
                    for job, _ in jobs_and_components:
                        try:
                            self._executor.execute(job)
                        except Exception, e:
                            LOG.warn(
                                "SetJy issue with field id=" + str(job.kw['field']) + " and spw=" + str(job.kw['spw']))
                            # LOG.exception(e)

        return True

    def _do_setjy(self, calMs, field, spw, model_image, fluxdensity):

        try:
            task_args = {'vis': calMs,
                         'field': field,
                         'spw': spw,
                         'selectdata': False,
                         'model': model_image,
                         'listmodels': False,
                         'scalebychan': True,
                         'fluxdensity': -1,
                         'standard': standard.Standard()(field),
                         'usescratch': True}

            job = casa_tasks.setjy(**task_args)

            return job
        except Exception, e:
            LOG.info(str(e))
            return None

    def _do_powerfit(self, field_spws):

        context = self.inputs.context

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        # field_spws = m.get_vla_field_spws()
        sources = context.evla['msinfo'][m.name].fluxscale_sources
        flux_densities = context.evla['msinfo'][m.name].fluxscale_flux_densities
        spws = context.evla['msinfo'][m.name].fluxscale_spws
        fluxscale_result = context.evla['msinfo'][m.name].fluxscale_result
        spw2band = m.get_vla_spw2band()
        bands = spw2band.values()

        # Look in spectral window domain object as this information already exists!
        with casatools.TableReader(self.inputs.vis + '/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
            originalBBClist = table.getcol('BBC_NO')
            spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
            reference_frequencies = table.getcol('REF_FREQUENCY')

        center_frequencies = map(lambda rf, spwbw: rf + spwbw / 2, reference_frequencies, spw_bandwidths)

        fitfunc = lambda p, x: p[0] + p[1] * x
        errfunc = lambda p, x, y, err: (y - fitfunc(p, x)) / err

        ##try:
        ##    ff = open(fluxscale_output, 'r')
        ##except IOError as err:
        ##    LOG.error(fluxscale_output+" doesn't exist, error: "+err.filename)

        # looking for lines like:
        # 2012-03-09 21:30:23     INFO    fluxscale::::    Flux density for J1717-3342 in SpW=3 is: 1.94158 +/- 0.0123058 (SNR = 157.777, N= 34)
        # sometimes they look like:
        # 2012-03-09 21:30:23     INFO    fluxscale::::    Flux density for J1717-3342 in SpW=0 is:  INSUFFICIENT DATA
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
                    bands.append(find_EVLA_band(center_frequencies[spws[indices[ii]]]))
            else:
                for ii in range(len(indices)):
                    bands_from_spw.append(spw2band[spws[indices[ii]]])
                bands = bands_from_spw

            unique_bands = list(np.unique(bands))

            fieldobject = m.get_fields(source)
            fieldid = str([str(f.id) for f in fieldobject if str(f.id) in fluxscale_result.keys()][0])

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

                if len(lfds) < 2:
                    pfinal = [lfds[0], 0.0]
                    covar = [0.0, 0.0]
                    aa = lfds[0]
                    bb = 0.0
                    SNR = 0.0
                    bberr = 0.0
                else:
                    alfds = scp.array(lfds)
                    alerrs = scp.array(lerrs)
                    alfreqs = scp.array(lfreqs)
                    pinit = [0.0, 0.0]
                    # fit_out = scpo.leastsq(errfunc, pinit, args=(alfreqs, alfds, alerrs), full_output=1)
                    # pfinal = fit_out[0]
                    # covar = fit_out[1]
                # aa = pfinal[0]
                # bb = pfinal[1]

                aa = fluxscale_result[fieldid]['spidx'][0]
                bb = fluxscale_result[fieldid]['spidx'][1]
                bberr = fluxscale_result[fieldid]['spidxerr'][1]

                # reffreq = 10.0**lfreqs[0]/1.0e9
                # fluxdensity = 10.0**(aa + bb*lfreqs[0])
                # spix = bb

                freqs = fluxscale_result['freq']
                fitflx = fluxscale_result[fieldid]['fitFluxd']
                fitreff = fluxscale_result[fieldid]['fitRefFreq']
                spidx = fluxscale_result[fieldid]['spidx']
                # fittedfluxd = []

                freqs = freqs[uspws]
                freqs.sort()

                fittedfluxd = map(
                    lambda x: 10.0 ** (
                        spidx[0] + spidx[1] * math.log10(x / fitreff) + spidx[2] * (math.log10(x / fitreff)) ** 2),
                    freqs)

                reffreq = fitreff / 1.e9
                fluxdensity = fitflx
                spix = bb
                spixerr = bberr

                results.append([source, uspws, fluxdensity, spix, reffreq])
                LOG.info(source + ' ' + band + ' fitted spectral index = ' + str(spix))
                LOG.info("Frequency, data, and fitted data:")

                # Sort arrays based on frequency
                lfreqs_orig = lfreqs
                lfreqs, lfds = zip(*sorted(zip(lfreqs, lfds)))
                lfreqs_orig, lerrs = zip(*sorted(zip(lfreqs_orig, lerrs)))

                for ii in range(len(freqs)):
                    # SS = fluxdensity * (10.0**lfreqs[ii]/reffreq/1.0e9)**spix
                    SS = fittedfluxd[ii]
                    freq = freqs[ii] / 1.e9
                    # LOG.info('    '+str(10.0**lfreqs[ii]/1.0e9)+'  '+ str(10.0**lfds[ii])+'  '+str(SS))
                    LOG.info('    ' + str(freq) + '  ' + str(10.0 ** lfds[ii]) + '  ' + str(SS))

        return results

    def _do_powerfitsetjy1(self, calMs, results):

        LOG.info("Setting power-law fit in the model column")

        for result in results:
            jobs_calMs = []
            for spw_i in result[1]:

                try:
                    LOG.info('Running setjy on spw ' + str(spw_i))
                    task_args = {'vis': calMs,
                                 'field': str(result[0]),
                                 'spw': str(spw_i),
                                 'selectdata': False,
                                 'modimage': '',
                                 'listmodels': False,
                                 'scalebychan': True,
                                 'fluxdensity': [result[2], 0, 0, 0],
                                 'spix': result[3],
                                 'reffreq': str(result[4]) + 'GHz',
                                 'standard': 'manual',
                                 'usescratch': True}

                    # job = casa_tasks.setjy(**task_args)
                    jobs_calMs.append(casa_tasks.setjy(**task_args))

                    # self._executor.execute(job)
                except Exception, e:
                    LOG.info(e)

            # merge identical jobs into one job with a multi-spw argument
            LOG.info("Merging setjy jobs for calibrators.ms")
            jobs_and_components_calMs = utils.merge_jobs(jobs_calMs, casa_tasks.setjy, merge=('spw',))
            for job, _ in jobs_and_components_calMs:
                self._executor.execute(job)

        return True

    def _do_powerfitsetjy2(self, calMs):

        LOG.info("Setting power-law fit in the model column")

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)

        fluxscale_result = self.inputs.context.evla['msinfo'][m.name].fluxscale_result
        dictkeys = fluxscale_result.keys()
        keys_to_remove = ['freq', 'spwName', 'spwID']
        dictkeys = [field_id for field_id in dictkeys if field_id not in keys_to_remove]

        for fieldid in dictkeys:
            jobs_calMs = []

            try:
                LOG.info('Running setjy for field ' + str(fieldid) + ': ' + str(fluxscale_result[fieldid]['fieldName']))
                task_args = {'vis': calMs,
                             'field': fluxscale_result[fieldid]['fieldName'],
                             'spw': ','.join([str(spw) for spw in list(fluxscale_result['spwID'])]),
                             'selectdata': False,
                             'model': '',
                             'listmodels': False,
                             'scalebychan': True,
                             'fluxdensity': [fluxscale_result[fieldid]['fitFluxd'], 0, 0, 0],
                             'spix': list(fluxscale_result[fieldid]['spidx'][1:3]),
                             'reffreq': str(fluxscale_result[fieldid]['fitRefFreq']) + 'Hz',
                             'standard': 'manual',
                             'usescratch': True}

                # job = casa_tasks.setjy(**task_args)
                jobs_calMs.append(casa_tasks.setjy(**task_args))

            except Exception, e:
                LOG.info(e)

            # merge identical jobs into one job with a multi-spw argument
            LOG.info("Merging setjy jobs for calibrators.ms")
            jobs_and_components_calMs = utils.merge_jobs(jobs_calMs, casa_tasks.setjy, merge=('spw',))
            for job, _ in jobs_and_components_calMs:
                self._executor.execute(job)

        return True

    def _do_calibratorgaincal(self, calMs, caltable, solint, minsnr, calmode, gaintablelist, refAnt, refantmode='flex'):

        context = self.inputs.context

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        minBL_for_cal = m.vla_minbaselineforcal()

        task_args = {'vis': calMs,
                     'caltable': caltable,
                     'field': '',
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
                     'parang': self.parang,
                     'refantmode' : refantmode}

        job = casa_tasks.gaincal(**task_args)

        return self._executor.execute(job)
