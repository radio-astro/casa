from __future__ import absolute_import

import os
import numpy
import math
import shutil
import itertools

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks
from pipeline.domain import DataTable
from .. import common
from ..common import utils

LOG = infrastructure.get_logger(__name__)


def ALMAImageCoordinateUtil(context, ms_names, ant_list, spw_list, fieldid_list):
    """
    An utility function to calculate spatial coordinate of image for ALMA
    """
    # A flag to use field direction as image center (True) rather than center of the map extent
    USE_FIELD_DIR = False

    idx = utils.get_parent_ms_idx(context, ms_names[0])
    if idx >= 0 and idx < len(context.observing_run.measurement_sets):
        ref_msobj = context.observing_run.measurement_sets[idx]
    else:
        raise ValueError, "The reference ms, %s, not registered to context" % ms_names[0]
    ref_fieldid = fieldid_list[0]
    ref_spw = spw_list[0]
    source_name = ref_msobj.fields[ref_fieldid].name
#     # trim source name to workaround '"name"' type of sources
#     trimmed_name = source_name.strip('"')

    # qa tool
    qa = casatools.quanta
    ### ALMA specific part ###
    # the number of pixels per beam
    grid_factor = 9.0
    # recommendation by EOC
    fwhmfactor = 1.13
    # hard-coded for ALMA-TP array
    diameter_m = 12.0
    obscure_alma = 0.75
    ### END OF ALMA part ###

    # msmd-less implementation
    spw = ref_msobj.get_spectral_window(ref_spw)
    freq_hz = numpy.float64(spw.mean_frequency.value)
#     fields = ref_msobj.get_fields(name=trimmed_name)
    fields = ref_msobj.get_fields(name=source_name)
    fnames = [f.name for f in fields]
    if USE_FIELD_DIR:
        me_center = fields[0].mdirection

    # cellx and celly
    import sdbeamutil
    theory_beam_arcsec = sdbeamutil.primaryBeamArcsec(freq_hz, diameter_m, obscure_alma, 10.0, fwhmfactor=fwhmfactor)
    grid_size = qa.quantity(theory_beam_arcsec, 'arcsec')
    cellx = qa.div(grid_size, grid_factor)
    celly = cellx
    cell_in_deg = qa.convert(cellx, 'deg')['value']
    LOG.info('Calculating image coordinate of field \'%s\', reference frequency %fGHz' % (fnames[0], freq_hz * 1.e-9))
    LOG.info('cell=%s' % (qa.tos(cellx)))

    datatable = DataTable(name=context.observing_run.ms_datatable_name, readonly=True)
    # nx, ny and center
    parent_mses = [utils.get_parent_ms_name(context, name) for name in ms_names]
    index_list = common.get_index_list_for_ms(datatable, parent_mses, ant_list, fieldid_list, spw_list)

    if len(index_list) == 0:
        antenna_name = ref_msobj.antennas[ant_list[0]].name
        LOG.warn('No valid data for source %s antenna %s spw %s in %s. Image will not be created.' % (
        source_name, antenna_name, ref_spw, ref_msobj.basename))
        return False

    index_list.sort()

    # the unit of RA and DEC should be in deg
    ra = datatable.getcol('RA').take(index_list)
    dec = datatable.getcol('DEC').take(index_list)
    if (datatable.getcolkeyword('RA', 'UNIT') != 'deg') or \
        (datatable.getcolkeyword('DEC', 'UNIT') != 'deg'):
        raise RuntimeError, "Found unexpected unit of RA/DEC in DataTable. It should be in 'deg'"
    outref = datatable.direction_ref
    if outref is None:
        LOG.warn('No direction reference is set. Assuming ICRS')
        outref = 'ICRS'
    del datatable

    ra_min = min(ra)
    ra_max = max(ra)
    dec_min = min(dec)
    dec_max = max(dec)

    if USE_FIELD_DIR:
        # phasecenter = field direction
        ra_center = qa.convert(me_center['m0'], 'deg')
        dec_center = qa.convert(me_center['m1'], 'deg')
    else:
        # map center
        ra_center = qa.quantity(0.5 * (ra_min + ra_max), 'deg')
        dec_center = qa.quantity(0.5 * (dec_min + dec_max), 'deg')
    ra_center_in_deg = qa.getvalue(ra_center)
    dec_center_in_deg = qa.getvalue(dec_center)
    phasecenter = '{0} {1} {2}'.format(outref,
                                       qa.formxxx(ra_center, 'hms'),
                                       qa.formxxx(dec_center, 'dms'))
    LOG.info('phasecenter=\'%s\'' % (phasecenter,))

    dec_correction = 1.0 / math.cos(dec_center_in_deg / 180.0 * 3.1415926535897931)
    width = 2 * max(abs(ra_center_in_deg - ra_min), abs(ra_max - ra_center_in_deg))
    height = 2 * max(abs(dec_center_in_deg - dec_min), abs(dec_max - dec_center_in_deg))
    LOG.debug('Map extent: [%f, %f] arcmin' % (width / 60., height / 60.))

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

    LOG.info('Image pixel size: [nx, ny] = [%s, %s]' % (nx, ny))
    return phasecenter, cellx, celly, nx, ny


