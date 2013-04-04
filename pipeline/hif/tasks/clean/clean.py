from __future__ import absolute_import
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from .cleanworker import CleanWorker
from pipeline.hif.heuristics import clean

LOG = infrastructure.get_logger(__name__)


class CleanInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, spw=None,
      intent=None, field=None, imagename=None, mode=None, imagermode=None,
      outframe=None, nchan=None, start=None, width=None, imsize=None,
      cell=None, phasecenter=None, restfreq=None, weighting=None,
      weighting_rmode=None, robust=None, noise=None):
        self._init_properties(vars())
        self.heuristics = clean.CleanHeuristics(
          context=context, vislist=self.vis, spw=self.spw)

    @property
    def cell(self):
        if self._cell is None:
            return self._context.observing_run.cellsize
        return self._cell

    @cell.setter
    def cell(self, value):
        self._cell = value

    @property
    def field(self):
        if self._field is None:
            return ''
        return self._field

    @field.setter
    def field(self, value):
        self._field = value

    @property
    def field_id(self):
        return self.heuristics.field(self._intent, self._field)

    @field_id.setter
    def field_id(self, value):
        print 'field_id setter being called, rethink required'

    @property
    def imagermode(self):
        if self._imagermode is None:
            return self.heuristics.imagermode(self._intent, self._field)
        return self._imagermode

    @imagermode.setter
    def imagermode(self, value):
         self._imagermode = value

    @property
    def imsize(self):
        if self._imsize is None:
            return self.heuristics.imsize(fields=self.field_id, cell=self.cell,
              spw=self.spw)
        return self._imsize

    @imsize.setter
    def imsize(self, value):
         self._imsize = value

    @property
    def imagename(self):
        if self._imagename is None:
            return self.heuristics.imagename(intent=self.intent,
              field=self.field, spw=self.spw)
        return self._imagename

    @imagename.setter
    def imagename(self, value):
         self._imagename = value

    @property
    def intent(self):
        if self._intent is None:
            return ''
        return self._intent

    @intent.setter
    def intent(self, value):
        self._intent = value

    @property
    def mode(self):
        if self._mode is None:
            if 'TARGET' in self.intent:
                return 'frequency'
            else:
                return 'mfs'
        return self._mode
  
    @mode.setter
    def mode(self, value):
        self._mode = value

    @property
    def nchan(self):
        if self._nchan is None:
            return -1
        return self._nchan

    @nchan.setter
    def nchan(self, value):
         self._nchan = value

    @property
    def outframe(self):
        if self._outframe is None:
            return 'LSRK'
        return self._outframe

    @outframe.setter
    def outframe(self, value):
         self._outframe = value

    @property
    def phasecenter(self):
        if self._phasecenter is None:
            return self.heuristics.phasecenter(self.field_id)
        return self._phasecenter

    @phasecenter.setter
    def phasecenter(self, value):
        self._phasecenter = value

    @property
    def restfreq(self):
        if self._restfreq is None:
            return ''
        return self._restfreq

    @restfreq.setter
    def restfreq(self, value):
        self._restfreq = value

    @property
    def start(self):
        if self._start is None:
            return ''
        return self._start

    @start.setter
    def start(self, value):
         self._start = value

    @property
    def spw(self):
        if self._spw is None:
            mslist = self._context.observing_run.measurement_sets
            spws = mslist[0].get_spectral_windows()
            spwids = [spw.id for spw in spws]
            return ','.join(spwids)
        return self._spw

    @spw.setter
    def spw(self, value):
        self._spw = value

    @property
    def weighting(self):
        if self._weighting is None:
            return 'natural'
        return self._weighting

    @weighting.setter
    def weighting(self, value):
         self._weighting = value

    @property
    def robust(self):
        if self._robust is None:
            return 0.0
        return self._robust

    @robust.setter
    def robust(self, value):
         self._robust = value

    @property
    def noise(self):
        if self._noise is None:
            return '1.0Jy'
        return self._noise

    @noise.setter
    def noise(self, value):
        self._noise = value

    @property
    def width(self):
        if self._width is None:
            return ''
        return self._width

    @width.setter
    def width(self, value):
         self._width = value


class Clean(basetask.StandardTaskTemplate):
    Inputs = CleanInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs

        # make sure inputs.vis is a list, even it is one that contains a
        # single measurement set
        if type(inputs.vis) is not types.ListType:
            inputs.vis = [inputs.vis]

        # this python class will produce one data product; a clean image using
        # all the data input to it.

        # get specs of field to be cleaned
        intent = inputs.intent
        field = inputs.field
        spw = inputs.spw
	if inputs.width == '':
	    if inputs.mode != 'mfs':
                width = inputs.heuristics.width(int(spw))
	    else:
	        width = inputs.width 
	else:
	    width = inputs.width

        # use scanids to select data with the specified intent
        scanidlist = []
        for vis in inputs.vis:
            ms = inputs.context.observing_run.get_ms(name=vis)
            scanids = [scan.id for scan in ms.scans if
              intent in scan.intents and
              field in [fld.name for fld in scan.fields]]
            scanids = str(scanids)
            scanids = scanids.replace('[', '')
            scanids = scanids.replace(']', '')
            scanidlist.append(scanids)

        LOG.info('#')
        LOG.info("# Reduction for intent '%s', field %s, SpW %s" % (intent,
          field, spw))
        LOG.info('#')
        
        datainputs = CleanWorker.Inputs(context=inputs.context, 
          output_dir=inputs.output_dir, vis=inputs.vis,
          mode=inputs.mode, imagermode=inputs.imagermode,
          imagename=inputs.imagename,
          intent=inputs.intent, field_id=inputs.field_id, 
          field=inputs.field, spw=spw,
          scan=scanidlist,
          phasecenter=inputs.phasecenter, cell=inputs.cell,
          imsize=inputs.imsize, outframe=inputs.outframe,
          restfreq=inputs.restfreq,
          #nchan=inputs.nchan, start=inputs.start, width=inputs.width,
          nchan=inputs.nchan, start=inputs.start, width=width,
          weighting=inputs.weighting,
          robust=inputs.robust,
          noise=inputs.noise)
        datatask = CleanWorker(datainputs)

        try:
            if inputs.imagermode == 'mosaic':
                # remove rows in POINTING table - bug workaround.
                self._empty_pointing_table()

            # do the clean
            result = self._executor.execute(datatask)

        finally:
            if inputs.imagermode == 'mosaic':
                # restore POINTING table to input state
                self._restore_pointing_table()

        return result

    def analyse(self, result):
        return result

    def _empty_pointing_table(self):
        # Concerned that simply renaming things directly 
        # will corrupt the table cache, so do things using only the
        # table tool.
        for vis in self.inputs.vis:
            with casatools.TableReader(
              '%s/POINTING' % vis, nomodify=False) as table:
                # make a copy of the table
                LOG.debug('making copy of POINTING table')
                table.copy('%s/POINTING_COPY' % vis, valuecopy=True)
                LOG.debug('removing all POINTING table rows')
                table.removerows(range(table.nrows()))

    def _restore_pointing_table(self):
        for vis in self.inputs.vis:
            # restore the copy of the POINTING table
            with casatools.TableReader(
              '%s/POINTING_COPY' % vis, nomodify=False) as table:
                LOG.debug('copying back into POINTING table')
                table.copy('%s/POINTING' % vis, valuecopy=True)
