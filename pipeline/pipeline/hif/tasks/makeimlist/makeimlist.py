from __future__ import absolute_import
import types
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.basetask as basetask
from .resultobjects import MakeImListResult
from pipeline.hif.heuristics import makeimlist 
#from pipeline.hif.heuristics import clean
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)

class MakeImListInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, 
      imagename=None, intent=None, field=None, spw=None, contfile=None,
      linesfile=None, uvrange=None, specmode=None, outframe=None,
      imsize=None, cell=None, calmaxpix=None, phasecenter=None,
      nchan=None, start=None, width=None):

        self._init_properties(vars())

    # context, output_dir and vis setters/getters are handled by 
    # the base class 

    @property
    def imagename(self):
        if self._imagename is None:
            return ''
        return self._imagename

    @imagename.setter
    def imagename(self, value):
         self._imagename = value

    @property
    def intent(self):
        if self._intent is None:
            return 'TARGET'
        return self._intent

    @intent.setter
    def intent(self, value):
        self._intent = value

    @property
    def field(self):
        if self._field is None:
            return ''
        return self._field

    @field.setter
    def field(self, value):
        self._field = value

    @property
    def spw(self):
        if self._spw is None:
            return ''
        return self._spw

    @spw.setter
    def spw(self, value):
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
        if self._uvrange is None:
            return ''
        return self._uvrange

    @uvrange.setter
    def uvrange(self, value):
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
    def imsize(self):
        if self._imsize is None:
            return []
        elif type(self._imsize) is types.StringType:
            if self._imsize[0] == '[':
                temp = self._imsize.translate(None, '[]\'')
            temp = temp.split(',')
            self._imsize = map(int, temp)

        return self._imsize

    @imsize.setter
    def imsize(self, value):
        self._imsize = value

    @property
    def cell(self):
        if self._cell is None:
            return []
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
        if self._phasecenter is None:
            return ''
        return self._phasecenter

    @phasecenter.setter
    def phasecenter(self, value):
        self._phasecenter = value

    # optional added parameters start here

    @property
    def nchan(self):
        if self._nchan is None:
            return -1
        return self._nchan

    @nchan.setter
    def nchan(self, value):
        self._nchan = value

    @property
    def start(self):
        if self._start is None:
            return ''
        return self._start

    @start.setter
    def start(self, value):
        self._start = value

    @property
    def width(self):
        if self._width is None:
            return ''
        return self._width

    @width.setter
    def width(self, value):
        self._width = value


