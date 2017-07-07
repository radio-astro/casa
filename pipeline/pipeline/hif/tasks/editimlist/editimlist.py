"""A pipeline task to add to a list of images to be made by hif_makeimages()

The hif_editimlist() task typically uses a parameter file as input.  Depending
on the use case, there will usually be a minimal set of input parameters
defined in this file.  Each set of image parameters gets stored in the global
context in the clean_list_pending attribute.

Example:
    A common case is providing a list of VLASS image parameters via a file::

        CASA <1>: hif_editimlist(parameter_file='vlass_QLIP_parameters.list')

    The ``vlass_QLIP_parameters.list`` file might contain something like the
    following::

        phasecenter='J2000 12:16:04.600 +059.24.50.300'
        imagename='QLIP_image'

    An equivalent way to invoke the above example would be::

        CASA <2>: hif_editimlist(phasecenter='J2000 12:16:04.600 +059.24.50.300',
                                 imagename='QLIP_image')

Any imaging parameters that are not specified when hif_editimlist() is called,
either as a task parameter or via a parameter file, will have a default value
or heuristic applied.

Todo:
    * In the future this task will be modified to allow editing the parameters
    of an existing context.clean_list_pending entry.

"""

from __future__ import absolute_import
from __future__ import print_function  # get python 3 print function
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from .resultobjects import EditimlistResult
from pipeline.hif.tasks.makeimlist.cleantarget import CleanTarget
from pipeline.hif.heuristics import imageparams_factory

LOG = infrastructure.get_logger(__name__)


class EditimlistInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
                 search_radius_arcsec=None,
                 cell=None,
                 cyclefactor=None,
                 cycleniter=None,
                 deconvolver=None,
                 editmode=None,
                 field=None,
                 imaging_mode=None,
                 imagename=None,
                 imsize=None,
                 intent=None,
                 gridder=None,
                 mask=None,
                 nbin=None,
                 nchan=None,
                 niter=None,
                 nterms=None,
                 parameter_file=None,
                 phasecenter=None,
                 reffreq=None,
                 robust=None,
                 scales=None,
                 specmode=None,
                 spw=None,
                 start=None,
                 stokes=None,
                 threshold=None,
                 threshold_nsigma=None,
                 uvtaper=None,
                 uvrange=None,
                 width=None,
                 ):

        self._init_properties(vars())

        keys_to_consider = ('field', 'intent', 'spw', 'cell', 'deconvolver', 'imsize',
                            'phasecenter', 'specmode', 'gridder', 'imagename', 'scales',
                            'start', 'width', 'nbin', 'nchan', 'uvrange', 'stokes', 'nterms',
                            'robust', 'uvtaper', 'niter', 'cyclefactor', 'cycleniter', 'mask',
                            'search_radius_arcsec', 'threshold', 'imaging_mode', 'reffreq',
                            'editmode', 'threshold_nsigma')
        self.keys_to_change = []
        for key in keys_to_consider:
            # print key, eval(key)
            if self.__dict__[key] is not None:
                self.keys_to_change.append(key)

# tell the infrastructure to give us mstransformed data when possible by
# registering our preference for imaging measurement sets
basetask.ImagingMeasurementSetsPreferred.register(EditimlistInputs)


