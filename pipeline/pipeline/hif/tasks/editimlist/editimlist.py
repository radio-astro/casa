from __future__ import absolute_import
import types
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.basetask as basetask
from .resultobjects import EditimlistResult
from pipeline.hif.heuristics import imageparams
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


class EditimlistInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
                 imagename=None, intent=None, field=None, spw=None, contfile=None,
                 linesfile=None, uvrange=None, specmode=None, outframe=None,

                 # gridder=None, spwsel_lsrk=None, spwsel_topo=None,

                 imsize=None, cell=None, calmaxpix=None, phasecenter=None,
                 nchan=None, start=None, width=None, nbin=None,
                 stokes=None, parameter_file=False, nterms=None,
                 editmode=None):

        self._init_properties(vars())

    # context, output_dir and vis setters/getters are handled by
    # the base class

    @property
    def imagename(self):
        return self._imagename

    @imagename.setter
    def imagename(self, value):
        self._imagename = value

    @property
    def intent(self):
        return self._intent

    @intent.setter
    def intent(self, value):
        self._intent = value

    @property
    def field(self):
        return self._field

    @field.setter
    def field(self, value):
        self._field = value

    @property
    def spw(self):
        return self._spw

    @spw.setter
    def spw(self, value):
        self._spw = value

    @property
    def contfile(self):
        return self._contfile

    @contfile.setter
    def contfile(self, value=None):
        self._contfile = value

    @property
    def linesfile(self):
        return self._linesfile

    @linesfile.setter
    def linesfile(self, value=None):
        self._linesfile = value

    @property
    def uvrange(self):
        return self._uvrange

    @uvrange.setter
    def uvrange(self, value):
        if value is None:
            value = ''
        self._uvrange = value

    @property
    def specmode(self):
        return self._specmode

    @specmode.setter
    def specmode(self, value):
        self._specmode = value

    @property
    def outframe(self):
        return self._outframe

    @outframe.setter
    def outframe(self, value):
        self._outframe = value

    @property
    def imsize(self):
         return self._imsize

    @imsize.setter
    def imsize(self, value):
        self._imsize = value

    @property
    def cell(self):
        return self._cell

    @cell.setter
    def cell(self, value):
        self._cell = value

    @property
    def calmaxpix(self):
        return self._calmaxpix

    @calmaxpix.setter
    def calmaxpix(self, value):
        if value is None:
            value = 300
        self._calmaxpix = value

    @property
    def phasecenter(self):
        return self._phasecenter

    @phasecenter.setter
    def phasecenter(self, value):
        self._phasecenter = value

    # optional added parameters start here

    @property
    def nchan(self):
        return self._nchan

    @nchan.setter
    def nchan(self, value):
        self._nchan = value

    @property
    def start(self):
        return self._start

    @start.setter
    def start(self, value):
        self._start = value

    @property
    def width(self):
        return self._width

    @width.setter
    def width(self, value):
        self._width = value

    @property
    def nbin(self):
        return self._nbin

    @nbin.setter
    def nbin(self, value):
        self._nbin = value

    @property
    def stokes(self):
        return self._stokes

    @stokes.setter
    def stokes(self, value):
        self._stokes = value

    @property
    def parameter_file(self):
        return self._parameter_file

    @parameter_file.setter
    def parameter_file(self, value):
        self._parameter_file = value

    @property
    def nterms(self):
        return self._nterms

    @nterms.setter
    def nterms(self, value):
        if value is None:
            value = 2
        self._nterms = value

    @property
    def editmode(self):
        return self._editmode

    @editmode.setter
    def editmode(self, value):
        self._editmode = value

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


# maps intent and specmode Inputs parameters to textual description of execution context.
_DESCRIPTIONS = {
    ('PHASE,BANDPASS,CHECK', 'mfs'): 'Set-up image parameters for calibrator imaging',
    ('TARGET', 'mfs'): 'Set-up image parameters for target per-spw continuum imaging',
    ('TARGET', 'cont'): 'Set-up image parameters for target aggregate continuum imaging',
    ('TARGET', 'cube'): 'Set-up image parameters for target cube imaging',
}
