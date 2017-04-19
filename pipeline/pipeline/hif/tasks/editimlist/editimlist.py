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
                 uvtaper=None,
                 uvrange=None,
                 width=None,
                 ):

        self._init_properties(vars())

        keys_to_consider = ('field', 'intent', 'spw', 'cell', 'deconvolver', 'imsize',
                            'phasecenter', 'specmode', 'gridder', 'imagename', 'scales',
                            'start', 'width', 'nbin', 'nchan', 'uvrange', 'stokes', 'nterms',
                            'robust', 'uvtaper', 'niter', 'cyclefactor', 'cycleniter', 'mask',
                            'search_radius_arcsec', 'threshold', 'reffreq')
        self.keys_to_change = []
        for key in keys_to_consider:
            # print key, eval(key)
            if self.__dict__[key] is not None:
                self.keys_to_change.append(key)

# tell the infrastructure to give us mstransformed data when possible by
# registering our preference for imaging measurement sets
basetask.ImagingMeasurementSetsPreferred.register(EditimlistInputs)


class Editimlist(basetask.StandardTaskTemplate):
    Inputs = EditimlistInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):

        # this python class will produce a list of images to be calculated.
        inputs = self.inputs

        # if a file is given, read whatever parameters are defined in the file
        if inputs.parameter_file:
            if os.access(inputs.parameter_file, os.R_OK):
                with open(inputs.parameter_file) as parfile:
                    for line in parfile:
                        if line.startswith('#') or '=' not in line:
                            continue
                        parameter, value = line.partition('=')[::2]
                        parameter = parameter.strip()
                        value = value.strip()
                        exec ('inputs.' + parameter + '=' + value)
                        inputs.keys_to_change.append(parameter)
            else:
                LOG.error('Input parameter file is not readable: {fname}'.format(fname=inputs.parameter_file))

        # now construct the list of imaging command parameter lists that must
        # be run to obtain the required images
        result = EditimlistResult()

        target = dict()
        inputs.editmode = 'add' if not inputs.editmode else inputs.editmode

        ms = self.inputs.context.observing_run.get_ms(inputs.vis[0])
        fieldnames = []
        if type(inputs.field) is not type(None):
            for fid in inputs.field:
                if isinstance(fid, int):
                    fieldobj = ms.get_fields(field_id=fid)
                    fieldnames.append(fieldobj[0].name)
                else:
                    fieldnames.append(fid)

            if len(fieldnames) > 1:
                fieldnames = [','.join(fieldnames)]

        target = CleanTarget()
        target['threshold'] = '1.0mJy' if not inputs.threshold else inputs.threshold
        target['reffreq'] = '3.0GHz' if not inputs.reffreq else inputs.reffreq
        target['deconvolver'] = 'mtmfs' if not inputs.deconvolver else inputs.deconvolver
        target['scales'] = [0] if not inputs.scales else inputs.scales
        target['robust'] = 2.0 if not inputs.robust else inputs.robust
        target['uvtaper'] = [] if not inputs.uvtaper else inputs.uvtaper
        target['niter'] = 10000 if not inputs.niter else inputs.niter
        target['cycleniter'] = -1 if not inputs.cycleniter else inputs.cycleniter
        target['cyclefactor'] = 3.0 if not inputs.cyclefactor else inputs.cyclefactor
        target['mask'] = '' if not inputs.mask else inputs.mask
        target['specmode'] = 'mfs' if not inputs.specmode else inputs.specmode
        buffer_arcsec = 1000. if not inputs.search_radius_arcsec else inputs.search_radius_arcsec
        inputsdict = inputs.__dict__
        for parameter in inputsdict.keys():
            if inputsdict[parameter] and not parameter.startswith('_') and (parameter in target):
                inputspar_value = eval('inputs.' + parameter)
                # print(parameter + '=' + str(inputspar_value))
                cmd = "target['" + parameter + "'] = inputs." + parameter
                # print(cmd)
                exec cmd

        iph = imageparams_factory.ImageParamsHeuristicsFactory()
        target['heuristics'] = iph.getHeuristics(context=inputs.context, vislist=inputs.vis, spw=inputs.spw,
                                                 imaging_mode='VLASS')
        # if len(target['cell']) == 1:
        #     target['cell'] = [target['cell'][0], target['cell'][0]]
        # if len(target['imsize']) == 1:
        #     target['imsize'] = [target['imsize'][0], target['imsize'][0]]

        if fieldnames:
            target['field'] = fieldnames[0]
        else:
            if type(target['phasecenter']) is not type(None):
                cellsize_arcsec = float(target['cell'].strip('arcsec'))
                dist = ((target['imsize'][0] / 2.) * cellsize_arcsec) + float(buffer_arcsec)
                dist_arcsec = str(dist) + 'arcsec'
                found_fields = target['heuristics'].find_fields(distance=dist_arcsec, phase_center=target['phasecenter'])
                fieldnames = []
                if type(found_fields) is not type(None):
                    for fid in found_fields:
                        fieldobj = ms.get_fields(field_id=fid)
                        fieldnames.append(fieldobj[0].name)

                    if len(fieldnames) > 1:
                        fieldnames = [','.join(fieldnames)]

                    target['field'] = fieldnames[0]

        # import pprint
        # pprint.pprint(target)
        result.add_target(target)

        return result

    def analyse(self, result):
        return result

