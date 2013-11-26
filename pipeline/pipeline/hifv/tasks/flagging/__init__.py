from __future__ import absolute_import

from pipeline.hif.tasks.flagging.flagdeterbase import FlagDeterBase
from .flagdetervla import FlagDeterVLA
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter
from .flagbaddeformatters import FlagBadDeformatters
from .uncalspw import Uncalspw
from .checkflag import Checkflag
from .targetflag import Targetflag
from .hflag import Heuristicflag