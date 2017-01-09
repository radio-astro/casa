from __future__ import absolute_import

from .antpos import Antpos
from .applycal import IFApplycal
from .atmflag import Atmflag
from .bandpass import BandpassMode, ChannelBandpass, PhcorBandpass
from .bpflagchans import Bandpassflagchans
from .checkproductsize import CheckProductSize
from .findcont import FindCont
from .flagcorrected import Flagcorrected
from .fluxscale import Fluxcal
from .fluxscale import Fluxscale
from .fluxscale import NormaliseFlux
from .gaincal import GaincalMode, GTypeGaincal, GSplineGaincal, KTypeGaincal
from .gainflag import Gainflag
from .lowgainflag import Lowgainflag
from .makeimages import MakeImages
from .makeimlist import MakeImList
from .mstransform import Mstransform
from .rawflagchans import Rawflagchans
from .refant import RefAnt
from .setmodel import Setjy
from .setmodel import SetModels
from .tclean import Tclean
from .uvcontsub import UVcontFit, UVcontSub
from .polarization import Polarization
from .linfeedpolcal import Linfeedpolcal
from .circfeedpolcal import Circfeedpolcal
from .editimlist import Editimlist

# set default tasks for tasks with several implementations to our desired
# specific implementation
Bandpass = PhcorBandpass
Gaincal = GaincalMode
