from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.renderer.basetemplates as basetemplates


from pipeline.hif.tasks.flagging import flagdeterbase
from . import vlaagentflagger
from . import flagbaddeformatters
from . import checkflag
from . import targetflag
from . import renderer

from pipeline.hif.tasks.flagging.flagdeterbase import FlagDeterBase
from .flagdetervla import FlagDeterVLA
from .vlaagentflagger import VLAAgentFlagger
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter
from .flagbaddeformatters import FlagBadDeformatters
from .uncalspw import Uncalspw
from .checkflag import Checkflag
from .targetflag import Targetflag
#from .hflag import Heuristicflag

weblog.add_renderer(FlagDeterVLA, renderer.T2_4MDetailsVLAAgentFlaggerRenderer())

weblog.add_renderer(FlagBadDeformatters, basetemplates.T2_4MDetailsDefaultRenderer(uri='flagbaddef.mako',
                                                                      description='Flag bad deformatters', always_rerender=False))
                                                                      
weblog.add_renderer(Checkflag, basetemplates.T2_4MDetailsDefaultRenderer(uri='checkflag.mako',
                                                                      description='Flag possible RFI on BP calibrator using rflag', always_rerender=False))                                                                      

weblog.add_renderer(Targetflag, renderer.T2_4MDetailstargetflagRenderer())