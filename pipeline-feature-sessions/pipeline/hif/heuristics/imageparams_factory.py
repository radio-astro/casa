from .imageparams_alma import ImageParamsHeuristicsALMA
from .imageparams_vlass_quick_look import ImageParamsHeuristicsVlassQl
from .imageparams_vlass_single_epoch import ImageParamsHeuristicsVlassSe
from .imageparams_vla import ImageParamsHeuristicsVLA


class ImageParamsHeuristicsFactory(object):

    '''Imaging heuristics factory class.'''

    @staticmethod
    def getHeuristics(vislist, spw, observing_run, imagename_prefix='', proj_params=None, contfile=None, linesfile=None, imaging_mode='ALMA'):
        if imaging_mode == 'ALMA':
            return ImageParamsHeuristicsALMA(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile)
        elif imaging_mode == 'VLASS-QL':  # quick look
            return ImageParamsHeuristicsVlassQl(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile)
        elif imaging_mode == 'VLASS-SE':  # single epoch
            return ImageParamsHeuristicsVlassSe(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile)
        elif imaging_mode == 'VLA' or imaging_mode == 'JVLA' or imaging_mode == 'EVLA':  # VLA but not VLASS
            return ImageParamsHeuristicsVLA(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile)
        else:
            raise Exception('Unknown imaging mode: %s' % (imaging_mode))
