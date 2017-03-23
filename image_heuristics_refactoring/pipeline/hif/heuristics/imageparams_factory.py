from .imageparams_alma import ImageParamsHeuristicsALMA
from .imageparams_vlass import ImageParamsHeuristicsVLASS

class ImageParamsHeuristicsFactory(object):

    '''Imaging heuristics factory class.'''

    @staticmethod
    def getHeuristics(context, vislist, spw, contfile=None, linesfile=None, imaging_mode='ALMA'):
        if imaging_mode == 'ALMA':
            return ImageParamsHeuristicsALMA(context, vislist, spw, contfile, linesfile)
        elif imaging_mode == 'VLASS':
            return ImageParamsHeuristicsVLASS(context, vislist, spw, contfile, linesfile)
        else:
            raise Exception('Unknown imaging mode: %s' % (imaginge_mode))
