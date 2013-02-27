from __future__ import absolute_import

import os.path

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from .. import gaincal
from pipeline.hif.tasks.common import commonfluxresults
from . import fluxscale
from pipeline.hif.tasks.setmodel import setjy

LOG = logging.get_logger(__name__)

from pipeline.hif.heuristics import fluxscale as heufluxscale

class GcorFluxscaleInputs(fluxscale.FluxscaleInputs):
    def __init__(self, context, output_dir=None, vis=None, caltable=None,
                 fluxtable=None, reference=None, transfer=None, 
                 refspwmap=None, refintent=None, transintent=None,
                 solint=None, phaseupsolint=None, minsnr=None, refant=None,
                 hm_resolvedcals=None, antenna=None, peak_fraction=None):
        self._init_properties(vars())

    @property
    def solint(self):
        if self._solint is None:
            return 'inf'
        return self._solint
    
    @solint.setter
    def solint(self, value):
        self._solint = value

    @property
    def phaseupsolint(self):
        if self._phaseupsolint is None:
            return 'int'
        return self._phaseupsolint

    @phaseupsolint.setter
    def phaseupsolint(self, value):
        self._phaseupsolint = value

    @property
    def minsnr (self):
        if self._minsnr is None:
            return 2.0
        return self._minsnr

    @minsnr.setter
    def minsnr(self, value):
        self._minsnr = value

    @property
    def refant(self):
        if self._refant is None:
            return ''
        return self._refant

    @refant.setter
    def refant(self, value):
        self._refant = value

    @property
    def hm_resolvedcals(self):
        if self._hm_resolvedcals is None:
            return 'automatic'
        return self._hm_resolvedcals

    @hm_resolvedcals.setter
    def hm_resolvedcals(self, value):
        self._hm_resolvedcals = value

    @property
    def antenna(self):
        if self._antenna is None:
            return ''
        return self._antenna

    @antenna.setter
    def antenna(self, value):
        self._antenna = value

    @property
    def peak_fraction(self):
        if self._peak_fraction is None:
            return 0.2
        return self._peak_fraction

    @peak_fraction.setter
    def peak_fraction(self, value):
        self._peak_fraction = value

        
class GcorFluxscale(basetask.StandardTaskTemplate):
    Inputs = GcorFluxscaleInputs

    def prepare(self, **parameters):
        inputs = self.inputs
        ms = inputs.context.observing_run.get_ms(name=inputs.vis)

        # check that the measurement set does have an amplitude calibrator.
        if inputs.reference == '':
            # No point carrying on if not.
            LOG.error(
              '%s contains no data for AMPLITUDE calibrator' % 
              os.path.basename(inputs.vis))
            result = commonfluxresults.FluxCalibrationResults(fields={}, fields_setjy={})
            return result

        refant = inputs.refant
        if refant == '':
            # get the reference antenna for this measurement set from the 
            # context. This seems to come back as a string containing a ranked
            # list of antenna names, choose the first one.
            refant = ms.reference_antenna
            if refant is None:
                LOG.error('no reference antenna specified and none in context')
                raise Exception,\
                  'no reference antenna specified and none in context'
            refant = refant.split(',')
            refant = refant[0]
        LOG.info('refant:%s' % refant)

        hm_resolvedcals = inputs.hm_resolvedcals
        if hm_resolvedcals == 'automatic':
            # get the antennas to be used in the gaincals, limiting
            # the range if the reference calibrator is resolved.
            antenna = heufluxscale.antenna(ms=ms,
              refsource=inputs.reference, refant=refant,
              peak_frac=inputs.peak_fraction)
	else:
            antenna = inputs.antenna

        # do a phase-only gaincal
        self._do_gaincal(calmode='p', solint=inputs.phaseupsolint, 
          antenna=antenna, refant=refant)

        # now do the amplitude-only gaincal. This will produce the caltable
        # that fluxscale will analyse
        r = self._do_gaincal(calmode='a', solint=inputs.solint,
          antenna=antenna, refant=refant)
        # get the gaincal caltable from the results
        caltable = r.final.pop().gaintable

        # To make the following fluxscale reliable the caltable
        # should contain gains for the the same set of antennas for 
        # each of the amplitude and phase calibrators - looking
        # at each spw separately.
        check_ok = self._check_caltable(caltable=caltable,
          ms=ms, reference=inputs.reference, transfer=inputs.transfer) 

        if check_ok:
            # Schedule a fluxscale job using this caltable. This is the result
            # that contains the flux measurements for the context.
            try:
                result = self._do_fluxscale(caltable)

                # and finally, do a setjy, add its setjy_settings
                # to the main result
                setjy_result = self._do_setjy()
                result.setjy_settings = setjy_result.setjy_settings

            except:
                # something has gone wrong, return an empty result
                result = commonfluxresults.FluxCalibrationResults(fields={},
                  fields_setjy={})

        else:
            result = commonfluxresults.FluxCalibrationResults(fields={}, fields_setjy={})

        return result

    def analyse(self, result):
        return result

    def _check_caltable(self, caltable, ms, reference, transfer):
        """Check that the give caltable is well-formed so that a 'fluxscale'
        will run successfully on it:
          1. Check that the caltable contains results for the reference and
             transfer fields.
          2. For each spectral window:
                For each field find the set of antennas for which the 
                caltable holds  good results. From this derive the
                set of antennas that have good results for all fields.
                Edit the caltable to flag as bad rows with results for 
                antennas outside this set.
        """
        # get the ids of the reference source and phase source(s)
        amp_fieldid = set([field.id for field in ms.fields if
          field.name==reference])
        phase_fieldids = set([field.id for field in ms.fields if
          field.name in transfer])

