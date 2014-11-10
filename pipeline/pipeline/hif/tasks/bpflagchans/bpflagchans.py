from __future__ import absolute_import

import collections
import os
import numpy as np 
import re
import types

from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.casatools as casatools
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from .resultobjects import BandpassflagResults
from ..common import commonresultobjects
from ..common import calibrationtableaccess as caltableaccess
from ..common import commonhelpermethods
from ..common import viewflaggers

from .. import bandpass

LOG = infrastructure.get_logger(__name__)


class BandpassflagchansInputs(basetask.StandardInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
      flag_hilo=None, fhl_limit=None, fhl_minsample=None,
      flag_tmf=None, tmf_limit=None, niter=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def caltable(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('caltable')

        if self._caltable is None:
            caltables = self.context.callibrary.active.get_caltable(
              caltypes='bandpass')

            # return just the bandpass table that matches the vis being handled
            for name in caltables:
                # Get the bandpass table name
                bptable_vis = \
                  caltableaccess.CalibrationTableDataFiller._readvis(name)
                if bptable_vis in self.vis:
                    return name

        return self._caltable

    @caltable.setter
    def caltable(self, value):
        self._caltable = value

    @property
    def flag_hilo(self):
        return self._flag_hilo

    @flag_hilo.setter
    def flag_hilo(self, value):
        if value is None:
            value = True
        self._flag_hilo = value

    @property
    def fhl_limit(self):
        return self._fhl_limit

    @fhl_limit.setter
    def fhl_limit(self, value):
        if value is None:
            value = 7
        self._fhl_limit = value

    @property
    def fhl_minsample(self):
        return self._fhl_minsample

    @fhl_minsample.setter
    def fhl_minsample(self, value):
        if value is None:
            value = 5
        self._fhl_minsample = value

    @property
    def flag_tmf(self):
        return self._flag_tmf

    @flag_tmf.setter
    def flag_tmf(self, value):
        if value is None:
            value = True
        self._flag_tmf = value

    @property
    def tmf_limit(self):
        return self._tmf_limit

    @tmf_limit.setter
    def tmf_limit(self, value):
        if value is None:
            value = 0.3
        self._tmf_limit = value

    @property
    def niter(self):
        return self._niter

    @niter.setter
    def niter(self, value):
        if value is None:
            value = 1
        self._niter = value


class Bandpassflagchans(basetask.StandardTaskTemplate):
    Inputs = BandpassflagchansInputs

    def prepare(self):
        inputs = self.inputs

        # check that BANDPASS and PHASE intent data are not the same scans
        ms = inputs.context.observing_run.get_ms(name=inputs.vis)
        bandpass_scans = [scan.id for scan in ms.scans if 'BANDPASS' in scan.intents]
        phase_scans = [scan.id for scan in ms.scans if 'PHASE' in scan.intents]
        bandpass_scans.sort()
        phase_scans.sort()

        if bandpass_scans==phase_scans:
            LOG.error('%s BANDPASS and PHASE data identical - bpflagchans aborting' %
              os.path.basename(inputs.vis))
            return BandpassflagResults()
        else:
            LOG.info('%s BANDPASS scans %s differ from PHASE scans %s - safe to proceed' %
             (os.path.basename(inputs.vis), bandpass_scans, phase_scans))

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = BandpassflagchansWorkerInputs(context=inputs.context,
          output_dir=inputs.output_dir, caltable=inputs.caltable,
          vis=inputs.vis)
        datatask = BandpassflagchansWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, table=inputs.caltable, inpfile=[])
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Translate the input flagging parameters to a more compact
        # list of rules.
        rules = viewflaggers.MatrixFlagger.make_flag_rules(
          flag_hilo=inputs.flag_hilo, fhl_limit=inputs.fhl_limit,
          fhl_minsample=inputs.fhl_minsample,
          flag_tmf1=inputs.flag_tmf, tmf1_axis='Channels', 
          tmf1_limit=inputs.tmf_limit)
        flagger = viewflaggers.MatrixFlagger
 
        # Construct the flagger task around the data view task  and the
        # flagger task. When executed this will:
        #   loop:
        #     execute datatask to obtain view from underlying data
        #     examine view, raise flags
        #     execute flagsetter task to set flags in underlying data        
        #     exit loop if no flags raised or if # iterations > niter 
        flaggerinputs = flagger.Inputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, flagsettertask=flagsettertask,
          rules=rules, niter=inputs.niter)
        flaggertask = flagger(flaggerinputs)

 

	# Execute it to flag the data view
        summary_job = casa_tasks.flagdata(vis=inputs.vis, mode='summary')
        stats_before = self._executor.execute(summary_job)
        result = self._executor.execute(flaggertask)
        summary_job = casa_tasks.flagdata(vis=inputs.vis, mode='summary')
        stats_after = self._executor.execute(summary_job)

        result.summaries = [stats_before, stats_after]

        return result

    def analyse(self, result):
        return result


class BandpassflagchansWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, caltable=None):
        self._init_properties(vars())


class BandpassflagchansWorker(basetask.StandardTaskTemplate):
    Inputs = BandpassflagchansWorkerInputs

    def __init__(self, inputs):
        super(BandpassflagchansWorker, self).__init__(inputs)
        self.result = BandpassflagResults()

    def prepare(self):
        inputs = self.inputs
        final = []

        # Calculate new bandpass from PHASE intent
        view_inputs = bandpass.PhcorBandpass.Inputs(inputs.context,
          vis=inputs.vis,
          mode='channel',
          intent='PHASE',
          minsnr=0.0,
          qa2_intent='',
          run_qa2=False)
        name = view_inputs.caltable

        task = bandpass.PhcorBandpass(view_inputs)
        view_result = self._executor.execute(task, merge=False)

	# Loop over caltables.
        name = view_result.final[0].gaintable

        # Get the bandpass table name
        bptable = caltableaccess.CalibrationTableDataFiller.getcal(name)
        self.result.vis = bptable.vis

        # Get the MS object.
        ms = self.inputs.context.observing_run.get_ms(name=bptable.vis)

        # Get the spws from the bandpass table.
        bpspws = set()
        for row in bptable.rows:
            bpspws.update([row.get('SPECTRAL_WINDOW_ID')])

        LOG.info ('Computing flagging metrics for caltable %s ' % (name))
        for spwid in bpspws:
            # calculate view for each group of fieldids
            self.calculate_view(bptable, spwid)

        return self.result

    def analyse(self, result):
        return result

    def calculate_view(self, bptable, spwid):
        """
        bptable -- CalibrationTableData object giving access to the bandpass
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        """

        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # get antenna names
        antenna_name, antenna_ids = commonhelpermethods.get_antenna_names(ms)

        # get names of corr products - not quite what we want but better names
        # for pol than nothing
        corr_type = commonhelpermethods.get_corr_products(ms, spwid)
        npols = len(corr_type)
        pols = np.arange(npols)

        # get the frequency axis for this spectral window
        # need to get this info from bpcal table itself as channels may
        # have been averaged
        nchan = None
        spwtablename = '%s/SPECTRAL_WINDOW' % bptable.name
        with casatools.TableReader(spwtablename) as table:
            freqs = table.getcell('CHAN_FREQ', spwid)
            nchan = len(freqs)
            frequnits = table.getcolkeyword('CHAN_FREQ', 'QuantumUnits')[0]
            # make numbers sensible
            if frequnits=='Hz':
                if np.median(freqs) > 1.0e9:
                     freqs /= 1.0e9
                     frequnits = 'GHz'
                elif np.median(freqs) > 1.0e6:
                     freqs /= 1.0e6
                     frequnits = 'MHz'
                elif np.median(freqs) > 1.0e3:
                     freqs /= 1.0e3
                     frequnits = 'kHz'

        # arrays to hold view for this spw
        viewdata = np.zeros([npols, nchan, antenna_ids[-1]+1])
        viewflag = np.ones([npols, nchan, antenna_ids[-1]+1], np.bool)

        # fill in view arrays
        for row in bptable.rows:
            rowspw=row.get('SPECTRAL_WINDOW_ID')
            if rowspw != spwid:
                continue

            data = row.get('CPARAM')
            flag = row.get('FLAG')

            for pol in pols:
                antid = row.get('ANTENNA1')
                viewdata[pol,:,antid] = np.abs(data[pol,:,0])
                viewflag[pol,:,antid] = np.abs(flag[pol,:,0])

        # lastly, construct the view objects
        axes = [commonresultobjects.ResultAxis(name='Channels',
          units='', data=np.arange(nchan)),
          commonresultobjects.ResultAxis(name='Antenna1',
          units='id', data=np.arange(antenna_ids[-1]+1))]
# following has freq axis instead of channels
#        axes = [commonresultobjects.ResultAxis(name='Frequency',
#          units=frequnits, data=freqs),
#          commonresultobjects.ResultAxis(name='Antenna1',
#          units='id', data=np.arange(antenna_ids[-1]+1))]

        for pol in pols:
            viewresult = commonresultobjects.ImageResult(
              data=viewdata[pol],
              flag=viewflag[pol],
              datatype='Bandpass amp',
              filename='%s(bpcal)' % os.path.basename(bptable.vis),
              field_id=row.get('FIELD_ID'),
              spw=spwid,
              pol=corr_type[pol][0],
              axes=axes)

            # add the view results to the class result structure
            self.result.addview(viewresult.description, viewresult)
