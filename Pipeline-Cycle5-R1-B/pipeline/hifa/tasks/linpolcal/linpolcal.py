from __future__ import absolute_import

import types

import pipeline.hif.tasks.gaincal as gaincal
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as hcaltable
from pipeline.hif.tasks.common import commoncalinputs
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry
from .almapolhelperscopy import *
from .almapolhelpersfuture import *
from .resultobjects import LinpolcalResult

LOG = infrastructure.get_logger(__name__)


class LinpolcalInputs(commoncalinputs.VdpCommonCalibrationInputs):

    @vdp.VisDependentProperty
    def delaytable(self):
        # best to have logic for unset value in the getter, if in the setter
        # you have to avoid race conditions with other setters that this one
        # depends upon
        if self._delaytable is None:
            caltable = hcaltable.DelayCaltable()
            self._delaytable = caltable(output_dir=self.output_dir,
              stage=self.context.stage, vis=self.vis,
              field=self.field, intent=self.intent,
              solint='inf')

        return self._delaytable

    @delaytable.convert
    def delaytable(self, value):
        self._delaytable = value

    @vdp.VisDependentProperty
    def df0table(self):
        # best to have logic for unset value in the getter, if in the setter
        # you have to avoid race conditions with other setters that this one
        # depends upon
        if self._df0table is None:
            caltable = hcaltable.PolCaltable()
            self._df0table = caltable(output_dir=self.output_dir,
              stage=self.context.stage, vis=self.vis, 
              field=self.field, intent=self.intent,
              solint='inf')

        return self._df0table

    @df0table.convert
    def df0table(self, value):
        self._df0table = value

    @vdp.VisDependentProperty
    def g0table(self):
        # best to have logic for unset value in the getter, if in the setter
        # you have to avoid race conditions with other setters that this one
        # depends upon
        if self._g0table is None:
            caltable = hcaltable.GaincalCaltable()
            g0table = caltable(output_dir=self.output_dir,
              stage=self.context.stage, vis=self.vis, 
              field=self.field, intent=self.intent,
              solint=self.gaincalsolint)

            self._g0table = os.path.join(os.path.dirname(g0table),
              'g0.%s' % os.path.basename(g0table))

        return self._g0table

    @g0table.convert
    def g0table(self, value):
        self._g0table = value

    @vdp.VisDependentProperty
    def g1table(self):
        # best to have logic for unset value in the getter, if in the setter
        # you have to avoid race conditions with other setters that this one
        # depends upon
        if self._g1table is None:
            caltable = hcaltable.GaincalCaltable()
            g1table = caltable(output_dir=self.output_dir,
              stage=self.context.stage, vis=self.vis, 
              field=self.field, intent=self.intent,
              solint=self.gaincalsolint)

            self._g1table = os.path.join(os.path.dirname(g1table),
             'g1.%s' % os.path.basename(g1table))

        return self._g1table

    @g1table.convert
    def g1table(self, value):
        self._g1table = value

    @vdp.VisDependentProperty
    def gaincalsolint(self):
        return self._gaincalsolint

    @gaincalsolint.convert
    def gaincalsolint(self, value):
        if value is None:
            value = 'int'
        self._gaincalsolint = value

    @vdp.VisDependentProperty
    def xyf0table(self):
        # best to have logic for unset value in the getter, if in the setter
        # you have to avoid race conditions with other setters that this one
        # depends upon
        if self._xyf0table is None:
            caltable = hcaltable.XYf0Caltable()
            self._xyf0table = caltable(output_dir=self.output_dir,
              stage=self.context.stage, vis=self.vis, 
              field=self.field, intent=self.intent,
              solint='inf')

        return self._xyf0table

    @xyf0table.convert
    def xyf0table(self, value):
        self._xyf0table = value

    def __init__(self, context, output_dir=None, vis=None, g0table=None, delaytable=None,
                 xyf0table=None, g1table=None, df0table=None,
                 field=None, spw=None, antenna=None, intent=None,
                 gaincalsolint=None, minblperant=None, refant=None):
        super(LinpolcalInputs, self).__init__()

        self.context = context
        self.vis = vis
        self.output_dir = output_dir

        self.g0table = g0table
        self.delaytable = delaytable
        self.xyf0table = xyf0table
        self.g1table = g1table
        self.df0table = df0table

        self.intent = intent
        self.field = field
        self.spw = spw
        self.antenna = antenna
        self.refant = refant

        self.gaincalsolint = gaincalsolint
        self.minblperant = minblperant


