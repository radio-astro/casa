from __future__ import absolute_import

from .antpos import Antpos
from .applycal import Applycal
from .bandpass import BandpassMode, ChannelBandpass, PhcorBandpass
from .clean import Clean
from .cleanlist import CleanList
from .exportdata import ExportData
from .flagging import FlagDeterALMA
from .fluxscale import Fluxcal
from .fluxscale import Fluxscale
from .fluxscale import GcorFluxscale
from .fluxscale import NormaliseFlux
from .gaincal import GaincalMode, GTypeGaincal, GSplineGaincal
from .importdata import ImportData
from .makecleanlist import MakeCleanList
from .refant import RefAnt
from .setmodel import Setjy
from .setmodel import SetModel
from .tsyscal import Tsyscal
from .tsysflag import Tsysflag, Tsysflagedges
from .wvrgcal import Wvrgcal
from .wvrgcal import Wvrgcalflag

# set default tasks for tasks with several implementations to our desired
# specific implementation
Bandpass = PhcorBandpass
Gaincal = GaincalMode
