from __future__ import absolute_import
import math

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from recipes import tec_maps
from pipeline.hif.tasks.polarization import polarization
import pipeline.hif.tasks.gaincal as gaincal
import pipeline.hif.heuristics.findrefant as findrefant
import pipeline.infrastructure.callibrary as callibrary

LOG = infrastructure.get_logger(__name__)


class CircfeedpolcalResults(polarization.PolarizationResults):
    def __init__(self, final=[], pool=[], preceding=[], vis=None):
        super(CircfeedpolcalResults, self).__init__()
        self.vis = vis
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

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

        LOG.info("This Circfeedpolcal class is running.")

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        refantfield = self.inputs.context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis=self.inputs.vis, field=refantfield,
                                                geometry=True, flagging=True, intent='', spw='')

        RefAntOutput = refantobj.calculate()
        spwmapGinit = [1, 1, 1]
        spwmapK = [0, 0, 0]

        self.do_gaincal('crosshanddelay.Kx_1', RefAntOutput)

        phasefield = m.get_fields(intent='PHASE')[0]
        phaseid = str(phasefield.id)
        self.do_polcal('leakage.D_1', 'Df+QU', RefAntOutput, phaseid,
                       gainfield=['','','','','','','',phaseid,phaseid,''],
                       spwmap=[[],[],[],[],spwmapK,[],[],[],[],[]])

        ampfield = m.get_fields(intent='AMPLITUDE')[0]
        ampid = str(ampfield.id)
        self.do_polcal('RLphase.X_1', 'Xf', RefAntOutput, ampid,
                       gainfield=['', '', '', '', '', '', '', ampid, ampid, '',''],
                       spwmap=[[], [], [], [], spwmapK, [], [], [], [], [],[]])

        tablesToAdd = (('crosshanddelay.Kx_1', 'kcross'),('leakage.D_1', 'polarization'),('RLphase.X_1', 'polarization'))

        callist = []
        for (addcaltable,caltype) in tablesToAdd:
            calto = callibrary.CalTo(self.inputs.vis)
            calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='', calwt=False, caltype=caltype)
            calapp = callibrary.CalApplication(calto, calfrom)
            callist.append(calapp)

        return CircfeedpolcalResults(vis=self.inputs.vis, pool=callist, final=callist)

    def analyse(self, results):
        return results

    def do_gaincal(self, caltable, RefAntOutput):

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        field = self.inputs.context.evla['msinfo'][m.name].calibrator_field_select_string

        #Similar inputs to hifa/linpolcal.py
        task_inputs = gaincal.GTypeGaincal.Inputs(self.inputs.context,
                                                  output_dir='',
                                                  vis=self.inputs.vis,
                                                  caltable=caltable,
                                                  field=field,
                                                  intent='',
                                                  scan='',
                                                  spw='',
                                                  solint='int',
                                                  gaintype='KCROSS',
                                                  combine='scan',
                                                  refant=RefAntOutput[0].lower(),
                                                  smodel=[1, math.cos(66 * math.pi / 180), math.sin(66 * math.pi / 180), 0])

        gaincal_task = gaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task, merge=True)

        return result

    def do_polcal(self, caltable, poltype, RefAntOutput, field, gainfield=None, spwmap=None):

        GainTables = []

        calto = callibrary.CalTo(self.inputs.vis)
        calstate = self.inputs.context.callibrary.get_calstate(calto)
        merged = calstate.merged()
        for calto, calfroms in merged.items():
            calapp = callibrary.CalApplication(calto, calfroms)
            GainTables.append(calapp.gaintable)

        GainTables = GainTables[0]

        #import pdb
        #pdb.set_trace()

        task_args = {'vis': self.inputs.vis,
                     'caltable'  : caltable,
                     'field'     : field,
                     'refant'    : RefAntOutput[0].lower(),
                     'gaintable' : GainTables,
                     'poltype'   : poltype,
                     'gainfield' : gainfield,
                     'spwmap'    : spwmap,
                     'smodel'    : [1, math.cos(66 * math.pi / 180), math.sin(66 * math.pi / 180), 0]}

        task = casa_tasks.polcal(**task_args)

        result = self._executor.execute(task)

        calfrom = callibrary.CalFrom(gaintable=caltable, interp='', calwt=False, caltype='polarization')
        calto = callibrary.CalTo(self.inputs.vis)
        calapp = callibrary.CalApplication(calto, calfrom)
        self.inputs.context.callibrary.add(calapp.calto, calapp.calfrom)

        return result

