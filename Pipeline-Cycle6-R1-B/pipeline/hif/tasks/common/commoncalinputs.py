from __future__ import absolute_import
import types

import pipeline.domain as domain
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import fieldnames as fieldnames

# create the pipeline logger for this module
LOG = infrastructure.get_logger(__name__)


class VdpCommonCalibrationInputs(vdp.StandardInputs):
    """
    CommonCalibrationInputs collects together the parameters common to all
    calibration tasks.
    """
    @vdp.VisDependentProperty
    def antenna(self):
        return ''

    @antenna.convert
    def antenna(self, value):
        antennas = self.ms.get_antenna(value)
        # TODO enable this once sessions behaviour has been verified and merged to the main trunk
        # if all antennas are selected, return ''
        # if len(antennas) == len(self.ms.antennas):
        #     return ''
        return utils.find_ranges([a.id for a in antennas])

    @vdp.VisDependentProperty
    def field(self):
        # this will give something like '0542+3243,0343+242'
        field_finder = fieldnames.IntentFieldnames()
        intent_fields = field_finder.calculate(self.ms, self.intent)

        # run the answer through a set, just in case there are duplicates
        fields = set()
        fields.update(utils.safe_split(intent_fields))

        return ','.join(fields)

    @vdp.VisDependentProperty
    def minblperant(self):
        # set value to 4, otherwise use number of antennas to determine value
        num_antennas = len(self.ms.antennas)
        if num_antennas < 5:
            return max(2, num_antennas - 1)
        else:
            return 4

    opacity = vdp.VisDependentProperty(default='')

    @vdp.VisDependentProperty
    def refant(self):
        LOG.todo('What happens if self.ms is None?')
        # we cannot find the context value without the measurement set
        if not self.ms:
            return None

        # get the reference antenna for this measurement set
        ant = self.ms.reference_antenna
        if isinstance(ant, list):
            ant = ant[0]

        # return the antenna name/id if this is an Antenna domain object
        if isinstance(ant, domain.Antenna):
            return getattr(ant, 'name', ant.id)

        # otherwise return whatever we found. We assume the calling function
        # knows how to handle an object of this type.
        return ant

    @vdp.VisDependentProperty
    def selectdata(self):
        # set selectdata to True if antenna is specified so that CASA gaincal
        # task will check that parameter
        return self.antenna != ''

    @vdp.VisDependentProperty
    def spw(self):
        science_spws = self.ms.get_spectral_windows(with_channels=True, science_windows_only=True)
        return ','.join([str(spw.id) for spw in science_spws])

    uvrange = vdp.VisDependentProperty(default='')

    def __init__(self, context, output_dir=None, vis=None, intent=None, field=None, spw=None, refant=None,
                 antenna=None, minblperant=None, opacity=None, selectdata=None, uvrange=None):
        super(VdpCommonCalibrationInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.intent = intent
        self.field = field
        self.spw = spw
        self.antenna = antenna
        self.refant = refant

        # solution parameters
        self.minblperant = minblperant
        self.opacity = opacity
        self.selectdata = selectdata
        self.uvrange = uvrange
