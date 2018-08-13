import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


class ImagePreCheckHeuristics(object):
    def __init__(self, inputs):
        self.inputs = inputs
        self.context = inputs.context

    def compare_beams(self, beam_m0p5, beam_0p0, beam_0p5, beam_1p0, beam_2p0, minAR, maxAR):

        cqa = casatools.quanta

        beams = {-0.5: beam_m0p5, 0.0: beam_0p0, 0.5: beam_0p5, 1.0: beam_1p0, 2.0: beam_2p0}
        robusts = sorted(beams.keys())

        # PI requested resolution range is not available, robust=0.5 (pre-Cycle 5 data and all 7m-array datasets)
        if (cqa.getvalue(minAR) == 0.0) and \
           (cqa.getvalue(maxAR) == 0.0):
            hm_robust = 0.5
            hm_robust_score = (1.0, 'No representative target info found', 'No representative target')
        # both axes in of default robust=0.5
        elif cqa.le(minAR, beams[0.5]['major']) and \
             cqa.le(beams[0.5]['major'], maxAR) and \
             cqa.le(minAR, beams[0.5]['minor']) and \
             cqa.le(beams[0.5]['minor'], maxAR):
            hm_robust = 0.5
            hm_robust_score = (1.0, 'Predicted robust=0.5 beam is within the PI requested range', 'Beam within range')
        # both axes in range of robust=0.0
        elif cqa.le(minAR, beams[0.0]['major']) and \
             cqa.le(beams[0.0]['major'], maxAR) and \
             cqa.le(minAR, beams[0.0]['minor']) and \
             cqa.le(beams[0.0]['minor'], maxAR):
            hm_robust = 0.0
            hm_robust_score = (0.85, 'Predicted non-default robust=0.0 beam is within the PI requested range', 'Beam within range using non-default robust')
            # Informational message for the weblog. Must currently be a warning to get there.
            LOG.warn('Predicted non-default robust=0.0 beam is within the PI requested range')
        # both axes in range of robust=1.0
        elif cqa.le(minAR, beams[1.0]['major']) and \
             cqa.le(beams[1.0]['major'], maxAR) and \
             cqa.le(minAR, beams[1.0]['minor']) and \
             cqa.le(beams[1.0]['minor'], maxAR):
            hm_robust = 1.0
            hm_robust_score = (0.85, 'Predicted non-default robust=1.0 beam is within the PI requested range', 'Beam within range using non-default robust')
            # Informational message for the weblog. Must currently be a warning to get there.
            LOG.warn('Predicted non-default robust=1.0 beam is within the PI requested range')
        # both axes in range of robust=-0.5
        elif cqa.le(minAR, beams[-0.5]['major']) and \
             cqa.le(beams[-0.5]['major'], maxAR) and \
             cqa.le(minAR, beams[-0.5]['minor']) and \
             cqa.le(beams[-0.5]['minor'], maxAR):
            hm_robust = -0.5
            hm_robust_score = (0.75, 'Predicted non-default robust=-0.5 beam is within the PI requested range', 'Beam within range using non-default robust')
            # Informational message for the weblog. Must currently be a warning to get there.
            LOG.warn('Predicted non-default robust=-0.5 beam is within the PI requested range')
        # both axes in range of robust=2.0
        elif cqa.le(minAR, beams[2.0]['major']) and \
             cqa.le(beams[2.0]['major'], maxAR) and \
             cqa.le(minAR, beams[2.0]['minor']) and \
             cqa.le(beams[2.0]['minor'], maxAR):
            hm_robust = 2.0
            hm_robust_score = (0.75, 'Predicted non-default robust=2.0 beam is within the PI requested range', 'Beam within range using non-default robust')
            # Informational message for the weblog. Must currently be a warning to get there.
            LOG.warn('Predicted non-default robust=+2.0 beam is within the PI requested range')
        # If no robust got within range calculate the meanAR, to choose robust. The order of these definitions must match the robusts list.
        else:
            beamArea_m0p5 = cqa.mul(beams[-0.5]['minor'], beams[-0.5]['major'])
            beamArea_0p0 =  cqa.mul(beams[0.0]['minor'], beams[0.0]['major'])
            beamArea_0p5 =  cqa.mul(beams[0.5]['minor'], beams[0.5]['major'])
            beamArea_1p0 =  cqa.mul(beams[1.0]['minor'], beams[1.0]['major'])
            beamArea_2p0 =  cqa.mul(beams[2.0]['minor'], beams[2.0]['major'])
            meanARBeamArea = cqa.mul(maxAR, minAR)
            delta_m0p5 = cqa.convert(cqa.sub(beamArea_m0p5, meanARBeamArea), 'arcsec.arcsec')
            delta_0p0 = cqa.convert(cqa.sub(beamArea_0p0, meanARBeamArea), 'arcsec.arcsec')
            delta_0p5 = cqa.convert(cqa.sub(beamArea_0p5, meanARBeamArea), 'arcsec.arcsec')
            delta_1p0 = cqa.convert(cqa.sub(beamArea_1p0, meanARBeamArea), 'arcsec.arcsec')
            delta_2p0 = cqa.convert(cqa.sub(beamArea_2p0, meanARBeamArea), 'arcsec.arcsec')
            deltas = [delta_m0p5, delta_0p0, delta_0p5, delta_1p0, delta_2p0]
            absdeltas = [cqa.getvalue(cqa.abs(delta)) for delta in deltas]
            predictBeamAreas = [beamArea_m0p5, beamArea_0p0, beamArea_0p5, beamArea_1p0, beamArea_2p0]
            index = np.argmin(absdeltas)
            hm_robust = robusts[index]
            DiffmeanAR = 100 * (cqa.getvalue(deltas[index]) / cqa.getvalue(cqa.convert(meanARBeamArea, 'arcsec.arcsec')))
            maxARBeamArea = cqa.mul(maxAR, maxAR)
            minARBeamArea = cqa.mul(minAR, minAR)
            # Now give scores and messages when not in range
            # Even if not in range on one or more axes, might be in beam area range 
            if cqa.getvalue(cqa.convert(minARBeamArea, 'arcsec.arcsec')) <= cqa.getvalue(cqa.convert(predictBeamAreas[index],'arcsec.arcsec')) <= cqa.getvalue(cqa.convert(maxARBeamArea, 'arcsec.arcsec')): 
                hm_robust_score = (0.5, 'Robust cannot achieve the PI requested range for one or both axes, due to an elliptical beam, but the best match robust %+.1f (%%Diff from mean AR = %.1f%%), does produce a predicted beam area that is within the range of requested beam areas.' % (hm_robust, DiffmeanAR), 'Beam not in range, too elliptical') 
            # Robust = -0.5
            elif index in [0]:
                DiffmaxAR  =  abs(100 * (cqa.getvalue(cqa.convert(cqa.sub(predictBeamAreas[index], maxARBeamArea), 'arcsec.arcsec'))) / cqa.getvalue(cqa.convert(maxARBeamArea, 'arcsec.arcsec')))
                hm_robust_score = (0.25, 'Robust cannot achieve the PI requested range, the best match robust %+.1f (%%Diff from mean AR = %.1f%%), has a %%Diff from max AR = %.1f%%' % (hm_robust, DiffmeanAR, DiffmaxAR), 'Beam too large')
            # Robust = 2.0                         
            elif index in [4]:
                DiffminAR  =  abs(100 * (cqa.getvalue(cqa.convert(cqa.sub(minARBeamArea, predictBeamAreas[index]), 'arcsec.arcsec'))) / cqa.getvalue(cqa.convert(minARBeamArea, 'arcsec.arcsec')))
                hm_robust_score = (0.25, 'Robust cannot achieve the PI requested range, the best match robust %+.1f (%%Diff from mean AR = %.1f%%) has a %%Diff from min AR = %.1f%%' % (hm_robust, DiffmeanAR, DiffminAR), 'Beam too small')
            else:
                hm_robust_score = (0.25, 'Robust cannot achieve the PI requested range, due to an elliptical beam, but the best match robust %+.1f (%%Diff from mean AR = %.1f%%), produces a predicted beam area that is outside the range of requested beam areas.' % (hm_robust, DiffmeanAR), 'Beam not in range') 
                
        return hm_robust, hm_robust_score
