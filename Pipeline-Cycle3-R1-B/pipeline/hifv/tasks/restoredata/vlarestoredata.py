"""
The restore data module provides a class for reimporting, reflagging, and
recalibrating a subset of the ASDMs belonging to a member OUS, using pipeline
flagging and calibration data products.

The basic restore data module assumes that the ASDMs, flagging, and calibration
data products are on disk in the rawdata directory in the format produced by
the ExportData class.

This class assumes that the required data products have been
    o downloaded from the archive along with the ASDMs (not yet possible)
    o are sitting on disk in a form which is compatible with what is
      produced by ExportData

To test these classes, register some data with the pipeline using ImportData,
then execute:

import pipeline
vis = [ '<ASDM name>' ]

# Create a pipeline context and register some data
context = pipeline.Pipeline().context
inputs = pipeline.tasks.RestoreData.Inputs(context, vis=vis)
task = pipeline.tasks.RestoreData(inputs)
results = task.execute(dry_run=False)
results.accept(context)
"""
from __future__ import absolute_import
import glob
import os
import re
import shutil
import string
import tarfile
import tempfile
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.restoredata import restoredata

from pipeline.infrastructure import casa_tasks

# the logger for this module
LOG = infrastructure.get_logger(__name__)


class VLARestoreDataInputs(restoredata.RestoreDataInputs):
    
     
    bdfflags  = basetask.property_with_default('bdfflags', False)

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, copytoraw=None, products_dir=None,
        rawdata_dir=None, output_dir=None, session=None, vis=None,
        bdfflags=None):
        self._init_properties(vars())


class VLARestoreData(restoredata.RestoreData):
    Inputs = VLARestoreDataInputs