@task_registry.set_equivalent_casa_task('hifa_linpolcal')
class Linpolcal(basetask.StandardTaskTemplate):
    Inputs = LinpolcalInputs

    is_multi_vis_task = True

    def prepare(self):
        inputs = self.inputs

        # Create a results object.
        result = LinpolcalResult() 

        if type(inputs.vis) == types.ListType:
            raise Exception,\
              'hif_linpolcal can only handle single measurement sets'

        # Compute time-dependent gain over whole timerange
        g0_result = self._do_gaincal(caltable=inputs.g0table)

        # estimate source polarization Q,U from gains
        gqu = qufromgain(caltable=inputs.g0table)
        result.gqu = gqu

        # calibrate cross-hand delay. A significant delay between
        # X and Y will add a slope to the X-Y phase calibration
        # that is done next and will complicate resolution of
        # the X-Y phase ambiguity.

        # Need a heuristic to specify best scan to use
        best_scan = minparallelresponse(caltable=inputs.g0table, gqu=gqu)
        print('best scan {}'.format(best_scan))
        result.best_delay_scan = best_scan
        # best_scan dict is indexed by field id
        ms = inputs.context.observing_run.get_ms(inputs.vis)
        fields = ms.get_fields()
        field_id = [field.id for field in fields if field.name==inputs.field]
        kcrs_result = self._do_delaycal(caltable=inputs.delaytable,
          scan=best_scan[field_id[0]])

        # need to reorder cal application from ..., B, G, K to ..., B, K, G.
        calto = callibrary.get_calto_from_inputs(inputs)
        calstate = inputs.context.callibrary.get_calstate(calto)
        # should only be 1 calto - perhaps add an assert?
        calto, calfroms = calstate.merged().items()[0]
        calfrom_k = calfroms.pop()
        calfrom_g = calfroms.pop()

        # clear the calstate and rebuild
        inputs.context.callibrary.clear()
        inputs.context.callibrary.add(calto, calfroms)
        inputs.context.callibrary.add(calto, calfrom_k)
        inputs.context.callibrary.add(calto, calfrom_g)

        # Estimate the residual X-Y phase spectrum and source Q,U
        ambxyf0table = os.path.join(os.path.dirname(inputs.xyf0table),
          'amb.%s' % os.path.basename(inputs.xyf0table))
        xyf0ambcal_result = self._do_xyf0cal(caltable=ambxyf0table)

        # resolve the ambiguity in X-Y phase - do nothing with the returned
        # table at present as we only seem to be interested in the Stokes
        # vector
        # qu dict is indexed by field id
        result.stokes = xyamb(xytab=ambxyf0table, qu=gqu[field_id[0]],
          xyout=inputs.xyf0table)

        # revise the gain calibration now that we know the source polarization.
        # Again we need to modify the calstate, removing the last 2 cals: K, G.
        calto = callibrary.get_calto_from_inputs(inputs)
        calstate = inputs.context.callibrary.get_calstate(calto)
        calto, calfroms = calstate.merged().items()[0]
        calfrom_xyf0 = calfroms.pop()
        old_gain = calfroms.pop()
        calfrom_k = calfroms.pop()
        inputs.context.callibrary.clear()
        inputs.context.callibrary.add(calto, calfroms)

        g1_result = self._do_gaincal(caltable=inputs.g1table,
          smodel=result.stokes, parang=True)

        # estimate source polarization Q,U from gains
        gqu = qufromgain(caltable=inputs.g1table)

        # finally, calculate the instrumental polarization
        # Again we need to modify the calstate, to ...,B, K, G1, XYf0.
        calto = callibrary.get_calto_from_inputs(inputs)
        calstate = inputs.context.callibrary.get_calstate(calto)
        calto, calfroms = calstate.merged().items()[0]
        calfrom_g1 = calfroms.pop()

        inputs.context.callibrary.clear()
        inputs.context.callibrary.add(calto, calfroms)
        inputs.context.callibrary.add(calto, calfrom_k)
        inputs.context.callibrary.add(calto, calfrom_g1)
        inputs.context.callibrary.add(calto, calfrom_xyf0)

        # calculate instrument pol
        interim_df0table = \
          os.path.join(os.path.dirname(inputs.df0table),
          'interim.%s' % os.path.basename(inputs.df0table))
        polcal_result = self._do_df0cal(caltable=interim_df0table,
          smodel=result.stokes)

        # make D-table general so that it corrects parallel hands also
        Dgen(dtab=interim_df0table, dout=inputs.df0table)

        # modify/contsruct calapplications to match George's example
        # kcrs
        calapp = kcrs_result.final[0]
        newcalfrom = callibrary.CalFrom(calapp.calfrom[0].gaintable,
                                        caltype='unknown',
                                        gainfield='',
                                        interp='nearest')
        kcrscalapp = callibrary.CalApplication(calapp.calto, 
          newcalfrom)
        # XYf0
        calto = callibrary.CalTo(vis=inputs.vis,
                                 spw=inputs.spw)
        calfrom = callibrary.CalFrom(inputs.xyf0table,
                                     caltype='gaincal',
                                     gainfield='',
                                     interp='nearest')
        xyf0calapp = callibrary.CalApplication(calto, calfrom) 
        # construct CalApplication description for Df0
        calto = callibrary.CalTo(vis=inputs.vis,
                                 spw=inputs.spw)
        calfrom = callibrary.CalFrom(inputs.df0table,
                                     caltype='polarization',
                                     gainfield='',
                                     interp='')
        df0calapp = callibrary.CalApplication(calto, calfrom) 

        # construct result.final to hold the CalApplications to be applied
        result.pool = [kcrscalapp,
          g1_result.final[0], xyf0calapp, df0calapp]

        return result

    def analyse(self, result):
        # With no best caltables to find, our task is simply to set the 
        # ones held in pool as the best result - if they exist

        # double-check that the caltables were actually generated
        on_disk = [table for table in result.pool
                   if table.exists() or self._executor._dry_run]
        if on_disk == result.pool:
            result.final[:] = on_disk

        missing = [table for table in result.pool
                   if table not in on_disk and not self._executor._dry_run]     
        result.error.clear()
        result.error.update(missing)

        return result
    
    def _do_delaycal(self, caltable, scan):
        inputs = self.inputs

        task_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
          output_dir=inputs.output_dir,
          vis=inputs.vis,
          caltable=caltable,
          field=inputs.field,
          intent=inputs.intent,
          scan=str(scan),
          spw=inputs.spw,
          solint='inf',
          gaintype='KCROSS',
          refant=inputs.refant,
          smodel=[1,0,1,0])

        gaincal_task = gaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task, merge=True)
        
        return result

    def _do_df0cal(self, caltable, smodel):
        inputs = self.inputs

        # polcal not built in to hif so assemble parameters to
        # preapply calstate
        calto = callibrary.get_calto_from_inputs(inputs)
        calstate = inputs.context.callibrary.get_calstate(calto)
        calto, calfroms = calstate.merged().items()[0]
        calapp = callibrary.CalApplication(calto, calfroms)
        gaintable = calapp.gaintable
        gainfield = calapp.gainfield
        spwmap = calapp.spwmap
        interp = calapp.interp

        polcal_task = casa_tasks.polcal(
          vis=inputs.vis,
          caltable=caltable,
          field=inputs.field,
          intent=inputs.intent,
          spw=inputs.spw,
          smodel=smodel,
          poltype='Dflls',
          solint='inf',
          preavg=300,
          combine='obs,scan',
          gaintable=gaintable,
          gainfield=gainfield,
          interp=interp)

        result = self._executor.execute(polcal_task)

        return result

    def _do_gaincal(self, caltable,smodel=[1,0,0,0], parang=False):
        inputs = self.inputs

        task_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
          output_dir=inputs.output_dir,
          caltable=caltable,
          vis=inputs.vis,
          field=inputs.field,
          intent=inputs.intent,
          spw=inputs.spw,
          solint='int',
          gaintype='G',
          calmode='ap',
          minsnr=0.0,
          combine='',
          refant=inputs.refant,
          minblperant=4,
          solnorm=False,
          smodel=smodel,
          parang=parang)

        gaincal_task = gaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task, merge=True)

        return result

    def _do_xyf0cal(self, caltable):
        inputs = self.inputs

        task_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
          output_dir=inputs.output_dir,
          vis=inputs.vis,
          caltable=caltable,
          field=inputs.field,
          intent=inputs.intent,
          spw=inputs.spw,
          solint='inf',
          gaintype='XYf+QU',
          preavg=300,
          combine='obs,scan',
          refant=inputs.refant,
          smodel=[1,0,1,0])

        gaincal_task = gaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task, merge=True)

        return result
