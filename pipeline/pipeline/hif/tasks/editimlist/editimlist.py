from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from .resultobjects import EditimlistResult
from pipeline.hif.tasks.makeimlist.cleantarget import CleanTarget

LOG = infrastructure.get_logger(__name__)

class EditimlistInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
                 cell=None,
                 editmode=None,
                 field=None,
                 imagename=None,
                 imsize=None,
                 intent=None,
                 gridder=None,
                 nbin=None,
                 nchan=None,
                 nterms=None,
                 parameter_file=None,
                 phasecenter=None,
                 specmode=None,
                 spw=None,
                 start=None,
                 stokes=None,
                 uvrange=None,
                 width=None,
                 ):

        self._init_properties(vars())

        keys_to_consider = ('field', 'intent', 'spw', 'cell', 'imsize',
                            'phasecenter', 'specmode', 'gridder', 'imagename', 'start', 'width',
                            'nbin', 'nchan', 'uvrange', 'stokes', 'nterms')
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
        if inputs.parameter_file and os.access(inputs.parameter_file, os.R_OK):
            with open(inputs.parameter_file) as parfile:
                for line in parfile:
                    if line.startswith('#') or '=' not in line:
                        continue
                    parameter, value = line.partition('=')[::2]
                    parameter = parameter.strip()
                    value = value.strip()
                    exec ('inputs.' + parameter + '=' + value)
                    inputs.keys_to_change.append(parameter)

        # now construct the list of imaging command parameter lists that must
        # be run to obtain the required images
        result = EditimlistResult()

        target = dict()
        if inputs.editmode == 'add':
            target = CleanTarget()
            inputsdict = inputs.__dict__
            for parameter in inputsdict.keys():
                if inputsdict[parameter] and not parameter.startswith('_') and (parameter in target):
                    cmd = "target['" + parameter + "'] = inputs." + parameter
                    # print(cmd)
                    exec(cmd)
        elif inputs.editmode == 'edit':
            for parameter in inputs.keys_to_change:
                if parameter not in ('editmode'):
                    exec('target["' + parameter + '"] = inputs.' + parameter)

        result.add_target(target)

        return result

    def analyse(self, result):
        return result

