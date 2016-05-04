from __future__ import absolute_import

import os
import numpy
import math
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks
from .. import common
from ..common import temporary_filename
from ..common import utils

LOG = infrastructure.get_logger(__name__)

def ALMAImageCoordinateUtil(context, datatable, infiles, spw_list, pols_list, srctype, vislist):
    """
    An utility function to calculate spatial coordinate of image for ALMA
    """
    antenna_list = [context.observing_run.st_names.index(f) 
                    for f in infiles]
    vislist = [vislist[i] for i in antenna_list]
    reference_data = context.observing_run[antenna_list[0]]
    ref_spw = spw_list[0]
    target_sources = [v for v in reference_data.source.values() 
                      if 'TARGET' in v.intents]
    source_name = target_sources[0].name

    # qa tool
    qa = casatools.quanta
    ### ALMA specific part ###
    # the number of pixels per beam
    grid_factor = 9.0
    # recommendation by EOC
    fwhmfactor = 1.13
    # hard-coded for ALMA-TP array
    diameter_m = 12.0 #max([ context.observing_run[antid].antenna.diameter for antid in antenna_list ])
    obscure_alma = 0.75
    ### END OF ALMA part ###
    with casatools.MSMDReader(vislist[0]) as msmd:
        freq_hz = msmd.meanfreq(ref_spw)
        fnames = [name for name in msmd.fieldnames() if name.find(source_name) > -1 ]
        me_center = msmd.phasecenter(msmd.fieldsforname(fnames[0])[0])

    # cellx and celly
    import sdbeamutil
    theory_beam_arcsec = sdbeamutil.primaryBeamArcsec(freq_hz, diameter_m, obscure_alma, 10.0, fwhmfactor=fwhmfactor)
    grid_size = qa.quantity(theory_beam_arcsec, 'arcsec')
    cellx = qa.div(grid_size, grid_factor)
    celly = cellx
    cell_in_deg = qa.convert(cellx, 'deg')['value']
    LOG.info('Calculating image coordinate of field \'%s\', reference frequency %fGHz' % (fnames[0], freq_hz*1.e-9))
    LOG.info('cell=%s' % (qa.tos(cellx)))
    
    # phasecenter = field direction
    ra_center = qa.convert(me_center['m0'], 'deg')
    dec_center = qa.convert(me_center['m1'], 'deg')
    ra_center_in_deg = qa.getvalue(ra_center)
    dec_center_in_deg = qa.getvalue(dec_center)
    phasecenter = 'J2000 %s %s' % (qa.formxxx(ra_center, 'hms'),
                                     qa.formxxx(dec_center, 'dms'))
    LOG.info('phasecenter=\'%s\'' % (phasecenter, ))

    # nx and ny
    index_list = common.get_index_list(datatable, antenna_list, spw_list, pols_list, srctype)
    
    if len(index_list) == 0:
        antenna_name = reference_data.antenna.name
        LOG.warn('No valid data for source %s antenna %s spw %s. Image will not be created.'%(source_name, antenna_name, ref_spw))
        return False
        
    index_list.sort()
    
    ra = datatable.tb1.getcol('RA').take(index_list)
    dec = datatable.tb1.getcol('DEC').take(index_list)
    
    ra_min = min(ra)
    ra_max = max(ra)
    dec_min = min(dec)
    dec_max = max(dec)
    dec_correction = 1.0 / math.cos(dec_center_in_deg / 180.0 * 3.1415926535897931)
    width = 2*max(abs(ra_center_in_deg-ra_min), abs(ra_max-ra_center_in_deg))
    height = 2*max(abs(dec_center_in_deg-dec_min), abs(dec_max-dec_center_in_deg))
    LOG.debug('Map extent: [%f, %f] arcmin' % (width/60., height/60.))
    
    nx = int(width / (cell_in_deg * dec_correction)) + 1
    ny = int(height / cell_in_deg) + 1
    
    # Adjust nx and ny to be even number for performance (which is 
    # recommended by imager). 
    # Also increase nx and ny  by 2 if they are even number. 
    # This is due to a behavior of the imager. The imager configures 
    # direction axis as follows:
    #     reference value: phasecenter
    #           increment: cellx, celly
    #     reference pixel: ceil((nx-1)/2), ceil((ny-1)/2)
    # It means that reference pixel will not be map center if nx/ny 
    # is even number. It results in asymmetric area coverage on both 
    # sides of the reference pixel, which may miss certain range of 
    # (order of 1 pixel) observed area near the edge. 
    if nx % 2 == 0:
        nx += 2
    else:
        nx += 1
    if ny % 2 == 0:
        ny += 2
    else:
        ny += 1
    
    LOG.info('nx,ny=%s,%s' % (nx, ny))
    return phasecenter, cellx, celly, nx, ny    

class SDImagingWorkerInputs(common.SingleDishInputs):
    """
    Inputs for imaging worker
    """
    def __init__(self, context, infiles, outfile, mode, spwids, scans, pols, onsourceid, 
                 edge=None, vislist=None, phasecenter=None, cellx=None, celly=None, nx=None, ny=None):
        # NOTE: spwids and pols are list of numeric id list while scans
        #       is string (mssel) list
        self._init_properties(vars())

class SDImagingWorkerResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDImagingWorkerResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDImagingWorkerResults, self).merge_with_context(context)

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return self.outcome


