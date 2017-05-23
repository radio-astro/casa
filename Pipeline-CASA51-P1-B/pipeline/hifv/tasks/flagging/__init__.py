from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.basetemplates as basetemplates

from . import flagbaddeformatters
from . import checkflag
from . import targetflag
from . import renderer

from . import qa

from .flagdetervla import FlagDeterVLA
from .flagbaddeformatters import FlagBadDeformatters
from .checkflag import Checkflag
from .targetflag import Targetflag
from .flagcal import Flagcal

# pipelineqa.registry.add_handler(qa.FlagBadDeformattersQAHandler())
# pipelineqa.registry.add_handler(qa.FlagBadDeformattersListQAHandler())
# qaadapter.registry.register_to_dataset_topic(flagbaddeformatters.FlagBadDeformattersResults)

# pipelineqa.registry.add_handler(qa.CheckflagQAHandler())
# pipelineqa.registry.add_handler(qa.CheckflagListQAHandler())
# qaadapter.registry.register_to_dataset_topic(checkflag.CheckflagResults)

# pipelineqa.registry.add_handler(qa.TargetflagQAHandler())
# pipelineqa.registry.add_handler(qa.TargetflagListQAHandler())
# qaadapter.registry.register_to_dataset_topic(targetflag.TargetflagResults)


# Use locally defined renderer for VLA deterministic flagging.
weblog.add_renderer(FlagDeterVLA, renderer.T2_4MDetailsFlagDeterVLARenderer(), group_by=weblog.UNGROUPED)

weblog.add_renderer(FlagBadDeformatters,
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='flagbaddef.mako',
                                                              description='Flag bad deformatters'),
                    group_by=weblog.UNGROUPED)

weblog.add_renderer(Checkflag,
                    basetemplates.T2_4MDetailsDefaultRenderer(uri='checkflag.mako',
                                                              description='Flag possible RFI on BP calibrator using rflag'),
                    group_by=weblog.UNGROUPED)

# weblog.add_renderer(Targetflag, renderer.T2_4MDetailstargetflagRenderer(), group_by=weblog.UNGROUPED)

weblog.add_renderer(Targetflag, basetemplates.T2_4MDetailsDefaultRenderer(uri='targetflag.mako',
                                                                          description='Targetflag'),
                    group_by=weblog.UNGROUPED)

weblog.add_renderer(Flagcal, basetemplates.T2_4MDetailsDefaultRenderer(uri='flagcal.mako',
                         description='Flagcal'), group_by=weblog.UNGROUPED)