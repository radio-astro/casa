'''
Created on 2013/06/23

@author: kana
'''
from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from .. import common

LOG = infrastructure.get_logger(__name__)

class SDFlagDataInputs(common.SingleDishInputs):
    """
    Inputs for single dish flagging
    """
    def __init__(self, context, output_dir=None,
                 iteration=None, edge=None, flag_tsys=None, tsys_thresh=None,
                 flag_weath=None, weath_thresh=None,
                 flag_prfre=None, prfre_thresh=None,
                 flag_pofre=None, pofre_thresh=None,
                 flag_prfr=None, prfr_thresh=None,
                 flag_pofr=None, pofr_thresh=None,
                 flag_prfrm=None, prfrm_thresh=None, prfrm_nmean=None,
                 flag_pofrm=None, pofrm_thresh=None, pofrm_nmean=None,
                 flag_user=None, user_thresh=None,
                 plotflag=None,
                 infiles=None, antennalist=None, field=None,
                 iflist=None, pollist=None, scanlist=None):
        self._init_properties(vars())


class SDFlagDataResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDFlagDataResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDFlagDataResults, self).merge_with_context(context)

#    def _outcome_name(self): pass

class SDFlagData(common.SingleDishTaskTemplate):
    Inputs = SDFlagDataInputs

    def prepare(self):
        raise NotImplementedError
        ## inputs
        #inputs = self.inputs
        #context = inputs.context
        #datatable = context.observing_run.datatable_instance
        #reduction_group = context.observing_run.reduction_group
        #infiles = inputs.infiles
        #iflist = inputs.iflist
        #antennalist = inputs.antennalist
        #scanlist = inputs.scanlist
        #st_names = context.observing_run.st_names
        #file_index = [st_names.index(infile) for infile in infiles]


    def analyse(self, result):
        return result
