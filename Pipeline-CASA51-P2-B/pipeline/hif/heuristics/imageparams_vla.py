import pipeline.infrastructure as infrastructure
from .imageparams_alma import ImageParamsHeuristicsALMA

LOG = infrastructure.get_logger(__name__)


class ImageParamsHeuristicsVLA(ImageParamsHeuristicsALMA):

    def __init__(self, vislist, spw, observing_run, imagename_prefix='', proj_params=None, contfile=None, linesfile=None):
        ImageParamsHeuristicsALMA.__init__(self, vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile)
        self.imaging_mode = 'VLA'
