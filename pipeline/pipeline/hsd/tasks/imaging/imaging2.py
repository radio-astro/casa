from __future__ import absolute_import

import os
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.basetask as basetask
from . import gridding
from . import exportms
from . import applyflag
from . import weighting
from . import scaling
from . import worker
from .. import common
from ..baseline import baseline

LOG = infrastructure.get_logger(__name__)

class SDImaging2Inputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, infiles=None,
                 field=None, spw=None, scan=None, pol=None,
                 reffile=None):
        self._init_properties(vars())
        self._to_list(['infiles'])
        for key in ['spw', 'scan', 'pol']:
            val = getattr(self, key)
            if val is None or (val[0] == '[' and val[-1] == ']'):
                self._to_list([key])
        #self._to_list(['infiles', 'iflist', 'pollist'])

    @property
    def antennalist(self):
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

    @property
    def antennaid_list(self):
        st_names = self.context.observing_run.st_names
        return map(st_names.index, self.infiles)

    #@property
    #def spw(self):
    #    if len(self.iflist) == 0:
    #        return ''
    #    else:
    #        return callibrary.SDCalApplication.list_to_selection(self.iflist)

    #@property
    #def pol(self):
    #    if len(self.pollist) == 0:
    #        return ''
    #    else:
    #        return callibrary.SDCalApplication.list_to_selection(self.pollist)
            

class SDImaging2Results(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDImaging2Results, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDImaging2Results, self).merge_with_context(context)
        LOG.todo('need to decide what is done in SDImaging2Results.merge_with_context')
        
        if self.outcome.has_key('export_results'):
            self.outcome['export_results'].merge_with_context(context)

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return self.outcome['image'].imagename

