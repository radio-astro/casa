from __future__ import absolute_import

import os
import math
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.basetask as basetask
from .gridding import gridding_factory
from . import exportms
from . import applyflag
from . import weighting
from .. import common
from ..baseline import baseline
from ..common import temporary_filename

LOG = infrastructure.get_logger(__name__)

NoData = common.NoData

class SDImaging2Inputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
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
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        infiles = self.inputs.infiles
        iflist = self.inputs.iflist
        antennalist = self.inputs.antennalist
        pollist = self.inputs.pollist
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]

        # Step 1.
        # Apply flags to working scantable (antenna_files):
        #     - Apply SummaryFlag in Baseline Table to FLAGROW and FLAGTRA
        #     - Flag WVR and reference data
        LOG.info('Step 1. Apply flags')
        applyflag_inputs = applyflag.SDApplyFlag.Inputs(context, infiles)
        applyflag_task = applyflag.SDApplyFlag(applyflag_inputs)
        applyflag_results = applyflag_task.execute(dry_run=self._executor._dry_run)

        # Step 2.
        # Export each scantable to MS
        LOG.info('Step 2. Export data')
        export_inputs = exportms.ExportMS.Inputs(context, infiles)
        export_task = exportms.ExportMS(export_inputs)
        export_results = export_task.execute(dry_run=self._executor._dry_run)
        exported_mses = export_results.outcome

        # search results and retrieve edge parameter from the most
        # recent SDBaselineResults if it exists
        results = [r.read() for r in context.results]
        baseline_stage = -1
        for stage in xrange(len(results) - 1, -1, -1):
            if isinstance(results[stage], baseline.SDBaselineResults):
                baseline_stage = stage
        if baseline_stage > 0:
            edge = list(results[baseline_stage].outcome['edge'])
            LOG.info('Retrieved edge information from SDBaselineResults: %s' % (edge))
        else:
            LOG.info('No SDBaselineResults available. Set edge as [0,0]')
            edge = [0, 0]
        
        # task returns ResultsList
        results = basetask.ResultsList()

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

            # loop over antennas
            for (name, indices) in antenna_group.items():

                # skip antenna not included in antennalist
                if antennalist is not None and name not in antennalist:
                    LOG.debug('Skip antenna %s' % (name))
                    continue

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

                LOG.debug('filenames=%s' % (filenames))
                
                # image name
                namer = filenamer.Image()
                namer.casa_image()
                namer.source(source_name)
                namer.antenna_name(name)
                namer.spectral_window(spwid)
                if pols == [0, 1]:
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

                # Step 3.
                # Set weights
                LOG.info('Step 3. Set weights')
                for i in xrange(len(indices)):
                    index = indices[i]
                    original_st = filenames[i]
                    exported_ms = exported_mses[index]
                    spwtype = context.observing_run[index].spectral_window[spwid].type
                    #self.__set_weight(original_st, exported_ms, index,
                    #                  spwid, srctype,
                    #                  is_full_resolution)
                    weighting_inputs = weighting.WeightMS.Inputs(context, infile=original_st, 
                                                                 outfile=exported_ms, antenna=index,
                                                                 spwid=spwid, spwtype=spwtype, 
                                                                 onsourceid=srctype)
                    weighting_task = weighting.WeightMS(weighting_inputs)
                    weighting_result = weighting_task.execute(dry_run=self._executor._dry_run)

                # Step 4.
                # Imaging
                LOG.info('Step 4. Imaging')
                self._do_imaging(imagename, datatable, st, source_name,
                                 name, indices, filenames, spwid, pols,
                                 srctype, edge, validsps, rmss, exported_mses)

                # Additional Step.
                # Make grid_table and put rms and valid spectral number array 
                # to the outcome
                LOG.info('Additional Step. Make grid_table')
                validsps = []
                rmss = []
                grid_tables = self._make_grid_table(datatable, st, indices, 
                                                   filenames, spwid, pols)
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

        LOG.todo('logrecords for SDImaging2Results must be handled properly')
        for r in results:
            r.logrecords = []

        return results
    
    def _make_grid_table(self, datatable, reference_data, antenna_indices, 
                         antenna_files, spwid, polids):
        # spectral window
        spw = reference_data.spectral_window[spwid]
        nchan = spw.nchan
        
        # beam size
        grid_size = casatools.quanta.convert(reference_data.beam_size[spwid], 'deg')['value']

        # gridding engine
        observing_pattern = reference_data.pattern[spwid].values()[0]
        gridding_class = gridding_factory(observing_pattern)

        # assume all members have same calmode
        srctype = reference_data.calibration_strategy['srctype']
        
        grid_tables = []
        for pol in polids:                        
            gridder = gridding_class(datatable, antenna_indices, antenna_files, 
                                     spwid, pol, srctype, nchan, grid_size)

            grid_tables.append(self._executor.execute(gridder, merge=False))
            
        return grid_tables

    def _do_imaging(self, imagename, datatable, reference_data, source_name, antenna_name, antenna_indices, antenna_files, spwid, polids, srctype, edge, num_validsp_array, rms_array, exported_mses):

        # input MSs
        infiles = [exported_mses[i] for i in antenna_indices]

        # spw
        spwlist = [spwid for i in antenna_indices]

        do_imaging(self.inputs.context, infiles, spwlist, imagename, datatable, reference_data, source_name, antenna_indices, srctype, edge)

    def analyse(self, result):
        return result


