from __future__ import absolute_import

import collections
import numpy as np 
import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from .resultobjects import BandpassflagResults
from ..common import commonresultobjects
from ..common import calibrationtableaccess as caltableaccess
from ..common import viewflaggers

LOG = infrastructure.get_logger(__name__)


class BandpassflagchansInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
      flag_edges=None, edge_limit=None, flag_sharps=None, sharps_limit=None,
      flag_diffmad=None, diffmad_limit=None, diffmad_nchan_limit=None,
      flag_tmf=None, tmf_frac_limit=None, tmf_nchan_limit=None, niter=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def caltable(self):
        if self._caltable is None:
            caltables = self.context.callibrary.active.get_caltable(
              caltypes='bandpass')

            # return just the bandpass table that matches the vis being handled
            result = None
            for name in caltables:
                # Get the bandpass table name
                bptable = caltableaccess.CalibrationTableDataFiller.getcal(
                  name)
                if bptable.vis in self.vis:
                    result = name
                    break

            return result

        return self._caltable

    @caltable.setter
    def caltable(self, value):
        self._caltable = value

    @property
    def flag_edges(self):
        if self._flag_edges is None:
            return False
        return self._flag_edges

    @flag_edges.setter
    def flag_edges(self, value):
        self._flag_edges = value

    @property
    def edge_limit(self):
        if self._edge_limit is None:
            return 3.0
        return self._edge_limit

    @edge_limit.setter
    def edge_limit(self, value):
        self._edge_limit = value

    @property
    def flag_sharps(self):
        if self._flag_sharps is None:
            return False
        return self._flag_sharps

    @flag_sharps.setter
    def flag_sharps(self, value):
        self._flag_sharps = value

    @property
    def sharps_limit(self):
        if self._sharps_limit is None:
            return 0.05
        return self._sharps_limit

    @sharps_limit.setter
    def sharps_limit(self, value):
        self._sharps_limit = value

    @property
    def flag_diffmad(self):
        if self._flag_diffmad is None:
            return True
        return self._flag_diffmad

    @flag_diffmad.setter
    def flag_diffmad(self, value):
        self._flag_diffmad = value

    @property
    def diffmad_limit(self):
        if self._diffmad_limit is None:
            return 14
        return self._diffmad_limit

    @diffmad_limit.setter
    def diffmad_limit(self, value):
        self._diffmad_limit = value

    @property
    def diffmad_nchan_limit(self):
        if self._diffmad_nchan_limit is None:
            return 10000
        return self._diffmad_nchan_limit

    @diffmad_nchan_limit.setter
    def diffmad_nchan_limit(self, value):
        self._diffmad_nchan_limit = value

    @property
    def flag_tmf(self):
        if self._flag_tmf is None:
            return False
        return self._flag_tmf

    @flag_tmf.setter
    def flag_tmf(self, value):
        self._flag_tmf = value

    @property
    def tmf_frac_limit(self):
        if self._tmf_frac_limit is None:
            return 0.05
        return self._tmf_frac_limit

    @tmf_frac_limit.setter
    def tmf_frac_limit(self, value):
        self._tmf_frac_limit = value

    @property
    def tmf_nchan_limit(self):
        if self._tmf_nchan_limit is None:
            return 4
        return self._tmf_nchan_limit

    @tmf_nchan_limit.setter
    def tmf_nchan_limit(self, value):
        self._tmf_nchan_limit = value

    @property
    def niter(self):
        if self._niter is None:
            return 1
        return self._niter

    @niter.setter
    def niter(self, value):
        self._niter = value


class Bandpassflagchans(basetask.StandardTaskTemplate):
    Inputs = BandpassflagchansInputs

    def prepare(self):
        inputs = self.inputs

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
        rules = viewflaggers.VectorFlagger.make_flag_rules (
          flag_edges=inputs.flag_edges, edge_limit=inputs.edge_limit,
          flag_sharps=inputs.flag_sharps, sharps_limit=inputs.sharps_limit,
          flag_diffmad=inputs.flag_diffmad, diffmad_limit=inputs.diffmad_limit,
          diffmad_nchan_limit=inputs.diffmad_nchan_limit,
          flag_tmf=inputs.flag_tmf, tmf_frac_limit=inputs.tmf_frac_limit,
          tmf_nchan_limit=inputs.tmf_nchan_limit)
        flagger = viewflaggers.VectorFlagger
 
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
        result = self._executor.execute(flaggertask)
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

        # Get bandpass results
        final = []

	# Loop over caltables.
        name = inputs.caltable

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
        antenna_ids = [antenna.id for antenna in ms.antennas]
        antenna_ids.sort()
        antenna_name = {}
        for antenna_id in antenna_ids:
            antenna_name[antenna_id] = [antenna.name for antenna in ms.antennas
              if antenna.id==antenna_id][0]

        for row in bptable.rows:
            # The bandpass array has 2 values for each result,
            # presumably 1 number for each polarization.
            # Assume this for now and check later. Pol IDs are
            # unknown so store as '0' and '1'.
            data = row.get('CPARAM')
            flag = row.get('FLAG')
            pols = range(np.shape(data)[0])
            for pol in pols:
                viewresult = commonresultobjects.SpectrumResult(
                  data=np.abs(data[pol,:,0]),
                  flag=flag[pol,:,0],
                  datatype='Bandpass amp', filename=bptable.name,
                  field_id=row.get('FIELD_ID'),
                  spw=row.get('SPECTRAL_WINDOW_ID'),
                  ant=(row.get('ANTENNA1'),
                  antenna_name[row.get('ANTENNA1')]), pol=pol,
                  time=row.get('TIME'), normalise=False)

                # add the view results and their children results to the
                # class result structure
                self.result.addview(viewresult.description, viewresult)
