from __future__ import absolute_import

import os
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.imagelibrary as imagelibrary
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
    def __init__(self, context, reffile=None, infiles=None, iflist=None, pollist=None):
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

class SDImaging2Results(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDImaging2Results, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDImaging2Results, self).merge_with_context(context)
        LOG.todo('need to decide what is done in SDImaging2Results.merge_with_context')

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return self.outcome['image'].imagename

class SDImaging2(common.SingleDishTaskTemplate):
    Inputs = SDImaging2Inputs

    def prepare(self):
        context = self.inputs.context
        reduction_group = context.observing_run.reduction_group
        infiles = self.inputs.infiles
        iflist = self.inputs.iflist
        antennalist = self.inputs.antennalist
        pollist = self.inputs.pollist
        st_names = context.observing_run.st_names
        reffile = self.inputs.reffile
        
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

        # Step 2.
        # Export each scantable to MS
        LOG.info('Step 2. Export data')
        export_inputs = exportms.ExportMS.Inputs(context, infiles)
        export_task = exportms.ExportMS(export_inputs)
        export_results = self._executor.execute(export_task, merge=True)
        exported_mses = export_results.outcome
        
        # to register exported_ms to each scantable instance
        results.append(export_results)
        
        # Step 3.
        # Intensity scaling
        LOG.info('Step 3. Intensity scaling')
        scaling_inputs = scaling.IntensityScaling.Inputs(context, 
                                                         infiles=infiles, 
                                                         reffile=reffile)
        scaling_task = scaling.IntensityScaling(scaling_inputs)
        scaling_results = self._executor.execute(scaling_task, merge=True)

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
            # assume all members have same spw and pollist
            first_member = group_desc[0]
            spwid = first_member.spw
            LOG.debug('spwid=%s' % (spwid))
            pols = first_member.pols
            if pollist is not None:
                pols = list(set(pollist).intersection(pols))

            # skip spw not included in iflist
            if iflist is not None and spwid not in iflist:
                LOG.debug('Skip spw %s' % (spwid))
                continue

            # image is created per antenna
            antenna_group = {}
            for m in group_desc:
                antenna = context.observing_run[m.antenna].antenna.name
                if antenna in antenna_group.keys():
                    antenna_group[antenna].append(m.antenna)
                else:
                    antenna_group[antenna] = [m.antenna]
            LOG.info('antenna_group=%s' % (antenna_group))

            # polarization string
            if pols == [0, 1]:
                polstr = 'XXYY'
            elif pols == [0] or pols == 0:
                polstr = 'XX'
            elif pols == [1] or pols == 1:
                polstr = 'YY'
            else:
                polstr = 'I'

            # loop over antennas
            combined_indices = []
            source_name = None
            combined_infiles = []
            srctype = None
            for (name, indices) in antenna_group.items():

                # skip antenna not included in antennalist
                if antennalist is not None and name not in antennalist:
                    LOG.debug('Skip antenna %s' % (name))
                    continue

                # register indices to combined_indices
                combined_indices.extend(indices)

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
                combined_infiles.extend(infiles)

                LOG.debug('filenames=%s' % (filenames))
                
                # image name
                namer = filenamer.Image()
                namer.casa_image()
                namer.source(source_name)
                namer.antenna_name(name)
                namer.spectral_window(spwid)
                namer.polarization(polstr)
                imagename = namer.get_filename()
                
                # Step 4.
                # Set weights
                LOG.info('Step 4. Set weights')
                for i in xrange(len(indices)):
                    index = indices[i]
                    original_st = filenames[i]
                    exported_ms = exported_mses[index]
                    spwtype = context.observing_run[index].spectral_window[spwid].type
                    weighting_inputs = weighting.WeightMS.Inputs(context, infile=original_st, 
                                                                 outfile=exported_ms, antenna=index,
                                                                 spwid=spwid, spwtype=spwtype, 
                                                                 onsourceid=srctype)
                    weighting_task = weighting.WeightMS(weighting_inputs)
                    weighting_result = self._executor.execute(weighting_task, merge=True)

                # Step 5.
                # Imaging
                LOG.info('Step 5. Imaging')
                imager_inputs = worker.SDImaging2Worker.Inputs(context, infiles=infiles, 
                                                               outfile=imagename, spwid=spwid, 
                                                               onsourceid=srctype, edge=edge,
                                                               vislist=exported_mses)
                imager_task = worker.SDImaging2Worker(imager_inputs)
                imager_result = self._executor.execute(imager_task, merge=True)

                # Additional Step.
                # Make grid_table and put rms and valid spectral number array 
                # to the outcome
                LOG.info('Additional Step. Make grid_table')
                validsps = []
                rmss = []
                observing_pattern = st.pattern[spwid].values()[0]
                grid_task_class = gridding.gridding_factory(observing_pattern)
                grid_tables = []
                for pol in pols:
                    gridding_inputs = grid_task_class.Inputs(context, antennaid=indices, 
                                                             spwid=spwid, polid=pol)
                    gridding_task = grid_task_class(gridding_inputs)
                    gridding_result = self._executor.execute(gridding_task, merge=True)
                    grid_tables.append(gridding_result.outcome)
                for i in xrange(len(pols)):
                    validsps.append([r[6] for r in grid_tables[i]])
                    rmss.append([r[8] for r in grid_tables[i]])
                
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
            namer = filenamer.Image()
            namer.casa_image()
            namer.source(source_name)
            namer.spectral_window(spwid)
            namer.polarization(polstr)
            imagename = namer.get_filename()

            # Step 4.
            # Imaging
            LOG.info('Step 4. Imaging')
            imager_inputs = worker.SDImaging2Worker.Inputs(context, infiles=combined_infiles, 
                                                           outfile=imagename, spwid=spwid, 
                                                           onsourceid=srctype, edge=edge,
                                                           vislist=exported_mses)
            imager_task = worker.SDImaging2Worker(imager_inputs)
            imager_result = self._executor.execute(imager_task, merge=True)

            # Additional Step.
            # Make grid_table and put rms and valid spectral number array 
            # to the outcome
            LOG.info('Additional Step. Make grid_table')
            validsps = []
            rmss = []
            observing_pattern = st.pattern[spwid].values()[0]
            grid_task_class = gridding.gridding_factory(observing_pattern)
            grid_tables = []
            for pol in pols:
                gridding_inputs = grid_task_class.Inputs(context, antennaid=combined_indices, 
                                                         spwid=spwid, polid=pol)
                gridding_task = grid_task_class(gridding_inputs)
                gridding_result = self._executor.execute(gridding_task, merge=True)
                grid_tables.append(gridding_result.outcome)
            for i in xrange(len(pols)):
                validsps.append([r[6] for r in grid_tables[i]])
                rmss.append([r[8] for r in grid_tables[i]])
            
            if imagename is not None:
                image_item = imagelibrary.ImageItem(imagename=imagename,
                                                    sourcename=source_name,
                                                    spwlist=spwid,
                                                    sourcetype='TARGET')
                image_item.antenna = 'COMBINED'
                outcome = {}
                outcome['image'] = image_item
                outcome['validsp'] = numpy.array(validsps)
                outcome['rms'] = numpy.array(rmss)
                outcome['edge'] = edge
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
        for r in results:
            r.logrecords = []

        return results
    
    def analyse(self, result):
        return result