def do_imaging(context, infiles, spwlist, imagename, datatable, reference_data, source_name, antennalist, srctype, edge):
    # imager tool
    im = casatools.imager

    # qa tool
    qa = casatools.quanta

    spwid = spwlist[0]

    # field
    field = '%s*' % (source_name)

    # baseline
    baseline = '0&&&'

    # cellx and celly
    grid_size = reference_data.beam_size[spwid]
    cellx = qa.div(grid_size, 3.0)
    celly = cellx
    cell_in_deg = qa.convert(cellx, 'deg')['value']
    print 'cell=%s' % (cellx)

    # nx and ny
    tb = datatable.tb1
    datatable_name = datatable.plaintable
    index_list = []
    for (_spw, _antenna) in zip(spwlist, antennalist):
        taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID FROM "%s" WHERE IF==%s && SRCTYPE == %s && ANTENNA == %s' % (os.path.join(datatable_name, 'RO'), _spw, srctype, _antenna)
        LOG.debug('taqlstring=\'%s\'' % (taqlstring))
        tx = tb.taql(taqlstring)
        index_list.append(tx.getcol('ID'))
        tx.close()
        del tx
    index_list = numpy.sort(numpy.concatenate(index_list))

    ra = datatable.tb1.getcol('RA').take(index_list)
    dec = datatable.tb1.getcol('DEC').take(index_list)

    ra_min = min(ra)
    ra_max = max(ra)
    dec_min = min(dec)
    dec_max = max(dec)
    ra_extent = ra_max - ra_min
    dec_extent = dec_max - dec_min
    dec_correction = 1.0 / math.cos(0.5 * (dec_min + dec_max) / 180.0 * 3.1415926535897931)

    nx = int((ra_max - ra_min) / (cell_in_deg * dec_correction)) + 1
    ny = int((dec_max - dec_min) / cell_in_deg) + 1

    # increase nx and ny to reduce the effect of edges
    margin = 3
    nx += 2 * margin
    ny += 2 * margin
    LOG.info('nx,ny=%s,%s' % (nx, ny))

    # phasecenter
    ra_center_in_deg = (ra_min + ra_max) * 0.5
    dec_center_in_deg = (dec_min + dec_max) * 0.5
    ra_center = qa.quantity(ra_center_in_deg, 'deg')
    dec_center = qa.quantity(dec_center_in_deg, 'deg')
    phasecenter = 'J2000 %s %s' % (qa.formxxx(ra_center, 'hms'),
                                 qa.formxxx(dec_center, 'dms'))
    LOG.info('phasecenter=\'%s\'' % (phasecenter))

    # mode
    mode = 'channel'

    # stokes
    # stokes = 'I'
    stokes = 'XXYY'

    # start, nchan, step
    total_nchan = reference_data.spectral_window[spwid].nchan
    if total_nchan == 1:
        start = 0
        step = 1
        nchan = 1
    else:
        start = edge[0]
        step = 1
        nchan = total_nchan - sum(edge)

    # restfreq
    spw = reference_data.spectral_window[spwid]
    rest_freqs = spw.rest_frequencies
    if len(rest_freqs) > 0:
        restfreq = rest_freqs[0]
    else:
        restfreq = spw.refval

    # outframe
    outframe = 'LSRK'

    # gridfunction
    gridfunction = 'GAUSS'

    # truncate, gwidth, jwidth, and convsupport
    truncate = '3pixel'
    gwidth = '1.5pixel'
    jwidth = '-1pixel'  # default (not used)
    convsupport = 3

    temporary_name = imagename.rstrip('/')+'.tmp'
    with temporary_filename(temporary_name) as name:
        # imaging
        for (vis, spw) in zip(infiles, spwlist):
            im.selectvis(vis=vis, spw=spw)
        im.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly, stokes=stokes,
                       phasecenter=phasecenter, mode=mode, start=start,
                       nchan=nchan, step=step, restfreq=restfreq,
                       outframe=outframe, spw=spwlist[0])
        im.setoptions(ftmachine='sd', gridfunction=gridfunction)
        im.setsdoptions(convsupport=convsupport, truncate=truncate,
                        gwidth=gwidth, jwidth=jwidth)
        im.makeimage(image=name)
        im.done()
        
        # post imaging process
        # cut margin area
        if os.path.exists(imagename):
            os.system('rm -rf %s'%(imagename))
        box = '%s,%s,%s,%s'%(margin, margin, nx - margin - 1, ny - margin - 1)
        args = {'imagename': name,
                  'mode': 'evalexpr',
                  'expr': 'IM0',
                  'outfile': imagename,
                  'box': box}
        LOG.debug('Executing immath task: args=%s'%(args))
        job = casa_tasks.immath(**args)
        executor = basetask.Executor(context, dry_run=False)
        executor.execute(job, merge=False)
        
    # post imaging process
    # set brightness unit to K
    with casatools.ImageReader(imagename) as ia:
        ia.setbrightnessunit('K')
         
        
        
