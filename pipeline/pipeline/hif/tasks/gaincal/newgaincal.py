from __future__ import absolute_import

import types
from . import common
from . import gaincalworker
from . import gaincalmode
from pipeline.hif.heuristics import caltable as gcaltable
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)



class NewGaincalInputs(gaincalmode.GaincalModeInputs):
    def __init__(self, context, mode='gtype',  calphasetable=None,
                 amptable=None, targetphasetable=None, calsolint=None,
                 targetsolint=None, calminsnr=None, targetminsnr=None, 
                 **parameters):
        super(NewGaincalInputs, self).__init__(context, mode,
            calphasetable=calphasetable, amptable=amptable,
            targetphasetable=targetphasetable,
            calsolint=calsolint, targetsolint=targetsolint,
            calminsnr=calminsnr, targetminsnr=targetminsnr, **parameters)

    @property
    def calphasetable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('calphasetable')
        if self._calphasetable is None:
            return gcaltable.GaincalCaltable()
        return self._calphasetable

    @calphasetable.setter
    def calphasetable(self, value):
        self._calphasetable = value

    @property
    def targetphasetable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('targetphasetable')
        if self._targetphasetable is None:
            return gcaltable.GaincalCaltable()
        return self._targetphasetable


    @targetphasetable.setter
    def targetphasetable(self, value):
        self._targetphasetable = value

    @property
    def amptable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('amptable')
        if self._amptable is None:
            return gcaltable.GaincalCaltable()
        return self._amptable

    @amptable.setter
    def amptable(self, value):
        self._amptable = value


    @property
    def calsolint(self):
        return self._calsolint

    @calsolint.setter
    def calsolint(self, value):
        if value is None:
            value = 'int'
        self._calsolint = value

    @property
    def calminsnr(self):
        return self._calminsnr

    @calminsnr.setter
    def calminsnr(self, value):
        if value is None:
            value = 2.0
        self._calminsnr = value

    @property
    def targetsolint(self):
        return self._targetsolint

    @targetsolint.setter
    def targetsolint(self, value):
        if value is None:
            value = 'inf'
        self._targetsolint = value

    @property
    def targetminsnr(self):
        return self._targetminsnr

    @targetminsnr.setter
    def targetminsnr(self, value):
        if value is None:
            value = 3.0
        self._targetminsnr = value

class NewGaincal(gaincalworker.GaincalWorker):
    Inputs = NewGaincalInputs

    def prepare(self, **parameters):

        # Create a results object.
        result = common.GaincalResults() 

        # Compute the science target phase solution
        targetphaseresult = self._do_scitarget_phasecal()

        # Adopt the target phase result
        result.pool.extend(targetphaseresult.pool)
        result.final.extend(targetphaseresult.final)

        # Compute the calibrator target phase solution
        # A local merge to context is done here.
        calphaseresult = self._do_caltarget_phasecal()
	# Readjust to the true calto.intent
        calphaseresult.pool[0].calto.intent = 'AMPLITUDE,BANDPASS'
        calphaseresult.final[0].calto.intent = 'AMPLITUDE,BANDPASS'

        # Accept calphase result as is.
        result.pool.extend(calphaseresult.pool)
        result.final.extend(calphaseresult.final)

        # Compute the amplitude calibration
        ampresult = self._do_target_ampcal()

        # Accept the ampitude result as is.
        result.pool.extend(ampresult.pool)
        result.final.extend(ampresult.final)

        return result

    def analyse(self, result):
        # With no best caltable to find, our task is simply to set the one
        # caltable as the best result

        # double-check that the caltable was actually generated
        on_disk = [table for table in result.pool
                   if table.exists() or self._executor._dry_run]
        result.final[:] = on_disk

        missing = [table for table in result.pool
                   if table not in on_disk and not self._executor._dry_run]     
        result.error.clear()
        result.error.update(missing)

        return result
    
    def _do_caltarget_phasecal(self):
        self.inputs.solint=self.inputs.calsolint
        self.inputs.minsnr=self.inputs.calminsnr
        self.inputs.calmode='p'
	# temporarily set this
        self.inputs.to_intent = 'PHASE,TARGET,AMPLITUDE,BANDPASS'
        #self.inputs.to_intent = 'AMPLITUDE,BANDPASS'
        self.inputs.to_field = None
        self.inputs.caltable=self.inputs.calphasetable
        
        gaincal_task = gaincalmode.GaincalMode(self.inputs)
        result = self._executor.execute(gaincal_task, merge=True)
        
        return result

    def _do_scitarget_phasecal(self):
        inputs = self.inputs
        inputs.solint = inputs.targetsolint
        inputs.minsnr = inputs.targetminsnr
        inputs.calmode = 'p'
        inputs.to_intent = 'PHASE,TARGET'
        inputs.caltable = inputs.targetphasetable

        gaincal_task = gaincalmode.GaincalMode(inputs)
        result = self._executor.execute(gaincal_task)
        
        return result

    def _do_target_ampcal(self):
        self.inputs.solint='inf'
        self.inputs.minsnr=self.inputs.targetminsnr
        self.inputs.calmode='a'
        self.inputs.to_intent = 'PHASE,TARGET,AMPLITUDE,BANDPASS'
        self.inputs.to_field = None
        self.inputs.caltable=self.inputs.amptable
        
        gaincal_task = gaincalmode.GaincalMode(self.inputs)
        result =  self._executor.execute(gaincal_task)
        
        return result
