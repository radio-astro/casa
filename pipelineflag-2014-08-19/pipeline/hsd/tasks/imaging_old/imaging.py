from __future__ import absolute_import

import os
import math
#from math import cos, sqrt, exp
import numpy
import time

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.basetask as basetask
from .. import common
from ..baseline import baseline
from .gridding import gridding_factory
from .imagegenerator import SDImageGenerator

LOG = infrastructure.get_logger(__name__)

NoData = common.NoData

class SDImagingInputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, infiles=None, iflist=None, pollist=None):
        self._init_properties(vars())
        self._to_list(['infiles', 'iflist', 'pollist'])

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
        return self.outcome['image'].imagename

class SDImaging(common.SingleDishTaskTemplate):
    Inputs = SDImagingInputs

    @common.datatable_setter
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

        # search results and retrieve edge parameter from the most
        # recent SDBaselineResults if it exists
        results = [r.read() for r in context.results]
        baseline_stage = -1
        for stage in xrange(len(results)-1, -1, -1):
            if isinstance(results[stage], baseline.SDBaselineResults):
                baseline_stage = stage
        if baseline_stage > 0:
            edge = list(results[baseline_stage].outcome['edge'])
            LOG.info('Retrieved edge information from SDBaselineResults: %s'%(edge))
        else:
            LOG.info('No SDBaselineResults available. Set edge as [0,0]')
            edge = [0,0]
        
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
                target_sources = [v for v in st.source.values() \
                                  if 'TARGET' in v.intents]
                original_source_name = target_sources[0].name
                source_name = original_source_name.replace(' ','_')

                # filenames for gridding
                data_name = lambda x: x.baselined_name \
                            if os.path.exists(x.baselined_name) else x.name
                filenames = [data_name(context.observing_run[i]) for i in indices]

                LOG.debug('filenames=%s'%(filenames))
                
                # create job for full channel image
                LOG.info('create full channel image')

                # image name
                namer = filenamer.Image()
                namer.casa_image()
                namer.source(source_name)
                namer.antenna_name(name)
                namer.spectral_window(spwid)
                if pols == [0,1]:
                    polstr = 'XXYY'
                elif pols == [0] or pols == 0:
                    polstr = 'XX'
                elif pols == [1] or pols == 1:
                    polstr = 'YY'
                else:
                    polstr = 'I'
                namer.polarization(polstr)
                imagename = namer.get_filename()
                
                validsps = []
                rmss = []

                # do imaging
                self._do_imaging(imagename, datatable, st, source_name,
                                 name, indices, filenames, spwid, pols,
                                 edge, validsps, rmss)

                # edit image header
                with casatools.TableReader(imagename, nomodify=False) as tb:
                    imageinfo = {'imagetype': 'Intensity',
                                 'objectname': original_source_name}
                    tb.putkeyword('imageinfo', imageinfo)
                
                if imagename is not None:
                    image_item = imagelibrary.ImageItem(imagename=imagename,
                                                        sourcename=source_name,
                                                        spwlist=spwid,
                                                        sourcetype='TARGET')
                    image_item.antenna = name
                    outcome = {}
                    outcome['image'] = image_item
                    outcome['validsp'] = numpy.array(validsps)
                    outcome['rms'] = numpy.array(rmss)
                    outcome['edge'] = edge
                    result = SDImagingResults(task = self.__class__,
                                              success = True, 
                                              outcome = outcome)
                    result.task = self.__class__

                    if self.inputs.context.subtask_counter is 0: 
                        result.stage_number = self.inputs.context.task_counter - 1
                    else:
                        result.stage_number = self.inputs.context.task_counter 

                    results.append(result)

        LOG.todo('logrecords for SDImagingResults must be handled properly')
        for r in results:
            r.logrecords = []

        return results

    def _do_imaging(self, imagename, datatable, reference_data, source_name, antenna_name, antenna_indices, antenna_files, spwid, polids, edge, num_validsp_array, rms_array):
        # spectral window
        spw = reference_data.spectral_window[spwid]
        refpix = spw.refpix
        refval = spw.refval
        increment = spw.increment
        rest_freqs = spw.rest_frequencies
        nchan = spw.nchan
        freq_frame = spw.frame
        
        # beam size
        grid_size = casatools.quanta.convert(reference_data.beam_size[spwid], 'deg')['value']

        # gridding engine
        observing_pattern = reference_data.pattern[spwid].values()[0]
        gridding_class = gridding_factory(observing_pattern)

        # assume all members have same calmode
        srctype = reference_data.calibration_strategy['srctype']
        
        data_array = []
        #num_validsp_array = []
        #rms_array = []
        for pol in polids:                        
            gridder = gridding_class(datatable, antenna_indices, antenna_files, spwid, pol, srctype, nchan, grid_size)

            (spectra,grid_table) = self._executor.execute(gridder, merge=False)
            data_array.append(spectra)
            num_validsp_array.append([r[6] for r in grid_table])
            rms_array.append([r[8] for r in grid_table])

        # imaging
        image_generator = SDImageGenerator(data_array, edge)
        antenna = reference_data.ms.antenna_array.name
        observer = reference_data.observer
        obs_date = reference_data.start_time
        image_generator.define_image(grid_table,
                                     freq_refpix=refpix, freq_refval=refval,
                                     freq_increment=increment,
                                     freq_frame=freq_frame,
                                     rest_frequency=rest_freqs,
                                     antenna=antenna, observer=observer, 
                                     obs_date=obs_date)
        image_generator.imagename = imagename

        # create image from gridded data
        self._executor.execute(image_generator, merge=False)


    def analyse(self, result):
        return result