#        casatools.ca.open(caltable)
#        spw = ca.spw(name=True)
#        ca.spw() does not seem to work. For now use direct table access.
        fluxscale_spwids = []
        with casatools.TableReader(caltable, nomodify=False) as table:
            spwids = table.getcol('SPECTRAL_WINDOW_ID')
            spwids = set(spwids)

            fieldids = table.getcol('FIELD_ID')
            fieldids = set(fieldids)

            # check that fieldids contains the amplitude and phase calibrators
            if fieldids.isdisjoint(amp_fieldid):
                LOG.error(
                  '%s contains no results for amplitude calibrator' % 
                  os.path.basename(caltable))
                return False
            if not fieldids.issuperset(phase_fieldids):
                LOG.error(
                  '%s does not contain results for all phase calibrators' %
                  os.path.basename(caltable))
                return False

            common_antennas = {}
            for spwid in spwids:
                common_antennas[spwid] = None
                for fieldid in fieldids:
                    subtable = table.query('''SPECTRAL_WINDOW_ID==%s &&
                      FIELD_ID==%s && not(all(FLAG))''' % (spwid, fieldid))
                    antennas = subtable.getcol('ANTENNA1')
		    subtable.close()
                    if common_antennas[spwid] is None:
                        common_antennas[spwid] = set(antennas)
                    else:
                        common_antennas[spwid].intersection_update(
                          set(antennas))

            # flag antennas in caltable that do not belong to the 
            # common_antennas set
            for spwid in spwids:
                if common_antennas[spwid] is None or \
                  common_antennas[spwid] == []:
                    LOG.error(
                      'No antennas in common between AMPLITUDE and PHASE calibrators for SpW %s'
                       % spwid)
                    return False
                else:
                    subtable = table.query('''SPECTRAL_WINDOW_ID==%s &&
                      ANTENNA1 not in %s && not(all(FLAG))''' % (spwid,
                      list(common_antennas[spwid])))
                    flag = subtable.getcol('FLAG')
                    flag[:] = True
                    subtable.putcol('FLAG', flag)
                    fluxscale_spwids.append(spwid)

        return True
                                
    def _do_gaincal(self, calmode=None, solint=None, antenna=None,
      refant=None):
        inputs = self.inputs

        task_args = {
          'output_dir'  : inputs.output_dir,
          'vis'         : inputs.vis,
          'intent'      : inputs.transintent + ',' + inputs.refintent,
          'solint'      : solint,
          'calmode'     : calmode,
          'minsnr'      : inputs.minsnr,
          'combine'     : '',
          'refant'      : refant,
          'antenna'     : antenna,
          'minblperant' : 2,
          'solnorm'     : False,
        }

        task_inputs = gaincal.GTypeGaincal.Inputs(inputs.context, **task_args)
        task = gaincal.GTypeGaincal(task_inputs)

        return self._executor.execute(task, merge=True)

    def _do_fluxscale(self, caltable=None):
        inputs = self.inputs
        
        task_args = {'output_dir' : inputs.output_dir,
          'vis'        : inputs.vis,
          'caltable'   : caltable,
          'reference'  : inputs.reference,
          'transfer'   : inputs.transfer,
          'refspwmap'  : inputs.refspwmap  }
        
        task_inputs = fluxscale.Fluxscale.Inputs(inputs.context, **task_args)
        task = fluxscale.Fluxscale(task_inputs)
        
        return self._executor.execute(task, merge=True)

    def _do_setjy(self):
        inputs = self.inputs
        
        task_args = {
          'output_dir' : inputs.output_dir,
          'vis'        : inputs.vis,
          'field'      : inputs.transfer,
          'intent'     : inputs.transintent,
        }

        task_inputs = setjy.Setjy.Inputs(inputs.context, **task_args)
        task = setjy.Setjy(task_inputs)
        
        return self._executor.execute(task, merge=True)
        
