from __future__ import absolute_import

import numpy as np
import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class WvrgcalFlagSetterResult(basetask.Results):
    def __init__(self):
        super(WvrgcalFlagSetterResult, self).__init__()

    def merge_with_context(self, context):
        pass


class WvrgcalFlagSetterInputs(basetask.StandardInputs):
    """This class handles the setting of bad antennas in
    wvrgcal.
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, 
      table=None, vis=None, wvrgcaltask=None):
        self._init_properties(vars())
        
        # include for compatibility with flagdata setter.
        self.inpfile = 'not used'
        self.reason = 'not used'


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
        # get current wvrflag value
        wvrflag = set(self.wvrgcaltask.inputs.wvrflag)
        new_wvrflag = set()
        for f in self._flags_to_set:
            # map antenna ID to antenna name
            # (need to find out if wvrgcal accepts antenna IDs in wvrflag
            # parameter, which would make this step unnecessary).
            ms = self.inputs.context.observing_run.get_ms(name=f.filename)
            antennas = ms.antennas
           
            axisnames = np.array(f.axisnames)
            index = np.arange(len(axisnames))
            antindex = [i for i in index if re.match('antenna$', axisnames[i],
              re.I)]
            antindex = antindex[0]

            # update the set of wvrflag antennas that have attracted flagging
            new_wvrflag.update([antenna.name for antenna in antennas 
              if antenna.id==f.flagcoords[antindex]])

        new_wvrflag = list(new_wvrflag)
        new_wvrflag = map(str, new_wvrflag)

        # set parameter of wvrgcal task for next calculation
        if new_wvrflag:
            LOG.info('additional antennas to be flagged: %s' % new_wvrflag)
            wvrflag.update(new_wvrflag)
            self.wvrgcaltask.inputs.wvrflag = list(wvrflag)
        else:
            LOG.info('no additional antennas need be flagged')

        return result

    def analyse(self, result):
        return result