class SDImagingWorkerInputs(vdp.StandardInputs):
    """
    Inputs for imaging worker
    NOTE: infile should be a complete list of MSes 
    """
    infiles = vdp.VisDependentProperty(default='', null_input=['', None, [], ['']])
    outfile = vdp.VisDependentProperty(default='')
    mode = vdp.VisDependentProperty(default='LINE')
    antids = vdp.VisDependentProperty(default=-1)
    spwids = vdp.VisDependentProperty(default=-1)
    fieldids = vdp.VisDependentProperty(default=-1)
    stokes = vdp.VisDependentProperty(default='I')
    edge = vdp.VisDependentProperty(default=(0,0))
    phasecenter = vdp.VisDependentProperty(default='')
    cellx = vdp.VisDependentProperty(default='')
    celly = vdp.VisDependentProperty(default='')
    nx = vdp.VisDependentProperty(default=-1)
    ny = vdp.VisDependentProperty(default=-1)

    # Synchronization between infiles and vis is still necessary
    @vdp.VisDependentProperty
    def vis(self):
        return self.infiles
    
    def __init__(self, context, infiles, outfile, mode, antids, spwids, fieldids, stokes, edge=None, phasecenter=None,
                 cellx=None, celly=None, nx=None, ny=None):
        # NOTE: spwids and pols are list of numeric id list while scans
        #       is string (mssel) list
        super(SDImagingWorkerInputs, self).__init__()

        self.context = context
        self.infiles = infiles
        self.outfile = outfile
        self.mode = mode
        self.antids = antids
        self.spwids = spwids
        self.fieldids = fieldids
        self.stokes = stokes
        self.edge = edge
        self.phasecenter = phasecenter
        self.cellx = cellx
        self.celly = celly
        self.nx = nx
        self.ny = ny


class SDImagingWorkerResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDImagingWorkerResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDImagingWorkerResults, self).merge_with_context(context)

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return self.outcome


