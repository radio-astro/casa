from __future__ import absolute_import
import types

import pipeline.domain as domain
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
#from pipeline.hif.heuristics import fieldnames as fieldnames
from pipeline.h.heuristics import fieldnames as fieldnames

# create the pipeline logger for this module
LOG = infrastructure.get_logger(__name__)


class CommonCalibrationInputs(basetask.StandardInputs,
                              basetask.OnTheFlyCalibrationMixin):
    """
    CommonCalibrationInputs collects together the parameters common to all
    calibration tasks.
    """

    @property
    def antenna(self):
        if self._antenna is not None:
            return self._antenna

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('antenna')

        antennas = self.ms.get_antenna(self._antenna)
        return ','.join([str(a.id) for a in antennas])

    @antenna.setter
    def antenna(self, value):
        if value is None:
            value = ''
        self._antenna = value

    @property
    def uvrange(self):
        if self._uvrange is not None:
            return self._uvrange

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('uvrange')

        return ''

    @uvrange.setter
    def uvrange(self, value):
        if value is None:
            value = ''
        self._uvrange = value

    @property
    def caltable(self):
        """
        Get the caltable argument for these inputs.

        If set to a table-naming heuristic, this should give a sensible name
        considering the current CASA task arguments.
        """
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('caltable')

        if callable(self._caltable):
            casa_args = self._get_task_args(ignore=('caltable',))
            return self._caltable(output_dir=self.output_dir,
                                  stage=self.context.stage,
                                  **casa_args)
        return self._caltable

    @property
    def field(self):
        if not callable(self._field):
            return self._field

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('field')

        # this will give something like '0542+3243,0343+242'
        intent_fields = self._field(self.ms, self.intent)

        # run the answer through a set, just in case there are duplicates
        fields = set()
        fields.update(utils.safe_split(intent_fields))

        return ','.join(fields)

    @field.setter
    def field(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._field = value

    @property
    def minblperant(self):
        """
        Get the value of minblperant.

        """

        # if minblperant was overridden, return the manually specified value
	if self._minblperant is not None:
            return self._minblperant

        # minblperant is ms-dependent, so if this inputs is handling multiple
        # measurement sets, return minblperant instead.
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('minblperant')

        # if we cannot find the context value without the measurement set
        # set value to 4, otherwise use number of antennas to determine value
        if not self.ms:
            minlperant = 4
        else:
            nant = len(self.ms.antennas)
            if nant < 5:
                minblperant = max(2, nant - 1)
            else:
                minblperant = 4

        # otherwise return whatever we found. We assume the calling function
        # knows how to handle an object of this type.
        return minblperant

    @minblperant.setter
    def minblperant(self, value):
        """
        Set the value of minblperant.

        Setting the value to None restores the default pipeline value.
        """
        self._minblperant = value

    @property
    def refant(self):
        """
        Get the reference antenna.

        refant is normally found by inspecting the context and returning the
        best reference antenna as calculated by a prior 'select reference
        antenna' task. However, if refant has been manually overridden, that
        manually specified value is returned.
    
        If a refant task has not been executed and a manual override value is
        not given, None is returned. 
        """
        # if refant was overridden, return the manually specified value
        if self._refant is not None:
            return self._refant

        # refant is ms-dependent, so if this inputs is handling multiple
        # measurement sets, return a list of refants instead.
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('refant')

        # we cannot find the context value without the measurement set
        if not self.ms:
            return None

        # get the reference antenna for this measurement set
        ant = self.ms.reference_antenna
        if type(ant) is types.ListType:
            ant = ant[0]

        # return the antenna name/id if this is an Antenna domain object 
        if isinstance(ant, domain.Antenna):
            return getattr(ant, 'name', ant.id)

        # otherwise return whatever we found. We assume the calling function
        # knows how to handle an object of this type.
        return ant

    @refant.setter
    def refant(self, value):
        """
        Set the value of refant.

        Setting the value to None lets the current context value take
        precedence.
        """
        self._refant = value

    @property
    def selectdata(self):
        return self.antenna != ''

    # @selectdata.setter
    # def selectdata(self, value):
    #     if value in (None, ''):
    #         # set selectdata True if antenna is specified so that CASA gaincal
    #         # task will check that parameter
    #         value = (self.antenna != '')
    #     else:
    #         # CASA default
    #         value = True
    #
    #     self._selectdata = value

    @property
    def spw(self):
        if self._spw is not None:
            return self._spw

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('spw')

        science_spws = self.ms.get_spectral_windows(self._spw, with_channels=True)
        return ','.join([str(spw.id) for spw in science_spws])

    @spw.setter
    def spw(self, value):
        self._spw = value
