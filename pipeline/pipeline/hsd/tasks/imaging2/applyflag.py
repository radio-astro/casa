from __future__ import absolute_import

import os
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
from pipeline.infrastructure import casa_tasks
from .. import common

LOG = infrastructure.get_logger(__name__)

class SDApplyFlagInputs(common.SingleDishInputs):
    """
    Inputs for applying flags to each scantable 
    """
    def __init__(self, context, infiles=None, iflist=None, pollist=None):
        self._init_properties(vars())
        self._to_list(['infiles', 'iflist', 'pollist'])

class SDApplyFlagResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDApplyFlagResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDApplyFlagResults, self).merge_with_context(context)

    def _outcome_name(self):
        return ''


class SDApplyFlag(common.SingleDishTaskTemplate):
    Inputs = SDApplyFlagInputs
    
    def prepare(self):
        # for each data
        context = self.inputs.context
        reduction_group = context.observing_run.reduction_group
        infiles = self.inputs.infiles

        index_for_infiles = [context.observing_run.st_names.index(v) 
                             for v in infiles]
        # flag all WVR data and off-source data  
        for index in index_for_infiles:
            data = context.observing_run[index]
            wvr_spws = [spw for (spw, desc) in data.spectral_window.items()
                        if desc.type == 'WVR']
            filename = data.baselined_name
            srctype = data.calibration_strategy['srctype']
            self._apply_apriori_flags(filename, wvr_spws, srctype)

        namer = filenamer.BaselineSubtractedTable()

        # loop over reduction group
        for (group_id, group_desc) in reduction_group.items():
            
            # for each group member
            for member in group_desc:
                index = member.antenna
                if index not in index_for_infiles:
                    continue

                # apply baseline flags to the data
                spwid = member.spw
                namer.spectral_window(spwid)
                namer.asdm(common.asdm_name(data))
                namer.antenna_name(data.antenna.name)
                bltable_name = namer.get_filename()
                filename = data.name
                self._apply_baseline_flags(filename, bltable_name)

            
        result = SDApplyFlagResults(task=self.__class__,
                                 success=True,
                                 outcome=None)
        result.task = self.__class__

        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 

        return result
    
    def analyse(self, result):
        return result

    def _apply_apriori_flags(self, filename, wvr_spws, on_source):
        with casatools.TableReader(filename, nomodify=False) as tb:
            # flag WVR spws
            tsel = tb.query('IFNO IN %s' % (list(wvr_spws)))
            nrow = tsel.nrows()
            flags = numpy.ones((4, nrow), dtype=int)
            tsel.putcol('FLAGROW', flags[0])
            tsel.putcol('FLAGTRA', flags)
            tsel.close()
            
            # flag all reference (off source) data
            tsel = tb.query('SRCTYPE != %s' % (on_source))
            nrow = tsel.nrows()
            for irow in xrange(nrow):
                tsel.putcell('FLAGROW', irow, 1)
                flags = tsel.getcell('FLAGTRA', irow)
                flags[:] = 1
                tsel.putcell('FLAGTRA', irow, flags)

    def _apply_baseline_flags(self, filename, bltable_name):
        if not os.path.exists(bltable_name):
            return
            
        with casatools.TableReader(bltable_name) as tb:
            tsel = tb.query('SummaryFlag == False')
            rows = tsel.getcol('Row')
            tsel.close()
    
        if len(rows) == 0:
            return
            
        with casatools.TableReader(filename, nomodify=False) as tb:
            tsel = tb.query('ROWNUMBER() IN %s' % (rows.tolist()), style='python')
            for irow in xrange(tsel.nrows()):
                tsel.putcell('FLAGROW', irow, 1)
                channel_flag = tsel.getcell('FLAGTRA', irow)
                channel_flag[:] = 1
                tsel.putcell('FLAGTRA', irow, channel_flag)
            tsel.close()
