from __future__ import absolute_import

from .antpos import Antpos
from .applycal import Applycal
from .atmflag import Atmflag
from .bandpass import BandpassMode, ChannelBandpass, PhcorBandpass
from .bpflagchans import Bandpassflagchans
from .clean import Clean
from .cleanlist import CleanList
from .exportdata import ExportData
from .findcont import FindCont
from .flagcorrected import Flagcorrected
from .flagging import AgentFlagger
from .fluxscale import Fluxcal
from .fluxscale import Fluxscale
from .fluxscale import NormaliseFlux
from .gaincal import GaincalMode, GTypeGaincal, GSplineGaincal, KTypeGaincal
from .gainflag import Gainflag
from .importdata import ImportData
from .lowgainflag import Lowgainflag
from .makecleanlist import MakeCleanList
from .makeimages import MakeImages
from .makeimlist import MakeImList
from .mstransform import Mstransform
from .rawflagchans import Rawflagchans
from .refant import RefAnt
from .restoredata import RestoreData
from .setmodel import Setjy
from .setmodel import SetModels
from .tclean import Tclean
from .uvcontsub import UVcontFit

# set default tasks for tasks with several implementations to our desired
# specific implementation
Bandpass = PhcorBandpass
Gaincal = GaincalMode
