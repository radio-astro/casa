import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


class ImagePreCheckHeuristics(object):
    def __init__(self, inputs):
        self.inputs = inputs
        self.context = inputs.context

    def compare_beams(self, beam_m0p5, beam_0p5, beam_2p0, minAR, maxAR):

        cqa = casatools.quanta

        beams = {-0.5: beam_m0p5, 0.5: beam_0p5, 2.0: beam_2p0}

        if (cqa.getvalue(minAR) == 0.0) and \
           (cqa.getvalue(maxAR) == 0.0):
            # PI requested resolution range is not available, robust=0.5 (pre-Cycle 5 data and all 7m-array datasets)
            hm_robust = 0.5
            hm_robust_scoreA = (1.0, 'PI requested resolution range is not available', 'PI range unavailable')
        elif cqa.le(minAR, beams[0.5]['major']) and \
             cqa.le(beams[0.5]['major'], maxAR) and \
             cqa.le(minAR, beams[0.5]['minor']) and \
             cqa.le(beams[0.5]['minor'], maxAR):
            # both axes fall within the range, default robust=0.5
            hm_robust = 0.5
            hm_robust_scoreA = (1.0, 'Predicted robust=0.5 beam is within the PI requested range', 'Beam within range')
        elif cqa.gt(beams[0.5]['major'], maxAR) and \
             cqa.gt(beams[0.5]['minor'], maxAR):
            # both fall outside the range, and are too large -> select robust=-0.5
            hm_robust = -0.5
            hm_robust_scoreA = (0.75, 'Predicted robust=0.5 beam is outside PI requested range', 'Beam outside range')
        elif cqa.lt(beams[0.5]['major'], minAR) and \
             cqa.lt(beams[0.5]['minor'], minAR):
            # both fall outside the range, and are too small -> select robust=2.0
            hm_robust = 2.0
            hm_robust_scoreA = (0.75, 'Predicted robust=0.5 beam is outside PI requested range', 'Beam outside range')
        elif cqa.gt(beams[0.5]['major'], maxAR) and \
             cqa.lt(beams[0.5]['minor'], minAR):
            # both fall outside the range, one too large and one too small, no change from default robust=0.5
            hm_robust = 0.5
            hm_robust_scoreA = (0.25, 'Predicted robust=0.5 beam is outside PI requested range and cannot be mitigated by robust', 'Cannot mitigate by robust')
        elif cqa.gt(beams[0.5]['major'], maxAR) and \
             cqa.le(minAR, beams[0.5]['minor']) and \
             cqa.le(beams[0.5]['minor'], maxAR):
            # Major axis is too large, minor axis in range -> robust=-0.5, unless
            # it results in the minor axis falling outside of the range, in which
            # case the default value should be used.
            if cqa.le(minAR, beams[-0.5]['minor']) and \
               cqa.le(beams[-0.5]['minor'], maxAR):
                hm_robust = -0.5
                hm_robust_scoreA = (0.75, 'Predicted robust=0.5 beam is outside PI requested range', 'Beam outside range')
            else:
                hm_robust = 0.5
                hm_robust_scoreA = (0.25, 'Predicted robust=0.5 beam is outside PI requested range and cannot be mitigated by robust', 'Cannot mitigate by robust')
        elif cqa.lt(beams[0.5]['minor'], minAR) and \
             cqa.le(minAR, beams[0.5]['major']) and \
             cqa.le(beams[0.5]['major'], maxAR):
            # Minor axis is too small, major axis in range -> robust=2.0, unless
            # it results in the majoror axis falling outside of the range, in which
            # case the default value should be used.
            if cqa.le(minAR, beams[2.0]['major']) and \
               cqa.le(beams[2.0]['major'], maxAR):
                hm_robust = 2.0
                hm_robust_scoreA = (0.75, 'Predicted robust=0.5 beam is outside PI requested range', 'Beam outside range')
            else:
                hm_robust = 0.5
                hm_robust_scoreA = (0.25, 'Predicted robust=0.5 beam is outside PI requested range and cannot be mitigated by robust', 'Cannot mitigate by robust')
        else:
            raise Exception, 'Unexpected case: beam(robust=-0.5)=%s, beam(robust=0.5)=%s, beam(robust=2.0)=%s, minAR=%s, maxAR=%s' % (str(beams[-0.5]), str(beams[0.5]), str(beams[2.0]), str(minAR), str(maxAR))

        if hm_robust == 0.5:
            return hm_robust, hm_robust_scoreA
        elif (cqa.gt(beams[hm_robust]['major'], maxAR) and cqa.gt(beams[hm_robust]['minor'], maxAR)) or \
             (cqa.lt(beams[hm_robust]['major'], minAR) and cqa.lt(beams[hm_robust]['minor'], minAR)):
            hm_robust_scoreB = (0.25, \
                'Predicted beam %#.3g x %#.3g arcsec still has both axes outside of user requested range %#.3g-%#.3g arcsec using a non-default robust value of %.1f' % \
                (cqa.getvalue(cqa.convert(beams[hm_robust]['major'], 'arcsec')), \
                 cqa.getvalue(cqa.convert(beams[hm_robust]['minor'], 'arcsec')), \
                 cqa.getvalue(cqa.convert(minAR, 'arcsec')), \
                 cqa.getvalue(cqa.convert(maxAR, 'arcsec')), \
                 hm_robust), \
                'Non-default robust beam outside range')
        elif cqa.gt(beams[hm_robust]['major'], maxAR) and \
             cqa.le(minAR, beams[hm_robust]['minor']) and \
             cqa.le(beams[hm_robust]['minor'], maxAR):
            hm_robust_scoreB = (0.5, \
                'Predicted beam %#.3g x %#.3g arcsec still has major axes outside of user requested range %#.3g-%#.3g arcsec using a non-default robust value of %.1f' % \
                (cqa.getvalue(cqa.convert(beams[hm_robust]['major'], 'arcsec')), \
                 cqa.getvalue(cqa.convert(beams[hm_robust]['minor'], 'arcsec')), \
                 cqa.getvalue(cqa.convert(minAR, 'arcsec')), \
                 cqa.getvalue(cqa.convert(maxAR, 'arcsec')), \
                 hm_robust), \
                'Non-default robust beam outside range')
        elif cqa.lt(beams[hm_robust]['minor'], minAR) and \
             cqa.le(minAR, beams[hm_robust]['major']) and \
             cqa.le(beams[hm_robust]['major'], maxAR):
            hm_robust_scoreB = (0.5, \
                'Predicted beam %#.3g x %#.3g arcsec still has minor axes outside of user requested range %#.3g-%#.3g arcsec using a non-default robust value of %.1f' % \
                (cqa.getvalue(cqa.convert(beams[hm_robust]['major'], 'arcsec')), \
                 cqa.getvalue(cqa.convert(beams[hm_robust]['minor'], 'arcsec')), \
                 cqa.getvalue(cqa.convert(minAR, 'arcsec')), \
                 cqa.getvalue(cqa.convert(maxAR, 'arcsec')), \
                 hm_robust), \
                'Non-default robust beam outside range')
        elif cqa.le(minAR, beams[hm_robust]['major']) and \
             cqa.le(beams[hm_robust]['major'], maxAR) and \
             cqa.le(minAR, beams[hm_robust]['minor']) and \
             cqa.le(beams[hm_robust]['minor'], maxAR):
            hm_robust_scoreB = (0.7, \
                'Predicted beam is within the PI requested range %#.3g-%#.3g arcsec using a non-default robust value of %.1f' % \
                (cqa.getvalue(cqa.convert(minAR, 'arcsec')), \
                 cqa.getvalue(cqa.convert(maxAR, 'arcsec')), \
                 hm_robust), \
                'Non-default robust beam within range')
        else:
            raise Exception, 'Unexpected case: beam(robust=-0.5)=%s, beam(robust=0.5)=%s, beam(robust=2.0)=%s, minAR=%s, maxAR=%s' % (str(beams[-0.5]), str(beams[0.5]), str(beams[2.0]), str(minAR), str(maxAR))

        if hm_robust_scoreA[0] < hm_robust_scoreB[0]:
            return hm_robust, hm_robust_scoreA
        else:
            return hm_robust, hm_robust_scoreB
