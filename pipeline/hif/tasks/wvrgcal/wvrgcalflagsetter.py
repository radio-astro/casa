from __future__ import absolute_import

import numpy as np
import re

import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)


class WvrgcalFlagSetterResult(api.Results):

    def merge_with_context(self, context):
        pass


class WvrgcalFlagSetterInputs(basetask.StandardInputs):
    """This class handles the setting of bad antennas in
    wvrgcal.
    """
    def __init__(self, context, output_dir=None, vis=None,
      wvrgcaltask=None):
        self._init_properties(vars())


class WvrgcalFlagSetter(basetask.StandardTaskTemplate):
    Inputs = WvrgcalFlagSetterInputs

    # override the inherited __init__ method so that references to the
    # task objects can be kept outside self.inputs. Later on the
    # infrastructure replaces self.inputs with a copy, breaking the
    # connection between the self.inputs references to the tasks and 
    # the tasks themselves.
    def __init__(self, inputs):
        self.inputs = inputs
        self.wvrgcaltask = inputs.wvrgcaltask

    def flags_to_set(self, flags):
        self._flags_to_set = flags

    def prepare(self):
        result = WvrgcalFlagSetterResult()
        wvrflag = set()
        vis = {}
        for f in self._flags_to_set:
            # ensure we know which ms the wvr table is derived from - we
            # need to know this to be able map antenna ID to antenna name
            # (need to find out if wvrgcal accepts antenna IDs in wvrflag
            # parameter, which would make this step unnecessary).
            if f.filename not in vis.keys():
                with casatools.TableReader(f.filename) as table:
                    visname = table.getkeyword('MSName')
                    vis[f.filename] = visname

            ms = self.inputs.context.observing_run.get_ms(
              name=vis[f.filename])
            antennas = ms.antennas
           
            axisnames = np.array(f.axisnames)
            index = np.arange(len(axisnames))
            antindex = [i for i in index if re.match('antenna$', axisnames[i],
              re.I)]
            antindex = antindex[0]

            # update the set of wvrflag antennas that have attracted flagging
            for flagcoord in f.flagcoords:
                wvrflag.update([antenna.name for antenna in antennas 
                  if antenna.id==flagcoord[antindex]])

        wvrflag = list(wvrflag)
        wvrflag = map(str, wvrflag)

        # set parameter of wvrgcal task for next calculation
        if wvrflag:
            LOG.info('antennas to be flagged: %s' % wvrflag)
        else:
            LOG.info('no antennas need be flagged')
        self.wvrgcaltask.inputs.wvrflag = wvrflag

        return result

    def analyse(self, result):
        return result