class SDImagingWorker(common.SingleDishTaskTemplate):
    Inputs = SDImagingWorkerInputs
    
    @common.datatable_setter
    def prepare(self):
        infiles = self.inputs.infiles
        outfile = self.inputs.outfile
        edge = self.inputs.edge
        spwid_list = self.inputs.spwids
        scan_list = self.inputs.scans
        pols_list = self.inputs.pols
        imagemode = self.inputs.mode
        phasecenter, cellx, celly, nx, ny = self._get_map_coord(self.inputs, self.context, self.datatable, infiles, spwid_list, pols_list)
        
        status = self._do_imaging(infiles, spwid_list, scan_list, outfile, imagemode, edge, phasecenter, cellx, celly, nx, ny)
 
        if status is True:
            result = SDImagingWorkerResults(task=self.__class__,
                                            success=True,
                                            outcome=outfile)
        else:
            # Imaging failed due to missing valid data
            result = SDImagingWorkerResults(task=self.__class__,
                                            success=False,
                                            outcome=None)

        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 

        return result
    
    def analyse(self, result):
        return result

    def _get_map_coord(self, inputs, context, datatable, infiles, spw_list, pols_list):
        params = (inputs.phasecenter, inputs.cellx, inputs.celly, inputs.nx, inputs.ny)
        coord_set = (params.count(None) == 0)
        if coord_set:
            return params
        else:
            return ALMAImageCoordinateUtil(context, datatable, infiles, spw_list, pols_list, inputs.onsourceid, inputs.vislist)

    def _do_imaging(self, infiles, spwid_list, scan_list, imagename, imagemode, edge, phasecenter, cellx, celly, nx, ny):
        context = self.context
        antenna_list = [context.observing_run.st_names.index(f) 
                        for f in infiles]
        vislist = [self.inputs.vislist[i] for i in antenna_list]
        reference_data = context.observing_run[antenna_list[0]]
        spwid = spwid_list[0]
        
        LOG.debug('Members to be processed:')
        for (a,s) in zip(antenna_list, spwid_list):
            LOG.debug('\tAntenna %s Spw %s'%(a,s))
    
        # field
        target_sources = [v for v in reference_data.source.values() 
                          if 'TARGET' in v.intents]
        source_name = target_sources[0].name
        field = '"%s*"' % (source_name)
        # Check for ephemeris source
        known_ephemeris_list = ['MERCURY', 'VENUS', 'MARS', 'JUPITER', 'SATURN', 'URANUS', 'NEPTUNE', 'PLUTO', 'SUN', 'MOON']
        ephemsrcname = ''
        if source_name.upper() in known_ephemeris_list:
            ephemsrcname = source_name.upper()
            LOG.info("Generating an image of ephemeris source. Setting ephemsrcname='%s'" % ephemsrcname)
    
        # baseline
        #baseline = '0&&&'
    
        # mode
        mode = 'channel'
    
        # stokes
        stokes = 'I'
    
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
        # ampcal
        if imagemode == 'AMPCAL':
            step = nchan
            nchan = 1
    
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
        gridfunction = 'SF'
    
        # truncate, gwidth, jwidth, and convsupport
        truncate = gwidth = jwidth = -1  # defaults (not used)
        convsupport = 6
    
#         temporary_name = imagename.rstrip('/')+'.tmp'
        cleanup_params = ['outfile', 'infiles', 'spw', 'scan']
        qa = casatools.quanta
        image_args = {'field': field, 
                      'mode': mode,
                      'nchan': nchan,
                      'start': start,
                      'width': step,
                      'outframe': outframe,
                      'gridfunction': gridfunction,
                      'convsupport': convsupport,
                      'truncate': truncate,
                      'gwidth': gwidth,
                      'jwidth': jwidth,
                      'imsize': [nx, ny],
                      'cell': [qa.tos(cellx), qa.tos(celly)],
                      'phasecenter': phasecenter,
                      'restfreq': restfreq,
                      'stokes': stokes,
                      'ephemsrcname': ephemsrcname}

        #remove existing image explicitly
        for rmname in [imagename, imagename.rstrip('/')+'.weight']:
            if os.path.exists(rmname):
                shutil.rmtree(rmname)

        # imaging
        infile_list = []
        spwsel_list = []
        scansel_list = []
        spwsel = []
        for (vis, spw, scan) in zip(vislist, spwid_list, scan_list):
            LOG.debug('Registering data to image: vis=\'%s\', spw=%s, field=%s%s'%(vis, spw, field,
                                                                                       (' (ephemeris source)' if ephemsrcname!='' else '')))
            infile_list.append(vis)
            scansel_list.append(common.list_to_selection(scan))
            # WORKAROUND for a bug in sdimaging
            #spwsel_list.append(common.list_to_selection(utils.to_list(spw)))
            if not (spw in spwsel):
                spwsel.append(spw)
        spwsel_list = common.list_to_selection(spwsel)
        # set-up image dependent parameters
        for p in cleanup_params: image_args[p] = None
        image_args['outfile'] = imagename
        image_args['infiles'] = infile_list
        image_args['spw'] = spwsel_list
        image_args['scan'] = scansel_list
        LOG.debug('Executing sdimaging task: args=%s'%(image_args))
        image_job = casa_tasks.sdimaging(**image_args)

        # execute job
        self._executor.execute(image_job)
        
        return True
