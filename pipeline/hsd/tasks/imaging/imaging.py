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
#import pipeline.infrastructure.logging as logging
from .. import common
from .gridding import gridding_factory
from .imagegenerator import SDImageGenerator

LOG = infrastructure.get_logger(__name__)
#logging.set_logging_level('trace')
#logging.set_logging_level('info')

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
    SRCTYPE = {'ps': 0,
               'otf': 0,
               'otfraster': 0}

    def prepare(self):
        context = self.inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        grid_tables = []
        infiles = self.inputs.infiles
        iflist = self.inputs.iflist
        antennalist = self.inputs.antennalist
        pollist = self.inputs.pollist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]

        image_list = []
        results = basetask.ResultsList()
        for (group_id,group_desc) in reduction_group.items():
            # assume all members have same spw and pollist
            spwid = group_desc['member'][0][1]
            LOG.debug('spwid=%s'%(spwid))
            pols = group_desc['member'][0][2]
            if pollist is not None:
                pols = list(set(pollist).intersection(pols))

            # skip spw not included in iflist
            if iflist is not None and spwid not in iflist:
                LOG.debug('Skip spw %s'%(spwid))
                continue

            # image is created per antenna
            antenna_group = {}
            for m in group_desc['member']:
                antenna = context.observing_run[m[0]].antenna.name
                if antenna in antenna_group.keys():
                    antenna_group[antenna].append(m[0])
                else:
                    antenna_group[antenna] = [m[0]]

            # loop over antennas
            for (name,indices) in antenna_group.items():

                # skip antenna not included in antennalist
                if antennalist is not None and name not in antennalist:
                    LOG.debug('Skip antenna %s'%(name))
                    continue

                # first scantable 
                ant = indices[0]#group_desc['member'][0][0]
                st = context.observing_run[ant]

                # source name
                source_name = st.source[0].name.replace(' ','_')

                # get working class
                obs_pattern = st.pattern[spwid].values()[0]
                gridding_class = gridding_factory(obs_pattern)

                # assume all members have same calmode
                calmode = st.calibration_strategy['calmode']
                srctype = self.SRCTYPE[calmode] if self.SRCTYPE.has_key(calmode) else None

                # beam size
                grid_size = casatools.quanta.convert(st.beam_size[spwid], 'deg')['value']

                # information for spectral coordinate
                spw = st.spectral_window[spwid]
                refpix = spw.refpix
                refval = spw.refval
                increment = spw.increment
                rest_freqs = spw.rest_frequencies
                nchan = spw.nchan

                # loop over pol
                grid_table = None
                data_array = []
                for pol in pols:                        
                    worker = gridding_class(datatable, indices, spwid, pol, srctype, nchan, grid_size)
            
                    ## create job for gridding
                    #job = jobrequest.JobRequest(worker.execute) 
                    #
                    ## execute job
                    #(spectra,grid_table) = self._executor.execute(job)
                    (spectra,grid_table) = worker.execute()

                    data_array.append(spectra)

                # imaging
                LOG.todo('How to set edge parameter? Is it local? or global?')
                edge = []
                worker = SDImageGenerator(data_array, edge)
                antenna = st.ms.antenna_array.name
                observer = st.observer
                obs_date = st.start_time
                worker.define_image(grid_table, 
                                    freq_refpix=refpix, freq_refval=refval,
                                    freq_increment=increment,
                                    rest_frequency=rest_freqs,
                                    antenna=antenna, observer=observer, 
                                    obs_date=obs_date)
                
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
                imagename = '%s.spw%s.%s.image'%(name,spwid,polstr)
                kwargs = {'imagename':imagename}
                job = jobrequest.JobRequest(worker.full_channel_image,**kwargs)

                # execute job
                self._executor.execute(job)
                
                if imagename is not None:
                    #image_list.append(imagename)
                    image_item = imagelibrary.ImageItem(imagename=imagename,
                                                        sourcename=source_name,
                                                        spwlist=spwid,
                                                        sourcetype='TARGET')
                    image_item.antenna = name
                    #image_list.append(image_item)

                    result = SDImagingResults(task = self.__class__,
                                              success = True, 
                                              outcome = image_item)
                    result.task = self.__class__
                    results.append(result)

                    if self.inputs.context.subtask_counter is 0: 
                        result.stage_number = self.inputs.context.task_counter - 1
                    else:
                        result.stage_number = self.inputs.context.task_counter 
                    LOG.info('task class is %s'%(result.task.__name__))

        #result.task = self.__class__
        #return result
        return results

    def analyse(self, result):
        return result

