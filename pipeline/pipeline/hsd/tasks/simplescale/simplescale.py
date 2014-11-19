from __future__ import absolute_import

import os
import shutil
import glob
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks

from .. import common

LOG = infrastructure.get_logger(__name__)

class SDSimpleScaleInputs(common.SingleDishInputs):
    """
    Inputs for simple scaling
    """
    @basetask.log_equivalent_CASA_call
#     def __init__(self, context, infiles=None, spw=None, pol=None, factor=None):
    def __init__(self, context, infiles=None, factor=None):
        self._init_properties(vars())
        self._to_numeric('factor')

    @property
    def antennalist(self):
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

class SDSimpleScaleResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDSimpleScaleResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDSimpleScaleResults, self).merge_with_context(context)

#         # increment iteration counter
#         # register detected lines to reduction group member
#         reduction_group = context.observing_run.reduction_group
#         for b in self.outcome['corrected']:
#             group_id = b['group_id']
#             spw = b['spw']
#             antenna = b['index']
#             pols = b['pols']
#             lines = b['lines']
#             channelmap_range = b['channelmap_range']
#             group_desc = reduction_group[group_id]
#             for (ant,spw,pol) in zip(antenna,spw,pols):
#                 group_desc.iter_countup(ant, spw, pol)
#                 group_desc.add_linelist(lines, ant, spw, pol,
#                                         channelmap_range=channelmap_range)
#                 st = context.observing_run[ant]
#                 st.work_data = st.baselined_name

    def _outcome_name(self):
        return ['%s: %s (spw=%s, pol=%s)'%(idx, name, b['spw'], b['pols'])
                for b in self.outcome['corrected']
                for (idx,name) in zip(b['index'], b['name'])]

class SDSimpleScale(common.SingleDishTaskTemplate):
    Inputs = SDSimpleScaleInputs

    @common.datatable_setter
    def prepare(self):
        inputs = self.inputs
        context = inputs.context
        reduction_group = context.observing_run.reduction_group
        infiles = inputs.infiles
        factor = inputs.factor
        args = inputs.to_casa_args()
        # parse infiles
        file_names = []
        file_index = [] #Need to get indices too to update blsource
        for name in infiles:
            stidx = self._get_index_from_name(name)
            if not os.path.exists(name):
                LOG.warn("Could not find '%s'. skipping" % name)
            elif stidx > -1:
                file_names.append(name)
                file_index.append(stidx)
            else:
                LOG.warn("Could not find corresponding file index for '%s'" % name)
#         file_names = [ name for name in infiles if os.path.exists(name) ]
        # get files from context
        if len(file_names) == 0:
            # No selection parse from reduction_group
            LOG.info("Getting scantables to scale from reduction_group")
            for (group_id,group_desc) in reduction_group.items():
                LOG.info('Processing Reduction Group %s'%(group_id))
                LOG.info('Group Summary:')
                for m in group_desc:
                    LOG.info('\tAntenna %s Spw %s Pol %s'%(m.antenna, m.spw, m.pols))
                    if m.antenna not in file_index:
                        file_index.append(m.antenna)
            file_names = [ context.observing_run[idx].work_data for idx in file_index ]

        LOG.info("Selected file names for scaling = %s" % str(file_names))
        LOG.info("Selected file indices from reduction group = %s" % str(file_index))
        
        # loop over scantable
        for idx in range(len(file_index)):
            in_name = file_names[idx]
            st = context.observing_run[file_index[idx]]
            out_name = st.scaled_name
            scale_args = {'infile': in_name,
                          'factor': factor,
                          'scaletsys': False,
                          'outfile': out_name,
                          'overwrite': True}
            LOG.info('Processing %s' % scale_args['infile'])
            LOG.info('\tscaling factor = %f' % scale_args['factor'])
            LOG.info("\toutput file name = '%s'" % scale_args['outfile'])
            scale_job = casa_tasks.sdscale(**scale_args)
            # execute job
            self._executor.execute(scale_job)
            # update working scantable
            st.work_data = out_name
            # update scantable to be used in baseline subtraction only if not yet BL-ed
            if not os.path.exists(st.baselined_name):
                st.baseline_source = out_name

        outcome = {'SimpleScaled': baselined,
                   'factor': factor}
        results = SDSimpleScaleResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
        return results

    def analyse(self, result):
        return result
    
    def _get_index_from_name(self, name):
        """ Get corresponding file index for the name """
        st_names = [ obsrun.name for obsrun in self.inputs.context.observing_run ]
        for idx in range(len(st_names)):
            if name.startswith(st_names[idx]):
                return idx
        return -1
 

