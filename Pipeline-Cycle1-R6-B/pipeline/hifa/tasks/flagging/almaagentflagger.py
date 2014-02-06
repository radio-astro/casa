from __future__ import absolute_import
import os
import re
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.flagging import agentflagger

LOG = infrastructure.get_logger(__name__)


class ALMAAgentFlaggerInputs(agentflagger.AgentFlaggerInputs):
    """
    Flagger inputs class for ALMA data. It extends the standard inputs with
    an extra parameter (fracspwfps) used when processing ACA windows.
    """
    # override superclass inputs with ALMA-specific values
    edgespw = basetask.property_with_default('edgespw', True)
    fracspw = basetask.property_with_default('fracspw', 0.0625)
    template = basetask.property_with_default('template', True)
    
    # new property for ACA correlator
    fracspwfps = basetask.property_with_default('fracspwfps', 0.048387)

    def __init__(self, context, vis=None, output_dir=None, flagbackup=None,
                  autocorr=None, shadow=None, scan=None, scannumber=None,
                  intents=None, edgespw=None, fracspw=None, fracspwfps=None,
                  online=None, fileonline=None, template=None,
                  filetemplate=None):
        self._init_properties(vars())


class ALMAAgentFlagger(agentflagger.AgentFlagger):
    """
    Agent flagger class for ALMA data.
    """
    Inputs = ALMAAgentFlaggerInputs
    
    def get_fracspw(self, spw):    
        # override the default fracspw getter with our ACA-aware code
        if spw.num_channels in (62, 124, 248):
            return self.inputs.fracspwfps
        else:
            return self.inputs.fracspw
    
    def verify_spw(self, spw):
        # override the default verifier, adding an extra test that bypasses
        # flagging of TDM windows
        super(ALMAAgentFlagger, self).verify_spw(spw)

        # Skip if TDM mode where TDM modes are defined to be modes with 
        # <= 256 channels per correlation
        dd = self.inputs.ms.get_data_description(spw=spw)
        ncorr = len(dd.corr_axis)
        if ncorr*spw.num_channels > 256:
            raise ValueError('Skipping edge flagging for FDM spw %s' % spw.id)            
