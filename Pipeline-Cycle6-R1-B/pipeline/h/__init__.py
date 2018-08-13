from __future__ import absolute_import

from . import tasks
from . import heuristics
from . import templates

from pipeline.infrastructure.basetask import FailedTask
from pipeline.infrastructure.basetask import FailedTaskResults
from pipeline.infrastructure.renderer import basetemplates
from pipeline.infrastructure.renderer import qaadapter
from pipeline.infrastructure.renderer import weblog

# Register QA handler and weblog renderer for failed tasks.
qaadapter.registry.register_to_miscellaneous_topic(FailedTaskResults)
weblog.add_renderer(FailedTask, basetemplates.T2_4MDetailsFailedTaskRenderer(), group_by=weblog.UNGROUPED)
