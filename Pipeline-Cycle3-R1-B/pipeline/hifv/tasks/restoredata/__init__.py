from __future__ import absolute_import

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

from .vlarestoredata import VLARestoreData

weblog.add_renderer(VLARestoreData, basetemplates.T2_4MDetailsDefaultRenderer(
      description='Restore calibrated data'))
