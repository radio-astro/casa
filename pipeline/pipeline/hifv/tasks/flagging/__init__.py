from __future__ import absolute_import
import pipeline.infrastructure.renderer.weblog as weblog



from pipeline.hif.tasks.flagging import flagdeterbase
from . import vlaagentflagger
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