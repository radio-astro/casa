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
                 hm_imsize=None, hm_cell=None, calmaxpix=None, phasecenter=None,
                 nchan=None, start=None, width=None, nbins=None,
                 polproducts=None, parameter_file=False, mfs_nterms=None, tapersize=None):

        self._init_properties(vars())

    # context, output_dir and vis setters/getters are handled by 
    # the base class 

    @property
    def imagename(self):
        return self._imagename

    @imagename.setter
    def imagename(self, value):
        if value is None:
            value = ''
        self._imagename = value

    @property
    def intent(self):
        return self._intent

    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'TARGET'
        self._intent = value

    @property
    def field(self):
        if (self._field in [None, '']) and (
            'TARGET' in self.intent) and self.context.size_mitigation_parameters.has_key('field'):
            return self.context.size_mitigation_parameters['field']
        else:
            return self._field

    @field.setter
    def field(self, value):
        if value is None:
            value = ''
        self._field = value

    @property
    def spw(self):
        return self._spw

    @spw.setter
    def spw(self, value):
        if value is None:
            value = ''
        self._spw = value

    @property
    def contfile(self):
        return self._contfile

    @contfile.setter
    def contfile(self, value=None):
        if value in (None, ''):
            value = os.path.join(self.context.output_dir, 'cont.dat')
        self._contfile = value

    @property
    def linesfile(self):
        return self._linesfile

    @linesfile.setter
    def linesfile(self, value=None):
        if value in (None, ''):
            value = os.path.join(self.context.output_dir, 'lines.dat')
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
        if self._specmode is None:
            if 'TARGET' in self.intent:
                return 'cube'
            else:
                return 'mfs'
        return self._specmode

    @specmode.setter
    def specmode(self, value):
        self._specmode = value

    @property
    def outframe(self):
        if self._outframe is None:
            return 'LSRK'
        return self._outframe

    @outframe.setter
    def outframe(self, value):
        self._outframe = value

    @property
    def hm_imsize(self):
        if self._hm_imsize is None:
            if ('TARGET' in self.intent) and self.context.size_mitigation_parameters.has_key('hm_imsize'):
                self._hm_imsize = self.context.size_mitigation_parameters['hm_imsize']
            else:
                self._hm_imsize = []

        # Convert string to list
        if type(self._hm_imsize) is types.StringType:
            if len(self._hm_imsize) > 0:
                if self._hm_imsize[0] == '[':
                    temp = self._hm_imsize.translate(None, '[]\'')
                    temp = temp.split(',')
                    try:
                        self._hm_imsize = map(int, temp)
                    except:
                        self._hm_imsize = temp
                    self._hm_imsize = map(int, temp)
            else:
                self._hm_imsize = []
        elif type(self._hm_imsize) is types.IntType:
            self._hm_imsize = [self._hm_imsize, self._hm_imsize]

        # Convert to single string for '<number>pb' option
        temp = None
        for item in self._hm_imsize:
            if type(item) is types.StringType:
                if item.find('pb') != -1:
                    temp = item
        if temp:
            self._hm_imsize = temp

        return self._hm_imsize

    @hm_imsize.setter
    def hm_imsize(self, value):
        self._hm_imsize = value

    @property
    def hm_cell(self):
        if self._hm_cell is None:
            if ('TARGET' in self.intent) and self.context.size_mitigation_parameters.has_key('hm_cell'):
                self._hm_cell = self.context.size_mitigation_parameters['hm_cell']
            else:
                self._hm_cell = []

        # Convert string to list
        if type(self._hm_cell) is types.StringType:
            if len(self._hm_cell) > 0:
                if self._hm_cell[0] == '[':
                    temp = self._hm_cell.translate(None, '[]\'')
                    temp = temp.split(',')
                    self._hm_cell = temp
            else:
                self._hm_cell = []

        # Convert to single string for '<nummber>ppb' option
        temp = None
        for item in self._hm_cell:
            if item.find('ppb') != -1:
                temp = item.replace('"', '').replace("'", "")
        if temp:
            self._hm_cell = temp

        return self._hm_cell

    @hm_cell.setter
    def hm_cell(self, value):
        self._hm_cell = value

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
        if value is None:
            value = ''
        self._phasecenter = value

    # optional added parameters start here

    @property
    def nchan(self):
        return self._nchan

    @nchan.setter
    def nchan(self, value):
        if value is None:
            value = -1
        self._nchan = value

    @property
    def start(self):
        return self._start

    @start.setter
    def start(self, value):
        if value is None:
            value = ''
        self._start = value

    @property
    def width(self):
        return self._width

    @width.setter
    def width(self, value):
        if value is None:
            value = ''
        self._width = value

    @property
    def nbins(self):
        if (self._nbins in [None, '']) and (
            'TARGET' in self.intent) and self.context.size_mitigation_parameters.has_key('nbins'):
            return self.context.size_mitigation_parameters['nbins']
        else:
            return self._nbins

    @nbins.setter
    def nbins(self, value):
        if value is None:
            value = ''
        self._nbins = value

    @property
    def polproducts(self):
        return self._polproducts

    @polproducts.setter
    def polproducts(self, value):
        if value is None:
            value = ''
        self._polproducts = value


    @property
    def parameter_file(self):
        return self._parameter_file

    @parameter_file.setter
    def parameter_file(self, value):
        if value is None:
            value = ''
        self._parameter_file = value

    @property
    def mfs_nterms(self):
        return self._mfs_nterms

    @mfs_nterms.setter
    def mfs_nterms(self, value):
        if value is None:
            value = ''
        self._mfs_nterms = value

    @property
    def tapersize(self):
        return self._tapersize

    @tapersize.setter
    def tapersize(self, value):
        if value is None:
            value = ''
        self._tapersize = value



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

        if os.access(inputs.parameter_file, os.R_OK):
            with open(inputs.parameter_file) as parfile:
                for line in parfile:
                    parameter, value = line.partition('=')[::2]
                    if parameter.strip() is not 'parameter_file':
                        exec ('inputs.' + parameter.strip() + '=' + value.strip())

        # make sure inputs.vis is a list, even it is one that contains a
        # single measurement set
        if type(inputs.vis) is not types.ListType:
            inputs.vis = [inputs.vis]

        # read the spw, if none then set default 
        spw = inputs.spw

        if spw == '':
            ms = inputs.context.observing_run.get_ms(name=inputs.vis[0])
            spws = ms.get_spectral_windows(science_windows_only=True)
            spwids = [spw.id for spw in spws]
            spw = ','.join("'%s'" % (spwid) for spwid in spwids)
            spw = '[%s]' % spw
        else:
            spwids = spw.split(',')
            spw = ','.join("'%s'" % (spwid) for spwid in spwids)
            spw = '[%s]' % spw

        spwlist = spw.replace('[','').replace(']','')
        spwlist = spwlist[1:-1].split("','")

        if inputs.specmode == 'cont':
            # Make sure the spw list is sorted numerically
            spwlist = [','.join(map(str, sorted(map(int, spwlist))))]

        # now construct the list of imaging command parameter lists that must
        # be run to obtain the required images
        result = EditimlistResult()

        target = {'field': inputs.field,
                  'intent': inputs.intent,
                  'spw': spwlist,
                  'spwsel_lsrk': '',
                  'spwsel_topo': [],
                  'cell': '',
                  'imsize': '',
                  'phasecenter': '',
                  'specmode': inputs.specmode,
                  'gridder': '',
                  'imagename': inputs.imagename,
                  'start': inputs.start,
                  'width': inputs.width,
                  'nbin': '',
                  'nchan': inputs.nchan,
                  'uvrange': inputs.uvrange,
                  'polproducts': inputs.polproducts,
                  'mfs_nterms': inputs.mfs_nterms,
                  'tapersize': inputs.tapersize
                  }

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