class MakeImList(basetask.StandardTaskTemplate):
    Inputs = MakeImListInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        # this python class will produce a list of images to be calculated.
        inputs = self.inputs

        qaTool = casatools.quanta

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

        # instantiate the heuristics classes needed, some sorting out needed
        # here to remove duplicated code
        self.heuristics = makeimlist.MakeImListHeuristics(
          context=inputs.context, vislist=inputs.vis, spw=spw, contfile=inputs.contfile, linesfile=inputs.linesfile)

        # get list of field_ids/intents to be cleaned
        field_intent_list = self.heuristics.field_intent_list(
          intent=inputs.intent, field=inputs.field)

        # Remove bad spws in cont mode
        if inputs.specmode == 'cont':
            filtered_spwlist = []
            for spw in spwlist[0].split(','):
                # Can not just use "has_data" as it only sets up
                # a selection which checks against existance of
                # a given item (e.g. an spw).
                cell, valid_data = self.heuristics.cell(field_intent_list=field_intent_list, spwspec=spw)
                # For now we consider the spw for all fields / intents.
                # May need to handle this individually.
                if (valid_data[list(field_intent_list)[0]]):
                    filtered_spwlist.append(spw)
            spwlist = [reduce(lambda x,y: x+','+y, filtered_spwlist)]

        # get beams for each spw
        beams = {}
        for spwspec in spwlist:
            beams[spwspec] = self.heuristics.beam(spwspec=spwspec)

        # cell is a list of form [cellx, celly]. If the list has form [cell]
        # then that means the cell is the same size in x and y. If cell is
        # empty then fill it with a heuristic result
        cell = inputs.cell
        cells = {}
        valid_data = {}
        if cell == []:
            min_cell = ['3600arcsec']
            for spwspec in spwlist:
                # the heuristic cell is always the same for x and y as
                # the value derives from the single value returned by
                # imager.advise
                cells[spwspec], valid_data[spwspec] = self.heuristics.cell(
                  field_intent_list=field_intent_list, spwspec=spwspec)
                if (cells[spwspec] != ['invalid']):
                    min_cell = cells[spwspec] if (qaTool.convert(cells[spwspec][0], 'arcsec')['value'] < qaTool.convert(min_cell[0], 'arcsec')['value']) else min_cell
            # Rounding to two significant figures
            min_cell = ['%.2g%s' % (qaTool.getvalue(min_cell[0]), qaTool.getunit(min_cell[0]))]
            # Use same cell size for all spws (in a band (TODO))
            for spwspec in spwlist:
                if (cells[spwspec] != ['invalid']):
                    cells[spwspec] = min_cell
        else:
            for spwspec in spwlist:
                cells[spwspec] = cell
                # TODO: "has_data" does not really check the data
                valid_data[spwspec] = self.heuristics.has_data(
                  field_intent_list=field_intent_list, spwspec=spwspec)

        # if phase center not set then use heuristic code to calculate the
        # centers for each field
        phasecenter = inputs.phasecenter
        if phasecenter == '':
            phasecenters = {}
            for field_intent in field_intent_list:
                try:
                    field_ids = self.heuristics.field(
                      field_intent[1], field_intent[0])
                    phasecenters[field_intent[0]] = \
                      self.heuristics.phasecenter(field_ids)
                except Exception, e:
                    # problem defining center
                    LOG.warn(e)
                    pass

        # if imsize not set then use heuristic code to calculate the
        # centers for each field/spwspec
        imsize = inputs.imsize
        imsizes = {}
        if imsize == []:
            for field_intent in field_intent_list:
                max_x_size = 1
                max_y_size = 1
                for spwspec in spwlist:
                    if not valid_data[spwspec][field_intent]:
                        continue

                    try:
                        field_ids = self.heuristics.field(
                          field_intent[1], field_intent[0])
                        himsize = self.heuristics.imsize(fields=field_ids,
                          cell=cells[spwspec], beam=beams[spwspec])
                        if field_intent[1] in ['PHASE', 'BANDPASS', 'AMPLITUDE', 'FLUX', 'CHECK']:
                            himsize = [min(npix, inputs.calmaxpix) for npix in himsize]
                        imsizes[(field_intent[0],spwspec)] = himsize
                        if (imsizes[(field_intent[0],spwspec)][0] > max_x_size):
                            max_x_size = imsizes[(field_intent[0],spwspec)][0]
                        if (imsizes[(field_intent[0],spwspec)][1] > max_y_size):
                            max_y_size = imsizes[(field_intent[0],spwspec)][1]
                    except Exception, e:
                        # problem defining imsize
                        LOG.warn(e)
                        pass

                # Use same size for all spws (in a band (TODO))
                for spwspec in spwlist:
                    if valid_data[spwspec][field_intent]:
                        imsizes[(field_intent[0],spwspec)] = [max_x_size, max_y_size]
 
        else:
            for field_intent in field_intent_list:
                for spwspec in spwlist:
                    imsizes[(field_intent[0],spwspec)] = imsize

        # if nchan is not set then use heuristic code to calculate it
        # for each field/spwspec. The channel width needs to be calculated
        # at the same time.
        specmode = inputs.specmode
        nchan = inputs.nchan
        nchans = {}
        width = inputs.width
        widths = {}
        if ((specmode not in ('mfs', 'cont')) and (width == 'pilotimage')):
            for field_intent in field_intent_list:
                for spwspec in spwlist:
                    if not valid_data[spwspec][field_intent]:
                        continue

                    try:
                        nchans[(field_intent[0],spwspec)], widths[(field_intent[0],spwspec)] = \
                          self.heuristics.nchan_and_width(field_intent=field_intent[1], \
                          spwspec=spwspec)
                    except Exception, e:
                        # problem defining nchan and width
                        LOG.warn(e)
                        pass

        else:
            for field_intent in field_intent_list:
                for spwspec in spwlist:
                    nchans[(field_intent[0],spwspec)] = nchan
                    widths[(field_intent[0],spwspec)] = width

        # construct imagename
        imagename = inputs.imagename
        imagenames = {}
        for field_intent in field_intent_list:
            for spwspec in spwlist:
                if inputs.imagename == '':
                    imagenames[(field_intent,spwspec)] = \
                      self.heuristics.imagename(
                      output_dir=inputs.output_dir, intent=field_intent[1],
                      field=field_intent[0], spwspec=spwspec, specmode=specmode)
                else:
                    imagenames[(field_intent,spwspec)] = inputs.imagename

        # now construct the list of imaging command parameter lists that must
        # be run to obtain the required images
        result = MakeImListResult()
        result.set_max_num_targets(len(field_intent_list)*len(spwlist))
        for field_intent in field_intent_list:
            for spwspec in spwlist:
                spwspec_ok = True
                if (specmode in ('mfs', 'cont')):
                    new_spwspec = []
                    spwsel = {}
                    for spwid in spwspec.split(','):
                        spwsel_spwid = self.heuristics.cont_ranges_spwsel[utils.dequote(field_intent[0])][spwid]
                        if (field_intent[1] == 'TARGET'):
                            if (spwsel_spwid == 'NONE'):
                                LOG.warn('No continuum frequency range information detected for %s, spw %s.' % (field_intent[0], spwid))
                            elif (spwsel_spwid == ''):
                                LOG.warn('Empty continuum frequency range for %s, spw %s. Run hif_findcont ?' % (field_intent[0], spwid))

                        if (spwsel_spwid in ('', 'NONE')):
                            spwsel_spwid_freqs = ''
                            spwsel_spwid_refer = 'LSRK'
                        else:
                            spwsel_spwid_freqs, spwsel_spwid_refer = spwsel_spwid.split()

                        if (spwsel_spwid_refer != 'LSRK'):
                            LOG.warn('Frequency selection is specified in %s but must be in LSRK' % (spwsel_refer))
                            # TODO: skip this field and/or spw ?

                        new_spwspec.append(spwid)
                        spwsel['spw%s' % (spwid)] = spwsel_spwid

                    new_spwspec = ','.join(new_spwspec)
                    if ((new_spwspec == '') and (field_intent[1] == 'TARGET')):
                        LOG.warn('No continuum selection for target %s, spw %s. Will not image this selection.' % (field_intent[1], spwspec))
                        spwspec_ok = False
                else:
                    new_spwspec = spwspec
                    spwsel = {}

                if spwspec_ok and valid_data[spwspec][field_intent] and imsizes.has_key((field_intent[0],spwspec)):
                    LOG.debug (
                      'field:%s intent:%s spw:%s cell:%s imsize:%s phasecenter:%s'
                      % (field_intent[0], field_intent[1], spwspec,
                      cells[spwspec], imsizes[(field_intent[0],spwspec)],
                      phasecenters[field_intent[0]]))

                    target = {'field':field_intent[0],
                              'intent':field_intent[1],
                              'spw': new_spwspec,
                              'spwsel': spwsel,
                              'cell':cells[spwspec],
                              'imsize':imsizes[(field_intent[0],spwspec)],
                              'phasecenter':phasecenters[field_intent[0]],
                              'specmode':inputs.specmode,
                              'imagename':imagenames[(field_intent,spwspec)],
                              'start':inputs.start,
                              'width':widths[(field_intent[0],spwspec)],
                              'nchan':nchans[(field_intent[0],spwspec)],
                              'uvrange':inputs.uvrange}

                    result.add_target(target)

        # Temporarily pass contfile and linefile for hif_findcont and hif_makeimages
        result.contfile = inputs.contfile
        result.linesfile = inputs.linesfile

        return result

    def analyse(self, result):
        return result