class SDImagingWorker(basetask.StandardTaskTemplate):
    Inputs = SDImagingWorkerInputs

    is_multi_vis_task = True

    def prepare(self):
        inputs = self.inputs
        context = self.inputs.context
        infiles = inputs.infiles
        outfile = inputs.outfile
        edge = inputs.edge
        antid_list = inputs.antids
        spwid_list = inputs.spwids
        fieldid_list = inputs.fieldids
        imagemode = inputs.mode
        phasecenter, cellx, celly, nx, ny = self._get_map_coord(inputs, context, infiles, antid_list, spwid_list,
                                                                fieldid_list)

        status = self._do_imaging(infiles, antid_list, spwid_list, fieldid_list, outfile, imagemode, edge, phasecenter,
                                  cellx, celly, nx, ny)

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

    def _get_map_coord(self, inputs, context, infiles, ant_list, spw_list, field_list):
        params = (inputs.phasecenter, inputs.cellx, inputs.celly, inputs.nx, inputs.ny)
        coord_set = (params.count(None) == 0)
        if coord_set:
            return params
        else:
            return ALMAImageCoordinateUtil(context, infiles, ant_list, spw_list, field_list)

    def _do_imaging(self, infiles, antid_list, spwid_list, fieldid_list, imagename, imagemode, edge, phasecenter, cellx,
                    celly, nx, ny):
        context = self.inputs.context
        idx = utils.get_parent_ms_idx(context, infiles[0])
        if idx >= 0 and idx < len(context.observing_run.measurement_sets):
            reference_data = context.observing_run.measurement_sets[idx]
        else:
            raise ValueError, "The reference ms, %s, not registered to context" % infiles[0]
        ref_spwid = spwid_list[0]

        LOG.debug('Members to be processed:')
        for (m, a,s, f) in itertools.izip(infiles, antid_list, spwid_list, fieldid_list):
            LOG.debug('\tMS %s: Antenna %s Spw %s Field %s'%(os.path.basename(m), a,s,f))
    
        # Check for ephemeris source
        known_ephemeris_list = ['MERCURY', 'VENUS', 'MARS', 'JUPITER', 'SATURN', 'URANUS', 'NEPTUNE', 'PLUTO', 'SUN',
                                'MOON']
        ephemsrcname = ''
        source_name = reference_data.fields[fieldid_list[0]].name
        if source_name.upper() in known_ephemeris_list:
            ephemsrcname = source_name.upper()
            LOG.info("Generating an image of ephemeris source. Setting ephemsrcname='%s'" % ephemsrcname)

        # baseline
        # baseline = '0&&&'

        # mode
        mode = 'channel'

        # stokes
        stokes = self.inputs.stokes

        # start, nchan, step
        ref_spwobj = reference_data.spectral_windows[ref_spwid]
        total_nchan = ref_spwobj.num_channels
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
        rest_freq = ref_spwobj.ref_frequency
        if rest_freq is not None:
            qa = casatools.quanta
            restfreq = qa.tos(qa.quantity(numpy.double(rest_freq.value), rest_freq.units['symbol']))
        else:
            raise RuntimeError, "Could not get reference frequency of Spw %d" % ref_spwid

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
        image_args = {'mode': mode,
                      'intent': "OBSERVE_TARGET#ON_SOURCE",
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

        # remove existing image explicitly
        for rmname in [imagename, imagename.rstrip('/') + '.weight']:
            if os.path.exists(rmname):
                shutil.rmtree(rmname)

        # imaging
        infile_list = []
        spwsel_list = []
        fieldsel_list = []
        antsel_list = []
        for (msname, ant, spw, field) in itertools.izip(infiles, antid_list, spwid_list, fieldid_list):
            LOG.debug('Registering data to image: vis=\'%s\', ant=%s, spw=%s, field=%s%s'%(msname, ant, spw, field,
                                                                                           (' (ephemeris source)' if ephemsrcname!='' else '')))
            infile_list.append(msname)
            spwsel_list.append(str(spw))
            fieldsel_list.append(str(field))
            antsel_list.append(str(ant))
        # collapse selection if possible
        spwsel_list = spwsel_list[0] if len(set(spwsel_list)) == 1 else spwsel_list
        fieldsel_list = fieldsel_list[0] if len(set(fieldsel_list)) == 1 else fieldsel_list
        antsel_list = antsel_list[0] if len(set(antsel_list)) == 1 else antsel_list
        # set-up image dependent parameters
        for p in cleanup_params: image_args[p] = None
        image_args['outfile'] = imagename
        image_args['infiles'] = infile_list
        image_args['spw'] = spwsel_list
        image_args['field'] = fieldsel_list
        image_args['antenna'] = antsel_list
        LOG.debug('Executing sdimaging task: args=%s' % (image_args))
        image_job = casa_tasks.sdimaging(**image_args)

        # execute job
        self._executor.execute(image_job)
        # check imaging result
        imagename = image_args['outfile']
        weightname = imagename + '.weight'
        if not os.path.exists(imagename) or not os.path.exists(weightname):
            LOG.error("Generation of %s failed" % imagename)
            return False
        # check for valid pixels (non-zero weight)
        # Task sdimaging does not fail even if no data is gridded to image.
        # In that case, image is not masked, no restoring beam is set to 
        # image, and all pixels in corresponding weight image is zero. 
        with casatools.ImageReader(weightname) as ia:
            sumsq = ia.statistics()['sumsq'][0]
        if sumsq == 0.0:
            LOG.warning("No valid pixel found in image, %s. Discarding the image from futher processing." % imagename)
            return False

        return True
