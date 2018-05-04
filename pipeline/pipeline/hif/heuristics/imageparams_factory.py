from .imageparams_alma import ImageParamsHeuristicsALMA
from .imageparams_vlass_quick_look import ImageParamsHeuristicsVlassQl
from .imageparams_vlass_single_epoch_continuum import ImageParamsHeuristicsVlassSeCont
from .imageparams_vlass_single_epoch_taper import ImageParamsHeuristicsVlassSeTaper
from .imageparams_vlass_single_epoch_cube import ImageParamsHeuristicsVlassSeCube
from .imageparams_vla import ImageParamsHeuristicsVLA


class ImageParamsHeuristicsFactory(object):

    '''Imaging heuristics factory class.'''

    @staticmethod
    def getHeuristics(vislist, spw, observing_run, imagename_prefix='', proj_params=None, contfile=None, linesfile=None, imaging_params={}, imaging_mode='ALMA'):
        if imaging_mode == 'ALMA':
            return ImageParamsHeuristicsALMA(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile, imaging_params)
        elif imaging_mode == 'VLASS-QL':  # quick look
            return ImageParamsHeuristicsVlassQl(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile, imaging_params)
        elif imaging_mode == 'VLASS-SE-CONT':  # single epoch continuum
            return ImageParamsHeuristicsVlassSeCont(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile, imaging_params)
        elif imaging_mode == 'VLASS-SE-TAPER':  # single epoch taper
            return ImageParamsHeuristicsVlassSeTaper(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile, imaging_params)
        elif imaging_mode == 'VLASS-SE-CUBE':  # single epoch cube
            return ImageParamsHeuristicsVlassSeCube(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile, imaging_params)
        elif imaging_mode == 'VLA' or imaging_mode == 'JVLA' or imaging_mode == 'EVLA':  # VLA but not VLASS
            return ImageParamsHeuristicsVLA(vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile, imaging_params)
        else:
            raise Exception('Unknown imaging mode: %s' % imaging_mode)
