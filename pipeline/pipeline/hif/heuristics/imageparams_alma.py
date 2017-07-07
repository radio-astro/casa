import os.path
import decimal
import math
import numpy as np
import re
import types
import collections

import cleanhelper

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.contfilehandler as contfilehandler
import pipeline.domain.measures as measures
from .imageparams_base import ImageParamsHeuristics

LOG = infrastructure.get_logger(__name__)


class ImageParamsHeuristicsALMA(ImageParamsHeuristics):

    def __init__(self, vislist, spw, observing_run, imagename_prefix='', proj_params=None, contfile=None, linesfile=None):
        ImageParamsHeuristics.__init__(self, vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile)
        self.imaging_mode = 'ALMA'

    def robust(self, beam):

        '''Adjustment of robust parameter based on desired resolutions.
           beam is the synthesized beam.'''

        cqa = casatools.quanta

        repr_target, repr_source, repr_spw, reprBW_mode, real_repr_target, minAcceptableAngResolution, maxAcceptableAngResolution = self.representative_target()

        # Only apply the robust heuristic if a representative source is defined
        if not real_repr_target:
            LOG.info('ALMA robust heuristics: No representative target found. Using robust=0.5')
            return 0.5

        native_resolution = cqa.getvalue(cqa.convert(beam['minor'], 'arcsec'))

        if native_resolution > cqa.getvalue(cqa.convert(maxAcceptableAngResolution, 'arcsec')):
            robust = -0.5
        elif native_resolution < cqa.getvalue(cqa.convert(minAcceptableAngResolution, 'arcsec')):
            robust = 2.0
        else:
            robust = 0.5

        return robust

    def uvtaper(self, beam_natural):

        '''Adjustment of uvtaper parameter based on desired resolution.'''

        if (beam_natural is None):
            return []

        cqa = casatools.quanta

        repr_target, repr_source, repr_spw, reprBW_mode, real_repr_target, minAcceptableAngResolution, maxAcceptableAngResolution = self.representative_target()

        if not real_repr_target:
            LOG.info('ALMA uvtaper heuristics: No representative target found. Using uvtaper=[]')
            return []

        beam_natural_v = cqa.getvalue(cqa.convert(beam_natural['minor'], 'arcsec'))
        minAR_v = cqa.getvalue(cqa.convert(minAcceptableAngResolution, 'arcsec'))
        maxAR_v = cqa.getvalue(cqa.convert(maxAcceptableAngResolution, 'arcsec'))

        if beam_natural_v < 1.2 * minAR_v:
            beam_taper = math.sqrt(maxAR_v ** 2 - beam_natural_v ** 2)
            uvtaper = ['%.3garcsec' % (beam_taper)]
        else:
            uvtaper = []

        return uvtaper

    def dr_correction(self, threshold, dirty_dynamic_range, residual_max, intent, tlimit):

        '''Adjustment of cleaning threshold due to dynamic range limitations.'''

        qaTool = casatools.quanta
        maxEDR_used = False
        DR_correction_factor = 1.0

        diameter = self.observing_run.get_measurement_sets()[0].antennas[0].diameter
        old_threshold = qaTool.convert(threshold, 'Jy')['value']
        if (intent == 'TARGET' ) or (intent == 'CHECK'):
            n_dr_max = 2.5
            if (diameter == 12.0):
                if (dirty_dynamic_range > 150.):
                    maxSciEDR = 150.0
                    new_threshold = max(n_dr_max * old_threshold, residual_max / maxSciEDR * tlimit)
                    LOG.info('DR heuristic: Applying maxSciEDR(Main array)=%s' % (maxSciEDR))
                    maxEDR_used = True
                else:
                    if (dirty_dynamic_range > 100.):
                        n_dr = 2.5
                    elif (50. < dirty_dynamic_range <= 100.):
                        n_dr = 2.0
                    elif (20. < dirty_dynamic_range <= 50.):
                        n_dr = 1.5
                    elif (dirty_dynamic_range <= 20.):
                        n_dr = 1.0
                    LOG.info('DR heuristic: N_DR=%s' % (n_dr))
                    new_threshold = old_threshold * n_dr
            else:
                if (dirty_dynamic_range > 30.):
                    maxSciEDR = 30.0
                    new_threshold = max(n_dr_max * old_threshold, residual_max / maxSciEDR * tlimit)
                    LOG.info('DR heuristic: Applying maxSciEDR(ACA)=%s' % (maxSciEDR))
                    maxEDR_used = True
                else:
                    if (dirty_dynamic_range > 20.):
                        n_dr = 2.5
                    elif (10. < dirty_dynamic_range <= 20.):
                        n_dr = 2.0
                    elif (4. < dirty_dynamic_range <= 10.):
                        n_dr = 1.5
                    elif (dirty_dynamic_range <= 4.):
                        n_dr = 1.0
                    LOG.info('DR heuristic: N_DR=%s' % (n_dr))
                    new_threshold = old_threshold * n_dr
        else:
            # Calibrators are usually dynamic range limited. The sensitivity from apparentsens
            # is not a valid estimate for the threshold. Use a heuristic based on the dirty peak
            # and some maximum expected dynamic range (EDR) values.
            if (diameter == 12.0):
                maxCalEDR = 1000.0
            else:
                maxCalEDR = 200.0
            LOG.info('DR heuristic: Applying maxCalEDR=%s' % (maxCalEDR))
            new_threshold = max(old_threshold, residual_max / maxCalEDR * tlimit)
            maxEDR_used = True

        if (new_threshold != old_threshold):
            LOG.info('DR heuristic: Modified threshold from %s Jy to %s Jy based on dirty dynamic range calculated from dirty peak / final theoretical sensitivity: %.1f' % (old_threshold, new_threshold, dirty_dynamic_range))
            DR_correction_factor = new_threshold / old_threshold

        return '%sJy' % (new_threshold), DR_correction_factor, maxEDR_used

    def niter_correction(self, niter, cell, imsize, residual_max, threshold):

        '''Adjustment of number of cleaning iterations due to mask size.'''

        qaTool = casatools.quanta

        threshold_value = qaTool.convert(threshold, 'Jy')['value']

        # Compute automatic niter estimate
        old_niter = niter
        kappa = 5
        loop_gain = 0.1
        # TODO: Replace with actual pixel counting rather than assumption about geometry
        r_mask = 0.45 * max(imsize[0], imsize[1]) * qaTool.convert(cell[0], 'arcsec')['value']
        # TODO: Pass synthesized beam size explicitly rather than assuming a
        #       certain ratio of beam to cell size (which can be different
        #       if the product size is being mitigated or if a different
        #       imaging_mode uses different heuristics).
        beam = qaTool.convert(cell[0], 'arcsec')['value'] * 5.0
        new_niter_f = int(kappa / loop_gain * (r_mask / beam) ** 2 * residual_max / threshold_value)
        new_niter = int(round(new_niter_f, -int(np.log10(new_niter_f))))
        if (new_niter != old_niter):
            LOG.info('niter heuristic: Modified niter from %d to %d based on mask vs. beam size heuristic' % (old_niter, new_niter))

        return new_niter

    def get_autobox_params(self):

        '''Default auto-boxing parameters for ALMA main array and ACA.'''

        min_diameter = 1.e9
        for msname in self.vislist:
            min_diameter = min(min_diameter, min([antenna.diameter for antenna in self.observing_run.get_ms(msname).antennas]))
        if min_diameter == 7.0:
            sidelobethreshold = 2.0
            noisethreshold = 5.0
            lownoisethreshold = 2.0
            minbeamfrac = 0.0
        elif min_diameter == 12.0:
            sidelobethreshold = 3.0
            noisethreshold = 5.0
            lownoisethreshold = 1.5
            minbeamfrac = 0.3
        else:
            sidelobethreshold = None
            noisethreshold = None
            lownoisethreshold = None
            minbeamfrac = None

        return sidelobethreshold, noisethreshold, lownoisethreshold, minbeamfrac
