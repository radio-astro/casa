from __future__ import absolute_import

import pipeline.infrastructure.api as api
#from .. import common as tasks_common
import pipeline.infrastructure.casatools as casatools
import pipeline.domain as domain
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure.jobrequest import casa_tasks
import pipeline.infrastructure.tablereader as tablereader

from pipeline.hif.tasks.clean.resultobjects import BoxResult

from pipeline.hif.heuristics import cleanbox 

LOG = logging.get_logger(__name__)


class BoxWorkerInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir, vis, psf, model, restored,
      residual, fluxscale, old_cleanmask, new_cleanmask):
        self._init_properties(vars())


class BoxWorker(basetask.StandardTaskTemplate):
    Inputs = BoxWorkerInputs
    
    def __init__(self, inputs):
        super(BoxWorker, self).__init__(inputs)
        self.result = BoxResult()

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs

        jobs = []

        psf = inputs.psf
        residual = inputs.residual

        sidelobe_ratio = cleanbox.psf_sidelobe_ratio(psf=inputs.psf)

        threshold, island_peaks = cleanbox.threshold_and_mask(
          residual=inputs.residual, old_mask=inputs.old_cleanmask,
          new_mask=inputs.new_cleanmask, sidelobe_ratio=sidelobe_ratio,
          fluxscale=inputs.fluxscale)

        self.result.threshold = threshold
        self.result.cleanmask = inputs.new_cleanmask
        self.result.island_peaks = island_peaks

        return self.result

    def analyse(self, result):
        return result
