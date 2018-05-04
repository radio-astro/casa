import pipeline.infrastructure as infrastructure
from .imageparams_alma import ImageParamsHeuristicsALMA

LOG = infrastructure.get_logger(__name__)


class ImageParamsHeuristicsVLA(ImageParamsHeuristicsALMA):

    def __init__(self, vislist, spw, observing_run, imagename_prefix='', proj_params=None, contfile=None, linesfile=None, imaging_params={}):
        ImageParamsHeuristicsALMA.__init__(self, vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile, imaging_params)
        self.imaging_mode = 'VLA'

    def robust(self):

        return 0.5

    def uvtaper(self, beam_natural=None, protect_long=None):

        return []