class SDImaging2(common.SingleDishTaskTemplate):
    Inputs = SDImaging2Inputs

    def prepare(self):
        context = self.inputs.context
        reduction_group = context.observing_run.reduction_group
        infiles = self.inputs.infiles
        file_index = self.inputs.antennaid_list
        #iflist = self.inputs.iflist
        args = self.inputs.to_casa_args()
        scan = self.inputs.scan
        scansel_list = map(lambda x: self.inputs._to_casa_arg(scan, x), file_index)
        antennalist = self.inputs.antennalist
        #pollist = self.inputs.pollist
        st_names = context.observing_run.st_names
        reffile = self.inputs.reffile
        logrecords = []

        LOG.debug('scansel_list=%s'%(scansel_list))
        
        # task returns ResultsList
        results = basetask.ResultsList()

        # Step 1.
        # Apply flags to working scantable (antenna_files):
        #     - Apply SummaryFlag in Baseline Table to FLAGROW and FLAGTRA
        #     - Flag WVR and reference data
        LOG.info('Step 1. Apply flags')
        applyflag_inputs = applyflag.SDApplyFlag.Inputs(context, infiles)
        applyflag_task = applyflag.SDApplyFlag(applyflag_inputs)
        applyflag_results = self._executor.execute(applyflag_task, merge=True)
        logrecords.extend(applyflag_results.logrecords)

        # Step 2.
        # Export each scantable to MS
        LOG.info('Step 2. Export data')
        export_inputs = exportms.ExportMS.Inputs(context, infiles)
        export_task = exportms.ExportMS(export_inputs)
        export_results = self._executor.execute(export_task, merge=True)
        exported_mses = export_results.outcome
        logrecords.extend(export_results.logrecords)
        
        # to register exported_ms to each scantable instance
        #results.append(export_results)
        
        # Step 3.
        # Intensity scaling
        LOG.info('Step 3. Intensity scaling')
        scaling_inputs = scaling.IntensityScaling.Inputs(context, 
                                                         infiles=infiles, 
                                                         reffile=reffile)
        scaling_task = scaling.IntensityScaling(scaling_inputs)
        scaling_results = self._executor.execute(scaling_task, merge=True)
        logrecords.extend(scaling_results.logrecords)

        # search results and retrieve edge parameter from the most
        # recent SDBaselineResults if it exists
        getresult = lambda r : r.read() if hasattr(r, 'read') else r
        registered_results = [getresult(r) for r in context.results]
        baseline_stage = -1
        for stage in xrange(len(registered_results) - 1, -1, -1):
            if isinstance(registered_results[stage], baseline.SDBaselineResults):
                baseline_stage = stage
        if baseline_stage > 0:
            edge = list(registered_results[baseline_stage].outcome['edge'])
            LOG.info('Retrieved edge information from SDBaselineResults: %s' % (edge))
        else:
            LOG.info('No SDBaselineResults available. Set edge as [0,0]')
            edge = [0, 0]
        
        # loop over reduction group
        for (group_id, group_desc) in reduction_group.items():
            LOG.debug('Processing Reduction Group %s'%(group_id))
            LOG.debug('Group Summary:')
            for m in group_desc:
                LOG.debug('\tAntenna %s Spw %s Pol %s'%(m.antenna, m.spw, m.pols))

            #member_list = list(common.get_valid_members(group_desc, file_index, iflist))
            pols_list = list(common.pol_filter(group_desc, self.inputs.get_pollist))
            member_list = list(common.get_valid_members(group_desc, file_index, args['spw']))
            LOG.debug('group %s: member_list=%s'%(group_id, member_list))
            
            # skip this group if valid member list is empty
            if len(member_list) == 0:
                LOG.info('Skip reduction group %d'%(group_id))
                continue

            member_list.sort()
            antenna_list = [group_desc[i].antenna for i in member_list]
            spwid_list = [group_desc[i].spw for i in member_list]
            pols_list = [pols_list[i] for i in member_list]
                
            # assume all members have same spw and pollist
            first_member = group_desc[0]
            #pols = first_member.pols
            #if pollist is not None:
            #    pols = list(set(pollist).intersection(pols))

            LOG.debug('Members to be processed:')
            for i in xrange(len(member_list)):
                LOG.debug('\tAntenna %s Spw %s Pol %s'%(antenna_list[i], spwid_list[i], pols_list[i]))

            #continue

            # image is created per antenna
            antenna_group = {}
            for (ant, spwid, pols) in zip(antenna_list, spwid_list, pols_list):
                antenna = context.observing_run[ant].antenna.name
                if antenna in antenna_group.keys():
                    antenna_group[antenna].append([ant, spwid, pols])
                else:
                    antenna_group[antenna] = [[ant, spwid, pols]]
            LOG.info('antenna_group=%s' % (antenna_group))

            # loop over antennas
            combined_indices = []
            source_name = None
            combined_infiles = []
            combined_spws = []
            combined_scans = []
            combined_pols = []
            srctype = None
            for (name, _members) in antenna_group.items():
                indices = map(lambda x: x[0], _members)
                spwids = map(lambda x: x[1], _members)
                pols = map(lambda x: x[2], _members)
                net_pols = set()
                for p in map(set, pols):
                    net_pols = net_pols | p
                net_pols = list(net_pols)
                
                # reference data is first scantable 
                st = context.observing_run[indices[0]]

                # SRCTYPE for ON-SOURCE
                srctype = st.calibration_strategy['srctype']

                # source name
                target_sources = [v for v in st.source.values() \
                                  if 'TARGET' in v.intents]
                source_name = target_sources[0].name.replace(' ', '_')

                # filenames for gridding
                data_name = lambda x: x.baselined_name \
                            if os.path.exists(x.baselined_name) else x.name
                filenames = [data_name(context.observing_run[i]) for i in indices]
                infiles = [context.observing_run[i].basename for i in indices]

                LOG.debug('filenames=%s' % (filenames))
                
                # image name
                namer = filenamer.Image()
                namer.casa_image()
                namer.source(source_name)
                namer.antenna_name(name)
                namer.spectral_window(spwids[0])
                namer.polarization(common.polstring(net_pols))
                imagename = namer.get_filename()
                
                # Step 4.
                # Set weights
                LOG.info('Step 4. Set weights')
                for i in xrange(len(indices)):
                    index = indices[i]
                    spwid = spwids[i]
                    LOG.debug('Setting weight for Antenna %s Spw %s'%(index,spwid))
                    original_st = filenames[i]
                    exported_ms = exported_mses[index]
                    spwtype = context.observing_run[index].spectral_window[spwid].type
                    weighting_inputs = weighting.WeightMS.Inputs(context, infile=original_st, 
                                                                 outfile=exported_ms, antenna=index,
                                                                 spwid=spwid, spwtype=spwtype, 
                                                                 onsourceid=srctype)
                    weighting_task = weighting.WeightMS(weighting_inputs)
                    weighting_result = self._executor.execute(weighting_task, merge=True)
                    logrecords.extend(weighting_result.logrecords)

                # Step 5.
                # Imaging
                LOG.info('Step 5. Imaging')
                scansels = [scansel_list[i] for i in indices]
                scans = map(common.selection_to_list, scansels)

                # register data for combining
                combined_indices.extend(indices)
                combined_infiles.extend(infiles)
                combined_spws.extend(spwids)
                combined_pols.extend(pols)
                combined_scans.extend(scans)

                imager_inputs = worker.SDImaging2Worker.Inputs(context, infiles=infiles, 
                                                               outfile=imagename, spwids=spwids,
                                                               scans=scans, pols=pols,
                                                               onsourceid=srctype, edge=edge,
                                                               vislist=exported_mses)
                imager_task = worker.SDImaging2Worker(imager_inputs)
                imager_result = self._executor.execute(imager_task, merge=True)
                logrecords.extend(imager_result.logrecords)

                # Additional Step.
                # Make grid_table and put rms and valid spectral number array 
                # to the outcome
                LOG.info('Additional Step. Make grid_table')
                with casatools.ImageReader(imager_result.outcome) as ia:
                    cs = ia.coordsys()
                    dircoords = [i for i in xrange(cs.naxes())
                                 if cs.axiscoordinatetypes()[i] == 'Direction']
                    nx = ia.shape()[dircoords[0]]
                    ny = ia.shape()[dircoords[1]]
                validsps = []
                rmss = []
                observing_pattern = st.pattern[spwids[0]].values()[0]
                grid_task_class = gridding.gridding_factory(observing_pattern)
                grid_tables = []
                grid_input_dict = {}
                for (ant, spw, pol) in _members:
                    for p in pol:
                        if not grid_input_dict.has_key(p):
                            grid_input_dict[p] = [[ant], [spw]]
                        else:
                            grid_input_dict[p][0].append(ant)
                            grid_input_dict[p][1].append(spw)

                for (pol,ant_spw) in grid_input_dict.items():
                    _indices = ant_spw[0]
                    _spwids = ant_spw[1]
                    _pols = [[pol] for i in _indices]
                    gridding_inputs = grid_task_class.Inputs(context, antennaid=_indices, 
                                                             spwid=_spwids, polid=_pols,
                                                             nx=nx, ny=ny)
                    gridding_task = grid_task_class(gridding_inputs)
                    gridding_result = self._executor.execute(gridding_task, merge=True)
                    grid_tables.append(gridding_result.outcome)
                    logrecords.extend(gridding_result.logrecords)
                for i in xrange(len(grid_input_dict)):
                    validsps.append([r[6] for r in grid_tables[i]])
                    rmss.append([r[8] for r in grid_tables[i]])
                
                if imagename is not None:
                    image_item = imagelibrary.ImageItem(imagename=imagename,
                                                        sourcename=source_name,
                                                        spwlist=spwids,
                                                        sourcetype='TARGET')
                    image_item.antenna = name
                    outcome = {}
                    outcome['image'] = image_item
                    outcome['validsp'] = numpy.array(validsps)
                    outcome['rms'] = numpy.array(rmss)
                    outcome['edge'] = edge
                    outcome['reduction_group_id'] = group_id
                    outcome['file_index'] = indices
                    outcome['assoc_spws'] = spwids
                    outcome['assoc_pols'] = pols
                    result = SDImaging2Results(task=self.__class__,
                                              success=True,
                                              outcome=outcome)
                    result.task = self.__class__

                    if self.inputs.context.subtask_counter is 0: 
                        result.stage_number = self.inputs.context.task_counter - 1
                    else:
                        result.stage_number = self.inputs.context.task_counter 
                                                
                    results.append(result)


            # Make combined image
            # reference scantable
            st = context.observing_run[context.observing_run.st_names.index(combined_infiles[0])]
            
            # image name
            net_pols = set()
            for p in map(set, combined_pols):
                net_pols = net_pols | p
            net_pols = list(net_pols)
            namer = filenamer.Image()
            namer.casa_image()
            namer.source(source_name)
            namer.spectral_window(combined_spws[0])
            namer.polarization(common.polstring(net_pols))
            imagename = namer.get_filename()

            # Step 4.
            # Imaging
            LOG.info('Step 4. Imaging')
            imager_inputs = worker.SDImaging2Worker.Inputs(context, infiles=combined_infiles, 
                                                           outfile=imagename, spwids=combined_spws,
                                                           scans=combined_scans, pols=combined_pols,
                                                           onsourceid=srctype, edge=edge,
                                                           vislist=exported_mses)
            imager_task = worker.SDImaging2Worker(imager_inputs)
            imager_result = self._executor.execute(imager_task, merge=True)
            logrecords.extend(imager_result.logrecords)

            # Additional Step.
            # Make grid_table and put rms and valid spectral number array 
            # to the outcome
            LOG.info('Additional Step. Make grid_table')
            with casatools.ImageReader(imager_result.outcome) as ia:
                cs = ia.coordsys()
                dircoords = [i for i in xrange(cs.naxes())
                             if cs.axiscoordinatetypes()[i] == 'Direction']
                nx = ia.shape()[dircoords[0]]
                ny = ia.shape()[dircoords[1]]
            validsps = []
            rmss = []
            observing_pattern = st.pattern[combined_spws[0]].values()[0]
            grid_task_class = gridding.gridding_factory(observing_pattern)
            grid_tables = []
            grid_input_dict = {}
            for (ant, spw, pol) in zip(combined_indices, combined_spws, combined_pols):
                for p in pol:
                    if not grid_input_dict.has_key(p):
                        grid_input_dict[p] = [[ant], [spw]]
                    else:
                        grid_input_dict[p][0].append(ant)
                        grid_input_dict[p][1].append(spw)

            for (pol,ant_spw) in grid_input_dict.items():
                _indices = ant_spw[0]
                _spwids = ant_spw[1]
                _pols = [[pol] for i in _indices]
                gridding_inputs = grid_task_class.Inputs(context, antennaid=_indices,
                                                         spwid=_spwids, polid=_pols,
                                                         nx=nx, ny=ny)
                gridding_task = grid_task_class(gridding_inputs)
                gridding_result = self._executor.execute(gridding_task, merge=True)
                logrecords.extend(gridding_result.logrecords)
                grid_tables.append(gridding_result.outcome)
            for i in xrange(len(grid_input_dict)):
                validsps.append([r[6] for r in grid_tables[i]])
                rmss.append([r[8] for r in grid_tables[i]])
            
            if imagename is not None:
                image_item = imagelibrary.ImageItem(imagename=imagename,
                                                    sourcename=source_name,
                                                    spwlist=combined_spws,
                                                    sourcetype='TARGET')
                image_item.antenna = 'COMBINED'
                outcome = {}
                outcome['image'] = image_item
                outcome['validsp'] = numpy.array(validsps)
                outcome['rms'] = numpy.array(rmss)
                outcome['edge'] = edge
                outcome['reduction_group_id'] = group_id
                outcome['file_index'] = combined_indices
                outcome['assoc_spws'] = combined_spws
                
                # to register exported_ms to each scantable instance
                outcome['export_results'] = export_results
                outcome['reduction_group_id'] = group_id
                result = SDImaging2Results(task=self.__class__,
                                          success=True,
                                          outcome=outcome)
                result.task = self.__class__

                if self.inputs.context.subtask_counter is 0: 
                    result.stage_number = self.inputs.context.task_counter - 1
                else:
                    result.stage_number = self.inputs.context.task_counter 
                    
                results.append(result)
                    
        LOG.todo('logrecords for SDImaging2Results must be handled properly')
        # only add logrecords to first result
        if len(results) > 0:
            results[0].logrecords = logrecords
            for r in results[1:]:   
                r.logrecords = []

        return results
    
    def analyse(self, result):
        return result
    
