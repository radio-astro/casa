from __future__ import absolute_import

import os
import math
#from math import cos, sqrt, exp
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.jobrequest as jobrequest
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from .. import common
from .worker import SDBaselineWorker

LOG = infrastructure.get_logger(__name__)
logging.set_logging_level('trace')
#logging.set_logging_level('info')

class SDBaselineInputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
    def __init__(self, context, infiles=None, iflist=None, pollist=None,
                 linewindow=None, edge=None, broadline=None, fitorder=None,
                 fitfunc=None):
        self._init_properties(vars())

    @property
    def antennalist(self):
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

class SDBaselineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDBaselineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDBaselineResults, self).merge_with_context(context)
        LOG.todo('need to decide what is done in SDBaselineResults.merge_with_context')

    def _outcome_name(self):
        return self.outcome

class SDBaseline(common.SingleDishTaskTemplate):
    Inputs = SDBaselineInputs

    def prepare(self):
        inputs = self.inputs
        context = inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        infiles = inputs.infiles
        iflist = inputs.iflist
        antennalist = inputs.antennalist
        pollist = inputs.pollist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]

        window = [] if inputs.linewindow is None else inputs.linewindow
        edge = (0,0) if inputs.edge is None else inputs.edge
        broadline = False if inputs.broadline is None else inputs.broadline
        fitorder = 'automatic' if inputs.fitorder is None or inputs.fitorder < 0 else inputs.fitorder
        fitfunc = 'spline' if inputs.fitfunc is None else inputs.fitfunc
        
        # task returns ResultsList
        results = basetask.ResultsList()

        # loop over reduction group
        files = set()
        for (group_id,group_desc) in reduction_group.items():
            # assume all members have same spw and pollist
            spwid = group_desc['member'][0][1]
            LOG.debug('spwid=%s'%(spwid))
            pols = group_desc['member'][0][2]
            if pollist is not None:
                pols = list(set(pollist) & set(pols))

            # skip spw not included in iflist
            if iflist is not None and spwid not in iflist:
                LOG.debug('Skip spw %s'%(spwid))
                continue
                
            # reference data is first scantable 
            st = context.observing_run[group_desc['member'][0][0]]

            # skip channel averaged spw
            nchan = group_desc['nchan']
            if nchan == 1:
                LOG.info('Skip channel averaged spw %s.'%(spwid))
                continue

            beam_size = st.beam_size[spwid]
            calmode = st.calibration_strategy['calmode']
            srctype = common.SrcTypeMap(calmode)
            worker = SDBaselineWorker(context)
            _file_index = set(file_index) & set([m[0] for m in group_desc['member']])
            files = files | _file_index
            pattern = st.pattern[spwid][pols[0]]
            parameters = {'datatable': datatable,
                          'spwid': spwid,
                          'nchan': nchan,
                          'beam_size': beam_size,
                          'pollist': pols,
                          'srctype': srctype,
                          'file_index': list(_file_index),
                          'window': window,
                          'edge': edge,
                          'broadline': broadline,
                          'fitorder': fitorder,
                          'fitfunc': fitfunc,
                          'observing_pattern': pattern}
            job = jobrequest.JobRequest(worker.execute, **parameters)
            self._executor.execute(job)

        for f in files:
            name = context.observing_run[f].baselined_name
            result = SDBaselineResults(task=self.__class__,
                                       success=True,
                                       outcome=name)

            if self.inputs.context.subtask_counter is 0: 
                result.stage_number = self.inputs.context.task_counter - 1
            else:
                result.stage_number = self.inputs.context.task_counter 

            results.append(result)
                
        return results

    def analyse(self, result):
        return result


