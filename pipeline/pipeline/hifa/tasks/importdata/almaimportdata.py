from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask

import pipeline.hif.tasks.importdata.importdata as importdata


class ALMAImportDataInputs(importdata.ImportDataInputs):
    # These are ALMA specific settings and override the defaults in
    # the base class
    process_caldevice  = basetask.property_with_default('process_caldevice', False)
    asis = basetask.property_with_default('asis', 'Antenna Station Receiver Source CalAtmosphere CalWVR')

    def __init__(self, context, vis=None, output_dir=None, asis=None,
                 process_caldevice=None, session=None, overwrite=None,
                 save_flagonline=None):
        self._init_properties(vars())


class ALMAImportData(importdata.ImportData):
    Inputs = ALMAImportDataInputs


