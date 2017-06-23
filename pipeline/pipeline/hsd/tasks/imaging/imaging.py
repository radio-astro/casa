from __future__ import absolute_import

import os
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks
#from pipeline.hif.heuristics import fieldnames
from pipeline.h.heuristics import fieldnames
from pipeline.domain import DataTable
from . import gridding
from . import weighting
from . import worker
from . import sdcombine
from .. import common
from ..common import utils as sdutils
from ..baseline import msbaseline

#import memory_profiler

LOG = infrastructure.get_logger(__name__)

class SDImagingInputs(basetask.StandardInputs):
    """
    Inputs for imaging
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, mode=None, infiles=None,
                 field=None, spw=None):
        self._init_properties(vars())
        self.vis = infiles
        self.infiles = self.vis
        if self.mode is None: self.mode='line'

    @property
    def msid_list(self):
        """
        Returns MS index in context observing run specified as infiles.
        """
        ms_names = [ms.name for ms in self.context.observing_run.measurement_sets]
        return map(ms_names.index, map(os.path.abspath, self.infiles))

    @property
    def antenna(self):
        return ''
#         if self._antenna is not None:
#             return self._antenna
#   
#         antennas = self.ms.get_antenna(self._antenna)
#         return ','.join([str(a.id) for a in antennas])
#   
#     @antenna.setter
#     def antenna(self, value):
#         if value is None:
#             # use all antenna by default
#             value = ''
#         self._antenna = value

    @property
    def field(self):
        if not callable(self._field):
            return self._field
        # filters field with intents in self.intent
        fields = set()
        for idx in self.msid_list:
            # This assumes the same fields in all MSes
            msobj = self.context.observing_run.measurement_sets[idx]
            # this will give something like '0542+3243,0343+242'
            intent_fields = self._field(msobj, self.intent)
            fields.update(utils.safe_split(intent_fields))

        return ','.join(fields)

    @field.setter
    def field(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._field = value

    @property
    def intent(self):
        return "TARGET"

    @property
    def spw(self):
        if self._spw is not None:
            return self._spw
        # filters science spws by default (assumes the same spw setting for all MSes)
        msobj = self.context.observing_run.measurement_sets[self.msid_list[0]]
        science_spws = msobj.get_spectral_windows(self._spw, with_channels=True)
        return ','.join([str(spw.id) for spw in science_spws])

    @spw.setter
    def spw(self, value):
        self._spw = value


class SDImagingResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDImagingResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDImagingResults, self).merge_with_context(context)
        LOG.todo('need to decide what is done in SDImagingResults.merge_with_context')
        
        if self.outcome.has_key('export_results'):
            self.outcome['export_results'].merge_with_context(context)
            
        # register ImageItem object to context.sciimlist if antenna is COMBINED
        if self.outcome.has_key('image'):
            image_item = self.outcome['image']
            if isinstance(image_item, imagelibrary.ImageItem) \
              and image_item.antenna == 'COMBINED':
                context.sciimlist.add_item(image_item)

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return self.outcome['image'].imagename

class SDImaging(basetask.StandardTaskTemplate):
    Inputs = SDImagingInputs
    # stokes to image and requred POLs for it
    stokes = 'I'
    # for linear feed in ALMA. this affects pols passed to gridding module
    required_pols = ['XX', 'YY']

    def is_multi_vis_task(self):
        return True

    #@memory_profiler.profile
    def prepare(self):
        inputs = self.inputs
        context = inputs.context
        reduction_group = context.observing_run.ms_reduction_group
        infiles = inputs.infiles
        # list of ms to process
        ms_list = [context.observing_run.measurement_sets[idx] for idx in inputs.msid_list]
        ms_names = [msobj.name for msobj in ms_list]
        in_spw = inputs.spw
        # in_field is comma-separated list of target field names that are 
        # extracted from all input MSs
        in_field = inputs.field
#         antennalist = inputs.antennalist
        imagemode = inputs.mode.upper()
        datatable = DataTable(name=context.observing_run.ms_datatable_name, readonly=True)
        logrecords = []
        cqa = casatools.quanta
         
        # task returns ResultsList
        results = basetask.ResultsList()
        # search results and retrieve edge parameter from the most
        # recent SDBaselineResults if it exists
        getresult = lambda r : r.read() if hasattr(r, 'read') else r
        registered_results = [getresult(r) for r in context.results]
        baseline_stage = -1
        for stage in xrange(len(registered_results) - 1, -1, -1):
            if isinstance(registered_results[stage], msbaseline.SDMSBaselineResults):
                baseline_stage = stage
        if baseline_stage > 0:
            edge = list(registered_results[baseline_stage].outcome['edge'])
            LOG.info('Retrieved edge information from SDBaselineResults: %s' % (edge))
        else:
            LOG.info('No SDBaselineResults available. Set edge as [0,0]')
            edge = [0, 0]
         
        # loop over reduction group (spw and source combination)
        for (group_id, group_desc) in reduction_group.iteritems():
            LOG.debug('Processing Reduction Group %s'%(group_id))
            LOG.debug('Group Summary:')
            for m in group_desc:
                LOG.debug('\t%s: Antenna %d (%s) Spw %d Field %d (%s)' % \
                          (os.path.basename(m.ms.work_data), m.antenna_id,
                           m.antenna_name, m.spw_id, m.field_id, m.field_name))
            # Which group in group_desc list should be processed
            
            # fix for CAS-9747
            # There may be the case that observation didn't complete so that some of 
            # target fields are missing in MS. In this case, directly pass in_field 
            # to get_valid_ms_members causes trouble. As a workaround, ad hoc pre-selection 
            # of field name is applied here.
            # 2017/02/23 TN
            field_sel = ''
            if len(in_field) == 0:
                # fine, just go ahead
                field_sel = in_field
            elif group_desc.field_name in map(lambda x: x.strip('"'), in_field.split(',')):
                # pre-selection of the field name
                field_sel = group_desc.field_name
            else:
                # no field name is included in in_field, skip
                LOG.info('Skip reduction group %d'%(group_id))
                continue
                
            member_list = list(common.get_valid_ms_members(group_desc, ms_names, inputs.antenna, field_sel, in_spw))
            LOG.trace('group %s: member_list=%s'%(group_id, member_list))
            
            # skip this group if valid member list is empty
            if len(member_list) == 0:
                LOG.info('Skip reduction group %d'%(group_id))
                continue
 
            member_list.sort() #list of group_desc IDs to image
            antenna_list = [group_desc[i].antenna_id for i in member_list]
            spwid_list = [group_desc[i].spw_id for i in member_list]
            ms_list = [group_desc[i].ms for i in member_list]
            fieldid_list = [group_desc[i].field_id for i in member_list]
            temp_dd_list = [ms_list[i].get_data_description(spw=spwid_list[i]) \
                            for i in xrange(len(member_list))]
            channelmap_range_list = [ group_desc[i].channelmap_range for i in member_list ]
            # this becomes list of list [[poltypes for ms0], [poltypes for ms1], ...]
#             polids_list = [[ddobj.get_polarization_id(corr) for corr in ddobj.corr_axis \
#                             if corr in self.required_pols ] for ddobj in temp_dd_list]
            pols_list = [[corr for corr in ddobj.corr_axis \
                          if corr in self.required_pols ] for ddobj in temp_dd_list]
            del temp_dd_list
             
            LOG.debug('Members to be processed:')
            for i in xrange(len(member_list)):
                LOG.debug('\t%s: Antenna %s Spw %s Field %s' % \
                          (os.path.basename(ms_list[i].work_data),
                           antenna_list[i], spwid_list[i], fieldid_list[i]))
 
            #continue
 
            # image is created per antenna (science) or per asdm and antenna (ampcal)
            image_group = {}
            for (msobj, ant, spwid, fieldid, pollist, chanmap) in zip(ms_list, antenna_list,
                                                             spwid_list, fieldid_list,
                                                             pols_list, channelmap_range_list):
                field_name = msobj.fields[fieldid].name
                identifier = field_name
                antenna = msobj.antennas[ant].name
                identifier += ('.'+antenna)
                # create image per asdm and antenna for ampcal
                if imagemode=='AMPCAL':
                    asdm_name = common.asdm_name_from_ms(msobj)
                    identifier += ('.'+asdm_name)
                if identifier in image_group.keys():
                    image_group[identifier].append([msobj, ant, spwid, fieldid, pollist, chanmap])
                else:
                    image_group[identifier] = [[msobj, ant, spwid, fieldid, pollist, chanmap]]
            LOG.debug('image_group=%s' % (image_group))
 
            # loop over antennas
            combined_infiles = []
            combined_antids = []
            combined_fieldids = []
            combined_spws = []
            tocombine_images = []
            combined_pols = []
            combined_rms_exclude = []
  
            coord_set = False
            for (name, _members) in image_group.iteritems():
                msobjs =  map(lambda x: x[0], _members)
                antids = map(lambda x: x[1], _members)
                spwids = map(lambda x: x[2], _members)
                fieldids = map(lambda x: x[3], _members)
                polslist = map(lambda x: x[4], _members)
                chanmap_range_list = map(lambda x: x[5], _members)
                LOG.info("Processing image group: %s" % name)
                for idx in xrange(len(msobjs)):
                    LOG.info("\t%s: Antenna %d (%s) Spw %s Field %d (%s)" % \
                             (msobjs[idx].basename,
                              antids[idx], msobjs[idx].antennas[antids[idx]].name,
                              spwids[idx], fieldids[idx],
                              msobjs[idx].fields[fieldids[idx]].name))
                # reference data is first MS
                ref_ms = msobjs[0]
                ant_name = ref_ms.antennas[antids[0]].name
                # for ampcal
                asdm = None
                if imagemode=='AMPCAL':
                    asdm = common.asdm_name_from_ms(ref_ms)
 
                # source name
                source_name =  group_desc.field_name.replace(' ', '_')
                  
                # filenames for gridding
                infiles = [ms.work_data for ms in msobjs]
  
                LOG.debug('infiles=%s' % (infiles))
                  
                # image name
                namer = filenamer.Image()
                namer.casa_image()
                namer.source(source_name)
                namer.antenna_name(ant_name)
                namer.asdm(asdm)
                namer.spectral_window(spwids[0])
                namer.polarization(self.stokes)
                imagename = namer.get_filename()
                LOG.info("Output image name: %s" % imagename)
                 
                # Step 1.
                # Initialize weight column based on baseline RMS.
                LOG.info('Set weights based on baseline RMS')
                for i in xrange(len(msobjs)):
                    msobj = msobjs[i]
                    antid = antids[i]
                    spwid = spwids[i]
                    fieldid = fieldids[i]
                    original_ms = msobj.name
                    work_ms = msobj.work_data
                    # weighting module sets weight to all pols
                    LOG.info('Setting weight for %s Antenna %s Spw %s Field %s' % \
                             (os.path.basename(work_ms), msobj.antennas[antid].name,
                              spwid, msobj.fields[fieldid].name))
########## TODO: NEED TO HANDLE SPWTYPE PROPERLY
                    spwtype = msobj.spectral_windows[spwid].type
                    weighting_inputs = weighting.WeightMSInputs(context, infile=original_ms, 
                                                                outfile=work_ms, antenna=antid,
                                                                spwid=spwid, fieldid=fieldid,
                                                                spwtype=spwtype)
                    weighting_task = weighting.WeightMS(weighting_inputs)
                    weighting_result = self._executor.execute(weighting_task, merge=True)
                    logrecords.extend(weighting_result.logrecords)
   
                # Step 2.
                # Imaging
                # Image per antenna, source
                LOG.info('Imaging Source %s, Ant %s Spw %d' % (source_name, ant_name, spwids[0]))
                # map coordinate (use identical map coordinate per spw)
                if not coord_set:
                    image_coord = worker.ALMAImageCoordinateUtil(context, datatable, infiles, antids, spwids, fieldids)
                    if not image_coord: #No valid data is found
                        continue
                    coord_set = True
                    (phasecenter, cellx, celly, nx, ny) = image_coord
  
                # register data for combining
                combined_infiles.extend(infiles)
                combined_antids.extend(antids)
                combined_fieldids.extend(fieldids)
                combined_spws.extend(spwids)
                combined_pols.extend(polslist)
                
                imager_inputs = worker.SDImagingWorker.Inputs(context, infiles, 
                                                              outfile=imagename,
                                                              mode=imagemode,
                                                              antids=antids,
                                                              spwids=spwids,
                                                              fieldids=fieldids,
                                                              stokes = self.stokes,
                                                              edge=edge,
                                                              phasecenter=phasecenter,
                                                              cellx=cellx,
                                                              celly=celly,
                                                              nx=nx, ny=ny)
                imager_task = worker.SDImagingWorker(imager_inputs)
                imager_result = self._executor.execute(imager_task, merge=True)
                logrecords.extend(imager_result.logrecords)
                  
                if imager_result.outcome is not None:
                    # Imaging was successful, proceed following steps
  
                    # add image list to combine
                    if os.path.exists(imagename) and os.path.exists(imagename+'.weight'):
                        tocombine_images.append(imagename)
  
                    # Additional Step.
                    # Make grid_table and put rms and valid spectral number array 
                    # to the outcome.
                    # The rms and number of valid spectra is used to create RMS maps.
                    validsps = []
                    rmss = []
#                     if imagemode != 'AMPCAL':
                    LOG.info('Additional Step. Make grid_table')
                    with casatools.ImageReader(imager_result.outcome) as ia:
                        cs = ia.coordsys()
                        dircoords = [i for i in xrange(cs.naxes())
                                     if cs.axiscoordinatetypes()[i] == 'Direction']
                        nx = ia.shape()[dircoords[0]]
                        ny = ia.shape()[dircoords[1]]
 
                    observing_pattern = msobjs[0].observing_pattern[antids[0]][spwids[0]][fieldids[0]]
                    grid_task_class = gridding.gridding_factory(observing_pattern)
                    grid_tables = []
                    grid_input_dict = {}
                    for (msobj, antid, spwid, fieldid, poltypes, _dummy) in _members:
                        msname = msobj.name # Use parent ms
                        for p in poltypes:
                            if not grid_input_dict.has_key(p):
                                grid_input_dict[p] = [[msname], [antid], [fieldid], [spwid]]
                            else:
                                grid_input_dict[p][0].append(msname)
                                grid_input_dict[p][1].append(antid)
                                grid_input_dict[p][2].append(fieldid)
                                grid_input_dict[p][3].append(spwid)

                    # Generate grid table for each POL in image (per ANT,
                    # FIELD, and SPW, over all MSes)
                    for (pol,member) in grid_input_dict.iteritems():
                        _mses = member[0]
                        _antids = member[1]
                        _fieldids = member[2]
                        _spwids = member[3]
                        _pols = [pol for i in xrange(len(_mses))]
                        gridding_inputs = grid_task_class.Inputs(context, msnames=_mses, 
                                                                 antennaids=_antids, 
                                                                 fieldids=_fieldids,
                                                                 spwids=_spwids,
                                                                 poltypes=_pols,
                                                                 nx=nx, ny=ny)
                        gridding_task = grid_task_class(gridding_inputs)
                        gridding_result = self._executor.execute(gridding_task, merge=True)
                        grid_tables.append(gridding_result.outcome)
                        logrecords.extend(gridding_result.logrecords)
                    # Extract RMS and number of spectra from grid_tables
                    for i in xrange(len(grid_input_dict)):
                        validsps.append([r[6] for r in grid_tables[i]])
                        rmss.append([r[8] for r in grid_tables[i]])
                    
                    # define RMS ranges in image
                    with casatools.ImageReader(imager_result.outcome) as ia:
                        cs = ia.coordsys()
                        frequency_frame = cs.getconversiontype('spectral')
                        rms_exclude_freq = self._get_rms_exclude_freq_range_image(frequency_frame, chanmap_range_list, edge, msobjs, antids, spwids, fieldids)
                        LOG.info("#####FREQ RANGE IN IMAGE FRAME = %s" % str(rms_exclude_freq))
                    combined_rms_exclude.extend(rms_exclude_freq)
 
                    image_item = imagelibrary.ImageItem(imagename=imagename,
                                                        sourcename=source_name,
                                                        spwlist=spwids,
                                                        specmode='cube',
                                                        sourcetype='TARGET')
                    image_item.antenna = ant_name #name #(group name)
                    outcome = {}
                    outcome['image'] = image_item
                    outcome['imagemode'] = imagemode
                    outcome['validsp'] = validsps
                    outcome['rms'] = rmss
                    outcome['edge'] = edge
                    outcome['reduction_group_id'] = group_id
                    outcome['file_index'] = [common.get_parent_ms_idx(context, name) for name in infiles]
                    outcome['assoc_antennas'] = antids
                    outcome['assoc_fields'] = fieldids
                    outcome['assoc_spws'] = spwids
#                     outcome['assoc_pols'] = pols
                    if imagemode == 'AMPCAL':
                        if len(infiles)==1 and (asdm not in ['', None]): outcome['vis'] = asdm
#                         # to register exported_ms to each scantable instance
#                         outcome['export_results'] = export_results
  
                    result = SDImagingResults(task=self.__class__,
                                              success=True,
                                              outcome=outcome)
                    result.task = self.__class__
  
                    result.stage_number = inputs.context.task_counter 
                                                  
                    results.append(result)
                      
            if imagemode == 'AMPCAL':
                LOG.info("Skipping combined image for the amplitude calibrator.")
                continue
  
            # Make combined image
            if len(tocombine_images) == 0:
                LOG.warn("No valid image to combine for Source %s, Spw %d" % (source_name, spwids[0]))
                continue
            # reference MS
            ref_ms = context.observing_run.get_ms(name=sdutils.get_parent_ms_name(context, combined_infiles[0]))
            
            # image name
            namer = filenamer.Image()
            namer.casa_image()
            namer.source(source_name)
            namer.spectral_window(combined_spws[0])
            namer.polarization(self.stokes)
            imagename = namer.get_filename()
  
            # Step 3.
            # Imaging of all antennas
            LOG.info('Combine images of Source %s Spw %d' % (source_name, combined_spws[0]))
            if False:
                imager_inputs = worker.SDImagingWorker.Inputs(context, combined_infiles, 
                                                              outfile=imagename, mode=imagemode,
                                                              antids=combined_antids,
                                                              spwids=combined_spws,
                                                              fieldids=combined_fieldids,
                                                              stokes = self.stokes,
                                                              edge=edge,
                                                              phasecenter=phasecenter,
                                                              cellx=cellx, celly=celly,
                                                              nx=nx, ny=ny)
                imager_task = worker.SDImagingWorker(imager_inputs)
                imager_result = self._executor.execute(imager_task, merge=True)
            else:
                combine_inputs = sdcombine.SDImageCombineInputs(context, inimages=tocombine_images,
                                                                outfile=imagename)
                combine_task = sdcombine.SDImageCombine(combine_inputs)
                imager_result = self._executor.execute(combine_task, merge=True)

            logrecords.extend(imager_result.logrecords)
            if imager_result.outcome is not None:
                # Imaging was successful, proceed following steps
      
                # Additional Step.
                # Make grid_table and put rms and valid spectral number array 
                # to the outcome
                # The rms and number of valid spectra is used to create RMS maps
                LOG.info('Additional Step. Make grid_table')
                with casatools.ImageReader(imager_result.outcome) as ia:
                    cs = ia.coordsys()
                    dircoords = [i for i in xrange(cs.naxes())
                                 if cs.axiscoordinatetypes()[i] == 'Direction']
                    nx = ia.shape()[dircoords[0]]
                    ny = ia.shape()[dircoords[1]]
                validsps = []
                rmss = []
                observing_pattern =  ref_ms.observing_pattern[combined_antids[0]][combined_spws[0]][combined_fieldids[0]]
                grid_task_class = gridding.gridding_factory(observing_pattern)
                grid_tables = []
                grid_input_dict = {}
                for (msname, antid, spwid, fieldid, poltypes) in \
                zip(combined_infiles,combined_antids,combined_spws,combined_fieldids,combined_pols):
                    msobj = context.observing_run.get_ms(name=common.get_parent_ms_name(context,msname)) # Use parent ms
                    ddobj = msobj.get_data_description(spw=spwid)
                    for p in poltypes:
                        if not grid_input_dict.has_key(p):
                            grid_input_dict[p] = [[msname], [antid], [fieldid], [spwid]]
                        else:
                            grid_input_dict[p][0].append(msname)
                            grid_input_dict[p][1].append(antid)
                            grid_input_dict[p][2].append(fieldid)
                            grid_input_dict[p][3].append(spwid)
  
                for (pol,member) in grid_input_dict.iteritems():
                    _mses = member[0]
                    _antids = member[1]
                    _fieldids = member[2]
                    _spwids = member[3]
                    _pols = [pol for i in xrange(len(_mses))]
                    gridding_inputs = grid_task_class.Inputs(context, msnames=_mses, 
                                                             antennaids=_antids, 
                                                             fieldids=_fieldids,
                                                             spwids=_spwids,
                                                             poltypes=_pols,
                                                             nx=nx, ny=ny)
                    gridding_task = grid_task_class(gridding_inputs)
                    gridding_result = self._executor.execute(gridding_task, merge=True)
                    logrecords.extend(gridding_result.logrecords)
                    grid_tables.append(gridding_result.outcome)
                # Extract RMS and number of spectra from grid_tables
                for i in xrange(len(grid_input_dict)):
                    validsps.append([r[6] for r in grid_tables[i]])
                    rmss.append([r[8] for r in grid_tables[i]])

                # calculate RMS of line free frequencies in a combined image
                representative_bw = ref_ms.representative_target[2]
                is_representative_spw = (ref_ms.get_representative_source_spw()[1]==combined_spws[0] and \
                                         True) #representative_bw is not None)
                with casatools.ImageReader(imager_result.outcome) as ia:
                    cs = ia.coordsys()
                    faxis = cs.findaxisbyname('spectral')
                    num_chan = ia.shape()[faxis]
                    chan_width = cs.increment()['numeric'][faxis]
                    brightnessunit = ia.brightnessunit()
                ref_world = cs.referencevalue()['numeric']
                rms_exclude_freq = self._merge_ranges(combined_rms_exclude)
                LOG.info("#####FREQ RANGE IN IMAGE FRAME = %s" % str(rms_exclude_freq))
                rms_exclude_chan = numpy.zeros((len(rms_exclude_freq), 2), dtype=numpy.double)
                for i in range(len(rms_exclude_freq)):
                    segment = rms_exclude_freq[i]
                    ref_world[faxis] = segment[0]
                    start_chan = cs.topixel(ref_world)['numeric'][faxis]
                    ref_world[faxis] = segment[1]
                    end_chan = cs.topixel(ref_world)['numeric'][faxis]
                    # handling of LSB
                    rms_exclude_chan[i] = [max(int(min(start_chan, end_chan)), 0),
                                           min(int(max(start_chan, end_chan)), num_chan-1)]
                LOG.info("#####EXCLUDE CHANNEL RANGE IN IMAGE FRAME = %s" % str(rms_exclude_chan))
                include_channel_range = []
                if len(rms_exclude_chan) == 0:
                    include_channel_range = [edge[0], num_chan-1-edge[1]]
                else:
                    if rms_exclude_chan[0][0] > edge[0]:
                        include_channel_range.extend([edge[0], rms_exclude_chan[0][0]-1])
                    for j in range(len(rms_exclude_chan)-1):
                        include_channel_range.extend([rms_exclude_chan[j][1]+1, rms_exclude_chan[j+1][0]-1])
                    if rms_exclude_chan[-1][1] + 1 < num_chan-1-edge[1]:
                        include_channel_range.extend([rms_exclude_chan[-1][1] + 1, num_chan-1-edge[1]])
                LOG.info("#####RMS CHANNEL RANGE IN IMAGE FRAME = %s" % str(include_channel_range))

                stat_chans = str(';').join([ '%d~%d' % (include_channel_range[iseg], include_channel_range[iseg+1]) for iseg in range(0, len(include_channel_range), 2) ])
                # statistics
                imstat_job = casa_tasks.imstat(imagename=imagename, chans=stat_chans)
                statval = self._executor.execute(imstat_job)
                image_rms = statval['rms'][0]
                LOG.info("Statistics of line free channels (%s): RMS = %f %s, Stddev = %f %s, Mean = %f %s" % (stat_chans, statval['rms'][0], brightnessunit, statval['sigma'][0], brightnessunit, statval['mean'][0], brightnessunit))

                # estimate 
                if is_representative_spw:
                    if representative_bw is None: # temporal assignment
                        representative_bw = cqa.quantity(chan_width, 'Hz')
                    LOG.info("Estimate RMS in representative bandwidth: %fkHz (native: %fkHz)" % \
                             (cqa.getvalue(cqa.quantity(representative_bw, 'kHz')), chan_width*1.e-3))
                    LOG.todo("DO SOMETHING TO ESTIMATE RMS")
                    chan_width = cqa.getvalue(cqa.quantity(representative_bw, 'Hz'))[0]

                # calculate channel and frequency ranges of line free channels
                ref_pixel = cs.referencepixel()['numeric']
                freqs = []
                for ichan in include_channel_range:
                    ref_pixel[faxis] = ichan
                    freqs.append(cs.toworld(ref_pixel)['numeric'][faxis])
                if len(freqs) > 1 and freqs[0] > freqs[1]: #LSB
                    freqs.reverse()
                stat_freqs = str(', ').join([ '%f~%fGHz' % (freqs[iseg]*1.e-9, freqs[iseg+1]*1.e-9) for iseg in range(0, len(freqs), 2) ])
                  
                image_item = imagelibrary.ImageItem(imagename=imagename,
                                                    sourcename=source_name,
                                                    spwlist=combined_spws,
                                                    specmode='cube',
                                                    sourcetype='TARGET')
                image_item.antenna = 'COMBINED'
                outcome = {}
                outcome['image'] = image_item
                outcome['imagemode'] = imagemode
                outcome['validsp'] = validsps
                outcome['rms'] = rmss
                outcome['edge'] = edge
                outcome['reduction_group_id'] = group_id
                outcome['file_index'] = [common.get_parent_ms_idx(context, name) for name in combined_infiles]
                outcome['assoc_antennas'] = combined_antids
                outcome['assoc_fields'] = combined_fieldids
                outcome['assoc_spws'] = combined_spws
                outcome['image_sensitivity'] = {'frequency_range': stat_freqs, 'rms': image_rms,
                                                'channel_width': chan_width, 'representative': is_representative_spw}
#                 outcome['assoc_pols'] = pols

#                 # to register exported_ms to each scantable instance
#                 outcome['export_results'] = export_results
                result = SDImagingResults(task=self.__class__,
                                          success=True,
                                          outcome=outcome)
                result.task = self.__class__
  
                result.stage_number = inputs.context.task_counter 
                      
                results.append(result)
                                      
        LOG.todo('logrecords for SDImagingResults must be handled properly')
        # only add logrecords to first result
        if len(results) > 0:
            results[0].logrecords = logrecords
            for r in results[1:]:   
                r.logrecords = []

        return results
    
    def analyse(self, result):
        return result

    def _get_rms_exclude_freq_range_image(self, to_frame, chanmap_range_list, edge, msobjs, antids, spwids, fieldids):
        image_rms_freq_range = []
        channelmap_range = []
        print("##### chanmap_range=%s" % str(chanmap_range_list))
        for chanmap_range in chanmap_range_list:
            for map_range in chanmap_range:
                if map_range[2]:
                    min_chan = int(map_range[0]-map_range[1]*0.5)
                    max_chan = int(numpy.ceil(map_range[0]+map_range[1]*0.5))
                    channelmap_range.append([min_chan, max_chan])
        LOG.info("#####CHANNEL MAP RANGE = %s" % str(channelmap_range))
        for i in range(len(msobjs)):
            # define channel ranges of lines and deviation mask for each MS
            msobj = msobjs[i]
            fieldid = fieldids[i]
            antid = antids[i]
            spwid = spwids[i]
            spwobj = msobj.get_spectral_window(spwid)
            exclude_range = msobj.deviation_mask[(fieldid, antid, spwid)]
            LOG.info("#####MS: %s" % str(msobj.basename))
            LOG.info("#####DEVIATION MASK = %s" % str(msobj.deviation_mask[(fieldid, antid, spwid)]))
            if len(exclude_range)==1 and exclude_range[0] == [0, spwobj.num_channels-1]:
                # deviation mask is full channel range when all data are flagged
                LOG.warn("Ignoring DEVIATION MASK of %s (SPW %d, FIELD %d, ANT %d). Possibly all data flagged" % (msobj.basename, spwid, antid, fieldid))
                exclude_range = []
            if edge[0] > 0: exclude_range.append([0, edge[0]-1])
            if edge[1] > 0: exclude_range.append([spwobj.num_channels-edge[1], spwobj.num_channels-1])
            if len(channelmap_range) >0:
                exclude_range.extend(channelmap_range)
            exclude_channel_range = self._merge_ranges(exclude_range)
            LOG.info("#####CHANNEL MAP AND DEVIATION MASK CHANNEL RANGE = %s" % str(exclude_channel_range))
#             # define channel ranges of RMS (1-D list for start and end channel ids)
#             include_channel_range = []
#             if len(exclude_channel_range) == 0:
#                 include_channel_range = [0, spwobj.num_channels-1]
#             else:
#                 if exclude_channel_range[0][0] > 0:
#                     include_channel_range.extend([0, exclude_channel_range[0][0]-1])
#                 for j in range(len(exclude_channel_range)-1):
#                     include_channel_range.extend([exclude_channel_range[j][1]+1, exclude_channel_range[j+1][0]-1])
#                 if exclude_channel_range[-1][1] + 1 < spwobj.num_channels-1:
#                     include_channel_range.extend([exclude_channel_range[-1][1] + 1, spwobj.num_channels-1])
            # define frequency ranges of RMS
            exclude_freq_range = numpy.zeros(2*len(exclude_channel_range))
            for jseg in range(len(exclude_channel_range)):
                (lfreq, rfreq) = (spwobj.channels.chan_freqs[jchan] for jchan in exclude_channel_range[jseg])
                # handling of LSB
                exclude_freq_range[2*jseg:2*jseg+2] = [min(lfreq, rfreq), max(lfreq, rfreq)]
            LOG.info("#####CHANNEL MAP AND DEVIATION MASK FREQ RANGE = %s" % str(exclude_freq_range))
            if len(exclude_freq_range)==0: continue # no ranges to add
            # convert MS freqency ranges to image frame
            field = msobj.fields[fieldid]
            direction_ref = field.mdirection
            start_time = msobj.start_time
            end_time = msobj.end_time
            me = casatools.measures
            qa = casatools.quanta
            qmid_time = qa.quantity(start_time['m0'])
            qmid_time = qa.add(qmid_time, end_time['m0'])
            qmid_time = qa.div(qmid_time, 2.0)
            time_ref = me.epoch(rf=start_time['refer'], 
                                v0=qmid_time)
            position_ref = msobj.antennas[antid].position
                    
            # initialize
            me.done()
            me.doframe(time_ref)
            me.doframe(direction_ref)
            me.doframe(position_ref)
            def _to_imageframe(x):
                m = me.frequency(rf=spwobj.frame, v0=qa.quantity(x, 'Hz'))
                converted = me.measure(v=m, rf=to_frame)
                qout = qa.convert(converted['m0'], outunit='Hz')
                return qout['value']
            to_imageframe = numpy.vectorize(_to_imageframe)
            image_rms_freq_range.append(to_imageframe(exclude_freq_range))
        LOG.info("#####Overall LINE CHANNELS IN IMAGE FRAME = %s" % str(image_rms_freq_range))
        if len(image_rms_freq_range) == 0:
            return image_rms_freq_range
        return self._merge_ranges(numpy.reshape(image_rms_freq_range, (len(image_rms_freq_range), 2), 'C'))

    
    def _merge_ranges(self, range_list):
        """
        Merge overlapping ranges in range_list = [ [min0, max0], [min1, max1], .... ]
        """
        LOG.info("#####Merge ranges: %s" % str(range_list))
        num_range = len(range_list)
        if num_range == 0:
            return []
        merged = [ range_list[0][0:2] ]
        for i in range(1, num_range):
            segment = range_list[i]
            if len(segment) < 2:
                raise ValueError, "segments in range list much have 2 elements"
            overlap = -1
            for j in range(len(merged)):
                if segment[1]<merged[j][0] or segment[0] > merged[j][1]: # no overlap
                    continue
                else:
                    overlap = j
                    break
            if overlap < 0:
                merged.append(segment[0:2])
            else:
                merged[j][0] = min(merged[j][0], segment[0])
                merged[j][1] = max(merged[j][1], segment[1])
        # Check if further merge is necessary
        while len(merged) < num_range:
            num_range = len(merged)
            merged = self._merge_ranges(merged)
        LOG.info("#####Merged: %s" % str(merged))
        return merged
            
            
    
