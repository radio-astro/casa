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

    def __init__(self, vislist, spw, observing_run, imagename_prefix='', science_goals=None, contfile=None, linesfile=None):
        ImageParamsHeuristics.__init__(self, vislist, spw, observing_run, imagename_prefix, science_goals, contfile, linesfile)
        self.imaging_mode = 'ALMA'

    def robust(self, spw):

        '''Adjustment of robust parameter based on desired resolutions.'''

        # Check if there is a non-zero min/max angular resolution
        cqa = casatools.quanta
        minAcceptableAngResolution = cqa.convert(self.science_goals.min_angular_resolution, 'rad')['value']
        maxAcceptableAngResolution = cqa.convert(self.science_goals.max_angular_resolution, 'rad')['value']
        if (minAcceptableAngResolution == 0.0) or (maxAcceptableAngResolution == 0.0):
            desired_angular_resolution = cqa.convert(self.science_goals.desired_angular_resolution, 'rad')['value']
            if (desired_angular_resolution != 0.0):
                minAcceptableAngResolution = 0.8 * desired_angular_resolution
                maxAcceptableAngResolution = 1.2 * desired_angular_resolution
            else:
                science_goals = self.observing_run.get_measurement_sets()[0].science_goals
                minAcceptableAngResolution = cqa.convert(science_goals['minAcceptableAngResolution'], 'rad')['value']
                maxAcceptableAngResolution = cqa.convert(science_goals['maxAcceptableAngResolution'], 'rad')['value']
                if (minAcceptableAngResolution == 0.0) or (maxAcceptableAngResolution == 0.0):
                    LOG.info('No value for desired angular resolution. Setting "robust" parameter to 0.5.')
                    return 0.5

        # Get maximum baseline length in metres
        bmax = 0.0
        for ms in self.observing_run.get_measurement_sets():
            if (ms.antenna_array.max_baseline.length.to_units(measures.DistanceUnits.METRE) > bmax):
                bmax = float(ms.antenna_array.max_baseline.length.to_units(measures.DistanceUnits.METRE))

        if (bmax == 0.0):
            LOG.warning('Bmax is zero. Setting "robust" parameter to 0.5.')
            return 0.5

        # Get spw center wavelength

        # get the spw from the first vis set, assume all others the same for now
        ms = self.observing_run.get_ms(name=self.vislist[0])
        spw = ms.get_spectral_window(spw)

        centre_frequency = float(spw.centre_frequency.to_units(measures.FrequencyUnits.HERTZ))
        centre_lambda = cqa.constants('c')['value'] / centre_frequency

        # Smallest spatial scale
        # TODO: Use actual beam sizes from CASA ?
        native_resolution = 1.2 * centre_lambda / bmax

        if (native_resolution > maxAcceptableAngResolution):
            robust = -0.5
        elif (native_resolution < minAcceptableAngResolution):
            robust = 1.0
        else:
            robust = 0.5

        return robust

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
            noisethreshold = 3.0
            lownoisethreshold = 2.5
            minbeamfrac = 0.2
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
