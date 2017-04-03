from __future__ import absolute_import
import math

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from pipeline.hif.tasks.polarization import polarization
import pipeline.hif.tasks.gaincal as gaincal
import pipeline.hif.heuristics.findrefant as findrefant
import pipeline.infrastructure.callibrary as callibrary

LOG = infrastructure.get_logger(__name__)


class CircfeedpolcalResults(polarization.PolarizationResults):
    def __init__(self, final=None, pool=None, preceding=None, vis=None, refant=None, calstrategy=None):

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []
        if refant is None:
            refant = ''
        if calstrategy is None:
            calstrategy = ''

        super(CircfeedpolcalResults, self).__init__()
        self.vis = vis
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.refant = refant
        self.calstrategy = calstrategy

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.warn('No circfeedpolcal results to add to the callibrary')
            return

        for calapp in self.final:
            LOG.debug('Adding pol calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):
        #return 'CircfeedpolcalResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'CircfeedpolcalResults:'


class CircfeedpolcalInputs(polarization.PolarizationInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class Circfeedpolcal(polarization.Polarization):
    Inputs = CircfeedpolcalInputs

    def prepare(self):

        self.callist = []

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        intents = list(m.intents)

        self.RefAntOutput = ['']
        self.calstrategy = ''

        if [intent for intent in intents if 'POL' in intent]:
            self.do_prepare()

        return CircfeedpolcalResults(vis=self.inputs.vis, pool=self.callist, final=self.callist,
                                     refant=self.RefAntOutput[0].lower(), calstrategy=self.calstrategy)

    def analyse(self, results):
        return results

    def do_prepare(self):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        refantfield = self.inputs.context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis=self.inputs.vis, field=refantfield,
                                                geometry=True, flagging=True, intent='', spw='')
        self.RefAntOutput = refantobj.calculate()

        # setjy for amplitude/flux calibrator (VLASS 3C286 or 3C48)
        fluxcal = self._do_setjy()

        tablesToAdd = ((self.inputs.vis + '.kcross', 'kcross'),
                       (self.inputs.vis + '.D2', 'polarization'),
                       (self.inputs.vis + '.X1', 'polarization'))

        # D-terms   - do we need this?
        # self.do_polcal(self.inputs.vis+'.D1', 'D+QU',field='',
        #               intent='CALIBRATE_POL_LEAKAGE#UNSPECIFIED',
        #               gainfield=[''], spwmap=[],
        #               solint='inf')

        # First pass R-L delay
        self.do_gaincal(tablesToAdd[0][0], field=fluxcal)

        # Determine number of scans with POLLEAKGE intent
        polleakagescans = []
        poltype = 'Df+QU'  # Default
        for scan in m.scans:
            if 'POLLEAKAGE' in scan.intents:
                polleakagescans.append((scan.id, scan.intents))

        # Calibration Strategies
        LOG.info("Number of POL_LEAKAGE scans: {!s}".format(len(polleakagescans)))
        self.calstrategy = ''
        if len(polleakagescans) >= 3:
            poltype = 'Df+QU'   # C4
            self.calstrategy = "Using Calibration Strategy C4: 3 or more slices CALIBRATE_POL_LEAKAGE, KCROSS, Df+QU, Xf."
        if len(polleakagescans) < 3:
            poltype = 'Df'      # C1
            self.calstrategy = "Using Calibration Strategy C1: Less than 3 slices CALIBRATE_POL_LEAKAGE, KCROSS, Df, Xf."
        LOG.info(self.calstrategy)

        # D-terms in 16MHz pieces, minsnr of 5.0
        self.do_polcal(tablesToAdd[1][0], poltype=poltype, field='',
                       intent='CALIBRATE_POL_LEAKAGE#UNSPECIFIED',
                       gainfield=[''], spwmap=[], solint='inf,16MHz', minsnr=5.0)

        # 2MHz pieces, minsnr of 3.0
        self.do_polcal(tablesToAdd[2][0], poltype='Xf', field=fluxcal,
                       intent='',
                       gainfield=[''], spwmap=[], solint='inf,2MHz', minsnr=3.0)

        for (addcaltable, caltype) in tablesToAdd:
            calto = callibrary.CalTo(self.inputs.vis)
            calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='', calwt=False, caltype=caltype)
            calapp = callibrary.CalApplication(calto, calfrom)
            self.callist.append(calapp)


    def do_gaincal(self, caltable, field=''):

        # Similar inputs to hifa/linpolcal.py
        task_inputs = gaincal.GTypeGaincal.Inputs(self.inputs.context,
                                                  output_dir='',
                                                  vis=self.inputs.vis,
                                                  caltable=caltable,
                                                  field=field,
                                                  intent='',
                                                  scan='',
                                                  spw='',
                                                  solint='inf',
                                                  gaintype='KCROSS',
                                                  combine='scan',
                                                  refant=self.RefAntOutput[0].lower(),
                                                  parang=True,
                                                  append=False)

        gaincal_task = gaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task, merge=True)

        return result

    def do_polcal(self, caltable, poltype='', field='', intent='',
                  gainfield=[''], spwmap=[], solint='inf', minsnr=5.0):

        GainTables = []

        calto = callibrary.CalTo(self.inputs.vis)
        calstate = self.inputs.context.callibrary.get_calstate(calto)
        merged = calstate.merged()
        for calto, calfroms in merged.items():
            calapp = callibrary.CalApplication(calto, calfroms)
            GainTables.append(calapp.gaintable)

        GainTables = GainTables[0]
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        minBL_for_cal = m.vla_minbaselineforcal()

        # import pdb
        # pdb.set_trace()

        task_args = {'vis': self.inputs.vis,
                     'caltable'   : caltable,
                     'field'      : field,
                     'intent'     : intent,
                     'refant'     : self.RefAntOutput[0].lower(),
                     'gaintable'  : GainTables,
                     'poltype'    : poltype,
                     'gainfield'  : gainfield,
                     'minsnr'     : minsnr,
                     'minblperant': minBL_for_cal,
                     'combine'    : 'scan',
                     'spwmap'     : spwmap,
                     'solint'     : solint}

        task = casa_tasks.polcal(**task_args)

        result = self._executor.execute(task)

        calfrom = callibrary.CalFrom(gaintable=caltable, interp='', calwt=False, caltype='polarization')
        calto = callibrary.CalTo(self.inputs.vis)
        calapp = callibrary.CalApplication(calto, calfrom)
        self.inputs.context.callibrary.add(calapp.calto, calapp.calfrom)

        return result

    def _do_setjy(self):
        '''
        The code in this private class method are (for now) specific to VLASS
        requirements and heuristics.

        Returns: string name of the amplitude flux calibrator

        '''

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)

        fluxfields = m.get_fields(intent='AMPLITUDE')
        fluxfieldnames = [amp.name for amp in fluxfields]

        fluxcal = ''
        for field in fluxfieldnames:
            if fluxcal == '' and ('3C286' in field):
                fluxcal = field
            elif fluxcal == '' and ('1331+3030' in field):
                fluxcal = field
            elif fluxcal == '' and ('3C48' in field):
                fluxcal = field
            elif fluxcal != '' and ('3C48' in field or '3C286' in field):
                LOG.info("Two flux calibrators found, selecting 3C286!")
                if '"1331+305=3C286"' in fluxfieldnames:
                    fluxcal='"1331+305=3C286"'
                if '3C286' in fluxfieldnames:
                    fluxcal = '3C286'
                if '1331+3030' in fluxfieldnames:
                    fluxcal = '1331+3030'

        delmodjob = casa_tasks.delmod(vis=self.inputs.vis, field='')
        self._executor.execute(delmodjob)

        try:
            task_args = {}
            if '3C286' or '1331+3030' in fluxcal:
                d0 = 33.0 * math.pi / 180.0
                task_args = {'vis'           : self.inputs.vis,
                             'field'         : fluxcal,
                             'standard'      : 'manual',
                             'spw'           : '',
                             'fluxdensity'   : [8.30468,0,0,0],
                             'spix'          : [-0.630458,-0.132252],
                             'reffreq'       : '3000.0MHz',
                             'polindex'      : [0.107943,0.01184,-0.0055,0.0224,-0.0312],
                             'polangle'      : [d0,0],
                             'rotmeas'       : 0,
                             'scalebychan'   : True,
                             'usescratch'    : False}

            elif '3C48' in fluxcal:
                task_args = {'vis'           : self.inputs.vis,
                             'field'         : fluxcal,
                             'spw'           : '',
                             'selectdata'    : False,
                             'timerange'     : '',
                             'scan'          : '',
                             'intent'        : '',
                             'observation'   : '',
                             'scalebychan'   : True,
                             'standard'      : 'manual',
                             'model'         : '',
                             'modimage'      : '',
                             'listmodels'    : False,
                             'fluxdensity'   : [6.4861, -0.132, 0.0417, 0],
                             'spix'          : [-0.934677, -0.125579],
                             'reffreq'       : '3000.0MHz',
                             'polindex'      : [0.02143, 0.0392, 0.002349, -0.0230],
                             'polangle'      : [-1.7233, 1.569, -2.282, 1.49],
                             'rotmeas'       : 0,  # inside polangle
                             'fluxdict'      : {},
                             'useephemdir'   : False,
                             'interpolation' : 'nearest',
                             'usescratch'    : False}
            else:
                LOG.error("No known flux calibrator found - please check the data.")

            job = casa_tasks.setjy(**task_args)

            self._executor.execute(job)
        except Exception, e:
            print(e)
            return None

        return fluxcal