class Editimlist(basetask.StandardTaskTemplate):
    # 'Inputs' will be used later in execute_task().
    #   See h/cli/utils.py and infrastructure/argmagger.py
    Inputs = EditimlistInputs

    # TODO:  check to see if I should set this to False
    def is_multi_vis_task(self):
        return True

    def prepare(self):

        # this python class will produce a list of images to be calculated.
        inp = self.inputs
        inpdict = inp.__dict__

        # if a file is given, read whatever parameters are defined in the file
        if inp.parameter_file:
            if os.access(inp.parameter_file, os.R_OK):
                with open(inp.parameter_file) as parfile:
                    for line in parfile:
                        # ignore comment lines or lines that don't contain '='
                        if line.startswith('#') or '=' not in line:
                            continue
                        # split key=value into a key, value components
                        parameter, value = line.partition('=')[::2]
                        # strip whitespace
                        parameter = parameter.strip()
                        value = value.strip()
                        value = value.strip('\'')
                        value = value.strip('"')
                        # use this information to change the values in inputs
                        print("Setting inputdict['{k}'] to {v}".format(k=parameter, v=value))
                        inpdict[parameter] = value
                        inp.keys_to_change.append(parameter)
            else:
                LOG.error('Input parameter file is not readable: {fname}'.format(fname=inp.parameter_file))

        # now construct the list of imaging command parameter lists that must
        # be run to obtain the required images
        result = EditimlistResult()

        # will default to adding a new image list entry
        inpdict['editmode'] = 'add' if not inpdict['editmode'] else inpdict['editmode']

        # TODO think about how to handle multiple MSs
        # we use the ms to change field ids to fieldnames, if needed
        ms = inp.context.observing_run.get_ms(inp.vis[0])
        fieldnames = []
        if not isinstance(inpdict['field'], type(None)):
            for fid in inpdict['field']:
                if isinstance(fid, int):
                    fieldobj = ms.get_fields(field_id=fid)
                    fieldnames.append(fieldobj[0].name)
                else:
                    fieldnames.append(fid)

            if len(fieldnames) > 1:
                fieldnames = [','.join(fieldnames)]
        # fieldnames is now a list of fieldnames: ['fieldA', 'fieldB', ...]

        target = CleanTarget()  # initialize a target structure for clean_list_pending

        img_mode = 'VLASS-QL' if not inpdict['imaging_mode'] else inpdict['imaging_mode']
        result.img_mode = img_mode

        # The default spw range for VLASS is 2~17. hif_makeimages() needs an csv list.
        # We set the target spw before the heuristics object because the heursitics class
        # uses it in initialization.
        if 'VLASS-QL' == img_mode:
            if not inpdict['spw']:
                target['spw'] = ','.join([str(x) for x in range(2, 18)])
                inpdict['spw'] = target['spw']
            else:
                target['spw'] = inpdict['spw']
        else:
            target['spw'] = inpdict['spw']

        target['phasecenter'] = inpdict['phasecenter']

        iph = imageparams_factory.ImageParamsHeuristicsFactory()
        th = target['heuristics'] = iph.getHeuristics(vislist=inp.vis, spw=target['spw'],
                                                      observing_run=inp.context.observing_run,
                                                      imaging_mode=img_mode)

        target['threshold'] = th.threshold() if not inpdict['threshold'] else inpdict['threshold']
        target['nsigma'] = th.threshold_nsigma() if not inpdict['threshold_nsigma'] else inpdict['threshold_nsigma']
        target['reffreq'] = th.reffreq() if not inpdict['reffreq'] else inpdict['reffreq']
        target['niter'] = th.niter_correction(None, None, None, None, None) if not inpdict['niter'] else inpdict['niter']
        target['cyclefactor'] = th.cyclefactor() if not inpdict['cyclefactor'] else inpdict['cyclefactor']
        target['cycleniter'] = th.cycleniter() if not inpdict['cycleniter'] else int(inpdict['cycleniter'])
        target['scales'] = th.scales() if not inpdict['scales'] else inpdict['scales']
        target['uvtaper'] = th.uvtaper(None, None, None) if not inpdict['uvtaper'] else inpdict['uvtaper']
        target['uvrange'] = th.uvrange() if not inpdict['uvrange'] else inpdict['uvrange']
        target['deconvolver'] = th.deconvolver(None, None) if not inpdict['deconvolver'] else inpdict['deconvolver']
        target['robust'] = th.robust(None)[0] if not inpdict['robust'] else inpdict['robust']
        target['mask'] = th.mask() if not inpdict['mask'] else inpdict['mask']
        target['specmode'] = th.specmode() if not inpdict['specmode'] else inpdict['specmode']
        target['gridder'] = th.gridder(None, None) if not inpdict['gridder'] else inpdict['gridder']
        buffer_arcsec = th.buffer_radius() if not inpdict['search_radius_arcsec'] else inpdict['search_radius_arcsec']
        result.capture_buffer_size(buffer_arcsec)
        target['cell'] = th.cell(None, None) if not inpdict['cell'] else inpdict['cell']
        target['imsize'] = th.imsize(None, None, None, None, None, None) if not inpdict['imsize'] else inpdict['imsize']
        target['intent'] = th.intent() if not inpdict['intent'] else inpdict['intent']
        target['nterms'] = th.nterms() if not inpdict['nterms'] else inpdict['nterms']
        target['stokes'] = th.stokes() if not inpdict['stokes'] else inpdict['stokes']
        # ------------------------------
        target['nchan'] = inpdict['nchan']
        target['nbin'] = inpdict['nbin']
        target['start'] = inpdict['start']
        target['width'] = inpdict['width']
        target['imagename'] = inpdict['imagename']

        # set the field name list in the image list target
        if fieldnames:
            target['field'] = fieldnames[0]
        else:
            if not isinstance(target['phasecenter'], type(None)):
                # TODO: remove the dependency on cell size being in arcsec

                # remove brackets and begin/end string characters
                # if cell is a list, get the first string element
                if isinstance(target['cell'], type([])):
                    target['cell'] = target['cell'][0]
                target['cell'] = target['cell'].strip('[').strip(']')
                target['cell'] = target['cell'].replace("'", '')
                target['cell'] = target['cell'].replace('"', '')
                # We always search for fields in 1sq degree with a surrounding buffer
                mosaic_side_arcsec = 3600  # 1 degree
                dist = (mosaic_side_arcsec / 2.) + float(buffer_arcsec)
                dist_arcsec = str(dist) + 'arcsec'
                found_fields = target['heuristics'].find_fields(distance=dist_arcsec,
                                                                phase_center=target['phasecenter'],
                                                                matchregex=['^0', '^1', '^2'])
                if found_fields:
                    target['field'] = ','.join(str(x) for x in found_fields)

        for key, value in target.items():
            LOG.info("{k} = {v}".format(k=key, v=value))

        try:
            if len(target['field']) > 0:
                result.add_target(target)
            else:
                raise
        except TypeError:
            LOG.error('No fields to image.')

        if not target['imagename']:
            LOG.error('No imagename provided.')

        return result

    def analyse(self, result):
        return result
