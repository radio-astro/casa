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
from .. import common
from .gridding import gridding_factory
from .imagegenerator import SDImageGenerator

LOG = infrastructure.get_logger(__name__)

NoData = common.NoData

class SDImagingInputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
    def __init__(self, context, infiles=None, iflist=None, pollist=None):
        self._init_properties(vars())

    @property
    def antennalist(self):
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

class SDImagingResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDImagingResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDImagingResults, self).merge_with_context(context)
        LOG.todo('need to decide what is done in SDImagingResults.merge_with_context')

    def _outcome_name(self):
        #return [image.imagename for image in self.outcome]
        return self.outcome.imagename

class SDImaging(common.SingleDishTaskTemplate):
    Inputs = SDImagingInputs

    def prepare(self):
        context = self.inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        infiles = self.inputs.infiles
        iflist = self.inputs.iflist
        antennalist = self.inputs.antennalist
        pollist = self.inputs.pollist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]

        # task returns ResultsList
        results = basetask.ResultsList()

        # loop over reduction group
        for (group_id,group_desc) in reduction_group.items():
            # assume all members have same spw and pollist
            first_member = group_desc[0]
            spwid = first_member.spw
            LOG.debug('spwid=%s'%(spwid))
            pols = first_member.pols
            if pollist is not None:
                pols = list(set(pollist).intersection(pols))

            # skip spw not included in iflist
            if iflist is not None and spwid not in iflist:
                LOG.debug('Skip spw %s'%(spwid))
                continue

            # image is created per antenna
            antenna_group = {}
            for m in group_desc:
                antenna = context.observing_run[m.antenna].antenna.name
                if antenna in antenna_group.keys():
                    antenna_group[antenna].append(m.antenna)
                else:
                    antenna_group[antenna] = [m.antenna]

            # loop over antennas
            for (name,indices) in antenna_group.items():

                # skip antenna not included in antennalist
                if antennalist is not None and name not in antennalist:
                    LOG.debug('Skip antenna %s'%(name))
                    continue

                # reference data is first scantable 
                st = context.observing_run[indices[0]]

                # source name
                source_name = st.source[0].name.replace(' ','_')

                # filenames for gridding
                data_name = lambda x: x.baselined_name \
                            if os.path.exists(x.baselined_name) else x.name
                filenames = [data_name(context.observing_run[i]) for i in indices]

                LOG.debug('filenames=%s'%(filenames))
                
                worker = SDImagingWorker()
                parameters = {'datatable': datatable,
                              'reference_data': st,
                              'source_name': source_name,
                              'antenna_name': name,
                              'antenna_indices': indices,
                              'antenna_files': filenames,
                              'spwid': spwid,
                              'polids': pols}

                # create job for imaging
                job = jobrequest.JobRequest(worker.execute, **parameters)
                
                # create job for full channel image
                LOG.info('create full channel image')
                if pols == [0,1]:
                    polstr = 'XXYY'
                elif pols == [0] or pols == 0:
                    polstr = 'XX'
                elif pols == [1] or pols == 1:
                    polstr = 'YY'
                else:
                    polstr = 'I'
                imagename = '%s.%s.spw%s.%s.image'%(source_name,name,spwid,polstr)
                kwargs = {'imagename':imagename}

                # execute job
                self._executor.execute(job)
                
                if imagename is not None:
                    image_item = imagelibrary.ImageItem(imagename=imagename,
                                                        sourcename=source_name,
                                                        spwlist=spwid,
                                                        sourcetype='TARGET')
                    image_item.antenna = name

                    result = SDImagingResults(task = self.__class__,
                                              success = True, 
                                              outcome = image_item)
                    result.task = self.__class__

                    if self.inputs.context.subtask_counter is 0: 
                        result.stage_number = self.inputs.context.task_counter - 1
                    else:
                        result.stage_number = self.inputs.context.task_counter 

                    results.append(result)
        return results

    def analyse(self, result):
        return result


class SDImagingWorker(object):

    def __init__(self):
        pass

    def execute(self, datatable, reference_data, source_name, antenna_name, antenna_indices, antenna_files, spwid, polids):
        # spectral window
        spw = reference_data.spectral_window[spwid]
        refpix = spw.refpix
        refval = spw.refval
        increment = spw.increment
        rest_freqs = spw.rest_frequencies
        nchan = spw.nchan
        
        # beam size
        grid_size = casatools.quanta.convert(reference_data.beam_size[spwid], 'deg')['value']

        # gridding engine
        observing_pattern = reference_data.pattern[spwid].values()[0]
        gridding_class = gridding_factory(observing_pattern)

        # assume all members have same calmode
        calmode = reference_data.calibration_strategy['calmode']
        srctype = common.SrcTypeMap(calmode)
        
        data_array = []
        for pol in polids:                        
            worker = gridding_class(datatable, antenna_indices, antenna_files, spwid, pol, srctype, nchan, grid_size)

            (spectra,grid_table) = worker.execute()

            data_array.append(spectra)

        # imaging
        LOG.todo('How to set edge parameter? Is it local? or global?')
        edge = []
        worker = SDImageGenerator(data_array, edge)
        antenna = reference_data.ms.antenna_array.name
        observer = reference_data.observer
        obs_date = reference_data.start_time
        worker.define_image(grid_table, 
                            freq_refpix=refpix, freq_refval=refval,
                            freq_increment=increment,
                            rest_frequency=rest_freqs,
                            antenna=antenna, observer=observer, 
                            obs_date=obs_date)

        # create image from gridded data
        LOG.info('create full channel image')
        if polids == [0,1]:
            polstr = 'XXYY'
        elif polids == [0] or polids == 0:
            polstr = 'XX'
        elif polids == [1] or polids == 1:
            polstr = 'YY'
        else:
            polstr = 'I'
        imagename = '%s.%s.spw%s.%s.image'%(source_name.replace(' ','_'),antenna_name,spwid,polstr)
        kwargs = {'imagename':imagename}
        worker.full_channel_image(imagename=imagename)

        

