import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


class ImagePreCheckHeuristics(object):
    def __init__(self, inputs):
        self.inputs = inputs
        self.context = inputs.context
