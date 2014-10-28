from __future__ import absolute_import
import os
import re
import contextlib
import tarfile
import string
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.basetask as basetask
import pipeline.domain as domain
from pipeline.infrastructure import tablereader
from ... import heuristics
from .. import common
from ..common import utils

import pipeline.hif.tasks.importdata.importdata as importdata

LOG = infrastructure.get_logger(__name__)

class SDImportData2Inputs(importdata.ImportDataInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context=None, vis=None, output_dir=None,
                 asis=None, process_caldevice=None,
         session=None, overwrite=None, save_flagonline=None,
         bdfflags=None):
        self._init_properties(vars())

class SDImportData2Results(basetask.Results):
    '''
    SDImportData2Results is an equivalent class with ImportDataResults. 
    Purpose of SDImportData2Results is to replace QA scoring associated 
    with ImportDataResults with single dish specific QA scoring, which 
    is associated with this class.
    
    ImportDataResults holds the results of the ImportData task. It contains
    the resulting MeasurementSet domain objects and optionally the additional 
    SetJy results generated from flux entries in Source.xml.
    '''
    
    def __init__(self, mses=None, setjy_results=None):
        super(SDImportData2Results, self).__init__()
        self.mses = [] if mses is None else mses
        self.setjy_results = setjy_results
        self.origin = {}
        self.results = importdata.ImportDataResults(mses=mses, setjy_results=setjy_results)
        
    def merge_with_context(self, context):
        if not isinstance(context.observing_run, domain.ScantableList):
            context.observing_run = domain.ScantableList()
        self.results.merge_with_context(context)
           
    def __repr__(self):
        return 'SDImportData2Results:\n\t{0}'.format(
                '\n\t'.join([ms.name for ms in self.mses]))

class SDImportData2(importdata.ImportData):
    Inputs = SDImportData2Inputs 
    
    def prepare(self, **parameters):
        results = super(SDImportData2, self).prepare()
        myresults = SDImportData2Results(mses=results.mses, setjy_results=results.setjy_results)
        myresults.origin = results.origin
        return myresults
