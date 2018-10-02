from __future__ import absolute_import

import numpy as np
import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.h.tasks.common.arrayflaggerbase import FlagCmd

LOG = infrastructure.get_logger(__name__)


class WvrgcalFlagSetterResult(basetask.Results):
    def __init__(self, jobs=None, results=None):
        """
        Initialise the results object with the given list of JobRequests.

        Implemented for compatibility with FlagdataSetter;
        WvrgcalFlagSetter does not run any flag setting tasks.
        """
        if jobs is None:
            jobs = []
        if results is None:
            results = []

        super(WvrgcalFlagSetterResult, self).__init__()

        self.jobs = jobs
        self.results = results

    def merge_with_context(self, context):
        pass


class WvrgcalFlagSetterInputs(vdp.StandardInputs):
    """This class handles the setting of bad antennas in
    wvrgcal.
    """
    def __init__(self, context, table, vis=None, datatask=None):
        self.context = context
        self.vis = vis
        self.datatask = datatask
        self.table = table


class WvrgcalFlagSetter(basetask.StandardTaskTemplate):
    Inputs = WvrgcalFlagSetterInputs

    # override the inherited __init__ method so that references to the
    # task objects can be kept outside self.inputs. Later on the
    # infrastructure replaces self.inputs with a copy, breaking the
    # connection between the self.inputs references to the tasks and 
    # the tasks themselves.
    def __init__(self, inputs):
        self.inputs = inputs
        self.datatask = inputs.datatask
        self._flags_to_set = []

    def prepare(self):

        # Get current WVR flags from datatask.
        wvrflag = set(self.datatask.inputs.wvrflag)

        new_wvrflag = set()
        for f in self._flags_to_set:

            # map antenna ID to antenna name
            # (need to find out if wvrgcal accepts antenna IDs in wvrflag
            # parameter, which would make this step unnecessary).
            ms = self.inputs.context.observing_run.get_ms(name=f.filename)
            antennas = ms.antennas
           
            axisnames = np.array(f.axisnames)
            index = np.arange(len(axisnames))
            antindex = [i for i in index
                        if re.match('antenna$', axisnames[i], re.I)]
            antindex = antindex[0]

            # update the set of wvrflag antennas that have attracted flagging
            new_wvrflag.update(
                [antenna.name for antenna in antennas
                 if antenna.id == f.flagcoords[antindex]])

        new_wvrflag = list(new_wvrflag)
        new_wvrflag = map(str, new_wvrflag)

        # Update the inputs of the datatask to include the flags newly found by
        # pipeline heuristics, to be used during next iteration of datatask.
        if new_wvrflag:
            LOG.info('additional antennas to be flagged: %s' % new_wvrflag)
            wvrflag.update(new_wvrflag)
            self.datatask.inputs.wvrflag = list(wvrflag)
        else:
            LOG.info('no additional antennas need be flagged')

        # Return a placeholder result structure, for compatibility with
        # FlagdataSetter.
        return WvrgcalFlagSetterResult(jobs=[], results=[])

    def analyse(self, result):
        return result

    def flags_to_set(self, flags):
        # Only add flag commands to the list of flags to set; this will ignore
        # requests for flagging summaries that are represented as strings.
        self._flags_to_set.extend(
            [flag for flag in flags if isinstance(flag, FlagCmd)])
