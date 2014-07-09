from __future__ import absolute_import

from .antpos import Antpos
from .applycal import Applycal
from .atmflag import Atmflag
from .bandpass import BandpassMode, ChannelBandpass, PhcorBandpass
from .bpflagchans import Bandpassflagchans
from .clean import Clean
#from .myclean import MyClean
from .cleanlist import CleanList
from .exportdata import ExportData
from .flagchans import Flagchans
#from .fluxcalflag import FluxcalFlag
#from .flagging import FlagDeterALMA
#from .flagging import AgentFlagger, ALMAAgentFlagger
from .flagging import AgentFlagger
from .fluxscale import Fluxcal
from .fluxscale import Fluxscale
#from .fluxscale import GcorFluxscale
from .fluxscale import NormaliseFlux
#from .gaincal import GaincalMode, GTypeGaincal, GSplineGaincal, TimeGaincal, KTypeGaincal
from .gaincal import GaincalMode, GTypeGaincal, GSplineGaincal, KTypeGaincal
from .importdata import ImportData
from .lowgainflag import Lowgainflag
from .makecleanlist import MakeCleanList
from .refant import RefAnt
from .restoredata import RestoreData
from .setmodel import Setjy
from .setmodel import SetModel
#from .tsyscal import Tsyscal
#from .tsysflag import Tsysflag, Tsysflagchans
#from .wvrgcal import Wvrgcal
#from .wvrgcal import Wvrgcalflag

# set default tasks for tasks with several implementations to our desired
# specific implementation
Bandpass = PhcorBandpass
Gaincal = GaincalMode
