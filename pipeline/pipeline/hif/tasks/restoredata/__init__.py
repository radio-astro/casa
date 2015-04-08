from __future__ import absolute_import

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog
from .restoredata import RestoreData

weblog.add_renderer(RestoreData, basetemplates.T2_4MDetailsDefaultRenderer( \
    description='Restore Calibrated Data'), group_by='ungrouped')

