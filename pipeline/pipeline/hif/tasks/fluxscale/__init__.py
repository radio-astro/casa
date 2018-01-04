from __future__ import absolute_import

# The following 2 tasks are old general purpose utility tasks which
# are no longer used or imported into the task dictionaries but are
# retained for the purposes of illusrating how to combine several
# CASA tasks to derive a flux scale and how to do a simple flux
# normalization.

#from .fluxcal import Fluxcal
#from .normflux import NormaliseFlux

from .fluxscale import Fluxscale

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates

weblog.add_renderer(Fluxscale, 
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='fluxscale.mako',
                                                              description='Compute calibrator source fluxes'),
                    group_by='session')
