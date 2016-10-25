from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

from pipeline.h.tasks.exportdata import exportdata

LOG = infrastructure.get_logger(__name__)

class VLAExportDataInputs(exportdata.ExportDataInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, session=None, vis=None,
                 pprfile=None, calintents=None, calimages=None, targetimages=None,
                 products_dir=None ):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class VLAExportData(exportdata.ExportData):

    # link the accompanying inputs to this task
    Inputs = VLAExportDataInputs
