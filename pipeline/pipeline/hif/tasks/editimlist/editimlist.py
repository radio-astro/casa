from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from .resultobjects import EditimlistResult

LOG = infrastructure.get_logger(__name__)


class EditimlistInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
                 imagename=None, intent=None, field=None, spw=None, contfile=None,
                 linesfile=None, uvrange=None, specmode=None, outframe=None,
                 imsize=None, cell=None, phasecenter=None,
                 nchan=None, start=None, width=None, nbin=None,
                 stokes=None, parameter_file=None, nterms=None,
                 editmode=None):

        self._init_properties(vars())

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
        keys_to_change = []
        if inputs.parameter_file and os.access(inputs.parameter_file, os.R_OK):
            with open(inputs.parameter_file) as parfile:
                for line in parfile:
                    if line.startswith('#') or '=' not in line:
                        continue
                    parameter, value = line.partition('=')[::2]
                    parameter = parameter.strip()
                    value = value.strip()
                    exec ('inputs.' + parameter + '=' + value)
                    keys_to_change.append(parameter)

        # now construct the list of imaging command parameter lists that must
        # be run to obtain the required images
        result = EditimlistResult()

        if inputs.editmode == 'add':
            target = {'field': inputs.field,
                      'intent': inputs.intent,
                      'spw': inputs.spw,
                      'spwsel_lsrk': '',#inputs.spwsel_lsrk,
                      'spwsel_topo': '',#inputs.spwsel_topo,
                      'cell': inputs.cell,
                      'imsize': inputs.imsize,
                      'phasecenter': inputs.phasecenter,
                      'specmode': inputs.specmode,
                      'gridder': '', #inputs.gridder,
                      'imagename': inputs.imagename,
                      'start': inputs.start,
                      'width': inputs.width,
                      'nbin': inputs.nbin,
                      'nchan': inputs.nchan,
                      'uvrange': inputs.uvrange,
                      'stokes': inputs.stokes,
                      'nterms': inputs.nterms,
                      }
        elif inputs.editmode == 'edit':
            target = {}
            for parameter in keys_to_change:
                if parameter != 'editmode':
                    exec('target["' + parameter + '"] = inputs.' + parameter)

        result.add_target(target)

        # Temporarily pass contfile and linefile for hif_findcont and hif_makeimages
        result.contfile = inputs.contfile
        result.linesfile = inputs.linesfile

        return result

    def analyse(self, result):
        return result

